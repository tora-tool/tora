
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

#include <QToolBar>
#include <QMenu>
#include <QMdiSubWindow>
#include <QMdiArea>
#include <QSettings>
#include <QTreeView>
#include <QDebug>
#include <QTemporaryFile>
#include <QProcess>

#include "config.h"
#include "utils.h"
#include "toconnection.h"
#include "totreewidget.h"
#include "toplsqleditor.h"
#include "tomarkedtext.h"
#include "tomain.h"
#include "tosql.h"
#include "toresultview.h"
#include "toplsqltext.h"
#include "tocodemodel.h"
#include "todescribe.h"

#include "icons/close.xpm"
#include "icons/compile.xpm"
#include "icons/compilewarning.xpm"
#include "icons/refresh.xpm"
#include "icons/toworksheet.xpm"
#include "icons/nextbug.xpm"
#include "icons/prevbug.xpm"
#include "icons/checkcode.xpm"
#include "icons/describe.xpm"


class toPLSQLEditorTool : public toTool
{
    std::map<toConnection *, QWidget *> Windows;

    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(compile_xpm);
    }
public:
    toPLSQLEditorTool()
            : toTool(109, "PL/SQL Editor")
    { }
    virtual const char *menuItem()
    {
        return "PL/SQL Editor";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toPLSQLEditor(parent, connection);
    }
    void closeWindow(toConnection &connection)
    {
    }
    virtual bool canHandle(toConnection &conn)
    {
        if (toIsOracle(conn))
        {
            return true;
        }
        return false;
    }
};

static toPLSQLEditorTool PLSQLEditorTool;



QString toPLSQLEditor::editorName(const QString &schema, const QString &object, const QString &type)
{
    QString ret = connection().quote(schema) + "." + connection().quote(object);
    if (type.contains("BODY"))
        ret += tr(" body");
    return ret;

}

QString toPLSQLEditor::editorName(toPLSQLWidget *text)
{
    return editorName(text->editor()->schema(),
                      text->editor()->object(),
                      text->editor()->type());
}

QString toPLSQLEditor::currentSchema(void)
{
    return Schema->currentText();
}

bool toPLSQLEditor::viewSource(const QString &schema, const QString &name, const QString &type,
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
            if (Editors->tabText(Editors->indexOf(te)) == tr("Unknown") && !te->editor()->isModified())
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
            editor->editor()->getCursorPosition(&row, &col);
        }
        // set pointer to this PLSQLEditor to be used when saving packages/types
        editor->editor()->setEditor(this);
        if (editor->editor()->lines() <= 1)
        {
            editor->editor()->setData(schema, type, name);
            editor->editor()->readData(connection()/*, StackTrace*/);
            Editors->setTabText(Editors->indexOf(editor), editorName(editor));
            editor->setObjectName(editorName(editor));
            if (editor->editor()->hasErrors())
                Editors->setTabIcon(Editors->indexOf(editor),
                                    QIcon(QPixmap(const_cast<const char**>(nextbug_xpm))));
            else
                Editors->setTabIcon(Editors->indexOf(editor), QIcon());
        }
        Editors->setCurrentIndex(Editors->indexOf(editor));
        editor->editor()->setCursorPosition(row, col);
        if (setCurrent)
            editor->editor()->setCurrent(line - 1);
        editor->setFocus();
        return true;
    }
    catch (const QString &str)
    {
        toStatusMessage(str);
        return false;
    }
}

toPLSQLEditor::toPLSQLEditor(QWidget *main, toConnection &connection)
        : toToolWidget(PLSQLEditorTool, "plsqleditor.html", main, connection, "toPLSQLEditor")
{
    createActions();
    QToolBar *toolbar = toAllocBar(this, tr("PLSQLEditor"));
    layout()->addWidget(toolbar);

    toolbar->addAction(refreshAct);

    toolbar->addSeparator();

    Schema = new QComboBox(toolbar);
    Schema->setObjectName("PLSQLEditorSchemaCombo");
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
    if (!toConfigurationSingle::Instance().staticChecker().isEmpty())
    {
        toolbar->addAction(checkCodeAct);
    }

    toolbar->addSeparator();

    toolbar->addAction(nextErrorAct);
    toolbar->addAction(previousErrorAct);
    toolbar->addAction(describeAct);

    toolbar->addWidget(new toSpacer());

    splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(splitter);

    Objects = new QTreeView(splitter);
    CodeModel = new toCodeModel(Objects);
    Objects->setModel(CodeModel);
    QString selected = Schema->currentText();
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
    connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this, SLOT(windowActivated(QMdiSubWindow *)));

    QSettings s;
    s.beginGroup("toPLSQLEditor");
    splitter->restoreState(s.value("splitter").toByteArray());
    s.endGroup();

    conn = &connection;

    refresh();
}


void toPLSQLEditor::createActions(void)
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

toPLSQLWidget *toPLSQLEditor::currentEditor(void)
{
    return dynamic_cast<toPLSQLWidget *>(Editors->currentWidget());
}

void toPLSQLEditor::changeSchema(int)
{
    refresh();
}

void toPLSQLEditor::refresh(void)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    try
    {
        QString selected = Schema->currentText();
        QString currentSchema;
        if (selected.isEmpty())
        {
            selected = connection().user().toUpper();
            Schema->clear();
            toQList users = toQuery::readQuery(connection(),
                                               toSQL::string(toSQL::TOSQL_USERLIST, connection()));
            for (toQList::iterator i = users.begin(); i != users.end(); i++)
                Schema->addItem(*i);
        }
        if (!selected.isEmpty())
        {
            for (int i = 0; i < Schema->count(); i++)
            {
                if (Schema->itemText(i) == selected)
                {
                    Schema->setCurrentIndex(i);
                    break;
                }
            }

            CodeModel->refresh(connection(), selected);
        }
    }
    TOCATCH;

    QApplication::restoreOverrideCursor();
}

void toPLSQLEditor::closeEvent(QCloseEvent *e)
{
    QSettings s;
    s.beginGroup("toPLSQLEditor");
    s.setValue("splitter", splitter->saveState());
    s.endGroup();

    toToolWidget::closeEvent(e);
}

void toPLSQLEditor::updateCurrent()
{
    try
    {
        toPLSQLWidget *editor = currentEditor();

        editor->editor()->readData(connection()/*, StackTrace*/);
        editor->editor()->setFocus();
    }
    TOCATCH
}

void toPLSQLEditor::changePackage(const QModelIndex &current)
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
bool toPLSQLEditor::hasCode(const QString &pSchema, const QString &pType, const QString &pName)
{
    int numberOfLines;

    try
    {
        toQuery lines(*conn, SQLHasCode, pSchema, pName, pType);
        numberOfLines = lines.readValue().toInt();
        if (numberOfLines > 0)
            return true;
        else
            return false;
    }
    TOCATCH
    return true; // try displaying the code in case of error
} // hasCode

void toPLSQLEditor::changePackage(const QModelIndex &current, const QModelIndex &previous)
{
    toBusy busy;

//     qDebug() << "toDebug::changePackage 1";
    toCodeModelItem *item = static_cast<toCodeModelItem*>(current.internalPointer());
    if (item && item->parent())
    {
        QString ctype = item->parent()->display();
        if (ctype.isEmpty() || ctype == "Code")
            return;
        ctype = ctype.toUpper();

        viewSource(Schema->currentText(), item->display(), ctype, 0);
        if (ctype == "PACKAGE" ||
                (ctype == "TYPE" && hasCode(Schema->currentText(), ctype + " BODY", item->display())))
            viewSource(Schema->currentText(), item->display(), ctype + " BODY", 0);
    }
#ifdef AUTOEXPAND
    else if (item && !item->parent())
        item->setOpen(true);
#endif
// qDebug() << "toDebug::changePackage 2";
}

void toPLSQLEditor::compileWarn()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    currentEditor()->editor()->compile(toPLSQLText::Warning);
    QApplication::restoreOverrideCursor();
}

void toPLSQLEditor::compile(void)
{
    // compile only current editor/tab
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    currentEditor()->editor()->compile(toPLSQLText::Production);
    QApplication::restoreOverrideCursor();
//     refresh();
}

toPLSQLEditor::~toPLSQLEditor()
{
}

void toPLSQLEditor::prevError(void)
{
    currentEditor()->editor()->previousError();
}

void toPLSQLEditor::nextError(void)
{
    currentEditor()->editor()->nextError();
}

void toPLSQLEditor::newSheet(void)
{
    toPLSQLWidget *text = new toPLSQLWidget(Editors);
    if (!Schema->currentText().isEmpty())
        text->editor()->setSchema(Schema->currentText());
    else
        text->editor()->setSchema(connection().user().toUpper());
    Editors->addTab(text, tr("Unknown"));
    Editors->setCurrentIndex(Editors->indexOf(text));
}

void toPLSQLEditor::showSource(toTreeWidgetItem *item)
{
    if (item)
        viewSource(item->text(2), item->text(0), item->text(3), item->text(1).toInt(), false);
}

void toPLSQLEditor::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
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

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toPLSQLEditor::closeEditor()
{
    toPLSQLWidget *editor = currentEditor();
    closeEditor(editor);
}

void toPLSQLEditor::closeEditor(int ix)
{
    toPLSQLWidget * w = qobject_cast<toPLSQLWidget*>(Editors->widget(ix));
    assert(w);
    closeEditor(w);
}

void toPLSQLEditor::closeAllEditor()
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

void toPLSQLEditor::closeEditor(toPLSQLWidget* &editor)
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

void toPLSQLEditor::describe()
{
    QString owner, table;
    toHighlightedText * marked = currentEditor()->editor();
    marked->tableAtCursor(owner, table);
    if (owner.isNull())
        owner = Schema->currentText();
    toDescribe * d = new toDescribe(this);
    d->changeParams(owner, table);
}

/* Purpose: should find and return object containing another part of package.
   For example if specification of package A is given as parameter it should
   find toPLSQLText object containing body of the same package.
*/
toPLSQLText * toPLSQLEditor::getAnotherPart(QString &pSchema, QString &pObject, QString &pType)
{
    QString other_part_type;
    toPLSQLText * ret = NULL;

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

void toPLSQLEditor::parseResults(const QString buf, QMultiMap<int, QString> &res)
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
void toPLSQLEditor::checkCode(void)
{
    if (currentEditor()->editor()->text().isEmpty())
    {
        // do nothing if code text is empty
        return;
    }

    QTemporaryFile tf;
    if (tf.open())
    {
        if (!toWriteFile(tf.fileName(), currentEditor()->editor()->text()))
        {
#ifdef DEBUG
            qDebug() << "Unable to write file (" + tf.fileName() + ")";
#endif
            return;
        }
        else
        {
#ifdef DEBUG
            qDebug() << "Success!!! Temporary file " + tf.fileName();
#endif
        }
    }

    QString program = toConfigurationSingle::Instance().staticChecker().arg(tf.fileName());
#ifdef DEBUG
    qDebug() << "program to be executed: " + program;
#endif
    QProcess staticCheck(qApp);

    staticCheck.setProcessChannelMode(QProcess::MergedChannels);
    staticCheck.start(program);
    staticCheck.waitForFinished(); // default timeout - 30000 miliseconds

    int exit_code = staticCheck.exitStatus();
    if (exit_code != 0)
    {
#ifdef DEBUG
        qDebug() << "Error executing static check. Exit code = " << exit_code;
        int run_error = staticCheck.error();
        // error values taken from Qt4.6 documentation for QProcess
        switch (run_error)
        {
        case 0:
            qDebug() << "The process failed to start. Either the invoked program is missing, or you may have insufficient permissions to invoke the program.";
            break;
        case 1:
            qDebug() << "The process crashed some time after starting successfully.";
            break;
        case 5:
            qDebug() << "An unknown error occurred.";
            break;
        default:
            qDebug() << "Error code: " << run_error << "--" << staticCheck.errorString();
        } // switch
#endif
        return;
    }
    QString qq = staticCheck.readAllStandardOutput();
#ifdef DEBUG
    qDebug() << "stdout" << qq;
#endif

    QMultiMap<int, QString> Observations;
    parseResults(qq, Observations);
    currentEditor()->editor()->setErrors(Observations, false);
    currentEditor()->applyResult("STATIC", Observations);
    currentEditor()->resizeResults();
} // checkCode

