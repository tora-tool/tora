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
#include <qworkspace.h>

#include "tobarchart.h"
#include "tomain.h"
#include "toconf.h"

#include "tobarchart.moc"

double toBarChart::round(double round,bool up)
{
  double base=1.0E-5;
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
  Samples=samples;

  if (Samples>0) {
    while (int(XValues.size())>Samples)
      XValues.erase(XValues.begin());

    for(list<list<double> >::iterator i=Values.begin();i!=Values.end();i++)
      while (int((*i).size())>Samples)
	(*i).erase((*i).begin());
  }
  update();
}

toBarChart::toBarChart(QWidget *parent,const char *name,WFlags f)
  : QWidget(parent,name,f)
{
  MaxAuto=true;
  MaxValue=0;
  MinAuto=false;
  MinValue=0;
  Legend=true;
  Grid=5;
  AxisText=true;
  Last=false;
  MousePoint[0]=MousePoint[1]=QPoint(-1,-1);

  clearZoom();

  setSamples(toTool::globalConfig(CONF_CHART_SAMPLES,DEFAULT_CHART_SAMPLES).toInt());

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
  if (int(XValues.size())==Samples&&Samples>0)
    XValues.erase(XValues.begin());
  XValues.insert(XValues.end(),xValue);

  if (Samples>0)
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

QRect toBarChart::fixRect(QPoint p1,QPoint p2)
{
  if (p1.x()<Chart.x())
    p1.setX(Chart.x());
  if (p2.x()<Chart.x())
    p2.setX(Chart.x());
  if (p1.x()>Chart.x()+Chart.width())
    p1.setX(Chart.x()+Chart.width());
  if (p2.x()>Chart.x()+Chart.width())
    p2.setX(Chart.x()+Chart.width());

  if (p1.y()<Chart.y())
    p1.setY(Chart.y());
  if (p2.y()<Chart.y())
    p2.setY(Chart.y());
  if (p1.y()>Chart.y()+Chart.height())
    p1.setY(Chart.y()+Chart.height());
  if (p2.y()>Chart.y()+Chart.height())
    p2.setY(Chart.y()+Chart.height());

  return QRect(min(p1.x(),p2.x()),
	       min(p1.y(),p2.y()),
	       abs(p1.x()-p2.x()),
	       abs(p1.y()-p2.y()));
}

#define FONT_ALIGN AlignLeft|AlignTop|ExpandTabs

void toBarChart::paintEvent(QPaintEvent *e)
{
  QPainter p(this);
  QFontMetrics fm=p.fontMetrics();

  int right=width();
  int bottom=height();
  int top=0;
  int left=0;

  p.fillRect(0,0,width(),height(),qApp->palette().active().background());

  if (!Title.isEmpty()) {
    p.save();
    QFont f=p.font();
    f.setBold(true);
    p.setFont(f);
    QRect bounds=fm.boundingRect(0,0,width(),height(),FONT_ALIGN,Title);
    p.drawText(0,2,width(),bounds.height(),AlignHCenter|AlignTop|ExpandTabs,Title);
    p.restore();
    p.translate(0,top=bounds.height()+2);
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
      top+=bounds.height();
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
  bool leftAxis=true;
  if (!Zooming) {
    if (MinAuto) {
      bool first=true;
      list<list<double> >::reverse_iterator i=Values.rbegin();
      if (i!=Values.rend()) {
	for(list<double>::iterator j=(*i).begin();j!=(*i).end();j++) {
	if (first) {
	  first=false;
	  zMinValue=*j;
	} else if (zMinValue>*j)
	  zMinValue=*j;
	}
      }
    }
    if (MaxAuto) {
      bool first=true;
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
      for(list<double>::iterator i=total.begin();i!=total.end();i++) {
	if (first) {
	  first=false;
	  zMaxValue=*i;
	} else if (zMaxValue<*i)
	  zMaxValue=*i;
      }
    }
    if (zMaxValue==0&&zMinValue==0)
      leftAxis=false;
    if(!MinAuto)
      zMinValue=MinValue;
    else
      zMinValue=round(zMinValue,false);
    if(!MaxAuto)
      zMaxValue=MaxValue;
    else
      zMaxValue=round(zMaxValue,true);
  }

  if (AxisText) {
    int yoffset=0;
    QString minstr;
    QString maxstr;
    if (leftAxis) {
      minstr=QString::number(zMinValue);
      maxstr=QString::number(zMaxValue);
      minstr+=YPostfix;
      maxstr+=YPostfix;
      QRect bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,minstr);
      yoffset=bounds.height();
      bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,maxstr);
      if (yoffset<bounds.height())
	yoffset=bounds.height();
    }
    
    QString maxXstr;
    QString minXstr;
    int xoffset=0;
    if (XValues.size()>1) {
      minXstr=*(XValues.begin());
      maxXstr=*(XValues.rbegin());

      if (Zooming) {
	int count=0;
	for(list<QString>::reverse_iterator i=XValues.rbegin();i!=XValues.rend();i++) {
	  if (count==SkipSamples)
	    maxXstr=*i;
	  else if (count==SkipSamples+UseSamples-1) {
	    minXstr=*i;
	    break;
	  }
	  count++;
	}
      }

      QRect bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,minXstr);
      xoffset=bounds.height();
      bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,maxXstr);
      if (xoffset<bounds.height())
	xoffset=bounds.height();

      if (zMinValue!=0||zMaxValue!=0) {
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
      }
      p.drawText(yoffset+2,bottom-xoffset-2,right-4-yoffset,xoffset,
		 AlignLeft|AlignTop|ExpandTabs,minXstr);
      p.drawText(yoffset+2,bottom-xoffset-2,right-4-yoffset,xoffset,
		 AlignRight|AlignTop|ExpandTabs,maxXstr);
      p.translate(left=yoffset,0);
    }
    right-=yoffset;
    bottom-=xoffset;
  }

  p.save();
  p.setBrush(white);
  Chart=QRect(left+2,top+2,right-3,bottom-3);
  p.drawRect(2,2,right-3,bottom-3);
  p.restore();
  if (Grid>1) {
    p.save();
    p.setPen(gray);
    for (int i=1;i<Grid;i++) {
      int ypos=(bottom-4)*i/Grid+2;
      int xpos=(right-4)*i/Grid+2;
      p.drawLine(3,ypos,right-3,ypos);
      p.drawLine(xpos,3,xpos,bottom-3);
    }
    p.restore();
  }

  list<QPointArray> Points;
  int cp=0;
  int samples=countSamples();
  int zeroy=int(bottom-2-(-zMinValue/(zMaxValue-zMinValue)*(bottom-4)));
  if (samples>1) {
    const QWMatrix &mtx=p.worldMatrix();
    p.setClipRect(mtx.dx()+2,mtx.dy()+2,right-3,bottom-3);
    if (Zooming)
      p.drawText(2,2,right-4,bottom-4,
		 AlignLeft|AlignTop,"Zoom");
    for(list<list<double> >::reverse_iterator i=Values.rbegin();i!=Values.rend();i++) {
      list<double> &val=*i;
      int count=0;
      int skip=SkipSamples;
      QPointArray a(samples+10);
      int x=right-2;
      for(list<double>::reverse_iterator j=val.rbegin();j!=val.rend()&&x>=2;j++) {
	if (skip>0)
	  skip--;
	else {
	  int val=int(bottom-2-((*j-zMinValue)/(zMaxValue-zMinValue)*(bottom-4)));
	  x=right-2-count*(right-4)/(samples-1);
	  a.setPoint(count,x,val);
	  count++;
	  if (count>=samples)
	    break;
	}
      }
      a.resize(count*2);
      Points.insert(Points.end(),a);
      cp++;
    }
  }

  map<int,int> Bottom;
  for(list<QPointArray>::iterator i=Points.begin();i!=Points.end();i++) {
    QPointArray a=*i;
    int lx=0;
    int lb=0;
    for(unsigned int j=0;j<a.size()/2;j++) {
      int x,y;
      a.point(j,&x,&y);
      if(Bottom.find(x)==Bottom.end())
	Bottom[x]=0;
      if (lx!=x)
	lb=Bottom[x];
      a.setPoint(a.size()-1-j,x,zeroy-lb);
      y-=lb;
      a.setPoint(j,x,y);
      Bottom[x]=zeroy-y;
      lx=x;
    }

    p.save();
    p.setBrush(toChartColor(--cp));
    p.drawPolygon(a);
    p.restore();
  }
  MousePoint[1]=QPoint(-1,-1);
}

void toBarChart::mouseReleaseEvent(QMouseEvent *e)
{
  if (e->button()==LeftButton&&MousePoint[0].x()>=0) {
    if (MousePoint[1].x()>=0) {
      QRect rect=fixRect(MousePoint[0],MousePoint[1]);
      QPainter p(this);
      p.setRasterOp(NotROP);
      p.drawRect(rect);
    }
    if (MousePoint[0].x()!=e->x()&&
	MousePoint[0].y()!=e->y()) {
      QRect rect=fixRect(MousePoint[0],e->pos());
      int samples=countSamples();
      UseSamples=samples*rect.width()/Chart.width()+2;
      if (UseSamples<2)
	UseSamples=2;
      SkipSamples+=samples*(Chart.width()+Chart.x()-rect.x()-rect.width())/Chart.width();
      Zooming=true;
      
      double t=(zMaxValue-zMinValue)*(Chart.y()+Chart.height()-rect.y()-rect.height())/Chart.height()+zMinValue;
      zMaxValue=(zMaxValue-zMinValue)*(Chart.y()+Chart.height()-rect.y())/Chart.height()+zMinValue;
      zMinValue=t;
      update();
    }
    MousePoint[1]=MousePoint[0]=QPoint(-1,-1);
  } else if (e->button()==RightButton) {
    clearZoom();
    update();
  }
}

int toBarChart::countSamples(void)
{
  int samples=Samples;
  if (Samples<=0)
    for(list<list<double> >::reverse_iterator i=Values.rbegin();i!=Values.rend();i++)
      samples=max(samples,int((*i).size()));
  if (UseSamples>1&&UseSamples<samples)
    samples=UseSamples;
  return samples;
}

void toBarChart::mousePressEvent(QMouseEvent *e)
{
  if (e->button()==LeftButton)
    MousePoint[0]=e->pos();
}

void toBarChart::clearZoom(void)
{
  Zooming=false;
  SkipSamples=0;
  UseSamples=-1;
  zMinValue=-1;
  zMaxValue=-1;
}

void toBarChart::mouseMoveEvent(QMouseEvent *e)
{
  if (MousePoint[0].x()>=0) {
    QPainter p(this);
    p.setRasterOp(NotROP);
    if (MousePoint[1].x()>=0)
      p.drawRect(fixRect(MousePoint[0],MousePoint[1]));
    MousePoint[1]=e->pos();
    p.drawRect(fixRect(MousePoint[0],MousePoint[1]));
  }
}

void toBarChart::mouseDoubleClickEvent(QMouseEvent *e)
{
  if (e->button()==LeftButton) {
    QWidget *newWin=new toBarChart(this,toMainWidget()->workspace());
    newWin->show();
    toMainWidget()->windowsMenu();

#if 1
      // This is a really ugly workaround for a Qt layout bug
      QWidget *tmp=NULL;
      QWidget *tmp2=NULL;
      for (unsigned int i=0;i<toMainWidget()->workspace()->windowList().count();i++) {
        QWidget *widget=toMainWidget()->workspace()->windowList().at(i);
        if (newWin!=widget)
	  tmp2=widget;
	else
	    tmp=newWin;
	if (tmp2&&tmp)
	  break;
      }
      if(tmp2&&tmp) {
        tmp2->setFocus();
        tmp->setFocus();
      }
#endif
  }
}

toBarChart::toBarChart (toBarChart *chart,QWidget *parent,const char *name,WFlags f)
  : QWidget(parent,name,f)
{
  Values=chart->Values;
  XValues=chart->XValues;
  Labels=chart->Labels;
  Legend=chart->Legend;
  Last=false;
  Grid=5;
  AxisText=true;
  MinValue=chart->MinValue;
  MinAuto=chart->MinAuto;
  MaxValue=chart->MaxValue;
  MaxAuto=chart->MaxAuto;
  Samples=chart->Samples;
  Title=chart->Title;
  YPostfix=chart->YPostfix;

  setCaption(Title);

  clearZoom();

  setMinimumSize(80,50);

  // Use list font
  QString str=toTool::globalConfig(CONF_LIST,"");
  if (!str.isEmpty()) {
    QFont font(toStringToFont(str));
    setFont(font);
  }
}
