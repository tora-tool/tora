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

#include "utils.h"

#include "toconf.h"
#include "tohighlightedtext.h"
#include "tomarkedtext.h"
#include "tomemoeditor.h"
#include "toresultview.h"

#include <qcheckbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include "tomemoeditor.moc"

#include "icons/commit.xpm"
#include "icons/copy.xpm"
#include "icons/cut.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/forward.xpm"
#include "icons/next.xpm"
#include "icons/paste.xpm"
#include "icons/previous.xpm"
#include "icons/rewind.xpm"

class toMemoText : public toMarkedText {
  toMemoEditor *MemoEditor;
public:
  toMemoText(toMemoEditor *edit,QWidget *parent,const char *name=NULL)
    : toMarkedText(parent,name),MemoEditor(edit)
  { }
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
			   bool sql,bool modal,bool navigation)
  : QDialog(parent,NULL,modal,modal?0:WDestructiveClose),Row(row),Col(col)
{
  setMinimumSize(400,300);

  QBoxLayout *l=new QVBoxLayout(this);

  Toolbar=toAllocBar(this,tr("Memo Editor"));
  l->addWidget(Toolbar);

  if (sql)
    Editor=new toMemoSQL(this,this);
  else
    Editor=new toMemoText(this,this);
  l->addWidget(Editor);
  Editor->setReadOnly(Row<0||Col<0||listView());
  Editor->setFocus();

  QToolButton *btn;
  if (Row>=0&&Col>=0&&!listView()) {
    new QToolButton(QPixmap((const char **)commit_xpm),
		    tr("Save changes"),
		    tr("Save changes"),
		    this,SLOT(store(void)),
		    Toolbar);
    Toolbar->addSeparator();
    new QToolButton(QPixmap((const char **)fileopen_xpm),
		    tr("Open file"),
		    tr("Open file"),
		    this,SLOT(openFile()),Toolbar);
    new QToolButton(QPixmap((const char **)filesave_xpm),
		    tr("Save file"),
		    tr("Save file"),
		    this,SLOT(saveFile()),Toolbar);
    Toolbar->addSeparator();
    btn=new QToolButton(QPixmap((const char **)cut_xpm),
			tr("Cut to clipboard"),
			tr("Cut to clipboard"),
			Editor,SLOT(cut()),Toolbar);
    connect(Editor,SIGNAL(copyAvailable(bool)),
	    btn,SLOT(setEnabled(bool)));
    btn->setEnabled(false);
  } else {
    new QToolButton(QPixmap((const char **)filesave_xpm),
		    tr("Save file"),
		    tr("Save file"),
		    this,SLOT(saveFile()),Toolbar);
    Toolbar->addSeparator();
  }
  btn=new QToolButton(QPixmap((const char **)copy_xpm),
			     tr("Copy to clipboard"),
			     tr("Copy to clipboard"),
			     Editor,SLOT(copy()),Toolbar);
  connect(Editor,SIGNAL(copyAvailable(bool)),
	  btn,SLOT(setEnabled(bool)));
  btn->setEnabled(false);
  if (Row>=0&&Col>=0&&!listView())
    new QToolButton(QPixmap((const char **)paste_xpm),
		    tr("Paste from clipboard"),
		    tr("Paste from clipboard"),
		    Editor,SLOT(paste()),Toolbar);

  toListView *lst=listView();
  if (lst||navigation) {
    Toolbar->addSeparator();
    new QToolButton(QPixmap((const char **)rewind_xpm),
		    tr("First column"),
		    tr("First column"),
		    this,SLOT(firstColumn()),Toolbar);
    new QToolButton(QPixmap((const char **)previous_xpm),
		    tr("Previous column"),
		    tr("Previous column"),
		    this,SLOT(previousColumn()),Toolbar);
    new QToolButton(QPixmap((const char **)next_xpm),
		    tr("Next column"),
		    tr("Next column"),
		    this,SLOT(nextColumn()),Toolbar);
    new QToolButton(QPixmap((const char **)forward_xpm),
		    tr("Last column"),
		    tr("Last column"),
		    this,SLOT(lastColumn()),Toolbar);
  }
  Toolbar->addSeparator();
  Null=new QCheckBox(tr("NULL"),Toolbar,TO_KDE_TOOLBAR_WIDGET);
  connect(Null,SIGNAL(toggled(bool)),this,SLOT(null(bool)));
  Null->setEnabled(!Editor->isReadOnly());
  Null->setFocusPolicy(StrongFocus);

  setText(str);

  Label=new QLabel(Toolbar,TO_KDE_TOOLBAR_WIDGET);
  Label->setAlignment(AlignRight|AlignVCenter);
  Toolbar->setStretchableWidget(Label);

  if (lst) {
    connect(parent,SIGNAL(currentChanged(QListViewItem *)),
	    this,SLOT(changeCurrent(QListViewItem *)));
    Label->setText(QString::fromLatin1("<B>")+
		   lst->header()->label(Col)+
		   QString::fromLatin1("</B>"));
  }

  if (!modal)
    show();
}

void toMemoEditor::setText(const QString &str)
{
  Editor->setText(str);
  Null->setChecked(str.isNull());
  Editor->setEdited(false);
}

void toMemoEditor::null(bool nul)
{
  Editor->setEdited(true);
  Editor->setDisabled(nul);
}

QString toMemoEditor::text(void)
{
  return Editor->text();
}

void toMemoEditor::store(void)
{
  if (Editor->isReadOnly())
    return;
  if (Editor->edited()) {
    if (!Editor->isEnabled())
      emit changeData(Row,Col,QString::null);
    else
      emit changeData(Row,Col,Editor->text());
  }
  accept();
}

void toMemoEditor::changePosition(int row,int cols)
{
  if (Editor->edited()) {
    if (!Editor->isEnabled())
      emit changeData(Row,Col,QString::null);
    else
      emit changeData(Row,Col,Editor->text());
    Editor->setEdited(false);
  }

  toListView *lst=listView();
  if (lst)
    Label->setText(QString::fromLatin1("<B>")+
		   lst->header()->label(Col)+
		   QString::fromLatin1("</B>"));

  Row=row;
  Col=cols;
}

toListView *toMemoEditor::listView(void)
{
  return dynamic_cast<toListView *>(parentWidget());
}

void toMemoEditor::firstColumn(void)
{
  toListView *lst=listView();
  if (lst) {
    QListViewItem *cur=lst->currentItem();
    if (!cur)
      return;

    if (Col==0) {
      QListViewItem *item=lst->firstChild();
      QListViewItem *next=item;
      while(next&&next!=cur) {
	item=next;
	if (item->firstChild())
	  next=item->firstChild();
	else if (item->nextSibling())
	  next=item->nextSibling();
	else {
	  next=item;
	  do {
	    next=next->parent();
	  } while(next&&!next->nextSibling());
	  if (next)
	    next=next->nextSibling();
	}
      }
      lst->setCurrentItem(item);
    } else {
      Col=0;
      Label->setText("<B>"+lst->header()->label(Col)+"</B>");

      toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(cur);
      toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(cur);
      if (resItem)
	setText(resItem->allText(Col));
      else if (chkItem)
	setText(chkItem->allText(Col));
      else
	setText(cur->text(Col));
    }
  }
}

void toMemoEditor::previousColumn(void)
{
  toListView *lst=listView();
  if (lst) {
    QListViewItem *cur=lst->currentItem();
    if (!cur)
      return;

    if (Col==0) {
      Col=lst->columns()-1;
      QListViewItem *item=lst->firstChild();
      QListViewItem *next=item;
      while(next&&next!=cur) {
	item=next;
	if (item->firstChild())
	  next=item->firstChild();
	else if (item->nextSibling())
	  next=item->nextSibling();
	else {
	  next=item;
	  do {
	    next=next->parent();
	  } while(next&&!next->nextSibling());
	  if (next)
	    next=next->nextSibling();
	}
      }
      if (item!=cur) {
	lst->setCurrentItem(item);
	cur=NULL;
      }
    } else
      Col--;

    if (cur) {
      toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(cur);
      toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(cur);
      if (resItem)
	setText(resItem->allText(Col));
      else if (chkItem)
	setText(chkItem->allText(Col));
      else
	setText(cur->text(Col));
    }
    Label->setText(QString::fromLatin1("<B>")+
		   lst->header()->label(Col)+
		   QString::fromLatin1("</B>"));
  }
}

void toMemoEditor::nextColumn(void)
{
  toListView *lst=listView();
  if (lst) {
    QListViewItem *cur=lst->currentItem();
    if (!cur)
      return;

    if (Col==lst->columns()-1) {
      Col=0;
      QListViewItem *next=cur;
      if (cur->firstChild())
	next=cur->firstChild();
      else if (cur->nextSibling())
	next=cur->nextSibling();
      else {
	next=cur;
	do {
	  next=next->parent();
	} while(next&&!next->nextSibling());
	if (next)
	  next=next->nextSibling();
      }
      if (next) {
	lst->setCurrentItem(next);
	cur=NULL;
      }
    } else
      Col++;
    if (cur) {
      toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(cur);
      toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(cur);
      if (resItem)
	setText(resItem->allText(Col));
      else if (chkItem)
	setText(chkItem->allText(Col));
      else
	setText(cur->text(Col));
    }
    Label->setText(QString::fromLatin1("<B>")+
		   lst->header()->label(Col)+
		   QString::fromLatin1("</B>"));
  }
}

void toMemoEditor::lastColumn(void)
{
  toListView *lst=listView();
  if (lst) {
    QListViewItem *cur=lst->currentItem();
    if (!cur)
      return;

    if (Col==lst->columns()-1) {
      QListViewItem *next=cur;
      if (cur->firstChild())
	next=cur->firstChild();
      else if (cur->nextSibling())
	next=cur->nextSibling();
      else {
	next=cur;
	do {
	  next=next->parent();
	} while(next&&!next->nextSibling());
	if (next)
	  next=next->nextSibling();
      }
      if (next)
	lst->setCurrentItem(next);
    } else {
      Col=lst->columns()-1;
      Label->setText(QString::fromLatin1("<B>")+
		     lst->header()->label(Col)+
		     QString::fromLatin1("</B>"));
      toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(cur);
      toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(cur);
      if (resItem)
	setText(resItem->allText(Col));
      else if (chkItem)
	setText(chkItem->allText(Col));
      else
	setText(cur->text(Col));
    }
  }
}

void toMemoEditor::changeCurrent(QListViewItem *)
{
  toListView *lst=listView();
  if (lst) {
    QListViewItem *cur=lst->currentItem();
    if (!cur)
      return;

    toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(cur);
    toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(cur);
    if (resItem)
      setText(resItem->allText(Col));
    else if (chkItem)
      setText(chkItem->allText(Col));
    else
      setText(cur->text(Col));
  }
}
