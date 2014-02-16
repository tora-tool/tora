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
 * Subgraph model
 */

#ifndef GRAPH_SUBGRAPH_H
#define GRAPH_SUBGRAPH_H

#include <QMap>
#include <QTextStream>

#include "dotgrammar.h"
#include "graphelement.h"
#include "dotrenderop.h"

#ifdef GV_LIB
#include <graphviz/gvc.h>
#endif

///namespace KGraphViewer
///{
  
class CanvasSubgraph;
class GraphSubgraph;

typedef QMap<QString, GraphSubgraph*> GraphSubgraphMap;


/**
 * Colors and styles are dot names
 */
class GraphSubgraph : public GraphElement
{
//   Q_OBJECT
public:
  GraphSubgraph();
#ifdef GV_LIB
  explicit GraphSubgraph(graph_t* sg);
#endif

  virtual ~GraphSubgraph() {}  

  inline const GraphSubgraphMap& subgraphs() const {return m_subgraphsMap;}
  inline GraphSubgraphMap& subgraphs() {return m_subgraphsMap;}
  
  void updateWithSubgraph(const GraphSubgraph& subgraph);
#ifdef GV_LIB
  void updateWithSubgraph(graph_t* subgraph);
#endif  
  CanvasSubgraph* canvasSubgraph() { return (CanvasSubgraph*)canvasElement();  }
  void setCanvasSubgraph(CanvasSubgraph* cs) { setCanvasElement((CanvasElement*)cs); }

  virtual QString backColor() const;

  inline const QList<GraphElement*>& content() const {return m_content;}
  inline QList<GraphElement*>& content() {return m_content;}
  inline void setContent(QList<GraphElement*>& c) {m_content=c;}

  void removeElement(GraphElement* element);

  /// Recursively walk through this subgraph and its subsubgraphs to find an element named id
  /// @return the node found or 0 if there is no such node
  GraphElement* elementNamed(const QString& id);


  /// Recursively walk through this subgraph and its subsubgraphs to make
  /// the given element selected or not depending on the selectValue parameter
  /// and unselect other elements depending on the  unselect others parameter
  /// @return true if the given node was found
  virtual bool setElementSelected(
      GraphElement* element,
      bool selectValue,
      bool unselectOthers);

  void retrieveSelectedElementsIds(QList<QString> selection);
  
 private:
  QList<GraphElement*> m_content;
  GraphSubgraphMap m_subgraphsMap;
};

QTextStream& operator<<(QTextStream& stream, const GraphSubgraph& s);

///}
#endif



