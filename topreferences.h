//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef TOPREFERENCES_H
#define TOPREFERENCES_H

#include <qvariant.h>
#include <qdialog.h>
#include <map>

#include "tohelp.h"
#include "topreferencesui.h"

class QHBoxLayout; 

class toPreferences : public toPreferencesUI, public toHelpContext
{ 
  Q_OBJECT

  QWidget *Shown;
  QVBoxLayout *Layout;

  std::map<QListBoxItem *,QWidget *> Tabs;

  void addWidget(QListBoxItem *,QWidget *);
public:
  toPreferences(QWidget* parent=0,const char* name=0,bool modal=false,WFlags fl=0);

  void saveSetting(void);

  static void displayPreferences(QWidget *parent);

public slots:
  void selectTab(QListBoxItem *item);
  void help(void); 
};

#endif // TOPREFERENCES_H
