/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconf.h"
#include "toextract.h"
#include "tofilesize.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "toreport.h"
#include "toresultview.h"
#include "toscript.h"
#include "tosql.h"
#include "totextview.h"
#include "totool.h"
#include "toworksheet.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include <QGridLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QFileDialog>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QButtonGroup>

#include "icons/execute.xpm"
#include "icons/toscript.xpm"

static toSQL SQLObjectListMySQL("toScript:ExtractObject",
                                "TOAD 1,0,0 SHOW DATABASES",
                                "Extract objects available to extract from the database, "
                                "should have same columns",
                                "3.23",
                                "MySQL");

static toSQL SQLObjectList("toScript:ExtractObject",
                           "SELECT *\n"
                           "  FROM (SELECT 'TABLESPACE',tablespace_name,NULL\n"
                           "   FROM sys.dba_tablespaces\n"
                           " UNION\n"
                           " SELECT 'ROLE',role,NULL\n"
                           "   FROM sys.dba_roles\n"
                           " UNION\n"
                           " SELECT 'PUBLIC',NULL,NULL\n"
                           "   FROM dual\n"
                           " UNION\n"
                           " SELECT username,NULL,NULL\n"
                           "   FROM sys.all_users)\n"
                           "  ORDER BY 1,2,3",
                           "",
                           "0801");

static toSQL SQLUserObjectList("toScript:UserExtractObject",
                               "SELECT owner,object_type,object_name\n"
                               "  FROM sys.all_objects\n"
                               " WHERE object_type IN ('VIEW','TABLE','TYPE','SEQUENCE','PACKAGE',\n"
                               "                'PACKAGE BODY','FUNCTION','PROCEDURE')\n"
                               " ORDER BY 1,2,3",
                               "Extract objects available to extract from the database if you "
                               "don't have admin access, should have same columns");

static toSQL SQLPublicSynonymList("toScript:PublicSynonyms",
                                  "SELECT synonym_name\n"
                                  "  FROM sys.all_synonyms WHERE owner = 'PUBLIC'\n"
                                  " ORDER BY 1",
                                  "Extract all public synonyms from database");

static toSQL SQLUserObjectsMySQL("toScript:UserObjects",
                                 "SHOW TABLES FROM :own<noquote>",
                                 "Get the objects available for a user, must have same columns and binds",
                                 "3.23",
                                 "MySQL");

static toSQL SQLUserObjects("toScript:UserObjects",
                            "SELECT *\n"
                            "  FROM (SELECT 'DATABASE LINK',db_link\n"
                            "          FROM sys.all_db_links\n"
                            "         WHERE owner = :own<char[101]>\n"
                            "        UNION\n"
                            "        SELECT object_type,object_name\n"
                            "          FROM sys.all_objects\n"
                            "         WHERE object_type IN ('VIEW','TYPE','SEQUENCE','PACKAGE',\n"
                            "                               'PACKAGE BODY','FUNCTION','PROCEDURE','TRIGGER')\n"
                            "           AND owner = :own<char[101]>\n"
                            "         UNION\n"
                            "        SELECT 'TABLE',table_name\n"
                            "          FROM sys.all_tables\n"
                            "         WHERE temporary != 'Y' AND secondary = 'N' AND iot_name IS NULL\n"
                            "           AND owner = :own<char[101]>\n"
                            "        UNION\n"
                            "        SELECT 'MATERIALIZED TABLE',mview_name AS object\n"
                            "          FROM sys.all_mviews\n"
                            "         WHERE owner = :own<char[101]>)\n"
                            " ORDER BY 1,2",
                            "");

static toSQL SQLUserObjects7("toScript:UserObjects",
                             "SELECT *\n"
                             "  FROM (SELECT 'DATABASE LINK',db_link\n"
                             "          FROM sys.all_db_links\n"
                             "         WHERE owner = :own<char[101]>\n"
                             "        UNION\n"
                             "        SELECT object_type,object_name\n"
                             "          FROM sys.all_objects\n"
                             "         WHERE object_type IN ('VIEW','TYPE','SEQUENCE','PACKAGE',\n"
                             "                               'PACKAGE BODY','FUNCTION','PROCEDURE','TRIGGER')\n"
                             "           AND owner = :own<char[101]>\n"
                             "         UNION\n"
                             "        SELECT 'TABLE',table_name\n"
                             "          FROM sys.all_tables\n"
                             "         WHERE temporary != 'Y' AND secondary = 'N'\n"
                             "           AND owner = :own<char[101]>\n"
                             "        UNION\n"
                             "        SELECT 'MATERIALIZED TABLE',mview_name AS object\n"
                             "          FROM sys.all_mviews\n"
                             "         WHERE owner = :own<char[101]>)\n"
                             " ORDER BY 1,2",
                             "",
                             "0703");

static toSQL SQLSchemasMySQL("toScript:ExtractSchema",
                             "SHOW DATABASES",
                             "Get usernames available in database, must have same columns",
                             "3.23",
                             "MySQL");

static toSQL SQLSchemas("toScript:ExtractSchema",
                        "SELECT username FROM sys.all_users ORDER BY username",
                        "");

class toScriptTool : public toTool
{
protected:
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(toscript_xpm);
    }
public:
    toScriptTool()
            : toTool(310, "DB Extraction/Compare/Search")
    { }
    virtual const char *menuItem()
    {
        return "DB Extraction/Compare/Search";
    }
    virtual const char *toolbarTip()
    {
        return "DB or schema extraction, compare and search";
    }
    virtual bool canHandle(toConnection &conn)
    {
        try
        {
            return toExtract::canHandle(conn) && !toSQL::string(SQLObjectList, conn).isEmpty();
        }
        catch (...)
        {
            return false;
        }
    }
    virtual QWidget *toolWindow(QWidget *main, toConnection &connection)
    {
        return new toScript(main, connection);
    }
    virtual void closeWindow(toConnection &connection){};
};

static toScriptTool ScriptTool;


toScript::toScript(QWidget *parent, toConnection &connection)
        : toToolWidget(ScriptTool, "script.html", parent, connection)
{
    QToolBar *toolbar = toAllocBar(this, tr("Extraction and Compare"));
    layout()->addWidget(toolbar);

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(execute_xpm))),
                       tr("Perform defined extraction"),
                       this,
                       SLOT(execute(void)));

    toolbar->addWidget(new toSpacer());

    ScriptUI = new Ui::toScriptUI();
    ScriptUI->setupUi(this);
    layout()->addWidget(ScriptUI->Tabs);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setSpacing(0);
    hbox->setContentsMargins(0, 0, 0, 0);

    QScrollArea *sa = new QScrollArea(ScriptUI->tab);
    sa->setWidget(ScriptUI->confContainer);
    sa->setWidgetResizable(true);
    sa->setSizePolicy(QSizePolicy::MinimumExpanding,
                      QSizePolicy::MinimumExpanding);
    hbox->addWidget(sa);
    ScriptUI->tab->setLayout(hbox);

    QSplitter *hsplitter = new QSplitter(Qt::Horizontal, ScriptUI->DifferenceTab);
    QWidget *box = new QWidget(ScriptUI->ResultTab);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    box->setLayout(vbox);
    Worksheet = new toWorksheet(box, connection);
    vbox->addWidget(Worksheet);
    SearchList = new toListView(box);
    vbox->addWidget(SearchList);
    SearchList->addColumn(tr("Search result"));
    SearchList->setRootIsDecorated(true);
    SearchList->setSorting(0);
    SearchList->hide();
    Report = new toTextView(box);
    vbox->addWidget(Report);
    Report->hide();

    DropList = new toListView(hsplitter);
    DropList->addColumn(tr("Dropped"));
    DropList->setRootIsDecorated(true);
    DropList->setSorting(0);
    CreateList = new toListView(hsplitter);
    CreateList->addColumn(tr("Created"));
    CreateList->setRootIsDecorated(true);
    CreateList->setSorting(0);
    ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->ResultTab), false);
    ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->DifferenceTab), false);

    connect(SearchList, SIGNAL(clicked(toTreeWidgetItem *)), this, SLOT(keepOn(toTreeWidgetItem *)));
    connect(DropList, SIGNAL(clicked(toTreeWidgetItem *)), this, SLOT(keepOn(toTreeWidgetItem *)));
    connect(CreateList, SIGNAL(clicked(toTreeWidgetItem *)), this, SLOT(keepOn(toTreeWidgetItem *)));

    QGridLayout *layout = new QGridLayout(ScriptUI->ResultTab);
    layout->addWidget(box, 0, 0);
    layout = new QGridLayout(ScriptUI->DifferenceTab);
    layout->addWidget(hsplitter, 0, 0);

    QButtonGroup *group = new QButtonGroup(ScriptUI->ModeGroup);
    group->addButton(ScriptUI->Compare, 1);
    group->addButton(ScriptUI->Extract, 2);
    group->addButton(ScriptUI->Search, 3);
    group->addButton(ScriptUI->Migrate, 4);
    group->addButton(ScriptUI->Report, 5);

    ScriptUI->Initial->setTitle(tr("&Initial"));
    ScriptUI->Limit->setTitle(tr("&Limit"));
    ScriptUI->Next->setTitle(tr("&Next"));
    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(changeMode(int)));
    ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->ResizeTab), false);
    ScriptUI->SourceObjects->setSorting(0);
    ScriptUI->SourceObjects->setResizeMode(toTreeWidget::AllColumns);
    ScriptUI->DestinationObjects->setSorting(0);
    ScriptUI->DestinationObjects->setResizeMode(toTreeWidget::AllColumns);

    // Remove when migrate and resize is implemented
#if 1

    ScriptUI->Migrate->hide();
#endif

    int def = 0;
    std::list<QString> cons = toMainWidget()->connections();
    int i = 0;
    while (cons.size() > 0)
    {
        QString str = toShift(cons);
        if (str == connection.description() && def == 0)
            def = i;
        i++;
        ScriptUI->SourceConnection->addItem(str);
        ScriptUI->DestinationConnection->addItem(str);
    }
    ScriptUI->SourceConnection->setCurrentIndex(def);
    changeSource(def);
    changeDestination(def);
    ScriptUI->DestinationConnection->setCurrentIndex(def);

    connect(ScriptUI->AddButton, SIGNAL(clicked()), this, SLOT(newSize()));
    connect(ScriptUI->Remove, SIGNAL(clicked()), this, SLOT(removeSize()));

    connect(ScriptUI->SourceConnection,
            SIGNAL(activated(int)),
            this,
            SLOT(changeSource(int)));
    connect(ScriptUI->DestinationConnection,
            SIGNAL(activated(int)),
            this,
            SLOT(changeDestination(int)));
    connect(ScriptUI->SourceSchema,
            SIGNAL(activated(int)),
            this,
            SLOT(changeSourceSchema(int)));
    connect(ScriptUI->DestinationSchema,
            SIGNAL(activated(int)),
            this,
            SLOT(changeDestinationSchema(int)));
    connect(ScriptUI->SourceObjects,
            SIGNAL(clicked(toTreeWidgetItem *)),
            this,
            SLOT(objectClicked(toTreeWidgetItem *)));
    connect(ScriptUI->DestinationObjects,
            SIGNAL(clicked(toTreeWidgetItem *)),
            this,
            SLOT(objectClicked(toTreeWidgetItem *)));
    connect(ScriptUI->Browse, SIGNAL(clicked()), this, SLOT(browseFile()));

    connect(ScriptUI->SourceObjects, SIGNAL(expanded(toTreeWidgetItem *)),
            this, SLOT(expandSource(toTreeWidgetItem *)));
    connect(ScriptUI->DestinationObjects, SIGNAL(expanded(toTreeWidgetItem *)),
            this, SLOT(expandDestination(toTreeWidgetItem *)));

    connect(toMainWidget(), SIGNAL(addedConnection(const QString &)),
            this, SLOT(addConnection(const QString &)));
    connect(toMainWidget(), SIGNAL(removedConnection(const QString &)),
            this, SLOT(delConnection(const QString &)));

    ScriptUI->Schema->setCurrentIndex(0);
    setFocusProxy(ScriptUI->Tabs);
}

void toScript::delConnection(const QString &name)
{
    for (int i = 0;i < ScriptUI->SourceConnection->count();i++)
    {
        if (ScriptUI->SourceConnection->itemText(i) == name)
        {
            ScriptUI->SourceConnection->removeItem(i);
            break;
        }
    }
    for (int j = 0;j < ScriptUI->DestinationConnection->count();j++)
    {
        if (ScriptUI->DestinationConnection->itemText(j) == name)
        {
            ScriptUI->DestinationConnection->removeItem(j);
            break;
        }
    }
}

void toScript::addConnection(const QString &name)
{
    ScriptUI->SourceConnection->addItem(name);
    ScriptUI->DestinationConnection->addItem(name);
}

toScript::~toScript()
{}

void toScript::closeEvent(QCloseEvent *event) {
    if(Worksheet && Worksheet->close())
        event->accept();
    else
        event->ignore();
}

std::list<QString> toScript::createObjectList(toTreeWidget *source)
{
    std::list<QString> lst;

    std::list<QString> otherGlobal;
    std::list<QString> profiles;
    std::list<QString> roles;
    std::list<QString> tableSpace;
    std::list<QString> tables;
    std::list<QString> userOther;
    std::list<QString> userViews;
    std::list<QString> users;

    for(toTreeWidgetItemIterator it(source); (*it); it++) {
        toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>((*it));

        if (chk && chk->isEnabled())
        {
            QString name = chk->text(0);
            QString type = chk->text(1);
            QString user = chk->text(2);
            if (!user.isEmpty())
            {
                if (chk->isOn() && chk->isEnabled())
                {
                    QString line;
                    if (type == QString::fromLatin1("TABLE"))
                    {
                        line = user;
                        line += QString::fromLatin1(".");
                        line += name;
                        toPush(tables, line);
                    }
                    else
                    {
                        line = type;
                        line += QString::fromLatin1(":");
                        line += user;
                        line += QString::fromLatin1(".");
                        line += name;
                        if (type == QString::fromLatin1("VIEW"))
                            toPush(userViews, line);
                        else
                            toPush(userOther, line);
                    }
                }
            }
            else if (!type.isEmpty())
            {
                if (chk->isOn() && chk->isEnabled())
                {
                    QString line = type;
                    line += QString::fromLatin1(":");
                    line += name;
                    if (type == QString::fromLatin1("TABLESPACE"))
                        toPush(tableSpace, line);
                    else if (type == QString::fromLatin1("PROFILE"))
                        toPush(profiles, line);
                    else if (type == QString::fromLatin1("ROLE"))
                        toPush(roles, name);
                    else if (type == QString::fromLatin1("USER"))
                        toPush(users, name);
                    else
                        toPush(otherGlobal, line);
                }
            }
        }
    }

    if (ScriptUI->IncludeDDL->isChecked())
    {
        lst.insert(lst.end(), tableSpace.begin(), tableSpace.end());
        lst.insert(lst.end(), profiles.begin(), profiles.end());
        lst.insert(lst.end(), otherGlobal.begin(), otherGlobal.end());
        for_each(roles.begin(), roles.end(), PrefixString(lst, QString::fromLatin1("ROLE:")));
        for_each(users.begin(), users.end(), PrefixString(lst, QString::fromLatin1("USER:")));
        for_each(tables.begin(), tables.end(), PrefixString(lst, QString::fromLatin1("TABLE FAMILY:")));
        lst.insert(lst.end(), userViews.begin(), userViews.end());
        lst.insert(lst.end(), userOther.begin(), userOther.end());
    }
    for_each(tables.begin(), tables.end(), PrefixString(lst, QString::fromLatin1("TABLE CONTENTS:")));
    if (ScriptUI->IncludeDDL->isChecked())
    {
        for_each(tables.begin(), tables.end(), PrefixString(lst, QString::fromLatin1("TABLE REFERENCES:")));
        for_each(roles.begin(), roles.end(), PrefixString(lst, QString::fromLatin1("ROLE GRANTS:")));
        for_each(users.begin(), users.end(), PrefixString(lst, QString::fromLatin1("USER GRANTS:")));
    }
    return lst;
}

void toScript::execute(void)
{
    try
    {
        int mode;
        if (ScriptUI->Compare->isChecked())
            mode = 0;
        else if (ScriptUI->Extract->isChecked())
            mode = 1;
        else if (ScriptUI->Migrate->isChecked())
            mode = 2;
        else if (ScriptUI->Search->isChecked())
            mode = 3;
        else if (ScriptUI->Report->isChecked())
            mode = 4;
        else
        {
            toStatusMessage(tr("No mode selected"));
            return ;
        }

        std::list<QString> sourceObjects = createObjectList(ScriptUI->SourceObjects);

        std::list<QString> sourceDescription;
        std::list<QString> destinationDescription;
        QString script;

        toExtract source(toMainWidget()->connection(ScriptUI->SourceConnection->currentText()), this);
        setupExtract(source);
        switch (mode)
        {
        case 1:
            if (ScriptUI->OutputTab->isChecked())
                script += source.create(sourceObjects);
            else if (ScriptUI->OutputFile->isChecked())
            {
                if (ScriptUI->Filename->text().isEmpty())
                    throw tr("No filename specified");

                QFile file(ScriptUI->Filename->text());
                if (file.exists())
                {
                    if (TOMessageBox::warning(
                            this,
                            tr("Overwrite file?"),
                            tr("The file %1 already exists,\n"
                               "are you sure you want to continue and write over it?")
                            .arg(ScriptUI->Filename->text()),
                            tr("&Yes"), tr("&Cancel"), QString::null, 0) != 0)
                    {
                        return ;
                    }
                }
                file.open(QIODevice::WriteOnly);

                if(file.error() != QFile::NoError)
                    throw tr("Couldn't open file %1").arg(file.fileName());

                QTextStream stream(&file);
                source.create(stream, sourceObjects);

                if(file.error() != QFile::NoError)
                    throw tr("Error writing to file %1").arg(file.fileName());

                script = tr("-- Script generated to file %1 successfully").arg(ScriptUI->Filename->text());
            }
            else if (ScriptUI->OutputDir->isChecked())
            {
                if (ScriptUI->Filename->text().isEmpty())
                    throw tr("No filename specified");

                QFile file(ScriptUI->Filename->text() + QDir::separator() + "script.sql");
                file.open(QIODevice::WriteOnly);

                if(file.error() != QFile::NoError)
                    throw QString(tr("Couldn't open file %1")).arg(file.fileName());

                QTextStream stream(&file);

                stream << tr("rem Master script for DDL reverse engineering by TOra\n"
                             "\n");

                QFile pfile(ScriptUI->Filename->text() + QDir::separator() + "script.tpr");
                pfile.open(QIODevice::WriteOnly);

                if(pfile.error() != QFile::NoError)
                    throw QString(tr("Couldn't open file %1")).arg(pfile.fileName());

                QTextStream pstream(&pfile);

                QRegExp repl("\\W+");
                for (std::list<QString>::iterator i = sourceObjects.begin();i != sourceObjects.end();i++)
                {
                    std::list<QString> t;
                    t.insert(t.end(), *i);
                    QString fn = *i;
                    fn.replace(repl, "_");
                    fn += ".sql";
                    stream << "@" << fn << "\n";

                    QFile tf(ScriptUI->Filename->text() + QDir::separator() + fn);
                    tf.open(QIODevice::WriteOnly);
                    pstream << tf.fileName() << "\n";

                    if(tf.error() != QFile::NoError)
                        throw QString(tr("Couldn't open file %1")).arg(tf.fileName());

                    QTextStream ts(&tf);
                    source.create(ts, t);

                    if(tf.error() != QFile::NoError)
                        throw QString(tr("Error writing to file %1")).arg(tf.fileName());


                    script = tr("-- Scripts generate to directory %1 successfully").arg(ScriptUI->Filename->text());
                    ;
                }

                if(file.error() != QFile::NoError)
                    throw QString(tr("Error writing to file %1")).arg(file.fileName());
                if(pfile.error() != QFile::NoError)
                    throw QString(tr("Error writing to file %1")).arg(pfile.fileName());
            }
            break;
        case 0:
        case 2:
        case 3:
        case 4:
            sourceDescription = source.describe(sourceObjects);
            break;
        }

        if (ScriptUI->Destination->isEnabled())
        {
            std::list<QString> destinationObjects = createObjectList(ScriptUI->DestinationObjects);
            toExtract destination(toMainWidget()->connection(ScriptUI->
                                  DestinationConnection->
                                  currentText()), this);
            setupExtract(destination);
            switch (mode)
            {
            case 0:
            case 2:
                destinationDescription = destination.describe(destinationObjects);
                break;
            case 1:
            case 3:
                throw tr("Destination shouldn't be enabled now, internal error");
            }

            std::list<QString> drop;
            std::list<QString> create;

            toExtract::srcDst2DropCreate(sourceDescription, destinationDescription,
                                         drop, create);
            sourceDescription = drop;
            destinationDescription = create;
        }
        ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->ResultTab), mode == 1 || mode == 2 || mode == 3 || mode == 4);
        ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->DifferenceTab), mode == 0 || mode == 2);
        if (!script.isEmpty())
        {
            Worksheet->editor()->setText(script);
            Worksheet->editor()->setFilename(QString::null);
            Worksheet->editor()->setModified(true);
        }
        if (mode == 3)
        {
            Worksheet->hide();
            Report->hide();
            SearchList->show();
            QRegExp re(ScriptUI->SearchWord->text(), Qt::CaseInsensitive);
            QStringList words(ScriptUI->SearchWord->text().toUpper().simplified().split(QRegExp(QString::fromLatin1(" "))));
            QString word = ScriptUI->SearchWord->text().toUpper();
            int searchMode = 0;
            if (ScriptUI->AllWords->isChecked())
                searchMode = 1;
            else if (ScriptUI->AnyWords->isChecked())
                searchMode = 2;
            else if (ScriptUI->RegExp->isChecked())
                searchMode = 3;
            else if (ScriptUI->ExactMatch->isChecked())
                searchMode = 4;
            std::list<QString> result;
            for (std::list<QString>::iterator i = sourceDescription.begin();
                    i != sourceDescription.end();
                    i++)
            {
                QStringList ctx = (*i).toUpper().split(QString("\01"));
                switch (searchMode)
                {
                case 1:
                case 2:
                {
                    int count = 0;
                    for (int k = 0;k < words.count();k++)
                    {
                        QString s = words[k];
                        if (ctx.last().contains(s))
                            count++;
                    }
                    if ((searchMode == 2 && count > 0) || (searchMode == 1 && count == words.count()))
                        result.insert(result.end(), *i);
                }
                break;
                case 4:
                    if (ctx.last() == word)
                        result.insert(result.end(), *i);
                    break;
                case 3:
                    if (re.indexIn(ctx.last()) >= 0)
                        result.insert(result.end(), *i);
                    break;
                }
            }
            fillDifference(result, SearchList);
        }
        else if (mode == 4)
        {
            Worksheet->hide();
            SearchList->hide();
            Report->show();
            QString res = toGenerateReport(source.connection(), sourceDescription);
            Report->setText(res);
            if (ScriptUI->OutputFile->isChecked())
            {
                if (ScriptUI->Filename->text().isEmpty())
                    toStatusMessage(tr("No filename specified"));
                else
                {
                    QFile file(ScriptUI->Filename->text());
                    file.open(QIODevice::WriteOnly);

                    if (file.error() != QFile::NoError)
                        toStatusMessage(tr("Couldn't open file %1").arg(file.fileName()));
                    else
                    {
                        QTextStream stream(&file);
                        stream << res;

                        if (file.error() != QFile::NoError)
                            toStatusMessage(tr("Error writing to file %1").arg(file.fileName()));
                    }
                }
            }
        }
        else
        {
            Worksheet->show();
            SearchList->hide();
            Report->hide();
            fillDifference(sourceDescription, DropList);
            fillDifference(destinationDescription, CreateList);
        }
        if (mode == 0)
            ScriptUI->Tabs->setCurrentIndex(ScriptUI->Tabs->indexOf(ScriptUI->DifferenceTab));
        else
            ScriptUI->Tabs->setCurrentIndex(ScriptUI->Tabs->indexOf(ScriptUI->ResultTab));
    }
    TOCATCH
}

void toScript::fillDifference(std::list<QString> &objects, toTreeWidget *view)
{
    view->clear();
    toTreeWidgetItem *last = NULL;
    int lastLevel = 0;
    QStringList lstCtx;
    for (std::list<QString>::iterator i = objects.begin();i != objects.end();i++)
    {
        //    printf("Adding %s\n",(const char *)*i);
        QStringList ctx = (*i).split(QString("\01"));
        if (last)
        {
            while (last && lastLevel >= int(ctx.count()))
            {
                last = last->parent();
                lastLevel--;
            }
            while (last && lastLevel >= 0 && !toCompareLists(lstCtx, ctx, (unsigned int)lastLevel))
            {
                last = last->parent();
                lastLevel--;
            }
        }
        if (lastLevel < 0)
            throw tr("Internal error, lastLevel < 0");
        while (lastLevel < int(ctx.count()) - 1)
        {
            if (last)
                last = new toResultViewMLine(last, NULL, ctx[lastLevel]);
            else
                last = new toResultViewMLine(view, NULL, ctx[lastLevel]);
            lastLevel++;
        }
        toTreeWidgetCheck *item;
        if (last)
            item = new toResultViewMLCheck(last, ctx[lastLevel], toTreeWidgetCheck::CheckBox);
        else
            item = new toResultViewMLCheck(view, ctx[lastLevel], toTreeWidgetCheck::CheckBox);
        last = item;
        item->setOn(true);
        lstCtx = ctx;
        lastLevel++;
    }
}

void toScript::changeConnection(int, bool source)
{
    try
    {
        toTreeWidget *sourceL = NULL;
        toTreeWidget *destinationL = NULL;
        if (ScriptUI->SourceConnection->currentText() ==
                ScriptUI->DestinationConnection->currentText())
        {
            if (source)
            {
                destinationL = ScriptUI->SourceObjects;
                sourceL = ScriptUI->DestinationObjects;
            }
            else
            {
                sourceL = ScriptUI->SourceObjects;
                destinationL = ScriptUI->DestinationObjects;
            }
        }
        if (sourceL && destinationL && sourceL->firstChild())
        {
            destinationL->clear();
            toTreeWidgetItem *next = NULL;
            toTreeWidgetItem *parent = NULL;
            for (toTreeWidgetItem *item = sourceL->firstChild();item;item = next)
            {
                toTreeWidgetItem * lastParent = parent;
                if (!parent)
                {
                    parent = new toResultViewCheck(destinationL, item->text(0),
                                                   toTreeWidgetCheck::CheckBox);
                    parent->setExpandable(true);
                }
                else
                    parent = new toResultViewCheck(parent, item->text(0),
                                                   toTreeWidgetCheck::CheckBox);
                parent->setText(1, item->text(1));
                parent->setText(2, item->text(2));

                if (item->firstChild())
                    next = item->firstChild();
                else if (item->nextSibling())
                {
                    next = item->nextSibling();
                    parent = lastParent;
                }
                else
                {
                    next = item;
                    parent = lastParent;
                    do
                    {
                        next = next->parent();
                        if (parent)
                            parent = parent->parent();
                    }
                    while (next && !next->nextSibling());
                    if (next)
                        next = next->nextSibling();
                }
            }

            return;
        }
        (source ? ScriptUI->SourceObjects : ScriptUI->DestinationObjects)->clear();
        (source ? ScriptUI->SourceSchema : ScriptUI->DestinationSchema)->clear();
        (source ? ScriptUI->SourceSchema : ScriptUI->DestinationSchema)->addItem(tr("All"));
        toConnection &conn = toMainWidget()->connection((source ?
                             ScriptUI->SourceConnection :
                             ScriptUI->DestinationConnection)
                             ->currentText());
        toQList object;
        try
        {
            object = toQuery::readQueryNull(conn, SQLObjectList);
        }
        catch (...)
        {
            object = toQuery::readQueryNull(conn, SQLUserObjectList);
        }
        toQList schema = toQuery::readQuery(conn, SQLSchemas);
        while (schema.size() > 0)
        {
            QString str = toShift(schema);
            (source ? ScriptUI->SourceSchema : ScriptUI->DestinationSchema)->addItem(str);
        }
        toTreeWidgetItem *lastTop = NULL;
        toTreeWidgetItem *lastFirst = NULL;
        while (object.size() > 0)
        {
            QString top = toShift(object);
            QString first = toShift(object);
            QString second = toShift(object);

            if (top != (lastTop ? lastTop->text(0) : QString::null))
            {
                lastFirst = NULL;
                lastTop = new toResultViewCheck((source ?
                                                 ScriptUI->SourceObjects :
                                                 ScriptUI->DestinationObjects),
                                                top, toTreeWidgetCheck::CheckBox);
                lastTop->setExpandable(true);
                if (!second.isEmpty() || first.isEmpty())
                    lastTop->setText(1, QString::fromLatin1("DATABASE"));
            }
            if (first != (lastFirst ? lastFirst->text(0) : QString::null) && !first.isEmpty())
            {
                lastFirst = new toResultViewCheck(lastTop, first, toTreeWidgetCheck::CheckBox);
                if (second.isEmpty())
                    lastFirst->setText(1, top);
            }
            if (!second.isEmpty() && lastFirst)
            {
                toTreeWidgetItem *item = new toResultViewCheck(lastFirst, second, toTreeWidgetCheck::CheckBox);
                item->setText(1, first);
                item->setText(2, top);
            }
        }
    }
    TOCATCH
}

void toScript::readOwnerObjects(toTreeWidgetItem *item, toConnection &conn)
{
    if (!item->parent() && !item->firstChild())
    {
        try
        {
            toTreeWidgetItem *lastFirst = NULL;
            QString top = item->text(0);
            toQuery object(conn, SQLUserObjects, top);

            while (!object.eof())
            {
                QString first = object.readValueNull();
                QString second;
                if (object.columns() > 1)
                    second = object.readValueNull();
                else
                {
                    second = first;
                    first = "TABLE";
                }

                if (first != (lastFirst ? lastFirst->text(0) : QString::null) && !first.isEmpty())
                {
                    lastFirst = new toResultViewCheck(item, first, toTreeWidgetCheck::CheckBox);
                    if (second.isEmpty())
                        lastFirst->setText(1, top);
                }
                if (!second.isEmpty() && lastFirst)
                {
                    toTreeWidgetItem *item = new toResultViewCheck(lastFirst, second, toTreeWidgetCheck::CheckBox);
                    item->setText(1, first);
                    item->setText(2, top);
                }
            }

            if (top == QString::fromLatin1("PUBLIC"))
            {
                toQList object = toQuery::readQueryNull(conn, SQLPublicSynonymList);
                toTreeWidgetItem *topItem = new toResultViewCheck(item, QString::fromLatin1("SYNONYM"), toTreeWidgetCheck::CheckBox);
                while (object.size() > 0)
                {
                    toTreeWidgetItem *item = new toResultViewCheck(topItem, toShift(object),
                            toTreeWidgetCheck::CheckBox);
                    item->setText(1, QString::fromLatin1("SYNONYM"));
                    item->setText(2, top);
                }
            }
        }
        TOCATCH
    }
}

void toScript::changeMode(int mode)
{
    if (mode < 0 || mode > 4)
        return ;

    if (mode == 0 || mode == 2)
        ScriptUI->Destination->setEnabled(true);
    else if (mode == 1 || mode == 3 || mode == 4)
        ScriptUI->Destination->setEnabled(false);

    if (mode == 1 || mode == 2)
        ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->ResizeTab), true);
    else if (mode == 0 || mode == 3 || mode == 4)
        ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->ResizeTab), false);

    ScriptUI->IncludeContent->setEnabled(mode == 1);
    ScriptUI->CommitDistance->setEnabled(mode == 1);

    if (mode == 1 || mode == 2)
    {
        ScriptUI->IncludeHeader->setEnabled(true);
        ScriptUI->IncludePrompt->setEnabled(true);
    }
    else if (mode == 0 || mode == 3 || mode == 4)
    {
        ScriptUI->IncludeHeader->setEnabled(false);
        ScriptUI->IncludePrompt->setEnabled(false);
    }

    if (mode == 0 || mode == 2 || mode == 3 || mode == 4)
    {
        ScriptUI->IncludeDDL->setEnabled(false);
        ScriptUI->IncludeDDL->setChecked(true);
    }
    else if (mode == 1)
        ScriptUI->IncludeDDL->setEnabled(true);

    ScriptUI->OutputGroup->setEnabled(mode == 1 || mode == 2 || mode == 4);

    ScriptUI->IncludeConstraints->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeIndexes->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeGrants->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeStorage->setEnabled(ScriptUI->IncludeDDL->isChecked() && mode != 3);
    ScriptUI->IncludeParallell->setEnabled(ScriptUI->IncludeDDL->isChecked() && mode != 3);
    ScriptUI->IncludePartition->setEnabled(ScriptUI->IncludeDDL->isChecked() && mode != 3);
    ScriptUI->IncludeCode->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeComment->setEnabled(ScriptUI->IncludeDDL->isChecked());
}

void toScript::keepOn(toTreeWidgetItem *parent)
{
    if (!parent)
        return ;
    toResultViewCheck *pchk = dynamic_cast<toResultViewCheck *>(parent);
    if (!pchk)
        return ;
    pchk->setOn(true);
}

void toScript::objectClicked(toTreeWidgetItem *parent)
{
    if (!parent)
        return ;
    toResultViewCheck *pchk = dynamic_cast<toResultViewCheck *>(parent);
    if (!pchk)
        return ;
    if (!parent->parent() && !parent->firstChild())
    {
        if (parent->listView() == ScriptUI->SourceObjects)
            expandSource(parent);
        else if (parent->listView() == ScriptUI->DestinationObjects)
            expandDestination(parent);
    }
    bool on = pchk->isOn();

    for(toTreeWidgetItemIterator it(parent); (*it); it++) {
        toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>((*it));
        if (chk)
            chk->setOn(on);
    }
}

void toScript::changeSchema(int, bool source)
{
    QString src = (source ? ScriptUI->SourceSchema : ScriptUI->DestinationSchema)->currentText();
    for (toTreeWidgetItem *parent = (source ?
                                   ScriptUI->SourceObjects :
                                   ScriptUI->DestinationObjects)->firstChild();
            parent;
            parent = parent->nextSibling())
    {
        toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>(parent);
        if (chk)
        {
            bool ena = ((src == chk->text(0)) || (src == QString::fromLatin1("All")));

            toTreeWidgetItem *next = NULL;
            for (toTreeWidgetItem *item = parent;item;item = next)
            {
                chk = dynamic_cast<toResultViewCheck *>(item);
                if (chk)
                {
                    chk->setEnabled(ena);
                    (source ? ScriptUI->SourceObjects : ScriptUI->DestinationObjects)->repaintItem(chk);
                }

                if (item->firstChild())
                    next = item->firstChild();
                else if (item->nextSibling() && item->nextSibling()->depth())
                    next = item->nextSibling();
                else
                {
                    next = item;
                    do
                    {
                        next = next->parent();
                        if (next == parent)
                            break;
                    }
                    while (next && !next->nextSibling());
                    if (next == parent)
                        break;
                    if (next)
                        next = next->nextSibling();
                }
            }

        }
    }
}

void toScript::newSize(void)
{
    QString init = ScriptUI->Initial->sizeString();
    QString next = ScriptUI->Next->sizeString();
    QString max = ScriptUI->Limit->sizeString();
    QString maxNum;
    maxNum.sprintf("%010d", ScriptUI->Limit->value());

    for (toTreeWidgetItem *item = ScriptUI->Sizes->firstChild();item;item = item->nextSibling())
        if (max == item->text(0))
        {
            toStatusMessage(tr("Replacing existing size with new"), false, false);
            delete item;
            break;
        }

    new toTreeWidgetItem(ScriptUI->Sizes, max, init, next, maxNum);
    ScriptUI->Sizes->setSorting(3);
}

void toScript::removeSize(void)
{
    toTreeWidgetItem *item = ScriptUI->Sizes->selectedItem();
    if (item)
        delete item;
}

void toScript::setupExtract(toExtract &extr)
{
    extr.setCode (ScriptUI->IncludeCode->isEnabled() &&
                  ScriptUI->IncludeCode->isChecked() );
    extr.setComments (ScriptUI->IncludeComment->isEnabled() &&
                      ScriptUI->IncludeComment->isChecked() );
    extr.setConstraints(ScriptUI->IncludeConstraints->isEnabled() &&
                        ScriptUI->IncludeConstraints->isChecked());
    extr.setContents (ScriptUI->IncludeContent->isEnabled() &&
                      ScriptUI->IncludeContent->isChecked() ,
                      ScriptUI->CommitDistance->value() );
    extr.setGrants (ScriptUI->IncludeGrants->isEnabled() &&
                    ScriptUI->IncludeGrants->isChecked() );
    extr.setHeading (ScriptUI->IncludeHeader->isEnabled() &&
                     ScriptUI->IncludeHeader->isChecked() );
    extr.setIndexes (ScriptUI->IncludeIndexes->isEnabled() &&
                     ScriptUI->IncludeIndexes->isChecked() );
    extr.setParallel (ScriptUI->IncludeParallell->isEnabled() &&
                      ScriptUI->IncludeParallell->isChecked() );
    extr.setPartition (ScriptUI->IncludePartition->isEnabled() &&
                       ScriptUI->IncludePartition->isChecked() );
    extr.setPrompt (ScriptUI->IncludePrompt->isEnabled() &&
                    ScriptUI->IncludePrompt->isChecked() );
    extr.setStorage (ScriptUI->IncludeStorage->isEnabled() &&
                     ScriptUI->IncludeStorage->isChecked() );

    if (ScriptUI->Schema->currentText() == tr("Same"))
        extr.setSchema(QString::fromLatin1("1"));
    else if (ScriptUI->Schema->currentText() == tr("None"))
        extr.setSchema(QString::null);
    else
        extr.setSchema(ScriptUI->Schema->currentText());

    if (ScriptUI->DontResize->isChecked())
        extr.setResize(QString::null);
    else if (ScriptUI->AutoResize->isChecked())
        extr.setResize(QString::fromLatin1("1"));
    else
    {
        QString siz;
        for (toTreeWidgetItem *item = ScriptUI->Sizes->firstChild();item;item = item->nextSibling())
        {
            siz += item->text(0);
            siz += QString::fromLatin1(":");
            siz += item->text(1);
            siz += QString::fromLatin1(":");
            siz += item->text(2);
            if (item->nextSibling())
                siz += QString::fromLatin1(":");
        }
        extr.setResize(siz);
    }
}

void toScript::expandSource(toTreeWidgetItem *item)
{
    try
    {
        if (item)
            readOwnerObjects(item,
                             toMainWidget()->connection(ScriptUI->SourceConnection->currentText()));
    }
    TOCATCH
}

void toScript::expandDestination(toTreeWidgetItem *item)
{
    try
    {
        if (item)
            readOwnerObjects(item,
                             toMainWidget()->connection(ScriptUI->DestinationConnection->currentText()));
    }
    TOCATCH
}

void toScript::browseFile(void)
{
    if (ScriptUI->OutputFile->isChecked())
    {
        QString f = toOpenFilename(QString::null, QString::null, this);
        if (!f.isEmpty())
            ScriptUI->Filename->setText(f);
    }
    else if (ScriptUI->OutputDir->isChecked())
    {
        QString f = TOFileDialog::getExistingDirectory(this, tr("Select a Directory"), QString::null);
        if (!f.isEmpty())
            ScriptUI->Filename->setText(f);
    }
}
