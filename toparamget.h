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

#ifndef __TOPARAMGET_H
#define __TOPARAMGET_H

#include <list>
#include <map>

#include <qdialog.h>
#include <qpushbutton.h>

#include "toconnection.h"

class QLineEdit;
class QGrid;

class toParamGetButton : public QPushButton {
  Q_OBJECT
  int Row;
public:
  toParamGetButton(int row,QWidget *parent=0,const char *name=0)
    : QPushButton(parent,name),Row(row)
  { connect(this,SIGNAL(clicked()),this,SLOT(internalClicked())); }
private slots:
  void internalClicked(void)
  { emit clicked(Row); }
signals:
  void clicked(int);
};

/** A help class to parse SQL for input/output bindings and if available also pop up a 
 * dialog and ask for values. Also maintains an internal cache of old values.
 */

class toParamGet : public QDialog {
  Q_OBJECT
  /** Default values cache
   */
  static std::map<QString,QString> DefaultCache;
  /** Specified values cache
   */
  static std::map<QString,QString> Cache;

  QGrid *Container;

  std::list<QLineEdit *> Value;
  toParamGet(QWidget *parent=0,const char *name=0);
public:
  /** Get parameters for specified SQL string.
   * @param parent Parent widget if dialog is needed.
   * @param str SQL to parse for bindings. Observe that this string can be changed to further
   *            declare the binds.
   * @return Returns a list of values to feed into the query.
   */
  static toQList getParam(QWidget *parent,QString &str);
  /** Specify a default value for the cache. This can not overwrite a manually
   * specified value by the user.
   * @param name Name of the bind variable.
   * @param val Value of the bind variable.
   */
  static void setDefault(const QString &name,const QString &val);
private slots:

  virtual void showMemo(int row);
};

#endif
