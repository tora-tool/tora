/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include <qtooltip.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qgroupbox.h>

#include "tofilesize.h"
#include "tostoragedefinition.h"

#include "tostoragedefinition.moc"

toStorageDefinition::toStorageDefinition(QWidget* parent,const char* name,WFlags fl)
  : QWidget(parent,name,fl)
{
  if (!name)
    setName("toStorageDefinition");
  setCaption(tr("Form1"));

  InitialSize=new toFilesize("&Initial size",this,"InitialSize");
  InitialSize->setGeometry(QRect(10,10,310,60)); 
  InitialSize->setValue(50);

  NextSize=new toFilesize("&Next size",this,"NextSize");
  NextSize->setGeometry(QRect(10,80,310,60)); 
  NextSize->setValue(50);

  GroupBox1=new QGroupBox(this,"GroupBox1");
  GroupBox1->setGeometry(QRect(10,150,310,115)); 
  GroupBox1->setTitle(tr("&Extents"));

  Line1=new QFrame(GroupBox1,"Line1");
  Line1->setGeometry(QRect(145,16,20,90)); 
  Line1->setFrameStyle(QFrame::VLine | QFrame::Sunken);

  InitialExtent=new QSpinBox(GroupBox1,"InitialExtent");
  InitialExtent->setGeometry(QRect(10,80,130,23)); 
  InitialExtent->setMinValue(1);
  InitialExtent->setValue(1);
  InitialExtent->setMaxValue(2147483647);

  UnlimitedExtent=new QCheckBox(GroupBox1,"UnlimitedExtent");
  UnlimitedExtent->setGeometry(QRect(170,25,130,19)); 
  UnlimitedExtent->setText(tr("&Unlimited Extents"));
  UnlimitedExtent->setChecked(true);
  QToolTip::add( UnlimitedExtent,tr("No limit on the number of allocated extents.\nCan caused decreased performance due to fragmentation."));
  connect(UnlimitedExtent,SIGNAL(toggled(bool)),this,SLOT(unlimitedExtents(bool)));

  TextLabel1_2=new QLabel(GroupBox1,"TextLabel1_2");
  TextLabel1_2->setGeometry(QRect(170,55,110,16));
  TextLabel1_2->setText(tr("&Maximum Extents"));
  QToolTip::add( TextLabel1_2,tr("Maximum number of extents to allocate."));

  MaximumExtent=new QSpinBox(GroupBox1,"MaximumExtent");
  MaximumExtent->setGeometry(QRect(170,80,130,23)); 
  MaximumExtent->setMinValue(1);
  MaximumExtent->setValue(1);
  MaximumExtent->setMaxValue(2147483647);
  MaximumExtent->setEnabled(false);

  TextLabel1=new QLabel(GroupBox1,"TextLabel1");
  TextLabel1->setGeometry(QRect(10,55,110,16)); 
  TextLabel1->setText(tr("Initial &Extents"));
  QToolTip::add( TextLabel1,tr("Number of extents to initially allocated."));

  Optimal=new QGroupBox(this,"Optimal");
  Optimal->setGeometry(QRect(10,275,310,130)); 
  Optimal->setTitle(tr("&Optimal Size"));
  QToolTip::add( Optimal,tr("Optimal allocation of extents. Will free unused extents down to specified value."));
  Optimal->setEnabled(false);

  OptimalNull=new QCheckBox(Optimal,"OptimalNull");
  OptimalNull->setGeometry(QRect(10,25,130,19)); 
  OptimalNull->setText(tr("&No Optimal Size"));
  OptimalNull->setChecked(true);
  QToolTip::add( OptimalNull,tr("Don't deallocated unused extents"));
  connect(OptimalNull,SIGNAL(toggled(bool)),this,SLOT(optimalExtents(bool)));

  OptimalSize=new toFilesize("&Size",Optimal,"OptimalSize");
  OptimalSize->setGeometry(QRect(10,55,290,60)); 
  OptimalSize->setEnabled(false);

  PCTIncrease=new QSpinBox(this,"PCTIncrease");
  PCTIncrease->setGeometry(QRect(170,415,150,23)); 
  PCTIncrease->setValue(0);
  PCTIncrease->setMaxValue(1000);
  PCTIncrease->setLineStep(10);

  TextLabel2=new QLabel(this,"TextLabel2");
  TextLabel2->setGeometry(QRect(10,420,119,15)); 
  TextLabel2->setText(tr("&Default PCT Increase"));
  QToolTip::add( TextLabel2,tr("Default increase in size of next allocated extent.\nA size of 0 will prevent background coalesce of free space."));

  // buddies
  TextLabel1_2->setBuddy(MaximumExtent);
  TextLabel1->setBuddy(InitialExtent);
  TextLabel2->setBuddy(PCTIncrease);
}

QString toStorageDefinition::getSQL(void)
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
  return str;
}

void toStorageDefinition::optimalExtents(bool val)
{
  OptimalSize->setEnabled(!val);
}

void toStorageDefinition::forRollback(bool val)
{
  Optimal->setEnabled(val);
  PCTIncrease->setEnabled(!val);
  InitialExtent->setMinValue(2);
  InitialExtent->setValue(4);
}

void toStorageDefinition::unlimitedExtents(bool val)
{
  MaximumExtent->setEnabled(!val);
}
