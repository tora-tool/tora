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

#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qlabel.h>

#include "tomemoeditor.h"
#include "tohighlightedtext.h"
#include "tomarkedtext.h"
#include "tomain.h"

#include "tomemoeditor.moc"

#include "icons/filesave.xpm"

static QPixmap *toFileSavePixmap;

toMemoEditor::toMemoEditor(QWidget *parent,const QString &str,int row,int col,
			   bool sql,bool modal)
  : QDialog(parent,NULL,modal,modal?0:WDestructiveClose)
{
  Row=row;
  Col=col;
  if (!toFileSavePixmap)
    toFileSavePixmap=new QPixmap((const char **)filesave_xpm);

  QBoxLayout *l=new QVBoxLayout(this);

  if (row>=0&&col>=0) {
    QToolBar *toolbar=toAllocBar(this,"Memo Editor",QString::null);

    new QToolButton(*toFileSavePixmap,
		    "Save changes",
		    "Save changes",
		    this,SLOT(store(void)),
		    toolbar);
    toolbar->setStretchableWidget(new QLabel("",toolbar));
    l->addWidget(toolbar);
  }

  if (sql)
    Editor=new toHighlightedText(this);
  else
    Editor=new toMarkedText(this);
  l->addWidget(Editor);
  Editor->setText(str);
  Editor->setReadOnly(row<0||col<0);
  Editor->setFocus();
  if (!modal)
    show();
}

QString toMemoEditor::text(void)
{
  return Editor->text();
}

void toMemoEditor::store(void)
{
  emit changeData(Row,Col,Editor->text());
  accept();
}
