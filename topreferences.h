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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/


#ifndef TOPREFERENCES_H
#define TOPREFERENCES_H

#include <qvariant.h>
#include <qdialog.h>
#include <map>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QFrame;
class QListBox;
class QListBoxItem;
class QPushButton;

class toPreferences : public QDialog
{ 
  Q_OBJECT

  QWidget *Shown;

  map<QListBoxItem *,QWidget *> Tabs;

  void addWidget(QListBoxItem *,QWidget *);

  QListBox* TabSelection;
  QPushButton* CancelButton;
  QPushButton* OkButton;
public:
  toPreferences(QWidget* parent=0,const char* name=0,bool modal=false,WFlags fl=0);

  void saveSetting(void);

  static void displayPreferences(QWidget *parent);

public slots:
  void selectTab(QListBoxItem *item);
};

#endif // TOPREFERENCES_H
