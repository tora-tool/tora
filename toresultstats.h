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
public:
  /** Create statistics widget for session statistics.
   * @param OnlyChanged Only display changed items.
   * @param ses Session ID to display info about.
   * @param conn Connection to display.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultStats(bool OnlyChanged,int ses,toConnection &conn,QWidget *parent,const char *name=NULL);
  /** Create statistics widget for the current session statistics.
   * @param OnlyChanged Only display changed items.
   * @param conn Connection to display.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultStats(bool OnlyChanged,toConnection &conn,QWidget *parent,const char *name=NULL);
  /** Create statistics widget for the current database statistics.
   * @param conn Connection to display.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultStats(toConnection &conn,QWidget *parent,const char *name=NULL);

  /** Reset statistics. Read in last values without updating widget data.
   */
  void resetStats(void);
  /** Change the session to the current session.
   * @param conn Connection to change session id to.
   */
  void changeSession(otl_connect &conn);
  /** Change session to specified id.
   * @param ses Session ID to change to.
   */
  void changeSession(int ses);
public slots:
  /** Update the statistics.
   * @param reset Set delta to current values. This means that the next time this widget
   *              is updated the delta will be from the new values.
   */
  void refreshStats(bool reset=true);
};

#endif
