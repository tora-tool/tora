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

#include "tomain.h"
#include "toworksheetstatistic.h"
#include "toanalyze.h"
#include "toresultview.h"
#include "tobarchart.h"

#include "toworksheetstatistic.moc"

#include "icons/filesave.xpm"
#include "icons/fileopen.xpm"
#include "icons/trash.xpm"

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
  cur.Label=new QLabel(stats["Description"],cur.Top);
  QSplitter *splitter=new QSplitter(Horizontal,cur.Top);
  cur.Statistics=new toListView(splitter);
  cur.Statistics->importData(stats,"Stat");
  cur.Wait=new toBarChart(splitter);
  cur.Wait->importData(stats,"Wait");
  cur.IO=new toBarChart(splitter);
  cur.IO->importData(stats,"IO");
  cur.Top->show();

  Open.insert(Open.end(),cur);

  if (Dummy) {
    delete Dummy;
    Dummy=NULL;
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
