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

#ifndef __TOCHANGECONNECTION_H
#define __TOCHANGECONNECTION_H

#include <qtoolbutton.h>

/**
 * This button will allow you to change the connection for the nearest toToolWidget
 * parent.
 */

class toChangeConnection : public QToolButton {
  Q_OBJECT
private slots:
  void changeConnection(int id);
  void popupMenu(void);
public:
  /** Create the button, only the toolbar parent is needed.
   * @param parent Toolbar to put connection in.
   * @param name Name of widget.
   */
  toChangeConnection(QToolBar *parent,const char *name=0);
};

#endif
