/* This file is part of KGraphViewer.
   Copyright (C) 2005-2007 Gael de Chalendar <kleag@free.fr>

   KGraphViewer is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA
*/

/* This file was callgraphview.cpp, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


#include "canvasedge.h"
#include "graphedge.h"
#include "graphnode.h"
#include "dotdefaults.h"
#include "dot2qtconsts.h"
#include "dotgraphview.h"
#include "FontsCache.h"

///#include <KAction>

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>

#include <iostream>

//
// CanvasEdge
//

///namespace KGraphViewer
///{
CanvasEdge::CanvasEdge(DotGraphView* view, GraphEdge* e,
                       qreal scaleX, qreal scaleY,
                       qreal xMargin, qreal yMargin, qreal gh,
                       qreal wdhcf, qreal hdvcf,
                       QGraphicsItem* parent)
                       : QAbstractGraphicsShapeItem(parent),
    m_scaleX(scaleX), m_scaleY(scaleY),
    m_xMargin(xMargin), m_yMargin(yMargin),
    m_gh(/*gh*/0), m_wdhcf(wdhcf), m_hdvcf(hdvcf), m_edge(e),
    m_font(0), m_view(view), m_popup(new QMenu())
{
  ///kDebug() << "edge "  << edge()->fromNode()->id() << "->"  << edge()->toNode()->id() << m_gh;
  setBoundingRegionGranularity(0.9);
  m_font = FontsCache::changeable().fromName(e->fontName());

  computeBoundingRect();
//   kDebug() << "boundingRect computed: " << m_boundingRect;
  
  QString tipStr = QString("%1 -> %2")
    .arg(edge()->fromNode()->id())
    .arg(edge()->toNode()->id());
  setToolTip(tipStr);

  // the message should be given (or possible to be given) by the part user
  /// KAction* removeEdgeAction = new KAction("Remove selected edge(s)"), this);
  /// m_popup->addAction(removeEdgeAction);
  /// connect(removeEdgeAction,SIGNAL(triggered(bool)),this,SLOT(slotRemoveEdge()));
  
  
  connect(e,SIGNAL(changed()),this,SLOT(modelChanged()));
  connect(this, SIGNAL(selected(CanvasEdge*, Qt::KeyboardModifiers)), view, SLOT(slotEdgeSelected(CanvasEdge*, Qt::KeyboardModifiers)));
  
  connect(this, SIGNAL(edgeContextMenuEvent(const QString&, const QPoint&)), view, SLOT(slotContextMenuEvent(const QString&, const QPoint&)));

  ///kDebug() << "connect slotElementHoverEnter";

  connect(this, SIGNAL(hoverEnter(CanvasEdge*)), view, SLOT(slotElementHoverEnter(CanvasEdge*)));
  connect(this, SIGNAL(hoverLeave(CanvasEdge*)), view, SLOT(slotElementHoverLeave(CanvasEdge*)));
  
} 

CanvasEdge::~CanvasEdge()
{
  delete m_popup;
}


QRectF CanvasEdge::boundingRect() const
{
  return m_boundingRect;
}

QPainterPath CanvasEdge::shape () const
{
//   kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id();
  if (m_boundingRegion.isEmpty()) {
    m_boundingRegion.addRegion(boundingRegion(QTransform()));
  }
  return m_boundingRegion;
  /*
  foreach (const DotRenderOp& dro, edge()->renderOperations())
  {
    if ( dro.renderop == "B" )
    {
      for (int splineNum = 0; splineNum < edge()->colors().count() || (splineNum==0 && edge()->colors().count()==0); splineNum++)
      {
        QPolygonF points(dro.integers[0]);
        for (int i = 0; i < dro.integers[0]; i++)
        {
          qreal nom = (dro.integers[2*dro.integers[0]]-dro.integers[2]);
          qreal denom = (dro.integers[2*dro.integers[0]-1]-dro.integers[1]);
          qreal diffX, diffY;
          if (nom == 0)
          {
            diffX = 0;
            diffY = 2*(edge()->colors().count()/2 - splineNum);
          }
          else if (denom ==0)
          {
            diffX = 2*(edge()->colors().count()/2 - splineNum);
            diffY = 0;
          }
          else
          {
            double pente = nom/denom;
            if (pente < 0)
            {
              diffX = 2*(edge()->colors().count()/2 - splineNum);
              diffY = edge()->colors().count()/2 + splineNum;
            }
            else
            {
              diffX = 2*(edge()->colors().count()/2 - splineNum);
              diffY = 2*(edge()->colors().count()/2 - splineNum);
            }
          }
          QPointF p(
              //NOTE: when uncommenting, fix nested comments in here:
              (dro.integers[2*i+1]/ *%m_wdhcf* /*m_scaleX) +m_xMargin + diffX,
              (m_gh-dro.integers[2*i+2]/ *%m_hdvcf* /)*m_scaleY + m_yMargin + diffY
                  );
          points[i] = p;
        }
        path.moveTo(points[0]);
        for (int j = 0; j < (points.size()-1)/3; j++)
        {
          path.cubicTo(points[3*j + 1],points[3*j+1 + 1], points[3*j+2 + 1]);
        }
        for (int j = (points.size()-1)/3-3; j >= 0 ; j--)
        {
          path.cubicTo(points[3*j + 1],points[3*j+1 + 1], points[3*j+2 + 1]);
        }
      }
    }
  }
  return path;
  */
}



void CanvasEdge::paint(QPainter* p, const QStyleOptionGraphicsItem* option,
                   QWidget* widget)
{
//   kDebug();
Q_UNUSED(option)
Q_UNUSED(widget)
  if (m_boundingRect == QRectF())
  {
    return;
  }
  /// computes the scaling of line width
  qreal widthScaleFactor = (m_scaleX+m_scaleY)/2;
  if (widthScaleFactor < 1)
  {
    widthScaleFactor = 1;
  }

  if (edge()->style()=="invis")
  {
    return;
  }
  if (edge()->renderOperations().isEmpty())
  {
    if ((edge()->fromNode()->canvasElement()!=0)
      && (edge()->toNode()->canvasElement()!=0))
    {
      p->drawLine(
        edge()->fromNode()->canvasElement()->boundingRect().center()+edge()->fromNode()->canvasElement()->pos(),
        edge()->toNode()->canvasElement()->boundingRect().center()+edge()->toNode()->canvasElement()->pos());
    }
    return;
  }

  QColor lineColor = Dot2QtConsts::componentData().qtColor(edge()->color(0));
  QColor backColor;
  
  QList<QPointF> allPoints;
  foreach (const DotRenderOp& dro, edge()->renderOperations())
  {
    //     kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "renderop" << dro.renderop << "; selected:" << edge()->isSelected();
    if (dro.renderop == "c")
    {
      QColor c(dro.str.mid(0,7));
      bool ok;
      c.setAlpha(255-dro.str.mid(8).toInt(&ok,16));
      lineColor = c;
//       kDebug() << "c" << dro.str.mid(0,7) << lineColor;
    }
    else if (dro.renderop == "C")
    {
      QColor c(dro.str.mid(0,7));
      bool ok;
      c.setAlpha(255-dro.str.mid(8).toInt(&ok,16));
/*      if (m_hovered && m_view->highlighting())
      {
        c = c.lighter();
      }*/
      backColor = c;
//       kDebug() << "C" << dro.str.mid(0,7) << backColor;
    }
    else if ( dro.renderop == "T" )
    {
      const QString& str = dro.str;
    
      qreal stringWidthGoal = dro.integers[3] * m_scaleX;
      int fontSize = edge()->fontSize();
      m_font->setPointSize(fontSize);
      QFontMetrics fm(*m_font);
      while (fm.width(str) > stringWidthGoal && fontSize > 1)
      {
        fontSize--;
        m_font->setPointSize(fontSize);
        fm = QFontMetrics(*m_font);
      }
      p->save();
      p->setFont(*m_font);
      
      p->setPen(Dot2QtConsts::componentData().qtColor(edge()->fontColor()));

      qreal x = (m_scaleX *
                       (
                         (dro.integers[0])
                         + (((-dro.integers[2])*(fm.width(dro.str)))/2)
                         - ( (fm.width(dro.str))/2 )
                       )
                      )
                      + m_xMargin;
      qreal y = ((m_gh - (dro.integers[1]))*m_scaleY)+ m_yMargin;
      QPointF point(x,y);
//       kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawText" << edge()->fontColor() << point;

      p->drawText(point,str);
      p->restore();
    }      
    else if (( dro.renderop == "p" ) || (dro.renderop == "P" ))
    {
      QPolygonF polygon(dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        QPointF point(
            (int(dro.integers[2*i+1])/*%m_wdhcf*/)*m_scaleX +m_xMargin,
            (int(m_gh-dro.integers[2*i+2])/*%m_hdvcf*/)*m_scaleY + m_yMargin
                );
        polygon[i] = point;
//         kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id()  << point;
        allPoints.append(point);
      }
      if (dro.renderop == "P" )
      {
        p->save();
        p->setBrush(lineColor);
        p->drawPolygon(polygon);
//         kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawPolygon" << edge()->color(0) << polygon;
        p->restore();
      }
      else
      {
        p->setBrush(Dot2QtConsts::componentData().qtColor("white"));
      }
      QPen pen(lineColor);
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth((int)(2 * widthScaleFactor));
      }
      else
      {
        pen.setWidth((int)(1 * widthScaleFactor));
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(edge()->style()));
      }
      p->save();
      p->setPen(pen);
//       kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawPolyline" << edge()->color(0) << polygon;
      p->drawPolyline(polygon);
      p->restore();
    }
    else if (( dro.renderop == "e" ) || (dro.renderop == "E" ))
    {
      qreal w = m_scaleX * dro.integers[2] * 2;
      qreal h = m_scaleY *  dro.integers[3] * 2;
      qreal x = (m_xMargin + (dro.integers[0]/*%m_wdhcf*/)*m_scaleX) - w/2;
      qreal y = ((m_gh -  dro.integers[1]/*%m_hdvcf*/)*m_scaleY + m_yMargin) - h/2;
      p->save();
      if (dro.renderop == "E" )
      {
        p->setBrush(lineColor);
      }
      else
      {
        p->setBrush(Dot2QtConsts::componentData().qtColor("white"));
      }
      QPen pen(lineColor);
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(int(2 * widthScaleFactor));
      }
      else
      {
        pen.setWidth(int(1 * widthScaleFactor));
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(edge()->style()));
      }
      p->setPen(pen);
      QRectF rect(x,y,w,h);
//       kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawEllipse" << edge()->color(0) << rect;
      p->drawEllipse(rect);
      p->restore();
    }
    else if ( dro.renderop == "B" )
    {
      uint lineWidth = 1;
      QPen pen;
      if (edge()->style() == "bold")
      {
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(int(2 * widthScaleFactor));
      }
      else if (edge()->style() != "filled")
      {
        pen.setStyle(Dot2QtConsts::componentData().qtPenStyle(edge()->style()));
      }
      if (edge()->style().left(12) == "setlinewidth")
      {
        bool ok;
        lineWidth = edge()->style().mid(12, edge()->style().length()-1-12).toInt(&ok);
        pen.setWidth(int(lineWidth * widthScaleFactor));
      }
      for (int splineNum = 0; splineNum < edge()->colors().count() || (splineNum==0 && edge()->colors().count()==0); splineNum++)
      {
        QPolygonF points(dro.integers[0]);
        for (int i = 0; i < dro.integers[0]; i++)
        {
          // computing of diffX and diffY to draw parallel edges
          // when asked through the corresponding GraphViz feature
          qreal nom = (dro.integers[2*dro.integers[0]]-dro.integers[2]);
          qreal denom = (dro.integers[2*dro.integers[0]-1]-dro.integers[1]);
          qreal diffX, diffY;
          if (nom == 0)
          {
            diffX = 0;
            diffY = 2*(edge()->colors().count()/2 - splineNum);
          }
          else if (denom ==0)
          {
            diffX = 2*(edge()->colors().count()/2 - splineNum);
            diffY = 0;
          }
          else
          {
            double pente = nom/denom;
            if (pente < 0)
            {
              diffX = 2*(edge()->colors().count()/2 - splineNum);
              diffY = edge()->colors().count()/2 + splineNum;
            }
            else
            {
              diffX = 2*(edge()->colors().count()/2 - splineNum);
              diffY = 2*(edge()->colors().count()/2 - splineNum);
            }
          }
          QPointF p(
              (dro.integers[2*i+1]/*%m_wdhcf*/*m_scaleX) +m_xMargin + diffX,
              (m_gh-dro.integers[2*i+2]/*%m_hdvcf*/)*m_scaleY + m_yMargin + diffY
                  );
          points[i] = p;
//           kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id()  << p;
          allPoints.append(p);
        }
        ///kDebug() << "Setting pen color to " << edge()->color(splineNum);
        if (splineNum != 0)
          lineColor = Dot2QtConsts::componentData().qtColor(edge()->color(splineNum));
        pen.setColor(splineNum);
        p->save();
//         p->setBrush(Dot2QtConsts::componentData().qtColor(edge()->color(0)));
        p->setBrush(Qt::NoBrush);
        p->setPen(pen);
        QPainterPath path;
        path.moveTo(points[0]);
        for (int j = 0; j < (points.size()-1)/3; j++)
        {
          path.cubicTo(points[3*j + 1],points[3*j+1 + 1], points[3*j+2 + 1]);
        }
//         kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "drawPath" << edge()->color(splineNum) << points.first() << points.last();
        p->drawPath(path);
        p->restore();
      }
    }
  }
  if (edge()->isSelected())
  {
//     kDebug() << "draw square";
//     p->drawRect(m_boundingRect);
    qreal maxDist = 0;
    QPair<QPointF,QPointF> pointsPair;
    foreach(const QPointF& point1, allPoints)
    {
      foreach(const QPointF& point2, allPoints)
      {
        if (distance(point1, point2) > maxDist)
        {
          maxDist = distance(point1, point2);
          pointsPair = qMakePair(point1, point2);
        }
      }
    }
    if (maxDist>0)
    {
      p->save();
      //         p->setBrush(Dot2QtConsts::componentData().qtColor(edge()->color(0)));
      p->setBrush(Qt::black);
      p->setPen(Qt::black);
      p->drawRect(QRectF(pointsPair.first-QPointF(3,3),QSizeF(6,6)));
      p->drawRect(QRectF(pointsPair.second-QPointF(3,3),QSizeF(6,6)));
      p->restore();
    }
  }
}

void CanvasEdge::modelChanged()
{
//   kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id();
  prepareGeometryChange();
  computeBoundingRect();
}

void CanvasEdge::computeBoundingRect()
{
//   kDebug();
  //invalidate bounding region cache
  m_boundingRegion = QPainterPath();
  if (edge()->renderOperations().isEmpty())
  {
    if ((edge()->fromNode()->canvasElement()==0)
      || (edge()->toNode()->canvasElement()==0)
      || edge()->style()=="invis")
    {
      m_boundingRect = QRectF();
    }
    else
    {
      QRectF br(
      edge()->fromNode()->canvasElement()->boundingRect().center()+edge()->fromNode()->canvasElement()->pos(),
                edge()->toNode()->canvasElement()->boundingRect().center()+edge()->toNode()->canvasElement()->pos());
//       kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() <<br;
      m_boundingRect = br;
    }
  }
  else
  {
    QPolygonF points;
    foreach (const DotRenderOp& dro, edge()->renderOperations())
    {
//       kDebug() << dro.renderop  << ", ";
      if ( (dro.renderop != "B") && (dro.renderop != "p") &&  (dro.renderop != "P") ) continue;
      uint previousSize = points.size();
      points.resize(previousSize+dro.integers[0]);
      for (int i = 0; i < dro.integers[0]; i++)
      {
        QPointF p(
            ((dro.integers[2*i+1]/*%m_wdhcf*/)*m_scaleX) +m_xMargin,
            ((m_gh-dro.integers[2*i+2]/*%m_hdvcf*/)*m_scaleY) + m_yMargin
                );
        points[previousSize+i] = p;
      }
    }
//     kDebug() << points.size() << "points";
    if (points.size() == 0) return;

    int len = points.count();
    QPolygonF a = points,  b = points;
    a.translate(-1, -1);
    b.translate(1, 1);
    a.resize(2*len);
    for (int i=0;i<len;i++)
    {
      a[len+i] = b[i];
    }
//     kDebug() << a.size() << "points";

    m_boundingRect = a.boundingRect();
  }
  ///kDebug() << edge()->fromNode()->id() << "->" << edge()->toNode()->id() << "New bounding rect is:" << m_boundingRect;
}

void CanvasEdge::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
  ///kDebug() << event;
  if (m_view->isReadOnly())
  {
    return;
  }
  if (event->button() == Qt::LeftButton)
  {
    edge()->setSelected(!edge()->isSelected());
    if (edge()->isSelected())
    {
      emit(selected(this,event->modifiers()));
    }
    update();
  }
  else if (event->button() == Qt::RightButton)
  {
    if (!edge()->isSelected())
    {
      edge()->setSelected(true);
      emit(selected(this,event->modifiers()));
      update();
    }
    ///kDebug() << "emiting edgeContextMenuEvent("<<m_edge->id()<<","<<event->screenPos()<<")";
    emit(edgeContextMenuEvent(m_edge->id(), event->screenPos() ));
// opens the selected edge contextual menu and if necessary select the edge
/*    kDebug() << "opens the contextual menu";
    m_popup->exec(event->screenPos());*/
  }
}

qreal CanvasEdge::distance(const QPointF& point1, const QPointF& point2)
{
  return sqrt(pow(point1.x()-point2.x(),2)+pow(point1.y()-point2.y(),2));
}

void CanvasEdge::slotRemoveEdge()
{
  ///kDebug();
  m_view->removeSelectedElements();
}

void CanvasEdge::hoverEnterEvent( QGraphicsSceneHoverEvent * event )
{
  Q_UNUSED(event)
  ///kDebug() << edge()->id();
  emit hoverEnter(this);
}

void CanvasEdge::hoverLeaveEvent( QGraphicsSceneHoverEvent * event )
{
  Q_UNUSED(event)
  ///kDebug() << edge()->id();
  emit hoverLeave(this);
}


//#include "canvasedge.moc"
