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


#ifndef __TOMAIN_H
#define __TOMAIN_H

#include <list>
#include <map>

#include <qmainwindow.h>

class QWorkspace;
class QComboBox;
class QToolButton;
class QPopupMenu;
class QTimer;
class QLabel;

#include "toconnection.h"
#include "totool.h"

class toMain : public QMainWindow {
  Q_OBJECT
private:
  list<toConnection *> Connections;
  QWorkspace *Workspace;
  QComboBox *ConnectionSelection;
  void addConnection(toConnection *conn);
  map<int,toTool *> Tools;
  list<QToolButton *> NeedConnection;

  QToolButton *DisconnectButton;
  QToolButton *LoadButton;
  QToolButton *SaveButton;
  QToolButton *UndoButton;
  QToolButton *RedoButton;
  QToolButton *CutButton;
  QToolButton *CopyButton;
  QToolButton *PasteButton;

  QPopupMenu *WindowsMenu;
  QLabel *ColumnLabel;
  QLabel *RowLabel;
public:
  toMain();

  QWorkspace *workspace()
  { return Workspace; }

  toConnection &currentConnection(void);

  static void editEnable(bool open,bool save,
			 bool undo,bool redo,
			 bool cut,bool copy,bool paste);
  static void editDisable(void)
  { editEnable(false,false,false,false,false,false,false); }

  virtual bool close(bool del);
  void createDefault(void);
  void setCoordinates(int,int);
public slots:
  void windowsMenu(void);
private slots:
  void addConnection(void);
  bool delConnection(void);
  void commandCallback(int cmd);

  void commitButton(void);
  void rollbackButton(void);

  void loadButton(void);
  void saveButton(void);

  void editFileMenu(void);

  void undoButton(void);
  void redoButton(void);
  void copyButton(void);
  void cutButton(void);
  void pasteButton(void);
};

void toStatusMessage(const QString &str);
toMain *toMainWidget(void);
QString toSQLToAddress(toConnection &conn,const QString &sql);
QString toSQLString(toConnection &conn,const QString &address);
QString toNow(toConnection &conn);
void toSetSessionType(const QString &str);
QString toGetSessionType(void);
QComboBox *toRefreshCreate(QWidget *parent,const char *name=NULL,const char *def=NULL);
void toRefreshParse(QTimer *timer,const QString &str);
QString toReadValue(const otl_column_desc &dsc,otl_stream &q,int maxSize);

#define TO_WINDOWS_MENU		10
#define TO_TOOL_MENU_INDEX	(toMainWidget()->menuBar()->indexOf(TO_WINDOWS_MENU))
#define TO_TOOL_MENU_ID		2000
#define TO_TOOL_MENU_ID_END	2999

#define TOCATCH \
    catch (const otl_exception &exc) {\
      toStatusMessage((const char *)exc.msg);\
    } catch (const QString &str) {\
      toStatusMessage((const char *)str);\
    }

#define toIsIdent(c) (isalnum(c)||(c)=='_'||(c)=='%'||(c)=='$'||(c)=='#')

#endif
