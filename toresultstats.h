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

#ifndef __TORESULTSTATS_H
#define __TORESULTSTATS_H

#include "toresultview.h"

#define TO_STAT_MAX 500
#define TO_STAT_BLOCKS 10

/** This widget will displays information about statistics in either a database or a session.
 */

class toResultStats : public toResultView {
  Q_OBJECT
  /** Session ID to get statistics for.
   */
  int SessionID;
  /** Number of rows of statistics
   */
  int Row;
  /** Get information about if only changed items are to be displayed.
   */
  bool OnlyChanged;
  /** Display system statistics.
   */
  bool System;
  /** Last read values, used to calculate delta values.
   */
  double LastValues[TO_STAT_MAX+TO_STAT_BLOCKS];
  /** Setup widget.
   */
  void setup();
  /** Add value
   */
  void addValue(bool reset,int id,const QString &name,double value);
public:
  /** Create statistics widget for session statistics.
   * @param OnlyChanged Only display changed items.
   * @param ses Session ID to display info about.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultStats(bool OnlyChanged,int ses,QWidget *parent,const char *name=NULL);
  /** Create statistics widget for the current session statistics.
   * @param OnlyChanged Only display changed items.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultStats(bool OnlyChanged,QWidget *parent,const char *name=NULL);
  /** Create statistics widget for the current database statistics.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultStats(QWidget *parent,const char *name=NULL);

  /** Reset statistics. Read in last values without updating widget data.
   */
  void resetStats(void);
  /** Change the session that the current query will run on.
   * @param query Query to check connection for.
   */
  void changeSession(toQuery &conn);
  /** Change session to specified id.
   * @param ses Session ID to change to.
   */
  void changeSession(int ses);
  /** Support Oracle
   */
  virtual bool canHandle(toConnection &conn)
  { return toIsOracle(conn); }
signals:
  /** Emitted when session is changed.
   * @param ses New session ID.
   */
  void sessionChanged(int ses);
  /** Emitted when session is changed.
   * @param ses New session ID as string.
   */
  void sessionChanged(const QString &);
public slots:
  /** Update the statistics.
   * @param reset Set delta to current values. This means that the next time this widget
   *              is updated the delta will be from the new values.
   */
  void refreshStats(bool reset=true);
};

#endif
