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

#include "dotgraphview.h"
#include "dotgraph.h"
#include "graphelement.h"
#include "pannerview.h"
#include "canvassubgraph.h"
#include "canvasedge.h"
#include "dot2qtconsts.h"
#include "graphnode.h"
#include "canvasnode.h"
#include "graphedge.h"
#include "FontsCache.h"
///#include "kgraphviewer_partsettings.h"
///#include "simpleprintingcommand.h"

#include <stdlib.h>
#include <math.h>
#include <iostream>

#include <QtGui/QMatrix>
#include <QtGui/QPainter>
#include <QStyle>
#include <QtGui/QImage>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QPixmap>
#include <QtGui/QBitmap>
#include <QtGui/QResizeEvent>
#include <QtGui/QFocusEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QMenu>
#include <QGraphicsSimpleTextItem>
#include <QScrollBar>
#include <QActionGroup>
#include <QInputDialog>
#include <QApplication>
#include <QFileDialog>

#include <QMessageBox> // TODO just a test - remove me

///#include <kdebug.h>
///#include <klocale.h>
///#include <kfiledialog.h>
///#include <kmessagebox.h>
///#include <kinputdialog.h>
///#include <kselectaction.h>
///#include <ktoggleaction.h>
///#include <kstandarddirs.h>
///#include <kactionmenu.h>
    
// DotGraphView defaults

#define DEFAULT_ZOOMPOS      KGraphViewerInterface::Auto
#define KGV_MAX_PANNER_NODES 100

//
// DotGraphView
//
DotGraphView::DotGraphView(QActionGroup* actions, QWidget* parent) : 
    QGraphicsView(parent), 
    m_labelViews(),
    m_popup(0),
    m_zoom(1),
    m_isMoving(false),
    m_exporter(),
    m_zoomPosition(DEFAULT_ZOOMPOS), 
    m_lastAutoPosition(KGraphViewerInterface::TopLeft),
    m_graph(0),
    ///m_printCommand(0),
    m_actions(actions),
    m_detailLevel(DEFAULT_DETAILLEVEL),
    m_defaultNewElement(0),
    ///m_defaultNewElementPixmap(KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-newnode.png")),
    m_editingMode(None),
    m_newEdgeSource(0),
    m_newEdgeDraft(0),
    m_readWrite(false),
    m_leavedTimer(std::numeric_limits<int>::max()),
    m_highlighting(false)
#ifdef GV_LIB
    m_loadThread(),
    m_layoutThread()
#endif
{
  ///kDebug() << "New node pic=" << KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-newnode.png");
  m_canvas = 0;
  m_xMargin = m_yMargin = 0;
  m_birdEyeView = new PannerView(this);
  m_cvZoom = 1;

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  
  m_birdEyeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_birdEyeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_birdEyeView->raise();
  m_birdEyeView->hide();

  setFocusPolicy(Qt::StrongFocus);
  setBackgroundRole(QPalette::Window);
//   viewport()->setMouseTracking(true);
  
  connect(m_birdEyeView, SIGNAL(zoomRectMovedTo(QPointF)),
          this, SLOT(zoomRectMovedTo(QPointF)));
  connect(m_birdEyeView, SIGNAL(zoomRectMoveFinished()),
          this, SLOT(zoomRectMoveFinished()));

  setWhatsThis(  
    "<h1>GraphViz dot format graph visualization</h1>"
    "<p>If the graph is larger than the widget area, an overview "
        "panner is shown in one edge. Choose through the context menu "
        "if the optimal position of this overview should be automatically "
        "computed or put it where you want.</p>"
    "<h2>How to work with it ?</h2>"
    "<ul>"
        "<li>To move the graph, you can:"
            "  <ul>"
                "    <li>click & drag it</li>"
                "    <li>use the elevators</li>"
                "    <li>press the arrows keys</li>"
                "    <li>click somewhere in the panner view</li>" 
                "    <li>use the mouse wheel (up and down with no modifier, left and right with the <Alt> key pressed)</li>"
                    "    <li>or click & drag the panner view</li>"
                    "  </ul>"
                "</li>"
            "<li>To zoom, you can either use the zoom in and zoom out toolbar buttons, or click on the <Shift> key while rolling your mouse wheel.</li>"
                "<li>Try the contextual menu (usually by right-clicking) to discover other "
                    "possibilities.</li>"
                "<li>Try the <tt>Print preview</tt> or the <tt>Page setup</tt> buttons to explore the printing options.</li>"
                "</ul>"
            );

  readViewConfig();
  
  QMatrix m;
  m.scale(m_zoom,m_zoom);
  setMatrix(m);
  setupPopup();
  setInteractive(true);
  setDragMode(NoDrag);
  setRenderHint(QPainter::Antialiasing);

  connect(this, SIGNAL(removeEdge(const QString&)), m_graph, SLOT(removeEdge(const QString&)));
  connect(this, SIGNAL(removeNodeNamed(const QString&)), m_graph, SLOT(removeNodeNamed(const QString&)));
  connect(this, SIGNAL(removeElement(const QString&)), m_graph, SLOT(removeElement(const QString&)));
#ifdef GV_LIB
  connect(&m_loadThread, SIGNAL(finished()), this, SLOT(slotAGraphReadFinished()));
  connect(&m_layoutThread, SIGNAL(finished()), this, SLOT(slotAGraphLayoutFinished()));
#endif
}

DotGraphView::~DotGraphView()
{
  saveViewConfig();
  delete m_birdEyeView;
  m_birdEyeView = 0;
  if (m_popup != 0)
  {
    delete m_popup;
  }
  if (m_canvas) 
  {
    setScene(0);
    delete m_canvas;
  }
  if (m_graph != 0)
  {
    delete m_graph;
  }
}

bool DotGraphView::initEmpty()
{
  ///kDebug();
  m_birdEyeView->hide();
  m_birdEyeView->setScene(0);
  
  if (m_canvas) 
  {
    delete m_canvas;
    m_canvas = 0;
  }

  if (m_graph != 0)
    delete m_graph;
  m_graph = new DotGraph();
  connect(m_graph,SIGNAL(readyToDisplay()),this,SLOT(displayGraph()));
  connect(this, SIGNAL(removeEdge(const QString&)), m_graph, SLOT(removeEdge(const QString&)));
  connect(this, SIGNAL(removeNodeNamed(const QString&)), m_graph, SLOT(removeNodeNamed(const QString&)));
//   connect(this, SIGNAL(removeElement(const QString&)), m_graph, SLOT(removeElement(const QString&)));

  if (m_readWrite)
  {
    m_graph->setReadWrite();
  }
  
//   kDebug() << "Parsing " << m_graph->dotFileName() << " with " << m_graph->layoutCommand();
  m_xMargin = 50;
  m_yMargin = 50;

  QGraphicsScene* newCanvas = new QGraphicsScene();
  QGraphicsSimpleTextItem* item = newCanvas->addSimpleText("no graph loaded");
//   kDebug() << "Created canvas " << newCanvas;
  
  m_birdEyeView->setScene(newCanvas);
//   std::cerr << "After m_birdEyeView set canvas" << std::endl;
  
  setScene(newCanvas);
  m_canvas = newCanvas;
  centerOn(item);

  m_cvZoom = 0;

  return true;
}

bool DotGraphView::loadDot(const QString& dotFileName)
{
  ///kDebug() << "'" << dotFileName << "'";
  m_birdEyeView->setScene(0);

  if (m_canvas)
  {
    m_canvas->deleteLater();
    m_canvas = 0;
  }

  QString layoutCommand = (m_graph!=0?m_graph->layoutCommand():"");
  if (m_graph != 0)
    delete m_graph;
  m_graph = new DotGraph(layoutCommand,dotFileName);
  connect(m_graph,SIGNAL(readyToDisplay()),this,SLOT(displayGraph()));
  connect(this, SIGNAL(removeEdge(const QString&)), m_graph, SLOT(removeEdge(const QString&)));
  connect(this, SIGNAL(removeNodeNamed(const QString&)), m_graph, SLOT(removeNodeNamed(const QString&)));
  connect(this, SIGNAL(removeElement(const QString&)), m_graph, SLOT(removeElement(const QString&)));

  if (m_readWrite)
  {
    m_graph->setReadWrite();
  }
  if (layoutCommand.isEmpty())
  {
    layoutCommand = m_graph->chooseLayoutProgramForFile(m_graph->dotFileName());
  }
  m_graph->layoutCommand(layoutCommand);

//   kDebug() << "Parsing " << m_graph->dotFileName() << " with " << m_graph->layoutCommand();
  m_xMargin = 50;
  m_yMargin = 50;

  QGraphicsScene* newCanvas = new QGraphicsScene();
  ///kDebug() << "Created canvas " << newCanvas;

  m_birdEyeView->setScene(newCanvas);
//   std::cerr << "After m_birdEyeView set canvas" << std::endl;

  setScene(newCanvas);
  connect(newCanvas,SIGNAL(selectionChanged ()),this,SLOT(slotSelectionChanged()));
  m_canvas = newCanvas;

  QGraphicsSimpleTextItem* loadingLabel = newCanvas->addSimpleText(QString("graph %1 is getting loaded...").arg(dotFileName));
  loadingLabel->setZValue(100);
  centerOn(loadingLabel);

  m_cvZoom = 0;

  if (!m_graph->parseDot(m_graph->dotFileName()))
  {
    ///kError() << "NOT successfully parsed!" << endl;
    loadingLabel->setText(QString("error parsing file %1").arg(dotFileName));
    return false;
  }
  return true;
}

#ifdef GV_LIB
bool DotGraphView::loadLibrary(const QString& dotFileName)
{
  ///kDebug() << "'" << dotFileName << "'";

  m_canvas->clear();
  QGraphicsSimpleTextItem* loadingLabel = m_canvas->addSimpleText(QString("graph %1 is getting loaded...").arg(dotFileName));
  loadingLabel->setZValue(100);
  centerOn(loadingLabel);

  m_loadThread.loadFile(dotFileName);
  
  return true;
}

bool DotGraphView::loadLibrary(graph_t* graph, const QString& layoutCommand)
{
  ///kDebug() << "graph_t";
  m_birdEyeView->setScene(0);
  
  if (m_canvas)
  {
    m_canvas->deleteLater();
    m_canvas = 0;
  }
  
  if (m_graph != 0)
    delete m_graph;

  ///kDebug() << "layoutCommand:" << layoutCommand;
  m_graph = new DotGraph(layoutCommand,"");
  m_graph->setUseLibrary(true);
  
  connect(m_graph,SIGNAL(readyToDisplay()),this,SLOT(displayGraph()));
  connect(this, SIGNAL(removeEdge(const QString&)), m_graph, SLOT(removeEdge(const QString&)));
  connect(this, SIGNAL(removeNodeNamed(const QString&)), m_graph, SLOT(removeNodeNamed(const QString&)));
  connect(this, SIGNAL(removeElement(const QString&)), m_graph, SLOT(removeElement(const QString&)));
  
  if (m_readWrite)
  {
    m_graph->setReadWrite();
  }
  
  m_xMargin = 50;
  m_yMargin = 50;
  
  QGraphicsScene* newCanvas = new QGraphicsScene();
  ///kDebug() << "Created canvas " << newCanvas;
  
  m_birdEyeView->setScene(newCanvas);
  setScene(newCanvas);
  connect(newCanvas,SIGNAL(selectionChanged ()),this,SLOT(slotSelectionChanged()));
  m_canvas = newCanvas;
  
  m_cvZoom = 0;

             
                                      
  m_graph->updateWithGraph(graph);

  return true;
}
#endif

void DotGraphView::slotSelectionChanged()
{
  ///kDebug() << scene()->selectedItems().size();
}

bool DotGraphView::displayGraph()
{
  ///kDebug();
//   hide();
  viewport()->setUpdatesEnabled(false);

  m_canvas->clear();

  if (m_graph->nodes().size() > KGV_MAX_PANNER_NODES)
  {
    m_birdEyeView->setDrawingEnabled(false);
  }
  //  QCanvasEllipse* eItem;
  double scaleX = 1.0, scaleY = 1.0;

  if (m_detailLevel == 0)      { scaleX = m_graph->scale() * 0.7; scaleY = m_graph->scale() * 0.7; }
  else if (m_detailLevel == 1) { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
  else if (m_detailLevel == 2) { scaleX = m_graph->scale() * 1.3; scaleY = m_graph->scale() * 1.3; }
  else                        { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }

  qreal gh = m_graph->height();

  m_xMargin = 50;
  m_yMargin = 50;


//   m_canvas->setSceneRect(0,0,w+2*m_xMargin, h+2*m_yMargin);
  m_canvas->setBackgroundBrush(QBrush(QColor(m_graph->backColor())));

//   kDebug() << "sceneRect is now " << m_canvas->sceneRect();
  
  ///kDebug() << "Creating" << d->m_graph->subgraphs().size() << "CanvasSubgraphs from" << d->m_graph;
  int zvalue = -1;
  foreach (GraphSubgraph* gsubgraph,m_graph->subgraphs())
  {
    int newZvalue = displaySubgraph(gsubgraph, zvalue);
    if (newZvalue > zvalue)
      zvalue = newZvalue;
  }

  ///kDebug() << "Creating" << d->m_graph->nodes().size() << "nodes from" << d->m_graph;
  foreach (QString id, m_graph->nodes().keys())
  {
    GraphNode* gnode = m_graph->nodes()[id];
    ///kDebug() << "Handling" << id << (void*)gnode;
    ///kDebug() << "  gnode id=" << gnode->id();
    ///kDebug() << "  gnode id=" << gnode->id();
    if (gnode->canvasNode()==0)
    {
      ///kDebug() << "Creating canvas node for" << gnode->id();
      CanvasNode *cnode = new CanvasNode(this, gnode, m_canvas);
      if (cnode == 0) continue;
      cnode->initialize(
        scaleX, scaleY, m_xMargin, m_yMargin, gh,
        m_graph->wdhcf(), m_graph->hdvcf());
      gnode->setCanvasNode(cnode);
      m_canvas->addItem(cnode);
//       cnode->setZValue(gnode->z());
      cnode->setZValue(zvalue+1);
      cnode->show();
    }
    gnode->canvasNode()->computeBoundingRect();
  }

  ///kDebug() << "Creating" << m_graph->edges().size() << "edges from" << m_graph;
  foreach (GraphEdge* gedge, m_graph->edges())
  {
    ///kDebug() << "One GraphEdge:" << gedge->id();
    if (gedge->canvasEdge() == 0
      && gedge->fromNode() != 0
      && gedge->toNode() != 0)
    {
      ///kDebug() << "New CanvasEdge for" << gedge->id();
      ///kDebug() << "edge fromNode=" << (void*)gedge->fromNode();
      ///kDebug() << "              "<< gedge->fromNode()->id();
      ///kDebug() << "edge toNode=" << (void*)gedge->toNode();
      ///kDebug() << "              "<< gedge->toNode()->id();
      CanvasEdge* cedge = new CanvasEdge(this, gedge, scaleX, scaleY, m_xMargin,
          m_yMargin, gh, m_graph->wdhcf(), m_graph->hdvcf());

      gedge->setCanvasEdge(cedge);
  //     std::cerr << "setting z = " << gedge->z() << std::endl;
  //    cedge->setZValue(gedge->z());
      cedge->setZValue(zvalue+2);
      cedge->show();
      m_canvas->addItem(cedge);
    }
    if (gedge->canvasEdge() != 0)
      gedge->canvasEdge()->computeBoundingRect();
  }
  ///kDebug() << "Adding graph render operations: " << m_graph->renderOperations().size();
  foreach (const DotRenderOp& dro,m_graph->renderOperations())
  {
    if ( dro.renderop == "T" )
    {
//       std::cerr << "Adding graph label '"<<dro.str<<"'" << std::endl;
      const QString& str = dro.str;
      int stringWidthGoal = int(dro.integers[3] * scaleX);
      int fontSize = m_graph->fontSize();
      QFont* font = FontsCache::changeable().fromName(m_graph->fontName());
      font->setPointSize(fontSize);
      QFontMetrics fm(*font);
      while (fm.width(str) > stringWidthGoal && fontSize > 1)
      {
        fontSize--;
        font->setPointSize(fontSize);
        fm = QFontMetrics(*font);
      }
      QGraphicsSimpleTextItem* labelView = new QGraphicsSimpleTextItem(str, 0/* obsolete parameter, m_canvas*/);
      labelView->setFont(*font);
      labelView->setPos(
                  (scaleX *
                       (
                         (dro.integers[0])
                         + (((dro.integers[2])*(dro.integers[3]))/2)
                         - ( (dro.integers[3])/2 )
                       )
                      + m_xMargin ),
                      ((gh - (dro.integers[1]))*scaleY)+ m_yMargin);
      /// @todo port that ; how to set text color ?
      labelView->setPen(QPen(Dot2QtConsts::componentData().qtColor(m_graph->fontColor())));
      labelView->setFont(*font);
      m_labelViews.insert(labelView);
    }
  }

  ///kDebug() << "Finalizing";
  m_cvZoom = 0;
  updateSizes();

  centerOn(m_canvas->sceneRect().center());

  viewport()->setUpdatesEnabled(true);
  QSet<QGraphicsSimpleTextItem*>::iterator labelViewsIt, labelViewsIt_end;
  labelViewsIt = m_labelViews.begin(); labelViewsIt_end = m_labelViews.end();
  for (; labelViewsIt != labelViewsIt_end; labelViewsIt++)
  {
    (*labelViewsIt)->show();
  }
  m_canvas->update();
  
  emit graphLoaded();

  return true;
}

int DotGraphView::displaySubgraph(GraphSubgraph* gsubgraph, int zValue, CanvasElement* parent)
{
  ///kDebug();
  double scaleX = 1.0, scaleY = 1.0;
  
  if (m_detailLevel == 0)      { scaleX = m_graph->scale() * 0.7; scaleY = m_graph->scale() * 0.7; }
  else if (m_detailLevel == 1) { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
  else if (m_detailLevel == 2) { scaleX = m_graph->scale() * 1.3; scaleY = m_graph->scale() * 1.3; }
  else                        { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
  
  qreal gh = m_graph->height();
  
  if (gsubgraph->canvasSubgraph() == 0)
  {
    ///kDebug() << "Creating canvas subgraph for" << gsubgraph->id();
    CanvasSubgraph* csubgraph = new CanvasSubgraph(this, gsubgraph, m_canvas, parent);
    csubgraph->initialize(
    scaleX, scaleY, m_xMargin, m_yMargin, gh,
                          m_graph->wdhcf(), m_graph->hdvcf());
                          gsubgraph->setCanvasSubgraph(csubgraph);
                          //       csubgraph->setZValue(gsubgraph->z());
                          csubgraph->setZValue(zValue+=2);
                          csubgraph->show();
                          m_canvas->addItem(csubgraph);
                          ///kDebug() << " one CanvasSubgraph... Done";
  }
  foreach (GraphElement* element, gsubgraph->content())
  {
    GraphNode* gnode = dynamic_cast<GraphNode*>(element);
    if (gnode->canvasNode()==0)
    {
      ///kDebug() << "Creating canvas node for:" << gnode->id();
      CanvasNode *cnode = new CanvasNode(this, gnode, m_canvas);
      if (cnode == 0) continue;
      cnode->initialize(
      scaleX, scaleY, m_xMargin, m_yMargin, gh,
                        m_graph->wdhcf(), m_graph->hdvcf());
                        gnode->setCanvasNode(cnode);
                        m_canvas->addItem(cnode);
                        //       cnode->setZValue(gnode->z());
                        cnode->setZValue(zValue+1);
                        cnode->show();
    }
    gnode->canvasNode()->computeBoundingRect();
  }
  gsubgraph->canvasSubgraph()->computeBoundingRect();

  int newZvalue = zValue;
  foreach(GraphSubgraph* ssg, gsubgraph->subgraphs())
  {
    int hereZvalue = displaySubgraph(ssg, zValue, gsubgraph->canvasSubgraph());
    if (hereZvalue > newZvalue)
      newZvalue = hereZvalue;
  }
  return newZvalue;
}


void DotGraphView::updateSizes(QSizeF s)
{
  ///kDebug() ;
  if (m_canvas == 0)
    return;
  if (s == QSizeF(0,0)) s = size();

  // the part of the canvas that should be visible
  qreal cWidth  = m_canvas->width()  - 2*m_xMargin + 100;
  qreal cHeight = m_canvas->height() - 2*m_yMargin + 100;

  // hide birds eye view if no overview needed
  if (//!_data || !_activeItem ||
      ///!KGraphViewerPartSettings::birdsEyeViewEnabled() ||
      (((cWidth * m_zoom) < s.width()) && (cHeight * m_zoom) < s.height())) 
  {
    m_birdEyeView->hide();
    return;
  }
  m_birdEyeView->hide();

  // first, assume use of 1/3 of width/height (possible larger)
  double zoom = .33 * s.width() / cWidth;
  if (zoom * cHeight < .33 * s.height()) zoom = .33 * s.height() / cHeight;

  // fit to widget size
  if (cWidth  * zoom  > s.width())   zoom = s.width() / (double)cWidth;
  if (cHeight * zoom  > s.height())  zoom = s.height() / (double)cHeight;

  // scale to never use full height/width
  zoom = zoom * 3/4;

  // at most a zoom of 1/3
  if (zoom > .33) zoom = .33;

  if (zoom != m_cvZoom) 
  {
    m_cvZoom = zoom;

    QMatrix wm;
    wm.scale( zoom, zoom );
    m_birdEyeView->setMatrix(wm);

    // make it a little bigger to compensate for widget frame
    m_birdEyeView->resize((cWidth * zoom) + 4,
                          (cHeight * zoom) + 4);

  }
  updateBirdEyeView();
  m_birdEyeView->setZoomRect(mapToScene(viewport()->rect()).boundingRect());
  m_birdEyeView->show();
  QSizeF newCanvasSize = m_canvas->sceneRect().size();
  if (newCanvasSize.width() < viewport()->width())
  {
    newCanvasSize.setWidth(viewport()->width());
  }
  else if (viewport()->width() < m_canvas->sceneRect().size().width())
  {
    newCanvasSize.setWidth(m_canvas->sceneRect().size().width());
  }
  if (newCanvasSize.height() < viewport()->height())
  {
    newCanvasSize.setHeight(viewport()->height());
  }
  else if (viewport()->height() < m_canvas->sceneRect().size().height())
  {
    newCanvasSize.setHeight(m_canvas->sceneRect().size().height());
  }
//   std::cerr << "done." << std::endl;
}

void DotGraphView::updateBirdEyeView()
{
  qreal cvW = m_birdEyeView->width();
  qreal cvH = m_birdEyeView->height();
  qreal x = width()- cvW - verticalScrollBar()->width()    -2;
  qreal y = height()-cvH - horizontalScrollBar()->height() -2;
  QPoint oldZoomPos = m_birdEyeView->pos();
  QPoint newZoomPos = QPoint(0,0);
  KGraphViewerInterface::PannerPosition zp = m_zoomPosition;
  if (zp == KGraphViewerInterface::Auto) 
  {
    QPointF tl1Pos = mapToScene(QPoint(0,0));
    QPointF tl2Pos = mapToScene(QPoint(cvW,cvH));
    QPointF tr1Pos = mapToScene(QPoint(x,0));
    QPointF tr2Pos = mapToScene(QPoint(x+cvW,cvH));
    QPointF bl1Pos = mapToScene(QPoint(0,y));
    QPointF bl2Pos = mapToScene(QPoint(cvW,y+cvH));
    QPointF br1Pos = mapToScene(QPoint(x,y));
    QPointF br2Pos = mapToScene(QPoint(x+cvW,y+cvH));
    int tlCols = m_canvas->items(QRectF(tl1Pos.x(),tl1Pos.y(),tl2Pos.x(),tl2Pos.y())).count();
    int trCols = m_canvas->items(QRectF(tr1Pos.x(),tr1Pos.y(),tr2Pos.x(),tr2Pos.y())).count();
    int blCols = m_canvas->items(QRectF(bl1Pos.x(),bl1Pos.y(),bl2Pos.x(),bl2Pos.y())).count();
    int brCols = m_canvas->items(QRectF(br1Pos.x(),br1Pos.y(),br2Pos.x(),br2Pos.y())).count();
    int minCols = tlCols;
    zp = m_lastAutoPosition;
    switch(zp) 
    {
      case KGraphViewerInterface::TopRight:    minCols = trCols; break;
      case KGraphViewerInterface::BottomLeft:  minCols = blCols; break;
      case KGraphViewerInterface::BottomRight: minCols = brCols; break;
      default:
      case KGraphViewerInterface::TopLeft:     minCols = tlCols; break;
    }
    if (minCols > tlCols) { minCols = tlCols; zp = KGraphViewerInterface::TopLeft; }
    if (minCols > trCols) { minCols = trCols; zp = KGraphViewerInterface::TopRight; }
    if (minCols > blCols) { minCols = blCols; zp = KGraphViewerInterface::BottomLeft; }
    if (minCols > brCols) { minCols = brCols; zp = KGraphViewerInterface::BottomRight; }
  
    m_lastAutoPosition = zp;
  }

  switch(zp) 
  {
    case KGraphViewerInterface::TopRight:
      newZoomPos = QPoint(x,0);
    break;
    case KGraphViewerInterface::BottomLeft:
      newZoomPos = QPoint(0,y);
    break;
    case KGraphViewerInterface::BottomRight:
      newZoomPos = QPoint(x,y);
    break;
    default:
    break;
  }
  if (newZoomPos != oldZoomPos) 
    m_birdEyeView->move(newZoomPos);
}

void DotGraphView::focusInEvent(QFocusEvent*)
{
  if (!m_canvas) return;

//   m_canvas->update();
}

void DotGraphView::focusOutEvent(QFocusEvent* e)
{
  // trigger updates as in focusInEvent
  focusInEvent(e);
}

void DotGraphView::keyPressEvent(QKeyEvent* e)
{
  if (!m_canvas) 
  {
    e->ignore();
    return;
  }

  // move canvas...
  if (e->key() == Qt::Key_Home)
    scrollContentsBy(int(-m_canvas->width()),0);
  else if (e->key() == Qt::Key_End)
    scrollContentsBy(int(m_canvas->width()),0);
  else if (e->key() == Qt::Key_PageUp)
    scrollContentsBy(0,-viewport()->height()/2);
  else if (e->key() == Qt::Key_PageDown)
    scrollContentsBy(0,viewport()->height()/2);
  else if (e->key() == Qt::Key_Left)
    scrollContentsBy(-viewport()->width()/10,0);
  else if (e->key() == Qt::Key_Right)
    scrollContentsBy(viewport()->width()/10,0);
  else if (e->key() == Qt::Key_Down)
    scrollContentsBy(0,viewport()->height()/10);
  else if (e->key() == Qt::Key_Up)
    scrollContentsBy(0,-viewport()->height()/10);
  else 
  {
    e->ignore();
    return;
  }
}

void DotGraphView::wheelEvent(QWheelEvent* e)
{
  if (!m_canvas) 
  {
    e->ignore();
    return;
  }
  e->accept();
  ///if (e->state() == Qt::ShiftModifier)
  if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
  {
    ///kDebug() << " + Shift: zooming";
    // move canvas...
    if (e->delta() < 0)
    {
      zoomOut();
    }
    else 
    {
      zoomIn();
    }
  }
  else
  {
    ///kDebug() << " : scrolling ";
    if (e->orientation() == Qt::Horizontal)
    {
      if (e->delta() < 0)
      {
        ///kDebug() << "scroll by " <<  -viewport()->width()/10 << 0;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()+viewport()->width()/10);
      }
      else
      {
        ///kDebug() << "scroll by " <<  viewport()->width()/10 << 0;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-viewport()->width()/10);
      }
    }
    else
    {
      if (e->delta() < 0)
      {
        ///kDebug() << "scroll by " << 0 << viewport()->width()/10;
        verticalScrollBar()->setValue(verticalScrollBar()->value()+viewport()->height()/10);
      }
      else
      {
        ///kDebug() << "scroll by " << 0 << -viewport()->width()/10;
        verticalScrollBar()->setValue(verticalScrollBar()->value()-viewport()->height()/10);
      }
    }
  }
}

void DotGraphView::zoomIn()
{
  applyZoom(1.10);
}


void DotGraphView::zoomOut()
{
  applyZoom(.90);
}

void DotGraphView::setZoomFactor(double newZoom)
{
  if (newZoom < 0.1 || newZoom > 10)
    return;
  m_zoom = newZoom;
  if (m_zoom > 1.0 && m_zoom < 1.1) 
  {
    m_zoom = 1;
  }

  const double factor = newZoom / m_zoom;
  qreal centerX = (sceneRect().x() + (viewport()->width() / 2))*factor;
  qreal centerY = (sceneRect().y() + (viewport()->height() / 2))*factor;
  
  setUpdatesEnabled(false);
  QMatrix m;
  m.scale(m_zoom,m_zoom);
  setMatrix(m);
  centerOn(centerX, centerY);
  emit zoomed(m_zoom);
  setUpdatesEnabled(true);
  updateSizes();
}

void DotGraphView::applyZoom(double factor)
{
  setZoomFactor(m_zoom * factor);
}

void DotGraphView::scrollContentsBy(int dx, int dy)
{
  QGraphicsView::scrollContentsBy(dx, dy);
  if (m_birdEyeView && scene()) { // we might be shutting down
    m_birdEyeView->moveZoomRectTo(mapToScene(viewport()->rect()).boundingRect().center(), false);
  }
}

void DotGraphView::resizeEvent(QResizeEvent* e)
{
  ///kDebug() << "resizeEvent";
  QGraphicsView::resizeEvent(e);
  if (m_canvas) updateSizes(e->size());
//   std::cerr << "resizeEvent end" << std::endl;
}

void DotGraphView::zoomRectMovedTo(QPointF newZoomPos)
{
//   kDebug() << "DotGraphView::zoomRectMovedTo " << newZoomPos;
  centerOn(newZoomPos);
}
                    
void DotGraphView::zoomRectMoveFinished()
{
//    kDebug() << "zoomRectMoveFinished";
    updateBirdEyeView();
//   std::cerr << "zoomRectMoveFinished end" << std::endl;
}

void DotGraphView::mousePressEvent(QMouseEvent* e)
{
  if (e->button() != Qt::LeftButton) {
    return;
  }
  ///kDebug() << e << m_editingMode;
  QGraphicsView::mousePressEvent(e);

  if (m_editingMode == AddNewElement)
  {
    double scaleX = 1.0, scaleY = 1.0;

    if (m_detailLevel == 0)      { scaleX = m_graph->scale() * 0.7; scaleY = m_graph->scale() * 0.7; }
    else if (m_detailLevel == 1) { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
    else if (m_detailLevel == 2) { scaleX = m_graph->scale() * 1.3; scaleY = m_graph->scale() * 1.3; }
    else                        { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }

    qreal gh = m_graph->height();


    QPointF pos = mapToScene(
        e->pos().x()-m_defaultNewElementPixmap.width()/2,
        e->pos().y()-m_defaultNewElementPixmap.height()/2);
    GraphNode* newNode = new GraphNode();
    newNode->attributes() = m_newElementAttributes;
    if (newNode->attributes().find("id") == newNode->attributes().end())
    {
      newNode->setId(QString("NewNode%1").arg(m_graph->nodes().size()));
    }
    if (newNode->attributes().find("label") == newNode->attributes().end())
    {
      newNode->setLabel(newNode->id());
    }
    m_graph->nodes().insert(newNode->id(), newNode);
    CanvasNode* newCNode = new CanvasNode(this, newNode, m_canvas);
    newCNode->initialize(
      scaleX, scaleY, m_xMargin, m_yMargin, gh,
      m_graph->wdhcf(), m_graph->hdvcf());
    newNode->setCanvasNode(newCNode);
    scene()->addItem(newCNode);
    ///kDebug() << "setting pos to " << pos;
    newCNode->setPos(pos);
    newCNode->setZValue(100);
    newCNode->show();


    m_editingMode = None;
    unsetCursor();
    emit newNodeAdded(newNode->id());
  }
  else if (m_editingMode == SelectingElements)
  {
  }
  else if (m_editingMode == SelectSingleElement)
  {
	  QGraphicsItem *i = itemAt(e->pos());
	  if (CanvasElement *cei = dynamic_cast<CanvasElement*>(i))
	  {
		  QString id = cei->element()->id();
		  //QMessageBox::information(NULL, tr("Selected table:"), id);
		  if (!id.isEmpty())
			  emit selected(id);
	  }
  }
  else
  {
    if (m_editingMode != None && itemAt(e->pos()) == 0) // click outside any item: unselect all
    {
      if (m_editingMode == DrawNewEdge) // was drawing an edge; cancel it
      {
        if (m_newEdgeDraft!=0)
        {
          m_newEdgeDraft->hide();
          scene()->removeItem(m_newEdgeDraft);
          delete m_newEdgeDraft;
          m_newEdgeDraft = 0;
        }
        m_newEdgeSource = 0;
        m_editingMode = None;
      }
      else if (m_editingMode == AddNewEdge)
      {
        m_editingMode = None;
      }
      foreach(GraphEdge* e, m_graph->edges())
      {
        if (e->isSelected()) {
          e->setSelected(false);
          e->canvasEdge()->update();
        }
      }
      foreach(GraphNode* n, m_graph->nodes())
      {
        if (n->isSelected()) {
          n->setSelected(false);
          n->canvasElement()->update();
        }
      }
      foreach(GraphSubgraph* s, m_graph->subgraphs())
      {
        if (s->isSelected()) {
          s->setSelected(false);
          s->canvasElement()->update();
        }
      }
      emit selectionIs(QList<QString>(),QPoint());
    }
    m_pressPos = e->globalPos();
    m_pressScrollBarsPos = QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
  }
  m_isMoving = true;
}

void DotGraphView::mouseMoveEvent(QMouseEvent* e)
{
  QGraphicsView::mouseMoveEvent(e);
//   kDebug() << scene()->selectedItems().size();

  if (m_editingMode == DrawNewEdge)
  {
    if (m_newEdgeDraft != 0)
    {
      QPointF src = m_newEdgeDraft->line().p1();
      QPointF tgt = mapToScene(e->pos());

//     kDebug() << "Setting new edge draft line to" << QLineF(src,tgt);
      m_newEdgeDraft->setLine(QLineF(src,tgt));
    }
  }
  else if (m_editingMode == SelectingElements)
  {
//     kDebug() << "selecting";
  }
  else if (e->buttons().testFlag(Qt::LeftButton))
  {
//     kDebug() << (e->globalPos() - m_pressPos);
    QPoint diff = e->globalPos() - m_pressPos;
    horizontalScrollBar()->setValue(m_pressScrollBarsPos.x()-diff.x());
    verticalScrollBar()->setValue(m_pressScrollBarsPos.y()-diff.y());
  }
}

void DotGraphView::mouseReleaseEvent(QMouseEvent* e)
{
  ///kDebug() << e << m_editingMode;
//   kDebug() << "setDragMode(NoDrag)";
//   setDragMode(NoDrag);
  if (m_editingMode == AddNewElement)
  {
    m_editingMode = None;
    unsetCursor();
  }
  else if (m_editingMode == SelectingElements)
  {
    QGraphicsView::mouseReleaseEvent(e);
    ///kDebug() << "Stopping selection" << scene() << m_canvas;
    QList<QGraphicsItem *> items = scene()->selectedItems();
    QList<QString> selection;
    foreach (QGraphicsItem * item, items)
    {
      CanvasElement* element = dynamic_cast<CanvasElement*>(item);
      element->element()->setSelected(true);
      if (element != 0)
      {
        selection.push_back(element->element()->id());
      }
    }
    m_editingMode = None;
    unsetCursor();
    setDragMode(NoDrag);
    if (!selection.isEmpty())
    {
      update();
      emit selectionIs(selection, mapToGlobal( e->pos() ));
    }
  }
  else
  {
    QGraphicsView::mouseReleaseEvent(e);
  }
  m_isMoving = false;
}

void DotGraphView::mouseDoubleClickEvent(QMouseEvent* e)
{
  QGraphicsView::mouseDoubleClickEvent(e);
}

void DotGraphView::contextMenuEvent(QContextMenuEvent* e)
{
  ///kDebug();
//   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

  m_popup->exec(e->globalPos());
}

void DotGraphView::slotContextMenuEvent(const QString& id, const QPoint& p)
{
  ///kDebug();
//   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));

  emit (contextMenuEvent(id, p));
}

void DotGraphView::slotElementHoverEnter(CanvasElement* element)
{
  ///kDebug() << element->element()->id();
  //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));
  
  emit (hoverEnter(element->element()->id()));
}

void DotGraphView::slotElementHoverLeave(CanvasElement* element)
{
  ///kDebug() << element->element()->id();
  //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));
  
  emit (hoverLeave(element->element()->id()));
}

void DotGraphView::slotElementHoverEnter(CanvasEdge* element)
{
  ///kDebug() << element->edge()->id();
  //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));
  
  emit (hoverEnter(element->edge()->id()));
}

void DotGraphView::slotElementHoverLeave(CanvasEdge* element)
{
  ///kDebug() << element->edge()->id();
  //   QList<QGraphicsItem *> l = scene()->collidingItems(scene()->itemAt(e->pos()));
  
  emit (hoverLeave(element->edge()->id()));
}

void DotGraphView::setLayoutCommand(const QString& command)
{
  m_graph->layoutCommand(command);
  reload();
}

KGraphViewerInterface::PannerPosition DotGraphView::zoomPos(const QString& s)
{
  KGraphViewerInterface::PannerPosition  res = DEFAULT_ZOOMPOS;
  if (s == QString("KGraphViewerInterface::TopLeft")) res = KGraphViewerInterface::TopLeft;
  if (s == QString("KGraphViewerInterface::TopRight")) res = KGraphViewerInterface::TopRight;
  if (s == QString("KGraphViewerInterface::BottomLeft")) res = KGraphViewerInterface::BottomLeft;
  if (s == QString("KGraphViewerInterface::BottomRight")) res = KGraphViewerInterface::BottomRight;
  if (s == QString("Automatic")) res = KGraphViewerInterface::Auto;

  return res;
}

void DotGraphView::setPannerEnabled(bool enabled)
{
  ///m_bevPopup->setEnabled(m_bevEnabledAction->isChecked());
  ///KGraphViewerPartSettings::setBirdsEyeViewEnabled(m_bevEnabledAction->isChecked());
  ///KGraphViewerPartSettings::self()->writeConfig();
  updateSizes();  
}

void DotGraphView::viewBevActivated(int newZoomPos)
{
  m_zoomPosition = (KGraphViewerInterface::PannerPosition)newZoomPos;
  updateSizes();
  emit(sigViewBevActivated(newZoomPos));
}

QString DotGraphView::zoomPosString(KGraphViewerInterface::PannerPosition p)
{
    if (p == KGraphViewerInterface::TopRight) return QString("KGraphViewerInterface::TopRight");
    if (p == KGraphViewerInterface::BottomLeft) return QString("KGraphViewerInterface::BottomLeft");
    if (p == KGraphViewerInterface::BottomRight) return QString("KGraphViewerInterface::BottomRight");
    if (p == KGraphViewerInterface::Auto) return QString("Automatic");

    return QString("KGraphViewerInterface::TopLeft");
}

void DotGraphView::readViewConfig()
{
  ///KConfigGroup g(KGlobal::config(),"GraphViewLayout");
  
  QVariant dl = DEFAULT_DETAILLEVEL;
  m_detailLevel     = dl.toInt(); /// g.readEntry("DetailLevel", dl).toInt();
  ///m_zoomPosition  = zoomPos(g.readEntry("KGraphViewerInterface::PannerPosition",
  ///          zoomPosString(DEFAULT_ZOOMPOS)));
  zoomPos(zoomPosString(DEFAULT_ZOOMPOS));
  emit(sigViewBevActivated(m_zoomPosition));
}

void DotGraphView::saveViewConfig()
{
//   kDebug() << "Saving view config";  
  ///KConfigGroup g(KGlobal::config(), "GraphViewLayout");

    ///writeConfigEntry(&g, "DetailLevel", m_detailLevel, DEFAULT_DETAILLEVEL);
    ///writeConfigEntry(&g, "KGraphViewerInterface::PannerPosition",
         ///zoomPosString(m_zoomPosition),
         ///zoomPosString(DEFAULT_ZOOMPOS).utf8().data());
  ///g.sync();
}

void DotGraphView::pageSetup()
{
  ///if (m_printCommand == 0)
  ///{
  ///  m_printCommand = new KGVSimplePrintingCommand(this, 0);
  ///}
  ///m_printCommand->showPageSetup(m_graph->dotFileName());
  return;
}

void DotGraphView::print()
{
  ///if (m_printCommand == 0)
  ///{
  ///  m_printCommand = new KGVSimplePrintingCommand(this, 0);
  ///}
  ///m_printCommand->print(m_graph->dotFileName());
  return;
}

void DotGraphView::printPreview()
{
  ///if (m_printCommand == 0)
  ///{
  ///  m_printCommand = new KGVSimplePrintingCommand(this, 0);
  ///}
  ///m_printCommand->showPrintPreview(m_graph->dotFileName(), false);
  return;
}

bool DotGraphView::reload()
{
  QString fileName = m_graph->dotFileName();
#ifdef GV_LIB
  if (m_graph->useLibrary())
    return loadLibrary(fileName);
  else
#endif
    return loadDot(fileName);
}

void DotGraphView::dirty(const QString& dotFileName)
{
//   std::cerr << "SLOT dirty for " << dotFileName << std::endl;
  if (dotFileName == m_graph->dotFileName())
  {
    // if (QMessageBox::questionYesNo(this, 
    //                             "The file %1 has been modified on disk.\nDo you want to reload it?",dotFileName,
    //                             "Reload Confirmation",
    //                             KStandardGuiItem::yes(),
    //                             KStandardGuiItem::no(),
    //                             "reloadOnChangeMode"   ) == KMessageBox::Yes)
    {
#ifdef GV_LIB
      if (m_graph->useLibrary())
        loadLibrary(dotFileName);
      else
#endif
        loadDot(dotFileName);
    }
  }
}

///KConfigGroup* DotGraphView::configGroup(KConfig* c,
///                                         const QString& group, const QString& post)
///{
///  QStringList gList = c->groupList();
///  QString res = group;
///  if (gList.contains((group+post).ascii()) ) res += post;
///  return new KConfigGroup(c, res);
///}

///void DotGraphView::writeConfigEntry(KConfigGroup* c, const char* pKey,
///                                     const QString& value, const char* def)
///{
///  if (!c) return;
///  if ((value.isEmpty() && ((def == 0) || (*def == 0))) ||
///      (value == QString(def)))
///    c->deleteEntry(pKey);
///  else
///    c->writeEntry(pKey, value);
///}

///void DotGraphView::writeConfigEntry(KConfigGroup* c, const char* pKey,
///                                     int value, int def)
///{
///  if (!c) return;
///  if (value == def)
///    c->deleteEntry(pKey);
///  else
///    c->writeEntry(pKey, value);
///}

///void DotGraphView::writeConfigEntry(KConfigGroup* c, const char* pKey,
///                                     double value, double def)
///{
///  if (!c) return;
///  if (value == def)
///    c->deleteEntry(pKey);
///  else
///    c->writeEntry(pKey, value);
//}

///void DotGraphView::writeConfigEntry(KConfigGroup* c, const char* pKey,
///                                     bool value, bool def)
///{
///  if (!c) return;
///  if (value == def)
///    c->deleteEntry(pKey);
///  else
///    c->writeEntry(pKey, value);
///}

const QString& DotGraphView::dotFileName() 
{
  return m_graph->dotFileName();
}

void DotGraphView::hideToolsWindows()
{
  ///if (m_printCommand != 0)
  ///{
  ///  m_printCommand->hidePageSetup();
  ///  m_printCommand->hidePrintPreview();
  ///}
}

void DotGraphView::setupPopup()
{
  if (m_popup != 0)
  {
    return;
  }
  ///kDebug() << "DotGraphView::setupPopup";
  m_popup = new QMenu();

  ///m_layoutAlgoSelectAction = new KSelectAction("Select Layout Algo"),this);
  ///actionCollection()->addAction("view_layout_algo",m_layoutAlgoSelectAction);
  
  ///QStringList layoutAlgos;
  ///QAction* lea = new QAction(" ", this);
  ///lea->setWhatsThis("Specify yourself the layout command to use. Given a dot file, it should produce an xdot file on its standard output.");
  ///actionCollection()->addAction("layout_specifiy",lea);
  ///lea->setCheckable(false);
  
  ///QAction* lda = new QAction("Dot", this);
  ///lda->setWhatsThis("Layout the graph using the dot program.");
  ///actionCollection()->addAction("layout_dot",lda);
  ///lda->setCheckable(false);
  
  ///QAction* lna = new QAction("Neato"), this);
  ///lna->setWhatsThis("Layout the graph using the neato program.");
  ///actionCollection()->addAction("layout_neato",lna);
  ///lna->setCheckable(false);
  
  ///QAction* lta = new QAction("Twopi"), this);
  ///lta->setWhatsThis("Layout the graph using the twopi program.");
  ///actionCollection()->addAction("layout_twopi",lta);
  ///lta->setCheckable(false);
  
  ///QAction* lfa = new QAction("Fdp"), this);
  ///lfa->setWhatsThis("Layout the graph using the fdp program.");
  ///actionCollection()->addAction("layout_fdp",lfa);
  ///lfa->setCheckable(false);
  
  ///QAction* lca = new QAction("Circo"), this);
  ///lca->setWhatsThis("Layout the graph using the circo program.");
  ///actionCollection()->addAction("layout_c",lca);
  ///lca->setCheckable(false);
  
  ///m_layoutAlgoSelectAction->addAction(lea);
  ///m_layoutAlgoSelectAction->addAction(lda);
  ///m_layoutAlgoSelectAction->addAction(lna);
  ///m_layoutAlgoSelectAction->addAction(lta);
  ///m_layoutAlgoSelectAction->addAction(lfa);
  ///m_layoutAlgoSelectAction->addAction(lca);

  ///m_layoutAlgoSelectAction->setCurrentItem(1);
  ///m_layoutAlgoSelectAction->setEditable(true);
  ///m_layoutAlgoSelectAction->setToolTip("Choose a GraphViz layout algorithm or edit your own one.");  
  ///m_layoutAlgoSelectAction->setWhatsThis(i18n(
  ///"Choose a GraphViz layout algorithm or type in your own command that will "
  ///"generate a graph in the xdot format on its standard output. For example, to "
  ///"manually specify the <tt>G</tt> option to the dot command, type in: "
  ///"<tt>dot -Gname=MyGraphName -Txdot </tt>");  
  ///connect(m_layoutAlgoSelectAction, SIGNAL(triggered (const QString &)),
  ///        this, SLOT(slotSelectLayoutAlgo(const QString&)));

  
  //QMenu* layoutPopup = m_popup->addMenu("Layout");
  ///layoutPopup->addAction(m_layoutAlgoSelectAction);
  ///QAction* slc = layoutPopup->addAction("Specify layout command"), this, SLOT(slotLayoutSpecify()));
  ///slc->setWhatsThis("Specify yourself the layout command to use. Given a dot file, it should produce an xdot file on its standard output.");
  ///QAction* rlc = layoutPopup->addAction("Reset layout command to default"), this, SLOT(slotLayoutReset()));
  ///rlc->setWhatsThis("Resets the layout command to use to the default depending on the graph type (directed or not).");
  
  m_popup->addAction(QIcon("zoom-in"), "Zoom In", this, SLOT(zoomIn()));
  m_popup->addAction(QIcon("zoom-out"), "Zoom Out", this, SLOT(zoomOut()));
                        
  ///m_popup->insertSeparator();
  
  ///QActionMenu* file_exportMenu = new QActionMenu("Export Graph", this);
  ///actionCollection()->addAction("file_export",file_exportMenu);
  ///file_exportMenu->setToolTip("Allows the graph to be exported in another format.");  
  ///file_exportMenu->setWhatsThis(
  ///"Use the Export Graph menu to export the graph in another format. "
  ///"There is currently only one export format supported: as a PNG image.");  
  

  ///m_popup->addAction(file_exportMenu);
  ///QAction* exportToImageAction = new QAction("As Image..."),this);
  ///exportToImageAction->setWhatsThis("Export the graph to an image file (currently PNG only.)");
  ///actionCollection()->addAction("export_image", exportToImageAction);
  ///connect(exportToImageAction,SIGNAL(triggered(bool)), this, SLOT(slotExportImage()));
  
  ///file_exportMenu->addAction(exportToImageAction);

  
  ///m_popup->insertSeparator();

  ///m_bevEnabledAction = new KToggleAction(
  ///        KIcon(KGlobal::dirs()->findResource("data","kgraphviewerpart/pics/kgraphviewer-bev.png")),
  ///        "Enable Bird's-eye View"), this);
  ///        actionCollection()->addAction("view_bev_enabled",m_bevEnabledAction);
  ///m_bevEnabledAction->setShortcut(Qt::CTRL+Qt::Key_B);
  ///m_bevEnabledAction->setWhatsThis("Enables or disables the Bird's-eye View");
  ///connect(m_bevEnabledAction, 
  ///        SIGNAL(toggled(bool)),
  ///        this,
  ///        SLOT(slotBevToggled()));
  ///m_bevEnabledAction->setCheckable(true);
  ///m_popup->addAction(m_bevEnabledAction);
  ///
  ///m_bevPopup = new KSelectAction("Birds-eye View"), this);
  ///m_bevPopup->setWhatsThis("Allows the Bird's-eye View to be setup.");
  ///m_popup->addAction(m_bevPopup);
  ///actionCollection()->addAction("view_bev",m_bevPopup);

  ///QAction* btla = new QAction("Top Left"), this);
  ///btla->setWhatsThis("Puts the Bird's-eye View in the top-left corner.");
  ///btla->setCheckable(true);
  ///actionCollection()->addAction("bev_top_left",btla);
  ///connect(btla, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)), 
  ///        this, SLOT(slotBevTopLeft()));
  ///QAction* btra = new QAction("Top Right"), this);
  ///btra->setWhatsThis("Puts the Bird's-eye View in the top-right corner.");
  ///btra->setCheckable(true);
  ///actionCollection()->addAction("bev_top_right",btra);
  ///connect(btra, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)), 
  ///        this, SLOT(slotBevTopRight()));
  ///QAction* bbla = new QAction("Bottom Left"), this);
  ///bbla->setWhatsThis("Puts the Bird's-eye View in the bottom-left corner.");
  ///bbla->setCheckable(true);
  ///actionCollection()->addAction("bev_bottom_left",bbla);
  ///connect(bbla, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)), 
  ///        this, SLOT(slotBevBottomLeft()));
  ///QAction* bbra = new QAction("Bottom Right"), this);
  ///bbra->setWhatsThis("Puts the Bird's-eye View in the bottom-right corner.");
  ///bbra->setCheckable(true);
  ///actionCollection()->addAction("bev_bottom_right",bbra);
  ///connect(bbra, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)), 
  ///        this, SLOT(slotBevBottomRight()));
  ///QAction* bba = new QAction("Automatic"), this);
  ///bba->setWhatsThis("Let KGraphViewer automatically choose the position of the Bird's-eye View.");
  ///bba->setCheckable(true);
  ///actionCollection()->addAction("bev_automatic",bba);
  ///connect(bba, SIGNAL(triggered (Qt::MouseButtons, Qt::KeyboardModifiers)), 
  ///        this, SLOT(slotBevAutomatic()));
  ///m_bevPopup->addAction(btla);
  ///m_bevPopup->addAction(btra);
  ///m_bevPopup->addAction(bbla);
  ///m_bevPopup->addAction(bbra);
  ///m_bevPopup->addAction(bba);
  ///switch (m_zoomPosition)
  ///{
  ///  case KGraphViewerInterface::TopLeft:
  ///    btla->setChecked(true);
  ///  break;
  ///  case KGraphViewerInterface::TopRight:
  ///    btra->setChecked(true);
  ///  break;
  ///  case KGraphViewerInterface::BottomLeft:
  ///    bbla->setChecked(true);
  ///  break;
  ///  case KGraphViewerInterface::BottomRight:
  ///    bbra->setChecked(true);
  ///  break;
  ///  case KGraphViewerInterface::Auto:
  ///    bba->setChecked(true);
  ///  break;
  ///}


  ///kDebug() << "    m_bevEnabledAction setting checked to : " << KGraphViewerPartSettings::birdsEyeViewEnabled();
  ///m_bevEnabledAction->setChecked(KGraphViewerPartSettings::birdsEyeViewEnabled());
  ///m_bevPopup->setEnabled(KGraphViewerPartSettings::birdsEyeViewEnabled());
}

void DotGraphView::exportToImage()
{
  // write current content of canvas as image to file
  if (!m_canvas) return;

  QString fn = QFileDialog::getSaveFileName(NULL, QString("Save File"), QString(""), QString("Images (*.png)"));
  
  if (!fn.isEmpty()) 
  {
    QPixmap pix(m_canvas->sceneRect().size().toSize());
    QPainter p(&pix);
    m_canvas->render( &p );
    pix.save(fn,"PNG");
  }
}

void DotGraphView::slotExportImage()
{
  exportToImage();
}

void DotGraphView::slotLayoutSpecify()
{
  {
    bool ok = false;
    QString currentLayoutCommand = m_graph->layoutCommand();
    QString layoutCommand = 
    QInputDialog::getText(
      this, 			  
      "Layout Command", 
      "Type in a layout command for the current graph:",
      QLineEdit::Normal,
      QString(),
      &ok
    );
    //       std::cerr << "Got layout command: " << layoutCommand << std::endl;
    if (ok && layoutCommand != currentLayoutCommand)
    {
      //         std::cerr << "Setting new layout command: " << layoutCommand << std::endl;
      setLayoutCommand(layoutCommand);
    }
  }
}

void DotGraphView::slotLayoutReset()
{
  setLayoutCommand("");
}

void DotGraphView::slotSelectLayoutAlgo(const QString& ttext)
{
QString text = ttext;//.mid(1);
  ///kDebug() << "DotGraphView::slotSelectLayoutAlgo '" << text << "'";
  if (text == "Dot")
  {
    setLayoutCommand("dot");
  }
  else if (text == "Neato")
  {
    setLayoutCommand("neato");
  }
  else if (text == "Twopi")
  {
    setLayoutCommand("twopi");
  }
  else if (text == "Fdp")
  {
    setLayoutCommand("fdp");
  }
  else if (text == "Circo")
  {
    setLayoutCommand("circo");
  }
  else 
  {
    setLayoutCommand(text);
  }
}

void DotGraphView::slotSelectLayoutDot()
{
  ///kDebug() << "DotGraphView::slotSelectLayoutDot";
  setLayoutCommand("dot -Txdot");
}

void DotGraphView::slotSelectLayoutNeato()
{
  ///kDebug() << "DotGraphView::slotSelectLayoutNeato";
  setLayoutCommand("neato -Txdot");
}

void DotGraphView::slotSelectLayoutTwopi()
{
  ///kDebug() << "DotGraphView::slotSelectLayoutTwopi";
  setLayoutCommand("twopi -Txdot");
}

void DotGraphView::slotSelectLayoutFdp()
{
  ///kDebug() << "DotGraphView::slotSelectLayoutFdp";
  setLayoutCommand("fdp -Txdot");
}

void DotGraphView::slotSelectLayoutCirco()
{
  ///kDebug() << "DotGraphView::slotSelectLayoutCirco";
  setLayoutCommand("circo -Txdot");
}

void DotGraphView::slotBevToggled()
{
  ///kDebug() << "DotGraphView::slotBevToggled";
  ///kDebug() << "    m_bevEnabledAction is checked ? " << m_bevEnabledAction->isChecked();
  ///setPannerEnabled(m_bevEnabledAction->isChecked());
}

void DotGraphView::slotBevTopLeft()
{
  viewBevActivated(KGraphViewerInterface::TopLeft);
}

void DotGraphView::slotBevTopRight()
{
  viewBevActivated(KGraphViewerInterface::TopRight);
}

void DotGraphView::slotBevBottomLeft()
{
  viewBevActivated(KGraphViewerInterface::BottomLeft);
}

void DotGraphView::slotBevBottomRight()
{
  viewBevActivated(KGraphViewerInterface::BottomRight); 
}

void DotGraphView::slotBevAutomatic()
{
  viewBevActivated(KGraphViewerInterface::Auto);
}

void DotGraphView::slotUpdate()
{
  ///kDebug();
  m_graph->update();
}

void DotGraphView::prepareAddNewElement(QMap<QString,QString> attribs)
{
  ///kDebug() ;
  m_editingMode = AddNewElement;
  m_newElementAttributes = attribs;
  unsetCursor();
  setCursor(QCursor(m_defaultNewElementPixmap));
}

void DotGraphView::prepareAddNewEdge(QMap<QString,QString> attribs)
{
  ///kDebug() << attribs;
  bool anySelected = false;
  foreach (GraphEdge* edge, m_graph->edges())
  {
    if (edge->isSelected())
    {
      anySelected = true;
      foreach(const QString& k, attribs.keys())
      {
        edge->attributes()[k] = attribs[k];
      }
    }
  }
  if (anySelected)
  {
    return;
  }
  m_editingMode = AddNewEdge;
  m_newElementAttributes = attribs;
  unsetCursor();
  QBitmap bm("kgraphviewerpart/pics/kgraphviewer-newedge.png");
  setCursor(QCursor(bm,bm,32,16));
}

void DotGraphView::prepareSelectElements()
{
  ///kDebug();
  m_editingMode = SelectingElements;
  setCursor(Qt::CrossCursor);
  setDragMode ( RubberBandDrag );
}

void DotGraphView::prepareSelectSinlgeElement()
{
  ///kDebug();
  m_editingMode = SelectSingleElement;
  setCursor(Qt::OpenHandCursor);
}

void DotGraphView::createNewEdgeDraftFrom(CanvasElement* node)
{
  ///kDebug() << node->element()->id();
  m_editingMode = DrawNewEdge;
  unsetCursor();
  m_newEdgeSource = node;

  if (m_newEdgeDraft != 0)
  {
    ///kDebug() << "removing new edge draft";
    m_newEdgeDraft->hide();
    scene()->removeItem(m_newEdgeDraft);
    delete m_newEdgeDraft;
    m_newEdgeDraft = 0;
  }
  m_newEdgeDraft = new QGraphicsLineItem(QLineF(node->boundingRect().center()+node->pos(),node->boundingRect().center()+node->pos()+QPointF(10,10)));
  scene()->addItem(m_newEdgeDraft);
  m_newEdgeDraft->setZValue(1000);
  m_newEdgeDraft->show();
  ///kDebug() << m_newEdgeDraft->line();
}

void DotGraphView::finishNewEdgeTo(CanvasElement* node)
{
  ///kDebug() << node->element()->id();
  m_editingMode = None;
  unsetCursor();

  if (m_newEdgeDraft != 0)
  {
    ///kDebug() << "removing new edge draft";
    m_newEdgeDraft->hide();
    scene()->removeItem(m_newEdgeDraft);
    delete m_newEdgeDraft;
    m_newEdgeDraft = 0;
  }

  emit newEdgeFinished(m_newEdgeSource->element()->id(),node->element()->id(),m_newElementAttributes);

  m_newEdgeSource = 0;
}

// void DotGraphView::slotFinishNewEdge(
//       const QString& srcId,
//       const QString& tgtId,
//       const QMap<QString, QString> newElementAttributes)
// {
//   kDebug() ;
// 
//   GraphEdge* gedge  = new GraphEdge();
//   gedge->setFromNode(m_graph->nodes()[srcId]);
//   gedge->setToNode(m_graph->nodes()[tgtId]);
//   gedge->attributes() = newElementAttributes;
//   foreach (const QString &attrib, newElementAttributes.keys())
//   {
//     if (attrib == "z")
//     {
//       bool ok;
//       gedge->setZ(newElementAttributes[attrib].toDouble(&ok));
//     }
//   }
//   gedge->setId(srcId+tgtId+QString::number(m_graph->edges().size()));
//   m_graph->edges().insert(gedge->id(), gedge);
// 
//   double scaleX = 1.0, scaleY = 1.0;
// 
//   if (m_detailLevel == 0)      { scaleX = m_graph->scale() * 0.7; scaleY = m_graph->scale() * 0.7; }
//   else if (m_detailLevel == 1) { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
//   else if (m_detailLevel == 2) { scaleX = m_graph->scale() * 1.3; scaleY = m_graph->scale() * 1.3; }
//   else                        { scaleX = m_graph->scale() * 1.0; scaleY = m_graph->scale() * 1.0; }
// 
//   qreal gh = m_graph->height();
//   CanvasEdge* cedge = new CanvasEdge(this, gedge, scaleX, scaleY, m_xMargin,
//         m_yMargin, gh, m_graph->wdhcf(), m_graph->hdvcf());
// 
//   gedge->setCanvasEdge(cedge);
// //     std::cerr << "setting z = " << gedge->z() << std::endl;
//   cedge->setZValue(gedge->z());
//   cedge->show();
//   scene()->addItem(cedge);
// 
//   emit newEdgeAdded(gedge->fromNode()->id(),gedge->toNode()->id());
// }

void DotGraphView::setReadOnly()
{
  ///kDebug() ;
  m_readWrite = false;
  if (m_graph != 0)
  {
    m_graph->setReadOnly();
  }
}

void DotGraphView::setReadWrite()
{
  ///kDebug() ;
  m_readWrite = true;
  if (m_graph != 0)
  {
    m_graph->setReadWrite();
  }
}

void DotGraphView::slotEdgeSelected(CanvasEdge* edge, Qt::KeyboardModifiers modifiers)
{
  ///kDebug() << edge->edge()->id();
  QList<QString> selection;
  selection.push_back(edge->edge()->id());
  if (!modifiers.testFlag(Qt::ControlModifier))
  {
    foreach(GraphEdge* e, m_graph->edges())
    {
      if (e->canvasEdge() != edge)
      {
        e->setSelected(false);
        e->canvasEdge()->update();
      }
    }
    foreach(GraphNode* n, m_graph->nodes())
    {
      n->setSelected(false);
      n->canvasNode()->update();
    }
    foreach(GraphSubgraph* s, m_graph->subgraphs())
    {
      s->setElementSelected(0, false, true);
    }
  }
  else
  {
    foreach(GraphEdge* e, m_graph->edges())
    {
      if (e->canvasEdge() != edge)
      {
        if (e->isSelected())
        {
          selection.push_back(e->id());
        }
      }
    }
    foreach(GraphNode* n, m_graph->nodes())
    {
      if (n->isSelected())
      {
        selection.push_back(n->id());
      }
    }
    foreach(GraphSubgraph* s, m_graph->subgraphs())
    {
      if (s->isSelected())
      {
        selection.push_back(s->id());
      }
    }
  }
  emit selectionIs(selection, QPoint());
}

void DotGraphView::slotElementSelected(CanvasElement* element, Qt::KeyboardModifiers modifiers)
{
  ///kDebug();
  QList<QString> selection;
  selection.push_back(element->element()->id());
  if (!modifiers.testFlag(Qt::ControlModifier))
  {
    foreach(GraphEdge* e, m_graph->edges())
    {
      if (e->isSelected()) {
        e->setSelected(false);
        e->canvasEdge()->update();
      }
    }
    foreach(GraphNode* e, m_graph->nodes())
    {
      if (e->canvasElement() != element)
      {
        if (e->isSelected()) {
          e->setSelected(false);
          e->canvasElement()->update();
        }
      }
    }
    foreach(GraphSubgraph* s, m_graph->subgraphs())
    {
      s->setElementSelected(element->element(), true, true);
    }
  }
  else
  {
    foreach(GraphEdge* e, m_graph->edges())
    {
      if (e->isSelected())
      {
        selection.push_back(e->id());
      }
    }
    foreach(GraphNode* n, m_graph->nodes())
    {
      if (n->isSelected())
      {
        selection.push_back(n->id());
      }
    }
    foreach(GraphSubgraph* s, m_graph->subgraphs())
    {
      s->retrieveSelectedElementsIds(selection);
    }
  }
  emit selectionIs(selection, QPoint());
}

void DotGraphView::removeSelectedEdges()
{
  ///kDebug();
  foreach(GraphEdge* e, m_graph->edges())
  {
    if (e->isSelected())
    {
      ///kDebug() << "emiting removeEdge " << e->id();
      emit removeEdge(e->id());
    }
  }
}

void DotGraphView::removeSelectedNodes()
{
  ///kDebug();
  foreach(GraphNode* e, m_graph->nodes())
  {
    if (e->isSelected())
    {
      ///kDebug() << "emiting removeElement " << e->id();
      emit removeElement(e->id());
    }
  }
}

void DotGraphView::removeSelectedSubgraphs()
{
  ///kDebug();
  foreach(GraphSubgraph* e, m_graph->subgraphs())
  {
    if (e->isSelected())
    {
      ///kDebug() << "emiting removeElement " << e->id();
      emit removeElement(e->id());
    }
  }
}

void DotGraphView::removeSelectedElements()
{
  ///kDebug();
  removeSelectedNodes();
  removeSelectedEdges();
  removeSelectedSubgraphs();
}

void DotGraphView::timerEvent ( QTimerEvent * event )
{
  ///kDebug() << event->timerId();
  qreal vpercent = verticalScrollBar()->value()*1.0/100;
  qreal hpercent = horizontalScrollBar()->value()*1.0/100;
  if (m_scrollDirection == Left)
  {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value()-(5*hpercent));
  }
  else if (m_scrollDirection == Right)
  {
    horizontalScrollBar()->setValue(horizontalScrollBar()->value()+(5*hpercent));
  }
  else if (m_scrollDirection == Top)
  {
    verticalScrollBar()->setValue(verticalScrollBar()->value()-(5*vpercent));
  }
  else if (m_scrollDirection == Bottom)
  {
    verticalScrollBar()->setValue(verticalScrollBar()->value()+(5*vpercent));
  }
}

void DotGraphView::leaveEvent ( QEvent * /*event*/ )
{
  ///kDebug() << mapFromGlobal(QCursor::pos());
  if (m_editingMode == DrawNewEdge)
  {
    m_leavedTimer = startTimer(10);
    if (mapFromGlobal(QCursor::pos()).x() <= 0)
    {
      m_scrollDirection = Left;
    }
    else if (mapFromGlobal(QCursor::pos()).y() <= 0)
    {
      m_scrollDirection = Top;
    }
    else if (mapFromGlobal(QCursor::pos()).x() >= width())
    {
      m_scrollDirection = Right;
    }
    else if (mapFromGlobal(QCursor::pos()).y() >= height())
    {
      m_scrollDirection = Bottom;
    }
  }
}

void DotGraphView::enterEvent ( QEvent * /*event*/ )
{
  ///kDebug();
  if (m_leavedTimer != std::numeric_limits<int>::max())
  {
    killTimer(m_leavedTimer);
    m_leavedTimer = std::numeric_limits<int>::max();
  }
}

#ifdef GV_LIB
void DotGraphView::slotAGraphReadFinished()
{
  QString layoutCommand = (m_graph!=0?m_graph->layoutCommand():"");
  if (layoutCommand.isEmpty())
  {
    if (!m_loadThread.dotFileName().isEmpty())
      layoutCommand = m_graph->chooseLayoutProgramForFile(m_loadThread.dotFileName());
    else
      layoutCommand = "dot";
  }
  m_layoutThread.layoutGraph(m_loadThread.g(), layoutCommand);
}

void DotGraphView::slotAGraphLayoutFinished()
{
  bool result = loadLibrary(m_layoutThread.g(), m_layoutThread.layoutCommand());
  if (result)
    m_graph->dotFileName(m_loadThread.dotFileName());

  gvFreeLayout(m_layoutThread.gvc(), m_layoutThread.g());
  agclose(m_layoutThread.g());
  bool freeresult = (gvFreeContext(m_layoutThread.gvc()) == 0);
}
#endif

///#include "dotgraphview.moc"

