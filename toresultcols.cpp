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

#include "utils.h"

#include "toconnection.h"
#include "toresultcols.h"
#include "tosql.h"

#include <map>

#include <qapplication.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qregexp.h>

#include "toresultcols.moc"

toResultColsComment::toResultColsComment(QWidget *parent)
  : QLineEdit(parent)
{
  connect(this,SIGNAL(textChanged(const QString &)),this,SLOT(commentChanged()));
  Cached=NULL;
  Changed=false;
}

void toResultColsComment::setComment(bool table,const QString &name,const QString &comment)
{
  saveUnchanged();
  disconnect(this,SIGNAL(textChanged(const QString &)),this,SLOT(commentChanged()));
  Table=table;
  Name=name;
  Cached=NULL;
  Changed=false;
  setText(comment);
  connect(this,SIGNAL(textChanged(const QString &)),this,SLOT(commentChanged()));
}

void toResultColsComment::setCachedComment(bool table,const QString &name,QString &comment)
{
  saveUnchanged();
  disconnect(this,SIGNAL(textChanged(const QString &)),this,SLOT(commentChanged()));
  Table=table;
  Name=name;
  Cached=&comment;
  Changed=false;
  setText(comment);
  connect(this,SIGNAL(textChanged(const QString &)),this,SLOT(commentChanged()));
}

static toSQL SQLChangeTableComment("toResultCols:ChangeTableComment",
				   "COMMENT ON TABLE %1 IS %2",
				   "Set a comment on a table. Must have same % signs");
static toSQL SQLChangeColumnComment("toResultCols:ChangeColumnComment",
				    "COMMENT ON COLUMN %1 IS %2",
				    "Set a comment on a column. Must have same % signs");

void toResultColsComment::commentChanged()
{
  Changed=true;
}

void toResultColsComment::focusOutEvent(QFocusEvent *e)
{
  QLineEdit::focusOutEvent(e);
  saveUnchanged();
}

void toResultColsComment::saveUnchanged()
{
  try {
    if (!Name.isEmpty()&&Changed) {
      Changed=false;
      toConnection &conn=toCurrentConnection(this);
      QString sql;
      if (Table)
	sql=SQLChangeTableComment(conn);
      else
	sql=SQLChangeColumnComment(conn);
      QString comment=text();
      comment.replace(QRegExp("'"),"''");
      comment="'"+comment+"'";
      conn.execute(sql.arg(Name).arg(comment));
      if (Cached)
	*Cached=text();
    }
  } TOCATCH
}

QWidget *toResultCols::resultColsEdit::createValue(QWidget *parent)
{
  toResultColsComment *widget=new toResultColsComment(parent);
  return widget;
}

void toResultCols::resultColsEdit::setValue(QWidget *widget,const QString &title,const QString &value)
{
  toResultColsComment *comment=dynamic_cast<toResultColsComment *>(widget);
  if (comment) {
    if (Cached)
      comment->setCachedComment(false,Table+"."+connection().quote(title),const_cast<QString &>(value));
    else
      comment->setComment(false,Table+"."+connection().quote(title),value);
  }
}

void toResultCols::resultColsEdit::describe(toQDescList &desc,const QString &table,bool cache)
{
  start();
  Table=table;
  Cached=cache;
  for (toQDescList::iterator i=desc.begin();i!=desc.end();i++)
    addItem((*i).Name,(*i).Comment);
  done();
}

static toSQL SQLInfo("toResultCols:Info",
		     "SELECT Data_Default,\n"
		     "       Num_Distinct,\n"
		     "       Low_Value,\n"
		     "       High_Value,\n"
		     "       Density,\n"
		     "       Num_Nulls,\n"
		     "       Num_Buckets,\n"
		     "       Last_Analyzed,\n"
		     "       Sample_Size,\n"
		     "       Avg_Col_Len\n"
		     "  FROM sys.All_Tab_Columns\n"
		     " WHERE Owner = :f1<char[100]>\n"
		     "   AND Table_Name = :f2<char[100]>\n"
		     "   AND Column_Name = :f3<char[100]>",
		     "Display analyze statistics about a column",
		     "8.1");
static toSQL SQLInfo8("toResultCols:Info",
		      "SELECT Data_Default,\n"
		      "       Num_Distinct,\n"
		      "       Low_Value,\n"
		      "       High_Value,\n"
		      "       Density,\n"
		      "       Num_Nulls,\n"
		      "       Num_Buckets,\n"
		      "       Last_Analyzed,\n"
		      "       Sample_Size,\n"
		      "       NULL\n"
		      "  FROM sys.All_Tab_Columns\n"
		      " WHERE Owner = :f1<char[100]>\n"
		      "   AND Table_Name = :f2<char[100]>\n"
		      "   AND Column_Name = :f3<char[100]>",
		      QString::null,
		      "8.0");
static toSQL SQLInfo7("toResultCols:Info",
		      "SELECT Data_Default,\n"
		      "       Num_Distinct,\n"
		      "       Low_Value,\n"
		      "       High_Value,\n"
		      "       Density,\n"
		      "       NULL,\n"
		      "       NULL,\n"
		      "       NULL,\n"
		      "       NULL,\n"
		      "       NULL\n"
		      "  FROM sys.All_Tab_Columns\n"
		      " WHERE Owner = :f1<char[100]>\n"
		      "   AND Table_Name = :f2<char[100]>\n"
		      "   AND Column_Name = :f3<char[100]>",
		      "",
		      "7.3");

class toResultColsItem : public toResultViewMLine {
public:
  toResultColsItem(QListView *parent,QListViewItem *after)
    : toResultViewMLine(parent,after)
  { }
  virtual QString key (int column,bool ascending)
  {
    if (column==0) {
      QString ret;
      ret.sprintf("%04d",text(0).toInt());
      return ret;
    }
    return toResultViewMLine::key(column,ascending);
  }
  virtual QString tooltip(int col) const
  {
    toResultCols::resultCols *view=dynamic_cast<toResultCols::resultCols *>(listView());
    try {
      toConnection &conn=toCurrentConnection(view);
      if (conn.provider()!="Oracle")
	return QString::null;
      toQList resLst=toQuery::readQueryNull(conn,SQLInfo,
					    text(10),text(11),text(1));
      QString result(QString::fromLatin1("<B>"));
      result+=(text(1));
      result+=QString::fromLatin1("</B><BR><BR>");

      int any=0;
      QString cur=toShift(resLst);
      if (!cur.isEmpty()) {
	result+=qApp->translate("toResultCols","Default value: <B>");
	result+=(cur);
	result+=QString::fromLatin1("</B><BR><BR>");
	any++;
      }

      QString analyze;
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","Distinct values: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","Low value: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","High value: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","Density: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","Number of nulls: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","Number of histogram buckets: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","Last analyzed: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","Sample size: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      cur=toShift(resLst);
      if (!cur.isEmpty()) {
	analyze+=qApp->translate("toResultCols","Average column size: <B>");
	analyze+=(cur);
	analyze+=QString::fromLatin1("</B><BR>");
	any++;
      }
      if (!analyze.isEmpty()) {	
	result+=QString::fromLatin1("<B>Analyze statistics:</B><BR>");
	result+=(analyze);
      }
      if (!any)
	return text(col);
      return result;
    } catch (const QString  &exc) {
      toStatusMessage(exc);
      return text(col);
    }
  }
};

static toSQL SQLTableComment("toResultCols:TableComment",
			     "SELECT Comments FROM sys.All_Tab_Comments\n"
			     " WHERE Owner = :f1<char[100]>\n"
			     "   AND Table_Name = :f2<char[100]>",
			     "Display table comments");

toResultCols::toResultCols(QWidget *parent,const char *name)
  : QVBox(parent,name)
{
  QHBox *box=new QHBox(this);
  Title=new QLabel(box);
  Title->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum));
  Comment=new QLabel(box);
  Comment->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum));
  EditComment=new toResultColsComment(box);
  EditComment->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Maximum));
  EditComment->hide();
  Edit=new QCheckBox("Edit comments",box);
  Edit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Maximum));
  connect(Edit,SIGNAL(toggled(bool)),this,SLOT(editComment(bool)));
  Columns=new resultCols(this);
}

void toResultCols::query(const QString &sql,const toQList &param,bool nocache)
{
  QString Owner;
  QString Name;
  QString object;

  // Intentionally ignore returncode.
  if (!setSQLParams(sql,param)&&!nocache)
    return;

  try {
    toQList subp;

    toConnection &conn=connection();

    toQList::iterator cp=((toQList &)param).begin();
    if (cp!=((toQList &)param).end()) {
      object=conn.quote(*cp);
      Owner=*cp;
    }
    cp++;
    if (cp!=((toQList &)param).end()) {
      object+=QString::fromLatin1(".");
      object+=conn.quote(*cp);
      Name=*cp;
    } else {
      Name=Owner;
      Owner=connection().user().upper();
    }

    QString synonym;

    Columns->clear();
    const toConnection::objectName &name=conn.realName(object,synonym,false);

    QString label=QString::fromLatin1("<B>");
    if (!synonym.isEmpty()) {
      label+=conn.quote(synonym);
      label+=tr("</B> synonym for <B>");
    }
    label+=conn.quote(name.Owner);
    if (label!=QString::fromLatin1("<B>"))
      label+=QString::fromLatin1(".");
    label+=conn.quote(name.Name);
    
    label+=QString::fromLatin1("</B>");
    if (name.Comment) {
      Comment->setText(QString::fromLatin1(" - ")+name.Comment);
      EditComment->setCachedComment(true,
				    conn.quote(name.Owner)+"."+conn.quote(name.Name),
				    const_cast<QString &>(name.Comment));
    } else
      Comment->setText(QString::null);

    if (connection().provider()=="Oracle") {
      editComment(Edit->isChecked());
      Edit->setEnabled(true);
    } else {
      editComment(false);
      Edit->setEnabled(false);
    }

    Columns->query(name,nocache);
    Title->setText(label);
  } catch(const QString &) {
    try {
      QString label=QString::fromLatin1("<B>");
      label+=object;
      label+=QString::fromLatin1("</B>");
      if (connection().provider()=="Oracle") {
	toConnection &conn=connection();
	toQuery query(conn,SQLTableComment,Owner,Name);
	QString t;
	while(!query.eof()) {
	  t+=QString::fromLatin1(" - ");
	  QString comment=query.readValueNull();
	  EditComment->setComment(true,conn.quote(Owner)+"."+conn.quote(Name),comment);
	  t+=comment;
	}
	Comment->setText(t);
	editComment(Edit->isChecked());
	Edit->setEnabled(true);
      } else {
	editComment(false);
	Edit->setEnabled(false);
      }
      label+=" "+tr("(Object cache not ready)");
      Columns->query(object,Owner,Name);
      Title->setText(label);
    } catch(const QString &str) {
      Title->setText(str);
      toStatusMessage(str);
    }
  }
}

toResultCols::resultCols::resultCols(QWidget *parent,const char *name)
  : toListView(parent,name)
{
  addColumn(QString::fromLatin1("#"));
  setColumnAlignment(0,AlignRight);
  addColumn(tr("Column Name"));
  addColumn(tr("Data Type"));
  addColumn(QString::fromLatin1("NULL"));
  addColumn(tr("Comments"));
  setSorting(0);
  Edit=new resultColsEdit(parent);
  Edit->hide();
}

void toResultCols::resultCols::editComment(bool val)
{
  if (val) {
    Edit->show();
    hide();
  } else {
    Edit->hide();
    show();
  }
}

void toResultCols::resultCols::describe(toQDescList &desc)
{
  int col=1;
  for (toQDescList::iterator i=desc.begin();i!=desc.end();i++) {
    QListViewItem *item=new toResultColsItem(this,NULL);
    
    item->setText(0,QString::number(col++));
    item->setText(1,(*i).Name);
    item->setText(2,(*i).Datatype);
    if ((*i).Null)
      item->setText(3,QString::fromLatin1("NULL"));
    else
      item->setText(3,QString::fromLatin1("NOT NULL"));
    item->setText(4,(*i).Comment);
    
    item->setText(10,Owner);
    item->setText(11,Name);
  }
}

static toSQL SQLComment("toResultCols:Comments",
			"SELECT Column_name,Comments FROM sys.All_Col_Comments\n"
			" WHERE Owner = :f1<char[100]>\n"
			"   AND Table_Name = :f2<char[100]>",
			"Display column comments");

void toResultCols::resultCols::query(const QString &object,
				     const QString &owner,const QString &name)
{
  try {
    toConnection &conn=toCurrentConnection(this);

    QString table;

    QString sql=QString::fromLatin1("SELECT * FROM ");
    if(conn.provider() == "PostgreSQL")
      table=name;
    else
      table=object;
    sql+=table;
    sql+=QString::fromLatin1(" WHERE NULL=NULL");

    setSQLName(tr("Description of %1").
	       arg(object));

    toQuery query(conn,sql);
    toQDescList desc=query.describe();

    if (conn.provider()=="Oracle") {
      try {
	toQuery query(conn,SQLComment,owner,name);
	while(!query.eof()) {
	  QString col=query.readValueNull();
	  QString com=query.readValueNull();
	  for(toQDescList::iterator i=desc.begin();i!=desc.end();i++) {
	    if ((*i).Name==col) {
	      (*i).Comment=com;
	      break;
	    }
	  }
	}
      } TOCATCH
    }

    Owner=owner;
    Name=name;

    describe(desc);
    Edit->describe(desc,table,false);
  } catch(...) {
    toStatusMessage(tr("Failed to describe %1").arg(object));
  }
}

void toResultCols::resultCols::query(const toConnection::objectName &name,bool nocache)
{
  try {
    clear();
    toConnection &conn=toCurrentConnection(this);

    Owner=name.Owner;
    Name=name.Name;

    QString wholename=conn.quote(Owner)+"."+conn.quote(Name);

    setSQLName(tr("Description of %1").
	       arg(wholename));

    toQDescList &desc=conn.columns(name,nocache);

    describe(desc);
    Edit->describe(desc,wholename,true);
  } catch(...) {
    toStatusMessage(tr("Failed to describe %1").arg(name.Owner+QString::fromLatin1(".")+name.Name));
  }
}

void toResultCols::editComment(bool val)
{
  Columns->editComment(val);
  if (val) {
    Comment->hide();
    EditComment->show();
  } else {
    Comment->show();
    EditComment->hide();
  }
}
