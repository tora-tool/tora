//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "toeditextensions.h"
#include "toeditextensionsetupui.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tosqlparse.h"
#include "totool.h"

#ifdef TO_KDE
#include <kmenubar.h>
#endif

#include <qcheckbox.h>
#include <qmenubar.h>
#include <qspinbox.h>
#include <qtoolbutton.h>

#include "toeditextensions.moc"
#include "toeditextensionsetupui.moc"
#include "toeditextensiongotoui.moc"

#include "icons/deindent.xpm"
#include "icons/indent.xpm"

static int AutoIndentBlock;
static int AutoIndentBuffer;
static int DeindentIndex;
static int IndentIndex;
static int ObfuscateBlock;
static int ObfuscateBuffer;
static int ReverseSearch;
static int IncrementalSearch;
static int UpperCase;
static int LowerCase;
static int GotoLine;

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

static QToolButton *IndentButton;
static QToolButton *DeindentButton;

void toEditExtensions::receivedFocus(toEditWidget *widget)
{
  if (widget)
    Current=dynamic_cast<toMarkedText *>(widget);
  else
    Current=NULL;

  bool enable=Current&&!Current->isReadOnly();

  toMainWidget()->editMenu()->setItemEnabled(IndentIndex,enable);
  toMainWidget()->editMenu()->setItemEnabled(DeindentIndex,enable);
  toMainWidget()->editMenu()->setItemEnabled(AutoIndentBlock,enable);
  toMainWidget()->editMenu()->setItemEnabled(ObfuscateBlock,enable);
  toMainWidget()->editMenu()->setItemEnabled(AutoIndentBuffer,Current);
  toMainWidget()->editMenu()->setItemEnabled(ObfuscateBuffer,Current);
  toMainWidget()->editMenu()->setItemEnabled(IncrementalSearch,Current);
  toMainWidget()->editMenu()->setItemEnabled(ReverseSearch,Current);
  toMainWidget()->editMenu()->setItemEnabled(UpperCase,Current);
  toMainWidget()->editMenu()->setItemEnabled(LowerCase,Current);
  toMainWidget()->editMenu()->setItemEnabled(GotoLine,Current);

  if(IndentButton)
    IndentButton->setEnabled(enable);
  if(DeindentButton)
    DeindentButton->setEnabled(enable);
}

void toEditExtensions::lostFocus(toEditWidget *widget)
{
  if (widget) {
    toMarkedText *current=dynamic_cast<toMarkedText *>(widget);
    if (current&&Current==current)
      receivedFocus(NULL);
  }
}

void toEditExtensions::gotoLine()
{
  if (Current) {
    toEditExtensionGoto dialog(Current);
    if (dialog.exec())
      dialog.gotoLine();
  }
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
    try {
      int line1,col1,line2,col2;
      if (Current->getMarkedRegion(&line1,&col1,&line2,&col2)) {
	QString t=Current->textLine(line1).mid(0,col1);
	t+=QString::fromLatin1("a");
	int chars=0;
	QString ind=toSQLParse::indentString(toSQLParse::countIndent(t,chars));
	QString mrk=Current->markedText();
	QString res;
	try {
	  res=toSQLParse::indent(ind+mrk,toCurrentConnection(Current));
	} catch(...) {
	  res=toSQLParse::indent(ind+mrk);
	}
	t=Current->textLine(line2);
	unsigned int l=res.length()-ind.length();
	if (col2==int(t.length())&&t.length()>0) // Strip last newline if on last col of line
	  l--;
	res=res.mid(ind.length(),l); // Strip indent.
	Current->insert(res,true);
      }
    } TOCATCH
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
    try {
      try {
	Current->insert(toSQLParse::indent(text.mid(pos),toCurrentConnection(Current)));
      } catch(...) {
	Current->insert(toSQLParse::indent(text.mid(pos)));
      }
    } TOCATCH
  }
}

void toEditExtensions::upperCase(void)
{
  if (Current) {
    QString text=Current->markedText().upper();
    if (!text.isEmpty())
      Current->insert(text,true);
  }
}

void toEditExtensions::lowerCase(void)
{
  if (Current) {
    QString text=Current->markedText().lower();
    if (!text.isEmpty())
      Current->insert(text,true);
  }
}

static int CountLine(const QString &str)
{
  int found=str.findRev(QString::fromLatin1("\n"));
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
      ret+=QString::fromLatin1(" ");
    else
      ret+=QString::fromLatin1("\n");
  }
  ret+=stat.String;
  if (!stat.Comment.isEmpty()) {
    ret+=stat.Comment;
    ret+=QString::fromLatin1("\n");
  }
  for(std::list<toSQLParse::statement>::iterator i=stat.subTokens().begin();
      i!=stat.subTokens().end();
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
      stat.subTokens()=toSQLParse::parse(str);
      QString res;
      ObfuscateStat(stat,res);
      Current->insert(res,true);
    }
  }
}

void toEditExtensions::obfuscateBuffer(void)
{
  if (Current) {
    QString str=Current->text();
    if (!str.isEmpty()) {
      toSQLParse::statement stat;
      stat.subTokens()=toSQLParse::parse(str);
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
    try {
      Example->setAnalyzer(toMainWidget()->currentConnection().analyzer());
    } TOCATCH

    try {
#ifdef TOAD
      Example->setText(toSQLParse::indent("CREATE PROCEDURE COUNT_EMPS_IN_DEPTS (OUT V_TOTAL INT)\n"
					  "BEGIN\n"
					  "  DECLARE V_DEPTNO INT DEFAULT 10;\n"
					  "  DECLARE V_COUNT INT DEFAULT 0;\n"
					  "  SET V_TOTAL = 0;\n"
					  "  WHILE V_DEPTNO < 100 DO\n"
					  "    SELECT COUNT(*)\n"
					  "      INTO V_COUNT\n"
					  "      FROM TEST.EMP\n"
					  "      WHERE DEPTNO = V_DEPTNO;\n"
					  "    SET V_TOTAL = V_TOTAL + V_COUNT;\n"
					  "    SET V_DEPTNO = V_DEPTNO + 10;\n"
					  "  END WHILE;\n"
					  "END",Example->analyzer()));
#else
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
					  "END;",Example->analyzer()));
#endif
    } TOCATCH
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
      try {
	Example->setText(toSQLParse::indent(Example->text(),Example->analyzer()));
      } TOCATCH
    }
  }
  virtual void saveSetting(void);
};

void toEditExtensions::searchForward(void)
{
  if (Current)
    Current->incrementalSearch(true);
}

void toEditExtensions::searchBackward(void)
{
  if (Current)
    Current->incrementalSearch(false);
}

#define TO_EDIT_SEARCH_NEXT	209

class toEditExtensionTool : public toTool {
public:
  toEditExtensionTool()
    : toTool(910,"Editor Extensions")
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
  virtual QWidget *toolWindow(QWidget *,toConnection &)
  {
    return NULL; // Has no tool window
  }
  virtual void customSetup(int)
  {
    toMainWidget()->editMenu()->insertSeparator();

    int idx=toMainWidget()->editMenu()->indexOf(TO_EDIT_SEARCH_NEXT);

    QPopupMenu *menu=new QPopupMenu(toMainWidget());
    
    IncrementalSearch=menu->insertItem(qApp->translate("toEditExtensionTool","Forward"),&EditExtensions,SLOT(searchForward()),
				       toKeySequence(qApp->translate("toEditExtensionTool","Ctrl+S", "Edit|Incremental search forward")));
    ReverseSearch=menu->insertItem(qApp->translate("toEditExtensionTool","Backward"),&EditExtensions,SLOT(searchBackward()),
				   toKeySequence(qApp->translate("toEditExtensionTool","Ctrl+R", "Edit|Incremental search backward")));

    toMainWidget()->editMenu()->insertItem(qApp->translate("toEditExtensionTool","Incremental Search"),menu,-1,(idx>=0?idx+1:0));
    
    menu=new QPopupMenu(toMainWidget());
    AutoIndentBlock=menu->insertItem(qApp->translate("toEditExtensionTool","Selection"),
				     &EditExtensions,
				     SLOT(autoIndentBlock()),
				     toKeySequence(qApp->translate("toEditExtensionTool","Ctrl+Alt+I", "Edit|Indent selection")));
    AutoIndentBuffer=menu->insertItem(qApp->translate("toEditExtensionTool","Editor"),
				      &EditExtensions,
				      SLOT(autoIndentBuffer()),
				      toKeySequence(qApp->translate("toEditExtensionTool","Ctrl+Alt+Shift+I", "Edit|Indent editor")));
    menu->insertSeparator();
    ObfuscateBlock=menu->insertItem(qApp->translate("toEditExtensionTool","Obfuscate Selection"),
				    &EditExtensions,
				    SLOT(obfuscateBlock()));
    ObfuscateBuffer=menu->insertItem(qApp->translate("toEditExtensionTool","Obfuscate Editor"),
				     &EditExtensions,
				     SLOT(obfuscateBuffer()));
    toMainWidget()->editMenu()->insertItem(qApp->translate("toEditExtensionTool","Auto Indent"),menu);

    menu=new QPopupMenu(toMainWidget());
    UpperCase=menu->insertItem(qApp->translate("toEditExtensionTool","Upper"),
			       &EditExtensions,
			       SLOT(upperCase()),
			       toKeySequence(qApp->translate("toEditExtensionTool","Ctrl+U", "Edit|Uppercase")));
    LowerCase=menu->insertItem(qApp->translate("toEditExtensionTool","Lower"),
			       &EditExtensions,
			       SLOT(lowerCase()),
			       toKeySequence(qApp->translate("toEditExtensionTool","Ctrl+L", "Edit|Lowercase")));
    toMainWidget()->editMenu()->insertItem(qApp->translate("toEditExtensionTool","Modify Case"),menu);

    IndentIndex=toMainWidget()->editMenu()->insertItem(QPixmap((const char **)indent_xpm),
						       qApp->translate("toEditExtensionTool","Indent Block"),&EditExtensions,
						       SLOT(indentBlock()),
						       toKeySequence(qApp->translate("toEditExtensionTool","Alt+Right", "Edit|Indent block")));
    DeindentIndex=toMainWidget()->editMenu()->insertItem(QPixmap((const char **)deindent_xpm),
							 qApp->translate("toEditExtensionTool","De-indent Block"),&EditExtensions,
							 SLOT(deindentBlock()),
							 toKeySequence(qApp->translate("toEditExtensionTool","Alt+Left", "Edit|De-indent block")));
    GotoLine=toMainWidget()->editMenu()->insertItem(qApp->translate("toEditExtensionTool","Goto Line"),&EditExtensions,
						    SLOT(gotoLine()));

    IndentButton=new QToolButton(QPixmap((const char **)indent_xpm),
				 qApp->translate("toEditExtensionTool","Indent block in editor"),
				 qApp->translate("toEditExtensionTool","Indent block in editor"),
				 &EditExtensions,SLOT(indentBlock()),
				 toMainWidget()->editToolbar());
    DeindentButton=new QToolButton(QPixmap((const char **)deindent_xpm),
				   qApp->translate("toEditExtensionTool","De-indent block in editor"),
				   qApp->translate("toEditExtensionTool","De-indent block in editor"),
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

toEditExtensionGoto::toEditExtensionGoto(toMarkedText *editor)
  :toEditExtensionGotoUI(editor,"GotoLine",true),Editor(editor)
{
  toHelp::connectDialog(this);
  Line->setMaxValue(Editor->numLines());
  Line->setMinValue(1);
  {
    int curline,curcol;
    Editor->getCursorPosition(&curline,&curcol);
    Line->setValue(curline);
  }
}

void toEditExtensionGoto::gotoLine()
{
  Editor->setCursorPosition(Line->value()-1,0,false);
}
