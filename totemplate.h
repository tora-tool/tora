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

#ifndef __TOTEMPLATE_H
#define __TOTEMPLATE_H

#include <list>
#include <map>

#include <qvbox.h>
#include "toresultview.h"

class QListView;
class QListViewItem;
class QSplitter;
class QTextView;
class toConnection;
class toListView;
class toTemplateItem;
class toTemplateProvider;

class toTemplate : public QVBox {
  Q_OBJECT

  QSplitter *Splitter;
  toListView *List;
  map<toTemplateItem *,toTemplateProvider *> Providers;
  QWidget *WidgetExtra;
public:
  toTemplate(QWidget *parent);
  virtual ~toTemplate();
  void setWidget(QWidget *widget);
  QWidget *widget(void)
  { return WidgetExtra; }
public slots:
  void expand(QListViewItem *item);
  void collapse(QListViewItem *item);
};

class toTemplateProvider {
  static list<toTemplateProvider *> *Providers;
public:
  toTemplateProvider();
  virtual ~toTemplateProvider()
  { }

  virtual toTemplateItem *insertItem(QListView *parent)=0;
  virtual void removeItem(toTemplateItem *item)=0;

  static QWidget *parentWidget(QListViewItem *item);
  static toTemplate *templateWidget(QListViewItem *item);

  friend class toTemplate;
};

class toTemplateItem : public toResultViewItem {
  toTemplateProvider &Provider;
public:
  toTemplateItem(toTemplateProvider &prov,QListView *parent,const QString &name)
    : toResultViewItem(parent,NULL,name),Provider(prov)
  { }
  toTemplateItem(toTemplateItem *parent,const QString &name)
    : toResultViewItem(parent,NULL,name),Provider(parent->provider())
  { }
  toTemplateProvider &provider(void)
  { return Provider; }
  virtual void expand(void)
  { }
  virtual void collapse(void)
  { }
  virtual void setSelected(bool);
};

class toTemplateText : public toTemplateItem {
  const QString Note;
public:
  toTemplateText(toTemplateItem *parent,const QString &name,const QString &note)
    : toTemplateItem(parent,name), Note(note)
  { }
  virtual void setSelected(bool);
};

#endif
