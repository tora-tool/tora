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

#ifndef __TORESULTLONG_H
#define __TORESULTLONG_H

#include <qtimer.h>
#include "toresultview.h"
#include "toconnection.h"

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
  toTimer Timer;
  /** Indicator if headers have been parsed yet.
   */
  bool HasHeaders;
  /** Indicator if first row has been read yet.
   */
  bool First;
  /** Widget to store query statistics to.
   */
  toResultStats *Statistics;
  /** Description of query
   */
  toQDescList Description;
  /** Number of rows to fetch when starting. (-1 for all)
   */
  int MaxNumber;

  /** Check if at eof.
   */
  virtual bool eof(void);
  /** Clean up when query is done
   */
  void cleanup(void);
public:
  /** Create widget.
   * @param readable Indicate if columns are to be made more readable. This means that the
   * descriptions are capitalised and '_' are converted to ' '.
   * @param numCol If number column is to be displayed.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultLong(bool readable,bool numCol,QWidget *parent,const char *name=NULL);
  /** Create widget. The columns are not readable and the number column is displayed.
   * @param parent Parent of list.
   * @param name Name of widget.
   */
  toResultLong(QWidget *parent,const char *name=NULL);
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
  virtual void query(const QString &sql,const toQList &param);
  /** Reimplemented for internal reasons.
   */
  void query(const QString &sql)
  { toQList p; query(sql,p); }
  /** Reimplemented for internal reasons.
   */
  virtual void readAll(void);
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
