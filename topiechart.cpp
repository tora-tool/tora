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
#include <qapplication.h>
#include <math.h>

#include "topiechart.h"
#include "totool.h"
#include "toconf.h"
#include "tomain.h"

toPieChart::toPieChart(QWidget *parent,const char *name,WFlags f=0)
  : QWidget(parent,name,f)
{
  Legend=true;
  setMinimumSize(100,100);

  // Use list font
  QString str=toTool::globalConfig(CONF_LIST,"");
  if (!str.isEmpty()) {
    QFont font(toStringToFont(str));
    setFont(font);
  }
}

#define FONT_ALIGN AlignLeft|AlignTop|ExpandTabs

void toPieChart::paintEvent(QPaintEvent *e)
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
  double tot=0;
  for(list<double>::iterator i=Values.begin();i!=Values.end();i++)
    tot+=*i;

  int cp=0;
  int pos=0;
  unsigned int count=0;
  for(list<double>::iterator i=Values.begin();i!=Values.end();i++) {
    count++;
    int size=int(*i*5760/tot);
    if (size<=0)
      size=1;
    if (count==Values.size())
      size=5760-pos;
    p.save();
    p.setBrush(toChartColor(cp++));
    p.drawPie(2,2,right-4,bottom-4,pos,size);
    p.restore();
    pos+=size;
  }

  pos=0;
  count=0;
  for(list<double>::iterator i=Values.begin();i!=Values.end();i++) {
    count++;
    int size=int(*i*5760/tot);
    if (size<=0)
      size=1;
    if (count==Values.size())
      size=5760-pos;

    double posy=sin((2*pos+size)*M_PI/5760);
    double posx=cos((2*pos+size)*M_PI/5760);
    posy*=-bottom/3;
    posy+=bottom/2;
    posx*=right/3;
    posx+=right/2;
    QString sizstr=QString::number(*i);
    sizstr+=Postfix;
    QRect bounds=fm.boundingRect(sizstr);
    p.drawText(bounds.x()-bounds.width()/2+posx,
	       bounds.y()-bounds.height()/2+posy+fm.ascent(),
	       sizstr);
    pos+=size;
  }
}
