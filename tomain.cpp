/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/



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
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qworkspace.h>

#include <stdio.h>

#include "tomain.h"
#include "totool.h"
#include "tonewconnection.h"
#include "tomarkedtext.h"
#include "toabout.h"
#include "topreferences.h"
#include "toresultview.h"

#include "tomain.moc"

#include "icons/connect.xpm"
#include "icons/disconnect.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/commit.xpm"
#include "icons/rollback.xpm"
#include "icons/undo.xpm"
#include "icons/redo.xpm"
#include "icons/cut.xpm"
#include "icons/copy.xpm"
#include "icons/paste.xpm"

#define DEFAULT_TITLE "TOra %s"

#define TO_NEW_CONNECTION	100
#define TO_CLOSE_CONNECTION	101
#define TO_FILE_OPEN		102
#define TO_FILE_SAVE		103
#define TO_FILE_SAVE_AS		104
#define TO_FILE_COMMIT		105
#define TO_FILE_ROLLBACK	106
#define TO_FILE_QUIT		107

#define TO_EDIT_UNDO		200
#define TO_EDIT_REDO		201
#define TO_EDIT_CUT		202
#define TO_EDIT_COPY		203
#define TO_EDIT_PASTE		204
#define TO_EDIT_SELECT_ALL	205
#define TO_EDIT_OPTIONS		206
#define TO_EDIT_READ_ALL	207

#define TO_WINDOWS_TILE		300
#define TO_WINDOWS_CASCADE	301
#define TO_WINDOWS_CLOSE	302
#define TO_WINDOWS_CLOSE_ALL	303
#define TO_WINDOWS_WINDOWS	310
#define TO_WINDOWS_END		399
#define TO_HELP_ABOUT		900

#define TO_TOOLS		1000

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

toMain::toMain()
  : QMainWindow(0,"Main Window")
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

  QPopupMenu *curr;

  char buffer[100];
  sprintf(buffer,DEFAULT_TITLE,TOVERSION);
  setCaption(buffer);

  curr=new QPopupMenu(this);
  curr->insertItem(*toConnectPixmap,"&New Connection...",TO_NEW_CONNECTION);
  curr->insertItem(*toDisconnectPixmap,"&Close Connection",this,SLOT(delConnection()),0,TO_CLOSE_CONNECTION);
  curr->insertSeparator();
  curr->insertItem(*toCommitPixmap,"&Commit connection",TO_FILE_COMMIT);
  curr->insertItem(*toRollbackPixmap,"&Rollback connection",TO_FILE_ROLLBACK);
  curr->insertSeparator();
  curr->insertItem(*toLoadPixmap,"&Open File...",TO_FILE_OPEN);
  curr->insertItem(*toSavePixmap,"&Save",TO_FILE_SAVE);
  curr->insertItem("&Save As..",TO_FILE_SAVE_AS);
  curr->insertSeparator();
  curr->insertItem("&Quit",TO_FILE_QUIT);
  menuBar()->insertItem("&File",curr);
  curr->setAccel(Key_N|CTRL,TO_NEW_CONNECTION);
  curr->setAccel(Key_O|CTRL,TO_FILE_OPEN);
  curr->setAccel(Key_S|CTRL,TO_FILE_SAVE);
  connect(curr,SIGNAL(aboutToShow()),this,SLOT( editFileMenu()));

  curr=new QPopupMenu(this);
  curr->insertItem(*toUndoPixmap,"&Undo",TO_EDIT_UNDO);
  curr->insertItem(*toRedoPixmap,"&Redo",TO_EDIT_REDO);
  curr->insertSeparator();
  curr->insertItem(*toCutPixmap,"Cu&t",TO_EDIT_CUT);
  curr->insertItem(*toCopyPixmap,"&Copy",TO_EDIT_COPY);
  curr->insertItem(*toPastePixmap,"&Paste",TO_EDIT_PASTE);
  curr->insertSeparator();
  curr->insertItem("&Select All",TO_EDIT_SELECT_ALL);
  curr->insertItem("Read All &Items",TO_EDIT_READ_ALL);
  curr->insertSeparator();
  curr->insertItem("&Options",TO_EDIT_OPTIONS);
  curr->setAccel(Key_Z|CTRL,TO_EDIT_UNDO);
  curr->setAccel(Key_Z|CTRL|SHIFT,TO_EDIT_REDO);
  curr->setAccel(Key_X|CTRL,TO_EDIT_CUT);
  curr->setAccel(Key_C|CTRL,TO_EDIT_COPY);
  curr->setAccel(Key_V|CTRL,TO_EDIT_PASTE);
  connect(curr,SIGNAL(aboutToShow()),this,SLOT( editFileMenu()));
  menuBar()->insertItem("&Edit",curr);

  map<QString,toTool *> &tools=toTool::tools();

  QToolBar *toolbar=new QToolBar(this);
  addToolBar(toolbar,tr("Main toolbar"),Top,FALSE);

  LoadButton=new QToolButton(*toLoadPixmap,
			     "Load file into editor",
			     "Load file into editor",
			     this,SLOT(loadButton()),toolbar);
  SaveButton=new QToolButton(*toSavePixmap,
			     "Save file from editor",
			     "Save file from editor",
			     this,SLOT(saveButton()),toolbar);
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

  curr=new QPopupMenu(this);

#ifdef TOOL_TOOLBAR
  toolbar=new QToolBar(this);
  addToolBar(toolbar,tr("Tools toolbar"),Top,FALSE);
#else
  toolbar->addSeparator();
#endif

  int toolID=TO_TOOLS;
  int lastPriority=0;
  for (map<QString,toTool *>::iterator i=tools.begin();i!=tools.end();i++) {
    const QPixmap *pixmap=(*i).second->toolbarImage();
    const char *toolTip=(*i).second->toolbarTip();
    const char *menuName=(*i).second->menuItem();

    int priority=(*i).second->priority();
    if (priority/100!=lastPriority/100) {
      curr->insertSeparator();
      toolbar->addSeparator();
    }
    lastPriority=priority;

    if (pixmap) {
      if (!toolTip)
	toolTip="";
      NeedConnection.insert(NeedConnection.end(),
			    new QToolButton(*pixmap,
					    toolTip,
					    toolTip,
					    (*i).second,
					    SLOT(createWindow(void)),
					    toolbar));
    }

    if (menuName) {
      if (pixmap)
	curr->insertItem(*pixmap,menuName,toolID);
      else
	curr->insertItem(menuName,toolID);
      curr->setItemEnabled(toolID,false);
    }

    Tools[toolID]=(*i).second;

    toolID++;
  }

#ifndef TOOL_TOOLBAR
  QLabel *label=new QLabel("",toolbar);
  toolbar->setStretchableWidget(label);
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
  NeedConnection.insert(NeedConnection.end(),
			new QToolButton(*toCommitPixmap,
					"Commit connection",
					"Commit connection",
					this,SLOT(commitButton()),toolbar));
  NeedConnection.insert(NeedConnection.end(),
			new QToolButton(*toRollbackPixmap,
					"Rollback connection",
					"Rollback connection",
					this,SLOT(rollbackButton()),toolbar));
  toolbar->addSeparator();
  ConnectionSelection=new QComboBox(toolbar);
  ConnectionSelection->setFixedWidth(200);

  menuBar()->insertItem("&Tools",curr);

  WindowsMenu=new QPopupMenu(this);
  WindowsMenu->setCheckable( TRUE );
  connect(WindowsMenu,SIGNAL(aboutToShow()),this,SLOT( windowsMenu()));
  menuBar()->insertItem("&Windows",WindowsMenu);

  curr=new QPopupMenu(this);
  curr->insertItem("&About TOra",TO_HELP_ABOUT);
  menuBar()->insertItem("&Help",curr);

  Workspace=new QWorkspace(this);
  setCentralWidget(Workspace);
  statusBar()->message("Ready");
  menuBar()->setItemEnabled(TO_CLOSE_CONNECTION,false);
  menuBar()->setItemEnabled(TO_FILE_COMMIT,false);
  menuBar()->setItemEnabled(TO_FILE_ROLLBACK,false);
  DisconnectButton->setEnabled(false);

  for (list<QToolButton *>::iterator i=NeedConnection.begin();i!=NeedConnection.end();i++)
    (*i)->setEnabled(false);

  connect(menuBar(),SIGNAL(activated(int)),this,SLOT(commandCallback(int)));
  show();
  {
    toNewConnection newConnection(this,"First connection",true);
    
    toConnection *conn;
    
    do {
      conn=NULL;
      if (newConnection.exec()) {
	conn=newConnection.makeConnection();
      } else {
	return;
      }
    } while(!conn);
    
    addConnection(conn);
  }
  toTool::saveConfig();
}

void toMain::editFileMenu(void)	// Ugly hack to disable edit with closed child windows
{
  QWidget *currWidget=qApp->focusWidget();
  if (dynamic_cast<toMarkedText *>(currWidget)) {
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
    if (dynamic_cast<toResultView *>(currWidget))
      menuBar()->setItemEnabled(TO_EDIT_READ_ALL,true);
    else
      menuBar()->setItemEnabled(TO_EDIT_READ_ALL,false);
  }
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
    for (unsigned int i=0;i<workspace()->windowList().count();i++) {
      WindowsMenu->insertItem(workspace()->windowList().at(i)->caption(),TO_WINDOWS_WINDOWS+i);
      WindowsMenu->setItemChecked(TO_WINDOWS_WINDOWS+i,workspace()->activeWindow()==workspace()->windowList().at(i));
      if (i<9)
	WindowsMenu->setAccel(Key_1+i|CTRL,TO_WINDOWS_WINDOWS+i);
    }
  }
}

void toMain::commandCallback(int cmd)
{
  if (Tools[cmd])
    Tools[cmd]->createWindow();
  else if (cmd>=TO_WINDOWS_WINDOWS&&cmd<=TO_WINDOWS_END)
    workspace()->windowList().at(cmd-TO_WINDOWS_WINDOWS)->setFocus();
  else {
    toMarkedText *mark=dynamic_cast<toMarkedText *>(qApp->focusWidget());
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
	  QString filename=QFileDialog::getOpenFileName(mark->filename(),"*.sql\n*.txt",this);
	  if (!filename.isEmpty()) {
	    QFile file(filename);
	    if (!file.open(IO_ReadOnly)) {
	      QMessageBox::warning(this,"File error","Couldn't read file");
	      return;
	    }
	    
	    int size=file.size();
	    
	    char buf[size+1];
	    if (file.readBlock(buf,size)==-1) {
	      throw QString("Encountered problems read configuration");
	    }
	    buf[size]=0;
	    mark->setText(buf);
	    mark->setFilename(filename);
	    toStatusMessage("File opened successfully");
	  }
	}
	break;
      case TO_FILE_SAVE_AS:
	newFilename=true;
      case TO_FILE_SAVE:
	QString filename=mark->filename();
	if (newFilename||filename.isEmpty())
	  filename=QFileDialog::getSaveFileName(filename,"*.sql\n*.txt",this);
	if (!filename.isEmpty()) {
	  QFile file(filename);
	  if (!file.open(IO_WriteOnly)) {
	    QMessageBox::warning(this,"File error","Couldn't open file for writing");
	    return;
	  }
	  QString data=mark->text();
	  file.writeBlock(data,data.length());
	  mark->setFilename(filename);
	  mark->setEdited(false);
	  toStatusMessage("File saved successfully");
	}
	break;
      }
    }
    switch (cmd) {
    case TO_FILE_COMMIT:
      try {
	currentConnection().commit();
      } catch (const otl_exception &exc) {
	toStatusMessage((const char *)exc.msg);
      } catch (const QString &str) {
	toStatusMessage((const char *)str);
      }
      break;
    case TO_FILE_ROLLBACK:
      try {
	currentConnection().rollback();
      } catch (const otl_exception &exc) {
	toStatusMessage((const char *)exc.msg);
      } catch (const QString &str) {
	toStatusMessage((const char *)str);
      }
      break;
    case TO_EDIT_READ_ALL:
      {
	toResultView *res=dynamic_cast<toResultView *>(qApp->focusWidget());
	if (res)
	  res->readAll();
      }
      break;
    case TO_FILE_QUIT:
      close(true);
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
    case TO_HELP_ABOUT:
      {
	toAbout *about=new toAbout(this,"About TOra",true);
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
    toNewConnection newConnection(this,"New connection",true);

    toConnection *conn=NULL;

    if (newConnection.exec()) {
      conn=newConnection.makeConnection();
    }

    if (conn)
      addConnection(conn);
}

toConnection &toMain::currentConnection()
{
  for (list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if ((*i)->connectString()==ConnectionSelection->currentText()) {
      return *(*i);
    }
  }
  throw QString("Can't find active connection");
}

void toMain::addConnection(toConnection *conn)
{
  for (list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if ((*i)->connectString()==conn->connectString()) {
      return;
    }
  }

  Connections.insert(Connections.end(),conn);
  ConnectionSelection->insertItem(conn->connectString());
  ConnectionSelection->setCurrentItem(ConnectionSelection->count()-1);

  if (ConnectionSelection->count()==1) {
    menuBar()->setItemEnabled(TO_FILE_COMMIT,true);
    menuBar()->setItemEnabled(TO_FILE_ROLLBACK,true);
    menuBar()->setItemEnabled(TO_CLOSE_CONNECTION,true);
    DisconnectButton->setEnabled(true);
    for (list<QToolButton *>::iterator i=NeedConnection.begin();i!=NeedConnection.end();i++)
      (*i)->setEnabled(true);
    for (map<int,toTool *>::iterator i=Tools.begin();i!=Tools.end();i++)
      menuBar()->setItemEnabled((*i).first,true);
  }
}

bool toMain::delConnection(void)
{
  toConnection *conn=NULL;
  int pos=0;
  for (list<toConnection *>::iterator i=Connections.begin();i!=Connections.end();i++) {
    if ((*i)->connectString()==ConnectionSelection->currentText()) {
      conn=(*i);
      if (conn->needCommit()) {
	QString str("Commit work in session to ");
	str.append(conn->connectString());
	str.append(" before closing it?");
	switch(QMessageBox::warning(this,"Commit work?",str,"&Yes","&No","&Cancel")) {
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
    for (list<QToolButton *>::iterator i=NeedConnection.begin();i!=NeedConnection.end();i++)
      (*i)->setEnabled(false);
    for (map<int,toTool *>::iterator i=Tools.begin();i!=Tools.end();i++)
      menuBar()->setItemEnabled((*i).first,false);
  }
  return true;
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

void toMain::editEnable(bool open,bool save,
			bool undo,bool redo,
			bool cut,bool copy,bool paste)
{
  toMain *main=(toMain *)qApp->mainWidget();
  if (main) {
    if (open)
      main->LoadButton->setEnabled(TRUE);
    else
      main->LoadButton->setEnabled(FALSE);
    if (save)
      main->SaveButton->setEnabled(TRUE);
    else
      main->SaveButton->setEnabled(FALSE);

    if (undo)
      main->UndoButton->setEnabled(TRUE);
    else
      main->UndoButton->setEnabled(FALSE);
    if (redo)
      main->RedoButton->setEnabled(TRUE);
    else
      main->RedoButton->setEnabled(FALSE);

    if (cut)
      main->CutButton->setEnabled(TRUE);
    else
      main->CutButton->setEnabled(FALSE);
    if (copy)
      main->CopyButton->setEnabled(TRUE);
    else
      main->CopyButton->setEnabled(FALSE);

    if (paste)
      main->PasteButton->setEnabled(TRUE);
    else
      main->PasteButton->setEnabled(FALSE);
  }
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
  commandCallback(TO_TOOLS);
  toStatusMessage("Welcome to Tora");
}
