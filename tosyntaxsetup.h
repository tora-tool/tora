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

#ifndef __TOSYNTAXSETUP_H
#define __TOSYNTAXSETUP_H

#include <map>
#include "totool.h"
#include "tosyntaxsetupui.h"
#include "tohighlightedtext.h"
#include "tohelp.h"

class QFont;

class toSyntaxSetup : public toSyntaxSetupUI, public toSettingTab { 
  QString Text;
  QString List;
  QListBoxItem *Current;
  std::map<QString,QColor> Colors;
  toSyntaxAnalyzer Analyzer;
public:
  toSyntaxSetup(QWidget *parent=0,const char *name=0,WFlags fl=0);
  virtual void saveSetting(void);
  void checkFixedWidth(const QFont &fnt);
public slots:
  virtual void changeLine(QListBoxItem *);
  virtual void selectColor(void);
  virtual void selectFont(void);
  virtual void selectResultFont(void);
  virtual void changeUpper(bool val)
  { Example->setKeywordUpper(val); Example->update(); }
  virtual void changeHighlight(bool val)
  { Example->setHighlight(val); Example->update(); }
};

#endif
