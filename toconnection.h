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

#ifndef __TOCONNECTION_H
#define __TOCONNECTION_H

#include <list>
#include <map>

#include <qstring.h>

class QWidget;
class toConnection;
class toConnectionProvider;
class toSQL;

class toConnectionSub {
public:
  toConnectionSub()
  { }
  virtual ~toConnectionSub()
  { }
};

class toQuery {
public:
  enum queryMode {
    Normal,
    Long,
    All
  };

  struct queryDescribe {
    QString Name;
    QString Datatype;
    bool Null;
  };
  class queryValue {
    enum {
      intType,
      doubleType,
      stringType,
      nullType
    } Type;
    union {
      int Int;
      double Double;
      QString *String;
    } Value;
  public:
    queryValue(void);
    queryValue(int i);
    queryValue(const QString &str);
    queryValue(double d);
    queryValue(const queryValue &copy);
    ~queryValue();

    bool isInt(void) const;
    bool isDouble(void) const;
    bool isString(void) const;
    bool isNull(void) const;

    QCString utf8Value(void) const;
    int toInt(void) const;
    double toDouble(void) const;

    operator QString() const;
  };
  class queryImpl {
    toQuery *Query;
  public:
    toQuery *query()
    { return Query; }
    
    queryImpl(toQuery *query)
      : Query(query)
    { }
    virtual ~queryImpl()
    { }
    virtual void execute(void) = 0;
    virtual queryValue readValue(void) = 0;
    virtual bool eof(void) = 0;
    virtual int rowsProcessed(void) = 0;
    virtual list<queryDescribe> describe(void) = 0;
    virtual int columns(void) = 0;
  };

private:
  toConnection &Connection;
  toConnectionSub *ConnectionSub;
  list<queryValue> Params;
  QCString SQL;
  queryMode Mode;

  queryImpl *Query;
  toQuery(const toQuery &);
public:
  toQuery(toConnection &conn,toSQL &sql,const list<queryValue> &params);
  toQuery(toConnection &conn,const QString &sql,const list<queryValue> &params);
  toQuery(toConnection &conn,toSQL &sql,
	  const QString &arg1=QString::null,const QString &arg2=QString::null,
	  const QString &arg3=QString::null,const QString &arg4=QString::null,
	  const QString &arg5=QString::null,const QString &arg6=QString::null,
	  const QString &arg7=QString::null,const QString &arg8=QString::null,
	  const QString &arg9=QString::null);
  toQuery(toConnection &conn,const QString &sql,
	  const QString &arg1=QString::null,const QString &arg2=QString::null,
	  const QString &arg3=QString::null,const QString &arg4=QString::null,
	  const QString &arg5=QString::null,const QString &arg6=QString::null,
	  const QString &arg7=QString::null,const QString &arg8=QString::null,
	  const QString &arg9=QString::null);

  toQuery(toConnection &conn,queryMode mode,toSQL &sql,const list<queryValue> &params);
  toQuery(toConnection &conn,queryMode mode,const QString &sql,const list<queryValue> &params);
  toQuery(toConnection &conn,queryMode mode=Normal);

  virtual ~toQuery();

  void execute(toSQL &sql,const list<queryValue> &params);
  void execute(const QString &sql,const list<queryValue> &params);

  toConnection &connection(void)
  { return Connection; }
  toConnectionSub *connectionSub(void)
  { return ConnectionSub; }
  list<queryValue> &params(void)
  { return Params; }
  QCString sql(void)
  { return SQL; }

  queryValue readValue(void);
  queryValue readValueNull(void);
  bool eof(void);

  int rowsProcessed(void)
  { return Query->rowsProcessed(); }
  list<queryDescribe> describe(void)
  { return Query->describe(); }
  int columns(void)
  { return Query->columns(); }

  static list<queryValue> readQuery(toConnection &conn,
				    toSQL &sql,
				    list<queryValue> &params);
  static list<queryValue> readQuery(toConnection &conn,
				    const QString &sql,
				    list<queryValue> &params);
  static list<queryValue> readQuery(toConnection &conn,toSQL &sql,
				    const QString &arg1=QString::null,const QString &arg2=QString::null,
				    const QString &arg3=QString::null,const QString &arg4=QString::null,
				    const QString &arg5=QString::null,const QString &arg6=QString::null,
				    const QString &arg7=QString::null,const QString &arg8=QString::null,
				    const QString &arg9=QString::null);
  static list<queryValue> readQuery(toConnection &conn,const QString &sql,
				    const QString &arg1=QString::null,const QString &arg2=QString::null,
				    const QString &arg3=QString::null,const QString &arg4=QString::null,
				    const QString &arg5=QString::null,const QString &arg6=QString::null,
				    const QString &arg7=QString::null,const QString &arg8=QString::null,
				    const QString &arg9=QString::null);
};

typedef toQuery::queryValue toQValue;
typedef list<toQValue> toQList;
typedef toQuery::queryDescribe toQDescribe;
typedef list<toQDescribe> toQDescList;

class toConnection {
  toConnectionProvider &Provider;
  QString User;
  QString Password;
  QString Host;
  QString Database;
  QString Mode;
  QString Version;
  list<QWidget *> Widgets;
  list<QCString> InitStrings;
  list<toConnectionSub *> Connections;
  bool NeedCommit;

public:

  class connectionImpl {
    toConnection *Connection;
  public:
    toConnection &connection(void)
    { return *Connection; }
    connectionImpl(toConnection *conn)
    { Connection=conn; };
    virtual ~connectionImpl()
    { }

    virtual void commit(toConnectionSub *) = 0;
    virtual void rollback(toConnectionSub *) = 0;

    virtual toConnectionSub *createConnection(void) = 0;
    virtual void closeConnection(toConnectionSub *) = 0;

    virtual QString version(toConnectionSub *) = 0;
    virtual connectionImpl *clone(toConnection *newConn) const = 0;

    virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *conn) = 0;
    virtual void execute(toConnectionSub *,const QCString &sql,toQList &params) = 0;
  };

private:

  void addConnection(void);
  list<toConnectionSub *> &connections(void)
  { return Connections; }

  connectionImpl *Connection;

public:
  toConnection(const QString &provider,const QString &user,const QString &password,
	       const QString &host,const QString &database,const QString &mode=QString::null);
  toConnection(const toConnection &conn);
  virtual ~toConnection();

  bool closeWidgets(void);
  const QString &user() const
  { return User; }
  const QString &password() const
  { return Password; }
  const QString &host() const
  { return Host; }
  const QString &database() const
  { return Database; }
  const QString &mode() const
  { return Mode; }
  const QString &version() const
  { return Version; }
  const QString &provider() const;

  toConnectionSub *mainConnection(void);
  toConnectionSub *longConnection(void);
  void freeConnection(toConnectionSub *);

  virtual QString description(void) const;

  void setNeedCommit(bool needCommit=true)
  { NeedCommit=needCommit; }
  bool needCommit(void) const
  { return NeedCommit; }
  virtual void commit(void);
  virtual void rollback(void);

  void execute(toSQL &sql,
	       toQList &params);
  void execute(const QString &sql,
	       toQList &params);
  void execute(toSQL &sql,
	       const QString &arg1=QString::null,const QString &arg2=QString::null,
	       const QString &arg3=QString::null,const QString &arg4=QString::null,
	       const QString &arg5=QString::null,const QString &arg6=QString::null,
	       const QString &arg7=QString::null,const QString &arg8=QString::null,
	       const QString &arg9=QString::null);
  void execute(const QString &sql,
	       const QString &arg1=QString::null,const QString &arg2=QString::null,
	       const QString &arg3=QString::null,const QString &arg4=QString::null,
	       const QString &arg5=QString::null,const QString &arg6=QString::null,
	       const QString &arg7=QString::null,const QString &arg8=QString::null,
	       const QString &arg9=QString::null);

  void allExecute(toSQL &sql,
		  toQList &params);
  void allExecute(const QString &sql,
		  toQList &params);
  void allExecute(toSQL &sql,
		  const QString &arg1=QString::null,const QString &arg2=QString::null,
		  const QString &arg3=QString::null,const QString &arg4=QString::null,
		  const QString &arg5=QString::null,const QString &arg6=QString::null,
		  const QString &arg7=QString::null,const QString &arg8=QString::null,
		  const QString &arg9=QString::null);
  void allExecute(const QString &sql,
		  const QString &arg1=QString::null,const QString &arg2=QString::null,
		  const QString &arg3=QString::null,const QString &arg4=QString::null,
		  const QString &arg5=QString::null,const QString &arg6=QString::null,
		  const QString &arg7=QString::null,const QString &arg8=QString::null,
		  const QString &arg9=QString::null);

  void addWidget(QWidget *widget)
  { Widgets.insert(Widgets.end(),widget); }
  void delWidget(QWidget *widget);

  void addInit(const QString &sql);
  void delInit(const QString &sql);

  friend toQuery;
};

class toConnectionProvider {
  static map<QString,toConnectionProvider *> *Providers;
  QString Provider;
public:
  toConnectionProvider(const QString &provider);
  virtual ~toConnectionProvider();
  const QString &provider(void) const
  { return Provider; }

  virtual toConnection::connectionImpl *connection(toConnection *conn)=0;
  virtual list<QString> modes(void);
  virtual list<QString> hosts(void);
  virtual list<QString> databases(const QString &host)=0;

  static toConnectionProvider &fetchProvider(const QString &provider);
  static list<QString> providers();
  static list<QString> modes(const QString &provider);
  static toConnection::connectionImpl *connection(const QString &provider,toConnection *conn);
  static list<QString> hosts(const QString &provider);
  static list<QString> databases(const QString &provider,const QString &host);
};

#endif
