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

#ifndef TOBROWSER_H
#define TOBROWSER_H

#include "totemplate.h"
#include "totool.h"

#include <map>

class QComboBox;
class QListViewItem;
class QPopupMenu;
class QTabWidget;
class QToolBar;
class toBrowserFilter;
class toResult;
class toResultCombo;
class toResultContent;
class toResultFilter;
class toResultView;

class toBrowser : public toToolWidget {
  Q_OBJECT

  toResultCombo *Schema;
  QTabWidget *TopTab;
  QPopupMenu *ToolMenu;

  QString SecondText;
  toResultView *FirstTab;
  toResult *SecondTab;
  toBrowserFilter *Filter;
  QWidget *CurrentTop;

  toResultContent *ViewContent;
  toResultContent *TableContent;

  std::map<QCString,toResultView *> Map;
  std::map<QCString,toResult *> SecondMap;
  void setNewFilter(toBrowserFilter *filter);
  QTimer Poll;

  QString schema(void);
  void enableDisableConstraints(const QString &);
public:
  toBrowser(QWidget *parent,toConnection &connection);
  virtual ~toBrowser();

  virtual bool canHandle(toConnection &conn);

  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix);
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix);
public slots:
  void refresh(void);
  void updateTabs(void);
  void changeSchema(int);
  void changeTab(QWidget *tab);
  void changeSecond(void);
  void changeSecondTab(QWidget *tab);
  void changeItem(QListViewItem *item);
  void clearFilter(void);
  void defineFilter(void);
  void windowActivated(QWidget *widget);
  void firstDone(void);
  void focusObject(void);

  void modifyTable(void);
  void addTable(void);
  void fixIndexCols(void);

  void enableConstraints(void);
  void disableConstraints(void);
};

class toBrowseTemplate : public QObject,public toTemplateProvider {
  Q_OBJECT

  toBrowserFilter *Filter;
  std::list<toTemplateItem *> Parents;
  bool Registered;
public:
  toBrowseTemplate(void)
    : QObject(NULL,"browsertemplate"),toTemplateProvider("Browser")
  { Registered=false; Filter=NULL; }
  virtual void insertItems(QListView *parent,QToolBar *toolbar);
  virtual void removeItem(QListViewItem *item);
  toBrowserFilter *filter(void)
  { return Filter; }
  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix);
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix);
public slots:
  void addDatabase(const QString &);
  void removeDatabase(const QString &); 
  void defineFilter(void);
  void clearFilter(void);
};

class toBrowseButton : public QToolButton {
  Q_OBJECT
public:
  toBrowseButton(const QIconSet &iconSet,
		 const QString &textLabel,
		 const QString & grouptext,
		 QObject * receiver,
		 const char * slot,
		 QToolBar * parent,
		 const char * name=0);
private slots:
  void connectionChanged(void);
};

#endif
