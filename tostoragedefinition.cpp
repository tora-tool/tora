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

#include <qtooltip.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qgroupbox.h>

#include "tofilesize.h"
#include "tostoragedefinition.h"

#include "tostoragedefinitionui.moc"

toStorageDefinition::toStorageDefinition(QWidget* parent,const char* name,WFlags fl)
  : toStorageDefinitionUI(parent,name,fl)
{
  InitialSize->setTitle("&Initial size");
  InitialSize->setValue(50);

  NextSize->setTitle("&Next size");
  NextSize->setValue(50);

  OptimalSize->setTitle("&Size");
  OptimalSize->setEnabled(false);
}

std::list<QString> toStorageDefinition::sql(void)
{
  QString str("STORAGE (INITIAL ");
  str.append(InitialSize->sizeString());
  str.append(" NEXT ");
  str.append(NextSize->sizeString());
  str.append(" MINEXTENTS ");
  str.append(InitialExtent->text());
  str.append(" MAXEXTENTS ");
  if (UnlimitedExtent->isChecked())
    str.append("UNLIMITED");
  else
    str.append(MaximumExtent->text());
  if (!Optimal->isEnabled()) {
    str.append(" PCTINCREASE ");
    str.append(PCTIncrease->text());
  }

  if (!OptimalNull->isChecked()) {
    str.append(" OPTIMAL ");
    str.append(OptimalSize->sizeString());
  }
  str.append(")");
  list<QString> ret;
  ret.insert(ret.end(),str);
  return ret;
}

void toStorageDefinition::forRollback(bool val)
{
  Optimal->setEnabled(val);
  PCTIncrease->setEnabled(!val);
  InitialExtent->setMinValue(2);
  InitialExtent->setValue(4);
}
