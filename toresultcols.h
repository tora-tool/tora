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

#ifndef TORESULTCOLS_H
#define TORESULTCOLS_H

#include "toconnection.h"
#include "toresultview.h"

#include <qvbox.h>

class QLabel;
class toResultColsItem;

/** This widget displays information about the returned columns of an object
 * specified by the first and second parameter in the query. The sql is not
 * used in the query.
 */

class toResultCols : public QVBox, public toResult {
  class resultCols : public toListView {
    QString Owner;
    QString Name;
  public:
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    resultCols(QWidget *parent,const char *name=NULL);
    virtual void describe(toQDescList &desc);
    virtual void query(const toConnection::objectName &,bool nocache);
    virtual void query(const QString &table,const QString &owner,const QString &name);
    friend class toResultColsItem;
  };
  QLabel *Title;
  resultCols *Columns;
  bool NoCache;

  friend class resultCols;
public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultCols(QWidget *parent,const char *name=NULL);
  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param);
  /** Handle any connection by default
   */
  virtual bool canHandle(toConnection &)
  { return true; }

  // Why are these needed?
#if 1
  /** Set the SQL statement of this list
   * @param sql String containing statement.
   */
  void setSQL(const QString &sql)
  { toResult::setSQL(sql); }
  /** Set the SQL statement of this list. This will also affect @ref Name.
   * @param sql SQL containing statement.
   */
  void setSQL(toSQL &sql)
  { toResult::setSQL(sql); }
  /** Set new SQL and run query.
   * @param sql New sql.
   * @see setSQL
   */
  void query(const QString &sql)
  { toResult::query(sql); }
  /** Set new SQL and run query.
   * @param sql New sql.
   * @see setSQL
   */
  void query(toSQL &sql)
  { toResult::query(sql); }
#endif

  friend class toResultColsItem;
public slots:
  /** Erase last parameters
   */
  virtual void clearParams(void)
  { toResult::clearParams(); NoCache=true; }
  /** Reimplemented for internal reasons.
   */
  virtual void refresh(void)
  { toResult::refresh(); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1)
  { toResult::changeParams(Param1); }
  /** Reimplemented For internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2)
  { toResult::changeParams(Param1,Param2); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { toResult::changeParams(Param1,Param2,Param3); }
};

#endif
