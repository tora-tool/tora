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

/* This file was callgraphview.h, part of KCachegrind.
   Copyright (C) 2003 Josef Weidendorfer <Josef.Weidendorfer@gmx.de>

   KCachegrind is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation, version 2.
*/


/*
 * Callgraph View
 */

#ifndef PANNER_VIEW_H
#define PANNER_VIEW_H

#include <QGraphicsView>
#include <QWidget>
//Added by qt3to4:
#include <QMouseEvent>

///namespace KGraphViewer
///{
class DotGraphView;

/**
 * A panner layed over a QCanvas
 */
class PannerView: public QGraphicsView
{
  Q_OBJECT

public:
  explicit PannerView(DotGraphView * parent, const char * name = 0);

  inline void setDrawingEnabled(bool val) {m_drawContents = val;}

public slots:
  void setZoomRect(QRectF r);
  void moveZoomRectTo(const QPointF& newPos, const bool notify = true);

signals:
  void zoomRectMovedTo(QPointF newPos);
  void zoomRectMoveFinished();

protected:
  virtual void mousePressEvent(QMouseEvent*);
  virtual void mouseMoveEvent(QMouseEvent*);
  virtual void mouseReleaseEvent(QMouseEvent*);
  virtual void drawForeground(QPainter * p, const QRectF & rect );
  virtual void contextMenuEvent(QContextMenuEvent* event);

  QRectF m_zoomRect;
  bool m_movingZoomRect;
  QPointF m_lastPos;
  bool m_drawContents;
  DotGraphView* m_parent;
};

///}
#endif



