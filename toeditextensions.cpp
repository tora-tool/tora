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
#include <qspinbox.h>
#include <qcheckbox.h>

#ifdef TO_KDE
#include <kmenubar.h>
#endif

#include "totool.h"
#include "toeditextensions.h"
#include "tohighlightedtext.h"
#include "toeditextensionsetupui.h"

#include "toeditextensions.moc"
#include "toeditextensionsetupui.moc"

#include "icons/indent.xpm"
#include "icons/deindent.xpm"

static int IndentIndex;
static int DeindentIndex;

#define TABSTOP 8

#define CONF_EXPAND_SPACES	"ExpandSpaces"
#define CONF_INDENT_LEVEL	"IndentLevel"
#define DEFAULT_INDENT_LEVEL	"4"

static bool ExpandSpaces;
static int IndentLevel;

QToolButton *IndentButton;
QToolButton *DeindentButton;

static int CountIndent(const QString &txt,int &chars)
{
  int level=0;
  chars=0;
  while(txt[chars].isSpace()&&chars<int(txt.length())) {
    char c=txt[chars];
    if (c=='\n')
      level=0;
    else if (c==' ')
      level++;
    else if (c=='\t')
      level=(level/TABSTOP+1)*TABSTOP;
    chars++;
  }
  return level;
}

static QString IndentString(int level,bool &eol)
{
  if (eol) {
    QString ret="\n";
    if (ExpandSpaces) {
      for(int i=0;i<level/8;i++)
	ret+="\t";
      for(int j=0;j<level%8;j++)
	ret+=" ";
    } else
      for(int j=0;j<level;j++)
	ret+=" ";
    return ret;
  } else
    return " ";
}

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

void toEditExtensions::intIndent(int delta)
{
  bool eol;

  int line1,col1,line2,col2;

  if (!Current->getMarkedRegion(&line1,&col1,&line2,&col2)) {
    Current->getCursorPosition (&line1,&col1);
    line2=line1;
  } else if (col2==0) {
    line2--;
  }

  QString res;
  for(int i=line1;i<=line2;i++) {
    QString t=Current->textLine(i);
    int chars;
    int level=CountIndent(t,chars);
    eol=true;
    res+=IndentString(max(0,level+delta),eol);
    res+=t.mid(chars);
  }

  Current->setCursorPosition(line1,0,false);
  Current->setCursorPosition(line2,Current->textLine(line2).length(),true);

  Current->insert(res.mid(1));

  Current->setCursorPosition(line1,0,false);
  Current->setCursorPosition(line2,Current->textLine(line2).length(),true);
}

void toEditExtensions::deindentBlock(void)
{
  intIndent(-IndentLevel);
}

void toEditExtensions::indentBlock(void)
{
  intIndent(IndentLevel);
}

static toEditExtensions EditExtensions;

class toEditExtensionTool;

class toEditExtensionSetup : public toEditExtensionSetupUI, public toSettingTab
{
  toEditExtensionTool *Tool;

public:
  toEditExtensionSetup(toEditExtensionTool *tool,QWidget *parent,const char *name=NULL)
    : toEditExtensionSetupUI(parent,name),toSettingTab("editextension.html"),Tool(tool)
  {
    IndentLevel->setValue(::IndentLevel);
    ExpandSpaces->setChecked(::ExpandSpaces);
  }
  virtual void saveSetting(void);
};

class toEditExtensionTool : public toTool {
public:
  void cacheConfig(void)
  {
    ExpandSpaces=!config(CONF_EXPAND_SPACES,"Yes").isEmpty();
    IndentLevel=config(CONF_INDENT_LEVEL,DEFAULT_INDENT_LEVEL).toInt();
    if (!IndentLevel)
      IndentLevel=4;
  }
  toEditExtensionTool()
    : toTool(300,"Editor")
  {
    cacheConfig();
  }
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
  virtual QWidget *configurationTab(QWidget *parent)
  {
    return new toEditExtensionSetup(this,parent);
  }
};

void toEditExtensionSetup::saveSetting(void)
{
  Tool->setConfig(CONF_INDENT_LEVEL,QString::number(IndentLevel->value()));
  Tool->setConfig(CONF_EXPAND_SPACES,ExpandSpaces->isChecked()?"Yes":"");
  Tool->cacheConfig();
}

static toEditExtensionTool EditExtensionTool;

#if 0 // Unfinished stuff for pretty print

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
