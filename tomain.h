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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef __TOMAIN_H
#define __TOMAIN_H

#include <list>
#include <map>

#include <qmainwindow.h>

#ifdef TO_KDE
#  include <kdockwidget.h>
#  include <ktoolbar.h>
#  include <kmenubar.h>
// In preparation of KPrinter landing
#  define TOPrinter QPrinter
#  define TODock KDockWidget
#  define TOFileDialog KFileDialog
#  include "tomainwindow.kde.h"
#else
#  include <qtoolbar.h>
#  define TOPrinter QPrinter
#  define TODock QWidget
#  define TOFileDialog QFileDialog
#  include "tomainwindow.h"
#endif
#define TOMessageBox QMessageBox

class QWorkspace;
class QComboBox;
class QToolButton;
class QPopupMenu;
class QTimer;
class QLabel;
class toSearchReplace;

#include "toconnection.h"
#include "totool.h"

class toMain : public toMainWindow {
  Q_OBJECT
private:
  list<toConnection *> Connections;
  QWorkspace *Workspace;
  QComboBox *ConnectionSelection;
  void addConnection(toConnection *conn);
  map<int,toTool *> Tools;
  list<QToolButton *> NeedConnection;
  int SQLEditor;

  QToolButton *DisconnectButton;
  QToolButton *LoadButton;
  QToolButton *SaveButton;
  QToolButton *PrintButton;
  QToolButton *UndoButton;
  QToolButton *RedoButton;
  QToolButton *CutButton;
  QToolButton *CopyButton;
  QToolButton *PasteButton;

  QPopupMenu *FileMenu;
  QPopupMenu *EditMenu;
  QPopupMenu *ToolsMenu;
  QPopupMenu *WindowsMenu;
  QPopupMenu *HelpMenu;
  QLabel *ColumnLabel;
  QLabel *RowLabel;
  toSearchReplace *Search;
public:
  toMain();

  QWorkspace *workspace()
  { return Workspace; }

  toConnection &currentConnection(void);

  static void editEnable(bool open,bool save,bool print,
			 bool undo,bool redo,
			 bool cut,bool copy,bool paste,
			 bool search);
  static void editDisable(void)
  { editEnable(false,false,false,false,false,false,false,false,false); }

  virtual bool close(bool del);
  void createDefault(void);
  void setCoordinates(int,int);

  QPopupMenu *fileMenu()
  { return FileMenu; }
  QPopupMenu *editMenu()
  { return EditMenu; }
  QPopupMenu *toolsMenu()
  { return ToolsMenu; }
  QPopupMenu *helpMenu()
  { return HelpMenu; }

  list<QString> connections(void);
  toConnection &connection(const QString &);

  void editSQL(const QString &str);
  void registerSQLEditor(int toolid);
signals:
  void sqlEditor(const QString &str);
  void addedConnection(const QString &str);
  void removedConnection(const QString &str);
public slots:
  void windowsMenu(void);
  void contextHelp(void);
private slots:
  void addConnection(void);
  bool delConnection(void);
  void commandCallback(int cmd);

  void commitButton(void);
  void rollbackButton(void);

  void loadButton(void);
  void saveButton(void);
  void printButton(void);

  void editFileMenu(void);

  void undoButton(void);
  void redoButton(void);
  void copyButton(void);
  void cutButton(void);
  void pasteButton(void);
};

void toStatusMessage(const QString &str,bool save=false);
toMain *toMainWidget(void);
QString toSQLToAddress(toConnection &conn,const QString &sql);
QString toSQLString(toConnection &conn,const QString &address);
QString toNow(toConnection &conn);
void toSetSessionType(const QString &str);
QString toGetSessionType(void);
QComboBox *toRefreshCreate(QWidget *parent,const char *name=NULL,const char *def=NULL);
void toRefreshParse(QTimer *timer,const QString &str);
QString toReadValue(const otl_column_desc &dsc,otl_stream &q,int maxSize);
bool toMonolithic(void);
QString toDeepCopy(const QString &);
QToolBar *toAllocBar(QWidget *parent,const QString &name,const QString &db);
TODock *toAllocDock(const QString &name,
		    const QString &db,
		    const QPixmap &icon);
int toSizeDecode(const QString &str);
void toAttachDock(TODock *dock,QWidget *container,QMainWindow::ToolBarDock place);
list<QString> toReadQuery(otl_stream &,list<QString> &args);
list<QString> toReadQuery(otl_stream &,
			  const QString &arg1=QString::null,const QString &arg2=QString::null,
			  const QString &arg3=QString::null,const QString &arg4=QString::null,
			  const QString &arg5=QString::null,const QString &arg6=QString::null,
			  const QString &arg7=QString::null,const QString &arg8=QString::null,
			  const QString &arg9=QString::null);
list<QString> toReadQuery(toConnection &conn,const QCString &query,list<QString> &args);
list<QString> toReadQuery(toConnection &conn,const QCString &query,
			  const QString &arg1=QString::null,const QString &arg2=QString::null,
			  const QString &arg3=QString::null,const QString &arg4=QString::null,
			  const QString &arg5=QString::null,const QString &arg6=QString::null,
			  const QString &arg7=QString::null,const QString &arg8=QString::null,
			  const QString &arg9=QString::null);
QString toShift(list<QString> &lst);
void toUnShift(list<QString> &lst,const QString &);
QString toPop(list<QString> &lst);
void toPush(list<QString> &lst,const QString &);
QFont toStringToFont(const QString &str);
QString toFontToString(const QFont &fnt);
QString toHelpPath(void);
QCString toReadFile(const QString &);
bool toCompareLists(QStringList &lst1,QStringList &lst2,unsigned int len);
void toSetEnv(const QCString &var,const QCString &val);
void toUnSetEnv(const QCString &var);

#define TO_FILE_MENU		10
#define TO_EDIT_MENU		20
#define TO_TOOLS_MENU		30
#define TO_WINDOWS_MENU		40
#define TO_HELP_MENU		40

#define TO_TOOL_MENU_INDEX	(toMainWidget()->menuBar()->indexOf(TO_WINDOWS_MENU))
#define TO_TOOL_MENU_ID		2000
#define TO_TOOL_MENU_ID_END	2999

#define TOSQL_USERLIST "Global:UserList"

#define TOCATCH \
    catch (const otl_exception &exc) {\
      toStatusMessage(QString::fromUtf8((const char *)exc.msg));\
    } catch (const QString &str) {\
      toStatusMessage(str);\
    }

#define toIsIdent(c) (c.isLetterOrNumber()||(c)=='_'||(c)=='%'||(c)=='$'||(c)=='#')

#ifdef WIN32
template <class T>
T max(T a, T b)
{
    return a > b ? a : b ;
}

template <class T>
T min(T a, T b)
{
    return a < b ? a : b ;
}

#endif

#endif
