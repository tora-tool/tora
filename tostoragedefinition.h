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

#ifndef __TOSTORAGEDEFINITION_H
#define __TOSTORAGEDEFINITION_H

#include <list>

#include <qwidget.h>

#include "tostoragedefinitionui.h"

class QGroupBox;
class QFrame;
class QCheckBox;
class QLabel;
class QSpinBox;
class toFilesize;
class QGroupBox;
class toStorageDialog;

class toStorageDefinition : public toStorageDefinitionUI
{ 
public:
  toStorageDefinition(QWidget *parent=0,const char *name=0,WFlags fl=0);

  void forRollback(bool val);

  std::list<QString> sql(void);

  friend class toStorageDialog;
};

#endif
