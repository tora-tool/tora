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
  friend queryTask;

  /** This semaphore indicates wether the query is still running.
   */
  toSemaphore Running;
  /** This semaphore indicates wether the child thread should
   * continue reading values.
   */
  toSemaphore Continue;
  /** Max column size when reading values.
   */
  int MaxColSize;
  /** Lock for all this stuff
   */
  toLock Lock;
  /** Current location that values are being read.
   */
  list<QString>::iterator CurrentValue;
  /** Values read by the task. This can be changed without holding @ref Lock.
   */
  list<QString> ReadingValues;
  /** Values ready to be read by client.
   */
  list<QString> Values;
  /** Indicator if at end of query.
   */
  bool EOQ;
  /** Indicator if to quit reading from query.
   */
  bool Quit;
  /** Long connection to execute query on.
   */
  otl_connect *LongConn;
  /** Connection to get long connection from.
   */
  toConnection &Connection;
  /** SQL to execute.
   */
  QString SQL;
  /** Error string if error occurs.
   */
  QString Error;
  /** Length of description of result columns.
   */
  int DescriptionLength;
  /** Number of rows processed.
   */
  int Processed;
  /** Description of result columns.
   */
  otl_column_desc *Description;
  /** Parameters to pass to query before execution.
   */
  list<QString> Param;
  /** Child thread.
   */
  toThread *Thread;
  /** Statistics to be used if any.
   */
  toResultStats *Statistics;

  /** Throw error if any.
   */
  void checkError()
  { if (!Error.isNull()) throw Error; }
public:
  /** Create a new query.
   * @param conn Connection to run on.
   * @param sql SQL to execute.
   * @param param Parameters to pass to query.
   * @param statistics Optional statistics widget to update with values from query.
   */
  toNoBlockQuery(toConnection &conn,
		 const QString &sql,
		 const list<QString> &param,
		 toResultStats *statistics=NULL);
  virtual ~toNoBlockQuery();

  /** Poll if any result is available.
   * @return True if at least one row is available.
   */
  bool poll(void);

  /** Get description of columns.
   * @param length Number of columns.
   * @return Description of columns array.
   */
  otl_column_desc *describe(int &length);

  /** Read the next value from the query.
   * @return The next available value.
   */
  QString readValue(void);

  /** Get the number of rows processed.
   * @return Number of rows processed.
   */
  int getProcessed(void);

  /** Check if at end of query.
   * @return True if query is done.
   */
  bool eof(void);
};

#endif
