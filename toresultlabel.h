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

#ifndef TORESULTLABEL_H
#define TORESULTLABEL_H

#include "tobackground.h"
#include "toresult.h"

#include <qlabel.h>

class toNoBlockQuery;
class toSQL;

/** This widget displays the result of a query where each item in the stream
 * is added to a label separated by an optional string.
 */

class toResultLabel : public QLabel, public toResult {
  Q_OBJECT

  toNoBlockQuery *Query;
  toBackground Poll;

  QString Separator;

public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultLabel(QWidget *parent,const char *name=NULL);
  /** Destruct object
   */
  ~toResultLabel();

  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param);

  /** Handle any connection by default
   */
  virtual bool canHandle(toConnection &)
  { return true; }

  /** Get separator string to use between result.
   */
  const QString &separator(void) const
  { return Separator; }
  /** Set separator string to use between result.
   */
  void setSeparator(const QString &sep)
  { Separator=sep; }

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
public slots:
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
private slots:
  void poll(void);
};

#endif
