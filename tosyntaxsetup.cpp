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

#include <stdio.h>
#ifdef TO_KDE
#  include <kfontdialog.h>
#else
#  include <qfontdialog.h>
#endif
#include <qfont.h>
#include <qcolordialog.h>
#include <qapplication.h>
#include <qlistview.h>

#include "tohighlightedtext.h"
#include "tosyntaxsetup.h"
#include "toconf.h"
#include "tomain.h"

#include "tosyntaxsetup.ui.moc"
#include "tosyntaxsetup.ui.cpp"

toSyntaxSetup::toSyntaxSetup(QWidget *parent,const char *name,WFlags fl)
  : toSyntaxSetupUI(parent,name,fl),toSettingTab("fonts.html"),Analyzer(toDefaultAnalyzer())
{
  if (!toTool::globalConfig(CONF_KEYWORD_UPPER,"").isEmpty())
    KeywordUpper->setChecked(true);
  if (!toTool::globalConfig(CONF_HIGHLIGHT,"Yes").isEmpty())
    SyntaxHighlighting->setChecked(true);

  {
    QFont font(toStringToFont(toTool::globalConfig(CONF_TEXT,"")));
    Text=toFontToString(font);
    checkFixedWidth(font);
    CodeExample->setFont(font);
  }
  {
    QString str=toTool::globalConfig(CONF_LIST,"");
    QFont font;
    if (str.isEmpty()) {
      QWidget *wid=new QListView;
      font=qApp->font(wid);
    } else {
      font=toStringToFont(str);
    }
    List=toFontToString(font);
    ResultExample->setFont(font);
  }
  Colors[Analyzer.typeString(toSyntaxAnalyzer::NormalBkg)]=Analyzer.getColor(toSyntaxAnalyzer::NormalBkg);
  Colors[Analyzer.typeString(toSyntaxAnalyzer::ErrorBkg)]=Analyzer.getColor(toSyntaxAnalyzer::ErrorBkg);
  Colors[Analyzer.typeString(toSyntaxAnalyzer::CurrentBkg)]=Analyzer.getColor(toSyntaxAnalyzer::CurrentBkg);
  Colors[Analyzer.typeString(toSyntaxAnalyzer::Keyword)]=Analyzer.getColor(toSyntaxAnalyzer::Keyword);
  Colors[Analyzer.typeString(toSyntaxAnalyzer::Comment)]=Analyzer.getColor(toSyntaxAnalyzer::Comment);
  Colors[Analyzer.typeString(toSyntaxAnalyzer::Normal)]=Analyzer.getColor(toSyntaxAnalyzer::Normal);
  Colors[Analyzer.typeString(toSyntaxAnalyzer::String)]=Analyzer.getColor(toSyntaxAnalyzer::String);
  Colors[Analyzer.typeString(toSyntaxAnalyzer::Error)]=Analyzer.getColor(toSyntaxAnalyzer::Error);

  SyntaxComponent->insertItem(Analyzer.typeString(toSyntaxAnalyzer::NormalBkg));
  SyntaxComponent->insertItem(Analyzer.typeString(toSyntaxAnalyzer::Comment));
  SyntaxComponent->insertItem(Analyzer.typeString(toSyntaxAnalyzer::CurrentBkg));
  SyntaxComponent->insertItem(Analyzer.typeString(toSyntaxAnalyzer::ErrorBkg));
  SyntaxComponent->insertItem(Analyzer.typeString(toSyntaxAnalyzer::Keyword));
  SyntaxComponent->insertItem(Analyzer.typeString(toSyntaxAnalyzer::Normal));
  SyntaxComponent->insertItem(Analyzer.typeString(toSyntaxAnalyzer::String));
  SyntaxComponent->insertItem(Analyzer.typeString(toSyntaxAnalyzer::Error));

  Example->setAnalyzer(Analyzer);
  Example->setReadOnly(true);
  Example->setText("create procedure CheckObvious as\n"
		   "begin\n"
		   "  GlobeCom:='Great'; -- This variable doesn't exist\n"
		   "  if GlobeCom = 'Great' then\n"
		   "    Obvious(true);\n"
		   "  end if;\n"
		   "end;");
  Example->setCurrent(4);
  map<int,QString> Errors;
  Errors[2]="Unknown variable";
  Example->setErrors(Errors);

  Current=NULL;
}

void toSyntaxAnalyzer::readColor(const QColor &def,infoType typ)
{
  QString str=typeString(typ);
  QString conf(CONF_COLOR);
  conf+=":";
  conf+=str;
  QString res=toTool::globalConfig(conf,"");
  if (res.isEmpty())
    Colors[typ]=def;
  else {
    int r,g,b;
    if (sscanf(res,"%d,%d,%d",&r,&g,&b)!=3)
      throw QString("Wrong format of color in setings");
    QColor col(r,g,b);
    Colors[typ]=col;
  }
}

toSyntaxAnalyzer::infoType toSyntaxAnalyzer::typeString(const QString &str)
{
  if(str=="Normal")
    return Normal;
  if(str=="Keyword")
    return Keyword;
  if(str=="String")
    return String;
  if(str=="Unfinished string")
    return Error;
  if(str=="Comment")
    return Comment;
  if(str=="Error background")
    return ErrorBkg;
  if(str=="Background")
    return NormalBkg;
  if(str=="Current background")
    return CurrentBkg;
  throw QString("Unknown type");
}

QString toSyntaxAnalyzer::typeString(infoType typ)
{
  switch(typ) {
  case Normal:
    return "Normal";
  case Keyword:
    return "Keyword";
  case String:
    return "String";
  case Error:
    return "Unfinished string";
  case Comment:
    return "Comment";
  case ErrorBkg:
    return "Error background";
  case NormalBkg:
    return "Background";
  case CurrentBkg:
    return "Current background";
  }
  throw QString("Unknown type");
}

void toSyntaxAnalyzer::updateSettings(void)
{
  const QColorGroup &cg=qApp->palette().active();
  readColor(cg.base(),NormalBkg);
  readColor(Qt::darkRed,ErrorBkg);
  readColor(Qt::darkGreen,CurrentBkg);
  readColor(Qt::blue,Keyword);
  readColor(cg.text(),Normal);
  readColor(Qt::red,String);
  readColor(Qt::red,Error);
  readColor(Qt::green,Comment);
}

void toSyntaxSetup::checkFixedWidth(const QFont &fnt)
{
  QFontMetrics mtr(fnt);
  if (mtr.width("iiiiiiii")==mtr.width("MMMMMMMM"))
    KeywordUpper->setEnabled(true);
  else {
    KeywordUpper->setChecked(false);
    KeywordUpper->setEnabled(false);
  }
}

void toSyntaxSetup::selectFont(void)
{
#ifdef TO_KDE
  QFont font=toStringToFont(Text);
  bool ok=KFontDialog::getFont(font,false,this);
#else
  bool ok=true;
  QFont font=QFontDialog::getFont (&ok,toStringToFont(Text),this);
#endif
  if (ok) {
    Text=toFontToString(font);
    CodeExample->setFont(font);
    Example->setFont(font);
    checkFixedWidth(font);
  }
}

void toSyntaxSetup::selectResultFont(void)
{
#ifdef TO_KDE
  QFont font=toStringToFont(List);
  bool ok=KFontDialog::getFont(font,false,this);
#else
  bool ok=true;
  QFont font=QFontDialog::getFont (&ok,toStringToFont(List),this);
#endif
  if (ok) {
    List=toFontToString(font);
    ResultExample->setFont(font);
  }
}

void toSyntaxSetup::changeLine(QListBoxItem *item)
{
  Current=item;
  if (Current) {
    QColor col=Colors[Current->text()];
    ExampleColor->setBackgroundColor(col);
  }
}

void toSyntaxSetup::selectColor(void)
{
  if (Current) {
    QColor col=QColorDialog::getColor(Colors[Current->text()]);
    if (col.isValid()) {
      Colors[Current->text()]=col;
      ExampleColor->setBackgroundColor(col);
      Example->analyzer().Colors[toSyntaxAnalyzer::typeString(Current->text())]=col;
      Example->update();
    }
  }
}

void toSyntaxSetup::saveSetting(void)
{
  toTool::globalSetConfig(CONF_TEXT,Text);
  toTool::globalSetConfig(CONF_LIST,List);
  toTool::globalSetConfig(CONF_HIGHLIGHT,SyntaxHighlighting->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_KEYWORD_UPPER,KeywordUpper->isChecked()?"Yes":"");
  for (map<QString,QColor>::iterator i=Colors.begin();i!=Colors.end();i++) {
    QString str(CONF_COLOR);
    str+=":";
    str+=(*i).first;
    QString res;
    res.sprintf("%d,%d,%d",
		(*i).second.red(),
		(*i).second.green(),
		(*i).second.blue());
    toTool::globalSetConfig(str,res);
  }
  toDefaultAnalyzer().updateSettings();
}
