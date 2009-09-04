
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosql.h"
#include "tosqledit.h"
#include "totool.h"
#include "toworksheet.h"

#include <qcombobox.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlineedit.h>
#include "totreewidget.h"
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qsplitter.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <QMdiArea>

#include <QString>
#include <QFrame>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "icons/add.xpm"
#include "icons/commit.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/tosqledit.xpm"
#include "icons/trash.xpm"

class toSQLEditTool : public toTool
{
protected:
    QWidget * Window;

public:
    toSQLEditTool()
            : toTool(920, "SQL Dictionary Editor")
    {
        Window = NULL;
    }

    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        if (Window)
        {
            Window->setFocus(Qt::OtherFocusReason);
            // already opened, prevent creating new sub window
            return 0;
        }

        Window = new toSQLEdit(parent, connection);
        Window->setWindowIcon(QPixmap(const_cast<const char**>(tosqledit_xpm)));
        return Window;
    }

    virtual void customSetup()
    {
        toMainWidget()->getEditMenu()->addAction(
            QIcon(tosqledit_xpm),
            qApp->translate("toSQLEditTool", "&Edit SQL..."),
            this,
            SLOT(createWindow()));
        toMainWidget()->registerSQLEditor(key());
    }

    void closeWindow(void)
    {
        Window = NULL;
    }

    virtual bool canHandle(toConnection &)
    {
        return true;
    }

    virtual void closeWindow(toConnection &connection) {};
};

static toSQLEditTool SQLEditTool;

void toSQLEdit::updateStatements(const QString &sel)
{
    Statements->clear();
    toSQL::sqlMap sql = toSQL::definitions();

    toTreeWidgetItem *head = NULL;
    toTreeWidgetItem *item = NULL;

    for (toSQL::sqlMap::iterator pos = sql.begin();pos != sql.end();pos++)
    {
        QString str = (*pos).first;
        int i = str.indexOf(QString::fromLatin1(":"));
        if (i >= 0)
        {
            if (!head || head->text(0) != str.left(i))
            {
                head = new toTreeWidgetItem(Statements, str.left(i));
                head->setSelectable(false);
            }
            item = new toTreeWidgetItem(head, str.right(str.length() - i - 1));
            if (sel == str)
            {
                Statements->setSelected(item, true);
                Statements->setCurrentItem(item);
                Statements->setOpen(head, true);
            }
        }
        else
        {
            head = new toTreeWidgetItem(Statements, head, str);
            if (sel == str)
            {
                Statements->setSelected(head, true);
                Statements->setCurrentItem(item);
            }
        }
    }
}

toSQLEdit::toSQLEdit(QWidget *main, toConnection &connection)
        : toToolWidget(SQLEditTool, "sqledit.html", main, connection, "toSQLEdit")
{

    QToolBar *toolbar = toAllocBar(this, tr("SQL editor"));
    layout()->addWidget(toolbar);

    toolbar->addAction(QIcon(fileopen_xpm),
                       tr("Load SQL dictionary file"),
                       this,
                       SLOT(loadSQL()));
    toolbar->addAction(QIcon(filesave_xpm),
                       tr("Save modified SQL to dictionary file"),
                       this,
                       SLOT(saveSQL()));
    toolbar->addSeparator();

    CommitButton = toolbar->addAction(
        QIcon(commit_xpm),
        tr("Save this entry in the dictionary"),
        this,
        SLOT(commitChanges()));

    TrashButton = toolbar->addAction(
        QIcon(trash_xpm),
        tr("Delete this version from the SQL dictionary"),
        this,
        SLOT(deleteVersion()));

    toolbar->addAction(QIcon(add_xpm),
                       tr("Start new SQL definition"),
                       this,
                       SLOT(newSQL()));

    CommitButton->setEnabled(true);
    TrashButton->setEnabled(false);

    toolbar->addWidget(new toSpacer());

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(splitter);

    Statements = new toListView(splitter);
    Statements->setRootIsDecorated(true);
    Statements->addColumn(tr("Text Name"));
    Statements->setSorting(0);
    Statements->setSelectionMode(toTreeWidget::Single);

    QWidget *vbox = new QWidget(splitter);
    QVBoxLayout *vlay = new QVBoxLayout;

    QWidget *hbox = new QWidget(vbox);
    QHBoxLayout *hlay = new QHBoxLayout;
    vlay->addWidget(hbox);

    hlay->addWidget(new QLabel(tr("Name") + " ", hbox));

    Name = new QLineEdit(hbox);
    hlay->addWidget(Name);

    hlay->addWidget(new QLabel(" " + tr("Database") + " ", hbox));

    Version = new QComboBox(hbox);
    Version->setEditable(true);
    Version->setDuplicatesEnabled(false);
    hlay->addWidget(Version);

    LastVersion = connection.provider() + ":Any";
    Version->addItem(LastVersion);

    QFrame *line = new QFrame(vbox);
    vlay->addWidget(line);

    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    vlay->addWidget(new QLabel(tr("Description"), vbox));

    splitter = new QSplitter(Qt::Vertical, vbox);
    vlay->addWidget(splitter);
    Description = new toMarkedText(splitter);
    Editor = new toWorksheet(splitter, connection, false);

    hlay->setSpacing(0);
    hlay->setContentsMargins(0, 0, 0, 0);
    hbox->setLayout(hlay);

    vlay->setSpacing(0);
    vlay->setContentsMargins(0, 0, 0, 0);
    vbox->setLayout(vlay);

    connectList(true);
    connect(Version,
            SIGNAL(activated(const QString &)),
            this,
            SLOT(changeVersion(const QString &)));
    connect(toMainWidget(),
            SIGNAL(sqlEditor(const QString &)),
            this,
            SLOT(editSQL(const QString &)));

    updateStatements();

    setFocusProxy(Statements);
}

void toSQLEdit::connectList(bool conn)
{
    if (conn)
        connect(Statements, SIGNAL(selectionChanged(void)), this, SLOT(selectionChanged(void)));
    else
        disconnect(Statements, SIGNAL(selectionChanged(void)), this, SLOT(selectionChanged(void)));
}

toSQLEdit::~toSQLEdit()
{
    SQLEditTool.closeWindow();
    toSQL::saveSQL(toConfigurationSingle::Instance().sqlFile());
}

void toSQLEdit::loadSQL(void)
{
    try
    {
        QString filename = toOpenFilename(QString::null, QString::null, this);
        if (!filename.isEmpty())
        {
            toSQL::loadSQL(filename);
            Filename = filename;
        }
    }
    TOCATCH;
}

void toSQLEdit::saveSQL(void)
{
    // must commit changes first for sql to be present in dictionary.
    commitChanges();

    QString filename = toSaveFilename(QString::null, QString::null, this);
    if (!filename.isEmpty())
    {
        Filename = filename;
        toSQL::saveSQL(filename);
    }
}

void toSQLEdit::deleteVersion(void)
{
    QString provider;
    QString version;
    if (!splitVersion(Version->currentText(), provider, version))
        return ;

    try
    {
        toSQL::deleteSQL(Name->text().toLatin1(), version, provider);
        Version->removeItem(Version->currentIndex());

        if (Version->count() == 0)
        {
            toTreeWidgetItem *item = toFindItem(Statements, Name->text());
            if (item)
            {
                connectList(false);
                delete item;
                connectList(true);
            }
            newSQL();
        }
        else
            selectionChanged(QString(connection().provider() + ":" + connection().version()));
    }
    TOCATCH;
}

bool toSQLEdit::close(bool del)
{
    if (checkStore(false))
        return toToolWidget::close();
    return false;
}

bool toSQLEdit::splitVersion(const QString &split, QString &provider, QString &version)
{
    int found = split.indexOf(QString::fromLatin1(":"));
    if (found < 0)
    {
        TOMessageBox::warning(this, tr("Wrong format of version"),
                              tr("Should be database provider:version."),
                              tr("&Ok"));
        return false;
    }
    provider = split.mid(0, found).toLatin1();
    if (provider.length() == 0)
    {
        TOMessageBox::warning(this, tr("Wrong format of version"),
                              tr("Should be database provider:version. Can't start with :."),
                              tr("&Ok"));
        return false;
    }
    version = split.mid(found + 1).toLatin1();
    if (version.length() == 0)
    {
        TOMessageBox::warning(this, tr("Wrong format of version"),
                              tr("Should be database provider:version. Can't end with the first :."),
                              tr("&Ok"));
        return false;
    }
    return true;
}

void toSQLEdit::commitChanges(bool changeSelected)
{
    QString provider;
    QString version;
    if (!splitVersion(Version->currentText(), provider, version))
        return ;
    QString name = Name->text();
    toTreeWidgetItem *item = toFindItem(Statements, name);
    if (!item)
    {
        int i = name.indexOf(QString::fromLatin1(":"));
        if (i >= 0)
        {
            item = toFindItem(Statements, name.mid(0, i));
            if (!item)
                item = new toTreeWidgetItem(Statements, name.mid(0, i));
            item = new toTreeWidgetItem(item, name.mid(i + 1));
        }
        else
            item = new toTreeWidgetItem(Statements, name);
    }
    connectList(false);
    if (changeSelected)
    {
        Statements->setSelected(item, true);
        Statements->setCurrentItem(item);
        if (item->parent() && !item->parent()->isOpen())
            item->parent()->setOpen(true);
    }
    connectList(true);
    if (Description->text().isEmpty())
    {
        TOMessageBox::warning(this, tr("Missing description"),
                              tr("No description filled in. This is necessary to save SQL."
                                 " Adding undescribed as description."),
                              tr("&Ok"));
        Description->setText(tr("Undescribed"));
    }
    toSQL::updateSQL(name.toLatin1(),
                     Editor->editor()->text(),
                     Description->text(),
                     version,
                     provider);
    TrashButton->setEnabled(true);
    CommitButton->setEnabled(true);

    bool update = Name->isModified();


    Name->setModified(false);
    Description->setModified(false);
    Editor->editor()->setModified(false);
    LastVersion = Version->currentText();
    if (update)
        updateStatements(Name->text());
    Statements->setFocus();
}

bool toSQLEdit::checkStore(bool justVer)
{
    if ((Name->isModified() ||
            Editor->editor()->isModified() ||
            (!justVer && Version->currentText() != LastVersion) ||
            Description->isModified()) &&
            Version->currentText().length() > 0)
    {
        // grab focus so user can see file and decide to save
        setFocus(Qt::OtherFocusReason);

        switch (TOMessageBox::information(this, tr("Modified SQL dictionary"),
                                          tr("Save changes into the SQL dictionary"),
                                          tr("&Yes"), tr("&No"), tr("Cancel"), 0, 2))
        {
        case 0:
            commitChanges(false);
            break;
        case 1:
            Name->setModified(false);
            Description->setModified(false);
            Editor->editor()->setModified(false);
            LastVersion = Version->currentText();
            return true;
        case 2:
            return false;
        }
    }
    return true;
}

void toSQLEdit::changeVersion(const QString &ver)
{
    if (!Editor->editor()->isModified() || checkStore(true))
    {
        selectionChanged(ver);
        if (Version->currentText() != ver)
        {
            Version->addItem(ver);
            Version->lineEdit()->setText(ver);
        }
    }
}

void toSQLEdit::selectionChanged(void)
{
    try
    {
        if (checkStore(false))
            selectionChanged(QString(connection().provider() + ":" + connection().version()));
    }
    TOCATCH;
}

void toSQLEdit::changeSQL(const QString &name, const QString &maxver)
{
    toSQL::sqlMap sql = toSQL::definitions();
    Name->setText(name);
    Name->setModified(false);

    toTreeWidgetItem *item = toFindItem(Statements, name);
    if (item)
    {
        connectList(false);
        Statements->setSelected(item, true);
        Statements->setCurrentItem(item);
        if (item->parent() && !item->parent()->isOpen())
            item->parent()->setOpen(true);
        connectList(true);
    }

    Version->clear();
    LastVersion = "";
    if (sql.find(name.toLatin1()) != sql.end())
    {
        toSQL::definition &def = sql[name.toLatin1()];
        std::list<toSQL::version> &ver = def.Versions;

        Description->setText(def.Description);
        Description->setModified(false);

        std::list<toSQL::version>::iterator j = ver.end();
        int ind = 0;
        for (std::list<toSQL::version>::iterator i = ver.begin();i != ver.end();i++)
        {
            QString str = (*i).Provider;
            str += QString::fromLatin1(":");
            str += (*i).Version;
            Version->addItem(str);
            if (str <= maxver || j == ver.end())
            {
                j = i;
                LastVersion = str;
                ind = Version->count() - 1;
            }
        }
        if (j != ver.end())
        {
            Editor->editor()->setText((*j).SQL);
            TrashButton->setEnabled(true);
            CommitButton->setEnabled(true);
            Version->setCurrentIndex(ind);
        }
    }
    else
    {
        Description->clear();
        Editor->editor()->clear();
        TrashButton->setEnabled(false);
        CommitButton->setEnabled(true);
    }
    if (LastVersion.isEmpty())
    {
        LastVersion = QString::fromLatin1("Any:Any");
        Version->addItem(LastVersion);
    }
    Editor->editor()->setModified(false);
}

void toSQLEdit::selectionChanged(const QString &maxver)
{
    try
    {
        toTreeWidgetItem *item = Statements->selectedItem();
        if (item)
        {
            QString name = item->text(0);
            while (item->parent())
            {
                item = item->parent();
                name.prepend(QString::fromLatin1(":"));
                name.prepend(item->text(0));
            }
            changeSQL(name, maxver);
        }
    }
    TOCATCH;
}

void toSQLEdit::editSQL(const QString &nam)
{
    try
    {
        if (checkStore(false))
            changeSQL(nam, QString(connection().provider() + ":" + connection().version()));
    }
    TOCATCH;
}

void toSQLEdit::newSQL(void)
{
    if (checkStore(false))
    {
        QString name = Name->text();
        int found = name.indexOf(QString::fromLatin1(":"));
        if (found < 0)
            name = QString::null;
        else
            name = name.mid(0, found + 1);
        try
        {
            changeSQL(name, QString(connection().provider() + ":Any"));
        }
        TOCATCH;
    }
}

static toSQLTemplate SQLTemplate;

toSQLTemplateItem::toSQLTemplateItem(toTreeWidget *parent)
        : toTemplateItem(SQLTemplate, parent, qApp->translate("toSQL", "SQL Dictionary"))
{
    setExpandable(true);
}

static QString JustLast(const QString &str)
{
    int pos = str.lastIndexOf(":");
    if (pos >= 0)
        return QString(str.mid(pos + 1));
    return QString(str);
}

toSQLTemplateItem::toSQLTemplateItem(toSQLTemplateItem *parent,
                                     const QString &name)
        : toTemplateItem(parent, JustLast(name))
{
    Name = name;
    std::list<QString> def = toSQL::range(Name + ":");
    if (def.begin() != def.end())
        setExpandable(true);
}

void toSQLTemplateItem::expand(void)
{
    std::list<QString> def;
    if (Name.isEmpty())
        def = toSQL::range(Name);
    else
        def = toSQL::range(Name + ":");
    QString last;
    for (std::list<QString>::iterator sql = def.begin();sql != def.end();sql++)
    {
        QString name = *sql;
        if (!Name.isEmpty())
            name = name.mid(Name.length() + 1);
        if (name.indexOf(":") != -1)
            name = name.mid(0, name.indexOf(":"));
        if (name != last)
        {
            if (Name.isEmpty())
                new toSQLTemplateItem(this, name);
            else
                new toSQLTemplateItem(this, Name + ":" + name);
            last = name;
        }
    }
}

QString toSQLTemplateItem::allText(int) const
{
    try
    {
        toSQL::sqlMap defs = toSQL::definitions();
        if (defs.find(Name) == defs.end())
            return QString::null;
        return toSQL::string(Name, toMainWidget()->currentConnection()) + ";";
    }
    catch (...)
    {
        return QString::null;
    }
}

void toSQLTemplateItem::collapse(void)
{
    while (firstChild())
        delete firstChild();
}

QWidget *toSQLTemplateItem::selectedWidget(QWidget *parent)
{
    toHighlightedText *widget = new toHighlightedText(parent);
    widget->setReadOnly(true);
    widget->setText(allText(0));
    return widget;
}
