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

#ifndef __TORESULTRESOURCES_H
#define __TORESULTRESOURCES_H

#include "toresultitem.h"
#include "tosql.h"

#define TOSQL_RESULTRESOURCE "toResultResources:Information"

/** This widget displays information about resources of a query. The statement
 * is identified by the first parameter which should be the address as gotten
 * from the @ref toSQLToAddress function.
 */

class toResultResources : public toResultItem {
public:
  /** Create the widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toResultResources(QWidget *parent,const char *name=NULL)
    : toResultItem(3,true,parent,name)
  { toSQL sql(toSQL::sql(TOSQL_RESULTRESOURCE)); setSQL(sql); }
};

#endif
