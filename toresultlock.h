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

#ifndef __TORESULTLOCK_H
#define __TORESULTLOCK_H

#include "toresultview.h"

class toResultLock : public toResultView {
  Q_OBJECT
public:
  toResultLock(QWidget *parent,const char *name=NULL);

  virtual void query(const QString &sql,const toQList &param);
  // Why is this needed?
  void query(const QString &sql)
  { toQList p; query(sql,p); }
  /** Support Oracle
   */
  virtual bool canHandle(toConnection &conn)
  { return toIsOracle(conn); }
};

#endif
