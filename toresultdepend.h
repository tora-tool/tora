//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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

#ifndef __TORESULTDEPEND_H
#define __TORESULTDEPEND_H

#include <qtimer.h>

#include "toresultview.h"

class toNoBlockQuery;

/** This widget displays information about the dependencies of an object
 * specified by the first and second parameter in the query. The sql is not
 * used in the query. It will also recurs through all dependencies of the
 * objects depended on.
 */

class toResultDepend : public toResultView {
  Q_OBJECT

  /** Check if an object already exists.
   * @param owner Owner of object.
   * @param name Name of object.
   * @return True if object exists.
   */
  bool exists(const QString &owner,const QString &name);

  toNoBlockQuery *Query;
  QTimer Poll;
  QListViewItem *Current;
public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultDepend(QWidget *parent,const char *name=NULL);

  /** Object destructor.
   */
  ~toResultDepend();
  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param);
  /** Reimplemented for internal reasons.
   */
  void query(const QString &sql)
  { toQList p; query(sql,p); }
  /** Support Oracle
   */
  virtual bool canHandle(toConnection &conn)
  { return toIsOracle(conn); }
public slots:
  void poll(void);
};

#endif
