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


/*
 * Callgraph View
 */

#include "pannerview.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QGraphicsScene>
#include <QPainter>
#include <QMouseEvent>

///#include <kdebug.h>
///#include <klocale.h>

#include "dotgraphview.h"

///namespace KGraphViewer
///{
//
// PannerView
//
PannerView::PannerView(DotGraphView * parent, const char * name)
  : QGraphicsView(parent), m_drawContents(true), m_parent(parent)
{
  m_movingZoomRect = false;

  // why doesn't this avoid flicker ?
  // viewport()->setBackgroundMode(Qt::NoBackground);
  ///setBackgroundMode(Qt::NoBackground);
  setAttribute(Qt::WA_NoSystemBackground);

  // if there are ever graphic glitches to be found, remove this again
  setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontClipPainter |
                        QGraphicsView::DontSavePainterState);

  setToolTip("View of the complete graph. Click and drag to move the visible part.");
  setWhatsThis("<h1>View of the Complete Graph</h1>"
    "<p>Single clicking somewhere without the red square will move the center of the "
    "view to where the mouse was clicked.</p><p>Clicking and dragging within the red square "
    "will cause the view to follow the movement.</p>");
}

void PannerView::setZoomRect(QRectF r)
{
//   kDebug() << "PannerView::setZoomRect " << r;
  if (r == m_zoomRect) {
    return;
  }
  scene()->invalidate(m_zoomRect, QGraphicsScene::ForegroundLayer);
  // get new zoom rect
  m_zoomRect = r;
  qreal q = mapToScene(15,0).x();
  qreal d = mapToScene(10,0).x();
  if (!m_zoomRect.isValid() || m_zoomRect.width() < q || m_zoomRect.height() < q) 
  {
    double factor = ((double)m_zoomRect.width())/m_zoomRect.height();
    qreal newWidth, newHeight;
    if (factor > 1.0)
    {
      newWidth = q;
      newHeight = newWidth/factor;
      if (newHeight < d) newHeight = d;
    }
    else
    {
      newHeight = q;
      newWidth = newHeight*factor;
      if (newWidth < d) newWidth = d;
    }
    QRectF newRect = m_zoomRect;
    qreal newXPos = newRect.x() + (newRect.width()/2) - newWidth/2;
    newXPos = (newXPos<0)?0:newXPos;
    newRect.setX(newXPos);
    qreal newYPos = newRect.y() + (newRect.height()/2) -newHeight/2;
    newYPos = (newYPos<0)?0:newYPos;
    newRect.setY(newYPos);
    newRect.setWidth(newWidth);
    newRect.setHeight(newHeight);
    m_zoomRect = newRect;
  }
  scene()->invalidate(m_zoomRect, QGraphicsScene::ForegroundLayer);
}

void PannerView::moveZoomRectTo(const QPointF& newPos, bool notify)
{
  if (!m_zoomRect.isValid()) {
    return;
  }

  if (m_zoomRect.center() == newPos) {
    ///kDebug() << "same pos, don't do anything";
    return;
  }

  scene()->invalidate(m_zoomRect, QGraphicsScene::ForegroundLayer);
  m_zoomRect.moveCenter(newPos);
  scene()->invalidate(m_zoomRect, QGraphicsScene::ForegroundLayer);

  if (m_zoomRect.isValid() && notify) {
    emit zoomRectMovedTo(newPos);
    m_lastPos = newPos;
  }
}

void PannerView::drawForeground(QPainter * p, const QRectF & rect )
{
  if (m_zoomRect.isValid() && rect.intersects(m_zoomRect))
  {
    p->save();
    if (m_zoomRect.width() > 10 && m_zoomRect.height() > 10)
    {
      p->setPen(Qt::red);
      // substract pen width, i.e. draw inside
      qreal penWidth = p->pen().widthF();
      p->drawRect(m_zoomRect.adjusted(-penWidth, -penWidth, -penWidth, -penWidth));
    }
    else
    {
      QBrush brush(Qt::red);
      p->fillRect(m_zoomRect, brush);
    }
    p->restore();
  }
}

void PannerView::mousePressEvent(QMouseEvent* e)
{
  if (e->button() != Qt::LeftButton) {
    return;
  }
/*  kDebug() << "PannerView::mousePressEvent " 
              << mapToScene(e->pos()) << " / " << m_zoomRect << " / " << m_zoomRect.center() <<endl;*/
  moveZoomRectTo(mapToScene(e->pos()));
  m_movingZoomRect = true;
}

void PannerView::mouseMoveEvent(QMouseEvent* e)
{
  if (!m_movingZoomRect) {
    return;
  }

//   kDebug() << "PannerView::mouseMoveEvent " << pos;
  moveZoomRectTo(mapToScene(e->pos()));
}

void PannerView::mouseReleaseEvent(QMouseEvent* e)
{
  if (e->button() != Qt::LeftButton) {
    return;
  }
  moveZoomRectTo(mapToScene(e->pos()));
//   kDebug() << "PannerView::mouseReleaseEvent " << pos;
  m_movingZoomRect = false;
  emit zoomRectMoveFinished();
}

void PannerView::contextMenuEvent(QContextMenuEvent* event)
{
  m_parent->contextMenuEvent(event);
}

///}

///#include "pannerview.moc"
