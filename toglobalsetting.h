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

#ifndef __TOGLOBALSETTING_H
#define __TOGLOBALSETTING_H

#include "totool.h"
#include "toglobalsettingui.h"
#include "todatabasesettingui.h"
#include "totoolsettingui.h"
#include "tohelp.h"

class toGlobalSetting : public toGlobalSettingUI, public toSettingTab {
public:
  toGlobalSetting(QWidget *parent=0,const char *name=0,WFlags fl=0);

  virtual void saveSetting(void);

  virtual void pluginBrowse(void);
  virtual void sqlBrowse(void);
  virtual void helpBrowse(void);
};

class toDatabaseSetting : public toDatabaseSettingUI, public toSettingTab {
public:
  toDatabaseSetting(QWidget *parent=0,const char *name=0,WFlags fl=0);

  virtual void saveSetting(void);
  virtual void createPlanTable(void);
};

class toToolSetting : public toToolSettingUI, public toSettingTab {
  QString FirstTool;
public:
  toToolSetting(QWidget *parent=0,const char *name=0,WFlags fl=0);

  virtual void saveSetting(void);
  virtual void changeEnable(void);
};

#endif
