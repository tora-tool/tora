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

#include "utils.h"

#include "tobarchart.h"
#include "tolinechart.h"
#include "topiechart.h"
#include "toresultview.h"
#include "tovisualize.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include "tovisualize.moc"

#include "icons/axis.xpm"
#include "icons/compile.xpm"
#include "icons/grid.xpm"
#include "icons/legend.xpm"

toVisualize::toVisualize(toListView *source,QWidget *parent,const char *name)
  : QVBox(parent,name),Source(source)
{
  QToolBar *toolbar=toAllocBar(this,"Data visualization","");
  
  Type=new QComboBox(toolbar);
  Type->insertItem("Bar chart",0);
  Type->insertItem("Line chart",1);
  Type->insertItem("Pie chart",2);

  toolbar->addSeparator();

  Legend=new QToolButton(toolbar);
  Legend->setToggleButton(true);
  Legend->setIconSet(QIconSet(QPixmap((const char **)legend_xpm)));
  QToolTip::add(Legend,"Display legend");
  Legend->setOn(true);

  Grid=new QToolButton(toolbar);
  Grid->setToggleButton(true);
  Grid->setIconSet(QIconSet(QPixmap((const char **)grid_xpm)));
  QToolTip::add(Grid,"Display grid");
  Grid->setOn(true);

  Axis=new QToolButton(toolbar);
  Axis->setToggleButton(true);
  Axis->setIconSet(QIconSet(QPixmap((const char **)axis_xpm)));
  QToolTip::add(Axis,"Display axis legend");
  Axis->setOn(true);

  QToolTip::add(new QLabel(" Title columns ",toolbar),
		"Number of columns to use as title");
  Title=new QComboBox(toolbar);
  Title->insertItem("1");
  Title->insertItem("2");
  Title->insertItem("3");
  Title->insertItem("4");
  Title->insertItem("5");
  Title->insertItem("6");

  toolbar->addSeparator();

  new QToolButton(QPixmap((const char **)compile_xpm),
		  "Update chart",
		  "Update chart",
		  this,SLOT(display(void)),
		  toolbar);

  toolbar->setStretchableWidget(new QLabel(toolbar));

  Result=new QWidget(this);
  Result->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
}

void toVisualize::display(void)
{
  QWidget *last=Result;
  switch(Type->currentItem()) {
  case 0:
    {
      toBarChart *chart=new toBarChart(this);
      chart->showLegend(Legend->isOn());
      int add=0;
      int tit=max(1,Title->currentText().toInt());
      if(Source->columnText(0)=="#")
	add=1;
      chart->setTitle(Source->columnText(add));
      chart->showGrid(Grid->isOn()?5:0);
      chart->showAxisLegend(Axis->isOn());
      chart->setSamples();

      std::list<QString> lst;
      for (int i=tit+add;i<Source->columns();i++)
	toPush(lst,Source->columnText(i));
      chart->setLabels(lst);

      for (QListViewItem *item=Source->firstChild();item;item=item->nextSibling()) {
	QString label=item->text(add);
	for (int j=add+1;j<tit+add;j++) {
	  label+=", ";
	  label+=item->text(j);
	}
	std::list<double> val;
	for (int i=tit+add;i<Source->columns();i++) {
	  toPush(val,item->text(i).toDouble());
	}
	chart->addValues(val,label);
      }
      Result=chart;
    }
    break;
  case 1:
    {
      toLineChart *chart=new toLineChart(this);
      chart->showLegend(Legend->isOn());
      int add=0;
      int tit=max(1,Title->currentText().toInt());
      if(Source->columnText(0)=="#")
	add=1;
      chart->setTitle(Source->columnText(add));
      chart->showGrid(Grid->isOn()?5:0);
      chart->showAxisLegend(Axis->isOn());
      chart->setSamples();

      std::list<QString> lst;
      for (int i=tit+add;i<Source->columns();i++)
	toPush(lst,Source->columnText(i));
      chart->setLabels(lst);

      for (QListViewItem *item=Source->firstChild();item;item=item->nextSibling()) {
	QString label=item->text(add);
	for (int j=add+1;j<tit+add;j++) {
	  label+=", ";
	  label+=item->text(j);
	}
	std::list<double> val;
	for (int i=tit+add;i<Source->columns();i++) {
	  toPush(val,item->text(i).toDouble());
	}
	chart->addValues(val,label);
      }
      Result=chart;
    }
    break;
  case 2:
    {
      toPieChart *chart=new toPieChart(this);
      chart->showLegend(Legend->isOn());
      int add=0;
      int tit=max(1,Title->currentText().toInt());
      if(Source->columnText(0)=="#")
	add=1;
      chart->setTitle(Source->columnText(add));
      for (QListViewItem *item=Source->firstChild();item;item=item->nextSibling()) {
	QString label=item->text(add);
	for (int j=add+1;j<tit+add;j++) {
	  label+=", ";
	  label+=item->text(j);
	}
	chart->addValue(item->text(tit+add).toDouble(),label);
      }
      Result=chart;
    }
    break;
  default:
    Result=new QWidget(this);
  }
  Result->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  Result->show();
  delete last;
}
