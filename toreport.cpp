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

#include "toconf.h"
#include "toconnection.h"
#include "tohtml.h"
#include "toextract.h"
#include "toreport.h"

#ifndef WIN32
#include <unistd.h>
#else
#include <winsock.h>
#endif

#include <qdatetime.h>

static bool SameContext(const QString &str,const QString &str2)
{
  if (str.startsWith(str2)) {
    if (str.length()==str2.length())
      return true;
    if (str.length()>str2.length()&&
	str[str2.length()]==QChar('\001'))
      return true;
  }
  return false;
}

static bool HasChildren(std::list<QString> &desc,
			std::list<QString>::iterator i,
			const QString &context)
{
  if ((*i).length()>context.length())
    return true;
  i++;
  if (i==desc.end())
    return false;
  return SameContext(*i,context);
}

static std::list<QString>::iterator FindItem(std::list<QString> &desc,
					     std::list<QString>::iterator i,
					     int level,
					     const QString &context,
					     const QString &search)
{
  while(i!=desc.end()) {
    if (!SameContext(*i,context)&&!context.isEmpty())
      return desc.end();
    if (toExtract::partDescribe(*i,level)==search)
      return i;
    i++;
  }
  return desc.end();
}

static QString GenerateConstraint(const QString &name,
				  const QString &def,
				  const QString &status)
{
  return QString("<TR><TD VALIGN=top WIDTH=25%>%1</TD><TD VALIGN=top WIDTH=50%>%2</TD><TD VALIGN=top WIDTH=25%>%3</TD></TR>\n").
    arg(toHtml::escape(name)).
    arg(toHtml::escape(def)).
    arg(toHtml::escape(status));
}

static QString DescribeConstraints(std::list<QString> &desc,
				   std::list<QString>::iterator i,
				   int level,
				   const QString &context)
{
  QString ret=
    "<H4>CONSTRAINTS</H4>\n"
    "<TABLE BORDER=0 WIDTH=100%><TR><TH ALIGN=left WIDTH=25%>Name</TH><TH ALIGN=left WIDTH=50%>Constraint</TH><TH ALIGN=left WIDTH=25%>Status</TH></TR>\n";

  QString lastName;
  QString status;
  QString definition;

  do {
    if (!SameContext(*i,context))
      break;

    QString name=toExtract::partDescribe(*i,level);
    QString extra=toExtract::partDescribe(*i,level+1);

    if (lastName!=name) {
      if (!lastName.isNull())
	ret+=GenerateConstraint(lastName,definition,status);
      definition=QString::null;
      status=QString::null;
      lastName=name;
    }

    if (extra=="DEFINITION")
      definition+=toExtract::partDescribe(*i,level+2)+" ";
    else if (extra=="STATUS") {
      QString t=toExtract::partDescribe(*i,level+2);
      if (t.startsWith("ENABLE")||t.startsWith("DISABLE"))
	status+=t+" ";;
    }

    i++;
  } while(i!=desc.end());
  ret+=GenerateConstraint(lastName,definition,status);
  ret+="</TABLE>\n";
  return ret;
}

struct toReportColumn {
  QString Order;
  QString Datatype;
  QString Comment;
};

static QString DescribeColumns(std::list<QString> &desc,
			       std::list<QString>::iterator i,
			       int level,
			       const QString &context)
{
  bool hasComments=false;
  bool hasDatatype=false;

  int maxCol=1;

  std::map<QString,toReportColumn> cols;
  do {
    if (!SameContext(*i,context))
      break;

    QString col=toExtract::partDescribe(*i,level+0);
    QString extra=toExtract::partDescribe(*i,level+1);

    if (extra=="ORDER") {
      cols[col].Order=toExtract::partDescribe(*i,level+2);
      maxCol=max(maxCol,cols[col].Order.toInt());
    } else if (extra=="COMMENT") {
      cols[col].Comment=toExtract::partDescribe(*i,level+2);
      hasComments=true;
    } else if (!extra.isEmpty()) {
      cols[col].Datatype+=extra+" ";
      hasDatatype=true;
    }

    i++;
  } while(i!=desc.end());

  QString ret=
    "<H4>COLUMNS</H4>\n"
    "<TABLE WIDTH=100% BORDER=0><TR><TH ALIGN=left WIDTH=25%>Name</TH>";
  if (hasDatatype)
    ret+="<TH ALIGN=left WIDTH=25%>Definition</TH>";
  if (hasComments)
    ret+="<TH ALIGN=left WIDTH=50%>Description</TH>";
  ret+="</TR>\n";

  for(int j=1;j<=maxCol;j++) {
    for(std::map<QString,toReportColumn>::iterator k=cols.begin();k!=cols.end();k++) {
      if ((*k).second.Order.toInt()==j) {
	ret+="<TR><TD VALIGN=top WIDTH=25%>";
	ret+=toHtml::escape((*k).first);
	ret+="</TD>";
	if (hasDatatype) {
	  ret+="<TD VALIGN=top WIDTH=25%>";
	  if (!(*k).second.Datatype.isEmpty())
	    ret+=toHtml::escape((*k).second.Datatype);
	  else
	    ret+="<BR>";
	  ret+="</TD>";
	}
	if (hasComments) {
	  ret+="<TD VALIGN=top WIDTH=50%>";
	  if (!(*k).second.Comment.isEmpty())
	    ret+=toHtml::escape((*k).second.Comment);
	  else
	    ret+="&nbsp;";
	  ret+="</TD>";
	}
	ret+="</TR>\n";
	break;
      }
    }
  }
  ret+="</TABLE>\n";
  return ret;
}

static QString ValidPart(std::list<QString> &desc,
			 std::list<QString>::iterator &i,
			 int level)
{
  QString part=toExtract::partDescribe(*i,level);
  while(part.isNull()&&i!=desc.end()) {
    i++;
    part=toExtract::partDescribe(*i,level);
  }
  return part;
}

static QString DescribePart(std::list<QString> &desc,
			    std::list<QString>::iterator &i,
			    int level,
			    const QString &parentContext)
{
  QString ret;
  QString text;

  QString lastPart;

  std::list<QString>::iterator start=i;

  do {
    if (!SameContext(*i,parentContext))
      break;
    QString part=ValidPart(desc,i,level);

    do {
      QString part=toExtract::partDescribe(*i,level);
    } while (part.isNull());
    if (lastPart!=part) {
      if (lastPart.isNull()) {
	i=FindItem(desc,start,level,parentContext,"TABLE");
	if (i==desc.end()) {
	  i=start;
	  part=ValidPart(desc,i,level);
	} else
	  part=toExtract::partDescribe(*i,level);
      } else {
	while(part=="TABLE") {
	  i++;
	  if (i==desc.end())
	    return ret;
	  part=toExtract::partDescribe(*i,level);
	}
      }
      if (lastPart=="TABLE") {
	i=start;
	part=ValidPart(desc,i,level);
      }
      lastPart=part;
    }
    QString child=parentContext+"\001"+part;

    if (part=="COLUMN"||part=="COMMENT"||part=="CONSTRAINT") {
      i++;
    } else if (HasChildren(desc,i,child)) {
      ret+=QString("<P><H%1>%2</H%3></P>\n").
	arg(level+1).
	arg(toHtml::escape(part)).
	arg(level+1);
      std::list<QString>::iterator com=FindItem(desc,
						i,
						level+1,
						child,
						"COMMENT");
      if (com!=desc.end())
	ret+="<P>"+toHtml::escape(toExtract::partDescribe(*com,level+2))+"</P>";

      std::list<QString>::iterator col=FindItem(desc,
						i,
						level+1,
						child,
						"COLUMN");

      if (col!=desc.end())
	ret+=DescribeColumns(desc,col,level+2,child+"\001COLUMN");

      std::list<QString>::iterator con=FindItem(desc,
						i,
						level+1,
						child,
						"CONSTRAINT");

      if (con!=desc.end())
 	ret+=DescribeConstraints(desc,con,level+2,child+"\001CONSTRAINT");

      ret+=DescribePart(desc,i,level+1,child);
    } else {
      if (!part.isEmpty())
	text+="<P>"+toHtml::escape(part)+"</P>\n";
      i++;
    }
  } while(i!=desc.end());
  return text+ret;
}

QString toGenerateReport(toConnection &conn,std::list<QString> &desc)
{
  char host[1024];
  gethostname(host,1024);

  QString db=conn.host();
  if (db.length()&&db!="*")
    db+=":";
  else
    db=QString::null;
  db+=conn.database();

  QString ret=QString("<HTML><HEAD><TITLE>Report on database %7</TITLE></HEAD>\n"
		      "<BODY><H1>Report on database %8</H1>\n"
		      "<TABLE BORDER=0>\n"
		      "<TR><TD VALIGN=top>Generated by:</TD><TD VALIGN=top>TOra, Version %1</TD></TR>\n"
		      "<TR><TD VALIGN=top>At:</TD><TD VALIGN=top>%2</TD></TR>\n"
		      "<TR><TD VALIGN=top>From:</TD><TD VALIGN=top>%3, an %4 %5 database</TD></TR>\n"
		      "<TR><TD VALIGN=top>On:</TD><TD VALIGN=top>%6</TD></TR>\n"
		      "</TABLE>\n").
    arg(TOVERSION).
    arg(host).
    arg(db).
    arg(conn.provider()).
    arg(conn.version()).
    arg(QDateTime::currentDateTime().toString()).
    arg(db).
    arg(db);

  std::list<QString>::iterator i;
  i=FindItem(desc,desc.begin(),0,QString::null,"NONE");
  if (i!=desc.end()) {
    ret+="<H1>Global Objects</H1>\n";
    ret+=DescribePart(desc,i,1,"NONE");
  }

  i=desc.begin();
  QString lastContext;
  while(i!=desc.end()) {
    QString context=toExtract::partDescribe(*i,0);
    if (context!="NONE") {
      if (context!=lastContext) {
	if (context=="public")
	  ret+="<H1>Public</H1>";
	else
	  ret+="<H1>Schema "+toHtml::escape(context)+"</H1>\n";
      }
      ret+=DescribePart(desc,i,1,context);
      lastContext=context;
    } else
      i++;
  }
  ret+="</BODY>\n</HTML>";

  return ret;

}
