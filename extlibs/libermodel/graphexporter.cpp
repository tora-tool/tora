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


#include "graphexporter.h"
#include "dotgraph.h"

#ifdef GV_LIB
#include <graphviz/gvc.h>
#endif

#include <QFile>
#include <QTextStream>
#include <QTemporaryFile>

//#include <kdebug.h>
//#include <ktemporaryfile.h>

GraphExporter::GraphExporter()
{
}

GraphExporter::~GraphExporter()
{
}

QString GraphExporter::writeDot(const DotGraph* graph, const QString& fileName)
{
  ///kDebug() << fileName;

  QString actualFileName = fileName;

  if (fileName.isEmpty())
  {
    QTemporaryFile tempFile;
    ///tempFile.setSuffix(".dot");
    if (!tempFile.open()) 
    {
      ///kError() << "Unable to open for temp file for writing " << tempFile.name() << endl;
      exit(2);
    }
    actualFileName = tempFile.fileName();
    ///kDebug() << "using " << actualFileName;
  }
  
  QFile f(actualFileName);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    ///kError() << "Unable to open file for writing " << fileName << endl;
    exit(2);
  }
  
  QTextStream stream(&f);

  stream << "digraph \"";
  if (graph->id()!="\"\"")
  {
    stream <<graph->id();
  }
  stream <<"\" {\n";

  stream << "graph [" << *graph <<"]" << Qt::endl;

  /// @TODO Subgraph are not represented as needed in DotGraph, so it is not
  /// possible to save them back : to be changed !
//   kDebug() << "writing subgraphs";
  GraphSubgraphMap::const_iterator sit;
  for ( sit = graph->subgraphs().begin();
  sit != graph->subgraphs().end(); ++sit )
  {
    const GraphSubgraph& s = **sit;
    (stream) << s;
  }

//   kDebug() << "writing nodes";
  GraphNodeMap::const_iterator nit;
  for ( nit = graph->nodes().begin();
        nit != graph->nodes().end(); ++nit )
  {
    (stream) << **nit;
  }

  ///kDebug() << "writing edges";
  GraphEdgeMap::const_iterator eit;
  for ( eit = graph->edges().begin();
        eit != graph->edges().end(); ++eit )
  {
    ///kDebug() << "writing edge" << (*eit)->id();
    stream << **eit;
  }

  stream << "}\n";

  f.close();
  return actualFileName;
}

#ifdef GV_LIB
graph_t* GraphExporter::exportToGraphviz(const DotGraph* graph)
{
  Agdesc_t type = Agstrictundirected;
  type.directed = graph->directed();
  type.strict = graph->strict();
  
  graph_t* agraph = agopen((graph->id()!="\"\"")?graph->id().toUtf8().data():QString("unnamed").toUtf8().data(), type, NULL);

  QTextStream stream;
  graph->exportToGraphviz(agraph);
  /// @TODO Subgraph are not represented as needed in DotGraph, so it is not
  /// possible to save them back : to be changed !
  //   kDebug() << "writing subgraphs";
  GraphSubgraphMap::const_iterator sit;
  for ( sit = graph->subgraphs().begin();
  sit != graph->subgraphs().end(); ++sit )
  {
    const GraphSubgraph& s = **sit;
    graph_t* subgraph = agsubg(agraph, s.id().toUtf8().data(), 1);
    s.exportToGraphviz(subgraph);
  }
  
  //   kDebug() << "writing nodes";
  GraphNodeMap::const_iterator nit;
  foreach (GraphNode* n, graph->nodes())
  {
    node_t* node = agnode(agraph, n->id().toUtf8().data(), 1);
    n->exportToGraphviz(node);
  }
  
  ///kDebug() << "writing edges";
  GraphEdgeMap::const_iterator eit;
  foreach (GraphEdge* e, graph->edges())
  {
    ///kDebug() << "writing edge" << e->id();
    edge_t* edge = agedge(agraph, agnode(agraph, e->fromNode()->id().toUtf8().data(), 0),
                          agnode(agraph, e->toNode()->id().toUtf8().data(), 0), NULL, 1);
    e->exportToGraphviz(edge);
  }
  
  return agraph;
}
#endif
