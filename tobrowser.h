//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

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
  void changeSecond(void);
public:
  toBrowser(QWidget *parent,toConnection &connection);
  virtual ~toBrowser();

  virtual bool canHandle(toConnection &conn);
public slots:
  void refresh(void);
  void updateTabs(void);
  void changeSchema(int)
  { refresh(); }
  void changeTab(QWidget *tab);
  void changeSecondTab(QWidget *tab);
  void changeItem(QListViewItem *item);
  void clearFilter(void);
  void defineFilter(void);
  void windowActivated(QWidget *widget);
  void firstDone(void);

  void modifyTable(void);
  void addTable(void);
  void fixIndexCols(void);
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
