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

#ifndef __TORESULT_H
#define __TORESULT_H

#include <list>

class QString;
class toConnection;
class toTimer;
class otl_connect;

/** Abstract baseclass of widgets that can perform queries. Usefull because you can execute
 * the query without knowing how it's UI is presented.
 */

class toResult {
protected:
  /** Get the current connection from the closest tool.
   * @return Reference to connection.
   */
  toConnection &connection(void);
  /** Get the otl_connect structure of the current connection from the closest tool.
   * @return Reference to otl_connect object.
   */
  otl_connect &otlConnection(void);
  /** Get the timer associated with the closest tool.
   * @return Pointer to tool timer.
   */
  toTimer *timer(void);
public:
  toResult(void)
  { }
  virtual ~toResult()
  { }

  /** Re execute last query
   */
  virtual void refresh(void) = 0;
  /** Perform a query.
   * @param sql Execute an SQL statement.
   * @param params Parameters needed as input to execute statement.
   */
  virtual void query(const QString &sql,const list<QString> &params) = 0;
  /** Reexecute with changed parameters.
   * @param Param1 First parameter.
   */
  virtual void changeParams(const QString &Param1) = 0;
  /** Reexecute with changed parameters.
   * @param Param1 First parameter.
   * @param Param1 Second parameter.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2) = 0;
  /** Reexecute with changed parameters.
   * @param Param1 First parameter.
   * @param Param2 Second parameter.
   * @param Param3 Third parameter.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3) = 0;
};

#endif
