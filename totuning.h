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
