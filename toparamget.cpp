//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qregexp.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qgrid.h>
#include <qscrollview.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qpalette.h>
#include <qlayout.h>
#include <qsizepolicy.h>

#include "toparamget.h"
#include "totool.h"
#include "toconf.h"
#include "tomain.h"
#include "tohelp.h"
#include "tomemoeditor.h"

#include "toparamget.moc"

std::map<QString,QString> toParamGet::DefaultCache;
std::map<QString,QString> toParamGet::Cache;

toParamGet::toParamGet(QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  toHelp::connectDialog(this);
  resize(500,480);
  setCaption("Define binding variables");

  QGridLayout *layout=new QGridLayout(this,3,2);
  layout->setSpacing( 6 );
  layout->setMargin( 11 );

  QScrollView *scroll=new QScrollView(this);
  scroll->enableClipper(true);
  scroll->setGeometry(10,10,330,480);
  scroll->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
  layout->addMultiCellWidget(scroll,0,2,0,0);

  Container=new QGrid(4,scroll->viewport());
  scroll->addChild(Container,5,5);
  Container->setSpacing(10);
  scroll->viewport()->setBackgroundColor(qApp->palette().active().background());

  QPushButton *OkButton = new QPushButton(this,"OkButton");
  OkButton->setText(tr("&OK"));
  OkButton->setDefault(true);
  layout->addWidget(OkButton,0,1);

  QPushButton *CancelButton=new QPushButton(this,"CancelButton");
  CancelButton->setText(tr("Cancel"));
  CancelButton->setDefault(false);
  layout->addWidget(CancelButton,1,1);

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  layout->addItem(spacer,2,1);

  connect(OkButton,SIGNAL(clicked()),this,SLOT(accept()));
  connect(CancelButton,SIGNAL(clicked()),this,SLOT(reject()));
}

toQList toParamGet::getParam(QWidget *parent,QString &str)
{
  std::map<QString,bool> parameters;
  std::list<QString> names;
  toParamGet *widget=NULL;

  enum {
    afterName,
    inString,
    normal,
    comment,
    name,
    specification,
    endInput
  } state;
  state=normal;

  QChar endString;
  QString fname;
  QString direction;
  QString res;

  QString def="<char[";
  def+=QString::number(min(toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt(),3999));
  def+="]>";

  int num=0;
  for(unsigned int i=0;i<str.length()+1;i++) {
    QChar c;
    QChar nc;

    if (i<str.length())
      c=str.at(i);
    else
      c='\n';
    if (i<str.length()-1)
      nc=str.at(i+1);
    else
      nc='\n';

    if (state==normal&&c=='-'&&nc=='-')
      state=comment;
    else {
      switch(state) {
      case inString:
	if (c==endString)
	  state=normal;
	break;
      case comment:
	if (c=='\n')
	  state=normal;
	break;
      case normal:
	switch(c) {
	case '\'':
	case '\"':
	  endString=c;
	  state=inString;
	  break;
	case ':':
	  if (nc!='=')
	    state=name;
	  direction="";
	  fname="";
	  break;
	}
	break;
      case name:
	if (c.isLetterOrNumber()||c=='_') {
	  fname+=c;
	  break;
	}
	if (fname.isEmpty())
	  throw QString("Missing field name");
	state=afterName;
      case afterName:
	if (c=='<')
	  state=specification;
	else {
	  state=normal;
	  res+=def;
	}
	break;
      case specification:
	if (c==',')
	  state=endInput;
	else if (c=='>')
	  state=normal;
	break;
      case endInput:
	if (c=='>')
	  state=normal;
	else
	  direction+=c;
	break;
      }
    }
    if (state==normal&&!fname.isEmpty()) {
#if 0
      fname.replace(QRegExp("_")," ");
#endif
      if (direction.isEmpty()||direction=="in"||direction=="inout") {
	if (!parameters[fname]) {
	  parameters[fname]=true;
	  if (!widget)
	    widget=new toParamGet(parent);
	  new QLabel(fname,widget->Container);
	  std::map<QString,QString>::iterator fnd=Cache.find(fname);
	  bool found=true;
	  if (fnd==Cache.end()) {
	    fnd=DefaultCache.find(fname);
	    if (fnd==DefaultCache.end())
	      found=false;
	  }
	  QLineEdit *edit=new QLineEdit(widget->Container,QString::number(num));
	  QCheckBox *box=new QCheckBox("NULL",widget->Container);
	  connect(box,SIGNAL(toggled(bool)),edit,SLOT(setDisabled(bool)));
	  if (found) {
	    edit->setText((*fnd).second);
	    if ((*fnd).second.isNull())
	      box->setChecked(true);
	  }
	  toParamGetButton *btn=new toParamGetButton(num,widget->Container);
	  btn->setText("Edit");
	  btn->setSizePolicy(QSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed));
	  connect(btn,SIGNAL(clicked(int)),widget,SLOT(showMemo(int)));
	  connect(box,SIGNAL(toggled(bool)),btn,SLOT(setDisabled(bool)));
	  widget->Value.insert(widget->Value.end(),edit);
	  names.insert(names.end(),fname);
	  num++;
	}
      }
      fname="";
    }
    if (i<str.length())
      res+=c;
  }

  toQList ret;
  if (widget) {
    (*widget->Value.begin())->setFocus();
    if (widget->exec()) {
      std::list<QString>::iterator cn=names.begin();
      for (std::list<QLineEdit *>::iterator i=widget->Value.begin();i!=widget->Value.end();i++) {
	QLineEdit *current=*i;
	QString val;
	if (current) {
	  if (current->isEnabled())
	    val=current->text();
	  else
	    val=QString::null;
	}
	if (cn!=names.end()) {
	  Cache[*cn]=val;
	  cn++;
	}
	ret.insert(ret.end(),val);
      }
      delete widget;
    } else {
      delete widget;
      throw QString("Aborted execution");
    }
  }
  str=res;
  return ret;
}

void toParamGet::setDefault(const QString &name,const QString &val)
{
  DefaultCache[name]=val;
}

void toParamGet::showMemo(int row)
{
  QObject *obj=child(QString::number(row));
  if (obj) {
    toMemoEditor *memo=new toMemoEditor(this,((QLineEdit *)obj)->text(),row,0,false,true);
    if (memo->exec())
      ((QLineEdit *)obj)->setText(memo->text());
  }
}
