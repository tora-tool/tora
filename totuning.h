//***************************************************************************
/*
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
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

#ifndef __TOTUNING_H
#define __TOTUNING_H

#include <list>
#include <map>

#include <qscrollview.h>

#include "totool.h"
#include "totuningoverviewui.h"

class QTabWidget;
class QComboBox;
class toConnection;
class toListView;
class toResultStats;
class toResultView;
class toResultChart;
class QLabel;
class QGridLayout;
class QVBox;
class toBarChart;

class toTuningFileIO : public QScrollView {
  Q_OBJECT

  QVBox *Box;
  map<QString,toBarChart *> Charts;
  time_t LastStamp;
  map<QString,list<double> > LastValues;
  virtual void resizeEvent(QResizeEvent *e);
public:
  toTuningFileIO(QWidget *parent=0,const char *name=0,WFlags fl=0);
public slots:
  virtual void refresh(void);
};

class toTuningOverview : public toTuningOverviewUI {
  list<QLabel *> Backgrounds;
  void setupChart(toResultLine *chart,const QString &,const QString &,toSQL &sql);
public:
  toTuningOverview(QWidget *parent=0,const char *name=0,WFlags fl=0);
  void refresh(void);
};

class toTuning : public toToolWidget {
  Q_OBJECT

  QTabWidget *Tabs;
  toListView *Indicators;
  toResultView *Parameters;
  toResultStats *Statistics;
  QComboBox *Schema;
  toTuningOverview *Overview;
  toTuningFileIO *FileIO;
public:
  toTuning(QWidget *parent,toConnection &connection);
public slots:
  virtual void refresh(void);
  virtual void changeRefresh(const QString &str);
};

#endif
