//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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

#include "toconf.h"
#include "tomain.h"
#include "totool.h"
#include "tonewconnection.h"
#include "tomarkedtext.h"
#include "toabout.h"
#include "topreferences.h"
#include "toresultview.h"
#include "toresultcontent.h"
#include "tosearchreplace.h"
#include "tohelp.h"

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

#define TO_NEW_CONNECTION	100
#define TO_CLOSE_CONNECTION	101
#define TO_FILE_OPEN		102
#define TO_FILE_SAVE		103
#define TO_FILE_SAVE_AS		104
#define TO_FILE_COMMIT		105
#define TO_FILE_ROLLBACK	106
#define TO_FILE_PRINT		107
#define TO_FILE_QUIT		108

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

#define TO_TOOLS		1000
#define TO_ABOUT_ID_OFFSET	(toMain::TO_TOOL_ABOUT_ID-TO_TOOLS)

static QPixmap *toLoadPixmap;
static QPixmap *toSavePixmap;
static QPixmap *toConnectPixmap;
static QPixmap *toDisconnectPixmap;
static QPixmap *toCommitPixmap;
static QPixmap *toRollbackPixmap;
static QPixmap *toUndoPixmap;
static QPixmap *toRedoPixmap;
static QPixmap *toCutPixmap;
static QPixmap *toCopyPixmap;
static QPixmap *toPastePixmap;
static QPixmap *toPrintPixmap;
static QPixmap *toTOraPixmap;
static QPixmap *toUpPixmap;

static toResultContent *toContent(QWidget *widget)
{
  while(widget) {
    if (widget->inherits("toResultContent")) {
      toResultContent *ret=dynamic_cast<toResultContent *>(widget);
      if (ret)
        return ret;
    }
    widget=widget->parentWidget();
  }
  return NULL;
}

toMain::toMain()
  : toMainWindow()
{
  if (!toConnectPixmap)
    toConnectPixmap=new QPixmap((const char **)connect_xpm);
  if (!toDisconnectPixmap)
    toDisconnectPixmap=new QPixmap((const char **)disconnect_xpm);
  if (!toLoadPixmap)
    toLoadPixmap=new QPixmap((const char **)fileopen_xpm);
  if (!toSavePixmap)
    toSavePixmap=new QPixmap((const char **)filesave_xpm);
  if (!toRollbackPixmap)
    toRollbackPixmap=new QPixmap((const char **)rollback_xpm);
  if (!toCommitPixmap)
    toCommitPixmap=new QPixmap((const char **)commit_xpm);
  if (!toUndoPixmap)
    toUndoPixmap=new QPixmap((const char **)undo_xpm);
  if (!toRedoPixmap)
    toRedoPixmap=new QPixmap((const char **)redo_xpm);
  if (!toCutPixmap)
    toCutPixmap=new QPixmap((const char **)cut_xpm);
  if (!toCopyPixmap)
    toCopyPixmap=new QPixmap((const char **)copy_xpm);
  if (!toPastePixmap)
    toPastePixmap=new QPixmap((const char **)paste_xpm);
  if (!toPrintPixmap)
    toPrintPixmap=new QPixmap((const char **)print_xpm);
  if (!toTOraPixmap)
    toTOraPixmap=new QPixmap((const char **)toramini_xpm);
  if (!toUpPixmap)
    toUpPixmap=new QPixmap((const char **)up_xpm);

  qApp->setMainWidget(this);
  setDockMenuEnabled(true);

  FileMenu=new QPopupMenu(this);
  FileMenu->insertItem(*toConnectPixmap,"&New Connection...",TO_NEW_CONNECTION);
  FileMenu->insertItem(*toDisconnectPixmap,"&Close Connection",this,SLOT(delConnection()),0,TO_CLOSE_CONNECTION);
  FileMenu->insertSeparator();
  FileMenu->insertItem(*toCommitPixmap,"&Commit connection",TO_FILE_COMMIT);
  FileMenu->insertItem(*toRollbackPixmap,"&Rollback connection",TO_FILE_ROLLBACK);
  FileMenu->insertSeparator();
  FileMenu->insertItem(*toLoadPixmap,"&Open File...",TO_FILE_OPEN);
  FileMenu->insertItem(*toSavePixmap,"&Save",TO_FILE_SAVE);
  FileMenu->insertItem("&Save As..",TO_FILE_SAVE_AS);
  FileMenu->insertSeparator();
  FileMenu->insertItem(*toPrintPixmap,"&Print..",TO_FILE_PRINT);
  FileMenu->insertSeparator();
  FileMenu->insertItem("&Quit",TO_FILE_QUIT);
  menuBar()->insertItem("&File",FileMenu,TO_FILE_MENU);
  FileMenu->setAccel(Key_G|CTRL,TO_NEW_CONNECTION);
  FileMenu->setAccel(Key_O|CTRL,TO_FILE_OPEN);
  FileMenu->setAccel(Key_S|CTRL,TO_FILE_SAVE);
  connect(FileMenu,SIGNAL(aboutToShow()),this,SLOT( editFileMenu()));

  EditMenu=new QPopupMenu(this);
  EditMenu->insertItem(*toUndoPixmap,"&Undo",TO_EDIT_UNDO);
  EditMenu->insertItem(*toRedoPixmap,"&Redo",TO_EDIT_REDO);
  EditMenu->insertSeparator();
  EditMenu->insertItem(*toCutPixmap,"Cu&t",TO_EDIT_CUT);
  EditMenu->insertItem(*toCopyPixmap,"&Copy",TO_EDIT_COPY);
  EditMenu->insertItem(*toPastePixmap,"&Paste",TO_EDIT_PASTE);
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

  QToolBar *toolbar=toAllocBar(this,"Application",QString::null);

  LoadButton=new QToolButton(*toLoadPixmap,
			     "Load file into editor",
			     "Load file into editor",
			     this,SLOT(loadButton()),toolbar);
  SaveButton=new QToolButton(*toSavePixmap,
			     "Save file from editor",
			     "Save file from editor",
			     this,SLOT(saveButton()),toolbar);
  PrintButton=new QToolButton(*toPrintPixmap,
			     "Print",
			     "Print",
			     this,SLOT(printButton()),toolbar);
  PrintButton->setEnabled(false);
  LoadButton->setEnabled(false);
  SaveButton->setEnabled(false);
  toolbar->addSeparator();
  UndoButton=new QToolButton(*toUndoPixmap,
			     "Undo",
			     "Undo",
			     this,SLOT(undoButton()),toolbar);
  RedoButton=new QToolButton(*toRedoPixmap,
			     "Redo",
			     "Redo",
			     this,SLOT(redoButton()),toolbar);
  CutButton=new QToolButton(*toCutPixmap,
			     "Cut to clipboard",
			     "Cut to clipboard",
			     this,SLOT(cutButton()),toolbar);
  CopyButton=new QToolButton(*toCopyPixmap,
			     "Copy to clipboard",
			     "Copy to clipboard",
			     this,SLOT(copyButton()),toolbar);
  PasteButton=new QToolButton(*toPastePixmap,
			     "Paste from clipboard",
			     "Paste from clipboard",
			     this,SLOT(pasteButton()),toolbar);
  UndoButton->setEnabled(false);
  RedoButton->setEnabled(false);
  CutButton->setEnabled(false);
  CopyButton->setEnabled(false);
  PasteButton->setEnabled(false);

  ToolsMenu=new QPopupMenu(this);

#ifdef TOOL_TOOLBAR
  toolbar=new toAllocBar(this,"Tools",QString::null);
#else
  toolbar->addSeparator();
#endif

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

  new QToolButton(*toConnectPixmap,
		  "Connect to database",
		  "Connect to database",
		  this,SLOT(addConnection()),toolbar);
  DisconnectButton=new QToolButton(*toDisconnectPixmap,
				   "Disconnect current connection",
				   "Disconnect current connection",
				   this,SLOT(delConnection()),toolbar);
  DisconnectButton->setEnabled(false);
  toolbar->addSeparator();
  NeedConnection[new QToolButton(*toCommitPixmap,
				 "Commit connection",
				 "Commit connection",
				 this,SLOT(commitButton()),toolbar)]=NULL;
  NeedConnection[new QToolButton(*toRollbackPixmap,
				 "Rollback connection",
				 "Rollback connection",
				 this,SLOT(rollbackButton()),toolbar)]=NULL;
  toolbar->addSeparator();
  ConnectionSelection=new QComboBox(toolbar);
  ConnectionSelection->setFixedWidth(200);
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
  KDockWidget *mainDock=createDockWidget(buffer,*toTOraPixmap);
  Workspace=new QWorkspace(mainDock);
  mainDock->setWidget(Workspace);
  setView(mainDock);
  setMainDockWidget(mainDock);
  mainDock->setEnableDocking(KDockWidget::DockNone);
#else
  Workspace=new QWorkspace(this);
  setCentralWidget(Workspace);
#endif
  setIcon(*toTOraPixmap);

  statusBar()->message("Ready");
  menuBar()->setItemEnabled(TO_CLOSE_CONNECTION,false);
  menuBar()->setItemEnabled(TO_FILE_COMMIT,false);
  menuBar()->setItemEnabled(TO_FILE_ROLLBACK,false);
  DisconnectButton->setEnabled(false);

  for (std::map<QToolButton *,toTool *>::iterator j=NeedConnection.begin();
       j!=NeedConnection.end();j++)
    (*j).first->setEnabled(false);

  connect(menuBar(),SIGNAL(activated(int)),this,SLOT(commandCallback(int)));

  QToolButton *dispStatus=new QToolButton(statusBar());
  dispStatus->setIconSet(*toUpPixmap);
  statusBar()->addWidget(dispStatus,0,true);
  StatusMenu=new QPopupMenu(dispStatus);
  dispStatus->setPopup(StatusMenu);
  dispStatus->setPopupDelay(0);
  connect(StatusMenu,SIGNAL(aboutToShow()),
	  this,SLOT(statusMenu()));

  RowLabel=new QLabel(statusBar());
  statusBar()->addWidget(RowLabel,0,true);
  RowLabel->setMinimumWidth(60);
  RowLabel->show();

  ColumnLabel=new QLabel(statusBar());
  statusBar()->addWidget(ColumnLabel,0,true);
  ColumnLabel->setMinimumWidth(60);
  ColumnLabel->show();

  toolID=TO_TOOLS;
  for (std::map<QString,toTool *>::iterator k=tools.begin();k!=tools.end();k++) {
    (*k).second->customSetup(toolID);
    toolID++;
  }
  Search=NULL;

  show();
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
    
    if (conn) {
      addConnection(conn);
      toTool::saveConfig();
    }
  } TOCATCH
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  toStatusMessage("Welcome to Tora",true);
}

void toMain::windowActivated(QWidget *widget)
{
  if (toTool::globalConfig(CONF_CHANGE_CONNECTION,"Yes").isEmpty())
    return;
  try {
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
  } catch(...) {

  }
}

void toMain::editFileMenu(void)	// Ugly hack to disable edit with closed child windows
{
  QWidget *currWidget=qApp->focusWidget();
  if (currWidget && currWidget->inherits("toMarkedText")) {
    toMarkedText *marked=dynamic_cast<toMarkedText *>(currWidget);
    bool readOnly=marked->isReadOnly();
    menuBar()->setItemEnabled(TO_EDIT_UNDO,!readOnly&&marked->getUndoAvailable());
    menuBar()->setItemEnabled(TO_EDIT_REDO,!readOnly&&marked->getRedoAvailable());
    if (marked->hasMarkedText()) {
      menuBar()->setItemEnabled(TO_EDIT_CUT,!readOnly);
      menuBar()->setItemEnabled(TO_EDIT_COPY,true);
    } else {
      menuBar()->setItemEnabled(TO_EDIT_CUT,false);
      menuBar()->setItemEnabled(TO_EDIT_COPY,false);
    }
    menuBar()->setItemEnabled(TO_EDIT_PASTE,!readOnly);
    menuBar()->setItemEnabled(TO_EDIT_SELECT_ALL,true);
    menuBar()->setItemEnabled(TO_FILE_OPEN,!readOnly);
    menuBar()->setItemEnabled(TO_FILE_SAVE,true);
    menuBar()->setItemEnabled(TO_FILE_SAVE_AS,true);
    menuBar()->setItemEnabled(TO_EDIT_READ_ALL,false);
    menuBar()->setItemEnabled(TO_FILE_PRINT,true);
    menuBar()->setItemEnabled(TO_EDIT_SEARCH,true);
  } else {
    menuBar()->setItemEnabled(TO_EDIT_UNDO,false);
    menuBar()->setItemEnabled(TO_EDIT_REDO,false);
    menuBar()->setItemEnabled(TO_EDIT_CUT,false);
    menuBar()->setItemEnabled(TO_EDIT_COPY,false);
    menuBar()->setItemEnabled(TO_EDIT_PASTE,false);
    menuBar()->setItemEnabled(TO_EDIT_SELECT_ALL,false);
    menuBar()->setItemEnabled(TO_FILE_OPEN,false);
    menuBar()->setItemEnabled(TO_FILE_SAVE,false);
    menuBar()->setItemEnabled(TO_FILE_SAVE_AS,false);
    if (currWidget && (currWidget->inherits("toResultView")||
	toContent(currWidget))) {
      menuBar()->setItemEnabled(TO_EDIT_READ_ALL,true);
      menuBar()->setItemEnabled(TO_FILE_PRINT,true);
      menuBar()->setItemEnabled(TO_EDIT_SEARCH,true);
      menuBar()->setItemEnabled(TO_FILE_SAVE,true);
      menuBar()->setItemEnabled(TO_FILE_SAVE_AS,true);
    } else {
      menuBar()->setItemEnabled(TO_EDIT_READ_ALL,false);
      if (currWidget && currWidget->inherits("toListView")) {
	menuBar()->setItemEnabled(TO_FILE_PRINT,true);
        menuBar()->setItemEnabled(TO_EDIT_SEARCH,true);
	menuBar()->setItemEnabled(TO_FILE_SAVE,true);
	menuBar()->setItemEnabled(TO_FILE_SAVE_AS,true);
      } else {
	menuBar()->setItemEnabled(TO_FILE_PRINT,false);
        menuBar()->setItemEnabled(TO_EDIT_SEARCH,false);
	menuBar()->setItemEnabled(TO_FILE_SAVE,false);
	menuBar()->setItemEnabled(TO_FILE_SAVE_AS,false);
      }
    }
  }
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
  if (Tools[cmd])
    Tools[cmd]->createWindow();
  else if (cmd>=TO_TOOL_ABOUT_ID&&cmd<=TO_TOOL_ABOUT_ID_END) {
    if (Tools[cmd-TO_ABOUT_ID_OFFSET])
      Tools[cmd-TO_ABOUT_ID_OFFSET]->about(this);
  } else if (cmd>=TO_WINDOWS_WINDOWS&&cmd<=TO_WINDOWS_END) {
    if (cmd-TO_WINDOWS_WINDOWS<int(workspace()->windowList().count()))
      workspace()->windowList().at(cmd-TO_WINDOWS_WINDOWS)->setFocus();
  } else {
    toMarkedText *mark;
    try {
      mark=dynamic_cast<toMarkedText *>(qApp->focusWidget());
    } catch(...) {
      mark=NULL;
    }
    if (mark) {
      bool readOnly=mark->isReadOnly();
      bool newFilename=false;
      switch(cmd) {
      case TO_EDIT_UNDO:
	if (!readOnly)
	  mark->undo();
	break;
      case TO_EDIT_REDO:
	if (!readOnly)
	  mark->redo();
	break;
      case TO_EDIT_COPY:
	mark->copy();
	break;
      case TO_EDIT_CUT:
	if (!readOnly)
	  mark->cut();
	break;
      case TO_EDIT_PASTE:
	if (!readOnly)
	  mark->paste();
	break;
      case TO_EDIT_SELECT_ALL:
	mark->selectAll();
	break;
      case TO_FILE_OPEN:
	if (!readOnly) {
	  QFileInfo file(mark->filename());
	  QString filename=toOpenFilename(file.dirPath(),"*.sql\n*.txt",this);
	  if (!filename.isEmpty()) {
	    try {
	      QCString data=toReadFile(filename);
	      mark->setText(QString::fromLocal8Bit(data));
	      mark->setFilename(filename);
	      toStatusMessage("File opened successfully",false,false);
	    } TOCATCH
	  }
	}
	break;
      case TO_FILE_SAVE_AS:
	newFilename=true;
      case TO_FILE_SAVE:
	QFileInfo file(mark->filename());
	QString filename=mark->filename();
	if (newFilename||filename.isEmpty())
	  filename=toSaveFilename(file.dirPath(),"*.sql\n*.txt",this);
	if (!filename.isEmpty()) {
	  if (!toWriteFile(filename,mark->text()))
	    return;
	  mark->setFilename(filename);
	  mark->setEdited(false);
	}
	break;
      }
    }
    switch (cmd) {
    case TO_FILE_COMMIT:
      try {
	toConnection &conn=currentConnection();
	emit willCommit(conn,true);
	currentConnection().commit();
      } TOCATCH
      break;
    case TO_FILE_ROLLBACK:
      try {
	toConnection &conn=currentConnection();
	emit willCommit(conn,false);
	currentConnection().commit();
      } TOCATCH
      break;
    case TO_EDIT_READ_ALL:
      {
	toResultView *res;
	try {
	  res=dynamic_cast<toResultView *>(qApp->focusWidget());
	} catch(...) {
	  res=NULL;
	}
	toResultContent *cnt=toContent(qApp->focusWidget());
	if (res)
	  res->readAll();
	else if (cnt)
	  cnt->readAll();
      }
      break;
    case TO_FILE_SAVE_AS:
    case TO_FILE_SAVE:
      {
	toListView *res;
	try {
	  res=dynamic_cast<toListView *>(qApp->focusWidget());
	} catch(...) {
	  res=NULL;
	}
	toResultContent *cnt=toContent(qApp->focusWidget());
	if (res)
	  res->exportFile();
	else if (cnt)
	  cnt->exportFile();
      }
      break;
    case TO_FILE_PRINT:
      {
	toListView *res;
	try {
	  res=dynamic_cast<toListView *>(qApp->focusWidget());
	} catch(...) {
	  res=NULL;
	}
	toResultContent *cnt=toContent(qApp->focusWidget());
	if (res)
	  res->print();
	else if (cnt)
	  cnt->print();
	else if (mark)
	  mark->print();
      }
      break;
    case TO_FILE_QUIT:
      close(true);
      break;
    case TO_EDIT_SEARCH:
      if (!Search)
	Search=new toSearchReplace(this);
      if (mark)
	Search->setTarget(mark);
      else {
	toListView *res;
	try {
	  res=dynamic_cast<toListView *>(qApp->focusWidget());
	} catch(...) {
	  res=NULL;
	}
	toResultContent *cnt=toContent(qApp->focusWidget());
	if (res)
	  Search->setTarget(res);
	else if (cnt)
	  Search->setTarget(cnt);
      }
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
    case TO_EDIT_OPTIONS:
      toPreferences::displayPreferences(this);
      break;
    case TO_WINDOWS_CLOSE_ALL:
      while (workspace()->windowList().count()>0)
	delete workspace()->windowList().at(0);
      break;
    case TO_WINDOWS_CLOSE:
      QWidget *widget=workspace()->activeWindow();
      if (widget)
	delete widget;
      break;
    }
  }
}

void toMain::addConnection(void)
{
  try {
    toNewConnection newConnection(this,"New connection",true);

    toConnection *conn=NULL;

    if (newConnection.exec()) {
      conn=newConnection.makeConnection();
    }

    if (conn)
      addConnection(conn);
  } TOCATCH
}

toConnection &toMain::currentConnection()
{
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if ((*i)->description()==ConnectionSelection->currentText()) {
      return *(*i);
    }
  }
  throw QString("Can't find active connection");
}

void toMain::addConnection(toConnection *conn)
{
  int j=0;
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++,j++) {
    if ((*i)->description()==conn->description()) {
      ConnectionSelection->setCurrentItem(j);
      createDefault();
      return;
    }
  }

  Connections.insert(Connections.end(),conn);
  ConnectionSelection->insertItem(conn->description());
  ConnectionSelection->setCurrentItem(ConnectionSelection->count()-1);

  if (ConnectionSelection->count()==1) {
    menuBar()->setItemEnabled(TO_FILE_COMMIT,true);
    menuBar()->setItemEnabled(TO_FILE_ROLLBACK,true);
    menuBar()->setItemEnabled(TO_CLOSE_CONNECTION,true);
    DisconnectButton->setEnabled(true);
  }

  changeConnection();
  emit addedConnection(conn->description());
  createDefault();
}

bool toMain::delConnection(void)
{
  toConnection *conn=NULL;
  int pos=0;
  for (std::list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if ((*i)->description()==ConnectionSelection->currentText()) {
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
      Connections.erase(i);
      ConnectionSelection->removeItem(pos);
      if (ConnectionSelection->count())
	ConnectionSelection->setCurrentItem(max(pos-1,0));
      emit removedConnection(conn->description());
      delete conn;
      break;
    }
    pos++;
  }
  if (ConnectionSelection->count()==0) {
    menuBar()->setItemEnabled(TO_FILE_COMMIT,false);
    menuBar()->setItemEnabled(TO_FILE_ROLLBACK,false);
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

void toMain::editEnable(bool open,bool save,bool print,
			bool undo,bool redo,
			bool cut,bool copy,bool paste,
			bool search)
{
  toMain *main=(toMain *)qApp->mainWidget();
  if (main) {
    main->LoadButton->setEnabled(open);
    if (save) {
      main->SaveButton->setEnabled(true);
      main->RowLabel->show();
      main->ColumnLabel->show();
    } else {
      main->SaveButton->setEnabled(false);
      main->RowLabel->setText("");
      main->ColumnLabel->setText("");
    }
    main->PrintButton->setEnabled(print);

    if (undo)
      main->UndoButton->setEnabled(true);
    else
      main->UndoButton->setEnabled(false);
    if (redo)
      main->RedoButton->setEnabled(true);
    else
      main->RedoButton->setEnabled(false);

    if (cut)
      main->CutButton->setEnabled(true);
    else
      main->CutButton->setEnabled(false);
    if (copy)
      main->CopyButton->setEnabled(true);
    else
      main->CopyButton->setEnabled(false);

    if (paste)
      main->PasteButton->setEnabled(true);
    else
      main->PasteButton->setEnabled(false);
  }
}

void toMain::registerSQLEditor(int tool)
{
  SQLEditor=tool;
}

bool toMain::close(bool del)
{
  while (workspace()->windowList().count()>0)
    if (!workspace()->windowList().at(0)->close(true))
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
  for(std::list<QString>::iterator i=status.begin();i!=status.end();i++)
    StatusMenu->insertItem(*i);
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
