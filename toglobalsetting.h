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

#ifndef __TOGLOBALSETTING_H
#define __TOGLOBALSETTING_H

#include "totool.h"
#include "toglobalsettingui.h"
#include "todatabasesettingui.h"

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

#endif
