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
  InitialSize->setTitle(tr("&Initial size"));
  InitialSize->setValue(50);

  NextSize->setTitle(tr("&Next size"));
  NextSize->setValue(50);

  OptimalSize->setTitle(tr("&Size"));
  OptimalSize->setEnabled(false);
}

std::list<QString> toStorageDefinition::sql(void)
{
  QString str(QString::fromLatin1("STORAGE (INITIAL "));
  str.append(InitialSize->sizeString());
  str.append(QString::fromLatin1(" NEXT "));
  str.append(NextSize->sizeString());
  str.append(QString::fromLatin1(" MINEXTENTS "));
  str.append(InitialExtent->text());
  str.append(QString::fromLatin1(" MAXEXTENTS "));
  if (UnlimitedExtent->isChecked())
    str.append(QString::fromLatin1("UNLIMITED"));
  else
    str.append(MaximumExtent->text());
  if (!Optimal->isEnabled()) {
    str.append(QString::fromLatin1(" PCTINCREASE "));
    str.append(PCTIncrease->text());
  }

  if (!OptimalNull->isChecked()) {
    str.append(QString::fromLatin1(" OPTIMAL "));
    str.append(OptimalSize->sizeString());
  }
  str.append(QString::fromLatin1(")"));
  std::list<QString> ret;
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
