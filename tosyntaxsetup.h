/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/


#ifndef __TOSYNTAXSETUP_H
#define __TOSYNTAXSETUP_H

#include <map>
#include "totool.h"
#include "tosyntaxsetup.ui.h"
#include "tohighlightedtext.h"

class QFont;

class toSyntaxSetup : public toSyntaxSetupUI, public toSettingTab { 
  QString Text;
  QListBoxItem *Current;
  map<QString,QColor> Colors;
  toSyntaxAnalyzer Analyzer;
public:
  toSyntaxSetup(QWidget *parent=0,const char *name=0,WFlags fl=0);
  virtual void saveSetting(void);
  void checkFixedWidth(const QFont &fnt);
public slots:
  virtual void changeLine(QListBoxItem *);
  virtual void selectColor(void);
  virtual void selectFont(void);
  virtual void changeUpper(bool val)
  { Example->setKeywordUpper(val); Example->repaint(); }
  virtual void changeHighlight(bool val)
  { Example->setHighlight(val); Example->repaint(); }
};

#endif
