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

#include "canvasnode.h"
#include "dotgraphview.h"
#include "graphnode.h"
#include "dotdefaults.h"
#include "dot2qtconsts.h"
#include "FontsCache.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QGraphicsScene>
#include <QMatrix>
#include <QPainter>
#include <QStyle>
#include <QPolygonF>
#include <QPixmap>

///#include <kdebug.h>
///#include <klocale.h>

///namespace KGraphViewer
///{
CanvasNode::CanvasNode(DotGraphView* v,
                        GraphNode* s,
                        QGraphicsScene* c,
                        QGraphicsItem* parent)
: CanvasElement(v,(GraphElement*)s, c, parent)

{
  ///kDebug() << s->id();
  connect(s,SIGNAL(changed()),this,SLOT(modelChanged()));

  QString tipStr;
  QString id = s->id();
  QString label = s->label();
  tipStr = QString("id='%1'\nlabel='%2'").arg(id).arg(label);
//   kDebug() << "CanvasEllipseNode setToolTip " << tipStr;
  setToolTip(tipStr);
}

// CanvasHtmlNode::CanvasHtmlNode(
//                                           DotGraphView* v, 
//                                           GraphNode* n,
//                                           const DotRenderOp& dro,
//                                           const DotRenderOpVec& dros,
//                                           QGraphicsScene* c,
//                                           double scaleX, double scaleY, 
//                                           int xMargin, int yMargin, int gh,
//                                           int wdhcf, int hdvcf
//                                         )
// : KHTMLPart(v->viewport()), CanvasNode(v, n)
// {
//   m_renderOperations = dros;
// //   kDebug() << "Creating "<<node()->id()<<" CanvasHtmlNode for" << n
// //     << " with label '" << n->label() << "'";
// 
//   QString myHTMLCode = n->label();
//   myHTMLCode = myHTMLCode.mid(1, myHTMLCode.length() - 2);
// //   kDebug() << "HTML = " << myHTMLCode;
//   begin(KUrl(QString("file:") + QDir::currentPath() + "/index.html"));
//   setAutoloadImages(true);
//   write(myHTMLCode);
//   kDebug() << "HTML written.";
//   end();
//   setStatusMessagesEnabled (false);
// //   view()->setFrameShape ( QFrame::NoFrame );
// //   view()->setFrameShadow ( QFrame::Plain );
// //   view()->setLineWidth ( 0 );
// //   view()->setMidLineWidth ( 0 );
// //   view()->setHScrollBarMode ( Q3ScrollView::AlwaysOff );
// //   view()->setVScrollBarMode ( Q3ScrollView::AlwaysOff );
//   view()->setMarginWidth(0);
//   view()->setMarginHeight(0);
//   m_zoomFactor = m_view->zoom();
//   view()->part()->setZoomFactor(int(m_zoomFactor*100));
//   view()->move(int(n->x()*scaleX*m_zoomFactor), int((gh-n->y())*scaleY*m_zoomFactor));
//   view()->setMinimumSize(int(n->w()*scaleX),int(n->h()*scaleY*m_zoomFactor));
//   view()->setMaximumSize(int(n->w()*scaleX),int(n->h()*scaleY*m_zoomFactor));
//   view()->adjustSize();
//   KHTMLPart::show();
//   CanvasHtmlNode::connect(v, SIGNAL(contentsMoving ( int, int)), this, SLOT(move(int, int)));
//   CanvasHtmlNode::connect(v, SIGNAL(zoomed (double)), this, SLOT(zoomed(double)));
// }
// 
// CanvasHtmlNode::~CanvasHtmlNode() 
// {
//   KHTMLPart::hide();
// }
// 
// // void CanvasHtmlNode::paint(QPainter& p)
// // {
// //   view()->drawContents(&p);
// // }
// 
// void CanvasHtmlNode::move(int x, int y)
// {
// //   kDebug() << "CanvasHtmlNode::move("<<x<<", "<<y<<")";
//   m_xMovedTo = x; m_yMovedTo = y;
//   view()->move(int((node()->x())*m_scaleX*m_zoomFactor - m_xMovedTo), int((m_gh-node()->y())*m_scaleY*m_zoomFactor) - m_yMovedTo);
// //   view()->move(int(x*m_scaleX), int((m_gh-y)*m_scaleY));
// }
// 
// void CanvasHtmlNode::zoomed(double factor)
// {
//   m_zoomFactor = factor;
//   view()->part()->setZoomFactor(int(factor*100));
//   view()->move(int(node()->x()*m_scaleX*m_zoomFactor - m_xMovedTo), int((m_gh-node()->y())*m_scaleY*m_zoomFactor - m_yMovedTo));
//   view()->setMinimumSize(int(node()->w()*m_scaleX*m_zoomFactor),int(node()->h()*m_scaleY*m_zoomFactor));
//   view()->setMaximumSize(int(node()->w()*m_scaleX*m_zoomFactor),int(node()->h()*m_scaleY*m_zoomFactor));
//   view()->adjustSize();
// }

///}

//#include "canvasnode.moc"
