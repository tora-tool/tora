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

#include <qpainter.h>
#include <qworkspace.h>

#include "tolinechart.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"

#include "tolinechart.moc"

double toLineChart::round(double round,bool up)
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

void toLineChart::setSamples(int samples)
{
  Samples=samples;

  if (Samples>0) {
    while (int(XValues.size())>Samples)
      XValues.erase(XValues.begin());

    for(std::list<std::list<double> >::iterator i=Values.begin();i!=Values.end();i++)
      while (int((*i).size())>Samples)
	(*i).erase((*i).begin());
  }
  update();
}

toLineChart::toLineChart(QWidget *parent,const char *name,WFlags f)
  : QWidget(parent,name,f)
{
  MinAuto=MaxAuto=true;
  MinValue=MaxValue=0;
  Legend=true;
  Last=false;
  Grid=5;
  AxisText=true;
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

void toLineChart::addValues(std::list<double> &value,const QString &xValue)
{
  if (int(XValues.size())==Samples&&Samples>0)
    XValues.erase(XValues.begin());
  XValues.insert(XValues.end(),xValue);

  if (Samples>0)
    for(std::list<std::list<double> >::iterator i=Values.begin();i!=Values.end();i++)
      if (int((*i).size())==Samples)
	(*i).erase((*i).begin());

  std::list<double>::iterator j=value.begin();
  for(std::list<std::list<double> >::iterator i=Values.begin();i!=Values.end()&&j!=value.end();i++) {
    (*i).insert((*i).end(),*j);
    j++;
  }
  while(j!=value.end()) {
    std::list<double> t;
    t.insert(t.end(),*j);
    Values.insert(Values.end(),t);
    j++;
  }
  update();
}

QRect toLineChart::fixRect(QPoint p1,QPoint p2)
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

void toLineChart::paintTitle(QPainter *p,QRect &rect)
{
  if (!Title.isEmpty()) {
    p->save();
    QFont f=p->font();
    f.setBold(true);
    p->setFont(f);
    QRect bounds=fm.boundingRect(0,0,rect.width(),rect.width(),FONT_ALIGN,Title);
    p->drawText(0,2,rect.width(),bounds.height(),AlignHCenter|AlignTop|ExpandTabs,Title);
    p->restore();
    p->translate(0,bounds.height()+2);
    rect.setTop(rect.top()+bounds.height()+2);
  }
  if (Last) {
    QString str;
    for(std::list<std::list<double> >::iterator i=Values.begin();i!=Values.end();i++) {
      if ((*i).begin()!=(*i).end()) {
	if (!str.isEmpty())
	  str+="\n";
	str+=QString::number(*(*i).rbegin());
	str+=YPostfix;
      }
    }
    if (!str.isEmpty()) {
      QRect bounds=fm.boundingRect(0,0,rect.width(),rect.height(),FONT_ALIGN,str);
      p->drawText(0,2,rect.width(),bounds.height(),AlignHCenter|AlignTop|ExpandTabs,str);
      p->translate(0,bounds.height());
      rect.setTop(rect.top()+bounds.height()+2);
    }
  }
}

void toLineChart::paintChart(QPainter *p,QRect &rect)
{
  paintTitle(p,rect);
}

void toLineChart::paintEvent(QPaintEvent *e)
{
  QPainter p(this);
  QRect rect(0,0,width(),height());
  paintChart(&p,rect);

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
    for(std::list<std::list<double> >::iterator i=Values.begin();i!=Values.end();i++) {
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
    {
      for(std::list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
	if (!(*i).isEmpty()&&*i!=" ") {
	  QRect bounds=fm.boundingRect(0,0,10000,10000,FONT_ALIGN,*i);
	  if (lwidth<bounds.width())
	    lwidth=bounds.width();
	  lheight+=bounds.height();
	}
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
    for(std::list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
	QRect bounds=fm.boundingRect(lx,ly,100000,100000,FONT_ALIGN,*i);
      if (!(*i).isEmpty()&&*i!=" ") {
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
    if (MinAuto||MaxAuto) {
      bool first=true;
      for(std::list<std::list<double> >::iterator i=Values.begin();i!=Values.end();i++) {
	for(std::list<double>::iterator j=(*i).begin();j!=(*i).end();j++) {
	  if (first) {
	    zMinValue=*j;
	    zMaxValue=*j;
	    first=false;
	  } else if (zMaxValue<*j)
	    zMaxValue=*j;
	  else if (zMinValue>*j)
	    zMinValue=*j;
	}
      }
      if (zMaxValue==0&&zMinValue==0)
	leftAxis=false;
      zMaxValue=round(zMaxValue,true);
      zMinValue=round(zMinValue,false);
    }
    if(!MinAuto)
      zMinValue=MinValue;
    if(!MaxAuto)
      zMaxValue=MaxValue;
  }

  if (AxisText) {
    int yoffset=0;
    QString minstr;
    QString maxstr;
    QRect ybounds;
    if (leftAxis) {
      minstr=QString::number(zMinValue);
      maxstr=QString::number(zMaxValue);
      minstr+=YPostfix;
      maxstr+=YPostfix;
      QRect bounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,minstr);
      yoffset=bounds.height();
      ybounds=fm.boundingRect(0,0,100000,100000,FONT_ALIGN,maxstr);
      if (yoffset<ybounds.height())
	yoffset=ybounds.height();
    }
    
    QString maxXstr;
    QString minXstr;
    int xoffset=0;
    if (XValues.size()>1) {
      minXstr=*(XValues.begin());
      maxXstr=*(XValues.rbegin());

      if (Zooming) {
	int count=0;
	for(std::list<QString>::reverse_iterator i=XValues.rbegin();i!=XValues.rend();i++) {
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
	p.drawText(xoffset-bottom+2,0,bottom-4-xoffset,yoffset,
		   AlignRight|AlignBottom|ExpandTabs,maxstr);
#else
	// Qt bug, seems to clip left edge of 0 among others.
	p.drawText(xoffset-bottom+2,fm.ascent()+1,minstr);
	p.drawText(-2-ybounds.width(),fm.ascent()+1,maxstr);
#endif
	p.restore();
      } else
	yoffset=0;
      p.drawText(yoffset+2,bottom-xoffset-2,right-4-yoffset,xoffset,
		 AlignLeft|AlignTop|ExpandTabs,minXstr);
      p.drawText(yoffset+2,bottom-xoffset-2,right-4-yoffset,xoffset,
		 AlignRight|AlignTop|ExpandTabs,maxXstr);
      p.translate(left=yoffset,0);
      right-=yoffset;
      bottom-=xoffset;
    }
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

  int cp=0;
  int samples=countSamples();
  if (samples>1) {
    const QWMatrix &mtx=p.worldMatrix();
    p.setClipRect(int(mtx.dx()+2),int(mtx.dy()+2),right-3,bottom-3);
    if (Zooming)
      p.drawText(2,2,right-4,bottom-4,
		 AlignLeft|AlignTop,"Zoom");
    for(std::list<std::list<double> >::iterator i=Values.begin();i!=Values.end();i++) {
      p.save();
      p.setPen(toChartColor(cp++));
      std::list<double> &val=*i;
      int count=0;
      bool first=true;
      int lval=0;
      int lx=right-2;
      int skip=SkipSamples;
      for(std::list<double>::reverse_iterator j=val.rbegin();j!=val.rend()&&lx>=2;j++) {
	if (skip>0)
	  skip--;
	else {
	  int val=int(bottom-2-((*j-zMinValue)/(zMaxValue-zMinValue)*(bottom-4)));
	  if (!first) {
	    int x=lx;
	    x=right-2-(count+1)*(right-4)/samples;
	    p.drawLine(x,val,lx,lval);
	    lx=x;
	  } else
	    first=false;
	  lval=val;
	  count++;
	  if (count>=samples)
	    break;
	}
      }
      p.restore();
    }
  }
  MousePoint[1]=QPoint(-1,-1);
}

void toLineChart::mouseReleaseEvent(QMouseEvent *e)
{
  if (e->button()==LeftButton&&MousePoint[0]!=QPoint(-1,-1)) {
    if (MousePoint[1]!=QPoint(-1,-1)) {
      QRect rect=fixRect(MousePoint[0],MousePoint[1]);
      QPainter p(this);
      p.setRasterOp(NotROP);
      p.drawRect(rect);
    }
    if (MousePoint[0].x()!=e->x()&&
	MousePoint[0].y()!=e->y()) {
      QRect rect=fixRect(MousePoint[0],e->pos());
      int samples=countSamples();
      UseSamples=samples*rect.width()/Chart.width()+1;
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

int toLineChart::countSamples(void)
{
  int samples=Samples;
  if (Samples<=0)
    for(std::list<std::list<double> >::reverse_iterator i=Values.rbegin();i!=Values.rend();i++)
      samples=max(samples,int((*i).size()));
  if (UseSamples>1&&UseSamples<samples)
    samples=UseSamples;
  return samples;
}

void toLineChart::mousePressEvent(QMouseEvent *e)
{
  if (e->button()==LeftButton)
    MousePoint[0]=e->pos();
}

void toLineChart::clearZoom(void)
{
  Zooming=false;
  SkipSamples=0;
  UseSamples=-1;
  zMinValue=-1;
  zMaxValue=-1;
}

void toLineChart::mouseMoveEvent(QMouseEvent *e)
{
  if (MousePoint[0]!=QPoint(-1,-1)) {
    QPainter p(this);
    p.setRasterOp(NotROP);
    if (MousePoint[1]!=QPoint(-1,-1))
      p.drawRect(fixRect(MousePoint[0],MousePoint[1]));
    MousePoint[1]=e->pos();
    p.drawRect(fixRect(MousePoint[0],MousePoint[1]));
  }
}


void toLineChart::mouseDoubleClickEvent(QMouseEvent *e)
{
  if (e->button()==LeftButton) {
    QWidget *newWin=new toLineChart(this,toMainWidget()->workspace());
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

toLineChart::toLineChart (toLineChart *chart,QWidget *parent,const char *name,WFlags f)
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
