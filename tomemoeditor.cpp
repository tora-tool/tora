//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
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
