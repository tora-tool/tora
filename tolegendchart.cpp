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

#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#include "tolegendchart.h"
#include "tomain.h"

#include "tolegendchart.moc"

toLegendChart::toLegendChart(QWidget *parent,const char *name,WFlags f)
  : QWidget(parent,name,f)
{
  setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred));
}

void toLegendChart::addLabel(const QString &label)
{
  Labels.insert(Labels.end());
  sizeHint();
  update();
}

void toLegendChart::setLabels(std::list<QString> &labels)
{
  Labels=labels;
  sizeHint();
  update();
}

#define FONT_ALIGN AlignLeft|AlignTop|ExpandTabs

QSize toLegendChart::sizeHint(void)
{
  QFontMetrics fm=fontMetrics();
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
  setFixedWidth(lwidth);
  return QSize(lwidth,lheight);
}

void toLegendChart::paintEvent(QPaintEvent *e)
{
  QSize size=sizeHint();
  int lwidth=size.width();
  int lheight=size.height();

  QPainter p(this);
  QFontMetrics fm=p.fontMetrics();

  int lx=2;
  int ly=2;
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
