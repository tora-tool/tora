/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2005 Quest Software, Inc
 * Portions Copyright (C) 2005 Other Contributors
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 *      You may link this product with any GPL'd Qt library such as Qt/Free
 *
 * All trademarks belong to their respective owners.
 *
 *****/

#include <stdio.h>

#include <qradiobutton.h>
#include <qspinbox.h>
#include <qlayout.h>

#include "tofilesize.h"

#include "tofilesize.moc"

void toFilesize::setup()
{
  QVBoxLayout *vlayout=new QVBoxLayout(this);
  vlayout->addSpacing(15);

  QHBoxLayout *layout=new QHBoxLayout(vlayout);
  layout->setMargin(10);
  layout->setSpacing(10);
  vlayout->addSpacing(2);

  Value=new QSpinBox(this,"Value");
  Value->setLineStep(1024);
  Value->setMaxValue(2097151);
  Value->setMinValue(1);
  Value->setValue(1024);
  layout->addWidget(Value);
  layout->setStretchFactor(Value,100);

  MBSize=new QRadioButton(this,"MBSize");
  MBSize->setText(QString::fromLatin1("MB"));
  MBSize->setChecked(true);
  layout->addWidget(MBSize);
  layout->setStretchFactor(MBSize,1);
  connect(MBSize,SIGNAL(toggled(bool)),this,SLOT(changeType(bool)));

  KBSize=new QRadioButton(this, "MBSize" );
  KBSize->setText(QString::fromLatin1("KB"));
  KBSize->setChecked(false);
  layout->addWidget(KBSize);
  layout->setStretchFactor(KBSize,1);
  connect(Value,SIGNAL(valueChanged(int)),this,SLOT(changedSize(void)));
}

toFilesize::toFilesize(QWidget* parent,const char* name)
  : QButtonGroup(parent,name)
{
  setup();
}

toFilesize::toFilesize(const QString &title,QWidget* parent,const char* name)
  : QButtonGroup(title,parent,name)
{
  setup();
}

int toFilesize::value(void)
{
  if (MBSize->isChecked())
    return Value->value()*1024;
  else
    return Value->value();
}

void toFilesize::changeType(bool)
{
  if (MBSize->isChecked()) {
#if 0
    Value->setValue(Value->value()/1024);
#endif
    Value->setMaxValue(2097151);
  } else {
    Value->setMaxValue(2147483647);
#if 0
    Value->setValue(Value->value()*1024);
#endif
  }
}

void toFilesize::setValue(int sizeInKB)
{
  if (sizeInKB%1024==0) {
    MBSize->setChecked(true);
    KBSize->setChecked(false);
    Value->setValue(sizeInKB/1024);
    Value->setMaxValue(2097151);
  } else {
    MBSize->setChecked(false);
    KBSize->setChecked(true);
    Value->setMaxValue(2147483647);
    Value->setValue(sizeInKB);
  }
}

QString toFilesize::sizeString(void)
{
  char buf[30];
  if (MBSize->isChecked())
    sprintf(buf,"%d K",Value->value()*1024);
  else
    sprintf(buf,"%d K",Value->value());
  return QString::fromLatin1(buf);
}
