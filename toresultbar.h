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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
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

#ifndef __TORESULTBAR_H
#define __TORESULTBAR_H

#include "toresult.h"
#include "tobarchart.h"

class toSQL;

/** Display the result of a query in a piechart. The first column of the query should
 * contain the x value and the rest of the columns should be values of the diagram. The
 * legend is the column name. Connects to the tool timer for updates automatically.
 */

class toResultBar : public toBarChart, public toResult {
  Q_OBJECT
  /** Query to run.
   */
  QString SQL;
  void query(const QString &sql,const list<QString> &param,bool first);
public:
  /** Create widget.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultBar(QWidget *parent,const char *name=NULL);

  /** Set SQL to query.
   * @param sql Query to run.
   */
  void setSQL(const QString &sql)
  { SQL=sql; }
  /** Set the SQL statement of this list.
   * @param sql SQL containing statement.
   */
  void setSQL(toSQL &sql);

  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const list<QString> &param)
  { query(sql,param,true); }
  /** Reimplemented for internal reasons.
   */
  void query(const QString &sql)
  { list<QString> p; query(sql,p); }
public slots:
  /** Read another value to the chart.
   */
  virtual void refresh(void)
  { list<QString> p; query(SQL,p,false); }
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
