/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2005 Quest Software, Inc
 * Portions Copyright (C) 2005 Other Contributors
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
 *      You may link this product with any GPL'd Qt library such as Qt/Free
 *
 * All trademarks belong to their respective owners.
 *
 *****/

#ifndef TOTUNING_H
#define TOTUNING_H

#include "config.h"
#include "tobackground.h"
#include "toresultline.h"
#include "tothread.h"
#include "totool.h"
#include "totuningoverviewui.h"

#include <list>
#include <map>

#include <qgrid.h>
#include <qscrollview.h>

class QComboBox;
class QGrid;
class QGridLayout;
class QLabel;
class QPopupMenu;
class QTabWidget;
class toBarChart;
class toConnection;
class toLegendChart;
class toListView;
class toNoBlockQuery;
class toPieChart;
class toResultChart;
class toResultItem;
class toResultLock;
class toResultLong;
class toResultParam;
class toResultStats;
class toWaitEvents;

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

  QString LastTablespace;
  double TblReads;
  double TblWrites;
  double TblReadBlk;
  double TblWriteBlk;
  double TblAvg;
  double TblMin;
  double TblMaxRead;
  double TblMaxWrite;
  toNoBlockQuery *Query;

  time_t CurrentStamp;
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

  void allocCharts(const QString &);

  toBackground Poll;
public:
  toTuningFileIO(QWidget *parent=0,const char *name=0,WFlags fl=0);
  ~toTuningFileIO();

  void stop(void);
  void start(void);
public slots:
  void refresh(void);
  void changeCharts(int val);
  void changeConnection(void);
  void poll(void);
};

class toTuningOverview : public toTuningOverviewUI {
  Q_OBJECT

  bool Quit;
  toSemaphore Done;
  std::map<QCString,QString> Values;
  toConnection *Connection;
  toLock Lock;
  QString UnitString;

  struct overviewQuery : public toTask {
    toTuningOverview &Parent;
    overviewQuery(toTuningOverview &parent)
      : Parent(parent)
    { }
    virtual void run(void);
    void setValue(const QCString &name,const QString &val);
  };

  friend struct overviewQuery;

  toBackground Poll;

  std::list<QLabel *> Backgrounds;
  void setupChart(toResultLine *chart,const QString &,const QString &,const toSQL &sql);
  void setValue(QLabel *label,const QCString &val);
public:
  toTuningOverview(QWidget *parent=0,const char *name=0,WFlags fl=0);
  ~toTuningOverview();
  void stop(void);
  void start(void);

public slots:
  void refresh(void);
  void poll(void);
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
  toResultLong *LibraryCache;
  toResultLong *ControlFiles;
  toWaitEvents *Waits;

  std::map<QCString,QGrid *> Charts;

  QComboBox *Refresh;
  toTuningOverview *Overview;
  toTuningFileIO *FileIO;
  QWidget *LastTab;

  virtual void enableTab(const QCString &name,bool enable);
  virtual QWidget *tabWidget(const QCString &name);

  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix);
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix);
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
