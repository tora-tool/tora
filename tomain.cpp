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
#include "toconf.h"
#include "toconnection.h"
#include "toeditwidget.h"
#include "tohelp.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "tomemoeditor.h"
#include "tomessageui.h"
#include "tonewconnection.h"
#include "topreferences.h"
#include "tosearchreplace.h"
#include "totemplate.h"
#include "totool.h"

#ifdef TO_KDE
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#endif

#include <qaccel.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvbox.h>
#include <qworkspace.h>

#if QT_VERSION >= 0x030000
#include <qurloperator.h>
#include <qnetwork.h>
#include <qstyle.h>
#endif

#include "tomain.moc"
#include "tomessageui.moc"
#ifdef TO_KDE
#include "tomainwindow.kde.moc"

#include "icons/toramini.xpm"

#else
#include "tomainwindow.moc"
#endif

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
#ifdef TOAD
#include "icons/options.xpm"
#endif

#ifndef OAS
#include "icons/commit.xpm"
#include "icons/rollback.xpm"
#include "icons/stop.xpm"
#include "icons/refresh.xpm"
#endif

#define DEFAULT_TITLE TOAPPNAME " %s"

const int toMain::TO_FILE_MENU		= 10;
const int toMain::TO_EDIT_MENU		= 20;
const int toMain::TO_TOOLS_MENU		= 30;
const int toMain::TO_WINDOWS_MENU	= 40;
const int toMain::TO_HELP_MENU		= 50;

const int toMain::TO_TOOL_MENU_ID	= 2000;
const int toMain::TO_TOOL_MENU_ID_END	= 2999;

const int toMain::TO_TOOL_ABOUT_ID	= 3000;
const int toMain::TO_TOOL_ABOUT_ID_END	= 3999;

const int toMain::TO_LAST_FILE_ID	= 5000;
const int toMain::TO_LAST_FILE_ID_END	= 5999;


#define TO_STATUS_ID		4000
#define TO_STATUS_ID_END	4999
#define TO_NEW_CONNECTION	100
#define TO_CLOSE_CONNECTION	101
#define TO_STOP_ALL		102
#define TO_FILE_OPEN		103
#define TO_FILE_SAVE		104
#define TO_FILE_SAVE_AS		105
#define TO_FILE_COMMIT		106
#define TO_FILE_ROLLBACK	107
#define TO_FILE_CURRENT         108
#define TO_FILE_CLEARCACHE	109
#define TO_FILE_PRINT		110
#define TO_FILE_QUIT		111
#define TO_FILE_OPEN_SESSION	112
#define TO_FILE_SAVE_SESSION	113
#define TO_FILE_CLOSE_SESSION	114
#define TO_FILE_LAST_SESSION	115

#define TO_EDIT_UNDO		200
#define TO_EDIT_REDO		201
#define TO_EDIT_CUT		202
#define TO_EDIT_COPY		203
#define TO_EDIT_PASTE		204
#define TO_EDIT_SELECT_ALL	205
#define TO_EDIT_OPTIONS		206
#define TO_EDIT_READ_ALL	207
#define TO_EDIT_SEARCH		208
#define TO_EDIT_SEARCH_NEXT	209  // Used by edit extension

#define TO_WINDOWS_TILE		300
#define TO_WINDOWS_CASCADE	301
#define TO_WINDOWS_CLOSE	302
#define TO_WINDOWS_CLOSE_ALL	303
#define TO_WINDOWS_WINDOWS	310
#define TO_WINDOWS_END		399

#define TO_HELP_CONTENTS	900
#define TO_HELP_CONTEXT		901
#define TO_HELP_ABOUT		902
#define TO_HELP_LICENSE		903
#define TO_HELP_QUOTES		904

#define TO_TOOLS		1000
#define TO_ABOUT_ID_OFFSET	(toMain::TO_TOOL_ABOUT_ID-TO_TOOLS)

toMain::toMain()
  : toMainWindow()
{
  qApp->setMainWidget(this);
  setDockMenuEnabled(true);

  Edit=NULL;

  FileMenu=new QPopupMenu(this);

  FileMenu->insertItem(QPixmap((const char **)connect_xpm),
		       tr("&New Connection..."),TO_NEW_CONNECTION);
  FileMenu->insertItem(QPixmap((const char **)disconnect_xpm),
		       tr("&Close Connection"),this,SLOT(delConnection()),0,TO_CLOSE_CONNECTION);
#ifndef OAS
  FileMenu->insertSeparator();
  FileMenu->insertItem(QPixmap((const char **)commit_xpm),tr("&Commit Connection"),TO_FILE_COMMIT);
  FileMenu->insertItem(QPixmap((const char **)rollback_xpm),tr("&Rollback Connection"),TO_FILE_ROLLBACK);
  FileMenu->insertItem(tr("C&urrent Connection"),TO_FILE_CURRENT);
  FileMenu->insertItem(QPixmap((const char **)stop_xpm),tr("Stop All Queries"),TO_STOP_ALL);
  FileMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("Reread Object Cache"),TO_FILE_CLEARCACHE);
#endif
  FileMenu->insertSeparator();
  FileMenu->insertItem(QPixmap((const char **)fileopen_xpm),tr("&Open File..."),TO_FILE_OPEN);
  FileMenu->insertItem(QPixmap((const char **)filesave_xpm),tr("&Save"),TO_FILE_SAVE);
  FileMenu->insertItem(tr("Save A&s..."),TO_FILE_SAVE_AS);
  FileMenu->insertSeparator();
  FileMenu->insertItem(QPixmap((const char **)fileopen_xpm),tr("Open Session..."),
		       TO_FILE_OPEN_SESSION);
  FileMenu->insertItem(QPixmap((const char **)filesave_xpm),tr("Save Session..."),
		       TO_FILE_SAVE_SESSION);
  FileMenu->insertItem(tr("Restore Last Session"),TO_FILE_LAST_SESSION);
  FileMenu->insertItem(tr("Close Session"),TO_FILE_CLOSE_SESSION);
  FileMenu->insertSeparator();
  FileMenu->insertItem(QPixmap((const char **)print_xpm),tr("&Print..."),TO_FILE_PRINT);
  FileMenu->insertSeparator();
  FileMenu->insertItem(tr("&Quit"),TO_FILE_QUIT);

  FileMenu->setAccel(toKeySequence(tr("Ctrl+G", "File|New connection")),TO_NEW_CONNECTION);
  FileMenu->setAccel(toKeySequence(tr("Ctrl+O", "File|File open")),TO_FILE_OPEN);
  FileMenu->setAccel(toKeySequence(tr("Ctrl+W", "File|File save")),TO_FILE_SAVE);
  FileMenu->setAccel(toKeySequence(tr("Ctrl+Shift+W", "File|File save as")),TO_FILE_SAVE_AS);
#ifndef OAS
  FileMenu->setAccel(toKeySequence(tr("Ctrl+Shift+C", "File|Commit")),TO_FILE_COMMIT);
  FileMenu->setAccel(toKeySequence(tr("Ctrl+J", "File|Stop all")),TO_STOP_ALL);
  FileMenu->setAccel(toKeySequence(tr("Ctrl+<", "File|Rollback"))|CTRL,TO_FILE_ROLLBACK);
  FileMenu->setAccel(toKeySequence(tr("Ctrl+Shift+U", "File|Current connection")),TO_FILE_CURRENT);
#endif

  updateRecent();

  connect(FileMenu,SIGNAL(aboutToShow()),this,SLOT(showFileMenu()));
  menuBar()->insertItem(tr("&File"),FileMenu,TO_FILE_MENU);
  connect(FileMenu,SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  EditMenu=new QPopupMenu(this);
  EditMenu->insertItem(QPixmap((const char **)undo_xpm),tr("&Undo"),TO_EDIT_UNDO);
  EditMenu->insertItem(QPixmap((const char **)redo_xpm),tr("&Redo"),TO_EDIT_REDO);
  EditMenu->insertSeparator();
  EditMenu->insertItem(QPixmap((const char **)cut_xpm),tr("Cu&t"),TO_EDIT_CUT);
  EditMenu->insertItem(QPixmap((const char **)copy_xpm),tr("&Copy"),TO_EDIT_COPY);
  EditMenu->insertItem(QPixmap((const char **)paste_xpm),tr("&Paste"),TO_EDIT_PASTE);
  EditMenu->insertSeparator();
  EditMenu->insertItem(QPixmap((const char **)search_xpm),tr("&Search && Replace..."),
		       TO_EDIT_SEARCH);
  EditMenu->insertItem(tr("Search &Next"),TO_EDIT_SEARCH_NEXT);
  EditMenu->insertItem(tr("Select &All"),TO_EDIT_SELECT_ALL);
  EditMenu->insertItem(tr("Read All &Items"),TO_EDIT_READ_ALL);
  EditMenu->insertSeparator();
  EditMenu->insertItem(
#ifdef TOAD
		       QPixmap((const char **)options_xpm),
#endif
		       tr("&Options..."),TO_EDIT_OPTIONS);

  QAccel *accel=new QAccel(this);
  accel->connectItem(accel->insertItem(toKeySequence(tr("Ctrl+Insert", "Edit|Copy"))),this,SLOT(copyButton()));
  accel->connectItem(accel->insertItem(toKeySequence(tr("Shift+Delete", "Edit|Cut"))),this,SLOT(cutButton()));
  accel->connectItem(accel->insertItem(toKeySequence(tr("Shift+Insert", "Edit|Paste"))),this,SLOT(pasteButton()));

  EditMenu->setAccel(toKeySequence(tr("Ctrl+A", "Edit|Select all")),TO_EDIT_SELECT_ALL);
  EditMenu->setAccel(toKeySequence(tr("Ctrl+Z", "Edit|Undo")),TO_EDIT_UNDO);
  EditMenu->setAccel(toKeySequence(tr("Ctrl+Y", "Edit|Redo")),TO_EDIT_REDO);
  EditMenu->setAccel(toKeySequence(tr("Ctrl+X", "Edit|Cut")),TO_EDIT_CUT);
  EditMenu->setAccel(toKeySequence(tr("Ctrl+C", "Edit|Copy")),TO_EDIT_COPY);
  EditMenu->setAccel(toKeySequence(tr("Ctrl+V", "Edit|Paste")),TO_EDIT_PASTE);
  EditMenu->setAccel(toKeySequence(tr("Ctrl+F", "Edit|Search")),TO_EDIT_SEARCH);
  EditMenu->setAccel(toKeySequence(tr("F3", "Edit|Search next")),TO_EDIT_SEARCH_NEXT);


  menuBar()->insertItem(tr("&Edit"),EditMenu,TO_EDIT_MENU);
  connect(EditMenu,SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  std::map<QCString,toTool *> &tools=toTool::tools();

  ToolsToolbar=toAllocBar(this,tr("Tools"));

  EditToolbar=toAllocBar(this,tr("Application"));

  LoadButton=new QToolButton(QPixmap((const char **)fileopen_xpm),
			     tr("Load file into editor"),
			     tr("Load file into editor"),
			     this,SLOT(loadButton()),EditToolbar);
  SaveButton=new QToolButton(QPixmap((const char **)filesave_xpm),
			     tr("Save file from editor"),
			     tr("Save file from editor"),
			     this,SLOT(saveButton()),EditToolbar);
  PrintButton=new QToolButton(QPixmap((const char **)print_xpm),
			     tr("Print"),
			     tr("Print"),
			     this,SLOT(printButton()),EditToolbar);
  PrintButton->setEnabled(false);
  LoadButton->setEnabled(false);
  SaveButton->setEnabled(false);
#ifdef TOAD
  new QToolButton(QPixmap((const char **)options_xpm),
		  tr("Edit options"),
		  tr("Edit options"),
		  this,SLOT(optionButton()),EditToolbar);
#endif
  EditToolbar->addSeparator();
  UndoButton=new QToolButton(QPixmap((const char **)undo_xpm),
			     tr("Undo"),
			     tr("Undo"),
			     this,SLOT(undoButton()),EditToolbar);
  RedoButton=new QToolButton(QPixmap((const char **)redo_xpm),
			     tr("Redo"),
			     tr("Redo"),
			     this,SLOT(redoButton()),EditToolbar);
  CutButton=new QToolButton(QPixmap((const char **)cut_xpm),
			     tr("Cut to clipboard"),
			     tr("Cut to clipboard"),
			     this,SLOT(cutButton()),EditToolbar);
  CopyButton=new QToolButton(QPixmap((const char **)copy_xpm),
			     tr("Copy to clipboard"),
			     tr("Copy to clipboard"),
			     this,SLOT(copyButton()),EditToolbar);
  PasteButton=new QToolButton(QPixmap((const char **)paste_xpm),
			      tr("Paste from clipboard"),
			      tr("Paste from clipboard"),
			      this,SLOT(pasteButton()),EditToolbar);
  EditToolbar->addSeparator();
  SearchButton=new QToolButton(QPixmap((const char **)search_xpm),
			       tr("Search & replace"),
			       tr("Search & replace"),
			       this,SLOT(searchButton()),EditToolbar);

  UndoButton->setEnabled(false);
  RedoButton->setEnabled(false);
  CutButton->setEnabled(false);
  CopyButton->setEnabled(false);
  PasteButton->setEnabled(false);
  SearchButton->setEnabled(false);
  
  ConnectionToolbar=toAllocBar(this,tr("Connections"));
  new QToolButton(QPixmap((const char **)connect_xpm),
		  tr("Connect to database"),
		  tr("Connect to database"),
		  this,SLOT(addConnection()),ConnectionToolbar);
  DisconnectButton=new QToolButton(QPixmap((const char **)disconnect_xpm),
				   tr("Disconnect current connection"),
				   tr("Disconnect current connection"),
				   this,SLOT(delConnection()),ConnectionToolbar);
  DisconnectButton->setEnabled(false);
#ifndef OAS
  ConnectionToolbar->addSeparator();
  NeedConnection[new QToolButton(QPixmap((const char **)commit_xpm),
				 tr("Commit connection"),
				 tr("Commit connection"),
				 this,SLOT(commitButton()),ConnectionToolbar)]=true;
  NeedConnection[new QToolButton(QPixmap((const char **)rollback_xpm),
				 tr("Rollback connection"),
				 tr("Rollback connection"),
				 this,SLOT(rollbackButton()),ConnectionToolbar)]=true;
  ConnectionToolbar->addSeparator();
  NeedConnection[new QToolButton(QPixmap((const char **)stop_xpm),
				 tr("Stop all running queries on connection"),
				 tr("Stop all running queries on connection"),
				 this,SLOT(stopButton()),ConnectionToolbar)]=true;
#else
  ConnectionToolbar->hide();
#endif
  ConnectionToolbar->addSeparator();
  ConnectionSelection=new QComboBox(ConnectionToolbar,TO_KDE_TOOLBAR_WIDGET);
  ConnectionSelection->setMinimumWidth(250);
  ConnectionSelection->setFocusPolicy(NoFocus);
  connect(ConnectionSelection,SIGNAL(activated(int)),this,SLOT(changeConnection()));

  ToolsMenu=new QPopupMenu(this);

  HelpMenu=new QPopupMenu(this);
#ifndef TOAD
  HelpMenu->insertItem(tr("C&urrent Context..."),TO_HELP_CONTEXT);
#endif
  HelpMenu->insertItem(tr("&Contents..."),TO_HELP_CONTENTS);
  HelpMenu->insertSeparator();
  HelpMenu->insertItem(tr("&About " TOAPPNAME "..."),TO_HELP_ABOUT);
  HelpMenu->insertItem(tr("&License..."),TO_HELP_LICENSE);
#ifndef TOAD
  HelpMenu->insertItem(tr("&Quotes..."),TO_HELP_QUOTES);
#endif
  HelpMenu->setAccel(toKeySequence(tr("F1", "Help|Help")),TO_HELP_CONTEXT);

  int toolID=TO_TOOLS;
  SQLEditor=-1;
  DefaultTool=toolID;
  QCString defName=toTool::globalConfig(CONF_DEFAULT_TOOL,"").latin1();

  QPopupMenu *toolAbout=NULL;

  for (std::map<QCString,toTool *>::iterator i=tools.begin();i!=tools.end();i++) {
    const QPixmap *pixmap=(*i).second->toolbarImage();
    const char *menuName=(*i).second->menuItem();

    QCString tmp=(*i).first;
    tmp+=CONF_TOOL_ENABLE;
    if (toTool::globalConfig(tmp,"Yes").isEmpty())
      continue;

    if (defName==(*i).first)
      DefaultTool=toolID;

    if ((*i).second->hasAbout()&&menuName) {
      if (!toolAbout) {
	toolAbout=new QPopupMenu(this);
	HelpMenu->insertItem(tr("Tools"),toolAbout);
      }
      if (pixmap)
	toolAbout->insertItem(*pixmap,qApp->translate("toTool",menuName),toolID+TO_ABOUT_ID_OFFSET);
      else
	toolAbout->insertItem(qApp->translate("toTool",menuName),toolID+TO_ABOUT_ID_OFFSET);
    }

    Tools[toolID]=(*i).second;

    toolID++;
  }

  if (!toTool::globalConfig(CONF_TOOLS_LEFT,"").isEmpty())
    moveToolBar(ToolsToolbar,Left);

  menuBar()->insertItem(tr("&Tools"),ToolsMenu,TO_TOOLS_MENU);
  connect(ToolsMenu,SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  WindowsMenu=new QPopupMenu(this);

  WindowsMenu->setCheckable(true);
  WindowsMenu->insertItem(tr("C&lose"),TO_WINDOWS_CLOSE);
  WindowsMenu->insertItem(tr("Close &All"),TO_WINDOWS_CLOSE_ALL);
  WindowsMenu->insertSeparator();
  WindowsMenu->insertItem(tr("&Cascade"),TO_WINDOWS_CASCADE);
  WindowsMenu->insertItem(tr("&Tile"),TO_WINDOWS_TILE);
  WindowsMenu->insertSeparator();

  connect(WindowsMenu,SIGNAL(aboutToShow()),this,SLOT( windowsMenu()));
  menuBar()->insertItem(tr("&Window"),WindowsMenu,TO_WINDOWS_MENU);
  connect(WindowsMenu,SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  menuBar()->insertSeparator();

  menuBar()->insertItem(tr("&Help"),HelpMenu,TO_HELP_MENU);
  connect(HelpMenu,SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  char buffer[100];
  sprintf(buffer,DEFAULT_TITLE,TOVERSION);
  setCaption(tr(buffer));

#ifdef TO_KDE
  KDockWidget *mainDock=createDockWidget(tr(buffer),QPixmap((const char **)toramini_xpm));
  Workspace=new QWorkspace(mainDock);
  mainDock->setWidget(Workspace);
  setView(mainDock);
  setMainDockWidget(mainDock);
  mainDock->setEnableDocking(KDockWidget::DockNone);
#else
  Workspace=new QWorkspace(this);
  setCentralWidget(Workspace);
#endif
  setIcon(QPixmap((const char **)tora_xpm));

  statusBar()->message(QString::null);
  FileMenu->setItemEnabled(TO_CLOSE_CONNECTION,false);
  FileMenu->setItemEnabled(TO_FILE_COMMIT,false);
  FileMenu->setItemEnabled(TO_STOP_ALL,false);
  FileMenu->setItemEnabled(TO_FILE_ROLLBACK,false);
  FileMenu->setItemEnabled(TO_FILE_CLEARCACHE,false);
  DisconnectButton->setEnabled(false);

  RowLabel=new QLabel(statusBar());
  statusBar()->addWidget(RowLabel,0,true);
  RowLabel->setMinimumWidth(60);
  //  RowLabel->hide();

  ColumnLabel=new QLabel(statusBar());
  statusBar()->addWidget(ColumnLabel,0,true);
  ColumnLabel->setMinimumWidth(60);
  //  ColumnLabel->hide();

  QToolButton *dispStatus=new toPopupButton(statusBar());
  dispStatus->setIconSet(QPixmap((const char **)up_xpm));
  statusBar()->addWidget(dispStatus,0,true);
  StatusMenu=new QPopupMenu(dispStatus);
  dispStatus->setPopup(StatusMenu);
  connect(StatusMenu,SIGNAL(aboutToShow()),
	  this,SLOT(statusMenu()));
  connect(StatusMenu,SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  toolID=TO_TOOLS;
  for (std::map<QCString,toTool *>::iterator k=tools.begin();k!=tools.end();k++) {
    (*k).second->customSetup(toolID);
    toolID++;
  }
  Search=NULL;

  for (std::map<QToolButton *,bool>::iterator j=NeedConnection.begin();
       j!=NeedConnection.end();j++)
    (*j).first->setEnabled(false);

  QString welcome;

  connect(&Poll,SIGNAL(timeout()),this,SLOT(checkCaching()));
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  if (!toTool::globalConfig(CONF_RESTORE_SESSION,"").isEmpty()) {
    try {
      std::map<QCString,QString> session;
      toTool::loadMap(toTool::globalConfig(CONF_DEFAULT_SESSION,DEFAULT_SESSION),session);
      importData(session,"TOra");
    } TOCATCH
  }

  if (!toTool::globalConfig(CONF_MAXIMIZE_MAIN,"Yes").isEmpty()&&Connections.size()==0)
    showMaximized();
  else
    show();

  toBackground::init();

  if (Connections.size()==0) {
    try {
      toNewConnection newConnection(this,tr("First connection"),true);
    
      toConnection *conn;
    
      do {
	conn=NULL;
	if (newConnection.exec()) {
	  conn=newConnection.makeConnection();
	} else {
	  break;
	}
      } while(!conn);
    
      if (conn)
	addConnection(conn);
    } TOCATCH
  }
}

void toMain::windowActivated(QWidget *widget)
{
  if (toTool::globalConfig(CONF_CHANGE_CONNECTION,"Yes").isEmpty())
    return;
  toToolWidget *tool=dynamic_cast<toToolWidget *>(widget);
  if (tool) {
    try {
      toConnection &conn=tool->connection();
      int pos=0;
      for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
	if (&conn==*i) {
	  ConnectionSelection->setCurrentItem(pos);
	  changeConnection();
	  break;
	}
	pos++;
      }
    } TOCATCH
  }
}

void toMain::showFileMenu(void)
{
  bool hascon=(ConnectionSelection->count()>0);

  FileMenu->setItemEnabled(TO_FILE_COMMIT,hascon);
  FileMenu->setItemEnabled(TO_STOP_ALL,hascon);
  FileMenu->setItemEnabled(TO_FILE_ROLLBACK,hascon);
  FileMenu->setItemEnabled(TO_FILE_CLEARCACHE,hascon);
  FileMenu->setItemEnabled(TO_CLOSE_CONNECTION,hascon);

  updateRecent();
}

void toMain::updateRecent()
{
  static bool first=true;
  int num=toTool::globalConfig(CONF_RECENT_FILES,"0").toInt();
  if (num>0) {
    if (first) {
      FileMenu->insertSeparator();
      first=false;
    }
    for (int i=0;i<num;i++) {
      QString file=toTool::globalConfig(QCString(CONF_RECENT_FILES ":")+QString::number(i).latin1(),"");
      if (!file.isEmpty()) {
	QFileInfo fi(file);
	if (FileMenu->indexOf(TO_LAST_FILE_ID+i)==-1)
	  FileMenu->insertItem(fi.fileName(),TO_LAST_FILE_ID+i);
	else {
	  FileMenu->changeItem(TO_LAST_FILE_ID+i,fi.fileName());
	}
      }
      FileMenu->setItemEnabled(TO_LAST_FILE_ID+i,Edit&&Edit->openEnabled());
    }
  }
}

void toMain::addRecentFile(const QString &file)
{
  int num=toTool::globalConfig(CONF_RECENT_FILES,"0").toInt();
  int maxnum=toTool::globalConfig(CONF_RECENT_MAX,DEFAULT_RECENT_MAX).toInt();
  std::list<QString> files;
  for (int j=0;j<num;j++) {
    QString t=toTool::globalConfig(QCString(CONF_RECENT_FILES ":")+QString::number(j).latin1(),"");
    if (t!=file)
      toPush(files,t);
  }
  toUnShift(files,file);

  num=0;
  for (std::list<QString>::iterator i=files.begin();i!=files.end();i++) {
    toTool::globalSetConfig(QCString(CONF_RECENT_FILES ":")+QString::number(num).latin1(),*i);
    num++;
    if (num>=maxnum)
      break;
  }
  toTool::globalSetConfig(CONF_RECENT_FILES,QString::number(num));
  toTool::saveConfig();
}

void toMain::windowsMenu(void)
{
  WindowsMenu->setItemEnabled(TO_WINDOWS_CLOSE,workspace()->activeWindow());

  bool first=true;
  QRegExp strip(QString::fromLatin1(" <[0-9]+>$"));
  int id=0;
  unsigned int i;
#if QT_VERSION < 0x030200
  for (i=0;i<workspace()->windowList().count();i++) {
    QWidget *widget=workspace()->windowList().at(i);
#else
  for (i=0;i<workspace()->windowList(QWorkspace::CreationOrder).count();i++) {
    QWidget *widget=workspace()->windowList(QWorkspace::CreationOrder).at(i);
#endif
    if (widget&&!widget->isHidden()) {
      if (first)
	first=false;
      QString caption=widget->caption();
      caption.replace(strip,QString::null);
      if(WindowsMenu->indexOf(TO_WINDOWS_WINDOWS+i)<0)
	WindowsMenu->insertItem(caption,TO_WINDOWS_WINDOWS+i);
      else
	WindowsMenu->changeItem(TO_WINDOWS_WINDOWS+i,caption);
#if QT_VERSION < 0x030200
      WindowsMenu->setItemChecked(TO_WINDOWS_WINDOWS+i,
				  workspace()->activeWindow()==workspace()->windowList().at(i));
#else
      WindowsMenu->setItemChecked(TO_WINDOWS_WINDOWS+i,
				  workspace()->activeWindow()==workspace()->windowList(QWorkspace::CreationOrder).at(i));
#endif
      if (i<9) {
	WindowsMenu->setAccel(Key_1+id|CTRL,TO_WINDOWS_WINDOWS+i);
	caption+=QString::fromLatin1(" <");
	caption+=QString::number(++id);
	caption+=QString::fromLatin1(">");
      }
      widget->setCaption(caption);
    }
  }
  while(WindowsMenu->indexOf(TO_WINDOWS_WINDOWS+i)>=0) {
    WindowsMenu->removeItem(TO_WINDOWS_WINDOWS+i);
    i++;
  }

  WindowsMenu->setItemEnabled(TO_WINDOWS_CASCADE,!first);
  WindowsMenu->setItemEnabled(TO_WINDOWS_TILE,!first);
  WindowsMenu->setItemEnabled(TO_WINDOWS_CLOSE_ALL,!first);
}

void toMain::commandCallback(int cmd)
{
  QWidget *focus=qApp->focusWidget();
  
  if (focus) {
    toEditWidget *edit=findEdit(focus);
    if (edit&&edit!=Edit)
      setEditWidget(edit);
    else if (focus->inherits("QLineEdit")||
	     focus->isA("QSpinBox"))
      editDisable(Edit);
  }

  if (Tools[cmd])
    Tools[cmd]->createWindow();
  else if (cmd>=TO_TOOL_ABOUT_ID&&cmd<=TO_TOOL_ABOUT_ID_END) {
    if (Tools[cmd-TO_ABOUT_ID_OFFSET])
      Tools[cmd-TO_ABOUT_ID_OFFSET]->about(this);
  } else if (cmd>=TO_WINDOWS_WINDOWS&&cmd<=TO_WINDOWS_END) {
#if QT_VERSION < 0x030200
    if (cmd-TO_WINDOWS_WINDOWS<int(workspace()->windowList().count())) {
      QWidget *widget=workspace()->windowList().at(cmd-TO_WINDOWS_WINDOWS);
#else
    if (cmd-TO_WINDOWS_WINDOWS<int(workspace()->windowList(QWorkspace::CreationOrder).count())) {
      QWidget *widget=workspace()->windowList(QWorkspace::CreationOrder).at(cmd-TO_WINDOWS_WINDOWS);
#endif
      widget->raise();
      widget->setFocus();
    }
  } else if (cmd>=TO_STATUS_ID&&cmd<=TO_STATUS_ID_END) {
    QString str=StatusMenu->text(cmd);
    new toMemoEditor(this,str);
  } else {
    QWidget *currWidget=qApp->focusWidget();
    toEditWidget *edit=NULL;
    while(currWidget&&!edit) {
      edit=dynamic_cast<toEditWidget *>(currWidget);
      currWidget=currWidget->parentWidget();
    }
    if (edit) {
      if (cmd>=TO_LAST_FILE_ID&&cmd<=TO_LAST_FILE_ID_END) {
	edit->editOpen(toTool::globalConfig(QCString(CONF_RECENT_FILES ":")+
					    QString::number(cmd-TO_LAST_FILE_ID).latin1(),
					    ""));
      } else {
	switch(cmd) {
	case TO_EDIT_REDO:
	  edit->editRedo();
	  break;
	case TO_EDIT_UNDO:
	  edit->editUndo();
	  break;
	case TO_EDIT_COPY:
	  edit->editCopy();
	  break;
	case TO_EDIT_PASTE:
	  edit->editPaste();
	  break;
	case TO_EDIT_CUT:
	  edit->editCut();
	  break;
	case TO_EDIT_SELECT_ALL:
	  edit->editSelectAll();;
	  break;
	case TO_EDIT_READ_ALL:
	  edit->editReadAll();
	  break;
	case TO_EDIT_SEARCH:
	  if (!Search)
	    Search=new toSearchReplace(this);
	  Search->show();
	  break;
	case TO_FILE_OPEN:
	  edit->editOpen();
	  break;
	case TO_FILE_SAVE_AS:
	  edit->editSave(true);
	  break;
	case TO_FILE_SAVE:
	  edit->editSave(false);
	  break;
	case TO_FILE_PRINT:
	  edit->editPrint();
	  break;
	}
      }
    }
    switch(cmd) {
    case TO_FILE_COMMIT:
      try {
	toConnection &conn=currentConnection();
	emit willCommit(conn,true);
	conn.commit();
	setNeedCommit(conn,false);
      } TOCATCH
      break;
    case TO_STOP_ALL:
      try {
	toConnection &conn=currentConnection();
	conn.cancelAll();
      } TOCATCH
      break;
    case TO_FILE_CLEARCACHE:
      try {
	currentConnection().rereadCache();
      } TOCATCH
      toMainWidget()->checkCaching();
      break;
    case TO_FILE_ROLLBACK:
      try {
	toConnection &conn=currentConnection();
	emit willCommit(conn,false);
	conn.rollback();
	setNeedCommit(conn,false);
      } TOCATCH
      break;
    case TO_FILE_CURRENT:
      ConnectionSelection->setFocus();
      break;
    case TO_FILE_QUIT:
      close(true);
      break;
    case TO_EDIT_SEARCH_NEXT:
      if (Search)
	Search->searchNext();
      break;
    case TO_WINDOWS_CASCADE:
      workspace()->cascade();
      break;
    case TO_WINDOWS_TILE:
      workspace()->tile();
      break;
    case TO_NEW_CONNECTION:
      addConnection();
      break;
    case TO_HELP_CONTEXT:
      contextHelp();
      break;
    case TO_HELP_CONTENTS:
      toHelp::displayHelp(QString::fromLatin1("toc.htm"));
      break;
    case TO_HELP_ABOUT:
    case TO_HELP_LICENSE:
    case TO_HELP_QUOTES:
      {
	toAbout about(cmd-TO_HELP_ABOUT,this,"About " TOAPPNAME,true);
	about.exec();
      }
      break;
    case TO_EDIT_OPTIONS:
      toPreferences::displayPreferences(this);
      break;
    case TO_WINDOWS_CLOSE_ALL:
#if QT_VERSION < 0x030200
      while (workspace()->windowList().count()>0&&workspace()->windowList().at(0))
	if (workspace()->windowList().at(0)&&
	    !workspace()->windowList().at(0)->close(true))
#else
      while (workspace()->windowList(QWorkspace::CreationOrder).count()>0&&workspace()->windowList(QWorkspace::CreationOrder).at(0))
	if (workspace()->windowList(QWorkspace::CreationOrder).at(0)&&
	    !workspace()->windowList(QWorkspace::CreationOrder).at(0)->close(true))
#endif
	  return;
      break;
    case TO_WINDOWS_CLOSE:
      {
	QWidget *widget=workspace()->activeWindow();
	if (widget)
	  widget->close(true);
	break;
      }
    case TO_FILE_OPEN_SESSION:
      loadSession();
      break;
    case TO_FILE_SAVE_SESSION:
      saveSession();
      break;
    case TO_FILE_LAST_SESSION:
      try {
	std::map<QCString,QString> session;
	toTool::loadMap(toTool::globalConfig(CONF_DEFAULT_SESSION,DEFAULT_SESSION),session);
	importData(session,"TOra");
      } TOCATCH
      break;
    case TO_FILE_CLOSE_SESSION:
      closeSession();
      break;
    }
  }
}

void toMain::addConnection(void)
{
  try {
    toNewConnection newConnection(this,"New connection",true);

    toConnection *conn=NULL;

    if (newConnection.exec())
      conn=newConnection.makeConnection();

    if (conn)
      addConnection(conn);
  } TOCATCH
}

toConnection &toMain::currentConnection()
{
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if (ConnectionSelection->currentText().startsWith((*i)->description())) {
      return *(*i);
    }
  }
  throw tr("Can't find active connection");
}

toConnection *toMain::addConnection(toConnection *conn,bool def)
{
  int j=0;
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++,j++) {
    if ((*i)->description()==conn->description()) {
      ConnectionSelection->setCurrentItem(j);
      if (def)
	createDefault();
      return *i;
    }
  }

  Connections.insert(Connections.end(),conn);
  ConnectionSelection->insertItem(conn->description());
  ConnectionSelection->setCurrentItem(ConnectionSelection->count()-1);

  if (ConnectionSelection->count()==1) {
    FileMenu->setItemEnabled(TO_FILE_COMMIT,true);
    FileMenu->setItemEnabled(TO_STOP_ALL,true);
    FileMenu->setItemEnabled(TO_FILE_ROLLBACK,true);
    FileMenu->setItemEnabled(TO_FILE_CLEARCACHE,true);
    FileMenu->setItemEnabled(TO_CLOSE_CONNECTION,true);
    DisconnectButton->setEnabled(true);
  }

  checkCaching();

  changeConnection();
  emit addedConnection(conn->description());

  if (def)
    createDefault();

  return conn;
}

void toMain::setNeedCommit(toConnection &conn,bool needCommit)
{
  int pos=0;
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if (conn.description()==(*i)->description()) {
      QString dsc=conn.description();
      if (needCommit)
	dsc+=QString::fromLatin1(" *");
      ConnectionSelection->changeItem(dsc,pos);
      break;
    }
    pos++;
  }
  conn.setNeedCommit(needCommit);
}

bool toMain::delConnection(void)
{
  toConnection *conn=NULL;
  int pos=0;
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if (ConnectionSelection->currentText().startsWith((*i)->description())) {
      conn=(*i);
      if (conn->needCommit()) {
	QString str=tr("Commit work in session to %1  before closing it?").arg(conn->description());
	switch(TOMessageBox::warning(this,tr("Commit work?"),str,tr("&Yes"),tr("&No"),tr("Cancel"))) {
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
	ConnectionSelection->setCurrentItem(max(pos-1,0));
      delete conn;
      break;
    }
    pos++;
  }
  if (ConnectionSelection->count()==0) {
    FileMenu->setItemEnabled(TO_FILE_COMMIT,false);
    FileMenu->setItemEnabled(TO_STOP_ALL,false);
    FileMenu->setItemEnabled(TO_FILE_ROLLBACK,false);
    FileMenu->setItemEnabled(TO_FILE_CLEARCACHE,false);
    FileMenu->setItemEnabled(TO_CLOSE_CONNECTION,false);
    DisconnectButton->setEnabled(false);
    ToolsMenu->clear();
    ToolsToolbar->clear();
    for (std::map<QToolButton *,bool>::iterator i=NeedConnection.begin();
	 i!=NeedConnection.end();i++)
      (*i).first->setEnabled(false);
  } else
    changeConnection();
  return true;
}

std::list<QString> toMain::connections(void)
{
  std::list<QString> ret;
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++)
    toPush(ret,(*i)->description());
  return ret;
}

toConnection &toMain::connection(const QString &str)
{
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++)
    if ((*i)->description()==str)
      return *(*i);
  throw tr("Couldn't find specified connectionts (%1)").arg(str);
}

void toMain::loadButton(void)
{
  commandCallback(TO_FILE_OPEN);
}

void toMain::commitButton(void)
{
  commandCallback(TO_FILE_COMMIT);
}

void toMain::stopButton(void)
{
  commandCallback(TO_STOP_ALL);
}

void toMain::rollbackButton(void)
{
  commandCallback(TO_FILE_ROLLBACK);
}

void toMain::saveButton(void)
{
  commandCallback(TO_FILE_SAVE);
}

void toMain::undoButton(void)
{
  commandCallback(TO_EDIT_UNDO);
}

void toMain::redoButton(void)
{
  commandCallback(TO_EDIT_REDO);
}

void toMain::copyButton(void)
{
  commandCallback(TO_EDIT_COPY);
}

void toMain::cutButton(void)
{
  commandCallback(TO_EDIT_CUT);
}

void toMain::searchButton(void)
{
  commandCallback(TO_EDIT_SEARCH);
}

void toMain::pasteButton(void)
{
  commandCallback(TO_EDIT_PASTE);
}

void toMain::printButton(void)
{
  commandCallback(TO_FILE_PRINT);
}

void toMain::optionButton(void)
{
  commandCallback(TO_EDIT_OPTIONS);
}

void toMain::setEditWidget(toEditWidget *edit)
{
  toMain *main=(toMain *)qApp->mainWidget();
  if (main&&edit) {
    if (main->Edit)
      main->Edit->lostFocus();
    main->Edit=edit;
#if 0
    main->RowLabel->hide();
    main->ColumnLabel->hide();
#else
    main->RowLabel->setText(QString::null);
    main->ColumnLabel->setText(QString::null);
#endif
    main->editEnable(edit);
  }
}

void toMain::editEnable(toEditWidget *edit)
{
  toMain *main=(toMain *)qApp->mainWidget();
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
		     edit->readAllEnabled());

#if QT_VERSION >= 0x030000
    // Set Selection Mode on X11
    QClipboard *clip=qApp->clipboard();
    if(clip->supportsSelection())
      clip->setSelectionMode(true);
#endif
}

void toMain::editDisable(toEditWidget *edit)
{
  toMain *main=(toMain *)qApp->mainWidget();

  if (main) {
    if(edit&&edit==main->Edit) {
      main->editEnable(edit,false,false,false,false,false,false,false,false,false,false,false);
      main->Edit->lostFocus();
      main->Edit=NULL;
    }
  }
}

toEditWidget *toMain::findEdit(QWidget *widget)
{
  while(widget) {
    toEditWidget *edit=dynamic_cast<toEditWidget *>(widget);
    if (edit)
      return edit;
    widget=widget->parentWidget();
  }
  return NULL;
}

void toMain::editEnable(toEditWidget *edit,bool open,bool save,bool print,
			bool undo,bool redo,
			bool cut,bool copy,bool paste,
			bool search,bool selectAll,bool readAll)
{
  if (edit&&edit==Edit) {
    LoadButton->setEnabled(open);
    SaveButton->setEnabled(save);
    PrintButton->setEnabled(print);

    UndoButton->setEnabled(undo);
    RedoButton->setEnabled(redo);

    CutButton->setEnabled(cut);
    CopyButton->setEnabled(copy);
    PasteButton->setEnabled(paste);
    SearchButton->setEnabled(search);

    EditMenu->setItemEnabled(TO_EDIT_UNDO,undo);
    EditMenu->setItemEnabled(TO_EDIT_REDO,redo);
    EditMenu->setItemEnabled(TO_EDIT_CUT,cut);
    EditMenu->setItemEnabled(TO_EDIT_COPY,copy);
    EditMenu->setItemEnabled(TO_EDIT_PASTE,paste);
    EditMenu->setItemEnabled(TO_EDIT_SELECT_ALL,selectAll);
    EditMenu->setItemEnabled(TO_EDIT_READ_ALL,readAll);
    EditMenu->setItemEnabled(TO_EDIT_SEARCH,search);
    EditMenu->setItemEnabled(TO_EDIT_SEARCH_NEXT,Search&&Search->searchNextAvailable());

    FileMenu->setItemEnabled(TO_FILE_OPEN,open);
    FileMenu->setItemEnabled(TO_FILE_SAVE,save);
    FileMenu->setItemEnabled(TO_FILE_SAVE_AS,save);
  }
}

void toMain::registerSQLEditor(int tool)
{
  SQLEditor=tool;
}

bool toMain::close(bool del)
{
  std::map<QCString,QString> session;
  exportData(session,"TOra");
  try {
    toTool::saveMap(toTool::globalConfig(CONF_DEFAULT_SESSION,
					 DEFAULT_SESSION),
		    session);
  } TOCATCH

  // Workaround in bug in Qt 3.0.0
  while (workspace()->windowList().count()>0&&workspace()->windowList().at(0))
    if (workspace()->windowList().at(0)&&
       !workspace()->windowList().at(0)->close(true))
      return false;
  while (Connections.end()!=Connections.begin()) {
    if (!delConnection())
      return false;
  }
  editDisable(Edit);
  toTool::saveConfig();
  return QMainWindow::close(del);
}

void toMain::createDefault(void)
{
  commandCallback(DefaultTool);
}

void toMain::setCoordinates(int line,int col)
{
  QString str=tr("Row:")+" ";
  str+=QString::number(line);
  RowLabel->setText(str);
  str=tr("Col:")+" ";
  str+=QString::number(col);
  ColumnLabel->setText(str);
#if 0
  RowLabel->show();
  ColumnLabel->show();
#endif
}

void toMain::editSQL(const QString &str)
{
  if (SQLEditor>=0&&Tools[SQLEditor]) {
    Tools[SQLEditor]->createWindow();
    emit sqlEditor(str);
  }
}

void toMain::contextHelp(void)
{
  toHelp::displayHelp();
}

void toMain::statusMenu(void)
{
  std::list<QString> status=toStatusMessages();
  StatusMenu->clear();
  int id=TO_STATUS_ID;
  for(std::list<QString>::iterator i=status.begin();i!=status.end();i++)
    StatusMenu->insertItem(*i,id++);
}

void toMain::changeConnection(void)
{
  try {
    toConnection &conn=currentConnection();
    for (std::map<QToolButton *,bool>::iterator j=NeedConnection.begin();
	 j!=NeedConnection.end();j++)
      (*j).first->setEnabled(true);

    int toolID=TO_TOOLS;
    int lastPriorityPix=0;
    int lastPriorityMenu=0;

    ToolsMenu->clear();
    ToolsToolbar->clear();

    std::map<QCString,toTool *> &tools=toTool::tools();
    for (std::map<QCString,toTool *>::iterator i=tools.begin();i!=tools.end();i++) {
      const QPixmap *pixmap=(*i).second->toolbarImage();
      const char *toolTip=(*i).second->toolbarTip();
      const char *menuName=(*i).second->menuItem();

      QCString tmp=(*i).first;
      tmp+=CONF_TOOL_ENABLE;
      if (toTool::globalConfig(tmp,"Yes").isEmpty())
	continue;

      if ((*i).second->canHandle(conn)) {
	int priority=(*i).second->priority();
	if (priority/100!=lastPriorityPix/100&&
	    pixmap) {
	  ToolsToolbar->addSeparator();
	  lastPriorityPix=priority;
	}
	if (priority/100!=lastPriorityMenu/100&&
	    menuName) {
	  ToolsMenu->insertSeparator();
	  lastPriorityMenu=priority;
	}

	if (pixmap) {
	  if (!toolTip)
	    toolTip="";
	  new QToolButton(*pixmap,
			  qApp->translate("toTool",toolTip),
			  qApp->translate("toTool",toolTip),
			  (*i).second,
			  SLOT(createWindow(void)),
			  ToolsToolbar);
	}

	if (menuName) {
#ifdef TODEBUG_TRANSLATION
	  printf("QT_TRANSLATE_NOOP(\"toTool\",\"%s\"),\n",(const char *)menuName);
#endif
	  if (pixmap)
	    ToolsMenu->insertItem(*pixmap,qApp->translate("toTool",menuName),toolID);
	  else
	    ToolsMenu->insertItem(qApp->translate("toTool",menuName),toolID);
	}
      }

      toolID++;
    }
  } TOCATCH
}

void toMain::checkCaching(void)
{
  int num=0;
  for(std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if (!(*i)->cacheAvailable(true,false,false))
      num++;
  }
  if (num==0)
    Poll.stop();
  else
    Poll.start(100);
}

void toMain::exportData(std::map<QCString,QString> &data,const QCString &prefix)
{
  try {
    if (isMaximized())
      data[prefix+":State"]=QString::fromLatin1("Maximized");
    else if (isMinimized())
      data[prefix+":State"]=QString::fromLatin1("Minimized");
    else {
      QRect rect=geometry();
      data[prefix+":X"]=QString::number(rect.x());
      data[prefix+":Y"]=QString::number(rect.y());
      data[prefix+":Width"]=QString::number(rect.width());
      data[prefix+":Height"]=QString::number(rect.height());
    }

    int id=1;
    std::map<toConnection *,int> connMap;
    {
      for(std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
	QCString key=prefix+":Connection:"+QString::number(id).latin1();
	if (toTool::globalConfig(CONF_SAVE_PWD,DEFAULT_SAVE_PWD)!=DEFAULT_SAVE_PWD)
	  data[key+":Password"]=toObfuscate((*i)->password());
	data[key+":User"]=(*i)->user();
	data[key+":Host"]=(*i)->host();

	QString options;
	for(std::set<QString>::const_iterator j=(*i)->options().begin();j!=(*i)->options().end();j++)
	  options+=","+*j;
	data[key+":Options"]=options.mid(1); // Strip extra , in beginning

	data[key+":Database"]=(*i)->database();
	data[key+":Provider"]=(*i)->provider();
	connMap[*i]=id;
	id++;
      }
    }

    id=1;
#if QT_VERSION < 0x030200
    for (unsigned int i=0;i<workspace()->windowList().count();i++) {
      toToolWidget *tool=dynamic_cast<toToolWidget *>(workspace()->windowList().at(i));
#else
    for (unsigned int i=0;i<workspace()->windowList(QWorkspace::CreationOrder).count();i++) {
      toToolWidget *tool=dynamic_cast<toToolWidget *>(workspace()->windowList(QWorkspace::CreationOrder).at(i));
#endif
      if (tool) {
	QCString key=prefix+":Tools:"+QString::number(id).latin1();
	tool->exportData(data,key);
	data[key+":Type"]=tool->tool().key();
	data[key+":Connection"]=QString::number(connMap[&tool->connection()]);
	id++;
      }
    }

    toTemplateProvider::exportAllData(data,prefix+":Templates");
  } TOCATCH
}

void toMain::importData(std::map<QCString,QString> &data,const QCString &prefix)
{
  if (data[prefix+":State"]==QString::fromLatin1("Maximized"))
    showMaximized();
  else if (data[prefix+":State"]==QString::fromLatin1("Minimized"))
    showMinimized();
  else {
    int width=data[prefix+":Width"].toInt();
    if (width==0) {
      TOMessageBox::warning(toMainWidget(),
			    tr("Invalid session file"),tr("The session file is not valid, can't read it."));
      return;
    } else
      setGeometry(data[prefix+":X"].toInt(),
		  data[prefix+":Y"].toInt(),
		  width,
		  data[prefix+":Height"].toInt());
    showNormal();
  }

  std::map<int,toConnection *> connMap;

  int id=1;
  std::map<QCString,QString>::iterator i;
  while((i=data.find(prefix+":Connection:"+QString::number(id).latin1()+":Database"))!=data.end()) {
    QCString key=prefix+":Connection:"+QString::number(id).latin1();
    QString database=(*i).second;
    QString user=data[key+":User"];
    QString host=data[key+":Host"];

    QStringList optionlist=QStringList::split(",",data[key+":Options"]);
    std::set<QString> options;
    for(unsigned int j=0;j<optionlist.count();j++)
      if (!optionlist[j].isEmpty())
	options.insert(optionlist[j]);

    QString password=toUnobfuscate(data[key+":Password"]);
    QString provider=data[key+":Provider"];
    bool ok=true;
    if (toTool::globalConfig(CONF_SAVE_PWD,DEFAULT_SAVE_PWD)==password) {
      password=QInputDialog::getText(tr("Input password"),
				     tr("Enter password for %1").arg(database),
				     QLineEdit::Password,
				     QString::fromLatin1(DEFAULT_SAVE_PWD),
				     &ok,
				     this);
    }
    if (ok) {
      try {
	toConnection *conn=new toConnection(provider.latin1(),user,password,host,database,options);
	if (conn) {
	  conn=addConnection(conn,false);
	  connMap[id]=conn;
	}
      } TOCATCH
    }
    id++;
  }

  id=1;
  while((i=data.find(prefix+":Tools:"+QString::number(id).latin1()+":Type"))!=data.end()) {
    QCString key=(*i).second.latin1();
    int connid=data[prefix+":Tools:"+QString::number(id).latin1()+":Connection"].toInt();
    std::map<int,toConnection *>::iterator j=connMap.find(connid);
    if (j!=connMap.end()) {
      toTool *tool=toTool::tool(key);
      if (tool) {
	QWidget *widget=tool->toolWindow(workspace(),*((*j).second));
	const QPixmap *icon=tool->toolbarImage();
	if (icon)
	  widget->setIcon(*icon);
	widget->show();
	if (widget) {
	  toToolWidget *tw=dynamic_cast<toToolWidget *>(widget);
	  if (tw) {
	    toToolCaption(tw,tool->name());
	    tw->importData(data,prefix+":Tools:"+QString::number(id).latin1());
	    toolWidgetAdded(tw);
	  }
	}
      }
    }
    id++;
  }

  toTemplateProvider::importAllData(data,prefix+":Templates");
  windowsMenu();
}

void toMain::saveSession(void)
{
  QString fn=toSaveFilename(QString::null,QString::fromLatin1("*.tse"),this);
  if (!fn.isEmpty()) {
    std::map<QCString,QString> session;
    exportData(session,"TOra");
    try {
      toTool::saveMap(fn,session);
    } TOCATCH
  }
}

void toMain::loadSession(void)
{
  QString filename=toOpenFilename(QString::null,QString::fromLatin1("*.tse"),this);
  if (!filename.isEmpty()) {
    try {
      std::map<QCString,QString> session;
      toTool::loadMap(filename,session);
      importData(session,"TOra");
    } TOCATCH
  }
}

void toMain::closeSession(void)
{
  std::map<QCString,QString> session;
  exportData(session,"TOra");
  try {
    toTool::saveMap(toTool::globalConfig(CONF_DEFAULT_SESSION,
					 DEFAULT_SESSION),
		    session);
  } TOCATCH

  // Workaround in bug in Qt 3.0.0
#if QT_VERSION < 0x030200
  while (workspace()->windowList().count()>0&&workspace()->windowList().at(0))
    if (workspace()->windowList().at(0)&&
	!workspace()->windowList().at(0)->close(true))
      return;
#else
  while (workspace()->windowList(QWorkspace::CreationOrder).count()>0&&workspace()->windowList(QWorkspace::CreationOrder).at(0))
    if (workspace()->windowList(QWorkspace::CreationOrder).at(0)&&
	!workspace()->windowList(QWorkspace::CreationOrder).at(0)->close(true))
      return;
#endif
  while (Connections.end()!=Connections.begin()) {
    if (!delConnection())
      return;
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
  static bool recursive=false;
  static bool disabled=false;
  if (disabled) {
    while(StatusMessages.size()>1) // Clear everything but the first one.
      toPop(StatusMessages);
    return;
  }

  if (StatusMessages.size()>=50) {
    disabled=true;
    toUnShift(StatusMessages,
	      tr("Message flood, temporary disabling of message box error reporting from now on.\n"
		 "Restart to reenable. You probably have a too high refreshrate in some tool."));
  }

  if (recursive)
    return;
  recursive=true;

  for(QString str=toShift(StatusMessages);!str.isEmpty();str=toShift(StatusMessages)) {
    toMessageUI dialog(toMainWidget(),NULL,true);
    dialog.Message->setReadOnly(true);
#if QT_VERSION >= 0x030000
    dialog.Icon->setPixmap(QApplication::style().stylePixmap(QStyle::SP_MessageBoxWarning));
#endif
    dialog.Message->setText(str);
    dialog.exec();
    if (dialog.Statusbar->isChecked()) {
      toTool::globalSetConfig(CONF_MESSAGE_STATUSBAR,"Yes");
      TOMessageBox::information(toMainWidget(),
				tr("Information"),
				tr("You can enable this through the Global Settings in the Options (Edit menu)"));
      toTool::saveConfig();
    }
  }
  recursive=false;
}

void toMain::displayMessage(const QString &str)
{
  toPush(StatusMessages,str);
  QTimer::singleShot(1,this,SLOT(displayMessage()));
}

void toMain::updateKeepAlive(void)
{
  int keepAlive=toTool::globalConfig(CONF_KEEP_ALIVE,"0").toInt();
  if (KeepAlive.isActive())
    disconnect(&KeepAlive,SIGNAL(timeout()),this,SLOT(keepAlive()));
  if (keepAlive) {
    connect(&KeepAlive,SIGNAL(timeout()),this,SLOT(keepAlive()));
    KeepAlive.start(keepAlive*1000);
  }
}

class toMainNopExecutor : public toTask {
private:
  toConnection &Connection;
  QString SQL;
public:
  toMainNopExecutor(toConnection &conn,const QString &sql)
    : Connection(conn),SQL(sql)
  { }
  virtual void run(void)
  {
    try {
      Connection.allExecute(SQL);
    } TOCATCH
  }
};

void toMain::keepAlive(void)
{
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    toThread *thread=new toThread(new toMainNopExecutor(*(*i),toSQL::string("Global:Now",*(*i))));
    thread->start();
  }
}

void toMain::toolWidgetAdded(toToolWidget *tool)
{
  emit addedToolWidget(tool);
}

void toMain::toolWidgetRemoved(toToolWidget *tool)
{
  emit removedToolWidget(tool);
}
