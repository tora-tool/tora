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

#ifndef __TOSGASTATEMENT_H
#define __TOSGASTATEMENT_H

#include <qtabwidget.h>

class toResultView;
class QTabWidget;
class toResultField;
class QComboBox;
class toResultPlan;
class toResultItem;

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
   */
  toSGAStatement(QWidget *parent);
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
