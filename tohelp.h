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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef __TOHELP_H
#define __TOHELP_H

#include <qdialog.h>

#include <map>

class toListView;
class QLineEdit;
class QTextBrowser;
class QListViewItem;
class QProgressBar;

class toHelp : public QDialog {
  Q_OBJECT

  static toHelp *Window;
  static map<QString,QString> *Files;

  toListView *Sections;
  toListView *Result;
  QLineEdit *SearchLine;
  QTextBrowser *Help;
  QProgressBar *Progress;
public:
  toHelp(QWidget *,const char *name);
  virtual ~toHelp();
  static void displayHelp(const QString &context);
public slots:
  void search(void);
  void changeContent(QListViewItem *);
};

class toHelpContext {
  QString Name;
public:
  toHelpContext(const QString &file)
    : Name(file)
  { }
  virtual ~toHelpContext()
  { }
  virtual const QString &context(void) const
  { return Name; }
};

#endif
