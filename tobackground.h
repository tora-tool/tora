/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2005 Quest Software, Inc
 * Portions Copyright (C) 2005 Other Contributors
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
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 *      You may link this product with any GPL'd Qt library such as Qt/Free
 *
 * All trademarks belong to their respective owners.
 *
 *****/

#ifndef TOBACKGROUND_H
#define TOBACKGROUND_H

#include <qtimer.h>

class QLabel;
class QMovie;

/** A timer descendant which also keep track of the last timer setting sent to it.
 */
class toTimer : public QTimer {
  int LastTimer;
public:
  /** Create timer.
   * @param parent Parent object of timer.
   * @param name Name of timer.
   */
  toTimer(QObject *parent=0,const char * name=0)
    : QTimer(parent,name)
  { }
  /** Start timer.
   * @param msec Milliseconds to timeout.
   * @param sshot Set to true if only timeout once.
   */
  int start(int msec,bool sshot=false)
  { LastTimer=msec; return QTimer::start(msec,sshot); }
  /** Get last timer start timeout.
   * @return Last timeout in millisecond.
   */
  int lastTimer(void)
  { return LastTimer; }
};

/**
 * A descendant of timer which will indicate in the statusbar of the
 * main window when any timer is running. Can not do singleshots, only
 * periodic intervals.
 */
class toBackground : public toTimer {
public:
  /**
   * Create a background timer widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toBackground(QObject* parent=0,const char* name=0);
  ~toBackground();
  /** Start repeating timer with msec interval.
   * @param msec Millsecond repeat interval.
   */
  void start(int msec);
  /** Stop timer
   */
  void stop(void);

  static void init(void);
};

#endif
