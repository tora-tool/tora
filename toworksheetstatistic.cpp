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

#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include "tomain.h"
#include "toworksheetstatistic.h"
#include "toanalyze.h"
#include "toresultview.h"
#include "tobarchart.h"

#include "toworksheetstatistic.moc"

#include "icons/filesave.xpm"
#include "icons/fileopen.xpm"
#include "icons/trash.xpm"

#include "icons/grid.xpm"
#include "icons/tree.xpm"

toAnalyze *toWorksheetStatistic::Widget;

toWorksheetStatistic::toWorksheetStatistic(QWidget *parent)
  : QVBox(parent)
{
  QToolBar *toolbar=toAllocBar(this,"Server Tuning","No connection");

  new QToolButton(QPixmap((const char **)fileopen_xpm),
		  "Load statistics from file",
		  "Load statistics from file",
		  this,SLOT(load(void)),
		  toolbar);

  QToolButton *button=new QToolButton(QPixmap((const char **)filesave_xpm),
				      "Save statistics to file",
				      "Save statistics to file",
				      this,SLOT(displayMenu(void)),
				      toolbar);
  SaveMenu=new QPopupMenu(button);
  button->setPopup(SaveMenu);
  button->setPopupDelay(0);
  connect(SaveMenu,SIGNAL(aboutToShow()),this,SLOT(displayMenu()));
  connect(SaveMenu,SIGNAL(activated(int)),this,SLOT(save(int)));

  toolbar->addSeparator();

  button=new QToolButton(QPixmap((const char **)trash_xpm),
			 "Remove statistics",
			 "Remove statistics",
			 this,SLOT(displayMenu(void)),
			 toolbar);
  RemoveMenu=new QPopupMenu(button);
  button->setPopup(RemoveMenu);
  button->setPopupDelay(0);
  connect(RemoveMenu,SIGNAL(aboutToShow()),this,SLOT(displayMenu()));
  connect(RemoveMenu,SIGNAL(activated(int)),this,SLOT(remove(int)));

  Splitter=new QSplitter(Vertical,this);

  toolbar->addSeparator();

  ShowCharts=new QToolButton(toolbar);
  ShowCharts->setToggleButton(true);
  ShowCharts->setIconSet(QIconSet(QPixmap((const char **)grid_xpm)));
  QToolTip::add(ShowCharts,"Display charts");
  ShowCharts->setOn(true);
  connect(ShowCharts,SIGNAL(toggled(bool)),this,SLOT(showCharts(bool)));

  ShowPlans=new QToolButton(toolbar);
  ShowPlans->setToggleButton(true);
  ShowPlans->setIconSet(QIconSet(QPixmap((const char **)tree_xpm)));
  QToolTip::add(ShowPlans,"Display execution plans");
  ShowPlans->setOn(true);
  connect(ShowPlans,SIGNAL(toggled(bool)),this,SLOT(showPlans(bool)));

  Dummy=new QWidget(Splitter);

  Tool=dynamic_cast<toAnalyze *>(toCurrentTool(this));
  if (!Widget)
    Widget=Tool;
}

toWorksheetStatistic::~toWorksheetStatistic()
{
  if (Widget==Tool)
    Widget=NULL;
}

void toWorksheetStatistic::saveStatistics(std::map<QString,QString> &stats)
{
  if (!Widget)
    toAnalyze::createTool();
  if (Widget)
    Widget->worksheet()->addStatistics(stats);
}

void toWorksheetStatistic::addStatistics(std::map<QString,QString> &stats)
{
  data cur;
  cur.Top=new QVBox(Splitter);
  QHBox *box=new QHBox(cur.Top);
  cur.Label=new QLabel(stats["Description"],box);
  cur.Label->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred));
  QCheckBox *check=new QCheckBox("Hide",box);
  cur.Charts=new toHideSplitter(Horizontal,cur.Top);
  connect(check,SIGNAL(toggled(bool)),cur.Charts,SLOT(setHidden(bool)));
  cur.Statistics=new toListView(cur.Charts);
  cur.Statistics->importData(stats,"Stat");
  cur.Wait=new toBarChart(cur.Charts);
  cur.Wait->importData(stats,"Wait");
  cur.IO=new toBarChart(cur.Charts);
  cur.IO->importData(stats,"IO");
  cur.Plan=new toListView(cur.Charts);
  cur.Plan->importData(stats,"Plan");

  QValueList<int> sizes;
  sizes<<1<<1<<1<<1;
  cur.Charts->setSizes(sizes);
  if (!cur.Plan->firstChild()) {
    delete cur.Plan;
    cur.Plan=NULL;
  } else if (!ShowPlans->isOn())
    cur.Plan->hide();
  if (!ShowCharts->isOn()) {
    cur.Statistics->hide();
    cur.Wait->hide();
    cur.IO->hide();
  }
  cur.Top->show();
  setFocus();

  Open.insert(Open.end(),cur);

  if (Dummy) {
    delete Dummy;
    Dummy=NULL;
  }
}

void toWorksheetStatistic::showPlans(bool show)
{
  for(std::list<data>::iterator i=Open.begin();i!=Open.end();i++) {
    if ((*i).Plan) {
      if (show) {
	(*i).Plan->show();
      } else {
	(*i).Plan->hide();
      }
    }
  }
}

void toWorksheetStatistic::showCharts(bool show)
{
  for(std::list<data>::iterator i=Open.begin();i!=Open.end();i++) {
    if (show) {
      (*i).Statistics->show();
      (*i).Wait->show();
      (*i).IO->show();
    } else {
      (*i).Statistics->hide();
      (*i).Wait->hide();
      (*i).IO->hide();
    }
  }
}

void toWorksheetStatistic::save(int selid)
{
  int id=1;
  for(std::list<data>::iterator i=Open.begin();i!=Open.end();i++) {
    if (selid==id) {
      QString fn=toSaveFilename(QString::null,"*.stat",this);
      if (!fn.isEmpty()) {
	std::map<QString,QString> stat;
	(*i).Statistics->exportData(stat,"Stat");
	(*i).IO->exportData(stat,"IO");
	(*i).Wait->exportData(stat,"Wait");
	if ((*i).Plan)
	  (*i).Plan->exportData(stat,"Plan");
	stat["Description"]=(*i).Label->text();
	try {
	  toTool::saveMap(fn,stat);
	} TOCATCH
      }
      break;
    }
    id++;
  }
}

void toWorksheetStatistic::remove(int selid)
{
  int id=1;
  for(std::list<data>::iterator i=Open.begin();i!=Open.end();i++) {
    if (selid==id) {
      delete (*i).Top;
      if (Open.size()==1) {
	Dummy=new QWidget(Splitter);
	Dummy->show();
      }
      Open.erase(i);
      break;
    }
    id++;
  }
}

void toWorksheetStatistic::load(void)
{
  QString filename=toOpenFilename(QString::null,"*.stat",this);
  if (!filename.isEmpty()) {
    try {
      std::map<QString,QString> ret;
      if (toTool::loadMap(filename,ret))
	addStatistics(ret);
    } TOCATCH
  }
}

void toWorksheetStatistic::displayMenu(void)
{
  SaveMenu->clear();
  RemoveMenu->clear();
  int id=1;
  for(std::list<data>::iterator i=Open.begin();i!=Open.end();i++) {
    SaveMenu->insertItem((*i).Label->text(),id);
    RemoveMenu->insertItem((*i).Label->text(),id);
    id++;
  }
}
