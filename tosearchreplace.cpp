//***************************************************************************
/*
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

TO_NAMESPACE;

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qmultilineedit.h>
#include <qaccel.h>

#include "tomarkedtext.h"
#include "tosearchreplace.h"
#include "tomain.h"
#include "toresultview.h"
#include "toresultcontent.h"
#include "tohelp.h"

#include "tosearchreplaceui.moc"

toSearchReplace::toSearchReplace(QWidget *parent)
  : toSearchReplaceUI(parent,"SearchReplace"),toHelpContext("searchreplace.html")
{
  QAccel *a=new QAccel(this);
  a->connectItem(a->insertItem(Key_F1),
		 this,
		 SLOT(displayHelp()));
  Text=NULL;
  List=NULL;
  Content=NULL;
}

void toSearchReplace::displayHelp(void)
{
  toHelp::displayHelp();
}

void toSearchReplace::release(void)
{
  if (Text)
    disconnect(Text,SIGNAL(destroyed()),this,SIGNAL(destroyed()));
  else if (List)
    disconnect(List,SIGNAL(destroyed()),this,SIGNAL(destroyed()));
  else if (Content)
    disconnect(Content,SIGNAL(destroyed()),this,SIGNAL(destroyed()));
  List=NULL;
  Text=NULL;
  Content=NULL;
}

void toSearchReplace::setTarget(toMarkedText *parent)
{
  release();
  Text=parent;
  connect(Text,SIGNAL(destroyed()),this,SIGNAL(destroyed()));
  show();
  searchChanged();
}

void toSearchReplace::setTarget(toResultContent *parent)
{
  release();
  Content=parent;
  connect(Content,SIGNAL(destroyed()),this,SIGNAL(destroyed()));
  show();
  searchChanged();
}

void toSearchReplace::setTarget(toListView *parent)
{
  disconnect();
  List=parent;
  connect(List,SIGNAL(destroyed()),this,SIGNAL(destroyed()));
  show();
  searchChanged();
}

void toSearchReplace::destroyed(void)
{
  List=NULL;
  Text=NULL;
  Content=NULL;
  hide();
  searchChanged();
}

void toSearchReplace::search(void)
{
  FromTop=true;
  searchNext();
}

int toSearchReplace::findIndex(const QString &str,int line,int col)
{
  int pos=0;
  for (int i=0;i<line;i++) {
    pos=str.find("\n",pos);
    if (pos<0)
      return pos;
    pos++;
  }
  return pos+col;
}

void toSearchReplace::findPosition(const QString &str,int index,int &line,int &col)
{
  int pos=0;
  for (int i=0;i<Text->numLines();i++) {
    QString str=Text->textLine(i);
    if (str.length()+pos>(unsigned int)index) {
      line=i;
      col=index-pos;
      return;
    }
    pos+=str.length()+1;
  }
  col=-1;
  line=-1;
  return ;
}

bool toSearchReplace::findString(const QString &text,int &pos,int &endPos)
{
  bool ok;
  int found;
  int foundLen;
  QString searchText=SearchText->text(); 
  do {
    ok=true;
    if(Exact->isOn()) {
      found=text.find(searchText,pos,!IgnoreCase->isOn());
      foundLen=searchText.length();
    } else {
      QRegExp re(searchText,!IgnoreCase->isOn(),false);
      found=re.match(text,pos,&foundLen,false);
    }
    if (found==-1) {
      return false;
    }
    if (WholeWord->isOn()) {
      if(found!=0&&!text[found].isSpace())
	ok=false;
      if(found+foundLen!=int(text.length())&&!text[found+foundLen].isSpace())
	ok=false;
      pos=found+1;
    }
  } while(!ok);

  pos=found;
  endPos=found+foundLen;
  return true;
}

void toSearchReplace::searchNext(void)
{
  if (Text) {
    if (FromTop) {
      Text->setCursorPosition(0,0);
      FromTop=false;
    }
    QString text=Text->text();
    int col;
    int line;
    Text->cursorPosition(&line,&col);
    int pos=findIndex(text,line,col);

    int endPos;
    if (findString(text,pos,endPos)) {
      int endCol;
      int endLine;
      findPosition(text,pos,line,col);
      findPosition(text,endPos,endLine,endCol);
      Text->setCursorPosition(line,col,false);
      Text->setCursorPosition(endLine,endCol,true);

      Replace->setEnabled(!Text->isReadOnly());
      if (Text->isReadOnly())
	SearchNext->setDefault(true);
      else
	Replace->setDefault(true);
      ReplaceAll->setEnabled(!Text->isReadOnly());
      return;
    }
  } else if (List) {
    QListViewItem *item;
    if (FromTop) {
      item=List->firstChild();
      FromTop=false;
    } else
      item=List->currentItem();

    bool first=FromTop;
    if (!item)
      item=List->firstChild();

    for (QListViewItem *next=NULL;item;item=next) {
      if (!first)
	first=true;
      else {
	for (int i=0;i<List->columns();i++) {
	  int pos=0;
	  int endPos=0;
	  if (findString(item->text(0),pos,endPos)) {
	    List->setCurrentItem(item);
	    SearchNext->setDefault(true);
	    for(;;) {
	      item=item->parent();
	      if (!item)
		return;
	      item->setOpen(true);
	    }
	  }
	}
      }

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
  } else if (Content) {
    int row=0;
    int col=0;
    if (FromTop)
      FromTop=false;
    else {
      row=Content->editor()->currentRow();
      col=Content->editor()->currentColumn()+1;
      if (col>=Content->editor()->numCols()) {
	row++;
	col=0;
      }
      while(row<Content->editor()->numRows()) {
	int pos=0;
	int endPos;
	if (findString(Content->editor()->text(row,col),pos,endPos)) {
	  Content->editor()->setCurrentCell(row,col);
	  SearchNext->setDefault(true);
	  return;
	}
	col++;
	if (col>=Content->editor()->numCols()) {
	  row++;
	  col=0;
	}
      }
    }
  }
  toStatusMessage("No more matches found");
  Replace->setEnabled(false);
  ReplaceAll->setEnabled(false);
}

void toSearchReplace::replace(void)
{
  if (!Text||Text->isReadOnly())
    return;
  Text->insert(ReplaceText->text());
  searchNext();
}

void toSearchReplace::replaceAll(void)
{
  if (!Text||Text->isReadOnly())
    return;
  while(Replace->isEnabled())
    replace();
}

void toSearchReplace::searchChanged(void)
{
  Search->setEnabled((Text||List||Content)&&SearchText->length()>0);
  SearchNext->setEnabled((Text||List||Content)&&SearchText->length()>0);
  Replace->setEnabled(false);
  ReplaceAll->setEnabled(false);
  if (Search->isEnabled())
    Search->setDefault(true);
  else
    Close->setDefault(true);
}
