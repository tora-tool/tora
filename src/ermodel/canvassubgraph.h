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

#ifndef CANVAS_SUBGRAPH_H
#define CANVAS_SUBGRAPH_H

#include "canvaselement.h"

///namespace KGraphViewer
///{
class GraphSubgraph;
class DotGraphView;
class QGraphicsScene;

class CanvasSubgraph: public CanvasElement
{
public:
  CanvasSubgraph(
      DotGraphView* v, 
      GraphSubgraph* s,
      QGraphicsScene* c,
      QGraphicsItem* parent = 0);
  virtual ~CanvasSubgraph() {}
  
protected:
};

///}
#endif



