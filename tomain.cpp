//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifdef TO_KDE
#include <kmenubar.h>
#include <kfiledialog.h>
#endif

#include <qapplication.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>
#include <qfileinfo.h>
#include <qvbox.h>
#include <qregexp.h>
#include <qinputdialog.h>

#include "toconf.h"
#include "tomain.h"
#include "totool.h"
#include "tonewconnection.h"
#include "toabout.h"
#include "topreferences.h"
#include "tosearchreplace.h"
#include "tohelp.h"
#include "tomemoeditor.h"
#include "toeditwidget.h"
#include "toconnection.h"

#include "tomain.moc"
#ifdef TO_KDE
#include "tomainwindow.kde.moc"
#else
#include "tomainwindow.moc"
#endif

#include "icons/connect.xpm"
#include "icons/disconnect.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/print.xpm"
#include "icons/commit.xpm"
#include "icons/rollback.xpm"
#include "icons/undo.xpm"
#include "icons/redo.xpm"
#include "icons/cut.xpm"
#include "icons/copy.xpm"
#include "icons/paste.xpm"
#include "icons/toramini.xpm"
#include "icons/up.xpm"
#include "icons/trash.xpm"

#define DEFAULT_TITLE "TOra %s"

const int toMain::TO_FILE_MENU		= 10;
const int toMain::TO_EDIT_MENU		= 20;
const int toMain::TO_TOOLS_MENU		= 30;
const int toMain::TO_WINDOWS_MENU	= 40;
const int toMain::TO_HELP_MENU		= 50;

const int toMain::TO_TOOL_MENU_ID	= 2000;
const int toMain::TO_TOOL_MENU_ID_END	= 2999;

const int toMain::TO_TOOL_ABOUT_ID	= 3000;
const int toMain::TO_TOOL_ABOUT_ID_END	= 3999;

#define TO_STATUS_ID		4000
#define TO_STATUS_ID_END	4999
#define TO_NEW_CONNECTION	100
#define TO_CLOSE_CONNECTION	101
#define TO_FILE_OPEN		102
#define TO_FILE_SAVE		103
#define TO_FILE_SAVE_AS		104
#define TO_FILE_COMMIT		105
#define TO_FILE_ROLLBACK	106
#define TO_FILE_CURRENT         107
#define TO_FILE_CLEARCACHE	108
#define TO_FILE_PRINT		109
#define TO_FILE_QUIT		110
#define TO_FILE_OPEN_SESSION	111
#define TO_FILE_SAVE_SESSION	112
#define TO_FILE_CLOSE_SESSION	113
#define TO_FILE_LAST_SESSION	114

#define TO_EDIT_UNDO		200
#define TO_EDIT_REDO		201
#define TO_EDIT_CUT		202
#define TO_EDIT_COPY		203
#define TO_EDIT_PASTE		204
#define TO_EDIT_SELECT_ALL	205
#define TO_EDIT_OPTIONS		206
#define TO_EDIT_READ_ALL	207
#define TO_EDIT_SEARCH		208
#define TO_EDIT_SEARCH_NEXT	209

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
#define TO_HELP_REGISTER	905

#define TO_TOOLS		1000
#define TO_ABOUT_ID_OFFSET	(toMain::TO_TOOL_ABOUT_ID-TO_TOOLS)

QString toCheckLicense(bool);
bool toFreeware(void);

toMain::toMain()
  : toMainWindow()
{
  qApp->setMainWidget(this);
  setDockMenuEnabled(true);

  Edit=NULL;

  FileMenu=new QPopupMenu(this);
  FileMenu->insertItem(QPixmap((const char **)connect_xpm),
		       "&New Connection...",TO_NEW_CONNECTION);
  FileMenu->insertItem(QPixmap((const char **)disconnect_xpm),
		       "&Close Connection",this,SLOT(delConnection()),0,TO_CLOSE_CONNECTION);
  FileMenu->insertSeparator();
  FileMenu->insertItem(QPixmap((const char **)commit_xpm),"&Commit connection",TO_FILE_COMMIT);
  FileMenu->insertItem(QPixmap((const char **)rollback_xpm),"&Rollback connection",TO_FILE_ROLLBACK);
  FileMenu->insertItem("C&urrent connection",TO_FILE_CURRENT);
  FileMenu->insertItem(QPixmap((const char **)trash_xpm),"Reread object cache",TO_FILE_CLEARCACHE);
  FileMenu->insertSeparator();
  FileMenu->insertItem(QPixmap((const char **)fileopen_xpm),"&Open File...",TO_FILE_OPEN);
  FileMenu->insertItem(QPixmap((const char **)filesave_xpm),"&Save",TO_FILE_SAVE);
  FileMenu->insertItem("Save A&s..",TO_FILE_SAVE_AS);
  FileMenu->insertSeparator();
  FileMenu->insertItem(QPixmap((const char **)fileopen_xpm),"Open Session...",
		       TO_FILE_OPEN_SESSION);
  FileMenu->insertItem(QPixmap((const char **)filesave_xpm),"Save Session...",
		       TO_FILE_SAVE_SESSION);
  FileMenu->insertItem("Restore last session",TO_FILE_LAST_SESSION);
  FileMenu->insertItem("Close Session",TO_FILE_CLOSE_SESSION);
  FileMenu->insertSeparator();
  FileMenu->insertItem(QPixmap((const char **)print_xpm),"&Print..",TO_FILE_PRINT);
  FileMenu->insertSeparator();
  FileMenu->insertItem("&Quit",TO_FILE_QUIT);
  menuBar()->insertItem("&File",FileMenu,TO_FILE_MENU);
  FileMenu->setAccel(Key_G|CTRL,TO_NEW_CONNECTION);
  FileMenu->setAccel(Key_O|CTRL,TO_FILE_OPEN);
  FileMenu->setAccel(Key_S|CTRL,TO_FILE_SAVE);
  FileMenu->setAccel(Key_W|CTRL,TO_FILE_COMMIT);
  FileMenu->setAccel(Key_R|CTRL,TO_FILE_ROLLBACK);
  FileMenu->setAccel(Key_U|CTRL,TO_FILE_CURRENT);
  connect(FileMenu,SIGNAL(aboutToShow()),this,SLOT( editFileMenu()));

  EditMenu=new QPopupMenu(this);
  EditMenu->insertItem(QPixmap((const char **)undo_xpm),"&Undo",TO_EDIT_UNDO);
  EditMenu->insertItem(QPixmap((const char **)redo_xpm),"&Redo",TO_EDIT_REDO);
  EditMenu->insertSeparator();
  EditMenu->insertItem(QPixmap((const char **)cut_xpm),"Cu&t",TO_EDIT_CUT);
  EditMenu->insertItem(QPixmap((const char **)copy_xpm),"&Copy",TO_EDIT_COPY);
  EditMenu->insertItem(QPixmap((const char **)paste_xpm),"&Paste",TO_EDIT_PASTE);
  EditMenu->insertSeparator();
  EditMenu->insertItem("&Search && Replace",TO_EDIT_SEARCH);
  EditMenu->insertItem("Search &Next",TO_EDIT_SEARCH_NEXT);
  EditMenu->insertItem("Select &All",TO_EDIT_SELECT_ALL);
  EditMenu->insertItem("Read All &Items",TO_EDIT_READ_ALL);
  EditMenu->insertSeparator();
  EditMenu->insertItem("&Options",TO_EDIT_OPTIONS);
  EditMenu->setAccel(Key_Z|CTRL,TO_EDIT_UNDO);
  EditMenu->setAccel(Key_Y|CTRL,TO_EDIT_REDO);
  EditMenu->setAccel(Key_X|CTRL,TO_EDIT_CUT);
  EditMenu->setAccel(Key_C|CTRL,TO_EDIT_COPY);
  EditMenu->setAccel(Key_V|CTRL,TO_EDIT_PASTE);
  EditMenu->setAccel(Key_F|CTRL,TO_EDIT_SEARCH);
  EditMenu->setAccel(Key_F3,TO_EDIT_SEARCH_NEXT);
  connect(EditMenu,SIGNAL(aboutToShow()),this,SLOT( editFileMenu()));
  menuBar()->insertItem("&Edit",EditMenu,TO_EDIT_MENU);

  std::map<QString,toTool *> &tools=toTool::tools();

  EditToolbar=toAllocBar(this,"Application",QString::null);

  LoadButton=new QToolButton(QPixmap((const char **)fileopen_xpm),
			     "Load file into editor",
			     "Load file into editor",
			     this,SLOT(loadButton()),EditToolbar);
  SaveButton=new QToolButton(QPixmap((const char **)filesave_xpm),
			     "Save file from editor",
			     "Save file from editor",
			     this,SLOT(saveButton()),EditToolbar);
  PrintButton=new QToolButton(QPixmap((const char **)print_xpm),
			     "Print",
			     "Print",
			     this,SLOT(printButton()),EditToolbar);
  PrintButton->setEnabled(false);
  LoadButton->setEnabled(false);
  SaveButton->setEnabled(false);
  EditToolbar->addSeparator();
  UndoButton=new QToolButton(QPixmap((const char **)undo_xpm),
			     "Undo",
			     "Undo",
			     this,SLOT(undoButton()),EditToolbar);
  RedoButton=new QToolButton(QPixmap((const char **)redo_xpm),
			     "Redo",
			     "Redo",
			     this,SLOT(redoButton()),EditToolbar);
  CutButton=new QToolButton(QPixmap((const char **)cut_xpm),
			     "Cut to clipboard",
			     "Cut to clipboard",
			     this,SLOT(cutButton()),EditToolbar);
  CopyButton=new QToolButton(QPixmap((const char **)copy_xpm),
			     "Copy to clipboard",
			     "Copy to clipboard",
			     this,SLOT(copyButton()),EditToolbar);
  PasteButton=new QToolButton(QPixmap((const char **)paste_xpm),
			      "Paste from clipboard",
			      "Paste from clipboard",
			      this,SLOT(pasteButton()),EditToolbar);
  UndoButton->setEnabled(false);
  RedoButton->setEnabled(false);
  CutButton->setEnabled(false);
  CopyButton->setEnabled(false);
  PasteButton->setEnabled(false);

  ToolsMenu=new QPopupMenu(this);

  QToolBar *toolbar=toAllocBar(this,"Tools",QString::null);
  if (!toTool::globalConfig(CONF_TOOLS_LEFT,"Yes").isEmpty())
    moveToolBar(toolbar,Left);

  int toolID=TO_TOOLS;
  int lastPriorityPix=0;
  int lastPriorityMenu=0;
  SQLEditor=-1;
  DefaultTool=toolID;
  QString defName=toTool::globalConfig(CONF_DEFAULT_TOOL,"");

  HelpMenu=new QPopupMenu(this);
  HelpMenu->insertItem("C&urrent Context",TO_HELP_CONTEXT);
  HelpMenu->insertItem("&Contents",TO_HELP_CONTENTS);
  HelpMenu->insertSeparator();
  HelpMenu->insertItem("&About TOra",TO_HELP_ABOUT);
  HelpMenu->insertItem("&License",TO_HELP_LICENSE);
  HelpMenu->insertItem("&Quotes",TO_HELP_QUOTES);
  HelpMenu->setAccel(Key_F1,TO_HELP_CONTEXT);
  if (!toFreeware()) {
    HelpMenu->insertSeparator();
    HelpMenu->insertItem("&Register",TO_HELP_REGISTER);
  }
  QPopupMenu *toolAbout=NULL;

  for (std::map<QString,toTool *>::iterator i=tools.begin();i!=tools.end();i++) {
    const QPixmap *pixmap=(*i).second->toolbarImage();
    const char *toolTip=(*i).second->toolbarTip();
    const char *menuName=(*i).second->menuItem();

    QString tmp=(*i).second->name();
    tmp+=CONF_TOOL_ENABLE;
    if (toTool::globalConfig(tmp,"Yes").isEmpty())
      continue;

    if (defName==menuName)
      DefaultTool=toolID;

    int priority=(*i).second->priority();
    if (priority/100!=lastPriorityPix/100&&
	pixmap) {
      toolbar->addSeparator();
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
      NeedConnection[new QToolButton(*pixmap,
				     toolTip,
				     toolTip,
				     (*i).second,
				     SLOT(createWindow(void)),
				     toolbar)]=(*i).second;
    }

    if (menuName) {
      if (pixmap)
	ToolsMenu->insertItem(*pixmap,menuName,toolID);
      else
	ToolsMenu->insertItem(menuName,toolID);
      ToolsMenu->setItemEnabled(toolID,false);
    }

    if ((*i).second->hasAbout()&&menuName) {
      if (!toolAbout) {
	toolAbout=new QPopupMenu(this);
	HelpMenu->insertItem("Tools",toolAbout);
      }
      if (pixmap)
	toolAbout->insertItem(*pixmap,menuName,toolID+TO_ABOUT_ID_OFFSET);
      else
	toolAbout->insertItem(menuName,toolID+TO_ABOUT_ID_OFFSET);
    }

    Tools[toolID]=(*i).second;

    toolID++;
  }

#ifndef TOOL_TOOLBAR
  toolbar->setStretchableWidget(new QLabel("",toolbar));
#endif


  ConnectionToolbar=toAllocBar(this,"Connections",QString::null);
  new QToolButton(QPixmap((const char **)connect_xpm),
		  "Connect to database",
		  "Connect to database",
		  this,SLOT(addConnection()),ConnectionToolbar);
  DisconnectButton=new QToolButton(QPixmap((const char **)disconnect_xpm),
				   "Disconnect current connection",
				   "Disconnect current connection",
				   this,SLOT(delConnection()),ConnectionToolbar);
  DisconnectButton->setEnabled(false);
  ConnectionToolbar->addSeparator();
  NeedConnection[new QToolButton(QPixmap((const char **)commit_xpm),
				 "Commit connection",
				 "Commit connection",
				 this,SLOT(commitButton()),ConnectionToolbar)]=NULL;
  NeedConnection[new QToolButton(QPixmap((const char **)rollback_xpm),
				 "Rollback connection",
				 "Rollback connection",
				 this,SLOT(rollbackButton()),ConnectionToolbar)]=NULL;
  ConnectionToolbar->addSeparator();
  ConnectionSelection=new QComboBox(ConnectionToolbar);
  ConnectionSelection->setFixedWidth(200);
  ConnectionSelection->setFocusPolicy(NoFocus);
  connect(ConnectionSelection,SIGNAL(activated(int)),this,SLOT(changeConnection()));

  menuBar()->insertItem("&Tools",ToolsMenu,TO_TOOLS_MENU);

  WindowsMenu=new QPopupMenu(this);
  WindowsMenu->setCheckable(true);
  connect(WindowsMenu,SIGNAL(aboutToShow()),this,SLOT( windowsMenu()));
  menuBar()->insertItem("&Windows",WindowsMenu,TO_WINDOWS_MENU);

  menuBar()->insertSeparator();

  menuBar()->insertItem("&Help",HelpMenu,TO_HELP_MENU);

  char buffer[100];
  sprintf(buffer,DEFAULT_TITLE,TOVERSION);
  setCaption(buffer);

#ifdef TO_KDE
  KDockWidget *mainDock=createDockWidget(buffer,QPixmap((const char **)toramini_xpm));
  Workspace=new QWorkspace(mainDock);
  mainDock->setWidget(Workspace);
  setView(mainDock);
  setMainDockWidget(mainDock);
  mainDock->setEnableDocking(KDockWidget::DockNone);
#else
  Workspace=new QWorkspace(this);
  setCentralWidget(Workspace);
#endif
  setIcon(QPixmap((const char **)toramini_xpm));

  statusBar()->message("Ready");
  menuBar()->setItemEnabled(TO_CLOSE_CONNECTION,false);
  menuBar()->setItemEnabled(TO_FILE_COMMIT,false);
  menuBar()->setItemEnabled(TO_FILE_ROLLBACK,false);
  menuBar()->setItemEnabled(TO_FILE_CLEARCACHE,false);
  DisconnectButton->setEnabled(false);

  for (std::map<QToolButton *,toTool *>::iterator j=NeedConnection.begin();
       j!=NeedConnection.end();j++)
    (*j).first->setEnabled(false);

  connect(menuBar(),SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  RowLabel=new QLabel(statusBar());
  statusBar()->addWidget(RowLabel,0,true);
  RowLabel->setMinimumWidth(60);
  RowLabel->hide();

  ColumnLabel=new QLabel(statusBar());
  statusBar()->addWidget(ColumnLabel,0,true);
  ColumnLabel->setMinimumWidth(60);
  ColumnLabel->hide();

  QToolButton *dispStatus=new QToolButton(statusBar());
  dispStatus->setIconSet(QPixmap((const char **)up_xpm));
  statusBar()->addWidget(dispStatus,0,true);
  StatusMenu=new QPopupMenu(dispStatus);
  dispStatus->setPopup(StatusMenu);
  dispStatus->setPopupDelay(0);
  connect(StatusMenu,SIGNAL(aboutToShow()),
	  this,SLOT(statusMenu()));
  connect(StatusMenu,SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  toolID=TO_TOOLS;
  for (std::map<QString,toTool *>::iterator k=tools.begin();k!=tools.end();k++) {
    (*k).second->customSetup(toolID);
    toolID++;
  }
  Search=NULL;

  if (!toTool::globalConfig(CONF_MAXIMIZE_MAIN,"Yes").isEmpty())
    showMaximized();
  show();

  QString welcome;

  do {
    welcome=toCheckLicense(false);
  } while(welcome.isNull());

  toStatusMessage(welcome,true);

  connect(&Poll,SIGNAL(timeout()),this,SLOT(checkCaching()));
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  if (!toTool::globalConfig(CONF_RESTORE_SESSION,"").isEmpty()) {
    try {
      std::map<QString,QString> session;
      if (toTool::loadMap(toTool::globalConfig(CONF_DEFAULT_SESSION,DEFAULT_SESSION),session))
	importData(session,"TOra");
    } TOCATCH
  }
  if (Connections.size()==0) {
    try {
      toNewConnection newConnection(this,"First connection",true);
    
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
    toConnection &conn=tool->connection();
    int pos=0;
    for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
      if (&conn==*i) {
	ConnectionSelection->setCurrentItem(pos);
	break;
      }
      pos++;
    }
  }
}

void toMain::editFileMenu(void)	// Ugly hack to disable edit with closed child windows
{
  menuBar()->setItemEnabled(TO_EDIT_UNDO,Edit&&Edit->undoEnabled());
  menuBar()->setItemEnabled(TO_EDIT_REDO,Edit&&Edit->redoEnabled());
  menuBar()->setItemEnabled(TO_EDIT_CUT,Edit&&Edit->cutEnabled());
  menuBar()->setItemEnabled(TO_EDIT_COPY,Edit&&Edit->copyEnabled());
  menuBar()->setItemEnabled(TO_EDIT_PASTE,Edit&&Edit->pasteEnabled());
  menuBar()->setItemEnabled(TO_EDIT_SELECT_ALL,Edit&&Edit->selectAllEnabled());
  menuBar()->setItemEnabled(TO_EDIT_READ_ALL,Edit&&Edit->readAllEnabled());
  menuBar()->setItemEnabled(TO_EDIT_SEARCH,Edit&&Edit->searchEnabled());
  menuBar()->setItemEnabled(TO_FILE_OPEN,Edit&&Edit->openEnabled());
  menuBar()->setItemEnabled(TO_FILE_SAVE,Edit&&Edit->saveEnabled());
  menuBar()->setItemEnabled(TO_FILE_SAVE_AS,Edit&&Edit->saveEnabled());
  menuBar()->setItemEnabled(TO_EDIT_SEARCH_NEXT,
			    Search&&Search->searchNextAvailable());
}

void toMain::windowsMenu(void)
{
  WindowsMenu->clear();

  WindowsMenu->insertItem("C&lose",TO_WINDOWS_CLOSE);
  if (!workspace()->activeWindow())
    WindowsMenu->setItemEnabled(TO_WINDOWS_CLOSE,false);

  WindowsMenu->insertItem("Close &All",TO_WINDOWS_CLOSE_ALL);
  WindowsMenu->insertSeparator();
  WindowsMenu->insertItem("&Cascade",TO_WINDOWS_CASCADE);
  WindowsMenu->insertItem("&Tile",TO_WINDOWS_TILE);
  if (workspace()->windowList().isEmpty()) {
    WindowsMenu->setItemEnabled(TO_WINDOWS_CASCADE,false);
    WindowsMenu->setItemEnabled(TO_WINDOWS_TILE,false);
    WindowsMenu->setItemEnabled(TO_WINDOWS_CLOSE_ALL,false);
  } else {
    WindowsMenu->insertSeparator();
    QRegExp strip(" <[0-9]+>$");
    for (unsigned int i=0;i<workspace()->windowList().count();i++) {
      QWidget *widget=workspace()->windowList().at(i);
      QString caption=widget->caption();
      caption.replace(strip,"");
      WindowsMenu->insertItem(caption,TO_WINDOWS_WINDOWS+i);
      WindowsMenu->setItemChecked(TO_WINDOWS_WINDOWS+i,workspace()->activeWindow()==workspace()->windowList().at(i));
      if (i<9) {
	WindowsMenu->setAccel(Key_1+i|CTRL,TO_WINDOWS_WINDOWS+i);
	caption+=" <";
	caption+=QString::number(i+1);
	caption+=">";
      }
      widget->setCaption(caption);
    }
  }
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
    if (cmd-TO_WINDOWS_WINDOWS<int(workspace()->windowList().count()))
      workspace()->windowList().at(cmd-TO_WINDOWS_WINDOWS)->setFocus();
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
	edit->editSearch(Search);
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
    switch(cmd) {
    case TO_FILE_COMMIT:
      try {
	toConnection &conn=currentConnection();
	emit willCommit(conn,true);
	conn.commit();
	setNeedCommit(conn,false);
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
      toHelp::displayHelp("toc.htm");
      break;
    case TO_HELP_ABOUT:
    case TO_HELP_LICENSE:
    case TO_HELP_QUOTES:
      {
	toAbout *about=new toAbout(cmd-TO_HELP_ABOUT,this,"About TOra",true);
	about->exec();
	delete about;
      }
      break;
    case TO_HELP_REGISTER:
      toCheckLicense(true);
      break;
    case TO_EDIT_OPTIONS:
      toPreferences::displayPreferences(this);
      break;
    case TO_WINDOWS_CLOSE_ALL:
      while (workspace()->windowList().count()>0)
	delete workspace()->windowList().at(0);
      break;
    case TO_WINDOWS_CLOSE:
      {
	QWidget *widget=workspace()->activeWindow();
	if (widget)
	  delete widget;
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
	std::map<QString,QString> session;
	if (toTool::loadMap(toTool::globalConfig(CONF_DEFAULT_SESSION,DEFAULT_SESSION),session))
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
  throw QString("Can't find active connection");
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
    menuBar()->setItemEnabled(TO_FILE_COMMIT,true);
    menuBar()->setItemEnabled(TO_FILE_ROLLBACK,true);
    menuBar()->setItemEnabled(TO_FILE_CLEARCACHE,true);
    menuBar()->setItemEnabled(TO_CLOSE_CONNECTION,true);
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
	dsc+=" *";
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
	QString str("Commit work in session to ");
	str.append(conn->description());
	str.append(" before closing it?");
	switch(TOMessageBox::warning(this,"Commit work?",str,"&Yes","&No","&Cancel")) {
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
    menuBar()->setItemEnabled(TO_FILE_COMMIT,false);
    menuBar()->setItemEnabled(TO_FILE_ROLLBACK,false);
    menuBar()->setItemEnabled(TO_FILE_CLEARCACHE,false);
    menuBar()->setItemEnabled(TO_CLOSE_CONNECTION,false);
    DisconnectButton->setEnabled(false);
    for (std::map<QToolButton *,toTool *>::iterator i=NeedConnection.begin();
	 i!=NeedConnection.end();i++)
      (*i).first->setEnabled(false);
    for (std::map<int,toTool *>::iterator j=Tools.begin();j!=Tools.end();j++)
      menuBar()->setItemEnabled((*j).first,false);
  }
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
  throw QString("Couldn't find specified connectionts (%1)").arg(str);
}

void toMain::loadButton(void)
{
  commandCallback(TO_FILE_OPEN);
}

void toMain::commitButton(void)
{
  commandCallback(TO_FILE_COMMIT);
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

void toMain::pasteButton(void)
{
  commandCallback(TO_EDIT_PASTE);
}

void toMain::printButton(void)
{
  commandCallback(TO_FILE_PRINT);
}

void toMain::setEditWidget(toEditWidget *edit)
{
  toMain *main=(toMain *)qApp->mainWidget();
  if (main&&edit) {
    if (main->Edit)
      main->Edit->lostFocus();
    main->Edit=edit;
    main->RowLabel->hide();
    main->ColumnLabel->hide();
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
			bool,bool,bool)
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
  }
}

void toMain::registerSQLEditor(int tool)
{
  SQLEditor=tool;
}

bool toMain::close(bool del)
{
  std::map<QString,QString> session;
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
  return QMainWindow::close(del);
}

void toMain::createDefault(void)
{
  commandCallback(DefaultTool);
}

void toMain::setCoordinates(int line,int col)
{
  QString str("Row: ");
  str+=QString::number(line);
  RowLabel->setText(str);
  str="Col:";
  str+=QString::number(col);
  ColumnLabel->setText(str);
  RowLabel->show();
  ColumnLabel->show();
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
  toConnection &conn=currentConnection();
  for (std::map<QToolButton *,toTool *>::iterator i=NeedConnection.begin();
       i!=NeedConnection.end();i++) {
    toTool *tool=(*i).second;
    if (!tool)
      (*i).first->setEnabled(true);
    else if (tool->canHandle(conn))
      (*i).first->setEnabled(true);
    else
      (*i).first->setEnabled(false);
  }  
  for (std::map<int,toTool *>::iterator j=Tools.begin();j!=Tools.end();j++) {
    toTool *tool=(*j).second;
    if (!tool)
      menuBar()->setItemEnabled((*j).first,true);
    else if (tool->canHandle(conn))
      menuBar()->setItemEnabled((*j).first,true);
    else
      menuBar()->setItemEnabled((*j).first,false);

  }
}

void toMain::checkCaching(void)
{
  int num=0;
  for(std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if (!(*i)->cacheAvailable(false,false))
      num++;
  }
  if (num==0)
    Poll.stop();
  else
    Poll.start(100);
}

void toMain::exportData(std::map<QString,QString> &data,const QString &prefix)
{
  if (isMaximized())
    data[prefix+":State"]="Maximized";
  else if (isMinimized())
    data[prefix+":State"]="Minimized";
  else {
    data[prefix+":X"]=QString::number(x());
    data[prefix+":Y"]=QString::number(y());
    data[prefix+":Width"]=QString::number(width());
    data[prefix+":Height"]=QString::number(height());
  }

  int id=1;
  std::map<toConnection *,int> connMap;
  for(std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    QString key=prefix+":Connection:"+QString::number(id);
    if (toTool::globalConfig(CONF_SAVE_PWD,DEFAULT_SAVE_PWD)!=DEFAULT_SAVE_PWD)
      data[key+":Password"]=(*i)->password();
    data[key+":User"]=(*i)->user();
    data[key+":Host"]=(*i)->host();
    data[key+":Mode"]=(*i)->mode();
    data[key+":Database"]=(*i)->database();
    data[key+":Provider"]=(*i)->provider();
    connMap[*i]=id;
    id++;
  }

  id=1;
  for (unsigned int i=0;i<workspace()->windowList().count();i++) {
    toToolWidget *tool=dynamic_cast<toToolWidget *>(workspace()->windowList().at(i));
    if (tool) {
      QString key=prefix+":Tools:"+QString::number(id);
      tool->exportData(data,key);
      data[key+":Type"]=tool->tool().key();
      data[key+":Connection"]=QString::number(connMap[&tool->connection()]);
    }
    id++;
  }
}

void toMain::importData(std::map<QString,QString> &data,const QString &prefix)
{
  if (data[prefix+":State"]=="Maximized")
    showMaximized();
  else if (data[prefix+":State"]=="Minimized")
    showMinimized();
  else {
    showNormal();
    setGeometry(data[prefix+":X"].toInt(),
		data[prefix+":Y"].toInt(),
		data[prefix+":Width"].toInt(),
		data[prefix+":Height"].toInt());
  }

  std::map<int,toConnection *> connMap;

  int id=1;
  std::map<QString,QString>::iterator i;
  while((i=data.find(prefix+":Connection:"+QString::number(id)+":Database"))!=data.end()) {
    QString key=prefix+":Connection:"+QString::number(id);
    QString database=(*i).second;
    QString user=data[key+":User"];
    QString host=data[key+":Host"];
    QString mode=data[key+":Mode"];
    QString password=data[key+":Password"];
    QString provider=data[key+":Provider"];
    bool ok=true;
    if (toTool::globalConfig(CONF_SAVE_PWD,DEFAULT_SAVE_PWD)==password) {
      password=QInputDialog::getText("Input password",
				     "Enter password for "+database,
				     QLineEdit::Password,
				     DEFAULT_SAVE_PWD,
				     &ok,
				     this);
    }
    if (ok) {
      try {
	toConnection *conn=new toConnection(provider,user,password,host,database,mode);
	if (conn) {
	  conn=addConnection(conn,false);
	  connMap[id]=conn;
	}
      } TOCATCH
    }
    id++;
  }

  id=1;
  while((i=data.find(prefix+":Tools:"+QString::number(id)+":Type"))!=data.end()) {
    QString key=(*i).second;
    int connid=data[prefix+":Tools:"+QString::number(id)+":Connection"].toInt();
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
	    tw->importData(data,prefix+":Tools:"+QString::number(id));
	  }
	}
      }
    }
    id++;
  }
  windowsMenu();
}

void toMain::saveSession(void)
{
  QString fn=toSaveFilename(QString::null,"*.tse",this);
  if (!fn.isEmpty()) {
    std::map<QString,QString> session;
    exportData(session,"TOra");
    try {
      toTool::saveMap(fn,session);
    } TOCATCH
  }
}

void toMain::loadSession(void)
{
  QString filename=toOpenFilename(QString::null,"*.tse",this);
  if (!filename.isEmpty()) {
    try {
      std::map<QString,QString> session;
      if (toTool::loadMap(filename,session))
	importData(session,"TOra");
    } TOCATCH
  }
}

void toMain::closeSession(void)
{
  std::map<QString,QString> session;
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
      return;
  while (Connections.end()!=Connections.begin()) {
    if (!delConnection())
      return;
  }
}
