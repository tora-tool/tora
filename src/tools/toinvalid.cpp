
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

#include "tools/toinvalid.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/tochangeconnection.h"
#include "core/toextract.h"
#include "editor/todebugtext.h"
#include "core/tosql.h"
#include "core/totool.h"
#include "core/toconnectiontraits.h"
#include "core/toconnectionsub.h"
#include "core/toconnectionsubloan.h"

#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QToolBar>
#include <QToolButton>
#include <QSplitter>
#include <QtGui/QPixmap>
#include <QProgressDialog>

#include "icons/refresh.xpm"
#include "icons/toinvalid.xpm"
#include "icons/compile.xpm"
#include "toresultcode.h"
#include "toresulttableview.h"
#include "toresultview.h"

#include <limits>

static toSQL SQLListInvalid("toInvalid:ListInvalid",
                            "SELECT owner \"Owner\",object_name \"Object\",object_type \"Type\",status \"Status\"\n"
                            "  FROM sys.all_objects\n"
                            " WHERE status <> 'VALID'",
                            "Get invalid objects, must have same first three columns.");

static toSQL SQLListSource("toInvalid:ListSource",
                           "SELECT Text FROM SYS.ALL_SOURCE\n"
                           " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]> AND type = :f3<char[101]>",
                           "List source of an object.");

static toSQL SQLReadErrors("toInvalid:ReadErrors",
                           "SELECT Line,Text FROM sys.All_Errors\n"
                           " WHERE OWNER = :f1<char[101]>\n"
                           "   AND NAME = :f2<char[101]>\n"
                           "   AND TYPE = :f3<char[101]>"
                           " ORDER BY Type,Line",
                           "Get lines with errors in object");


class toInvalidTool : public toTool
{
    public:
        toInvalidTool()
            : toTool(130, "Invalid Objects") { }
        virtual const char *menuItem()
        {
            return "Invalid Objects";
        }

        virtual bool canHandle(const toConnection &conn)
        {
            return (conn.providerIs("Oracle"));
        }

        virtual toToolWidget* toolWindow(QWidget *parent, toConnection &connection)
        {
            return new toInvalid(parent, connection);
        }
        virtual void closeWindow(toConnection &connection) {};

    private:
        virtual const char **pictureXPM(void)
        {
            return const_cast<const char**>(toinvalid_xpm);
        }
};


static toInvalidTool InvalidTool;


toInvalid::toInvalid(QWidget *main, toConnection &connection)
    : toToolWidget(InvalidTool, "invalid.html", main, connection, "toInvalid")
{

    QToolBar *toolbar = Utils::toAllocBar(this, tr("Invalid Objects"));
    layout()->addWidget(toolbar);

    QAction *refreshAct =
        toolbar->addAction(
            QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
            tr("Refresh list"),
            this,
            SLOT(refresh()));
    refreshAct->setShortcut(QKeySequence::Refresh);

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(compile_xpm))),
                       tr("Recompile all invalid"),
                       this,
                       SLOT(recompileSelected()));

    toolbar->addWidget(new Utils::toSpacer());

    new toChangeConnection(toolbar);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(splitter);

    Objects = new toResultTableView(false, false, splitter);
    Objects->setSQL(SQLListInvalid);

    connect(Objects, SIGNAL(selectionChanged()), this, SLOT(changeSelection()));

    Source = new toResultCode(splitter);
    Source->setHeading(false);
    Source->setSQL(SQLListSource);

    connect(Source, SIGNAL(executed()), this, SLOT(refresh()));

    refresh();
    setFocusProxy(Objects);
}

void toInvalid::recompileSelected(void)
{
    QProgressDialog progress(tr("Recompiling all invalid"),
                             tr("Cancel"),
                             0,
                             Objects->model()->rowCount(),
                             this);
    progress.setWindowTitle("Recompiling");
    progress.show();

    toConnectionTraits const& traits(connection().getTraits());
    toConnectionSubLoan conn(connection());
    for (toResultTableView::iterator it(Objects); (*it).isValid(); it++)
    {
        progress.setLabelText("Recompiling " +
                              Objects->model()->data((*it).row(), 2).toString() +
                              "." +
                              Objects->model()->data((*it).row(), 3).toString());
        progress.setValue(progress.value() + 1);

        qApp->processEvents();

        if (progress.wasCanceled())
            break;

        QString type  = Objects->model()->data((*it).row(), 3).toString();
        QString owner = Objects->model()->data((*it).row(), 1).toString();
        QString name  = Objects->model()->data((*it).row(), 2).toString();
        QString sql;
        if (type == "INDEX")
            sql = QString("ALTER INDEX %1.%2 REBUILD")
            .arg(traits.quote(owner))
            .arg(traits.quote(name));
        else if (type == "PACKAGE BODY")
            sql = QString("ALTER PACKAGE %1.%2 COMPILE BODY")
            .arg(traits.quote(owner))
            .arg(traits.quote(name));
        else if (type == "TYPE BODY")
            sql = QString("ALTER TYPE %1.%2 COMPILE BODY")
            .arg(traits.quote(owner))
            .arg(traits.quote(name));
        else if ((type == "SYNONYM") && (Objects->model()->data((*it).row(), 1).toString() == "PUBLIC"))
        {
            // only SYS user is allowed to do ALTER PUBLIC SYNONYM ...
            // other users can only do CREATE OR REPLACE PUBLIC SYNONYM ...
            QList<QPair<QString,toCache::ObjectRef> > objects;
            toExtract extract(const_cast<toConnection&>(conn.ParentConnection), NULL);
            extract.setCode(true);
            extract.setHeading(false);
            extract.setPrompt(false);
            extract.setReplace(true); // get create OR REPLACE statement
            objects.append(QPair<QString,toCache::ObjectRef>(type, toCache::ObjectRef("PUBLIC", name, "")));
            sql = extract.create(objects);
            throw tr("recompileSelected SYNONYM not implement yet");
        }
        else
            sql = QString("ALTER %1 %2.%3 COMPILE")
            .arg(type)
            .arg(traits.quote(owner))
            .arg(traits.quote(name));

        try
        {
            // remove trailing newlines, spaces, tabs and semicolons from execution
            // as this could cause "execution" of empty statement (doing nothing)
            int l = sql.length() - 1;
            while (l >= 0 && (sql.at(l) == ';' || sql.at(l).isSpace()))
                l--;

            TLOG(2, toDecorator, __HERE__) << "statement=" << sql.mid(0, l + 1) << std::endl;

            if (l >= 0)
            {
                conn.execute(sql.mid(0, l + 1));
            }
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        }
    }

    if (progress.isVisible())
        progress.close();

    qApp->processEvents();
    this->refresh();
}

void toInvalid::refresh(void)
{
    QModelIndex item = Objects->selectedIndex();

    QString owner;
    QString object;
    QString type;
    if (item.isValid())
    {
        owner = Objects->model()->data(item.row(), 1).toString();
        object = Objects->model()->data(item.row(), 2).toString();
        type = Objects->model()->data(item.row(), 3).toString();
    }

    Objects->refresh();

    if (item.isValid())
    {
        for (toResultTableView::iterator it(Objects); (*it).isValid(); it++)
        {
            if (Objects->model()->data(item.row(), 1).toString() == owner &&
                    Objects->model()->data(item.row(), 2).toString() == object &&
                    Objects->model()->data(item.row(), 3).toString() == type)
            {

                Objects->selectionModel()->select(QItemSelection((*it), (*it)),
                                                  QItemSelectionModel::ClearAndSelect);
                Objects->setCurrentIndex((*it));
                break;
            }
        }

        changeSelection();
    }
}

void toInvalid::changeSelection(void)
{
    try
    {
        QModelIndex item = Objects->selectedIndex();
        if (item.isValid())
        {
            int row = item.row();
            QString owner = Objects->model()->data(row, 1).toString();
            QString object = Objects->model()->data(row, 2).toString();
            QString type = Objects->model()->data(row, 3).toString();

            Source->refreshWithParams( toQueryParams() << owner << object << type);

            QMap<int, QString> Errors;
            toConnectionSubLoan conn(connection());
            toQuery errors(conn,
                           SQLReadErrors,
                           toQueryParams() << owner << object << type);

            int firstErrorLine = (std::numeric_limits<int>::max)();
            while (!errors.eof())
            {
                int line = errors.readValue().toInt() + Source->offset();
                Errors[line] += QString::fromLatin1(" ");
                Errors[line] += (QString)errors.readValue();
                firstErrorLine = (std::min)(firstErrorLine, line);
            }
            Source->setErrors(Errors);
            Source->gotoLine(firstErrorLine);
        }
    }
    TOCATCH;
}
