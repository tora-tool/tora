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

#include "graphedge.h"
#include "graphnode.h"
#include "graphsubgraph.h"
#include "canvasedge.h"
#include "dotdefaults.h"

///namespace KGraphViewer
///{
  
/*
 * Graph Edge
 */

GraphEdge::GraphEdge() : 
    GraphElement(),
    m_fromNode(0),m_toNode(0),
    m_visible(true),
    m_colors(),
    m_dir(DOT_DEFAULT_EDGE_DIR),
    m_arrowheads()
{
//   kDebug() ;
}

GraphEdge::~GraphEdge()
{
//   kDebug() ;
}

GraphEdge::GraphEdge(const GraphEdge& edge) :
  GraphElement(edge)
{
    m_fromNode = 0;
    m_toNode = 0;
    m_visible = edge.m_visible;
    m_colors = edge.m_colors;
    m_dir = edge.m_dir;
    m_arrowheads = edge.m_arrowheads;
}

void GraphEdge::colors(const QString& cs)
{
  m_colors = cs.split(':');
//   kDebug() << fromNode()->id() << " -> " << toNode()->id() << ": nb colors: " << m_colors.size();
}

const QString GraphEdge::color(uint i) 
{
  if (i >= (uint)m_colors.count() && m_attributes.find("color") != m_attributes.end())
  {
    colors(m_attributes["color"]);
  }
  if (i < (uint)m_colors.count())
  {
//     std::cerr << "edge color " << i << " is " << m_colors[i] << std::endl;
//     kDebug() << fromNode()->id() << " -> " << toNode()->id() << "color" << i << "is" << m_colors[i];
    return m_colors[i];
  }
  else
  {
//     kDebug() << fromNode()->id() << " -> " << toNode()->id() << "no edge color " << i << ". returning " << DOT_DEFAULT_EDGE_COLOR;
    return DOT_DEFAULT_EDGE_COLOR;
  }
}

void GraphEdge::updateWithEdge(const GraphEdge& edge)
{
  ///kDebug() << id() << edge.id();
  m_arrowheads = edge.arrowheads();
  m_colors = edge.colors();
  m_dir = edge.dir();
  GraphElement::updateWithElement(edge);
  if (canvasEdge())
  {
    canvasEdge()->computeBoundingRect();
    canvasEdge()->modelChanged();
  }
}

void GraphEdge::updateWithEdge(edge_t* edge)
{
  ///kDebug();
  renderOperations().clear();
  if (agget(edge, (char*)"_draw_") != NULL)
  {
    parse_renderop(agget(edge, (char*)"_draw_"), renderOperations());
    ///kDebug() << "element renderOperations size is now " << renderOperations().size();
  }
  if (agget(edge, (char*)"_ldraw_") != NULL)
  {
    parse_renderop(agget(edge, (char*)"_ldraw_"), renderOperations());
    ///kDebug() << "element renderOperations size is now " << renderOperations().size();
  }
  if (agget(edge, (char*)"_hdraw_") != NULL)
  {
    parse_renderop(agget(edge, (char*)"_hdraw_"), renderOperations());
    ///kDebug() << "element renderOperations size is now " << renderOperations().size();
  }
  if (agget(edge, (char*)"_tdraw_") != NULL)
  {
    parse_renderop(agget(edge, (char*)"_tdraw_"), renderOperations());
    ///kDebug() << "element renderOperations size is now " << renderOperations().size();
  }
  if (agget(edge, (char*)"_hldraw_") != NULL)
  {
    parse_renderop(agget(edge, (char*)"_hldraw_"), renderOperations());
    ///kDebug() << "element renderOperations size is now " << renderOperations().size();
  }
  if (agget(edge, (char*)"_tldraw_") != NULL)
  {
    parse_renderop(agget(edge, (char*)"_tldraw_"), renderOperations());
    ///kDebug() << "element renderOperations size is now " << renderOperations().size();
  }
  Agsym_t *attr = agfstattr(edge);
  while(attr)
  {
    ///kDebug() /*<< edge->name*/ << ":" << attr->name << agxget(edge,attr->index);
    m_attributes[attr->name] = agxget(edge,attr->index);
    attr = agnxtattr(edge,attr);
  }
  
}

QTextStream& operator<<(QTextStream& s, const GraphEdge& e)
{
  QString srcLabel = e.fromNode()->id();
  if (dynamic_cast<const GraphSubgraph*>(e.fromNode()))
  {
    srcLabel = QString("subgraph ") + srcLabel;
  }
  QString tgtLabel = e.toNode()->id();
  if (dynamic_cast<const GraphSubgraph*>(e.toNode()))
  {
    tgtLabel = QString("subgraph ") + tgtLabel;
  }
  s << srcLabel << " -> " << tgtLabel << "  ["
    << dynamic_cast<const GraphElement&>(e) << "];" << endl;

  return s;
}

///}
