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
#include "toextract.h"

#ifndef WIN32
#include <unistd.h>
#else
#include <winsock.h>
#endif
#include <stdio.h>

#include <qapplication.h>
#include <qlabel.h>
#include <qprogressdialog.h>
#include <qdatetime.h>

std::map<QString,toExtract::extractor *> *toExtract::Extractors;

void toExtract::extractor::registerExtract(const QString &db,
					   const QString &oper,
					   const QString &type)
{
  toExtract::allocExtract();

  QString name=toExtract::extractorName(db,oper,type);

  if ((*toExtract::Extractors).find(name)!=(*toExtract::Extractors).end())
    printf("Extractor %s multiply defined\n",(const char *)name);

  (*toExtract::Extractors)[name]=this;
}

void toExtract::extractor::unregisterExtract(const QString &db,
					     const QString &oper,
					     const QString &type)
{
  toExtract::allocExtract();

  QString name=toExtract::extractorName(db,oper,type);
  std::map<QString,extractor *>::iterator i=(*toExtract::Extractors).find(name);
  if (i==(*toExtract::Extractors).end())
    printf("Extractor %s not defined on unregistering\n",(const char *)name);
  else
    (*toExtract::Extractors).erase(i);
}

toExtract::extractor::extractor()
{
}

toExtract::extractor::~extractor()
{
}

void toExtract::allocExtract(void)
{
  if (!Extractors)
    Extractors=new std::map<QString,extractor *>;
}

QString toExtract::extractorName(const QString &db,const QString &oper,const QString &type)
{
  QString ret(db);
  if (!oper.isEmpty())
    ret+=":"+oper;
  if (!type.isEmpty())
    ret+=":"+type;
  return ret;
}

toExtract::extractor *toExtract::findExtractor(toConnection &conn,
					       const QString &oper,
					       const QString &type)
{
  allocExtract();

  QString name=extractorName(conn.provider(),oper,type);
  std::map<QString,extractor *>::iterator i=(*Extractors).find(name);
  if (i!=(*Extractors).end())
    return (*i).second;
  return NULL;
}

void toExtract::initialize(void)
{
  if (!Initialized) {
    Initialized=true;
    extractor *ext=findExtractor(QString::null,QString::null);
    if (ext)
      ext->initialize(*this);
  }
}

toExtract::toExtract(toConnection &conn,QWidget *parent)
  : Connection(conn),Parent(parent)
{
  Heading=true;
  Prompt=true;
  Constraints=true;
  Indexes=true;
  Grants=true;
  Storage=true;
  Parallel=true;
  Contents=true;
  Comments=true;
  Partition=true;
  Schema="1";
  Initialized=false;
  BlockSize=8192;
}

void toExtract::setState(const QString &name,const QVariant &val)
{
  Context[name]=val;
}

QVariant toExtract::state(const QString &name)
{
  return Context[name];
}

void toExtract::srcDst2DropCreate(std::list<QString> &source,std::list<QString> &destination,
				  std::list<QString> &drop,std::list<QString> &create)
{
  drop.clear();
  create.clear();

  std::list<QString>::iterator i=source.begin();
  std::list<QString>::iterator j=destination.begin();
  while(i!=source.end()&&j!=destination.end()) {
    if (*i!=*j) {
      if (*i<*j) {
	drop.insert(drop.end(),*i);
	i++;
      } else {
	create.insert(create.end(),*j);
	j++;
      }
    }
  }
  while(i!=source.end()) {
    drop.insert(drop.end(),*i);
    i++;
  }
  while(j!=destination.end()) {
    create.insert(create.end(),*j);
    j++;
  }
}

void toExtract::parseObject(const QString &object,QString &owner,QString &name)
{
  try {
    unsigned int search=0;
    if (object[0]=='\"') {
      int pos=object.find('\"',1);
      if (pos<0)
	throw 1;
      owner=object.left(pos);
      owner=owner.right(owner.length()-1);
      search=pos+1;
    } else {
      int pos=object.find('.');
      if (pos<0)
	pos=object.length();
      owner=object.left(pos).upper();
      search=pos;
    }
    if (search>=object.length()) {
      name=owner;
      owner=Connection.user();
    } else if (object.at(search)!='.')
      throw 2;
    search++;
    if (object.at(search)=='\"') {
      int pos=object.find('\"',search+1);
      if (pos<0)
	throw 3;
      name=object.left(pos);
      name=owner.right(pos-search-1);
      search=pos+1;
      if (search<object.length())
	throw 4;
    } else {
      name=object.right(object.length()-search).upper();
    }
  } catch(int i) {
    QString ret="Failed to extract owner and name from (";
    ret+=object;
    ret+="), internal error ";
    ret+=QString::number(i);
    throw ret;
  }
}

bool toExtract::canHandle(toConnection &conn)
{
  return bool(findExtractor(conn,QString::null,QString::null));
}

QString toExtract::create(std::list<QString> &objects)
{
  QString ret=generateHeading("CREATE",objects);

  QProgressDialog *progress=NULL;
  QLabel *label;
  if (Parent) {
    progress=new QProgressDialog("Creating create script","&Cancel",
				 objects.size(),Parent,"progress",true);
    progress->setCaption("Creating script");
    label=new QLabel(progress);
    progress->setLabel(label);
  }
  initialize();

  try {
    int num=1;
    for (std::list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
      if (progress) {
	progress->setProgress(num);
	label->setText(*i);
	qApp->processEvents();
	if (progress->wasCancelled())
	  throw QString("Creating script was cancelled");
      }
      num++;

      QString type=*i;
      QString owner;
      QString name;
      int pos=type.find(":");
      if (pos<0)
	throw QString("Internal error, missing : in object description");
      parseObject(type.right(type.length()-pos-1),owner,name);
      printf("BT %s\n",(const char *)type);
      type.truncate(pos);
      printf("AT %s\n",(const char *)type);
      QString utype=type.upper();
      QString schema=intSchema(owner,false);

      try {
	try {
	  extractor *ext=findExtractor("CREATE",utype);
	  if (ext)
	    ret+=ext->create(*this,
			     utype,
			     schema,
			     owner,
			     name);
	  else {
	    printf("OE %s\n",(const char *)type);
	    QString str="Invalid type ";
	    str+=type;
	    str+=" to create";
	    throw str;
	  }
	} catch (const QString &exc) {
	  rethrow("Create",*i,exc);
	}
      } catch (const QString &exc) {
	toStatusMessage(exc);
      }
    }
  } catch(...) {
    delete progress;
    throw;
  }
  delete progress;
  return ret;
}

std::list<QString> toExtract::describe(std::list<QString> &objects)
{
  std::list<QString> ret;

  QProgressDialog *progress=NULL;
  QLabel *label;
  if (Parent) {
    progress=new QProgressDialog("Creating description","&Cancel",
				 objects.size(),Parent,"progress",true);
    progress->setCaption("Creating description");
    label=new QLabel(progress);
    progress->setLabel(label);
  }
  initialize();

  try {
    int num=1;
    for (std::list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
      if (progress) {
	progress->setProgress(num);
	label->setText(*i);
	qApp->processEvents();
	if (progress->wasCancelled())
	  throw QString("Describe was cancelled");
      }
      num++;

      QString type=*i;
      QString owner;
      QString name;
      int pos=type.find(":");
      if (pos<0)
	throw QString("Internal error, missing : in object description");
      parseObject(type.right(type.length()-pos-1),owner,name);
      type.truncate(pos);
      QString utype=type.upper();
      QString schema=intSchema(owner,true);

      std::list<QString> cur;

      try {
	try {
	  extractor *ext=findExtractor("DESCRIBE",utype);
	  if (ext)
	    ext->describe(*this,
			  cur,
			  utype,
			  schema,
			  owner,
			  name);
	  else {
	    QString str="Invalid type ";
	    str+=type;
	    str+=" to describe";
	    throw str;
	  }
	} catch (const QString &exc) {
	  rethrow("Describe",*i,exc);
	}
	cur.sort();
	ret.merge(cur);
      } catch (const QString &exc) {
	toStatusMessage(exc);
      }
    }
  } catch(...) {
    delete progress;
    throw;
  }
  delete progress;
  return ret;
}

QString toExtract::drop(std::list<QString> &objects)
{
  QString ret=generateHeading("CREATE",objects);

  QProgressDialog *progress=NULL;
  QLabel *label;
  if (Parent) {
    progress=new QProgressDialog("Creating drop script","&Cancel",
				 objects.size(),Parent,"progress",true);
    progress->setCaption("Creating drop script");
    label=new QLabel(progress);
    progress->setLabel(label);
  }
  initialize();

  try {
    int num=1;
    for (std::list<QString>::iterator i=objects.begin();i!=objects.end();i++) {
      progress->setProgress(num);
      label->setText(*i);
      qApp->processEvents();
      if (progress->wasCancelled())
	throw QString("Creating drop script was cancelled");
      num++;

      QString type=*i;
      QString owner;
      QString name;
      int pos=type.find(":");
      if (pos<0)
	throw QString("Internal error, missing : in object description");
      parseObject(type.right(type.length()-pos-1),owner,name);
      type.truncate(pos);
      QString utype=type.upper();
      QString schema=intSchema(owner,false);

      try {
	try {
	  extractor *ext=findExtractor("CREATE",utype);
	  if (ext)
	    ret+=ext->drop(*this,
			   utype,
			   schema,
			   owner,
			   name);
	  else {
	    QString str="Invalid type ";
	    str+=type;
	    str+=" to drop";
	    throw str;
	  }
	} catch (const QString &exc) {
	  rethrow("Drop script",*i,exc);
	}
      } catch (const QString &exc) {
	toStatusMessage(exc);
      }
    }
  } catch(...) {
    delete progress;
    throw;
  }
  delete progress;
  return ret;
}

std::map<QString,std::list<QString> > toExtract::migrateGroup(std::list<QString> &grpLst)
{
  std::map<QString,std::list<QString> > ret;
  for(std::list<QString>::iterator i=grpLst.begin();i!=grpLst.end();i++) {
    std::list<QString> ctx=splitDescribe(*i);
    if (ctx.size()<3)
      toStatusMessage(QString("Invalid describe context (<3 parts) \"%1\")").arg(*i));
    else {
      QString t=toShift(ctx);
      t+=":";
      t+=toShift(ctx);
      t+=".";
      t+=toShift(ctx);
      toPush(ret[t],*i);
    }
  }
  return ret;
}

QString toExtract::migrate(std::list<QString> &drpLst,std::list<QString> &crtLst)
{
  std::list<QString> t;
  t.insert(t.end(),"Object list not available in migration");
  QString ret=generateHeading("MIGRATE",t);

  QProgressDialog *progress=NULL;
  QLabel *label;

  std::map<QString,std::list<QString> > objDrp;
  std::map<QString,std::list<QString> > objCrt;

  objDrp=migrateGroup(drpLst);
  objCrt=migrateGroup(crtLst);

  for (std::map<QString,std::list<QString> >::iterator j=objCrt.begin();j!=objCrt.end();j++)
    objDrp[(*j).first]; // Make sure all objects in the createlist also exists in the droplist

  if (Parent) {
    progress=new QProgressDialog("Creating migration script","&Cancel",
				 objDrp.size(),Parent,"progress",true);
    progress->setCaption("Creating migration script");
    label=new QLabel(progress);
    progress->setLabel(label);
  }
  initialize();

  try {
    int num=1;

    for (std::map<QString,std::list<QString> >::iterator i=objDrp.begin();i!=objDrp.end();i++) {
      QString t=(*i).first;

      progress->setProgress(num);
      qApp->processEvents();
      if (progress->wasCancelled())
	throw QString("Creating drop script was cancelled");
      num++;
      label->setText(t);

      std::list<QString> &crt=objCrt[t];
      std::list<QString> &drp=objDrp[t];
      crt.sort();
      drp.sort();
      std::list<QString> ctx=splitDescribe(t);
      QString schema=toShift(ctx);
      QString utype=toShift(ctx).upper();
      QString name=toShift(ctx);

      try {
	try {
	  extractor *ext=findExtractor("MIGRATE",utype);
	  if (ext)
	    ret+=ext->migrate(*this,
			      utype,
			      drp,
			      crt);
	  else {
	    QString str="Invalid type ";
	    str+=utype;
	    str+=" to migrate";
	    throw str;
	  }
	} catch (const QString &exc) {
	  rethrow("Migration script",(*i).first,exc);
	}
      } catch (const QString &exc) {
	toStatusMessage(exc);
      }

    }
  } catch(...) {
    delete progress;
    throw;
  }
  delete progress;
  return ret;
}

QString toExtract::generateHeading(const QString &action,
				   std::list<QString> &lst)
{
  if (!Heading)
    return "";
  char host[1024];
  gethostname(host,1024);

  QString db=Connection.host();
  if (db.length()&&db!="*")
    db+=":";
  else
    db=QString::null;
  db+=Connection.database();

  QString str=QString("REM This DDL was reverse engineered by\n"
		      "REM TOra, Version %1\n"
		      "REM\n"
		      "REM at:   %2\n"
		      "REM from: %3, an %4 %5 database\n"
		      "REM\n"
		      "REM on:   %6\n"
		      "REM\n").
    arg(TOVERSION).
    arg(host).
    arg(db).
    arg(Connection.provider()).
    arg(Connection.version()).
    arg(QDateTime::currentDateTime().toString());
  if (action=="FREE SPACE")
    str+="REM Generating free space report";
  else {
    str+="REM Generating ";
    str+=action;
    str+=" statement";
  }
  if (lst.size()!=1)
    str+="s";
  str+=" for:\nREM\n";
  for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
    if (!(*i).startsWith("TABLE REFERENCES")) {
      str+="REM ";
      str+=(*i);
      str+="\n";
    }
  }
  str+="\n";
  return str;
}

void toExtract::rethrow(const QString &what,const QString &object,const QString &exc)
{
  throw QString("Error in toExtract\n"
		"Operation:      %1\n"
		"Object:         %2\n"
		"Error:          %3").
    arg(what).
    arg(object).
    arg(exc);
}

void toExtract::setSizes(void)
{
  Initial.clear();
  Next.clear();
  Limit.clear();
  if (Resize=="1") {
    for (int i=1;i<10000;i*=10) {
      QString str;
      if (i<1000) {
	str=QString::number(40*i+1);
	str+=" K";
      } else
	str="UNLIMITED";
      toPush(Initial,str);
      str=QString::number(5*BlockSize*i);
      str+=" K";
      toPush(Next,str);
      toPush(Limit,str);
    }
  } else if (!Resize.isEmpty()) {
    QStringList lst=QStringList::split(":",Resize);
    if (lst.count()%3!=0)
      throw QString("Malformed resize string (Should contain multiple of 3 number of ':')");
    for(unsigned int i=0;i<lst.count();i+=3) {
      if (i+3<lst.count())
	toPush(Limit,lst[i]);
      else
	toPush(Limit,QString("UNLIMITED"));
      toPush(Initial,lst[i+1]);
      toPush(Next,lst[i+2]);
    }
  }
}

QString toExtract::intSchema(const QString &owner,bool desc)
{
  if (owner.upper()=="PUBLIC")
    return "PUBLIC";
  if (Schema=="1") {
    QString ret=Connection.quote(owner);
    if (!desc)
      ret+=".";
    return ret;
  } else if (Schema.isEmpty())
    return "";
  QString ret=Connection.quote(Schema);
  if (!desc)
    ret+=".";
  return ret;
}

void toExtract::initialNext(const QString &blocks,QString &initial,QString &next)
{
  std::list<QString>::iterator iinit=Initial.begin();
  std::list<QString>::iterator inext=Next.begin();
  std::list<QString>::iterator ilimit=Limit.begin();
  while(ilimit!=Initial.end()) {
    if (*ilimit=="UNLIMITED"||
	(*ilimit).toFloat()>blocks.toFloat()) {
      initial=*iinit;
      next=*inext;
      return;
    }
    iinit++;
    inext++;
    ilimit++;
  }
}

void toExtract::addDescription(std::list<QString> &ret,std::list<QString> &ctx,
			       const QString &arg1,const QString &arg2,
			       const QString &arg3,const QString &arg4,
			       const QString &arg5,const QString &arg6,
			       const QString &arg7,const QString &arg8,
			       const QString &arg9)
{
  int numArgs;
  if (!arg9.isNull())
    numArgs=9;
  else if (!arg8.isNull())
    numArgs=8;
  else if (!arg7.isNull())
    numArgs=7;
  else if (!arg6.isNull())
    numArgs=6;
  else if (!arg5.isNull())
    numArgs=5;
  else if (!arg4.isNull())
    numArgs=4;
  else if (!arg3.isNull())
    numArgs=3;
  else if (!arg2.isNull())
    numArgs=2;
  else if (!arg1.isNull())
    numArgs=1;
  else
    numArgs=0;

  std::list<QString> args=ctx;
  if (numArgs>0)
    args.insert(args.end(),arg1);
  if (numArgs>1)
    args.insert(args.end(),arg2);
  if (numArgs>2)
    args.insert(args.end(),arg3);
  if (numArgs>3)
    args.insert(args.end(),arg4);
  if (numArgs>4)
    args.insert(args.end(),arg5);
  if (numArgs>5)
    args.insert(args.end(),arg6);
  if (numArgs>6)
    args.insert(args.end(),arg7);
  if (numArgs>7)
    args.insert(args.end(),arg8);
  if (numArgs>8)
    args.insert(args.end(),arg9);

  QString str;
  bool first=true;
  for(std::list<QString>::iterator i=args.begin();i!=args.end();i++) {
    if (first)
      first=false;
    else
      str+="\01";
    if (!(*i).isEmpty())
      str+=*i;
  }
  ret.insert(ret.end(),str);
}

std::list<QString> toExtract::splitDescribe(const QString &str)
{
  QStringList ctx=QStringList::split("\01",str);
  std::list<QString> ret;
  for(unsigned int i=0;i<ctx.count();i++)
    ret.insert(ret.end(),ctx[i]);
  return ret;
}

QString toExtract::partDescribe(const QString &str,int level)
{
  QStringList ctx=QStringList::split("\01",str);
  if (ctx.count()<=level)
    return QString::null;
  return ctx[level];
}
