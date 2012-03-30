// interface.h
// Copyright (C)  2002  Dominique Devriese <devriese@kde.org>
 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
 
#ifndef KDELIBS_KDE_TERMINAL_INTERFACE_H
#define KDELIBS_KDE_TERMINAL_INTERFACE_H
 
#include <QtCore/QObject>
 
class QStringList;
 
/**
 * KGraphViewerInterface is an interface implemented by KGraphViewer to
 * allow developers access to the KGraphViewerPart in ways that are not
 * possible through the normal KPart interface.
 *
 * Note that besides the functions below here, KGraphViewer also has
 * some signals you can connect to.  They aren't in this class cause
 * we can't have signals without having a QObject, which
 * KGraphViewerInterface is not. To see a list of signals, take a look at kgraphviewer_part.h
 * 
 * See the example code below for how to connect to these..
 *
 * Use it like this:
 * \code
 *  // fetch the Part..
 *  KLibFactory *factory = KLibLoader::self()->factory("kgraphviewerpart");
 *  if (factory) {
 *      part = factory->create<KParts::ReadOnlyPart>(this);
 *
 *      // cast the part to the KGraphViewerInterface...
 *      KGraphViewerInterface* graph = qobject_cast<KGraphViewerInterface*>( part );
 *      if( ! graph )
 *      {
 *        // This should not happen
 *        return;
 *      }
 *      // now use the interface in all sorts of ways...
 *  }
 * \endcode
 *
 * @author Milian Wolff <mail@milianw.de>
 * 
 * 
 * WARNING: This is highly experimental and no kind of BC guarantees are given!
 * TODO: documentation
 */
class KGraphViewerInterface
{
public:
  enum LayoutMethod
  {
    ExternalProgram,
    InternalLibrary
  };
  virtual ~KGraphViewerInterface(){}
 
  virtual void setLayoutMethod(LayoutMethod method) = 0;
  virtual void zoomIn() = 0;
  virtual void zoomOut() = 0;
  virtual void zoomBy(double factor) = 0;
  virtual void setZoomFactor(double factor) = 0;
 
  enum PannerPosition { TopLeft, TopRight, BottomLeft, BottomRight, Auto };
  virtual void setPannerPosition(PannerPosition position) = 0;
  virtual void setPannerEnabled(bool enabled) = 0;
 
  virtual void setLayoutCommand(const QString& command) = 0;
 
  virtual void selectNode(const QString& nodeId) = 0;
  virtual void centerOnNode(const QString& nodeId) = 0;
};
 
Q_DECLARE_INTERFACE(KGraphViewerInterface, "org.kde.KGraphViewerInterface")
 
#endif
 