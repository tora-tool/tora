//***************************************************************************
/*
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

#ifndef __TORESULTITEM_H
#define __TORESULTITEM_H

#include <qscrollview.h>
#include <qfont.h>
#include "toresult.h"

class toConnection;
class QGrid;

/** Display the first row of a query with each column with a separate label.
 */

class toResultItem : public QScrollView, public toResult {
  Q_OBJECT
  /** Query to run.
   */
  QString SQL;

  /** Result widget.
   */
  QGrid *Result;

  /** Number of created widgets.
   */
  int NumWidgets;
  /** Last widget used.
   */
  int WidgetPos;
  /** List of allocated widgets.
   */
  QWidget **Widgets;

  /** If title names are to be made more readable.
   */
  bool ReadableColumns;
  /** If title names are to be displayed.
   */
  bool ShowTitle;
  /** Align widgets to the right.
   */
  bool AlignRight;
  /** Font to display data with.
   */
  QFont DataFont;

  /** Setup widget.
   * @param num Number of columns.
   * @param readable Make columns more readable.
   */
  void setup(int num,bool readable);
  /** Connection of item.
   */
  toConnection &Connection;
protected:
  /** Start new query, hide all widgets.
   */
  void start(void);
  /** Add a new widget.
   * @param title Title of this value.
   * @param value Value.
   */
  void addItem(const QString &title,const QString &value);
  /** Done with adding queries.
   */
  void done(void);
public:
  /** Create widget.
   * @param num Number of columns to arrange data in.
   * @param readable Indicate if columns are to be made more readable. This means that the
   * descriptions are capitalised and '_' are converted to ' '.
   * @param conn Connection to issue query on.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultItem(int num,bool readable,toConnection &conn,QWidget *parent,const char *name=NULL);
  /** Create widget. Readable columns by default.
   * @param num Number of columns to arrange data in.
   * @param conn Connection to issue query on.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultItem(int num,toConnection &conn,QWidget *parent,const char *name=NULL);

  /** Set SQL to query.
   * @param sql Query to run.
   */
  void setSQL(const QString &sql)
  { SQL=sql; }
  /** Set if titles are to be shown.
   * @param val If titles are to be shown.
   */
  void showTitle(bool val)
  { ShowTitle=val; }
  /** Set if labels are to be aligned right.
   * @param val If labels are to be aligned right.
   */
  void alignRight(bool val)
  { AlignRight=val; }
  /** Set the font to display data with.
   */
  void dataFont(const QFont &val)
  { DataFont=val; }

  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const list<QString> &param);
  /** Reimplemented for internal reasons.
   */
  void query(const QString &sql)
  { list<QString> p; query(sql,p); }
public slots:
  /** Reimplemented for internal reasons.
   */
  virtual void refresh(void)
  { query(SQL); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1)
  { list<QString> p; p.insert(p.end(),Param1); query(SQL,p); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2)
  { list<QString> p; p.insert(p.end(),Param1); p.insert(p.end(),Param2); query(SQL,p); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { list<QString> p; p.insert(p.end(),Param1); p.insert(p.end(),Param2); p.insert(p.end(),Param3); query(SQL,p); }
};

#endif
