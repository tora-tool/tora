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

#include <qmenubar.h>
#include <qtoolbutton.h>

#ifdef TO_KDE
#include <kmenubar.h>
#endif

#include "totool.h"
#include "toeditextensions.h"
#include "tohighlightedtext.h"

#include "toeditextensions.moc"

#include "icons/indent.xpm"
#include "icons/deindent.xpm"

static int IndentIndex;
static int DeindentIndex;

QToolButton *IndentButton;
QToolButton *DeindentButton;

void toEditExtensions::receivedFocus(QWidget *widget)
{
  if (widget)
    Current=dynamic_cast<toMarkedText *>(widget);
  else
    Current=NULL;

  toMainWidget()->menuBar()->setItemEnabled(IndentIndex,Current);
  toMainWidget()->menuBar()->setItemEnabled(DeindentIndex,Current);
  if(IndentButton)
    IndentButton->setEnabled(Current);
  if(DeindentButton)
    DeindentButton->setEnabled(Current);
}

void toEditExtensions::lostFocus(QWidget *widget)
{
  if (Current==widget)
    receivedFocus(NULL);
}

void toEditExtensions::indentBlock(void)
{

}

void toEditExtensions::deindentBlock(void)
{

}

static toEditExtensions EditExtensions;

class toEditExtensionTool : public toTool {
public:
  toEditExtensionTool()
    : toTool(300,"Editor")
  { }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return NULL; // Has no tool window
  }
  virtual void customSetup(int toolid)
  {
    toMainWidget()->editMenu()->insertSeparator();
    IndentIndex=toMainWidget()->editMenu()->insertItem(QPixmap((const char **)indent_xpm),
						       "Indent block",&EditExtensions,
						       SLOT(indentBlock()));
    DeindentIndex=toMainWidget()->editMenu()->insertItem(QPixmap((const char **)deindent_xpm),
							 "De-indent block",&EditExtensions,
							 SLOT(deindentBlock()));

    toMainWidget()->editToolbar()->addSeparator();
    IndentButton=new QToolButton(QPixmap((const char **)indent_xpm),
				 "Indent block in editor",
				 "Indent block in editor",
				 &EditExtensions,SLOT(indentBlock()),
				 toMainWidget()->editToolbar());
    DeindentButton=new QToolButton(QPixmap((const char **)deindent_xpm),
				   "De-indent block in editor",
				   "De-indent block in editor",
				   &EditExtensions,SLOT(deindentBlock()),
				   toMainWidget()->editToolbar());
    EditExtensions.receivedFocus(NULL);
  }
};

static toEditExtensionTool EditExtensionTool;

#if 0 // Unfinished stuff for pretty print

#define TABSTOP 8

static QString Indent(int level,bool &eol);
static int indent=4;

static void IndentBlock(const QString &sql,int &pos,QString &token,
			QString &ret,int level,bool eol)
{
  bool pre=false;
  QString upp=token.upper();

  ret+=Indent(level,eol);
  ret+=token;
  level+=indent;
  if (upp=="AS"||upp=="IS"||upp=="DECLARE") 
    pre=true;

  for (token=toGetToken(sql,pos);!token.isNull();token=toGetToken(sql,pos)) {
    upp=token.upper();
    if (upp="END") {
      eol=true;
      break;
    } else if (upp=="BEGIN"||upp=="THEN"||upp=="LOOP") {
      if (pre) {
	ret+=Indent(level-indent,eol);
	ret+=token;
	eol=true;
	pre=false;
      } else {
	IndentBlock(sql,pos,token,ret,level,eol);
      }
    } else if (upp=="AS"||upp=="IS"||upp=="DECLARE") {
      IndentBlock(sql,pos,token,ret,level,eol);
    }
  }
  if (!token.isNull()) {
    ret+=Indent(level,eol);
    ret+=token;
    for (token=toGetToken(sql,pos);!token.isNull()&&token!=";";token=toGetToken(sql,pos)) {
      ret+=" ";
      ret+=token;
    }
    ret+=";"
  }
}

QString toIndentSQL(const QString &sql,bool start)
{
  int level=0;
  int pos=0;
  std::list<int> statementIndent;

  if (!start) {
    while(sql[pos].isSpace()&&pos<sql.length()) {
      char c=sql[pos];
      if (c=='\n')
	level=0;
      else if (c==' ')
	level++;
      else if (c=='\t')
	level=(leve+TABSTOP-1)/TABSTOP*TABSTOP;
      pos++;
    }
  }
  bool pre=false;
  bool any=false;
  bool eol=false;
  QString ret;
  for (QString token=toGetToken(sql,pos);!token.isNull();token=toGetToken(sql,pos)) {
    QString upp=token.lower();
    if (upp=="BEGIN"||upp=="THEN"||upp=="LOOP") {
      if (pre)
	ret+=Indent(level-indent,eol);
      else
	ret+=Indent(level,eol);
      ret+=token;
      if (!pre)
	level+=indent;
      else
	pre=false;
      any=false;
      eol=true;
    } else if (upp=="END") {
      level-=indent;
      ret+=Indent(level,eol);
      ret+=token;
      any=pre=false;
    } else if (upp=="AND"||upp=="OR") {
      ret+=" ";
      ret+=upp;
      eol=true;
      any=false;
      ret+=Indent(level+statementIndent);
    } else if (upp==";") {
      ret+=";";
      statementIndent.clear();
      eol=true;
      any=false;
    } else if (upp=="DECLARE"||upp=="AS"||upp=="IS") {
      pre=true;
      eol=true;
      any=false;
    }
  }
}

#endif
