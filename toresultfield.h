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

#ifndef __TORESULTFIELD_H
#define __TORESULTFIELD_H

#include "tohighlightedtext.h"
#include "toresult.h"

class toSQL;

/** This widget displays the result of a query where each item in the stream
 * is added as a line in a text editer.
 */

class toResultField : public toHighlightedText, public toResult {
  Q_OBJECT

  /** SQL to run.
   */
  QString SQL;

public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultField(QWidget *parent,const char *name=NULL);

  /** Set the SQL to run.
   * @param sql Query.
   */
  void setSQL(const QString &sql)
  { SQL=sql; }
  /** Set the SQL to run.
   * @param sql Query.
   */
  void setSQL(toSQL &sql);

  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param);

  /** Run this query without parameters.
   * @param sql Query.
   */
  void query(const QString &sql)
  { toQList p; query(sql,p); }
  /** Run this query without parameters.
   * @param sql Query.
   */
  void query(toSQL &sql);
public slots:
  /** Reimplemented for internal reasons.
   */
  virtual void refresh(void)
  { query(SQL); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1)
  { toQList p; p.insert(p.end(),Param1); query(SQL,p); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2)
  { toQList p; p.insert(p.end(),Param1); p.insert(p.end(),Param2); query(SQL,p); }
  /** Reimplemented for internal reasons.
   */
  virtual void changeParams(const QString &Param1,const QString &Param2,const QString &Param3)
  { toQList p; p.insert(p.end(),Param1); p.insert(p.end(),Param2); p.insert(p.end(),Param3); query(SQL,p); }
};

#endif
