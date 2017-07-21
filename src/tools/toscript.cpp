
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

//
#include "tools/toscript.h"
#include "core/utils.h"
#include "core/totool.h"
#include "core/tomainwindow.h"
//#include "core/toreport.h"
#include "core/totextview.h"
#include "core/toextract.h"
#include "core/toconfiguration.h"
#include "editor/tosqltext.h"
#include "tools/toscripttreeitem.h"
#include "connection/tooracleconfiguration.h"

#include <QScrollArea>
#include <QMessageBox>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QFileDialog>
#include <QCompleter>
#include <QDirModel>
#include <QtCore/QSettings>
#include <QSplitter>
#include <QtCore/QTextStream>
#include <QToolBar>

#include "icons/execute.xpm"
#include "icons/toscript.xpm"
#include "toresultview.h"


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

        virtual bool canHandle(const toConnection &conn)
        {
            try
            {
                return
                    // toExtract::canHandle(conn) && TODO
                    conn.providerIs("Oracle"); //TODO&& !toSQL::string(SQLObjectList, conn).isEmpty();
            }
            catch (...)
            {
                return false;
            }
        }
        virtual toToolWidget *toolWindow(QWidget *main, toConnection &connection)
        {
            return new toScript(main, connection);
        }
        virtual void closeWindow(toConnection &connection) {};
};

static toScriptTool ScriptTool;


// aliases for basic tool modes
static const int MODE_COMPARE = 0;
static const int MODE_EXTRACT = 1;
static const int MODE_SEARCH  = 2;
static const int MODE_REPORT  = 4;

toScript::toScript(QWidget *parent, toConnection &connection)
    : toToolWidget(ScriptTool, "script.html", parent, connection, "toScript")
{
    QToolBar *toolbar = Utils::toAllocBar(this, tr("Extraction and Compare"));
    layout()->addWidget(toolbar);

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(execute_xpm))),
                       tr("Perform defined extraction"),
                       this,
                       SLOT(execute(void)));

    toolbar->addWidget(new Utils::toSpacer());

    ScriptUI = new Ui::toScriptUI();
    ScriptUI->setupUi(this);
    layout()->addWidget(ScriptUI->Tabs);

    ScriptUI->Source->setTitle(tr("Source"));
    ScriptUI->Destination->setTitle(tr("Destination"));

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
    WorksheetText = new toSqlText(box);
    vbox->addWidget(WorksheetText);
    SearchList = new toListView(box);
    vbox->addWidget(SearchList);
    SearchList->addColumn(tr("Search result"));
    SearchList->setRootIsDecorated(true);
    SearchList->setSorting(0);
    SearchList->hide();
    Report = new toTextView(box);
    vbox->addWidget(Report);
    Report->hide();

    QCompleter *FilenameCompleter = new QCompleter(this);
    FilenameCompleter->setModel(new QDirModel(FilenameCompleter));
    ScriptUI->Filename->setCompleter(FilenameCompleter);

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
    group->addButton(ScriptUI->Compare, MODE_COMPARE);
    group->addButton(ScriptUI->Extract, MODE_EXTRACT);
    group->addButton(ScriptUI->Search, MODE_SEARCH);
    group->addButton(ScriptUI->Report, MODE_REPORT);

    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(changeMode(int)));

    ScriptUI->Source->setConnectionString(connection.description());
    ScriptUI->Destination->setConnectionString(connection.description());

    connect(ScriptUI->Browse, SIGNAL(clicked()), this, SLOT(browseFile()));

    ScriptUI->Schema->setCurrentIndex(0);
    setFocusProxy(ScriptUI->Tabs);

    QSettings s;
    s.beginGroup("toScript");
    // radiobuttons - modes
    ScriptUI->Compare->setChecked(s.value("Compare", true).toBool());
    ScriptUI->Extract->setChecked(s.value("Extract", false).toBool());
    ScriptUI->Search->setChecked(s.value("Search", false).toBool());
    ScriptUI->Report->setChecked(s.value("Report", false).toBool());
    // checkboxes - options
    ScriptUI->IncludeDDL->setChecked(s.value("IncludeDDL", true).toBool());
    ScriptUI->UseDbmsMetadataBool->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::UseDbmsMetadataBool).toBool());
    ScriptUI->IncludeConstraints->setChecked(s.value("IncludeConstraints", true).toBool());
    ScriptUI->IncludeIndexes->setChecked(s.value("IncludeIndexes", true).toBool());
    ScriptUI->IncludeGrants->setChecked(s.value("IncludeGrants", true).toBool());
    ScriptUI->IncludeStorage->setChecked(s.value("IncludeStorage", true).toBool());
    ScriptUI->IncludeParallell->setChecked(s.value("IncludeParallell", true).toBool());
    ScriptUI->IncludePartition->setChecked(s.value("IncludePartition", true).toBool());
    ScriptUI->IncludeCode->setChecked(s.value("IncludeCode", true).toBool());
    ScriptUI->IncludeComment->setChecked(s.value("IncludeComment", true).toBool());
    ScriptUI->IncludePrompt->setChecked(s.value("IncludePrompt", false).toBool());
    ScriptUI->IncludeHeader->setChecked(s.value("IncludeHeader", true).toBool());
    ScriptUI->IncludeContent->setChecked(s.value("IncludeContent", false).toBool());
    ScriptUI->CommitDistance->setValue(s.value("CommitDistance", 0).toInt());
    ScriptUI->Schema->setEditText(s.value("Schema", "Same").toString());
    // target
    ScriptUI->OutputTab->setChecked(s.value("OutputTab", true).toBool());
    ScriptUI->OutputFile->setChecked(s.value("OutputFile", false).toBool());
    ScriptUI->OutputDir->setChecked(s.value("OutputDir", false).toBool());
    ScriptUI->Filename->setText(s.value("Filename", "").toString());
    s.endGroup();

    changeMode(group->checkedId());
}

toScript::~toScript()
{}

void toScript::closeEvent(QCloseEvent *event)
{
    if (WorksheetText /*&& WorksheetText->slotClose()*/)
    {
        QSettings s;
        s.beginGroup("toScript");
        // radiobuttons - modes
        s.setValue("Compare", ScriptUI->Compare->isChecked());
        s.setValue("Extract", ScriptUI->Extract->isChecked());
        s.setValue("Search", ScriptUI->Search->isChecked());
        s.setValue("Report", ScriptUI->Report->isChecked());
        // checkboxes - options
        s.setValue("Report", ScriptUI->Report->isChecked());
        s.setValue("IncludeDDL", ScriptUI->IncludeDDL->isChecked());
        s.setValue("IncludeConstraints", ScriptUI->IncludeConstraints->isChecked());
        s.setValue("IncludeIndexes", ScriptUI->IncludeIndexes->isChecked());
        s.setValue("IncludeGrants", ScriptUI->IncludeGrants->isChecked());
        s.setValue("IncludeStorage", ScriptUI->IncludeStorage->isChecked());
        s.setValue("IncludeParallell", ScriptUI->IncludeParallell->isChecked());
        s.setValue("IncludePartition", ScriptUI->IncludePartition->isChecked());
        s.setValue("IncludeCode", ScriptUI->IncludeCode->isChecked());
        s.setValue("IncludeComment", ScriptUI->IncludeComment->isChecked());
        s.setValue("IncludePrompt", ScriptUI->IncludePrompt->isChecked());
        s.setValue("IncludeHeader", ScriptUI->IncludeHeader->isChecked());
        s.setValue("IncludeContent", ScriptUI->IncludeContent->isChecked());
        s.setValue("CommitDistance", ScriptUI->CommitDistance->value());
        s.setValue("Schema", ScriptUI->Schema->currentText());
        // target
        s.setValue("OutputTab", ScriptUI->OutputTab->isChecked());
        s.setValue("OutputFile", ScriptUI->OutputFile->isChecked());
        s.setValue("OutputDir", ScriptUI->OutputDir->isChecked());
        s.setValue("Filename", ScriptUI->Filename->text());
        s.endGroup();
        event->accept();
    }
    else
        event->ignore();
}

toExtract::ObjectList toScript::createObjectList(QItemSelectionModel * selections)
{
    using ObjectRef = toCache::ObjectRef;
    ObjectList lst;

    ObjectList otherGlobal;
    ObjectList profiles;
    ObjectList roles;
    ObjectList tableSpaces;
    ObjectList tables;
    ObjectList userOther;
    ObjectList userViews;
    ObjectList users;

    if (!selections->hasSelection())
        return lst;

    toScriptTreeItem * item;
    foreach (QModelIndex i, selections->selectedIndexes())
    {
        item = static_cast<toScriptTreeItem*>(i.internalPointer());
        Q_ASSERT_X(item, "toScript::createObjectList()", "fatal logic error - never should go there");
        // TLOG(2,toDecorator,__HERE__) << "selected: "<< item << item->schema() << item->data() << item->type();

        if (item->type().isNull())
        {
            continue;
        }

        if (item->type() == "TABLESPACE")
            tableSpaces.append(QPair<QString, ObjectRef>("TABLESPACE", ObjectRef("", item->data(), "")));
        else if (item->type() == "PROFILE")
            profiles.append(QPair<QString, ObjectRef>("PROFILE", ObjectRef("", item->data(), "")));
        else if (item->type() == "ROLE")
            roles.append(QPair<QString, ObjectRef>("ROLE", ObjectRef("", item->data(), "")));
        else if (item->type() == "USER")
            users.append(QPair<QString, ObjectRef>("USER", ObjectRef("", item->data(), "")));
        else if (item->type() == "TABLE")
            tables.append(QPair<QString, ObjectRef>("TABLE FAMILY", ObjectRef(item->schema(), item->data(), "")));
        else if (item->type() == "VIEW") // just to get it *after* tables
            userViews.append(QPair<QString, ObjectRef>("VIEW", ObjectRef(item->schema(), item->data(), "")));
        else
            // the rest goes last (pkgs etc.)
            userOther.append(QPair<QString, ObjectRef>(item->type(), ObjectRef(item->schema(), item->data(), "")));
    }

    if (ScriptUI->IncludeDDL->isChecked())
    {
        lst.append(tableSpaces);
        lst.append(profiles);
        lst.append(otherGlobal);
        lst.append(roles);
        lst.append(users);
        lst.append(tables);
        lst.append(userViews);
        lst.append(userOther);
    }

    foreach(auto i, tables)
    {
        lst.append(QPair<QString, ObjectRef>("TABLE CONTENTS", i.second));
    }

    if (ScriptUI->IncludeDDL->isChecked())
    {
        foreach(auto i, tables)
        {
            lst.append(QPair<QString, ObjectRef>("TABLE REFERENCES", i.second));
        }
        foreach(auto i, roles)
        {
            lst.append(QPair<QString, ObjectRef>("ROLE GRANTS", i.second));
        }
        foreach(auto i, users)
        {
            lst.append(QPair<QString, ObjectRef>("USER GRANTS", i.second));
        }
    }
    return lst;
}

void toScript::execute(void)
{
    try
    {
        int mode;
        if (ScriptUI->Compare->isChecked())
            mode = MODE_COMPARE;
        else if (ScriptUI->Extract->isChecked())
            mode = MODE_EXTRACT;
        else if (ScriptUI->Search->isChecked())
            mode = MODE_SEARCH;
        else if (ScriptUI->Report->isChecked())
            mode = MODE_REPORT;
        else
        {
            Utils::toStatusMessage(tr("No mode selected"));
            return ;
        }
        toExtract::ObjectList sourceObjects = createObjectList(ScriptUI->Source->objectList());
        std::list<QString> sourceDescription;
        std::list<QString> destinationDescription;
        QString script;

        toExtract source(ScriptUI->Source->connection(), this);
        setupExtract(source);

        switch (mode)
        {
            case MODE_EXTRACT:
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

                    if (file.error() != QFile::NoError)
                        throw tr("Couldn't open file %1").arg(file.fileName());

                    QTextStream stream(&file);
                    source.create(stream, sourceObjects);

                    if (file.error() != QFile::NoError)
                        throw tr("Error writing to file %1").arg(file.fileName());

                    script = tr("-- Script generated to file %1 successfully").arg(ScriptUI->Filename->text());
                }
                else if (ScriptUI->OutputDir->isChecked())
                {
                    if (ScriptUI->Filename->text().isEmpty())
                        throw tr("No filename specified");

                    QFile file(ScriptUI->Filename->text() + QDir::separator() + "script.sql");
                    file.open(QIODevice::WriteOnly);

                    if (file.error() != QFile::NoError)
                        throw QString(tr("Couldn't open file %1")).arg(file.fileName());

                    QTextStream stream(&file);

                    stream << tr("rem Master script for DDL reverse engineering by TOra\n"
                                 "\n");

                    QFile pfile(ScriptUI->Filename->text() + QDir::separator() + "script.tpr");
                    pfile.open(QIODevice::WriteOnly);

                    if (pfile.error() != QFile::NoError)
                        throw QString(tr("Couldn't open file %1")).arg(pfile.fileName());

                    QTextStream pstream(&pfile);

                    QRegExp repl("\\W+");
                    for (auto i = sourceObjects.begin(); i != sourceObjects.end(); i++)
                    {
                        toExtract::ObjectList t;
                        t.append(*i);
                        QString fn = QString("%1_%2").arg(i->first).arg(i->second.toString());
                        fn.replace(repl, "_");
                        fn += ".sql";
                        stream << "@" << fn << "\n";

                        QFile tf(ScriptUI->Filename->text() + QDir::separator() + fn);
                        tf.open(QIODevice::WriteOnly);
                        pstream << tf.fileName() << "\n";

                        if (tf.error() != QFile::NoError)
                            throw QString(tr("Couldn't open file %1")).arg(tf.fileName());

                        QTextStream ts(&tf);
                        source.create(ts, t);

                        if (tf.error() != QFile::NoError)
                            throw QString(tr("Error writing to file %1")).arg(tf.fileName());


                        script = tr("-- Scripts generate to directory %1 successfully").arg(ScriptUI->Filename->text());
                        ;
                    }

                    if (file.error() != QFile::NoError)
                        throw QString(tr("Error writing to file %1")).arg(file.fileName());
                    if (pfile.error() != QFile::NoError)
                        throw QString(tr("Error writing to file %1")).arg(pfile.fileName());
                }
                break;
            case MODE_COMPARE:
            case MODE_SEARCH:
            case MODE_REPORT:
                sourceDescription = source.describe(sourceObjects);
                break;
        }

        if (ScriptUI->Destination->isEnabled())
        {
            ObjectList destinationObjects  = createObjectList(ScriptUI->Destination->objectList());
            toExtract destination(ScriptUI->Destination->connection(), this);
            setupExtract(destination);
            switch (mode)
            {
                case MODE_COMPARE:
                case MODE_SEARCH:
                    destinationDescription = destination.describe(destinationObjects);
                    break;
                case MODE_REPORT:
                case MODE_EXTRACT:
                    throw tr("Destination shouldn't be enabled now, internal error");
            }

            std::list<QString> drop;
            std::list<QString> create;

            toExtract::srcDst2DropCreate(sourceDescription, destinationDescription, drop, create);
            sourceDescription = drop;
            destinationDescription = create;
        }
        ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->ResultTab), mode == MODE_EXTRACT || mode == MODE_SEARCH || mode == MODE_REPORT);
        ScriptUI->Tabs->setTabEnabled(ScriptUI->Tabs->indexOf(ScriptUI->DifferenceTab), mode == MODE_COMPARE || mode == MODE_SEARCH);
        if (!script.isEmpty())
        {
            WorksheetText->setText(script);
            WorksheetText->setModified(true);
        }
        if (mode == MODE_SEARCH)
        {
            WorksheetText->hide();
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
                            for (int k = 0; k < words.count(); k++)
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
        else if (mode == MODE_REPORT)
        {
            WorksheetText->hide();
            SearchList->hide();
            Report->show();
#ifdef TORA_REPORT
            QString res = toGenerateReport(source.connection(), sourceDescription);
            Report->setText(res);
            if (ScriptUI->OutputFile->isChecked())
            {
                if (ScriptUI->Filename->text().isEmpty())
                    Utils::toStatusMessage(tr("No filename specified"));
                else
                {
                    QFile file(ScriptUI->Filename->text());
                    file.open(QIODevice::WriteOnly);

                    if (file.error() != QFile::NoError)
                        Utils::toStatusMessage(tr("Couldn't open file %1").arg(file.fileName()));
                    else
                    {
                        QTextStream stream(&file);
                        stream << res;

                        if (file.error() != QFile::NoError)
                            Utils::toStatusMessage(tr("Error writing to file %1").arg(file.fileName()));
                    }
                }
            }
#endif
        }
        else // TODO migrate
        {
            WorksheetText->show();
            SearchList->hide();
            Report->hide();
            fillDifference(sourceDescription, DropList);
            fillDifference(destinationDescription, CreateList);
        }

        if (mode == MODE_COMPARE)
            ScriptUI->Tabs->setCurrentIndex(ScriptUI->Tabs->indexOf(ScriptUI->DifferenceTab));
        else
            ScriptUI->Tabs->setCurrentIndex(ScriptUI->Tabs->indexOf(ScriptUI->ResultTab));
    }
    TOCATCH
}

void toScript::fillDifference(std::list<QString> &objects, toTreeWidget *view)
{
//     TLOG(2,toDecorator,__HERE__) << "toScript::fillDifference" << view;
    view->clear();
    toTreeWidgetItem *last = NULL;
    int lastLevel = 0;
    QStringList lstCtx;
    for (std::list<QString>::iterator i = objects.begin(); i != objects.end(); i++)
    {
//         TLOG(2,toDecorator,__HERE__) << "fillDifference add: " << (*i);
        QStringList ctx = (*i).split(QString("\01"));
        if (last)
        {
            while (last && lastLevel >= int(ctx.count()))
            {
                last = last->parent();
                lastLevel--;
            }

            while (last && lastLevel >= 0 && !Utils::toCompareLists(lstCtx, ctx, (unsigned int)lastLevel))
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
        toResultViewMLCheck *item;
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

void toScript::changeMode(int mode)
{
    if (mode < MODE_COMPARE || mode > MODE_REPORT)
        return ;

    if (mode == MODE_COMPARE)
        ScriptUI->Destination->setEnabled(true);
    else if (mode == MODE_EXTRACT || mode == MODE_REPORT || mode == MODE_SEARCH)
        ScriptUI->Destination->setEnabled(false);

    ScriptUI->IncludeContent->setEnabled(mode == MODE_EXTRACT);
    ScriptUI->CommitDistance->setEnabled(mode == MODE_EXTRACT);

    if (mode == MODE_EXTRACT)
    {
        ScriptUI->IncludeHeader->setEnabled(true);
        ScriptUI->IncludePrompt->setEnabled(true);
    }
    else if (mode == MODE_COMPARE || mode == MODE_REPORT || mode == MODE_SEARCH)
    {
        ScriptUI->IncludeHeader->setEnabled(false);
        ScriptUI->IncludePrompt->setEnabled(false);
    }

    if (mode == MODE_COMPARE || mode == MODE_SEARCH || mode == MODE_REPORT)
    {
        ScriptUI->IncludeDDL->setEnabled(false);
        ScriptUI->IncludeDDL->setChecked(true);
    }
    else if (mode == MODE_EXTRACT)
        ScriptUI->IncludeDDL->setEnabled(true);

    ScriptUI->OutputGroup->setEnabled(mode == MODE_EXTRACT || mode == MODE_SEARCH || mode == MODE_REPORT);

    ScriptUI->SearchGroup->setVisible(mode == MODE_SEARCH);

    ScriptUI->IncludeConstraints->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeIndexes->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeGrants->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeStorage->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeParallell->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludePartition->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeCode->setEnabled(ScriptUI->IncludeDDL->isChecked());
    ScriptUI->IncludeComment->setEnabled(ScriptUI->IncludeDDL->isChecked());
}

void toScript::keepOn(toTreeWidgetItem *parent)
{
//     TLOG(2,toDecorator,__HERE__) << "void toScript::keepOn(toTreeWidgetItem *parent)";
    if (!parent)
        return ;
    toResultViewCheck *pchk = dynamic_cast<toResultViewCheck *>(parent);
    if (!pchk)
        return ;
    pchk->setOn(true);
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

}

void toScript::browseFile(void)
{
    if (ScriptUI->OutputFile->isChecked())
    {
        QString f = Utils::toOpenFilename(QString::null, this);
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
