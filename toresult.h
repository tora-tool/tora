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

#ifndef __TORESULT_H
#define __TORESULT_H

#include <list>

#include <qobject.h>
#include "toqvalue.h"
#include "tosql.h"

class toTimer;
class toConnection;
class toResult;
class QTabWidget;

/** Used for getting slots into toResult.
 * @internal
 */

class toResultObject : public QObject {
  Q_OBJECT

  toResult *Result;
public:
  toResultObject(toResult *result)
    : Result(result)
  { }
public slots:
  /** Called when connection is changed. Be sure to call the parent if you reimplement this.
   */
  virtual void connectionChanged(void);
  /** Called when timer times out
   */
  virtual void setup(void); 
};

/** Abstract baseclass of widgets that can perform queries. Usefull because you can execute
 * the query without knowing how it's UI is presented.
 */

class toResult {
  toResultObject Slots;
  bool Handled;
  QTimer *Setup;
  QTabWidget *Tabs;

  QString SQL;
  toQList Params;
  bool FromSQL;
  QString Name;
protected:
  /** Get the current connection from the closest tool.
   * @return Reference to connection.
   */
  toConnection &connection(void);
  /** Get the timer associated with the closest tool.
   * @return Pointer to tool timer.
   */
  toTimer *timer(void);
  /** Set parameters of last query.
   */
  void setParams(const toQList &par)
  { Params=par; }
public:
  toResult(void);
  virtual ~toResult()
  { }

  /** Re execute last query
   */
  virtual void refresh(void)
  { query(SQL,Params); }
  /** Perform a query.
   * @param sql Execute an SQL statement.
   * @param params Parameters needed as input to execute statement.
   */
  virtual void query(const QString &sql,const toQList &params) = 0;
  /** Reexecute with changed parameters.
   * @param Param1 First parameter.
   */
  virtual void changeParams(const QString &Param1)
  { Params.clear(); Params.insert(Params.end(),toQValue(Param1)); query(SQL,Params); }
  /** Reexecute with changed parameters.
   * @param Param1 First parameter.
   * @param Param1 Second parameter.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2);
  /** Reexecute with changed parameters.
   * @param Param1 First parameter.
   * @param Param2 Second parameter.
   * @param Param3 Third parameter.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3);
  /** Reimplemented for internal reasons.
   */
  virtual bool canHandle(toConnection &)
  { return false; }
  /** Get last parameters used.
   * @return Don't modify the list returned.
   */
  toQList &params(void)
  { return Params; }
  /** Check if this result is handled by the current connection
   */
  bool handled(void)
  { return Handled; }
  /** Indicate that this result can not be handled even though indicated otherwise
   */
  void setHandle(bool);
  /** Indicate what tabbed widget to disable page if not immediate parent if this
   * result is not handled.
   */
  void setTabWidget(QTabWidget *tab)
  { Tabs=tab; }

  /** Set the SQL statement of this list
   * @param sql String containing statement.
   */
  void setSQL(const QString &sql)
  { SQL=sql; }
  /** Set the SQL statement of this list. This will also affect @ref Name.
   * @param sql SQL containing statement.
   */
  void setSQL(toSQL &sql);
  /** Set new SQL and run query.
   * @param sql New sql.
   * @see setSQL
   */
  void query(const QString &sql);
  /** Set new SQL and run query.
   * @param sql New sql.
   * @see setSQL
   */
  void query(toSQL &sql);
  /** Get SQL to execute
   */
  QString sql(void)
  { return SQL; }

  /** Get SQL name of list.
   */
  virtual QString sqlName(void)
  { return Name; }
  /** Set SQL name of list.
   */
  virtual void setSQLName(const QString &name)
  { Name=name; }

  friend class toResultObject;
protected:
  /** Called when connection is changed. Be sure to call the parent if you reimplement this.
   */
  virtual void connectionChanged(void);
private:
  void changeHandle(void); 
};

#endif
