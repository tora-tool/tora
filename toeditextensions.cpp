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
#include "tosqlparse.h"
#include "toconf.h"

#include "toeditextensions.moc"
#include "toeditextensionsetupui.moc"

#include "icons/indent.xpm"
#include "icons/deindent.xpm"

static int IndentIndex;
static int DeindentIndex;
static int AutoIndentBuffer;
static int AutoIndentBlock;
static int ObfuscateBuffer;
static int ObfuscateBlock;

#define CONF_EXPAND_SPACES	"ExpandSpaces"
#define CONF_COMMA_BEFORE	"CommaBefore"
#define CONF_BLOCK_OPEN_LINE	"BlockOpenLine"
#define CONF_OPERATOR_SPACE	"OperatorSpace"
#define CONF_KEYWORD_UPPER	"KeywordUpper"
#define CONF_RIGHT_SEPARATOR	"RightSeparator"
#define CONF_END_BLOCK_NEWLINE	"EndBlockNewline"
#define CONF_INDENT_LEVEL	"IndentLevel"
#define DEFAULT_INDENT_LEVEL	"4"
#define CONF_COMMENT_COLUMN	"CommentColumn"
#define DEFAULT_COMMENT_COLUMN	"60"

QToolButton *IndentButton;
QToolButton *DeindentButton;

void toEditExtensions::receivedFocus(QWidget *widget)
{
  if (widget)
    Current=dynamic_cast<toMarkedText *>(widget);
  else
    Current=NULL;

  bool enable=Current&&!Current->isReadOnly();

  toMainWidget()->menuBar()->setItemEnabled(IndentIndex,enable);
  toMainWidget()->menuBar()->setItemEnabled(DeindentIndex,enable);
  toMainWidget()->menuBar()->setItemEnabled(AutoIndentBlock,enable);
  toMainWidget()->menuBar()->setItemEnabled(ObfuscateBlock,enable);
  toMainWidget()->menuBar()->setItemEnabled(AutoIndentBuffer,Current);
  toMainWidget()->menuBar()->setItemEnabled(ObfuscateBuffer,Current);
  if(IndentButton)
    IndentButton->setEnabled(enable);
  if(DeindentButton)
    DeindentButton->setEnabled(enable);
}

void toEditExtensions::lostFocus(QWidget *widget)
{
  if (Current==widget)
    receivedFocus(NULL);
}

void toEditExtensions::intIndent(int delta)
{
  int line1,col1,line2,col2;

  if (Current) {
    if (!Current->getMarkedRegion(&line1,&col1,&line2,&col2)) {
      Current->getCursorPosition (&line1,&col1);
      line2=line1;
    } else if (col2==0) {
      line2--;
    }

    QString res;
    for(int i=line1;i<=line2;i++) {
      QString t=Current->textLine(i);
      int chars=0;
      int level=toSQLParse::countIndent(t,chars);
      res+=toSQLParse::indentString(max(0,level+delta));
      if (i<line2)
	res+=t.mid(chars)+"\n";
      else
	res+=t.mid(chars);
    }

    Current->setCursorPosition(line1,0,false);
    Current->setCursorPosition(line2,Current->textLine(line2).length(),true);

    Current->insert(res,true);
  }
}

void toEditExtensions::deindentBlock(void)
{
  intIndent(-toSQLParse::getSetting().IndentLevel);
}

void toEditExtensions::indentBlock(void)
{
  intIndent(toSQLParse::getSetting().IndentLevel);
}

void toEditExtensions::autoIndentBlock(void)
{
  if (Current) {
    int line1,col1,line2,col2;
    if (Current->getMarkedRegion(&line1,&col1,&line2,&col2)) {
      QString ind=toSQLParse::indentString(col1);
      QString mrk=Current->markedText();
      QString res=toSQLParse::indent(ind+mrk);
      res=res.mid(ind.length()); // Strip indent.
      Current->insert(res,true);
    }
  }
}

void toEditExtensions::autoIndentBuffer(void)
{
  if (Current) {
    QString text=Current->text();
    unsigned int pos=0;
    while(pos<text.length()&&text.at(pos).isSpace()) {
      pos++;
    }
    Current->selectAll();
    Current->insert(toSQLParse::indent(text.mid(pos)));
  }
}

static int CountLine(const QString &str)
{
  int found=str.findRev("\n");
  if (found<0)
    return str.length();
  else
    return str.length()-found+1;
}

static void ObfuscateStat(toSQLParse::statement &stat,QString &ret)
{
  if (ret.length()>0&&
      stat.String.length()>0&&
      toIsIdent(ret.at(ret.length()-1))&&
      toIsIdent(stat.String.at(0))) {
    if (CountLine(ret)<60)
      ret+=" ";
    else
      ret+="\n";
  }
  ret+=stat.String;
  if (!stat.Comment.isEmpty()) {
    ret+=stat.Comment;
    ret+="\n";
  }
  for(std::list<toSQLParse::statement>::iterator i=stat.SubTokens->begin();
      i!=stat.SubTokens->end();
      i++) {
    ObfuscateStat(*i,ret);
  }
}

void toEditExtensions::obfuscateBlock(void)
{
  if (Current) {
    QString str=Current->markedText();
    if (!str.isEmpty()) {
      toSQLParse::statement stat;
      *stat.SubTokens=toSQLParse::parse(str);
      QString res;
      ObfuscateStat(stat,res);
      Current->insert(res);
    }
  }
}

void toEditExtensions::obfuscateBuffer(void)
{
  if (Current) {
    QString str=Current->text();
    if (!str.isEmpty()) {
      toSQLParse::statement stat;
      *stat.SubTokens=toSQLParse::parse(str);
      Current->selectAll();
      QString res;
      ObfuscateStat(stat,res);
      Current->insert(res);
    }
  }
}

static toEditExtensions EditExtensions;

class toEditExtensionTool;

class toEditExtensionSetup : public toEditExtensionSetupUI, public toSettingTab
{
  toEditExtensionTool *Tool;
  toSQLParse::settings Current;
  bool Ok;
  bool Started;
public:
  toEditExtensionSetup(toEditExtensionTool *tool,QWidget *parent,const char *name=NULL)
    : toEditExtensionSetupUI(parent,name),toSettingTab("editextension.html"),Tool(tool)
  {
    Current=toSQLParse::getSetting();
    Started=false;
    ExpandSpaces->setChecked(Current.ExpandSpaces);
    CommaBefore->setChecked(Current.CommaBefore);
    BlockOpenLine->setChecked(Current.BlockOpenLine);
    OperatorSpace->setChecked(Current.OperatorSpace);
    KeywordUpper->setChecked(Current.KeywordUpper);
    RightSeparator->setChecked(Current.RightSeparator);
    EndBlockNewline->setChecked(Current.EndBlockNewline);
    IndentLevel->setValue(Current.IndentLevel);
    CommentColumn->setValue(Current.CommentColumn);
    AutoIndent->setChecked(!toTool::globalConfig(CONF_AUTO_INDENT_RO,"Yes").isEmpty());
    Ok=false;
    Example->setText(toSQLParse::indent("CREATE OR REPLACE procedure spTuxGetAccData (oRet OUT  NUMBER)\n"
					"AS\n"
					"  vYear  CHAR(4);\n"
					"BEGIN\n"
					"select a.TskCod TskCod, -- A Comment\n"
					"       count(1) Tot\n"
					"  from (select * from EssTsk where PrsID >= '1940');\n"
					"having count(a.TspActOprID) > 0;\n"
					"    DECLARE\n"
					"      oTrdStt NUMBER;\n"
					"    BEGIN\n"
					"      oTrdStt := 0;\n"
					"    END;\n"
					"    EXCEPTION\n"
					"        WHEN VALUE_ERROR THEN\n"
					"	    oRet := 3;\n"
					"END;"));
    Started=true;
  }
  virtual ~toEditExtensionSetup()
  {
    if (!Ok)
      toSQLParse::setSetting(Current);
  }
  void saveCurrent(void)
  {
    Current.ExpandSpaces=ExpandSpaces->isChecked();
    Current.CommaBefore=CommaBefore->isChecked();
    Current.BlockOpenLine=BlockOpenLine->isChecked();
    Current.OperatorSpace=OperatorSpace->isChecked();
    Current.KeywordUpper=KeywordUpper->isChecked();
    Current.RightSeparator=RightSeparator->isChecked();
    Current.EndBlockNewline=EndBlockNewline->isChecked();
    Current.IndentLevel=IndentLevel->value();
    Current.CommentColumn=CommentColumn->value();
    toSQLParse::setSetting(Current);
  }
  virtual void changed(void)
  {
    if (Started) {
      saveCurrent();
      Example->setText(toSQLParse::indent(Example->text()));
    }
  }
  virtual void saveSetting(void);
};

class toEditExtensionTool : public toTool {
public:
  toEditExtensionTool()
    : toTool(400,"Editor")
  {
    toSQLParse::settings cur;
    cur.ExpandSpaces=!config(CONF_EXPAND_SPACES,"Yes").isEmpty();
    cur.CommaBefore=!config(CONF_COMMA_BEFORE,"").isEmpty();
    cur.BlockOpenLine=!config(CONF_BLOCK_OPEN_LINE,"").isEmpty();
    cur.OperatorSpace=!config(CONF_OPERATOR_SPACE,"Yes").isEmpty();
    cur.KeywordUpper=!config(CONF_KEYWORD_UPPER,"Yes").isEmpty();
    cur.RightSeparator=!config(CONF_RIGHT_SEPARATOR,"Yes").isEmpty();
    cur.EndBlockNewline=!config(CONF_END_BLOCK_NEWLINE,"Yes").isEmpty();
    cur.IndentLevel=config(CONF_INDENT_LEVEL,DEFAULT_INDENT_LEVEL).toInt();
    cur.CommentColumn=config(CONF_COMMENT_COLUMN,DEFAULT_COMMENT_COLUMN).toInt();
    toSQLParse::setSetting(cur);
  }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return NULL; // Has no tool window
  }
  virtual void customSetup(int toolid)
  {
    toMainWidget()->editMenu()->insertSeparator();

    QPopupMenu *menu=new QPopupMenu(toMainWidget());
    AutoIndentBlock=menu->insertItem("Selection",
				     &EditExtensions,
				     SLOT(autoIndentBlock()),
				     ALT+CTRL+Key_I);
    AutoIndentBuffer=menu->insertItem("Editor",
				      &EditExtensions,
				      SLOT(autoIndentBuffer()),
				      ALT+CTRL+SHIFT+Key_I);
    menu->insertSeparator();
    ObfuscateBlock=menu->insertItem("Obfuscate selection",
				    &EditExtensions,
				    SLOT(obfuscateBlock()));
    ObfuscateBuffer=menu->insertItem("Obfuscate editor",
				     &EditExtensions,
				     SLOT(obfuscateBuffer()));
    toMainWidget()->editMenu()->insertItem("Auto indent",menu);

    IndentIndex=toMainWidget()->editMenu()->insertItem(QPixmap((const char **)indent_xpm),
						       "Indent block",&EditExtensions,
						       SLOT(indentBlock()),
						       ALT+Key_Right);
    DeindentIndex=toMainWidget()->editMenu()->insertItem(QPixmap((const char **)deindent_xpm),
							 "De-indent block",&EditExtensions,
							 SLOT(deindentBlock()),
							 ALT+Key_Left);

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
  Ok=true;
  Tool->setConfig(CONF_EXPAND_SPACES,ExpandSpaces->isChecked()?"Yes":"");
  Tool->setConfig(CONF_COMMA_BEFORE,CommaBefore->isChecked()?"Yes":"");
  Tool->setConfig(CONF_BLOCK_OPEN_LINE,BlockOpenLine->isChecked()?"Yes":"");
  Tool->setConfig(CONF_OPERATOR_SPACE,OperatorSpace->isChecked()?"Yes":"");
  Tool->setConfig(CONF_KEYWORD_UPPER,KeywordUpper->isChecked()?"Yes":"");
  Tool->setConfig(CONF_RIGHT_SEPARATOR,RightSeparator->isChecked()?"Yes":"");
  Tool->setConfig(CONF_END_BLOCK_NEWLINE,EndBlockNewline->isChecked()?"Yes":"");
  Tool->setConfig(CONF_INDENT_LEVEL,QString::number(IndentLevel->value()));
  Tool->setConfig(CONF_COMMENT_COLUMN,QString::number(CommentColumn->value()));
  toTool::globalSetConfig(CONF_AUTO_INDENT_RO,AutoIndent->isChecked()?"Yes":"");
  saveCurrent();
}

static toEditExtensionTool EditExtensionTool;
