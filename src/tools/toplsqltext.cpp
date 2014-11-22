
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

#include "tools/toplsqltext.h"
#include "tools/toplsqleditor.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconnection.h"
#include "core/toquery.h"
#include "core/totreewidget.h"
#include "core/tosql.h"
#include "core/toresultview.h"
#include "core/tocodemodel.h"
#include "core/todescribe.h"
#include "core/toresultschema.h"
#include "core/toglobalevent.h"
#include "core/topushbutton.h"
#include "editor/tosqltext.h"
#include "editor/tohighlightededitor.h"
#include "editor/todebugtext.h"

#include <QtCore/QSettings>
#include <QtCore/QTemporaryFile>
#include <QtCore/QProcess>
#include <QToolBar>
#include <QMenu>
#include <QTreeView>
#include <QSplitter>
#include <QToolButton>

#include "icons/close.xpm"
#include "icons/compile.xpm"
#include "icons/compilewarning.xpm"
#include "icons/refresh.xpm"
#include "icons/toworksheet.xpm"
#include "icons/nextbug.xpm"
#include "icons/prevbug.xpm"
#include "icons/checkcode.xpm"
#include "icons/describe.xpm"


class toPLSQLTool : public toTool
{
    std::map<toConnection *, QWidget *> Windows;

    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(compile_xpm);
    }
public:
    toPLSQLTool()
        : toTool(109, "PL/SQL Editor")
    { }
    virtual const char *menuItem()
    {
        return "PL/SQL Editor";
    }
    virtual toToolWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toPLSQL(parent, connection);
    }
    void closeWindow(toConnection &connection)
    {
    }
    virtual bool canHandle(const toConnection &conn)
    {
        if (conn.providerIs("Oracle"))
        {
            return true;
        }
        return false;
    }
};

static toPLSQLTool PLSQLTool;

QString toPLSQL::editorName(const QString &schema, const QString &object, const QString &type)
{
    //QString ret = connection().quote(schema) + "." + connection().quote(object);
    QString ret = schema + "." + object;
    if (type.contains("BODY"))
        ret += tr(" body");
    return ret;

}

QString toPLSQL::editorName(toPLSQLWidget *text)
{
    return editorName(text->editor()->schema(),
                      text->editor()->object(),
                      text->editor()->type());
}

QString toPLSQL::currentSchema(void)
{
    return Schema->selected();
}

bool toPLSQL::viewSource(const QString &schema, const QString &name, const QString &type,
                               int line, bool setCurrent)
{
    try
    {
        toPLSQLWidget *editor = NULL;
        int row = line - 1;
        int col = 0;
        for (int i = 0; i < Editors->count(); i++)
        {
            QString tabname = editorName(schema, name, type);
            toPLSQLWidget *te = dynamic_cast<toPLSQLWidget *>(Editors->widget(i));
            te->setObjectName(tabname);
            if (Editors->tabText(Editors->indexOf(te)) == tabname)
            {
                editor = te;
                break;
            }
            if (Editors->tabText(Editors->indexOf(te)) == tr("Unknown") && !te->editor()->sciEditor()->isModified())
                editor = te;
        }
        if (!editor)
        {
            editor = new toPLSQLWidget(Editors);
            Editors->addTab(editor, editorName(editor));
            editor->setObjectName(editorName(editor));
        }
        else
        {
            editor->editor()->sciEditor()->getCursorPosition(&row, &col);
        }
        // set pointer to this PLSQLEditor to be used when saving packages/types
        editor->editor()->setEditor(this);
        if (editor->editor()->sciEditor()->lines() <= 1)
        {
            editor->editor()->setData(schema, type, name);
            editor->editor()->readData(connection()/*, StackTrace*/);
            Editors->setTabText(Editors->indexOf(editor), editorName(editor));
            editor->setObjectName(editorName(editor));
            //TODO
//            if (editor->editor()->hasErrors())
//                Editors->setTabIcon(Editors->indexOf(editor),
//                                    QIcon(QPixmap(const_cast<const char**>(nextbug_xpm))));
//            else
//                Editors->setTabIcon(Editors->indexOf(editor), QIcon());
        }
        Editors->setCurrentIndex(Editors->indexOf(editor));
        editor->editor()->sciEditor()->setCursorPosition(row, col);
        if (setCurrent)
            editor->editor()->setCurrentDebugLine(line - 1);
        editor->setFocus();
        return true;
    }
    catch (const QString &str)
    {
        Utils::toStatusMessage(str);
        return false;
    }
}

toPLSQL::toPLSQL(QWidget *main, toConnection &connection)
    : toToolWidget(PLSQLTool, "plsqleditor.html", main, connection, "toPLSQLEditor")
	, conn(connection)
{
    createActions();
    QToolBar *toolbar = Utils::toAllocBar(this, tr("PLSQLEditor"));
    layout()->addWidget(toolbar);

    toolbar->addAction(refreshAct);

    toolbar->addSeparator();

    Schema = new toResultSchema(toolbar, "PLSQLEditorSchemaCombo");
    toolbar->addWidget(Schema);
    connect(Schema,
            SIGNAL(activated(int)),
            this,
            SLOT(changeSchema(int)));

    toolbar->addSeparator();

    toolbar->addAction(newSheetAct);
    toolbar->addAction(compileAct);
    toolbar->addAction(compileWarnAct);
    // only show static check button when static checker is specified
// TODO
//    if (!toConfigurationSingle::Instance().staticChecker().isEmpty())
//    {
//        toolbar->addAction(checkCodeAct);
//    }

    toolbar->addSeparator();

    toolbar->addAction(nextErrorAct);
    toolbar->addAction(previousErrorAct);
    toolbar->addAction(describeAct);

    toolbar->addWidget(new Utils::toSpacer());

    splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(splitter);

    Objects = new QTreeView(splitter);
    CodeModel = new toCodeModel(Objects);
    Objects->setModel(CodeModel);
    QString selected = Schema->selected();
    if (!selected.isEmpty())
        CodeModel->refresh(connection, selected);
    // even better (?) for reopening the tabs
//     connect(Objects->selectionModel(),
//             SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
//             this,
//             SLOT(changePackage(const QModelIndex &, const QModelIndex &)));
    connect(Objects, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(changePackage(const QModelIndex &)));

    splitter->addWidget(Objects);

    Editors = new QTabWidget(this);
#if QT_VERSION >= 0x040500
    Editors->setTabsClosable(true);
    connect(Editors, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeEditor(int)));
#endif
    splitter->addWidget(Editors);
    Editors->setTabPosition(QTabWidget::North);

    QToolButton *closeButton = new toPopupButton(Editors);
    closeButton->setIcon(QPixmap(const_cast<const char**>(close_xpm)));
    closeButton->setFixedSize(20, 18);
    // HACK: disable closing the editor tabs with the global shortcut.
    // it raises: "QAction::eventFilter: Ambiguous shortcut overload: Ctrl+W"
    // on some systems. But it rejects to close unfinished.unsaved work.
    // - <petr@scribus.info>
    closeButton->setShortcut(QKeySequence::Close);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeEditor()));

    Editors->setCornerWidget(closeButton);

    setFocusProxy(Editors);
    newSheet();

    ToolMenu = NULL;

    QSettings s;
    s.beginGroup("toPLSQLEditor");
    splitter->restoreState(s.value("splitter").toByteArray());
    s.endGroup();

    refresh();
}


void toPLSQL::createActions(void)
{
    refreshAct = new QAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                             tr("Refresh"),
                             this);
    connect(refreshAct,
            SIGNAL(triggered()),
            this,
            SLOT(refresh()),
            Qt::QueuedConnection);
    refreshAct->setShortcut(QKeySequence::Refresh);

    describeAct = new QAction(QPixmap(const_cast<const char**>(describe_xpm)),
                              tr("Describe under cursor"),
                              this);
    describeAct->setShortcut(Qt::Key_F4);
    connect(describeAct, SIGNAL(triggered()), this, SLOT(describe(void)));

    newSheetAct = new QAction(QIcon(QPixmap(const_cast<const char**>(toworksheet_xpm))),
                              tr("&New Sheet"),
                              this);
    connect(newSheetAct,
            SIGNAL(triggered()),
            this,
            SLOT(newSheet()),
            Qt::QueuedConnection);

    compileAct = new QAction(QIcon(QPixmap(const_cast<const char**>(compile_xpm))),
                             tr("&Compile"),
                             this);
    connect(compileAct,
            SIGNAL(triggered()),
            this,
            SLOT(compile()),
            Qt::QueuedConnection);
    compileAct->setShortcut(Qt::Key_F9);

    compileWarnAct = new QAction(QIcon(QPixmap(const_cast<const char**>(compilewarning_xpm))),
                                 tr("&Compile with Warnings"),
                                 this);
    connect(compileWarnAct,
            SIGNAL(triggered()),
            this,
            SLOT(compileWarn()),
            Qt::QueuedConnection);

    closeAct = new QAction(QIcon(QPixmap(const_cast<const char**>(close_xpm))),
                           tr("Close"),
                           this);
    connect(closeAct,
            SIGNAL(triggered()),
            this,
            SLOT(closeEditor()),
            Qt::QueuedConnection);

    closeAllAct = new QAction(tr("Close All"),
                              this);
    connect(closeAllAct,
            SIGNAL(triggered()),
            this,
            SLOT(closeAllEditor()),
            Qt::QueuedConnection);

    nextErrorAct = new QAction(QIcon(QPixmap(const_cast<const char**>(nextbug_xpm))),
                               tr("Next &Error"),
                               this);
    connect(nextErrorAct,
            SIGNAL(triggered()),
            this,
            SLOT(nextError()),
            Qt::QueuedConnection);
    nextErrorAct->setShortcut(Qt::CTRL + Qt::Key_N);

    previousErrorAct = new QAction(QIcon(QPixmap(const_cast<const char**>(prevbug_xpm))),
                                   tr("Pre&vious Error"),
                                   this);
    connect(previousErrorAct,
            SIGNAL(triggered()),
            this,
            SLOT(prevError()),
            Qt::QueuedConnection);
    previousErrorAct->setShortcut(Qt::CTRL + Qt::Key_P);

    // tool button to check code
    checkCodeAct = new QAction(QIcon(QPixmap(const_cast<const char**>(checkcode_xpm))),
                               tr("&Check Code"),
                               this);
    connect(checkCodeAct,
            SIGNAL(triggered()),
            this,
            SLOT(checkCode()),
            Qt::QueuedConnection);
    checkCodeAct->setShortcut(Qt::CTRL + Qt::Key_G); // TODO: change shortcut to something meaningful :)
}

toPLSQLWidget *toPLSQL::currentEditor(void)
{
    return dynamic_cast<toPLSQLWidget *>(Editors->currentWidget());
}

void toPLSQL::changeSchema(int)
{
    refresh();
}

void toPLSQL::refresh(void)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
           Schema->refresh();
    QString selected = Schema->selected();

    CodeModel->refresh(connection(), selected);

    QApplication::restoreOverrideCursor();
}

void toPLSQL::closeEvent(QCloseEvent *e)
{
    QSettings s;
    s.beginGroup("toPLSQLEditor");
    s.setValue("splitter", splitter->saveState());
    s.endGroup();

    toToolWidget::closeEvent(e);
}

void toPLSQL::updateCurrent()
{
    try
    {
        toPLSQLWidget *editor = currentEditor();

        editor->editor()->readData(connection()/*, StackTrace*/);
        editor->editor()->setFocus();
    }
    TOCATCH
}

void toPLSQL::changePackage(const QModelIndex &current)
{
    changePackage(current, QModelIndex());
}

static toSQL SQLHasCode("toPLSQLEditor:HasCode",
                        "SELECT count(1) FROM all_source\n"
                        " WHERE OWNER = :f1<char[101]>\n"
                        "   AND NAME = :f2<char[101]>\n"
                        "   AND TYPE = :f3<char[101]>\n",
                        "Checks if given object has any source");
// checks if given object has any source code
bool toPLSQL::hasCode(const QString &pSchema, const QString &pType, const QString &pName)
{
    int numberOfLines;

    try
    {
    	toConnectionSubLoan c(conn);
        toQuery lines(c, SQLHasCode, toQueryParams() << pSchema << pName << pType);
        numberOfLines = lines.readValue().toInt();
        if (numberOfLines > 0)
            return true;
        else
            return false;
    }
    TOCATCH
    return true; // try displaying the code in case of error
} // hasCode

void toPLSQL::changePackage(const QModelIndex &current, const QModelIndex &previous)
{
    Utils::toBusy busy;

//     TLOG(2,toDecorator,__HERE__) << "toDebug::changePackage 1";
    toCodeModelItem *item = static_cast<toCodeModelItem*>(current.internalPointer());
    if (item && item->parent())
    {
        QString ctype = item->parent()->display();
        if (ctype.isEmpty() || ctype == "Code")
            return;
        ctype = ctype.toUpper();

        viewSource(Schema->selected(), item->display(), ctype, 0);
        if (ctype == "PACKAGE" ||
                (ctype == "TYPE" && hasCode(Schema->selected(), ctype + " BODY", item->display())))
            viewSource(Schema->selected(), item->display(), ctype + " BODY", 0);
    }
#ifdef AUTOEXPAND
    else if (item && !item->parent())
        item->setOpen(true);
#endif
// TLOG(2,toDecorator,__HERE__) << "toDebug::changePackage 2";
}

void toPLSQL::compileWarn()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    currentEditor()->editor()->compile(toPLSQLEditor::Warning);
    QApplication::restoreOverrideCursor();
}

void toPLSQL::compile(void)
{
    // compile only current editor/tab
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    currentEditor()->editor()->compile(toPLSQLEditor::Production);
    QApplication::restoreOverrideCursor();
//     refresh();
}

toPLSQL::~toPLSQL()
{
}

void toPLSQL::prevError(void)
{
	//TODO
    //currentEditor()->editor()->previousError();
}

void toPLSQL::nextError(void)
{
	//TODO
    //currentEditor()->editor()->nextError();
}

void toPLSQL::newSheet(void)
{
    toPLSQLWidget *text = new toPLSQLWidget(Editors);
    if (!Schema->selected().isEmpty())
        text->editor()->setSchema(Schema->selected());
    else
        text->editor()->setSchema(connection().user().toUpper());
    Editors->addTab(text, tr("Unknown"));
    Editors->setCurrentIndex(Editors->indexOf(text));
}

void toPLSQL::showSource(toTreeWidgetItem *item)
{
    if (item)
        viewSource(item->text(2), item->text(0), item->text(3), item->text(1).toInt(), false);
}

void toPLSQL::slotWindowActivated(toToolWidget *widget)
{
    if (!widget)
        return;
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&PL/SQL Editor"), this);

            ToolMenu->addAction(refreshAct);
            ToolMenu->addAction(describeAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(newSheetAct);
            ToolMenu->addAction(compileAct);
            ToolMenu->addAction(compileWarnAct);
            ToolMenu->addAction(closeAct);
            ToolMenu->addAction(closeAllAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(nextErrorAct);
            ToolMenu->addAction(previousErrorAct);

    	    toGlobalEventSingle::Instance().addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toPLSQL::closeEditor()
{
    toPLSQLWidget *editor = currentEditor();
    closeEditor(editor);
}

void toPLSQL::closeEditor(int ix)
{
    toPLSQLWidget * w = qobject_cast<toPLSQLWidget*>(Editors->widget(ix));
    assert(w);
    closeEditor(w);
}

void toPLSQL::closeAllEditor()
{
    int editorCount = Editors->count();
    while (editorCount > 0)
    {
        editorCount--;
        toPLSQLWidget *editor = dynamic_cast<toPLSQLWidget *>(Editors->widget(editorCount));
        if (editor)
            closeEditor(editor);
    }
}

void toPLSQL::closeEditor(toPLSQLWidget* &editor)
{

    if (editor)
    {
        QString name = editor->objectName();
//         if (Objects->selectedItem() &&
//                 Objects->selectedItem()->text(0) == editor->editor()->object() &&
//                 Schema->currentText() == editor->editor()->schema())
//             Objects->clearSelection();

        Editors->removeTab(Editors->indexOf(editor));
        delete editor;
        if (Editors->count() == 0)
            newSheet();
    }
}

void toPLSQL::describe()
{
    toCache::ObjectRef table;
    table.context = currentSchema();
    toPLSQLEditor *marked = currentEditor()->editor();
    marked->editor()->tableAtCursor(table);
    if (table.first.isEmpty())
    	table.first = Schema->selected();
    toDescribe * d = new toDescribe(this);
    d->changeParams(table);
}

/* Purpose: should find and return object containing another part of package.
   For example if specification of package A is given as parameter it should
   find toPLSQLText object containing body of the same package.
*/
toPLSQLEditor * toPLSQL::getAnotherPart(QString &pSchema, QString &pObject, QString &pType)
{
    QString other_part_type;
    toPLSQLEditor * ret = NULL;

    if (pType == "PACKAGE")
        other_part_type = "PACKAGE BODY";
    else
        other_part_type = "PACKAGE";

    for (int i = 0; i < Editors->count(); i++)
    {
        toPLSQLWidget *te = dynamic_cast<toPLSQLWidget *>(Editors->widget(i));
        if ((te->editor()->schema() == pSchema) &&
                (te->editor()->object() == pObject) &&
                (te->editor()->type()   == other_part_type))
        {
            ret = te->editor();
            break;
        }
    }

    return ret;
} // getAnotherPart

void toPLSQL::parseResults(const QString buf, QMultiMap<int, QString> &res)
{
    // TODO: probably a problem on Mac as it has \r as new line character?
    QStringList list = buf.split("\n", QString::SkipEmptyParts);
    int j;
    for (int i = 0; i < list.size(); i++)
    {
        j = list[i].indexOf(":");
        res.insert(list[i].left(j).trimmed().toInt(), list[i].right(list[i].length() - j - 1).trimmed());
    }
} // parseResults

/* Purpose: calls external static code test tool and display it's results
   TODO: call only selected part of code (if something is selected)
*/
void toPLSQL::checkCode(void)
{
    if (currentEditor()->editor()->sciEditor()->text().isEmpty())
    {
        // do nothing if code text is empty
        return;
    }

    QTemporaryFile tf;
    if (tf.open())
    {
        if (!Utils::toWriteFile(tf.fileName(), currentEditor()->editor()->sciEditor()->text()))
        {
#ifdef DEBUG
            TLOG(2, toDecorator, __HERE__) << "Unable to write file (" + tf.fileName() + ")";
#endif
            return;
        }
        else
        {
#ifdef DEBUG
            TLOG(2, toDecorator, __HERE__) << "Success!!! Temporary file " + tf.fileName();
#endif
        }
    }

    //TODO
    //QString program = toConfigurationSingle::Instance().staticChecker().arg(tf.fileName());
#ifdef DEBUG
    ///TODOTLOG(2, toDecorator, __HERE__) << "program to be executed: " + program;
#endif
    QProcess staticCheck(qApp);

    staticCheck.setProcessChannelMode(QProcess::MergedChannels);
    //TODO
    //staticCheck.start(program);
    staticCheck.waitForFinished(); // default timeout - 30000 miliseconds

    int exit_code = staticCheck.exitStatus();
    if (exit_code != 0)
    {
#ifdef DEBUG
        TLOG(2, toDecorator, __HERE__) << "Error executing static check. Exit code = " << exit_code;
        int run_error = staticCheck.error();
        // error values taken from Qt4.6 documentation for QProcess
        switch (run_error)
        {
        case 0:
            TLOG(2, toDecorator, __HERE__) << "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.";
            break;
        case 1:
            TLOG(2, toDecorator, __HERE__) << "The process crashed some time after starting successfully.";
            break;
        case 5:
            TLOG(2, toDecorator, __HERE__) << "An unknown error occurred.";
            break;
        default:
            TLOG(2, toDecorator, __HERE__) << "Error code: " << run_error << "--" << staticCheck.errorString();
        } // switch
#endif
        return;
    }
    QString qq = staticCheck.readAllStandardOutput();
#ifdef DEBUG
    TLOG(2, toDecorator, __HERE__) << "stdout" << qq;
#endif

    QMultiMap<int, QString> Observations;
    parseResults(qq, Observations);
    currentEditor()->editor()->setErrors(Observations, false);
    currentEditor()->applyResult("STATIC", Observations);
    currentEditor()->resizeResults();
} // checkCode

