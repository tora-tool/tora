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

#ifndef __TOBROWSER_H
#define __TOBROWSER_H

#include <map>

#include "totool.h"
#include "totemplate.h"

class QComboBox;
class QTabWidget;
class toResult;
class toResultView;
class QListViewItem;
class toResultFilter;
class QPopupMenu;

class toBrowser : public toToolWidget {
  Q_OBJECT

  QComboBox *Schema;
  QTabWidget *TopTab;
  QPopupMenu *ToolMenu;

  QString SecondText;
  toResultView *FirstTab;
  toResult *SecondTab;
  toResultFilter *Filter;

  std::map<QString,toResultView *> Map;
  std::map<QString,toResult *> SecondMap;
  void setNewFilter(toResultFilter *filter);
  QString schema(void);
public:
  toBrowser(QWidget *parent,toConnection &connection);
  virtual ~toBrowser();

  virtual bool canHandle(toConnection &conn);
public slots:
  void refresh(void);
  void changeSchema(int)
  { refresh(); }
  void changeTab(QWidget *tab);
  void changeSecondTab(QWidget *tab);
  void changeItem(QListViewItem *item);
  void clearFilter(void);
  void defineFilter(void);
  void windowActivated(QWidget *widget);
};

class toBrowseTemplate : public QObject,public toTemplateProvider {
  Q_OBJECT

  std::list<toTemplateItem *> Parents;
  bool Registered;
public:
  toBrowseTemplate(void)
    : toTemplateProvider()
  { Registered=false; }
  void insertItems(QListView *parent);
  void removeItem(QListViewItem *item);
public slots:
  void addDatabase(const QString &);
  void removeDatabase(const QString &); 
};

#endif
