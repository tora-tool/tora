#include <stdio.h>

#include <qfontdialog.h>
#include <qcolordialog.h>

#include "tohighlightedtext.h"
#include "tosyntaxsetup.h"
#include "toconf.h"

#include "tosyntaxsetup.ui.moc"
#include "tosyntaxsetup.ui.cpp"

toSyntaxSetup::toSyntaxSetup(QWidget *parent,const char *name,WFlags fl)
  : toSyntaxSetupUI(parent,name,fl)
{
  if (!toTool::globalConfig(CONF_KEYWORD_UPPER,"").isEmpty())
    KeywordUpper->setChecked(true);
  if (!toTool::globalConfig(CONF_HIGHLIGHT,"Yes").isEmpty())
    SyntaxHighlighting->setChecked(true);

  QFont font( "Courier", 12, QFont::Bold);
  QString fnt=toTool::globalConfig(CONF_TEXT,"");
  if (fnt.length())
    font.setRawName(fnt);
  Text=font.rawName();
  CodeExample->setFont(font);
  Colors["Background"]=toDefaultAnalyzer().getColor(toSyntaxAnalyzer::NormalBkg);
  Colors["Error background"]=toDefaultAnalyzer().getColor(toSyntaxAnalyzer::ErrorBkg);
  Colors["Keyword"]=toDefaultAnalyzer().getColor(toSyntaxAnalyzer::Keyword);
  Colors["Comment"]=toDefaultAnalyzer().getColor(toSyntaxAnalyzer::Comment);
  Colors["Normal"]=toDefaultAnalyzer().getColor(toSyntaxAnalyzer::Normal);
  Colors["String"]=toDefaultAnalyzer().getColor(toSyntaxAnalyzer::String);
  Colors["Unfinished string"]=toDefaultAnalyzer().getColor(toSyntaxAnalyzer::Error);

  SyntaxComponent->insertItem("Background");
  SyntaxComponent->insertItem("Comment");
  SyntaxComponent->insertItem("Error background");
  SyntaxComponent->insertItem("Keyword");
  SyntaxComponent->insertItem("Normal");
  SyntaxComponent->insertItem("String");
  SyntaxComponent->insertItem("Unfinished string");

  Current=NULL;
}

void toSyntaxAnalyzer::readColor(const QString &str,const QColor &def,int pos)
{
  QString conf(CONF_COLOR);
  conf+="\\";
  conf+=str;
  QString res=toTool::globalConfig(conf,"");
  if (res.isEmpty())
    Colors[pos]=def;
  else {
    int r,g,b;
    if (sscanf(res,"%d,%d,%d",&r,&g,&b)!=3)
      throw QString("Wrong format of color in setings");
    QColor col(r,g,b);
    Colors[pos]=col;
  }
}

void toSyntaxAnalyzer::updateSettings(void)
{
  const QColorGroup &cg=qApp->palette().active();
  readColor("Background",cg.base(),NormalBkg);
  readColor("Error background",Qt::red,ErrorBkg);
  readColor("Keyword",Qt::blue,Keyword);
  readColor("Normal",cg.text(),Normal);
  readColor("String",Qt::red,String);
  readColor("Unfinished string",Qt::red,Error);
  readColor("Comment",Qt::green,Comment);
}

void toSyntaxSetup::selectFont(void)
{
  bool ok=true;
  QFont font;
  font.setRawName(Text);
  font=QFontDialog::getFont (&ok,font,this);
  if (ok) {
    Text=font.rawName();
    CodeExample->setFont(font);
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
    }
  }
}

void toSyntaxSetup::saveSetting(void)
{
  toTool::globalSetConfig(CONF_TEXT,Text);
  toTool::globalSetConfig(CONF_HIGHLIGHT,SyntaxHighlighting->isChecked()?"Yes":"");
  toTool::globalSetConfig(CONF_KEYWORD_UPPER,KeywordUpper->isChecked()?"Yes":"");
  for (map<QString,QColor>::iterator i=Colors.begin();i!=Colors.end();i++) {
    QString str(CONF_COLOR);
    str+="\\";
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
