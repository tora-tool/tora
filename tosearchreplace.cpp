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

TO_NAMESPACE;

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

#include "tomarkedtext.h"
#include "tosearchreplace.h"
#include "tomain.h"

#include "tosearchreplaceui.moc"

toSearchReplace::toSearchReplace(QWidget *parent)
  : toSearchReplaceUI(parent,"SearchReplace")
{
  Text=NULL;
}

void toSearchReplace::setTarget(toMarkedText *parent)
{
  if (Text)
    disconnect(Text,SIGNAL(destroyed()),this,SIGNAL(destroyed()));
  Text=parent;
  connect(Text,SIGNAL(destroyed()),this,SIGNAL(destroyed()));
  show();
  searchChanged();
}

void toSearchReplace::destroyed(void)
{
  Text=NULL;
  hide();
  searchChanged();
}

void toSearchReplace::search(void)
{
  if (!Text)
    return;
  Text->setCursorPosition(0,0);
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

void toSearchReplace::searchNext(void)
{
  if (!Text)
    return;
  QString text=Text->text();
  int col;
  int line;
  Text->cursorPosition(&line,&col);
  int pos=findIndex(text,line,col);
  QString searchText=SearchText->text();

  bool ok;
  int found;
  int foundLen;
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
      toStatusMessage("No more matches found in text");
      Replace->setEnabled(false);
      ReplaceAll->setEnabled(false);
      return;
    }
    if (WholeWord->isOn()) {
      if(found!=0&&!text[found].isSpace())
	ok=false;
      if(found+foundLen!=int(text.length())&&!text[found+foundLen].isSpace())
	ok=false;
      pos=found+1;
    }
  } while(!ok);

  int endCol;
  int endLine;
  findPosition(text,found,line,col);
  findPosition(text,found+foundLen,endLine,endCol);
  Text->setCursorPosition(line,col,false);
  Text->setCursorPosition(endLine,endCol,true);

  Replace->setEnabled(!Text->isReadOnly());
  if (Text->isReadOnly())
    SearchNext->setDefault(true);
  else
    Replace->setDefault(true);
  ReplaceAll->setEnabled(!Text->isReadOnly());
}

void toSearchReplace::replace(void)
{
  if (!Text)
    return;
  Text->insert(ReplaceText->text());
  searchNext();
}

void toSearchReplace::replaceAll(void)
{
  if (!Text)
    return;
  while(Replace->isEnabled())
    replace();
}

void toSearchReplace::searchChanged(void)
{
  Search->setEnabled(Text&&SearchText->length()>0);
  SearchNext->setEnabled(Text&&SearchText->length()>0);
  Replace->setEnabled(false);
  ReplaceAll->setEnabled(false);
  if (Search->isEnabled())
    Search->setDefault(true);
  else
    Close->setDefault(true);
}
