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

#ifndef __TORESULTINDEXES_H
#define __TORESULTINDEXES_H

#include "toresultview.h"

/** This widget displays information about indexes of an object
 * specified by the first and second parameter in the query. The sql is not
 * used in the query.
 */

class toResultIndexes : public toResultView {
  /** Get columns of index.
   * @param conOwner Owner of object.
   * @param conName Name of index.
   * @return Columns separated by ','.
   */
  QString indexCols(const QString &conOwner,const QString &conName);
public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultIndexes(QWidget *parent,const char *name=NULL);
  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param);
  /** Support Oracle & MySQL
   */
  virtual bool canHandle(toConnection &conn);
};

#endif
