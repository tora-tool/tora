//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef TORESULTPLAN_H
#define TORESULTPLAN_H

#include "tobackground.h"
#include "toresultview.h"

#include <map>

class toNoBlockQuery;

/** This widget displays the execution plan of a statement. The statement
 * is identified by the first parameter which should be the address as gotten
 * from the @ref toSQLToAddress function.
 */

class toResultPlan : public toResultView {
  Q_OBJECT

  QString Ident;
  std::map <QString,QListViewItem *> Parents;
  std::map <QString,QListViewItem *> Last;
  QListViewItem *LastTop;
  bool Reading;
  toNoBlockQuery *Query;
  toBackground Poll;
  void checkException(const QString &);
  void oracleSetup(void);
public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultPlan(QWidget *parent,const char *name=NULL);

  /** Destruct object
   */
  ~toResultPlan();

  /** Reimplemented for internal reasons. If you prepend "SAVED:" a saved plan is read
   * with the identified_by set to the string following the initial "SAVED:" string.
   */
  virtual void query(const QString &sql,const toQList &param);
  /** Reimplemented for internal reasons.
   */
  void query(const QString &sql)
  { toQList p; query(sql,p); }
  /** Support Oracle
   */
  virtual bool canHandle(toConnection &conn);
private slots:
  void poll();
};

#endif
