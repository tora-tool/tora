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

#ifndef __TOSGASTATEMENT_H
#define __TOSGASTATEMENT_H

#include <qtabwidget.h>

class toResultView;
class QTabWidget;
class toResultField;
class QComboBox;
class toResultPlan;
class toResultItem;
class toConnection;
class QTimer;

/** This widget displays information about a statement in the Oracle SGA. To get an
 * address use the @ref toSQLToAddress function.
 */

class toSGAStatement : public QTabWidget {
  Q_OBJECT
  /** The SQL run.
   */
  toResultField *SQLText;
  /** Tab widget
   */
  QWidget *CurrentTab;
  /** Resources used by the statement.
   */
  toResultItem *Resources;
  /** Connection of this widget.
   */
  toConnection &Connection;
  /** Address of the statement.
   */
  QString Address;
  /** Execution plan of the statement.
   */
  toResultPlan *Plan;

  /** Display resources of statement.
   */
  void viewResources(void);
private slots:
  /** Change the displayed tab.
   */
  void changeTab(QWidget *widget);
public:
  /** Create widget.
   * @param parent Parent widget.
   * @param connection Connection to use to get data.
   */
  toSGAStatement(QWidget *parent,toConnection &connection);
public slots:
  /** Display another statement.
   * @param address Address of the statement to display.
   */
  virtual void changeAddress(const QString &address);
  /** Update the contents of this widget.
   */ 
  void refresh(void)
  { changeTab(CurrentTab); }
};

#endif
