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

#ifndef __TORESULTLINE_H
#define __TORESULTLINE_H

#include <time.h>

#include "toresult.h"
#include "tolinechart.h"

class toSQL;

/** Display the result of a query in a piechart. The first column of the query should
 * contain the x value and the rest of the columns should be values of the diagram. The
 * legend is the column name. Connects to the tool timer for updates automatically.
 */

class toResultLine : public toLineChart, public toResult {
  Q_OBJECT
  /** Display flow in change per second instead of actual values.
   */
  bool Flow;
  /** Timestamp of last fetch.
   */
  time_t LastStamp;
  /** Last read values.
   */
  std::list<double> LastValues;
  void query(const QString &sql,const toQList &param,bool first);
public:
  /** Create widget.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultLine(QWidget *parent,const char *name=NULL);

  /** Display actual values or flow/s.
   * @param on Display flow or absolute values.
   */
  void setFlow(bool on)
  { Flow=on; }
  /** Return if flow is displayed.
   * @return If flow is used.
   */
  bool flow(void)
  { return Flow; }

  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param)
  { query(sql,param,true); }
  /** Reimplemented for internal reasons.
   */
  virtual void clear(void)
  { LastStamp=0; LastValues.clear(); toLineChart::clear(); }
  /** Transform valueset. Make it possible to perform more complex transformation.
   * called directly before adding the valueset to the chart. After flow transformation.
   * Default is passthrough.
   * @param input The untransformed valueset.
   * @return The valueset actually added to the chart.
   */
  virtual std::list<double> transform(std::list<double> &input);
  /** Handle any connection
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
protected slots:
  /** Reimplemented for internal reasons.
   */
  virtual void connectionChanged(void);
};

#endif
