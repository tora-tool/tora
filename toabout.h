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

#ifndef __TOABOUT_H
#define __TOABOUT_H

#include <qvbox.h>

#include "toaboutui.h"

class QPushButton;
class QTextView;
class QProgressBar;
class QLabel;

class toSplash : public QVBox {
  QProgressBar *Progress;
  QLabel *Label;
public:
  toSplash(QWidget *parent=0,const char *name=0,WFlags f=0);
  QLabel *label(void)
  { return Label; }
  QProgressBar *progress(void)
  { return Progress; }
};

class toAbout : public toAboutUI {
  Q_OBJECT

public:
  toAbout(int page,QWidget* parent=0,const char* name=0,bool modal=false,WFlags fl=0);
};

#endif
