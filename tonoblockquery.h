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

#ifndef __TONOBLOCKQUERY_H
#define __TONOBLOCKQUERY_H

#include "toconnection.h"
#include "tothread.h"

class toResultStats;

/** This is class to be able to run a query in the background without
 * blocking until a response is available from OCI.
 */

class toNoBlockQuery {
private:
  /** A task to implement running the query.
   */
  class queryTask : public toTask {
    toNoBlockQuery &Parent;
  public:
    queryTask(toNoBlockQuery &parent)
      : Parent(parent)
    { }
    virtual void run(void);
  };
  friend class queryTask;

  /** This semaphore indicates wether the query is still running.
   */
  toSemaphore Running;
  /** This semaphore indicates wether the child thread should
   * continue reading values.
   */
  toSemaphore Continue;
  /** Lock for all this stuff
   */
  toLock Lock;
  /** Current location that values are being read.
   */
  toQList::iterator CurrentValue;
  /** Values read by the task. This can be changed without holding @ref Lock.
   */
  toQList ReadingValues;
  /** Values ready to be read by client.
   */
  toQList Values;
  /** Indicator if at end of query.
   */
  bool EOQ;
  /** Indicator if to quit reading from query.
   */
  bool Quit;
  /** SQL to execute.
   */
  QString SQL;
  /** Error string if error occurs.
   */
  QString Error;
  /** Number of rows processed.
   */
  int Processed;
  /** Parameters to pass to query before execution.
   */
  toQList Param;
  /** Child thread.
   */
  toThread *Thread;
  /** Statistics to be used if any.
   */
  toResultStats *Statistics;
  /** Description of result
   */
  toQDescList Description;
  /** Query used to run query
   */
  toQuery Query;
  /** Throw error if any.
   */
  void checkError();
public:
  /** Create a new query.
   * @param conn Connection to run on.
   * @param sql SQL to execute.
   * @param param Parameters to pass to query.
   * @param statistics Optional statistics widget to update with values from query.
   */
  toNoBlockQuery(toConnection &conn,
		 const QString &sql,
		 const toQList &param,
		 toResultStats *statistics=NULL);
  virtual ~toNoBlockQuery();

  /** Poll if any result is available.
   * @return True if at least one row is available.
   */
  bool poll(void);

  /** Get description of columns.
   * @return Description of columns list. Don't modify this list.
   */
  toQDescList &describe(void);

  /** Read the next value from the query.
   * @return The next available value.
   */
  QString readValue(void);

  /** Get the number of rows processed.
   * @return Number of rows processed.
   */
  int rowsProcessed(void);

  /** Check if at end of query.
   * @return True if query is done.
   */
  bool eof(void);
};

#endif
