//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef __TOPARAMGET_H
#define __TOPARAMGET_H

#include <list>
#include <map>

#include <qdialog.h>
#include <qpushbutton.h>

#include "toconnection.h"
#include "tohelp.h"

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

class toParamGet : public QDialog, public toHelpContext {
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
