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
#include <qapplication.h>
#include <math.h>

#include "topiechart.h"
#include "totool.h"
#include "toconf.h"
#include "tomain.h"

#include "topiechart.moc"

toPieChart::toPieChart(QWidget *parent,const char *name,WFlags f)
  : QWidget(parent,name,f)
{
  Legend=true;
  DisplayPercent=false;

  setMinimumSize(60,60);

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

  double tot=0;
  {
    for(std::list<double>::iterator i=Values.begin();i!=Values.end();i++)
      tot+=*i;
  }

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

  if (Legend) {
    int lwidth=0;
    int lheight=0;

    std::list<double>::iterator j=Values.begin();
    {
      for(std::list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
	QString sizstr;
	if (j!=Values.end()) {
	  if (DisplayPercent)
	    sizstr.sprintf("%0.1f",100*(*j)/tot);
	  else
	    sizstr=QString::number(*j);
	  sizstr+=Postfix;
	}
        if (!(*i).isEmpty()) {
  	  QString str=*i;
	  str+=" (";
	  str+=sizstr;
	  str+=")";
	  QRect bounds=fm.boundingRect(0,0,10000,10000,FONT_ALIGN,str);
	  if (lwidth<bounds.width())
	    lwidth=bounds.width();
	  lheight+=bounds.height();
	}
      }
      if (j!=Values.end())
	j++;
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
    j=Values.begin();
    for(std::list<QString>::iterator i=Labels.begin();i!=Labels.end();i++) {
      QString sizstr;
      if (j!=Values.end()) {
	if (DisplayPercent)
	  sizstr.sprintf("%0.1f",100*(*j)/tot);
	else
	  sizstr=QString::number(*j);
	sizstr+=Postfix;
      }

      if (!(*i).isEmpty()) {
	QString str=*i;
	str+=" (";
	str+=sizstr;
	str+=")";

	QRect bounds=fm.boundingRect(lx,ly,100000,100000,FONT_ALIGN,str);
	p.drawText(bounds,FONT_ALIGN,str);
	p.save();
	p.setBrush(toChartColor(cp++));
	p.drawRect(lx-10,ly+bounds.height()/2-fm.ascent()/2,8,fm.ascent());
	p.restore();
	ly+=bounds.height();
      }

      if (j!=Values.end())
	j++;
    }
  }
  int cp=0;
  int pos=0;
  unsigned int count=0;
  for(std::list<double>::iterator i=Values.begin();i!=Values.end();i++) {
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
}
