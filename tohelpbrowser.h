//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __TOHELPBROWSER_H
#define __TOHELPBROWSER_H

#include <list>
#include <khtml_part.h>

class KURL;

/**
 * Widget used to display HTML of help pages.
 */

class toHelpBrowser : public KHTMLPart {
  Q_OBJECT

  /** List of backward history. The current page is at the end of the list.
   */
  std::list<QString> Backward;
  /** List of forward history if you have started walking backwards.
   */
  std::list<QString> Forward;
private slots:
  /** Reimplemented for internal reasons.
   */
  void openURLRequest(const KURL &url,const KParts::URLArgs &);
public:
  /** Create a help browser.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toHelpBrowser(QWidget *parent,const char *name=NULL);
  /** Open an URL in this widget.
   * @param url URL to open.
   * @return Success of operator.
   */
  virtual bool openURL(const KURL &url);
  /** Get source of URL.
   * @return String containing the current localtion.
   */
  QString source(void);
public slots:
  /** Go forward if available.
   */
  void forward(void);
  /** Go backward if available.
   */
  void backward(void);
signals:
  /** Change availability of backward history.
   * @param avail Wether history is available.
   */
  void backwardAvailable(bool avail);
  /** Change availability of forward history.
   * @param avail Wether history is available.
   */
  void forwardAvailable(bool avail);
  /** The contents of the browser changed.
   */
  void textChanged(void);
};

#endif
