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

#include "toresultfield.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"
#include "tosql.h"
#include "toconnection.h"

#include "toresultfield.moc"

toResultField::toResultField(QWidget *parent,const char *name)
  : toHighlightedText(parent,name)
{
  setReadOnly(true);
}

void toResultField::query(const QString &sql,const toQList &param)
{
  SQL=sql;

  try {
    toQuery query(connection(),sql,param);

    QString text;

    while(!query.eof())
      text.append(query.readValue());
    setText(text);
  } TOCATCH
}

void toResultField::setSQL(toSQL &sql)
{
  SQL=toSQL::string(sql,connection());
  setFilename(sql.name());
}
void toResultField::query(toSQL &sql)
{
  setFilename(sql.name());
  query(toSQL::string(sql,connection()));
}
