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

#include <list>

#include "totabwidget.h"

#include <qobject.h>
#include <qtabwidget.h>

#include "totabwidget.moc"

int toTabBar::insertTab(QTab *newTab,int ix)
{
  std::list<barTab>::iterator i=Tabs.begin();
  if (ix>=0) {
    for(int j=0;j<ix&&i!=Tabs.end();j++)
      i++;
  } else
    i=Tabs.end();
  Tabs.insert(i,barTab(newTab,true));
  return QTabBar::insertTab(newTab,ix);
}

void toTabBar::removeTab(QTab *tab)
{
  for(std::list<barTab>::iterator i=Tabs.begin();i!=Tabs.end();i++) {
    if ((*i).Tab==tab) {
      QTabBar::removeTab(tab);
      Tabs.erase(i);
      return;
    }
  }
}

void toTabWidget::setTabShown(QWidget *w,bool value)
{
  int ix=indexOf(w);
  if (ix>=0) {
    Tabs->setTabShown(ix,value);
    QShowEvent e;
    showEvent(&e); // Force an update
  }
}

void toTabBar::setTabShown(int ix,bool shown)
{
  if (ix<0)
    return;
    
  std::list<barTab>::iterator i=Tabs.begin();
  for(int j=0;j<ix&&i!=Tabs.end();j++)
    i++;
  if (i!=Tabs.end())
    (*i).Shown=shown;
  layoutTabs();
}

void toTabBar::layoutTabs()
{
  QTabBar::layoutTabs();
  int offset=0;
  for(std::list<barTab>::iterator i=Tabs.begin();i!=Tabs.end();i++) {
    if ((*i).Tab->rect().isValid())
      (*i).Rect=(*i).Tab->rect();
    else
      (*i).Tab->setRect((*i).Rect);
    if ((*i).Shown) {
      QRect r=(*i).Tab->rect();
      r.moveBy(offset-r.left(),0);
      (*i).Tab->setRect(r);
      offset+=r.width();
    } else {
      QRect r;
      (*i).Tab->setRect(r);
    }
  }
}
