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
 * Graph Node
 */

#include "graphnode.h"
#include "dotgraphview.h"
#include "pannerview.h"
#include "canvasnode.h"
#include "dotdefaults.h"

#include <math.h>

///#include <kdebug.h>

///namespace KGraphViewer
///{
//
// GraphNode
//

GraphNode::GraphNode() :
    GraphElement()
{
//   kDebug() ;
}

GraphNode::GraphNode(const GraphNode& gn) :
GraphElement(gn)
{
  //   kDebug() ;
}

#ifdef GV_LiB
GraphNode::GraphNode(node_t* gn) : GraphElement()
{
  ///kDebug();
  updateWithNode(gn);
}
#endif

void GraphNode::updateWithNode(const GraphNode& node)
{
  ///kDebug() << id() << node.id();
  GraphElement::updateWithElement(node);
  if (canvasNode())
  {
    canvasNode()->computeBoundingRect();
    canvasNode()->modelChanged();
  }
//   kDebug() << "done";
}

#ifdef GV_LiB
void GraphNode::updateWithNode(node_t* node)
{
  ///kDebug() << agnameof(node);
  m_attributes["id"] = agnameof(node);
  m_attributes["label"] = ND_label(node)->text;


  renderOperations().clear();
  if (agget(node, (char*)"_draw_") != NULL)
  {
    parse_renderop(agget(node, (char*)"_draw_"), renderOperations());
    ///kDebug() << "_draw_: element renderOperations size is now " << renderOperations().size();
  }
  if (agget(node, (char*)"_ldraw_") != NULL)
  {
    parse_renderop(agget(node, (char*)"_ldraw_"), renderOperations());
    ///kDebug() << "_ldraw_: element renderOperations size is now " << renderOperations().size();
  }

  Agsym_t *attr = agnxtattr(agraphof(node), AGNODE, NULL);
  while(attr)
  {
    ///kDebug() << agnameof(node) << ":" << attr->name << agxget(node,attr);
    m_attributes[attr->name] = agxget(node,attr);
    attr = agnxtattr(agraphof(node), AGNODE, attr);
  }
}
#endif

QTextStream& operator<<(QTextStream& s, const GraphNode& n)
{
  s << n.id() << "  ["
    << dynamic_cast<const GraphElement&>(n)
    <<"];"<<endl;
  return s;
}

///}
