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

#include <qpainter.h>

#include "tolinechart.h"
#include "tomain.h"
#include "toconf.h"

void toLineChart::setSamples(int samples)
{
  if (samples<=0) {
    QWidget *d=QApplication::desktop();
    Samples=d->width();
    AutoSamples=true;
  } else {
    Samples=samples;
    AutoSamples=false;
  }
  for(list<list<double> >::iterator i=Values.begin();i!=Values.end();i++)
    while (int((*i).size())>Samples)
      (*i).erase((*i).begin());
  update();
}

toLineChart::toLineChart(QWidget *parent,const char *name,WFlags f)
  : QWidget(parent,name,f)
{
  MinValue=MaxValue=0;
  Legend=true;
  Throbber=false;
  Grid=0;
  AxisText=false;
  
  setSamples();

  setMinimumSize(100,100);

  // Use list font
  QString str=toTool::globalConfig(CONF_LIST,"");
  if (!str.isEmpty()) {
    QFont font(toStringToFont(str));
    setFont(font);
  }
}

void toLineChart::addValues(list<double> &value)
{
  for(list<list<double> >::iterator i=Values.begin();i!=Values.end();i++)
    if (int((*i).size())==Samples)
      (*i).erase((*i).begin());
  list<double>::iterator j=value.begin();
  for(list<list<double> >::iterator i=Values.begin();i!=Values.end()&&j!=value.end();i++) {
    (*i).insert((*i).end(),*j);
    j++;
  }
  while(j!=value.end()) {
    list<double> t;
    t.insert(t.end(),*j);
    Values.insert(Values.end(),t);
    j++;
  }
  update();
}

#define FONT_ALIGN AlignLeft|AlignTop|ExpandTabs

void toLineChart::paintEvent(QPaintEvent *e)
{
  QPainter p(this);
  QFontMetrics fm=p.fontMetrics();

  int right=width();
  int bottom=height();

  p.fillRect(0,0,width(),height(),qApp->palette().active().background());
  if (Legend) {
    int lwidth=0;
    int lheight=0;
    for(list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
      if (!(*i).isEmpty()) {
	QRect bounds=fm.boundingRect(0,0,10000,10000,FONT_ALIGN,*i);
	if (lwidth<bounds.width())
	  lwidth=bounds.width();
	lheight+=bounds.height();
      }
    }
    if (lheight>0) {
      lheight+=4;
      lwidth+=14;
    }
    int lx=width()-lwidth-2;
    int ly=2;
    if (lx<50)
      lx=50;
    right=lx;
    p.drawRect(lx,ly,lwidth,lheight);
    lx+=12;
    ly+=2;
    int cp=0;
    for(list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
      if (!(*i).isEmpty()) {
	QRect bounds=fm.boundingRect(lx,ly,100000,100000,FONT_ALIGN,*i);
	p.drawText(bounds,FONT_ALIGN,*i);
	p.save();
	p.setPen(toChartColor(cp++));
	p.drawLine(lx-10,ly+bounds.height()/2,lx-2,ly+bounds.height()/2);
	p.restore();
	ly+=bounds.height();
      }
    }
  }
  double minval=0,maxval=0;
  if (MinAuto||MaxAuto) {
    bool first=true;
    for(list<list<double> >::iterator i=Values.begin();i!=Values.end();i++) {
      for(list<double>::iterator j=(*i).begin();j!=(*i).end();j++) {
	if (first) {
	  minval=*j;
	  maxval=*j;
	  first=false;
	} else if (maxval<*j)
	  maxval=*j;
	else if (minval>*j)
	  minval=*j;
      }
    }
  }
  if(!MinAuto)
    minval=MinValue;
  if(!MaxAuto)
    maxval=MaxValue;

  if (AxisText) {
    QString minstr=QString::number(minval);
    QString maxstr=QString::number(maxval);
    minstr+=YPostfix;
    maxstr+=YPostfix;
    QRect bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,minstr);
    int yoffset=bounds.height();
    bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,maxstr);
    if (yoffset<bounds.height())
      yoffset=bounds.height();
    
    bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,MinAxis);
    int xoffset=bounds.height();
    bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,MaxAxis);
    if (xoffset<bounds.height())
      xoffset=bounds.height();
    p.save();
    p.rotate(-90);
#if 0
    p.drawText(xoffset-bottom+2,0,bottom-4-xoffset,yoffset,
	       AlignLeft|AlignBottom|ExpandTabs,minstr);
#else
    // Qt bug, seems to clip left edge of 0 among others.
    p.drawText(xoffset-bottom+2,fm.ascent(),minstr);
#endif
    p.drawText(xoffset-bottom+2,0,bottom-4-xoffset,yoffset,
	       AlignRight|AlignBottom|ExpandTabs,maxstr);
    p.restore();
    p.drawText(yoffset+2,bottom-xoffset-2,right-4-yoffset,xoffset,
	       AlignLeft|AlignTop|ExpandTabs,MinAxis);
    p.drawText(yoffset+2,bottom-xoffset-2,right-4-yoffset,xoffset,
	       AlignRight|AlignTop|ExpandTabs,MaxAxis);
    p.translate(yoffset,0);
    right-=yoffset;
    bottom-=xoffset;
  }

  p.drawRect(2,2,right-4,bottom-4);
  if (Grid>1) {
    for (int i=1;i<Grid;i++) {
      int ypos=(bottom-4)*i/Grid+2;
      int xpos=(right-4)*i/Grid+2;
      p.drawLine(2,ypos,right-4,ypos);
      p.drawLine(xpos,2,xpos,bottom-4);
    }
  }

  int cp=0;
  for(list<list<double> >::iterator i=Values.begin();i!=Values.end();i++) {
    p.save();
    p.setPen(toChartColor(cp++));
    list<double> &val=*i;
    int count=0;
    bool first=true;
    int lval=0;
    int lx=right-2;
    for(list<double>::reverse_iterator j=val.rbegin();j!=val.rend()&&lx>=2;j++) {
      int val=int(bottom-2-(*j/(maxval-minval)*(bottom-4)));
      if (!first) {
	int x=lx;
	if (AutoSamples)
	  x--;
	else
	  x=right-4-count*(right-4)/Samples;
	p.drawLine(x,val,lx,lval);
	lx=x;
      } else
	first=false;
      lval=val;
      count++;
    }
    p.restore();
  }
}

