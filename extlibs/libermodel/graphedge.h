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
 * Graph Edge
 */

#ifndef GRAPH_EDGE_H
#define GRAPH_EDGE_H

#include "canvasnode.h"
#include "graphelement.h"
#include "dotgrammar.h"
#include "dotrenderop.h"

#include <graphviz/gvc.h>

#include <QStringList>
#include <QMap>
#include <QTextStream>

///namespace KGraphViewer
///{
  
class CanvasEdge;
class GraphNode;

class GraphEdge : public GraphElement
{
//   Q_OBJECT
public:
  GraphEdge();
  virtual ~GraphEdge();

  GraphEdge(const GraphEdge& edge);
  GraphEdge(edge_t* edge);
  
  CanvasEdge* canvasEdge() { return (CanvasEdge*)canvasElement(); }
  void setCanvasEdge(CanvasEdge* ce) { setCanvasElement((CanvasElement*)ce); }

  bool isVisible() { return m_visible; }
  void setVisible(bool v) { m_visible = v; }

  GraphElement* fromNode() { return m_fromNode; }
  GraphElement* toNode() { return m_toNode; }
  const GraphElement* fromNode() const { return m_fromNode; }
  const GraphElement* toNode() const { return m_toNode; }

  void setFromNode(GraphElement* n) { m_fromNode = n; }
  void setToNode(GraphElement* n) { m_toNode = n; }

//   inline const QVector< QPair< float, float > >& edgePoints() const {return m_edgePoints;}
//   inline QVector< QPair< float, float > >& edgePoints() {return m_edgePoints;}
//   inline void edgePoints(const QVector< QPair< float, float > >& ep) {m_edgePoints = ep;}
  
  inline const QStringList& colors() const {return m_colors;}
  const QString color(uint i);
  void colors(const QString& cs); 
  
/*  inline void labelX(float x) {m_labelX = x;}
  inline void labelY(float y) {m_labelY = y;}
  inline float labelX() const {return m_labelX;}
  inline float labelY() const {return m_labelY;}*/
  
  inline const QString& dir() const {return m_dir;}
  inline void dir(const QString& dir) {m_dir = dir;}

  inline QList< DotRenderOp >&  arrowheads() {return m_arrowheads;}
  inline const QList< DotRenderOp >&  arrowheads() const {return m_arrowheads;}

  virtual void updateWithEdge(const GraphEdge& edge);
  virtual void updateWithEdge(edge_t* edge);

private:
  // we have a _ce *and* _from/_to because for collapsed edges,
  // only _to or _from will be unequal NULL
  GraphElement *m_fromNode, *m_toNode;
  bool m_visible;
  QStringList m_colors;
  QString m_dir;
//   QVector< QPair< float, float > > m_edgePoints;
//   float m_labelX, m_labelY;
  
  QList< DotRenderOp > m_arrowheads;
};


/** A map associating the bounds nodes of a graph's edges to these edges */
typedef QMap<QString, GraphEdge*> GraphEdgeMap;

QTextStream& operator<<(QTextStream& s, const GraphEdge& e);

///}

#endif



