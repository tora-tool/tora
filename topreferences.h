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

#ifndef TOPREFERENCES_H
#define TOPREFERENCES_H

#include <qvariant.h>
#include <qdialog.h>
#include <map>

#include "tohelp.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QFrame;
class QListBox;
class QListBoxItem;
class QPushButton;

class toPreferences : public QDialog, public toHelpContext
{ 
  Q_OBJECT

  QWidget *Shown;

  std::map<QListBoxItem *,QWidget *> Tabs;

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
