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

#ifndef __TOTUNING_H
#define __TOTUNING_H

#include <list>
#include <map>

#include <qscrollview.h>
#include <qgrid.h>

#include "totool.h"
#include "totuningoverviewui.h"
#include "toresultline.h"

class QTabWidget;
class QComboBox;
class toConnection;
class toListView;
class toResultStats;
class toResultParam;
class toResultLong;
class toResultChart;
class toResultItem;
class toPieChart;
class QLabel;
class QGridLayout;
class QGrid;
class toBarChart;
class QPopupMenu;
class toLegendChart;
class toResultLock;

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

  void stop(void);
  void start(void);
public slots:
  virtual void refresh(void);
  virtual void changeCharts(int val);
  virtual void changeConnection(void);
};

class toTuningOverview : public toTuningOverviewUI {
  std::list<QLabel *> Backgrounds;
  void setupChart(toResultLine *chart,const QString &,const QString &,toSQL &sql);
public:
  toTuningOverview(QWidget *parent=0,const char *name=0,WFlags fl=0);
  void refresh(void);
  void stop(void);
  void start(void);
};

class toTuningWait : public QFrame {
  Q_OBJECT

  toBarChart *Delta;
  toPieChart *AbsolutePie;
  toPieChart *DeltaPie;
  QListView *Types;
  toNoBlockQuery *Query;
  toBackground Poll;

  bool First;
  bool ShowTimes;
  QString Now;
  std::list<QString> Labels;
  std::list<QString> XValues;
  std::list<time_t> TimeStamp;
  std::list<std::list<double> > Values;
  std::list<std::list<double> > Times;
  std::list<double> Current;
  std::list<double> CurrentTimes;
  std::map<int,int> LabelMap;

public:
  toTuningWait(QWidget *parent,const char *name);
public slots:
  virtual void connectionChanged(void);
  virtual void changeSelection(void);
  virtual void poll(void);
  virtual void refresh(void);
  virtual void start(void);
  virtual void stop(void);
  virtual void changeType(int);
};

class toTuning : public toToolWidget {
  Q_OBJECT

  QPopupMenu *TabMenu;
  QToolButton *TabButton;

  QPopupMenu *ToolMenu;
  QTabWidget *Tabs;
  toListView *Indicators;
  toResultParam *Parameters;
  toResultLong *Options;
  toResultLock *BlockingLocks;
  toResultItem *Licenses;
  toResultStats *Statistics;
  toTuningWait *Waits;

  QGrid *Charts;

  QComboBox *Refresh;
  toTuningOverview *Overview;
  toTuningFileIO *FileIO;
  QWidget *LastTab;

  virtual void enableTab(const QString &name,bool enable);
  virtual QWidget *tabWidget(const QString &name);

  virtual void exportData(std::map<QString,QString> &data,const QString &prefix);
  virtual void importData(std::map<QString,QString> &data,const QString &prefix);
public:
  toTuning(QWidget *parent,toConnection &connection);
public slots:
  virtual void refresh(void);
  virtual void changeTab(QWidget *widget); 
  virtual void changeRefresh(const QString &str);
  virtual void windowActivated(QWidget *widget);

  virtual void showTabMenu(void);
  virtual void enableTabMenu(int);
};

#endif
