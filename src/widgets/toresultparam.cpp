
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "widgets/toresultparam.h"
#include "core/utils.h"
#include "core/tosql.h"
#include "widgets/toresultview.h"
#include "editor/tomemoeditor.h"
#include "core/toconnection.h"
#include "widgets/toresultlong.h"
#include "core/toraversion.h"
#include "core/toconnectionsub.h"

#include <QLabel>
#include <QLineEdit>
#include <QtCore/QRegExp>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>
#include <QtGui/QPixmap>
#include <QVBoxLayout>

#include "icons/database.xpm"
#include "icons/filesave.xpm"
#include "icons/scansource.xpm"
#include "icons/tocurrent.xpm"
#include "icons/trash.xpm"
#include "icons/commit.xpm"

static toSQL SQLParamsMySQL("toResultParam:ListParam",
                            "show variables",
                            "List parameters available in the session",
                            "4.0",
                            "QMYSQL");

static toSQL SQLParamsGlobal("toResultParam:ListGlobal",
                             "show global variables",
                             "List parameters available in the database",
                             "4.0",
                             "QMYSQL");

static toSQL SQLParams("toResultParam:ListParam",
                       "select name \"Parameter\",value \"Value\",' ' \"Changed\",\n"
                       "       description \"Description\", num \" Number\",\n"
                       "       type \" Type\",isdefault \" Default\",\n"
                       "       isses_modifiable \" Sesmod\",issys_modifiable \" Sysmod\"\n"
                       "  from v$parameter order by name",
                       "");

static toSQL SQLHiddenParams("toResultParam:ListHidden",
                             "SELECT KSPPINM \"Parameter\",\n"
                             "       KSPFTCTXVL \"Value\",\n"
                             "       ' ' \"Changed\",\n"
                             "       KSPPDESC \"Description\",\n"
                             "       KSPFTCTXPN \" Num\",\n"
                             "       ksppity \" Type\",\n"
                             "       KSPFTCTXDF \" Default\",\n"
                             "       DECODE(MOD(TRUNC(KSPPIFLG/256),2),0,'FALSE','TRUE') \" Sesmod\",\n"
                             "       DECODE(MOD(TRUNC(KSPPIFLG/65536),8),0,'FALSE','TRUE') \" Sysmod\"\n"
                             "  FROM X$KSPPI x,\n"
                             "       X$KSPPCV2 y\n"
                             " WHERE x.INDX+1=y.KSPFTCTXPN ORDER BY KSPPINM",
                             "List parameters available in the database including hidden parameters");

bool toResultParam::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle") || conn.providerIs("QMYSQL");
}

toResultParam::toResultParam(QWidget *parent, const char *name)
    : QWidget(parent)
{

    if (name)
        setObjectName(name);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    QToolBar *toolbar = Utils::toAllocBar(this, tr("Parameter editor"));
    vbox->addWidget(toolbar);

    Toggle = new QToolButton(toolbar);
    Toggle->setCheckable(true);

    if (connection().providerIs("Oracle"))
    {
        Toggle->setIcon(QIcon(QPixmap(const_cast<const char**>(scansource_xpm))));
        connect(Toggle, SIGNAL(toggled(bool)), this, SLOT(showHidden(bool)));
        Toggle->setToolTip(
            tr("Display hidden parameters. This will only work if you are "
               "logged in as the sys user."));
    }
    else
    {
        QIcon iconset(QPixmap(const_cast<const char**>(tocurrent_xpm)));
        iconset.addPixmap(QPixmap(const_cast<const char**>(database_xpm)),
                          QIcon::Normal,
                          QIcon::On);
        Toggle->setIcon(iconset);
        connect(Toggle, SIGNAL(toggled(bool)), this, SLOT(showGlobal(bool)));
        Toggle->setToolTip(
            tr("Switch between global and session variables to show."));
    }
    toolbar->addWidget(Toggle);

    toolbar->addSeparator();

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(filesave_xpm))),
                       tr("Generate configuration file"),
                       this,
                       SLOT(generateFile()));

    toolbar->addSeparator();

    if (connection().providerIs("Oracle"))
    {
        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(database_xpm))),
                           tr("Apply changes to system"),
                           this,
                           SLOT(applySystem()));

        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(tocurrent_xpm))),
                           tr("Apply changes to session"),
                           this,
                           SLOT(applySession()));
    }
    else
    {
        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(commit_xpm))),
                           tr("Apply changes"),
                           this,
                           SLOT(applyChanges()));
    }

    if (connection().providerIs("Oracle"))
    {
        toolbar->addSeparator();

        toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(trash_xpm))),
                           tr("Drop current changes"),
                           this,
                           SLOT(dropChanges()));
    }

    toolbar->addWidget(new Utils::toSpacer());

    Params = new toResultLong(false, false, this);
    vbox->addWidget(Params);
    Params->setSQL(SQLParams);
    Params->setReadAll(true);
    Params->setSelectionMode(toTreeWidget::Single);
    connect(Params, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    connect(Params, SIGNAL(done()), this, SLOT(done()));

    Value = new QLineEdit(this);
    Value->setEnabled(false);
    vbox->addWidget(Value);

    refresh();
}

void toResultParam::showHidden(bool hid)
{
    if (hid)
        Params->setSQL(SQLHiddenParams);
    else
        Params->setSQL(SQLParams);
    refresh();
}

void toResultParam::showGlobal(bool glb)
{
    if (glb)
        Params->setSQL(SQLParamsGlobal);
    else
        Params->setSQL(SQLParams);
    refresh();
}

void toResultParam::query(const QString &sql, const toQueryParams &param)
{
    saveChange();
    LastItem = QString::null;

    Params->refresh();
}

void toResultParam::dropChanges(void)
{
    NewValues.clear();
    refresh();
}

void toResultParam::done()
{
    for (toTreeWidgetItem *item = Params->firstChild(); item; item = item->nextSibling())
    {
        std::map<QString, QString>::iterator i = NewValues.find(item->text(0));
        if (i != NewValues.end())
        {
            item->setText(1, (*i).second);
            item->setText(6, QString::fromLatin1("FALSE"));
            item->setText(2, tr("Changed"));
        }
    }
}

void toResultParam::saveChange()
{
    if (!LastItem.isEmpty() && LastValue != Value->text())
    {
        NewValues[LastItem] = Value->text();
        LastValue = Value->text();
        for (toTreeWidgetItem *item = Params->firstChild(); item; item = item->nextSibling())
        {
            if (item->text(0) == LastItem)
            {
                item->setText(1, LastValue);
                item->setText(6, QString::fromLatin1("FALSE"));
                item->setText(2, tr("Changed"));
                break;
            }
        }
    }
}

void toResultParam::generateFile(void)
{
    saveChange();
    QString str = tr("# Generated by TOra version %1\n\n").arg(QString::fromLatin1(TORAVERSION));
    QRegExp comma(QString::fromLatin1("\\s*,\\s+"));
    for (toTreeWidgetItem *item = Params->firstChild(); item; item = item->nextSibling())
    {
        if (connection().providerIs("Oracle"))
        {
            if (item->text(6) == QString::fromLatin1("FALSE"))
            {
                str += item->text(0);
                str += QString::fromLatin1(" = ");
                if (item->text(5) == QString::fromLatin1("2"))
                {
                    QStringList lst = item->text(1).split(comma);
                    if (lst.count() > 1)
                        str += QString::fromLatin1("( ");
                    for (int i = 0; i < lst.count(); i++)
                    {
                        if (i > 0)
                            str += QString::fromLatin1(", ");
                        str += QString::fromLatin1("\"") + lst[i] + QString::fromLatin1("\"");
                    }
                    if (lst.count() > 1)
                        str += QString::fromLatin1(" )");
                }
                else
                    str += item->text(1);
                str += QString::fromLatin1("\n");
            }
        }
        else
        {
            str += item->text(0) + " = '" + item->text(1) + "'\n";
        }
    }
    connect(new toMemoEditor(this, str, 0, 0), SIGNAL(changeData(int, int, const QString &)),
            this, SLOT(changedData(int, int, const QString &)));
}

void toResultParam::applySession(void)
{
    try
    {
        saveChange();
        toConnection &conn = connection();
        for (toTreeWidgetItem *item = Params->firstChild(); item; item = item->nextSibling())
        {
            if (item->text(2) == tr("Changed"))
            {
                try
                {
                    if (item->text(7) != QString::fromLatin1("FALSE"))
                    {
                        QString str = QString::fromLatin1("ALTER SESSION SET ");
                        str += item->text(0);
                        str += QString::fromLatin1(" = ");
                        if (item->text(5) == QString::fromLatin1("2"))
                        {
                            str += QString::fromLatin1("'");
                            str += item->text(1);
                            str += QString::fromLatin1("'");
                        }
                        else
                            str += item->text(1);
                        conn.allExecute(str);
                        std::map<QString, QString>::iterator i = NewValues.find(item->text(0));
                        if (i != NewValues.end())
                            NewValues.erase(i);
                    }
                }
                TOCATCH
            }
        }
    }
    TOCATCH
    refresh();
}

void toResultParam::applySystem(void)
{
    try
    {
        saveChange();
        toConnectionSubLoan conn(connection());
        for (toTreeWidgetItem *item = Params->firstChild(); item; item = item->nextSibling())
        {
            if (item->text(2) == tr("Changed"))
            {
                try
                {
                    if (item->text(8) != QString::fromLatin1("FALSE"))
                    {
                        QString str = QString::fromLatin1("ALTER SYSTEM SET ");
                        str += item->text(0);
                        str += QString::fromLatin1(" = ");
                        if (item->text(5) == QString::fromLatin1("2"))
                        {
                            str += QString::fromLatin1("'");
                            str += item->text(1);
                            str += QString::fromLatin1("'");
                        }
                        else
                            str += item->text(1);
                        conn->execute(str);
                        std::map<QString, QString>::iterator i = NewValues.find(item->text(0));
                        if (i != NewValues.end())
                            NewValues.erase(i);
                    }
                }
                TOCATCH
            }
        }
    }
    TOCATCH
    refresh();
}

// Only used by MySQL
void toResultParam::applyChanges(void)
{
    try
    {
        saveChange();
        toConnectionSubLoan conn(connection());

        for (toTreeWidgetItem *item = Params->firstChild(); item; item = item->nextSibling())
        {
            if (item->text(2) == tr("Changed"))
            {
                try
                {
                    QString str = "SET ";
                    if (!Toggle->isChecked())
                        str += "GLOBAL ";
                    else
                        str += "SESSION ";
                    str += item->text(0);
                    str += " = ";
                    str += QString::fromLatin1("'");
                    str += item->text(1);
                    str += QString::fromLatin1("'");
                    conn->execute(str);
                    std::map<QString, QString>::iterator i = NewValues.find(item->text(0));
                    if (i != NewValues.end())
                        NewValues.erase(i);
                }
                TOCATCH
            }
        }
    }
    TOCATCH
    refresh();
}

void toResultParam::changeItem(void)
{
    saveChange();

    toTreeWidgetItem *item = Params->selectedItem();
    if (item)
    {
        LastItem = item->text(0);
        LastValue = item->text(1);
        Value->setText(LastValue);
        Value->setEnabled(true);
    }
    else
    {
        LastItem = QString::null;
        Value->setEnabled(false);
    }
}

void toResultParam::changedData(int, int, const QString &data)
{
    QString file = Utils::toSaveFilename(QString::null, connection().providerIs("Oracle") ? "*.pfile" : "*.conf", this);
    if (!file.isEmpty())
        Utils::toWriteFile(file, data);
}
