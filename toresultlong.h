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

#ifndef __TORESULTLONG_H
#define __TORESULTLONG_H

#include <qlistview.h>
#include <qtimer.h>
#include "toconnection.h"
#include "toresultview.h"
#include "otlv32.h"

class QListViewItem;
class toresultlong;
class toResultTip;
class toNoBlockQuery;
class toResultStats;

class toResultLong : public toResultView {
  Q_OBJECT

protected:
  toNoBlockQuery *Query;
  QTimer Timer;
  bool HasHeaders;
  bool First;
  toResultStats *Statistics;

  virtual bool eof(void);
public:
  toResultLong(bool readable,bool numCol,toConnection &conn,QWidget *parent,const char *name=NULL);
  toResultLong(toConnection &conn,QWidget *parent,const char *name=NULL);
  ~toResultLong();

  void setStatistics(toResultStats *stats)
  { Statistics=stats; }

  bool running(void)
  { return Query; }

  virtual void query(const QString &sql,const list<QString> &param);
  void query(const QString &sql)
  { list<QString> p; query(sql,p); }

signals:
  void done(void);
  void firstResult(const QString &sql,const QString &res);

public slots:
  void stop(void);
  virtual void addItem(void);
};

#endif
