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

#ifndef TOTABWIDGET_H
#define TOTABWIDGET_H

#include <qtabbar.h>
#include <qtabwidget.h>
#include <list>

class toTabBar : public QTabBar {
  Q_OBJECT

  struct barTab {
    QTab *Tab;
    bool Shown;
    QRect Rect;

    barTab(QTab *tab,bool shown)
    { Tab=tab; Shown=shown; }
  };

  std::list<barTab> Tabs;
  QTab *copyTab(QTab *tab);
public:
  toTabBar(QWidget *parent=0,const char *name=0)
    : QTabBar(parent,name)
  { }
  virtual int insertTab(QTab *newTab,int ix=-1);
  virtual int addTab(QTab *newTab)
  { return insertTab(newTab); }
  virtual void removeTab(QTab *t);

  virtual void layoutTabs();
  virtual void setTabShown(int ix,bool shown);
};

class toTabWidget: public QTabWidget{ 
  Q_OBJECT
  toTabBar *Tabs;
public:
  toTabWidget(QWidget *parent=0,const char *name=0,WFlags f=0)
    : QTabWidget(parent,name,f)
  { setTabBar(Tabs=new toTabBar(this)); }

  virtual void setTabShown(QWidget *w, bool value);

  virtual void showTab(QWidget *w)
  { setTabShown(w,true); }
  virtual void hideTab(QWidget *w)
  { setTabShown(w,false); }
};

#endif
