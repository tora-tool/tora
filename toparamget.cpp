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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/


#include <qregexp.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qgrid.h>
#include <qscrollview.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qpalette.h>

#include "toparamget.h"
#include "totool.h"
#include "toconf.h"

map<QString,QString> toParamGet::Cache;

toParamGet::toParamGet(QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  resize(470,500);
  setMinimumSize(QSize(470,500));
  setMaximumSize(QSize(470,500));
  setCaption("Define binding variables");

  QScrollView *scroll=new QScrollView(this);
  scroll->setGeometry(10,10,330,480);

  Container=new QGrid(3,scroll->viewport());
  scroll->addChild(Container,5,5);
  Container->setSpacing(10);
  scroll->viewport()->setBackgroundColor(qApp->palette().active().background());

  QPushButton *OkButton = new QPushButton(this,"OkButton");
  OkButton->move(350,10); 
  OkButton->setText(tr("&OK"));
  OkButton->setDefault(true);

  QPushButton *CancelButton=new QPushButton(this,"CancelButton");
  CancelButton->move(350,60); 
  CancelButton->setText(tr("Cancel"));
  CancelButton->setDefault(false);
  
  connect(OkButton,SIGNAL(clicked()),this,SLOT(accept()));
  connect(CancelButton,SIGNAL(clicked()),this,SLOT(reject()));
}

list<QString> toParamGet::getParam(QWidget *parent,QString &str)
{
  map<QString,bool> parameters;
  list<QString> names;
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
  def+=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE);
  def+="]>";

  for(unsigned int i=0;i<str.length()+1;i++) {
    QChar c;
    QChar nc;

    if (i<str.length())
      c=str[i];
    else
      c='\n';
    if (i<str.length()-1)
      nc=str[i+1];
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
	  map<QString,QString>::iterator fnd=Cache.find(fname);
	  QLineEdit *edit=new QLineEdit(widget->Container);
	  QCheckBox *box=new QCheckBox("NULL",widget->Container);
	  connect(box,SIGNAL(toggled(bool)),edit,SLOT(setDisabled(bool)));
	  if (fnd!=Cache.end()) {
	    edit->setText(Cache[fname]);
	    if (Cache[fname].isNull())
	      box->setChecked(true);
	  }
	  widget->Value.insert(widget->Value.end(),edit);
	  names.insert(names.end(),fname);
	}
      }
      fname="";
    }
    res+=c;
  }

  list<QString> ret;
  if (widget) {
    (*widget->Value.begin())->setFocus();
    if (widget->exec()) {
      list<QString>::iterator cn=names.begin();
      for (list<QWidget *>::iterator i=widget->Value.begin();i!=widget->Value.end();i++) {
	QLineEdit *current=dynamic_cast<QLineEdit *>(*i);
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
    } else
      throw QString("Aborted execution");
    delete widget;
  }
  str=res;
  return ret;
}
