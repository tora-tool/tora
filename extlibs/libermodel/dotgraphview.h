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

#ifndef DOTGRAPHVIEW_H
#define DOTGRAPHVIEW_H

///#include <kconfig.h>
///#include <kactioncollection.h>
///#include <kconfiggroup.h>

#include <QGraphicsView>
#include <QSet>
#include <QMap>

#include "graphexporter.h"
#include "kgraphviewer_interface.h"
#include "loadagraphthread.h"
#include "layoutagraphthread.h"

class GraphElement;
class GraphSubgraph;
class CanvasElement;
class CanvasEdge;
class PannerView;
class DotGraph;
///class KGVSimplePrintingCommand;
///class KSelectAction;
///class KToggleAction;

class QMenu;
class QKeyEvent;
class QMouseEvent;
class QFocusEvent;
class QResizeEvent;
class QWheelEvent;
class QContextMenuEvent;
class QWidget;
class QActionGroup;

#define DEFAULT_DETAILLEVEL 1
/**
 * A CanvasView showing a part of the call graph
 * and another zoomed out CanvasView in a border acting as
 * a panner to select to visible part (only if needed)
 */
class DotGraphView: public QGraphicsView
{
 Q_OBJECT

public:
  enum EditingMode { None, AddNewElement, AddNewEdge, DrawNewEdge, SelectingElements, SelectSingleElement };
  enum ScrollDirection { Here, Left, Right, Top, Bottom };
  
  explicit DotGraphView(QActionGroup* actions, QWidget* parent=0);
  virtual ~DotGraphView();

  bool loadDot(const QString& dotFileName);
  bool loadLibrary(const QString& dotFileName);
  bool loadLibrary(graph_t* graph, const QString& layoutCommand = "dot");

  void readViewConfig();
  void saveViewConfig();

  QWidget* widget() { return this; }

  //TODO: rename zoomPos -> bev / panner, but _please_ make it consistent...
  KGraphViewerInterface::PannerPosition zoomPos() const { return m_zoomPosition; }
  static KGraphViewerInterface::PannerPosition zoomPos(const QString&);
  static QString zoomPosString(KGraphViewerInterface::PannerPosition);
  void setPannerEnabled(bool enabled);
  
  ///static KConfigGroup* configGroup(KConfig*, const QString& prefix, const QString& postfix);
  ///static void writeConfigEntry(KConfigGroup*, const char* pKey, const QString& value,
  ///                             const char* def);
  ///static void writeConfigEntry(KConfigGroup*, const char* pKey,
  ///                             int value, int def);
  ///static void writeConfigEntry(KConfigGroup*, const char* pKey,
  ///                             bool value, bool def);
  ///static void writeConfigEntry(KConfigGroup*, const char* pKey,
  ///                             double value, double def);

  virtual void wheelEvent(QWheelEvent* e);

  /// multiplies current zoom factor with @p factor
  void applyZoom(double factor);
  /// sets zoom factor to @p factor
  void setZoomFactor(double factor);
  
  void setLayoutCommand(const QString& command);
    
  const QString& dotFileName();

  void hideToolsWindows();
  inline double zoom() const {return m_zoom;}
  ///inline KSelectAction* bevPopup() {return m_bevPopup;}

  inline DotGraph* graph() {return m_graph;}
  inline const DotGraph* graph() const {return m_graph;}

  inline const GraphElement* defaultNewElement() const {return m_defaultNewElement;}
  inline QPixmap defaultNewElementPixmap() const {return m_defaultNewElementPixmap;}

  inline void setDefaultNewElement(GraphElement* elem) {m_defaultNewElement = elem;}
  inline void setDefaultNewElementPixmap(const QPixmap& pm) {m_defaultNewElementPixmap = pm;}

  void prepareAddNewElement(QMap<QString,QString> attribs);
  void prepareAddNewEdge(QMap<QString,QString> attribs);
  void prepareSelectElements();
  void prepareSelectSinlgeElement();
  
  void createNewEdgeDraftFrom(CanvasElement* node);
  void finishNewEdgeTo(CanvasElement* node);

  EditingMode editingMode() const {return m_editingMode;}

  void setReadOnly();
  void setReadWrite();
  inline bool isReadWrite() {return m_readWrite;}
  inline bool isReadOnly() {return !m_readWrite;}
  
  void removeSelectedNodes();
  void removeSelectedEdges();
  void removeSelectedSubgraphs();
  void removeSelectedElements();
  
  inline bool highlighting() {return m_highlighting;}
  inline void setHighlighting(bool highlightingValue) {m_highlighting = highlightingValue;}

  // public so that the panner view can bubble through
  void contextMenuEvent(QContextMenuEvent*);

Q_SIGNALS:
  void zoomed(double factor);
  void sigViewBevEnabledToggled(bool value);
  void sigViewBevActivated(int newPos);
  void graphLoaded();
  void newNodeAdded(const QString&);
  void newEdgeAdded(const QString&, const QString&);
  /** signals that the user has activated a remove edge command */
  void removeEdge(const QString&);
  /** signals that the user has activated a remove edge command */
  void removeNodeNamed(const QString&);
  /** signals that the user has activated a remove element command */
  void removeElement(const QString&);
  /** signals the content of the new selection */
  void selectionIs(const QList<QString>, const QPoint&);
  /** let the application tweak the created edge if necessary */
  void newEdgeFinished(
      const QString&, const QString&,
      const QMap<QString, QString>&);
  void contextMenuEvent(const QString&, const QPoint&);
  void hoverEnter(const QString&);
  void hoverLeave(const QString&);
  void selected(const QString&);
  
public Q_SLOTS:
  void zoomIn();
  void zoomOut();  
  void zoomRectMovedTo(QPointF newZoomPos);
  void zoomRectMoveFinished();
  bool initEmpty();
  bool reload();
  void dirty(const QString& dotFileName);
  void pageSetup();
  void print();
  void printPreview();
  void viewBevActivated(int newPos);
  void slotExportImage();
  void slotSelectLayoutAlgo(const QString& text);
  void slotLayoutSpecify();
  void slotLayoutReset();
  void slotSelectLayoutDot();
  void slotSelectLayoutNeato();
  void slotSelectLayoutTwopi();
  void slotSelectLayoutFdp();
  void slotSelectLayoutCirco();
  void slotBevToggled();
  void slotBevTopLeft();
  void slotBevTopRight();
  void slotBevBottomLeft();
  void slotBevBottomRight();
  void slotBevAutomatic();
  void slotUpdate();
  bool displayGraph();
  void slotEdgeSelected(CanvasEdge*, Qt::KeyboardModifiers);
  void slotElementSelected(CanvasElement*, Qt::KeyboardModifiers);
  void slotSelectionChanged();
  void slotContextMenuEvent(const QString&, const QPoint&);
  void slotElementHoverEnter(CanvasElement*);
  void slotElementHoverLeave(CanvasElement*);
  void slotElementHoverEnter(CanvasEdge*);
  void slotElementHoverLeave(CanvasEdge*);

private Q_SLOTS:
  void slotAGraphReadFinished();
  void slotAGraphLayoutFinished();
  
protected:
  void scrollContentsBy(int dx, int dy);
  void resizeEvent(QResizeEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);
  void keyPressEvent(QKeyEvent*);
  void focusInEvent(QFocusEvent*);
  void focusOutEvent(QFocusEvent*);

  void timerEvent ( QTimerEvent * event );
  void leaveEvent ( QEvent * event );
  void enterEvent ( QEvent * event );
  
  
private:
  void updateSizes(QSizeF s = QSizeF(0,0));
  void updateBirdEyeView();
  void setupPopup();
  void exportToImage();
  QActionGroup* actionCollection() {return m_actions;}
  int displaySubgraph(GraphSubgraph* gsubgraph, int zValue, CanvasElement* parent = 0);
  
  
  QSet<QGraphicsSimpleTextItem*> m_labelViews;
  QGraphicsScene* m_canvas;
  QMenu* m_popup;
  ///KSelectAction* m_bevPopup;
  ///KSelectAction* m_layoutAlgoSelectAction;
  int m_xMargin, m_yMargin;
  PannerView *m_birdEyeView;
  double m_cvZoom;
  double m_zoom;
  bool m_isMoving;
  QPoint m_lastPos;

  GraphExporter m_exporter;

  // widget options
  KGraphViewerInterface::PannerPosition m_zoomPosition, m_lastAutoPosition;
  
  DotGraph* m_graph;
  
  ///KGVSimplePrintingCommand* m_printCommand;
  
  ///KToggleAction* m_bevEnabledAction;
  QActionGroup* m_actions;

  int m_detailLevel;

  GraphElement* m_defaultNewElement;
  
  /** image used for a new node just added in an edited graph because this new node has
  still no attribute and thus no render operation */
  QPixmap m_defaultNewElementPixmap;
  EditingMode m_editingMode;

  CanvasElement* m_newEdgeSource;
  QGraphicsLineItem* m_newEdgeDraft;

  bool m_readWrite;

  QMap<QString, QString> m_newElementAttributes;

  /// identifier of the timer started when the mouse leaves the view during
  /// edge drawing
  int m_leavedTimer;

  ScrollDirection m_scrollDirection;

  QPoint m_pressPos;
  QPoint m_pressScrollBarsPos;

  /// true if elements should be highlighted on hover; false otherwise
  bool m_highlighting;

  /// A thread to load graphviz agraph files
  LoadAGraphThread m_loadThread;
  
  /// A thread to layout graphviz agraph files
  LayoutAGraphThread m_layoutThread;
};

#endif // DOTGRAPHVIEW_H
