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
 *      software in the executable aside from Oracle client libraries.
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

#ifndef __TORESULTLONG_H
#define __TORESULTLONG_H

#include <qtimer.h>
#include "toresultview.h"

class QListViewItem;
class toresultlong;
class toResultTip;
class toNoBlockQuery;
class toResultStats;

/** A widget which contains the result of a query run in the background.
 */
class toResultLong : public toResultView {
  Q_OBJECT

  /** No blocking query object.
   */
  toNoBlockQuery *Query;
  /** Timer to poll for new data.
   */
  QTimer Timer;
  /** Indicator if headers have been parsed yet.
   */
  bool HasHeaders;
  /** Indicator if first row has been read yet.
   */
  bool First;
  /** Widget to store query statistics to.
   */
  toResultStats *Statistics;

  /** Check if at eof.
   */
  virtual bool eof(void);
public:
  /** Create widget.
   * @param readable Indicate if columns are to be made more readable. This means that the
   * descriptions are capitalised and '_' are converted to ' '.
   * @param numCol If number column is to be displayed.
   * @param conn Connection to issue query on.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultLong(bool readable,bool numCol,toConnection &conn,QWidget *parent,const char *name=NULL);
  /** Create widget. The columns are not readable and the number column is displayed.
   * @param conn Connection to issue query on.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultLong(toConnection &conn,QWidget *parent,const char *name=NULL);
  ~toResultLong();

  /** Set statistics widget.
   * @param stats Statistics widget.
   */
  void setStatistics(toResultStats *stats)
  { Statistics=stats; }

  /** Check if query is still running.
   * @return True if query is still running.
   */
  bool running(void)
  { return Query; }

  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const list<QString> &param);
  /** Reimplemented for internal reasons.
   */
  void query(const QString &sql)
  { list<QString> p; query(sql,p); }

signals:
  /** Emitted when query is finnished.
   */
  void done(void);
  /** Emitted when the first result is available.
   * @param sql SQL that was run.
   * @param res String describing result.
   */
  void firstResult(const QString &sql,const QString &res);

public slots:
  /** Stop running query.
   */
  void stop(void);
  /** Reimplemented for internal reasons.
   */
  virtual void addItem(void);
};

#endif
