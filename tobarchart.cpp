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

#include "tobarchart.h"
#include "tomain.h"
#include "toconf.h"

#include "tobarchart.moc"

double toBarChart::round(double round,bool up)
{
  double base=1;
  double mult=1;
  if (round<0) {
    mult=-1;
    round=-round;
    up=!up;
  }
  for(;;base*=10) {
    if (up) {
      if (base>=round)
	return mult*base;
      else if (base*2.5>=round)
	return mult*base*2.5;
      else if (base*5>=round)
	return mult*base*5;
    } else if (base>round) {
      if (base/2<=round)
	return mult*base/2;
      else if (base/4<=round)
	return mult*base/4;
      else if (base/10<=round)
	return mult*base/10;
      else
	return 0;
    }
  }
}

void toBarChart::setSamples(int samples)
{
  if (samples<=0) {
    QWidget *d=QApplication::desktop();
    Samples=d->width();
    AutoSamples=true;
  } else {
    Samples=samples;
    AutoSamples=false;
  }

  while (int(XValues.size())>Samples)
    XValues.erase(XValues.begin());

  for(list<list<double> >::iterator i=Values.begin();i!=Values.end();i++)
    while (int((*i).size())>Samples)
      (*i).erase((*i).begin());
  update();
}

toBarChart::toBarChart(QWidget *parent,const char *name,WFlags f)
  : QWidget(parent,name,f)
{
  MaxAuto=true;
  MaxValue=0;
  Legend=true;
  Grid=5;
  AxisText=true;
  Last=false;

  setSamples();

  setMinimumSize(80,50);

  // Use list font
  QString str=toTool::globalConfig(CONF_LIST,"");
  if (!str.isEmpty()) {
    QFont font(toStringToFont(str));
    setFont(font);
  }
}

void toBarChart::addValues(list<double> &value,const QString &xValue)
{
  if (int(XValues.size())==Samples)
    XValues.erase(XValues.begin());
  XValues.insert(XValues.end(),xValue);

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

void toBarChart::paintEvent(QPaintEvent *e)
{
  QPainter p(this);
  QFontMetrics fm=p.fontMetrics();

  int right=width();
  int bottom=height();

  p.fillRect(0,0,width(),height(),qApp->palette().active().background());

  if (!Title.isEmpty()) {
    p.save();
    QFont f=p.font();
    f.setBold(true);
    p.setFont(f);
    QRect bounds=fm.boundingRect(0,0,width(),height(),FONT_ALIGN,Title);
    p.drawText(0,2,width(),bounds.height(),AlignHCenter|AlignTop|ExpandTabs,Title);
    p.restore();
    p.translate(0,bounds.height()+2);
    bottom-=bounds.height()+2;
  }
  if (Last) {
    QString str;
    for(list<list<double> >::iterator i=Values.begin();i!=Values.end();i++) {
      if ((*i).begin()!=(*i).end()) {
	if (!str.isEmpty())
	  str+="\n";
	str+=QString::number(*(*i).rbegin());
	str+=YPostfix;
      }
    }
    if (!str.isEmpty()) {
      QRect bounds=fm.boundingRect(0,0,width(),height(),FONT_ALIGN,str);
      p.drawText(0,2,width(),bounds.height(),AlignHCenter|AlignTop|ExpandTabs,str);
      p.translate(0,bounds.height());
      bottom-=bounds.height();
    }
  }

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
    p.save();
    p.setBrush(white);
    p.drawRect(lx,ly,lwidth,lheight);
    p.restore();
    lx+=12;
    ly+=2;
    int cp=0;
    for(list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
      if (!(*i).isEmpty()) {
	QRect bounds=fm.boundingRect(lx,ly,100000,100000,FONT_ALIGN,*i);
	p.drawText(bounds,FONT_ALIGN,*i);
	p.save();
	p.setBrush(toChartColor(cp++));
	p.drawRect(lx-10,ly+bounds.height()/2-fm.ascent()/2,8,fm.ascent());
	p.restore();
	ly+=bounds.height();
      }
    }
  }
  double minval=0,maxval=0;
  if (MaxAuto) {
    bool firstMax=true;
    list<double> total;
    for(list<list<double> >::iterator i=Values.begin();i!=Values.end();i++) {
      list<double>::iterator k=total.begin();
      for(list<double>::iterator j=(*i).begin();j!=(*i).end();j++) {
	if (k==total.end()) {
	  total.insert(total.end(),*j);
	  k=total.end();
	} else {
	  *k+=*j;
	  k++;
	}
      }
    }
    for(list<double>::iterator k=total.begin();k!=total.end();k++) {
      if (firstMax) {
	maxval=*k;
	firstMax=false;
      } else if (maxval<*k)
	maxval=*k;
    }
    maxval=round(maxval,true);
  }
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
    
    QString maxXstr;
    QString minXstr;
    if (XValues.begin()!=XValues.end()) {
      minXstr=*(XValues.begin());
      maxXstr=*(XValues.rbegin());
    }
    bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,minXstr);
    int xoffset=bounds.height();
    bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,maxXstr);
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
	       AlignLeft|AlignTop|ExpandTabs,minXstr);
    p.drawText(yoffset+2,bottom-xoffset-2,right-4-yoffset,xoffset,
	       AlignRight|AlignTop|ExpandTabs,maxXstr);
    p.translate(yoffset,0);
    right-=yoffset;
    bottom-=xoffset;
  }

  p.save();
  p.setBrush(white);
  p.drawRect(2,2,right-4,bottom-4);
  p.restore();
  if (Grid>1) {
    p.save();
    p.setPen(gray);
    for (int i=1;i<Grid;i++) {
      int ypos=(bottom-4)*i/Grid+2;
      int xpos=(right-4)*i/Grid+2;
      p.drawLine(3,ypos,right-4,ypos);
      p.drawLine(xpos,3,xpos,bottom-4);
    }
    p.restore();
  }

  const QWMatrix &mtx=p.worldMatrix();
  p.setClipRect(mtx.dx()+3,mtx.dy()+3,right-6,bottom-6);
  list<QPointArray> Points;
  int cp=0;
  {
    for(list<list<double> >::reverse_iterator i=Values.rbegin();i!=Values.rend();i++) {
      list<double> &val=*i;
      int count=0;
      QPointArray a(Samples+10);
      int x=right-2;
      for(list<double>::reverse_iterator j=val.rbegin();j!=val.rend()&&x>=2;j++) {
	int val=int(bottom-2-((*j-minval)/(maxval-minval)*(bottom-4)));
	if (AutoSamples)
	  x--;
	else
	  x=right-4-count*(right-4)/Samples;
	a.setPoint(count,x,val);
	count++;
      }
      a.resize(count*2);
      Points.insert(Points.end(),a);
      cp++;
    }
  }

  map<int,int> Bottom;
  for(list<QPointArray>::iterator i=Points.begin();i!=Points.end();i++) {
    QPointArray a=*i;
    for(unsigned int j=0;j<a.size()/2;j++) {
      int x,y;
      a.point(j,&x,&y);
      if(Bottom.find(x)==Bottom.end())
	Bottom[x]=0;
      a.setPoint(a.size()-1-j,x,bottom-2-Bottom[x]);
      y-=Bottom[x];
      a.setPoint(j,x,y);
      Bottom[x]=bottom-2-y;
    }

    p.save();
    p.setBrush(toChartColor(--cp));
    p.drawPolygon(a);
    p.restore();
  }
}

