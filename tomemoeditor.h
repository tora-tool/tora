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

#ifndef __TOMEMOEDITOR_H
#define __TOMEMOEDITOR_H

#include <qdialog.h>

class toMarkedText;

/** Displays an text in a widget which can optionally be modified and saved back.
 */

class toMemoEditor : public QDialog {
  Q_OBJECT

  /** Editor of widget
   */
  toMarkedText *Editor;
  /** Row of this field
   */
  int Row;
  /** Column of this field
   */
  int Col;
public:
  /** Create this editor. If row and col is specified, editing is posible.
   * @param parent Parent widget.
   * @param data Data of editor.
   * @param row Optional location specifier, pass on in @ref changeData call.
   * @param col Optional location specifier, pass on in @ref changeData call.
   * @param sql Use SQL syntax highlighting of widget.
   * @param modal Display in modal dialog
   */
  toMemoEditor(QWidget *parent,const QString &data,int row=-1,int col=-1,
	       bool sql=false,bool modal=false);
  /** Get editor text.
   * @return String of editor.
   */
  QString text(void);
signals:
  /** Emitted when data is to be stored back to the creator
   * @param row Pass on from creation.
   * @param col Pass on from creation.
   * @param data New data to save.
   */
  void changeData(int row,int col,const QString &data);
private slots:
  void store(void);
};

#endif
