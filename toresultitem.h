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

#ifndef __TORESULTITEM_H
#define __TORESULTITEM_H

#include <qscrollview.h>
#include <qfont.h>
#include "toresult.h"

class QGrid;

/** Display the first row of a query with each column with a separate label.
 * If the label of the item is a single '-' character the data is treated as
 * a horizontal label (Not bold or sunken), and the label is not printed.
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
  bool Right;
  /** Font to display data with.
   */
  QFont DataFont;

  /** Setup widget.
   * @param num Number of columns.
   * @param readable Make columns more readable.
   */
  void setup(int num,bool readable);
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
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultItem(int num,bool readable,QWidget *parent,const char *name=NULL);
  /** Create widget. Readable columns by default.
   * @param num Number of columns to arrange data in.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultItem(int num,QWidget *parent,const char *name=NULL);

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
  { Right=val; }
  /** Set the font to display data with.
   */
  void dataFont(const QFont &val)
  { DataFont=val; }

  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param);
  /** Reimplemented for internal reasons.
   */
  void query(const QString &sql)
  { toQList p; query(sql,p); }
public slots:
  /** Reimplemented for internal reasons.
   */
  virtual void refresh(void)
  { query(SQL); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1)
  { toQList p; p.insert(p.end(),Param1); query(SQL,p); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2)
  { toQList p; p.insert(p.end(),Param1); p.insert(p.end(),Param2); query(SQL,p); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { toQList p; p.insert(p.end(),Param1); p.insert(p.end(),Param2); p.insert(p.end(),Param3); query(SQL,p); }
};

#endif
