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

/*
 * Canvas Subgraph (subgraph node view)
 */

#ifndef CANVAS_ELEMENT_H
#define CANVAS_ELEMENT_H

#include <QAbstractGraphicsShapeItem>
#include <QPen>
#include <QBrush>

#include "dotgrammar.h"

///namespace KGraphViewer
///{
class GraphElement;
class DotGraphView;
class QGraphicsScene;
class QMenu;

class CanvasElement: public QObject, public QAbstractGraphicsShapeItem
{
Q_OBJECT
public:
  CanvasElement(
      DotGraphView* v, 
      GraphElement* s,
      QGraphicsScene* c,
      QGraphicsItem* parent = 0);
      
  virtual ~CanvasElement();
  
  GraphElement* element() { return m_element; }

  virtual void paint(QPainter* p, const QStyleOptionGraphicsItem *option,
        QWidget *widget = 0 );

  virtual QRectF boundingRect () const;

  void computeBoundingRect();
  
  void initialize(qreal scaleX, qreal scaleY,
                  qreal xMargin, qreal yMargin, qreal gh,
                  qreal wdhcf, qreal hdvcf);

  inline void setGh(qreal gh) {m_gh = gh;}
  
  protected:
  virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
  virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
  virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
  virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
  virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent * event );

  qreal m_scaleX, m_scaleY;
  qreal m_xMargin, m_yMargin, m_gh, m_wdhcf, m_hdvcf;
  GraphElement* m_element;
  DotGraphView* m_view;
  QFont* m_font;
  QPen m_pen;
  QBrush m_brush;
  QRectF m_boundingRect;
  QMenu* m_popup;

  bool m_hovered;
  
Q_SIGNALS:
  void selected(CanvasElement*, Qt::KeyboardModifiers);
  void elementContextMenuEvent(const QString&, const QPoint&);
  void hoverEnter(CanvasElement*);
  void hoverLeave(CanvasElement*);
  
public Q_SLOTS:
  void modelChanged();
  void slotRemoveElement();
};

///}

  #endif // CANVAS_ELEMENT_H



