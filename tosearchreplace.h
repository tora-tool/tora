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
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef __TOSEARCHREPLACE_H
#define __TOSEARCHREPLACE_H

#include "tosearchreplaceui.h"

class toMarkedText;

class toSearchReplace : public toSearchReplaceUI {
  toMarkedText *Text;

  int findIndex(const QString &str,int line,int col);
  void findPosition(const QString &str,int index,int &line,int &col);

public:
  toSearchReplace(QWidget *parent);

  void setTarget(toMarkedText *parent);
  bool searchNextAvailable()
  { return SearchNext->isEnabled(); }
public slots:
  virtual void destroyed();
  virtual void replaceAll();
  virtual void replace();
  virtual void search();
  virtual void searchNext();
  virtual void searchChanged();
};

#endif
