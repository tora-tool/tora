/****************************************************************************
 *
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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/


#ifndef __TONOBLOCKQUERY_H
#define __TONOBLOCKQUERY_H

#include "toconnection.h"
#include "tothread.h"

class toNoBlockQuery {
private:
  class queryTask : public toTask {
    toNoBlockQuery &Parent;
  public:
    queryTask(toNoBlockQuery &parent)
      : Parent(parent)
    { }
    virtual void run(void);
  };
  friend queryTask;

  toSemaphore Running;
  toSemaphore Continue;
  toLock Lock;
  list<QString>::iterator CurrentValue;
  list<QString> ReadingValues;
  list<QString> Values;
  bool EOQ;
  otl_connect *LongConn;
  toConnection &Connection;
  QString SQL;
  QString Error;
  int DescriptionLength;
  int Processed;
  otl_column_desc *Description;
  list<QString> Param;
  toThread *Thread;

  void checkError()
  { if (!Error.isNull()) throw Error; }
public:
  toNoBlockQuery(toConnection &conn,
		 const QString &sql,
		 const list<QString> &param);
  virtual ~toNoBlockQuery();

  bool poll(void)
  { return Running.getValue()||CurrentValue!=Values.end(); }

  otl_column_desc *describe(int &length);

  QString readValue(void);

  int getProcessed(void);

  bool eof(void);
};

#endif
