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

#ifndef __TONEWCONNECTION_H
#define __TONEWCONNECTION_H

#include "tonewconnectionui.h"
#include "tohelp.h"

class toNewConnection : public toNewConnectionUI, public toHelpContext
{
  Q_OBJECT
public:
  toNewConnection(QWidget* parent=0,const char* name=0,bool modal=false,WFlags fl=0);

  toConnection *makeConnection();
public slots:
  virtual void historySelection(void);
  virtual void changeProvider(void); 
  virtual void changeHost(void); 
};

#endif // TONEWCONNECTION_H
