//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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
#include "toresultline.h"

class QTabWidget;
class QComboBox;
class toConnection;
class toListView;
class toResultStats;
class toResultView;
class toResultChart;
class QLabel;
class QGridLayout;
class QGrid;
class toBarChart;
class QPopupMenu;

class toTuningMiss : public toResultLine {
public:
  toTuningMiss(QWidget *parent=0,const char *name=0);
  virtual std::list<double> transform(std::list<double> &trans);
};

class toTuningFileIO : public QScrollView {
  Q_OBJECT

  QGrid *TablespaceTime;
  QGrid *TablespaceReads;
  QGrid *FileTime;
  QGrid *FileReads;
  time_t LastStamp;
  std::map<QString,toBarChart *> ReadsCharts;
  std::map<QString,toLineChart *> TimeCharts;
  std::map<QString,std::list<double> > LastValues;
  virtual void resizeEvent(QResizeEvent *e);

  void saveSample(const QString &,const QString &,
		  double reads,double writes,
		  double readBlk,double writeBlk,
		  double avgTim,double minTim,
		  double maxRead,double maxWrite);

  void allocCharts(const QString &,const QString &);

public:
  toTuningFileIO(QWidget *parent=0,const char *name=0,WFlags fl=0);
public slots:
  virtual void refresh(void);
  virtual void changeCharts(int val);
};

class toTuningOverview : public toTuningOverviewUI {
  std::list<QLabel *> Backgrounds;
  void setupChart(toResultLine *chart,const QString &,const QString &,toSQL &sql);
public:
  toTuningOverview(QWidget *parent=0,const char *name=0,WFlags fl=0);
  void refresh(void);
};

class toTuning : public toToolWidget {
  Q_OBJECT

  QPopupMenu *ToolMenu;
  QTabWidget *Tabs;
  toListView *Indicators;
  toResultView *Parameters;
  toResultStats *Statistics;
  QComboBox *Refresh;
  toTuningOverview *Overview;
  toTuningFileIO *FileIO;
public:
  toTuning(QWidget *parent,toConnection &connection);
public slots:
  virtual void refresh(void);
  virtual void changeRefresh(const QString &str);
  virtual void windowActivated(QWidget *widget);
};

#endif
