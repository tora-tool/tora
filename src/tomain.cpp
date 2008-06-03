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

#include "toabout.h"
#include "tobackgroundlabel.h"
#include "toconf.h"
#include "toconnection.h"
#include "toeditwidget.h"
#include "tohelp.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "tomemoeditor.h"
#include "ui_tomessageui.h"
#include "tonewconnection.h"
#include "topreferences.h"
#include "tosearchreplace.h"
#include "totemplate.h"
#include "totool.h"
#include "tomessage.h"

#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <QToolBar>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <QMdiArea>
#include <QMdiSubWindow>

#include <qstyle.h>
#include <QPixmap>

#include "icons/connect.xpm"
#include "icons/copy.xpm"
#include "icons/cut.xpm"
#include "icons/disconnect.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/paste.xpm"
#include "icons/print.xpm"
#include "icons/redo.xpm"
#include "icons/search.xpm"
#include "icons/tora.xpm"
#include "icons/undo.xpm"
#include "icons/up.xpm"
#include "icons/commit.xpm"
#include "icons/rollback.xpm"
#include "icons/stop.xpm"
#include "icons/refresh.xpm"

#define DEFAULT_TITLE TOAPPNAME " %s"

toMain::toMain()
        : toMainWindow(),
        toBackupTool_(new toBackupTool),
        BackgroundLabel(new toBackgroundLabel(statusBar()))
{

    Edit = NULL;

    Workspace = new QMdiArea(this);
    LastActiveWindow = 0;

    Message = new toMessage(this);

    // setup all QAction objects
    createActions();

    // create all menus
    createMenus();

    createToolbars();

    createStatusbar();

    createToolMenus();

    handleToolsDisplay();

    updateRecent();

    char buffer[100];
    sprintf(buffer, DEFAULT_TITLE, TOVERSION);
    setWindowTitle(tr(buffer));

    setWindowIcon(QPixmap(const_cast<const char**>(tora_xpm)));

    restoreGeometry(toConfigurationSingle::Instance().mainWindowGeometry());
    restoreState(toConfigurationSingle::Instance().mainWindowState());

    // disable widgets related to an editor
    editDisable(NULL);

    enableConnectionActions(false);

    std::map<QString, toTool *> &tools = toTool::tools();

    QString defName(toConfigurationSingle::Instance().defaultTool());
    DefaultTool = NULL;
    for (std::map<QString, toTool *>::iterator k = tools.begin();
            k != tools.end();
            k++)
    {
        if(defName.isEmpty() || defName == (*k).first) {
            DefaultTool = (*k).second;
            defName = (*k).first;
        }

        (*k).second->customSetup();
    }
    Search = NULL;

    QString welcome;

    connect(&Poll, SIGNAL(timeout()), this, SLOT(checkCaching()));
    connect(toMainWidget()->workspace(),
            SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this,
            SLOT(windowActivated(QMdiSubWindow *)));
    connect(this,
            SIGNAL(messageRequested(const QString &, bool, bool)),
            this,
            SLOT(showMessageImpl(const QString &, bool, bool)),
            Qt::QueuedConnection);

    if (toConfigurationSingle::Instance().restoreSession())
    {
        try
        {
            std::map<QString, QString> session;
            toConfigurationSingle::Instance().loadMap(
                toConfigurationSingle::Instance().defaultSession(), session);
            importData(session, "TOra");
        }
        TOCATCH;
    }

//     if (toConfigurationSingle::Instance().maximizeMain() && Connections.empty())
//         showMaximized();
//     else
//         show();
    setCentralWidget(Workspace);

    show();

    if (Connections.empty())
    {
        try
        {
            toConnection *conn;

            do
            {
                toNewConnection newConnection(this);

                conn = NULL;
                if (newConnection.exec())
                    conn = newConnection.connection();
                else
                    break;
            }
            while (!conn);

            if (conn)
                addConnection(conn);
        }
        TOCATCH;
    }

    statusBar()->addPermanentWidget(BackgroundLabel, 0);
    BackgroundLabel->show();
    BackgroundLabel->setToolTip(tr("No background queries."));
}

void toMain::createActions()
{
    newConnAct = new QAction(QPixmap(const_cast<const char**>(connect_xpm)),
                             tr("&New Connection..."),
                             this);
    newConnAct->setShortcut(Qt::CTRL + Qt::Key_G);
    newConnAct->setToolTip(tr("Create a new connection"));
    connect(newConnAct,
            SIGNAL(triggered()),
            this,
            SLOT(addConnection()),
            Qt::QueuedConnection);

    closeConn = new QAction(QPixmap(const_cast<const char**>(disconnect_xpm)),
                            tr("&Close Connection"),
                            this);
    closeConn->setToolTip(tr("Disconnect"));
    connect(closeConn,
            SIGNAL(triggered()),
            this,
            SLOT(delConnection()),
            Qt::QueuedConnection);

    commitAct = new QAction(QPixmap(const_cast<const char**>(commit_xpm)),
                            tr("&Commit Connection"),
                            this);
    commitAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_C);
    commitAct->setToolTip(tr("Commit transaction"));

    rollbackAct = new QAction(QPixmap(const_cast<const char**>(rollback_xpm)),
                              tr("&Rollback Connection"),
                              this);
    rollbackAct->setShortcut(Qt::CTRL + Qt::Key_Less);
    rollbackAct->setToolTip(tr("Rollback transaction"));

    currentAct = new QAction(tr("&Current Connection"),
                             this);
    currentAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_U);

    stopAct = new QAction(QPixmap(const_cast<const char**>(stop_xpm)),
                          tr("Stop All Queries"),
                          this);
    stopAct->setShortcut(Qt::CTRL + Qt::Key_J);

    refreshAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                             tr("Reread Object Cache"),
                             this);
    refreshAct->setShortcut(Qt::CTRL + Qt::Key_R);

    openAct = new QAction(QPixmap(const_cast<const char**>(fileopen_xpm)),
                          tr("&Open File..."),
                          this);
    openAct->setShortcut(QKeySequence::Open);

    saveAct = new QAction(QPixmap(const_cast<const char**>(filesave_xpm)),
                          tr("&Save File..."),
                          this);
    saveAct->setShortcut(QKeySequence::Save);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_W);

    openSessionAct = new QAction(QPixmap(const_cast<const char**>(fileopen_xpm)),
                                 tr("Open Session..."),
                                 this);

    saveSessionAct = new QAction(QPixmap(const_cast<const char**>(filesave_xpm)),
                                 tr("Save Session..."),
                                 this);

    restoreSessionAct = new QAction(tr("Restore Last Session"), this);

    closeSessionAct = new QAction(tr("Close Session"), this);

    printAct = new QAction(QPixmap(const_cast<const char**>(print_xpm)),
                           tr("&Print..."),
                           this);
    saveAsAct->setShortcut(QKeySequence::Print);

    quitAct = new QAction(tr("&Quit"), this);

    // ---------------------------------------- edit menu

    undoAct = new QAction(QPixmap(const_cast<const char**>(undo_xpm)),
                          tr("&Undo"),
                          this);
    undoAct->setShortcut(QKeySequence::Undo);

    redoAct = new QAction(QPixmap(const_cast<const char**>(redo_xpm)),
                          tr("&Redo"),
                          this);
    redoAct->setShortcut(QKeySequence::Redo);

    cutAct = new QAction(QPixmap(const_cast<const char**>(cut_xpm)),
                         tr("Cu&t"),
                         this);
    cutAct->setShortcut(QKeySequence::Cut);
    cutAct->setToolTip(tr("Cut to clipboard"));

    copyAct = new QAction(QPixmap(const_cast<const char**>(copy_xpm)),
                          tr("&Copy"),
                          this);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setToolTip(tr("Copy to clipboard"));

    pasteAct = new QAction(QPixmap(const_cast<const char**>(paste_xpm)),
                           tr("&Paste"),
                           this);
    pasteAct->setShortcut(QKeySequence::Paste);
    pasteAct->setToolTip(tr("Paste from clipboard"));

    searchReplaceAct = new QAction(QPixmap(const_cast<const char**>(search_xpm)),
                                   tr("&Search && Replace..."),
                                   this);
    searchReplaceAct->setShortcut(QKeySequence::Find);
    searchReplaceAct->setToolTip(tr("Search & replace"));

    searchNextAct = new QAction(tr("Search &Next"), this);
    searchNextAct->setShortcut(QKeySequence::FindNext);

    selectAllAct = new QAction(tr("Select &All"), this);
    selectAllAct->setShortcut(QKeySequence::SelectAll);

    readAllAct = new QAction(tr("Read All &Items"), this);

    prefsAct = new QAction(tr("&Preferences..."), this);

    // ---------------------------------------- help menu

    helpCurrentAct = new QAction(tr("C&urrent Context..."), this);
    helpCurrentAct->setShortcut(QKeySequence::HelpContents);

    helpContentsAct = new QAction(tr("&Contents..."), this);

    aboutAct = new QAction(tr("&About " TOAPPNAME "..."), this);

    licenseAct = new QAction(tr("&License..."), this);

    // ---------------------------------------- windows menu

    windowCloseAct = new QAction(tr("C&lose"), this);

    windowCloseAllAct = new QAction(tr("Close &All"), this);

    cascadeAct = new QAction(tr("&Cascade"), this);

    tileAct = new QAction(tr("&Tile"), this);
}


void toMain::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newConnAct);
    fileMenu->addAction(closeConn);
    fileMenu->addSeparator();

    fileMenu->addAction(commitAct);
    fileMenu->addAction(rollbackAct);
    fileMenu->addAction(currentAct);
    fileMenu->addAction(stopAct);
    fileMenu->addAction(refreshAct);
    fileMenu->addSeparator();

    fileMenu->addAction(openAct);
    // add recentMenu after, setup later
    recentMenu = fileMenu->addMenu(tr("R&ecent Files"));
    fileMenu->addMenu(recentMenu);

    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();

    fileMenu->addAction(openSessionAct);
    fileMenu->addAction(saveSessionAct);
    fileMenu->addAction(restoreSessionAct);
    fileMenu->addAction(closeSessionAct);
    fileMenu->addSeparator();

    fileMenu->addAction(printAct);
    fileMenu->addSeparator();

    fileMenu->addAction(quitAct);

    connect(fileMenu, SIGNAL(aboutToShow()), this, SLOT(showFileMenu()));
    connect(fileMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(commandCallback(QAction *)),
            Qt::QueuedConnection);

    connect(recentMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(recentCallback(QAction *)),
            Qt::QueuedConnection);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();

    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();

    editMenu->addAction(searchReplaceAct);
    editMenu->addAction(searchNextAct);
    editMenu->addAction(selectAllAct);
    editMenu->addAction(readAllAct);
    editMenu->addSeparator();

    editMenu->addAction(prefsAct);
    connect(editMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(commandCallback(QAction *)),
            Qt::QueuedConnection);

    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    connect(toolsMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(commandCallback(QAction *)),
            Qt::QueuedConnection);

    // windows menu handled separately by update function
    windowsMenu = menuBar()->addMenu(tr("&Window"));
    updateWindowsMenu();
    connect(windowsMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(windowCallback(QAction *)),
            Qt::QueuedConnection);

    connect(windowsMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowsMenu()));
    connect(windowsMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(commandCallback(QAction *)),
            Qt::QueuedConnection);

    helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(helpCurrentAct);
    helpMenu->addAction(helpContentsAct);
    windowsMenu->addSeparator();
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(licenseAct);

    connect(helpMenu,
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(commandCallback(QAction *)),
            Qt::QueuedConnection);
}


void toMain::addCustomMenu(QMenu *menu)
{
    this->menuBar()->insertMenu(windowsMenu->menuAction(), menu);
}


void toMain::createToolbars()
{
    editToolbar = toAllocBar(this, tr("Application"));
    editToolbar->setObjectName("editToolbar");

    editToolbar->addAction(openAct);
    editToolbar->addAction(saveAct);
    editToolbar->addAction(printAct);
    editToolbar->addSeparator();

    editToolbar->addAction(undoAct);
    editToolbar->addAction(redoAct);
    editToolbar->addAction(cutAct);
    editToolbar->addAction(copyAct);
    editToolbar->addAction(pasteAct);
    editToolbar->addSeparator();

    editToolbar->addAction(searchReplaceAct);

    connectionToolbar = toAllocBar(this, tr("Connections"));
    connectionToolbar->setObjectName("connectionToolbar");

    connectionToolbar->addAction(newConnAct);
    connectionToolbar->addAction(closeConn);
    connectionToolbar->addAction(commitAct);
    connectionToolbar->addAction(rollbackAct);
    connectionToolbar->addSeparator();

    connectionToolbar->addAction(stopAct);
    connectionToolbar->addSeparator();

    ConnectionSelection = new QComboBox(connectionToolbar);
    ConnectionSelection->setMinimumWidth(300);
    ConnectionSelection->setFocusPolicy(Qt::NoFocus);
    connectionToolbar->addWidget(ConnectionSelection);
    connect(ConnectionSelection,
            SIGNAL(activated(int)),
            this,
            SLOT(changeConnection()));

    addToolBarBreak();

    toolsToolbar = toAllocBar(this, tr("Tools"));
    toolsToolbar->setObjectName("toolsToolbar");

    addToolBarBreak();
}


void toMain::addButtonApplication(QAction *act)
{
    editToolbar->addAction(act);
}


void toMain::createStatusbar()
{
    statusBar()->showMessage(QString::null);

    RowLabel = new QLabel(statusBar());
    statusBar()->addPermanentWidget(RowLabel, 0);
    RowLabel->setMinimumWidth(60);
    //  RowLabel->hide();

    ColumnLabel = new QLabel(statusBar());
    statusBar()->addPermanentWidget(ColumnLabel, 0);
    ColumnLabel->setMinimumWidth(60);
    //  ColumnLabel->hide();

    QToolButton *dispStatus = new toPopupButton(statusBar());
    dispStatus->setIcon(QPixmap(const_cast<const char**>(up_xpm)));
    statusBar()->addPermanentWidget(dispStatus, 0);
    statusMenu = new QMenu(dispStatus);
    dispStatus->setMenu(statusMenu);
    dispStatus->setPopupMode(QToolButton::MenuButtonPopup);
    connect(statusMenu,
            SIGNAL(aboutToShow()),
            this,
            SLOT(updateStatusMenu()));
    connect(statusMenu,
            SIGNAL(triggered(QAction*)),
            this,
            SLOT(statusCallback(QAction*)));
    connect(dispStatus,
            SIGNAL(pressed()),
            dispStatus,
            SLOT(showMenu()));
}


void toMain::createToolMenus()
{
    try
    {
        int lastPriorityPix = 0;
        int lastPriorityMenu = 0;

        std::map<QString, toTool *> &tools = toTool::tools();
        ToolsMap cfgTools(toConfigurationSingle::Instance().tools());

        for (std::map<QString, toTool *>::iterator i = tools.begin();
                i != tools.end();
                i++)
        {

            QAction *toolAct = (*i).second->getAction();
            const QPixmap *pixmap = (*i).second->toolbarImage();
            const char *menuName = (*i).second->menuItem();

//             QString tmp = (*i).first;
//             tmp += CONF_TOOL_ENABLE;
//             if(toConfigurationSingle::Instance().globalConfig(
//                    tmp, "Yes").isEmpty()) {
//                 continue;
//             }
            // set the tools for the first run
            if (!cfgTools.contains((*i).first))
                cfgTools[(*i).first] = true;
            // only enabled tools are set
            if (cfgTools[(*i).first] == false)
                continue;

            int priority = (*i).second->priority();
            if (priority / 100 != lastPriorityPix / 100 && pixmap)
            {
                toolsToolbar->addSeparator();
                lastPriorityPix = priority;
            }

            if (priority / 100 != lastPriorityMenu / 100 && menuName)
            {
                toolsMenu->addSeparator();
                lastPriorityMenu = priority;
            }

            if (pixmap)
                toolsToolbar->addAction(toolAct);

            if (menuName)
                toolsMenu->addAction(toolAct);
        } // for tools
        toConfigurationSingle::Instance().setTools(cfgTools);
    }
    TOCATCH;
}

void toMain::handleToolsDisplay()
{
#if QT_VERSION >= 0x040400
    if (toConfigurationSingle::Instance().tabbedTools())
        Workspace->setViewMode(QMdiArea::TabbedView);
    else
        Workspace->setViewMode(QMdiArea::SubWindowView);
#else
#warning "Use Qt 4.4.x if you want to use cool stuff."
#endif
}

void toMain::windowActivated(QMdiSubWindow *widget)
{
    if(widget)
        LastActiveWindow = widget;

    if (!toConfigurationSingle::Instance().changeConnection())
        return ;

    if (!widget)
        return;

    toToolWidget *tool = dynamic_cast<toToolWidget *>(widget->widget());

    if (tool)
    {
        try
        {
            toConnection &conn = tool->connection();
            int pos = 0;
            for (std::list<toConnection *>::iterator i = Connections.begin();i != Connections.end();i++)
            {
                if (&conn == *i)
                {
                    ConnectionSelection->setCurrentIndex(pos);
                    changeConnection();
                    break;
                }
                pos++;
            }
        }
        TOCATCH
    }
}


void toMain::showFileMenu(void)
{
    bool hascon = (ConnectionSelection->count() > 0);

    commitAct->setEnabled(hascon);
    stopAct->setEnabled(hascon);
    rollbackAct->setEnabled(hascon);
    refreshAct->setEnabled(hascon);
    closeConn->setEnabled(hascon);

    updateRecent();
}

void toMain::updateRecent()
{
//     static bool first = true;
//     int num = toConfigurationSingle::Instance().recentFiles();
//
//     recentMenu->clear();
//
//     if (num > 0) {
//         if(first) {
//             fileMenu->addSeparator();
//             first = false;
//         }
//
//         for(int i = 0; i < num; i++) {
//             QString file = toConfigurationSingle::Instance().globalConfig(
//                 QString(CONF_RECENT_FILES ":") +
//                 QString::number(i), "");
//
//             if(!file.isEmpty()) {
//                 QFileInfo fi(file);
//
//                 // store file name in tooltip. this is used later to
//                 // open the file, and is handy to know what file tora
//                 // is opening.
//
//                 QAction *r = new QAction(fi.fileName(), this);
//                 r->setToolTip(file);
//                 recentMenu->addAction(r);
//             }
//         }
//     }
    QStringList files(toConfigurationSingle::Instance().recentFiles());
    recentMenu->clear();
    if (files.count() > 0)
        recentMenu->addSeparator();
    foreach (QString s, files)
    {
        QFileInfo fi(s);
        if (!fi.exists())
        {
            files.removeAt(files.indexOf(s));
            continue;
        }
        // store file name in tooltip. this is used later to
        // open the file, and is handy to know what file tora
        // is opening.
        QAction *r = new QAction(fi.fileName(), this);
        r->setToolTip(s);
        recentMenu->addAction(r);
    }
    toConfigurationSingle::Instance().setRecentFiles(files);
}


void toMain::addRecentFile(const QString &file)
{
    QStringList files(toConfigurationSingle::Instance().recentFiles());
    int maxnum = toConfigurationSingle::Instance().recentMax();

    if (files.contains(file))
        return;
    if (files.count() > maxnum)
        files.removeAt(0);
    files.append(file);
    toConfigurationSingle::Instance().setRecentFiles(files);
//     std::list<QString> files;
//     for (int j = 0;j < num;j++)
//     {
//         QString t = toConfigurationSingle::Instance().globalConfig(
//             QString(CONF_RECENT_FILES ":") + QString::number(j), "");
//         if (t != file)
//             toPush(files, t);
//     }
//     toUnShift(files, file);

//     num = 0;
//     for (std::list<QString>::iterator i = files.begin();i != files.end();i++)
//     {
//         toConfigurationSingle::Instance().globalSetConfig(
//             QString(CONF_RECENT_FILES ":") + QString::number(num), *i);
//         num++;
//         if (num >= maxnum)
//             break;
//     }
//     toConfigurationSingle::Instance().globalSetConfig(CONF_RECENT_FILES, QString::number(num));
//     toConfigurationSingle::Instance().saveConfig();
}

void toMain::updateWindowsMenu(void)
{
    // i'm lazy and this beats the hell out of tracking all the
    // windowsMenu actions and adding/removing each.
    windowsMenu->clear();

    windowCloseAct->setEnabled(LastActiveWindow != 0);
    windowCloseAllAct->setEnabled(LastActiveWindow != 0);
    cascadeAct->setEnabled(LastActiveWindow != 0);
    tileAct->setEnabled(LastActiveWindow != 0);

    windowsMenu->addAction(windowCloseAct);
    windowsMenu->addAction(windowCloseAllAct);
    windowsMenu->addSeparator();
    windowsMenu->addAction(cascadeAct);
    windowsMenu->addAction(tileAct);
    windowsMenu->addSeparator();

    int index = 0;
    QList<QMdiSubWindow *> list = workspace()->subWindowList();

    for (QList<QMdiSubWindow *>::iterator it = list.begin(); it != list.end(); it++, index++)
    {
        if (!(*it)->isHidden())
        {
            QString caption = (*it)->windowTitle().trimmed();

            if (index < 9)
                caption = "&" + QString::number(index + 1) + " " + caption;

            QAction *action = new QAction(caption, (*it));
            if (index < 9)
                action->setShortcut(Qt::CTRL + Qt::Key_1 + index);

            windowsMenu->addAction(action);
            action->setCheckable(true);
            if ((*it) == LastActiveWindow)
                action->setChecked(true);
        }
    }
}


void toMain::windowCallback(QAction *action)
{
    // action's parent is the window widget. get parent and raise it.

    if (action == NULL || action->parentWidget() == NULL)
        return;

    if (action == windowCloseAllAct)
    {
        while (workspace()->subWindowList().count() > 0 &&
               workspace()->subWindowList().at(0))
            if (workspace()->subWindowList().at(0) &&
                !workspace()->subWindowList().at(0)->close())
                return;
    }
    else if (action == windowCloseAct)
    {
        QWidget *widget = LastActiveWindow;
        if (widget)
            widget->close();
    }
    else {
        QMdiSubWindow *w = dynamic_cast<QMdiSubWindow *>(action->parentWidget());
        if(w) {
            workspace()->setActiveSubWindow(w);
            w->raise();
            w->setFocus();
            // piece of shit mdi doesn't always send window activated
            // signal
            windowActivated(w);
        }
    }
}


void toMain::recentCallback(QAction *action)
{
    if (!action)
        return;

    toEditWidget *edit = NULL;
    QWidget *currWidget = qApp->focusWidget();
    while (currWidget && !edit)
    {
        edit = dynamic_cast<toEditWidget *>(currWidget);
        currWidget = currWidget->parentWidget();
    }

    if (edit)
        edit->editOpen(action->toolTip());
}


void toMain::statusCallback(QAction *action)
{
    new toMemoEditor(this, action->toolTip());
}


void toMain::commandCallback(QAction *action)
{
    QWidget *focus = qApp->focusWidget();

    if (focus)
    {
        toEditWidget *edit = findEdit(focus);
        if (edit && edit != Edit)
            setEditWidget(edit);
        else if (focus->inherits("QLineEdit") ||
                 QString(focus->metaObject()->className()) == QString("QSpinBox"))
            editEnable(edit);
    }

    QWidget *currWidget = qApp->focusWidget();
    toEditWidget *edit = NULL;
    while (currWidget && !edit)
    {
        edit = dynamic_cast<toEditWidget *>(currWidget);
        currWidget = currWidget->parentWidget();
    }

    if (edit)
    {
        if (action == redoAct)
            edit->editRedo();
        else if (action == undoAct)
            edit->editUndo();
        else if (action == copyAct)
            edit->editCopy();
        else if (action == pasteAct)
            edit->editPaste();
        else if (action == cutAct)
            edit->editCut();
        else if (action == selectAllAct)
            edit->editSelectAll();
        else if (action == refreshAct)
            edit->editReadAll();
        else if (action == searchReplaceAct)
        {
            if (!Search)
                Search = new toSearchReplace(this);
            Search->show();
        }
        else if (action == searchNextAct)
        {
            if (Search)
                Search->searchNext();
            else
                searchReplaceAct->activate(QAction::Trigger);
        }
        else if (action == openAct)
            edit->editOpen();
        else if (action == saveAsAct)
            edit->editSave(true);
        else if (action == saveAct)
            edit->editSave(false);
        else if (action == printAct)
            edit->editPrint();
    } // if edit

    if (action == commitAct)
    {
        try
        {
            toConnection &conn = currentConnection();
            emit willCommit(conn, true);
            conn.commit();
            setNeedCommit(conn, false);
        }
        TOCATCH;
    }
    else if (action == stopAct)
    {
        try
        {
            toConnection &conn = currentConnection();
            conn.cancelAll();
        }
        TOCATCH;
    }
    else if (action == refreshAct)
    {
        try
        {
            currentConnection().rereadCache();
        }
        TOCATCH;
        toMainWidget()->checkCaching();
    }
    else if (action == rollbackAct)
    {
        try
        {
            toConnection &conn = currentConnection();
            emit willCommit(conn, false);
            conn.rollback();
            setNeedCommit(conn, false);
        }
        TOCATCH;
    }
    else if (action == currentAct)
        ConnectionSelection->setFocus();
    else if (action == quitAct)
        close();
    else if (action == searchReplaceAct)
    {
        if (Search)
            Search->searchNext();
    }
    else if (action == cascadeAct)
        workspace()->cascadeSubWindows();
    else if (action == tileAct)
        workspace()->tileSubWindows();
    else if (action == helpCurrentAct)
        toHelp::displayHelp();
    else if (action == helpContentsAct)
        toHelp::displayHelp(QString::fromLatin1("toc.html"));
    else if (action == aboutAct)
    {
        toAbout about(toAbout::About, this, "About " TOAPPNAME, true);
        about.exec();
    }
    else if (action == licenseAct)
    {
        toAbout about(toAbout::License, this, "About " TOAPPNAME, true);
        about.exec();
    }
    else if (action == prefsAct)
    {
        toPreferences::displayPreferences(this);
        handleToolsDisplay();
    }
    else if (action == openSessionAct)
        loadSession();
    else if (action == saveSessionAct)
        saveSession();
    else if (action == restoreSessionAct)
    {
        try
        {
            std::map<QString, QString> session;
            toConfigurationSingle::Instance().loadMap(
                toConfigurationSingle::Instance().defaultSession(), session);
            importData(session, "TOra");
        }
        TOCATCH;
    }
    else if (action == closeSessionAct)
        closeSession();
}

void toMain::addConnection(void)
{
    try
    {
        toNewConnection newConnection(this);

        toConnection *conn = NULL;

        if (newConnection.exec())
            conn = newConnection.connection();

        if (conn)
            addConnection(conn);
    }
    TOCATCH
}

toConnection &toMain::currentConnection()
{
    for (std::list<toConnection *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        if (ConnectionSelection->currentText().startsWith((*i)->description()))
        {
            return *(*i);
        }
    }
    throw tr("Can't find active connection");
}

toConnection *toMain::addConnection(toConnection *conn, bool def)
{
    int j = 0;
    for (std::list<toConnection *>::iterator i = Connections.begin();i != Connections.end();i++, j++)
    {
        if ((*i)->description() == conn->description())
        {
            ConnectionSelection->setCurrentIndex(j);
            if (def)
                createDefault();
            return *i;
        }
    }

    Connections.insert(Connections.end(), conn);
    ConnectionSelection->addItem(conn->description());
    ConnectionSelection->setCurrentIndex(ConnectionSelection->count() - 1);

    if (ConnectionSelection->count() == 1)
        enableConnectionActions(true);

    checkCaching();

    changeConnection();
    emit addedConnection(conn->description());

    if (def)
        createDefault();

    return conn;
}

void toMain::setNeedCommit(toConnection &conn, bool needCommit)
{
    int pos = 0;
    for (std::list<toConnection *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        if (conn.description() == (*i)->description())
        {
            QString dsc = conn.description();
            if (needCommit)
                dsc += QString::fromLatin1(" *");
            ConnectionSelection->setCurrentIndex(pos);
            ConnectionSelection->setItemText(pos, dsc);
            break;
        }
        pos++;
    }
    conn.setNeedCommit(needCommit);
}

bool toMain::delConnection(void)
{
    toConnection *conn = NULL;
    int pos = 0;

    for (std::list<toConnection *>::iterator i = Connections.begin();
            i != Connections.end();
            i++)
    {

        if (ConnectionSelection->currentText().startsWith((*i)->description()))
        {
            conn = (*i);

            if (conn->needCommit())
            {
                QString str = tr("Commit work in session to %1 before "
                                 "closing it?").arg(conn->description());
                switch (TOMessageBox::warning(this,
                                              tr("Commit work?"),
                                              str,
                                              tr("&Yes"),
                                              tr("&No"),
                                              tr("Cancel")))
                {
                case 0:
                    conn->commit();
                    break;
                case 1:
                    conn->rollback();
                    break;
                case 2:
                    return false;
                }
            }

            if (!conn->closeWidgets())
                return false;

            emit removedConnection(conn->description());
            Connections.erase(i);
            ConnectionSelection->removeItem(pos);
            if (ConnectionSelection->count())
                ConnectionSelection->setCurrentIndex(std::max(pos - 1, 0));
            delete conn;
            break;
        }
        pos++;
    }

    if (ConnectionSelection->count() == 0)
        enableConnectionActions(false);
    else
        changeConnection();

    return true;
}

std::list<QString> toMain::connections(void)
{
    std::list<QString> ret;
    for (std::list<toConnection *>::iterator i = Connections.begin();i != Connections.end();i++)
        toPush(ret, (*i)->description());
    return ret;
}

toConnection &toMain::connection(const QString &str)
{
    for (std::list<toConnection *>::iterator i = Connections.begin();i != Connections.end();i++)
        if ((*i)->description() == str)
            return *(*i);
    throw tr("Couldn't find specified connectionts (%1)").arg(str);
}

void toMain::setEditWidget(toEditWidget *edit)
{
    toMain *main = toMainWidget();
    if (main && edit)
    {
        if (main->Edit)
            main->Edit->lostFocus();
        main->Edit = edit;
        main->RowLabel->setText(QString::null);
        main->ColumnLabel->setText(QString::null);

        main->editEnable(edit);
    }
}


void toMain::editEnable(toEditWidget *edit)
{
    if (!edit)
        return;

    toMain *main = toMainWidget();
    if (main)
        main->editEnable(edit,
                         edit->openEnabled(),
                         edit->saveEnabled(),
                         edit->printEnabled(),
                         edit->undoEnabled(),
                         edit->redoEnabled(),
                         edit->cutEnabled(),
                         edit->copyEnabled(),
                         edit->pasteEnabled(),
                         edit->searchEnabled(),
                         edit->selectAllEnabled(),
                         edit->readAllEnabled()
                        );

    // Set Selection Mode on X11
    // qt4 TODO
    // no idea what the docs are asking me to do here. the method was obsoleted in qt3
//     QClipboard *clip = qApp->clipboard();
//     if (clip->supportsSelection())
//         clip->setSelectionMode(true);
}

void toMain::editDisable(toEditWidget *edit)
{
    toMain *main = toMainWidget();

    if (main)
    {
        main->editEnable(edit,
                         false,
                         false,
                         false,
                         false,
                         false,
                         false,
                         false,
                         false,
                         false,
                         false,
                         false);

        if (edit && edit == main->Edit)
        {
            main->Edit->lostFocus();
            main->Edit = NULL;
        }
    }
}


toEditWidget *toMain::findEdit(QWidget *widget)
{
    while (widget)
    {
        toEditWidget *edit = dynamic_cast<toEditWidget *>(widget);
        if (edit)
            return edit;
        widget = widget->parentWidget();
    }
    return NULL;
}


void toMain::editEnable(toEditWidget *edit,
                        bool open,
                        bool save,
                        bool print,
                        bool undo,
                        bool redo,
                        bool cut,
                        bool copy,
                        bool paste,
                        bool search,
                        bool selectAll,
                        bool readAll)
{

    if (!edit)
    {
        openAct->setEnabled(false);
        recentMenu->setEnabled(false);
        saveAct->setEnabled(false);
        saveAsAct->setEnabled(false);
        printAct->setEnabled(false);

        undoAct->setEnabled(false);
        redoAct->setEnabled(false);

        cutAct->setEnabled(false);
        copyAct->setEnabled(false);
        pasteAct->setEnabled(false);
        searchReplaceAct->setEnabled(false);
        searchNextAct->setEnabled(false);
        selectAllAct->setEnabled(false);
        readAllAct->setEnabled(false);

        emit editEnabled(false);
    }
    else if (edit && edit == Edit)
    {
        openAct->setEnabled(open);
        recentMenu->setEnabled(open);
        saveAct->setEnabled(save);
        saveAsAct->setEnabled(save);
        printAct->setEnabled(print);

        undoAct->setEnabled(undo);
        redoAct->setEnabled(redo);

        cutAct->setEnabled(cut);
        copyAct->setEnabled(copy);
        pasteAct->setEnabled(paste);
        searchReplaceAct->setEnabled(search);
        searchNextAct->setEnabled(search);
        selectAllAct->setEnabled(search);
        readAllAct->setEnabled(readAll);

        emit editEnabled(open);
    }
}


void toMain::enableConnectionActions(bool enabled)
{
    commitAct->setEnabled(enabled);
    rollbackAct->setEnabled(enabled);
    stopAct->setEnabled(enabled);
    closeConn->setEnabled(enabled);
    refreshAct->setEnabled(enabled);

    // now, loop through tools and enable/disable

    std::map<QString, toTool *> &tools = toTool::tools();
    for (std::map<QString, toTool *>::iterator i = tools.begin();
            i != tools.end();
            i++)
    {

        if (!(*i).second)
            continue;

        if (!enabled)
            (*i).second->enableAction(false);
        else
        {
            toConnection &conn = currentConnection();
            (*i).second->enableAction(conn);
        }
    }
}


void toMain::registerSQLEditor(const QString &name)
{
    SQLEditor = name;
}


void toMain::closeEvent(QCloseEvent *event)
{
    while (Connections.end() != Connections.begin())
    {
        if (!delConnection())
        {
            event->ignore();
            return;
        }
    }

    Workspace->closeAllSubWindows();
    if (Workspace->currentSubWindow() != 0)
    {
        event->ignore();        // stop widget refused
        return;
    }

    std::map<QString, QString> session;
    exportData(session, "TOra");
    try
    {
        toConfigurationSingle::Instance().saveMap(
            toConfigurationSingle::Instance().defaultSession(),
            session);
    }
    TOCATCH;

    toConfigurationSingle::Instance().setMainWindowGeometry(saveGeometry());
    toConfigurationSingle::Instance().setMainWindowState(saveState());

    toConfigurationSingle::Instance().saveConfig();
    event->accept();
}


bool toMain::close()
{
    return QMainWindow::close();
}

void toMain::createDefault(void)
{
    if (DefaultTool)
        DefaultTool->createWindow();
}

void toMain::setCoordinates(int line, int col)
{
    QString str = tr("Row:") + " ";
    str += QString::number(line);
    RowLabel->setText(str);
    str = tr("Col:") + " ";
    str += QString::number(col);
    ColumnLabel->setText(str);
}

void toMain::editSQL(const QString &str)
{
    std::map<QString, toTool *> &tools = toTool::tools();

    if (!SQLEditor.isNull() && tools[SQLEditor])
    {
        tools[SQLEditor]->createWindow();
        emit sqlEditor(str);
    }
}

void toMain::updateStatusMenu(void)
{
    statusMenu->clear();
    for (std::list<QString>::iterator i = StatusMessages.begin();
            i != StatusMessages.end();
            i++)
    {

        QAction *s = new QAction(statusMenu);
        if ((*i).size() > 75)
            s->setText((*i).left(75) + "...");
        else
            s->setText(*i);

        s->setToolTip(*i);
        statusMenu->addAction(s);
    }
}

void toMain::changeConnection(void)
{
    enableConnectionActions(true);
}


void toMain::showMessage(const QString &str, bool save, bool log)
{
    // this function can be called from any thread.
    // this tomain class is in the main (gui) thread, so emitting
    // a signal with a queued flag will be picked up in the main
    // thread. otherwise tora crashes.
    emit messageRequested(str, save, log);
}


void toMain::showMessageImpl(const QString &str, bool save, bool log)
{
    if (!str.isEmpty())
    {
        int sec = toConfigurationSingle::Instance().statusMessage();
        if (save || sec == 0)
            statusBar()->showMessage(str.simplified());
        else
            statusBar()->showMessage(str.simplified(), sec * 1000);

        if (!save && log)
        {
            toPush(StatusMessages, str);
            if ((int) StatusMessages.size() > toConfigurationSingle::Instance().statusSave())
                toShift(StatusMessages);
            statusBar()->setToolTip(str);

            if (!toConfigurationSingle::Instance().messageStatusbar())
                displayMessage();
        }
    }
}


void toMain::checkCaching(void)
{
    int num = 0;
    for (std::list<toConnection *>::iterator i = Connections.begin();i != Connections.end();i++)
    {
        if (!(*i)->cacheAvailable(true, false, false))
            num++;
    }
    if (num == 0)
    {
        Poll.stop();
    }
    else
    {
        Poll.start(100);
    }
}

void toMain::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    try
    {
        if (isMaximized())
            data[prefix + ":State"] = QString::fromLatin1("Maximized");
        else if (isMinimized())
            data[prefix + ":State"] = QString::fromLatin1("Minimized");
        else
        {
            QRect rect = geometry();
            data[prefix + ":X"] = QString::number(rect.x());
            data[prefix + ":Y"] = QString::number(rect.y());
            data[prefix + ":Width"] = QString::number(rect.width());
            data[prefix + ":Height"] = QString::number(rect.height());
        }

        int id = 1;
        std::map<toConnection *, int> connMap;
        {
            for (std::list<toConnection *>::iterator i = Connections.begin();i != Connections.end();i++)
            {
                QString key = prefix + ":Connection:" + QString::number(id);
                if (toConfigurationSingle::Instance().savePassword())
                    data[key + ":Password"] = toObfuscate((*i)->password());
                data[key + ":User"] = (*i)->user();
                data[key + ":Host"] = (*i)->host();

                QString options;
                for (std::set
                        <QString>::const_iterator j = (*i)->options().begin();j != (*i)->options().end();j++)
                    options += "," + *j;
                data[key + ":Options"] = options.mid(1); // Strip extra , in beginning

                data[key + ":Database"] = (*i)->database();
                data[key + ":Provider"] = (*i)->provider();
                connMap[*i] = id;
                id++;
            }
        }

        id = 1;
        for (int i = 0;i < workspace()->subWindowList().count();i++)
        {
            toToolWidget *tool = dynamic_cast<toToolWidget *>(workspace()->subWindowList().at(i));

            if (tool)
            {
                QString key = prefix + ":Tools:" + QString::number(id);
                tool->exportData(data, key);
                data[key + ":Type"] = tool->tool().key();
                data[key + ":Connection"] = QString::number(connMap[&tool->connection()]);
                id++;
            }
        }

        toTemplateProvider::exportAllData(data, prefix + ":Templates");
    }
    TOCATCH
}

void toMain::importData(std::map<QString, QString> &data, const QString &prefix)
{
    if (data[prefix + ":State"] == QString::fromLatin1("Maximized"))
        showMaximized();
    else if (data[prefix + ":State"] == QString::fromLatin1("Minimized"))
        showMinimized();
    else
    {
        int width = data[prefix + ":Width"].toInt();
        if (width == 0)
        {
            TOMessageBox::warning(toMainWidget(),
                                  tr("Invalid session file"), tr("The session file is not valid, can't read it."));
            return ;
        }
        else
            setGeometry(data[prefix + ":X"].toInt(),
                        data[prefix + ":Y"].toInt(),
                        width,
                        data[prefix + ":Height"].toInt());
        showNormal();
    }

    std::map<int, toConnection *> connMap;

    int id = 1;
    std::map<QString, QString>::iterator i;
    while ((i = data.find(prefix + ":Connection:" + QString::number(id) + ":Database")) != data.end())
    {
        QString key = prefix + ":Connection:" + QString::number(id);
        QString database = (*i).second;
        QString user = data[key + ":User"];
        QString host = data[key + ":Host"];

        QStringList optionlist = data[key + ":Options"].split(",");
        std::set
        <QString> options;
        for (int j = 0;j < optionlist.count();j++)
            if (!optionlist[j].isEmpty())
                options.insert(optionlist[j]);

        QString password = toUnobfuscate(data[key + ":Password"]);
        QString provider = data[key + ":Provider"];
        bool ok = true;
        // NOTE: it looks like there was a CONF_SAVE_PWD mismatch for ages.
        // Sometimes it's used for "should I store passwords for sessions (bool),
        // here it's taken as a string. WTF?
        if (toConfigurationSingle::Instance().defaultPassword() == password)
        {
            password = QInputDialog::getText(this,
                                             tr("Input password"),
                                             tr("Enter password for %1").arg(database),
                                             QLineEdit::Password,
                                             QString::fromLatin1(DEFAULT_SAVE_PWD),
                                             &ok);
        }
        if (ok)
        {
            try
            {
                toConnection *conn = new toConnection(provider.toLatin1(), user, password, host, database, options);
                if (conn)
                {
                    conn = addConnection(conn, false);
                    connMap[id] = conn;
                }
            }
            TOCATCH
        }
        id++;
    }

    id = 1;
    while ((i = data.find(prefix + ":Tools:" + QString::number(id).toLatin1() + ":Type")) != data.end())
    {
        QString key = (*i).second.toLatin1();
        int connid = data[prefix + ":Tools:" + QString::number(id).toLatin1() + ":Connection"].toInt();
        std::map<int, toConnection *>::iterator j = connMap.find(connid);
        if (j != connMap.end())
        {
            toTool *tool = toTool::tool(key);
            if (tool)
            {
                QWidget *widget = tool->toolWindow(workspace(), *((*j).second));
                const QPixmap *icon = tool->toolbarImage();
                if (icon)
                    widget->setWindowIcon(*icon);
                widget->show();
                if (widget)
                {
                    toToolWidget *tw = dynamic_cast<toToolWidget *>(widget);
                    if (tw)
                    {
                        toToolCaption(tw, tool->name());
                        tw->importData(data, prefix + ":Tools:" + QString::number(id));
                        toolWidgetAdded(tw);
                    }
                }
            }
        }
        id++;
    }

    toTemplateProvider::importAllData(data, prefix + ":Templates");
    updateWindowsMenu();
}

void toMain::saveSession(void)
{
    QString fn = toSaveFilename(QString::null, QString::fromLatin1("*.tse"), this);
    if (!fn.isEmpty())
    {
        std::map<QString, QString> session;
        exportData(session, "TOra");
        try
        {
            toConfigurationSingle::Instance().saveMap(fn, session);
        }
        TOCATCH
    }
}

void toMain::loadSession(void)
{
    QString filename = toOpenFilename(QString::null, QString::fromLatin1("*.tse"), this);
    if (!filename.isEmpty())
    {
        try
        {
            std::map<QString, QString> session;
            toConfigurationSingle::Instance().loadMap(filename, session);
            importData(session, "TOra");
        }
        TOCATCH
    }
}

void toMain::closeSession(void)
{
    std::map<QString, QString> session;
    exportData(session, "TOra");
    try
    {
        toConfigurationSingle::Instance().saveMap(toConfigurationSingle::Instance().defaultSession(), session);
    }
    TOCATCH

    while (workspace()->subWindowList().count() > 0 && workspace()->subWindowList().at(0))
        if (workspace()->subWindowList().at(0) &&
                !workspace()->subWindowList().at(0)->close())
            return ;

    while (Connections.end() != Connections.begin())
    {
        if (!delConnection())
            return ;
    }
}

void toMain::addChart(toLineChart *chart)
{
    emit chartAdded(chart);
}

void toMain::setupChart(toLineChart *chart)
{
    emit chartSetup(chart);
}

void toMain::removeChart(toLineChart *chart)
{
    emit chartRemoved(chart);
}

void toMain::displayMessage(void)
{
    if (StatusMessages.size() < 1)
        return;

    Message->appendText(*(--StatusMessages.end()));
}

void toMain::toolWidgetAdded(toToolWidget *tool)
{
    emit addedToolWidget(tool);
}

void toMain::toolWidgetRemoved(toToolWidget *tool)
{
    emit removedToolWidget(tool);
}

toBackgroundLabel* toMain::getBackgroundLabel()
{
    return BackgroundLabel;
}

toMain* getMainWidget()
{
    QWidgetList widgets = qApp->topLevelWidgets();
    for (QWidgetList::iterator it = widgets.begin(); it != widgets.end(); it++)
    {
        toMain *main = dynamic_cast<toMain *>((*it));
        if (main)
            return main;
    }

    return NULL;
}

toMain* toMainWidget()
{
    static toMain *main = getMainWidget();
    return main;
}
