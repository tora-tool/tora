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
  MBSize->setText(tr("MB"));
  MBSize->setChecked(true);
  layout->addWidget(MBSize);
  layout->setStretchFactor(MBSize,1);
  connect(MBSize,SIGNAL(toggled(bool)),this,SLOT(changeType(bool)));

  KBSize=new QRadioButton(this, "MBSize" );
  KBSize->setText(tr("KB"));
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
  return buf;
}
