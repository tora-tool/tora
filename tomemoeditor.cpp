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

#include "icons/commit.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/cut.xpm"
#include "icons/copy.xpm"
#include "icons/paste.xpm"

class toMemoText : public toMarkedText {
  toMemoEditor *MemoEditor;
public:
  toMemoText(toMemoEditor *edit,QWidget *parent,const char *name=NULL)
    : toMarkedText(parent,name),MemoEditor(edit)
  { }
  /** Reimplemented for internal reasons.
   */
  virtual void keyPressEvent(QKeyEvent *e)
  {
    if (e->state()==ControlButton&&
	e->key()==Key_Return) {
      MemoEditor->store();
      e->accept();
    } else {
      toMarkedText::keyPressEvent(e);
    }
  }
};

class toMemoSQL : public toHighlightedText {
  toMemoEditor *MemoEditor;
public:
  toMemoSQL(toMemoEditor *edit,QWidget *parent,const char *name=NULL)
    : toHighlightedText(parent,name),MemoEditor(edit)
  { }
  /** Reimplemented for internal reasons.
   */
  virtual void keyPressEvent(QKeyEvent *e)
  {
    if (e->state()==ControlButton&&
	e->key()==Key_Return) {
      MemoEditor->store();
      e->accept();
    } else {
      toHighlightedText::keyPressEvent(e);
    }
  }
};

void toMemoEditor::openFile(void)
{
  Editor->editOpen();
}

void toMemoEditor::saveFile(void)
{
  Editor->editSave(true);
}

toMemoEditor::toMemoEditor(QWidget *parent,const QString &str,int row,int col,
			   bool sql,bool modal)
  : QDialog(parent,NULL,modal,modal?0:WDestructiveClose)
{
  setMinimumSize(400,300);
  Row=row;
  Col=col;

  QBoxLayout *l=new QVBoxLayout(this);

  QToolBar *toolbar=toAllocBar(this,"Memo Editor",QString::null);
  l->addWidget(toolbar);

  if (sql)
    Editor=new toMemoSQL(this,this);
  else
    Editor=new toMemoText(this,this);
  l->addWidget(Editor);
  Editor->setText(str);
  Editor->setReadOnly(row<0||col<0);
  Editor->setFocus();

  QToolButton *btn;
  if (row>=0&&col>=0) {
    new QToolButton(QPixmap((const char **)commit_xpm),
		    "Save changes",
		    "Save changes",
		    this,SLOT(store(void)),
		    toolbar);
    toolbar->addSeparator();
    new QToolButton(QPixmap((const char **)fileopen_xpm),
		    "Open file",
		    "Open file",
		    this,SLOT(openFile()),toolbar);
    new QToolButton(QPixmap((const char **)filesave_xpm),
		    "Save file",
		    "Save file",
		    this,SLOT(saveFile()),toolbar);
    toolbar->addSeparator();
    btn=new QToolButton(QPixmap((const char **)cut_xpm),
			"Cut to clipboard",
			"Cut to clipboard",
			Editor,SLOT(cut()),toolbar);
    connect(Editor,SIGNAL(copyAvailable(bool)),
	    btn,SLOT(setEnabled(bool)));
    btn->setEnabled(false);
  } else {
    new QToolButton(QPixmap((const char **)filesave_xpm),
		    "Save file",
		    "Save file",
		    this,SLOT(saveFile()),toolbar);
    toolbar->addSeparator();
  }
  btn=new QToolButton(QPixmap((const char **)copy_xpm),
			     "Copy to clipboard",
			     "Copy to clipboard",
			     Editor,SLOT(copy()),toolbar);
  connect(Editor,SIGNAL(copyAvailable(bool)),
	  btn,SLOT(setEnabled(bool)));
  btn->setEnabled(false);
  if (row>=0&&col>=0)
    new QToolButton(QPixmap((const char **)paste_xpm),
		    "Paste from clipboard",
		    "Paste from clipboard",
		    Editor,SLOT(paste()),toolbar);
  
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  if (!modal)
    show();
}

QString toMemoEditor::text(void)
{
  return Editor->text();
}

void toMemoEditor::store(void)
{
  if (Editor->isReadOnly())
    return;
  emit changeData(Row,Col,Editor->text());
  accept();
}
