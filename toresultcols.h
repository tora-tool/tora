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

#ifndef __TORESULTCOLS_H
#define __TORESULTCOLS_H

#include "toresultview.h"

class toResultColsItem;

/** This widget displays information about the returned columns of an object
 * specified by the first and second parameter in the query. The sql is not
 * used in the query.
 */

class toResultCols : public toResultView {
public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultCols(QWidget *parent,const char *name=NULL);
  /** Reimplemented for internal reasons.
   */
  virtual void query(const QString &sql,const toQList &param);
  /** Support Oracle & MySQL
   */
  virtual bool canHandle(const toConnection &);
  friend class toResultColsItem;
};

#endif
