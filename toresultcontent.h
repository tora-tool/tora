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

#ifndef TORESULTCONTENT_H
#define TORESULTCONTENT_H

#include "tobackground.h"
#include "toconnection.h"
#include "toeditwidget.h"
#include "tomemoeditor.h"
#include "toresult.h"

#include <list>

#include <qtable.h>
#include <qvbox.h>

class QChecBox;
class QCheckBox;
class QGrid;
class QLineEdit;
class toNoBlockQuery;
class toResultContent;
class toSearchReplace;

/** Implement memo editor in result content editor. Only for internal use.
 * @internal
 */

class toResultContentMemo : public toMemoEditor {
  Q_OBJECT

  toResultContentEditor *contentEditor();
public:
  toResultContentMemo(QWidget *parent,const QString &data,int row,int col,
		      bool sql=false);
public slots:
  virtual void firstColumn();
  virtual void nextColumn();
  virtual void previousColumn();
  virtual void lastColumn();

  virtual void changePosition(int row,int cols);
};

/** This widget is used for single record view in the content editor. Only for internal use.
 * @internal
 */

class toResultContentSingle : public QScrollView {
  Q_OBJECT

  int Row;
  QGrid *Container;
  std::list<QCheckBox *> Null;
  std::list<QLineEdit *> Value;
public:
  toResultContentSingle(QWidget *parent);

  void changeSource(QTable *table);
  void changeRow(QTable *table,int row);
  void saveRow(QTable *table,int row);
private slots:
  virtual void showMemo(int row);
};

/** This widget allows the user to browse the contents of a table and also edit
 * the content. The table is specified by the first and second parameter in the query.
 * The sql is not used in the query. Only for internal use.
 */

class toResultContentEditor : public QTable,public toEditWidget {
  Q_OBJECT

  /** Single record form.
   */
  toResultContentSingle *SingleEdit;
  /** Owner of table.
   */
  QString Owner;
  /** Tablename.
   */
  QString Table;
  /** The SQL used to read the data.
   */
  QString SQL;
  /** Original values of rows currently being edited.
   */
  std::list<QString> OrigValues;
  /** Stream to read data from.
   */
  toNoBlockQuery *Query;

  toQDescList Description;
  toBackground Poll;
  int MaxNumber;
  int SkipNumber;
  bool GotoEnd;
  /** Number of rows read from stream.
   */
  int Row;
  /** Current row of editing.
   */
  int CurrentRow;
  /** Current row of editing.
   */
  int NewRecordRow;
  /** Used to detect drag.
   */
  QPoint LastMove;
  
  /** Popup menu if available.
   */
  QPopupMenu *Menu;
  /** Column of item selected when popup menu displayed.
   */
  int MenuColumn;
  /** Row of item selected when popup menu displayed.
   */
  int MenuRow;
  
  /** Current sorting row.
   */
  int SortRow;
  /** Indicate if sorting ascending or descending.
   */
  bool SortRowAsc;
  
  /** Use filter for all tables.
   */
  bool AllFilter;
  /** Filter selection criteria
   */
  std::map<QCString,QString> Criteria;
  /** Filter retrieve order
   */
  std::map<QCString,QString> Order;
  /** Current filter name in map
   */
  QString FilterName;
  /** Never use returning.
   */
  bool NoUseReturning;

  /** Throw an exception about wrong usage.
   */
  void wrongUsage(void);
  
  void saveRow(int);
  /** Reimplemented for internal reasons.
   */
  virtual void drawContents(QPainter * p,int cx,int cy,int cw,int ch);
  /** Reimplemented for internal reasons.
   */
  virtual QWidget *beginEdit(int row,int col,bool replace);
  virtual void endEdit(int row,int col,bool accept,bool replace);

  /** Reimplemented for internal reasons.
   */
  virtual void paintCell(QPainter *p,int row,int col,const QRect &cr,bool selected);
  /** Reimplemented for internal reasons.
  */
  virtual bool eventFilter(QObject *o,QEvent *e);
  /** Reimplemented for internal reasons.
  */
  virtual void keyPressEvent(QKeyEvent *e);
  /** Reimplemented for internal reasons.
   */
  virtual void activateNextCell();
  
  /** Reimplemented for internal reasons.
   */
  virtual void dragEnterEvent(QDragEnterEvent *event);
  /** Reimplemented for internal reasons.
   */
  virtual void dropEvent(QDropEvent *event);
  /** Reimplemented for internal reasons.
   */
  virtual void contentsMousePressEvent(QMouseEvent *e);
  /** Reimplemented for internal reasons.
   */
  virtual void contentsMouseReleaseEvent(QMouseEvent *e);
  /** Reimplemented for internal reasons.
   */
  virtual void contentsMouseMoveEvent (QMouseEvent *e);
  /** Reimplemented for internal reasons.
   */
  virtual void focusInEvent (QFocusEvent *e);
  /** A setCurrentCell() replacement; makes sure that we have focus.
   */
  virtual void setCurrentCellFocus(int row, int col);

  int MaxColDisp;

  QString table(void);

  toConnection &connection();
  QLineEdit *CurrentEditor;
  int SearchStart;
  int SearchEnd;

  class contentItem : public QTableItem {
  public:
    contentItem(QTable *table,const QString &text);
    virtual QString key(void) const;
  };

  toListView *copySelection(bool);
public:
  /** Indicate that editor should never use returning clauses even if this is oracle.
   */
  void useNoReturning(bool use)
  { NoUseReturning=use; }
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultContentEditor(QWidget *parent,const char *name=NULL);
  /** Destruct object
   */
  ~toResultContentEditor();
  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param)
  { wrongUsage(); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1)
  { wrongUsage(); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { wrongUsage(); }
  
  /** Print this editor.
   */
  virtual void editPrint(void);
  /** Reimplemented for internal reasons.
   */
  virtual bool editSave(bool ask);
  /** Reimplemented for internal reasons.
   */
  virtual void editReadAll(void);
  /** Select all contents. Default NOP.
   */
  virtual void editSelectAll(void);

  /** Reimplemented for internal reasons.
   */
  virtual void setText(int row,int col,const QString &text);

  /** Set a new filter setting.
   * @param all Apply filter to all tables, otherwise only for this table.
   * @param criteria Criteria to filter on.
   * @param order Order to read data on.
   */
  void changeFilter(bool all,const QString &criteria,const QString &order);

  /** Get information about if filter affect all tables.
   */
  bool allFilter()
  { return AllFilter; }

  friend class contentItem;
  friend class toResultContent;

  /** Export data to a map.
   * @param data A map that can be used to recreate the data of a chart.
   * @param prefix Prefix to add to the map.
   */
  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix);
  /** Import data
   * @param data Data to read from a map.
   * @param prefix Prefix to read data from.
   */
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix);
public slots:
  /** Erase last parameters
   */
  virtual void clearParams(void)
  { Owner=Table=QString::null; }
  /** Change sorting column
   * @param col Column selected to change as sorting.
   */
  virtual void changeSort(int col); 
  /** Reimplemented for internal reasons.
   */
  virtual void refresh(void)
  { QString t=Owner; Owner=QString::null; changeParams(t,Table); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2);
  /** Current cell changed.
   * @param row New row.
   * @param col New column.
   */
  void changePosition(int row,int col);
  
  /** Display popup menu
   * @param p Point to display popup at.
   */
  virtual void displayMenu(const QPoint &p);
  /** Display editable memo viewer at current position.
   */
  virtual void displayMemo(void);
  /** Save unsaved changes in the editor
   */
  virtual void saveUnsaved(void);
  /** Delete the current row from the table.
   */
  virtual void deleteCurrent(void);
  /** Add a new record to the table.
   */
  virtual void addRecord(void);
  /** Discard the changes made to the table.
   */
  virtual void cancelEdit(void);
  /** Goto the last record in the table.
   */
  virtual void gotoLastRecord(void);
  /** Goto the first record in the table.
   */
  virtual void gotoFirstRecord(void);
  /** Goto the previous record in the table.
   */
  virtual void gotoPreviousRecord(void);
  /** Goto the next record in the table.
   */
  virtual void gotoNextRecord(void);
  /** Display single record form.
   */
  virtual void singleRecordForm(bool display);

  /** Move to top of data
   */
  virtual void searchTop(void)
  { setCurrentCell(0,0); }
  /** Search for next entry
   * @return True if found, should select the found text.
   */
  virtual bool searchNext(toSearchReplace *search);
  /** Replace entry with new data
   */
  virtual void searchReplace(const QString &newData);
  /** Check if data can be modified by search
   * @param all If true can replace all, otherwise can replace right now.
   */
  virtual bool searchCanReplace(bool all);
protected slots:
  /** Callback from popup menu.
   * @param cmd Command ID.
   */
  virtual void menuCallback(int cmd);
  /** Change data at specified position.
   * @param row Row to change.
   * @param col Column to change.
   * @param data New contents of data.
   */ 
  virtual void changeData(int row,int col,const QString &data); 
private slots:
  virtual void poll(void);
};

/** This widget allows the user to browse the contents of a table and also edit
 * the content. The table is specified by the first and second parameter in the query.
 * The sql is not used in the query.
 */

class toResultContent : public QVBox, public toResult {
  Q_OBJECT

  toResultContentEditor *Editor;
public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultContent(QWidget *parent,const char *name=NULL);

  /** Get content editor table widget
   * @return Pointer to editor.
   */
  toResultContentEditor *editor(void)
  { return Editor; }

  /** Read all rows from the table.
   */
  virtual void editReadAll(void)
  { Editor->editReadAll(); }
  /** Print the contents.
   */
  virtual void editPrint(void)
  { Editor->editPrint(); }
  /** Export contents to file.
   */
  virtual void editSave(bool ask)
  { Editor->editSave(ask); }

  /** Export data to a map.
   * @param data A map that can be used to recreate the data of a chart.
   * @param prefix Prefix to add to the map.
   */
  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix)
  { Editor->exportData(data,prefix); }
  /** Import data
   * @param data Data to read from a map.
   * @param prefix Prefix to read data from.
   */
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix)
  { Editor->importData(data,prefix); }
  /** Indicate that editor should never use returning clauses even if this is oracle.
   */
  void useNoReturning(bool use)
  { Editor->useNoReturning(use); }
private slots:
  void changeFilter(void);
  void removeFilter(void);
public slots:
  /** Erase last parameters
   */
  virtual void clearParams(void)
  { Editor->clearParams(); }
  /** Reimplemented for internal reasons.
   */
  virtual void refresh(void)
  { Editor->refresh(); }
  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param)
  { Editor->query(sql,param); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1)
  { Editor->changeParams(Param1); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2)
  { Editor->changeParams(Param1,Param2); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { Editor->changeParams(Param1,Param2,Param3); }
  /** Save unsaved changes in the editor
   */
  virtual void saveUnsaved(void)
  { Editor->saveUnsaved(); }
  /** Commit connection
   * @param conn Connection commit is made on.
   * @param cmt If commit or rollback
   */
  virtual void saveUnsaved(toConnection &conn,bool cmt);

  /** Handle Oracle & MySQL
   */
  virtual bool canHandle(toConnection &);
};

#endif
