//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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

#include "toqvalue.h"

class QWidget;
class toConnection;
class toConnectionProvider;
class toSQL;

/** This class is an abstract definition of an actual connection to a database.
 * Each @ref toConnection object can have one or more actual connections to the
 * database depending on long running queries. Normally you will never need to
 * bother with this class if you aren't creating a new database provider
 * (@ref toConnectionProvider).
 */

class toConnectionSub {
public:
  /** Create connection to database.
   */
  toConnectionSub()
  { }
  /** Close connection.
   */
  virtual ~toConnectionSub()
  { }
};

/** This class is used to perform a query on a database connection.
 */

class toQuery {
public:
  /** Represent different modes to run a query in.
   */
  enum queryMode {
    /** Run the query normally on the main connection of the @ref toConnection object.
     */
    Normal,
    /** Run the query in a separate connection for long running queries.
     */
    Long,
    /** Run the query on all non occupied connections of the @ref toConnection object.
     */
    All
  };

  /** This structure is used to describe the resultset of a query.
   */

  struct queryDescribe {
    /** Column name
     */
    QString Name;
    /** Datatype of string.
     */
    QString Datatype;
    /** If column can contain null values.
     */
    bool Null;
  };
  /** Abstract parent of implementations of a query for a database provider
   * (See @ref toConnection::connectionImpl and @ref toConnectionProvider)
   */
  class queryImpl {
    toQuery *Parent;
  public:
    /** Get the parent query object. All the parameters of the query must be read from here.
     * nothing is passed to the functions.
     */
    toQuery *query()
    { return Parent; }
    
    /** Create a query implementation. The constructor must not perform any actions with the
     * database that could block for a noticable time (Like execute or parse a query). The
     * data for the query may not be available when this object created.
     * @param query Parent query object.
     */
    queryImpl(toQuery *query)
      : Parent(query)
    { }
    /** Destroy query implementation.
     */
    virtual ~queryImpl()
    { }
    /** Execute a query. Parameters can be gotten from the @ref toQuery object.
     */
    virtual void execute(void) = 0;
    /** Read the next value from the stream.
     * @return The value read from the query.
     */
    virtual toQValue readValue(void) = 0;
    /** Check if the end of the query has been reached.
     * @return True if all values have been read.
     */
    virtual bool eof(void) = 0;
    /** Get the number of rows processed in the last executed query.
     */
    virtual int rowsProcessed(void) = 0;
    /** Describe the currently running query.
     * @return A list of column descriptions of the query.
     */
    virtual std::list<queryDescribe> describe(void) = 0;
    /** Get number of columns in the resultset.
     * @return Column number.
     */
    virtual int columns(void) = 0;
  };

private:
  toConnection &Connection;
  toConnectionSub *ConnectionSub;
  std::list<toQValue> Params;
  QCString SQL;
  queryMode Mode;

  queryImpl *Query;
  toQuery(const toQuery &);
public:
  /** Create a normal query.
   * @param conn Connection to create query on.
   * @param sql SQL to run.
   * @param params Parameters to pass to query.
   */
  toQuery(toConnection &conn,toSQL &sql,const std::list<toQValue> &params);
  /** Create a normal query.
   * @param conn Connection to create query on.
   * @param sql SQL to run.
   * @param params Parameters to pass to query.
   */
  toQuery(toConnection &conn,const QString &sql,const std::list<toQValue> &params);
  /** Create a normal query.
   * @param conn Connection to create query on.
   * @param sql SQL to run.
   * @param arg1 Arguments to pass to query.
   */
  toQuery(toConnection &conn,toSQL &sql,
	  const QString &arg1=QString::null,const QString &arg2=QString::null,
	  const QString &arg3=QString::null,const QString &arg4=QString::null,
	  const QString &arg5=QString::null,const QString &arg6=QString::null,
	  const QString &arg7=QString::null,const QString &arg8=QString::null,
	  const QString &arg9=QString::null);
  /** Create a normal query.
   * @param conn Connection to create query on.
   * @param sql SQL to run.
   * @param arg1 Arguments to pass to query.
   */
  toQuery(toConnection &conn,const QString &sql,
	  const QString &arg1=QString::null,const QString &arg2=QString::null,
	  const QString &arg3=QString::null,const QString &arg4=QString::null,
	  const QString &arg5=QString::null,const QString &arg6=QString::null,
	  const QString &arg7=QString::null,const QString &arg8=QString::null,
	  const QString &arg9=QString::null);

  /** Create a query.
   * @param conn Connection to create query on.
   * @param mode Mode to run query in.
   * @param sql SQL to run.
   * @param params Arguments to pass to query.
   */
  toQuery(toConnection &conn,queryMode mode,toSQL &sql,const std::list<toQValue> &params);
  /** Create a query.
   * @param conn Connection to create query on.
   * @param mode Mode to run query in.
   * @param sql SQL to run.
   * @param params Arguments to pass to query.
   */
  toQuery(toConnection &conn,queryMode mode,const QString &sql,const std::list<toQValue> &params);
  /** Create a query. Don't runn any SQL using it yet. Observe though that the @ref
   * toConnectionSub object is assigned here so you know that all queries run using this
   * query object will run on the same actual connection to the database (Unless mode is All off
   * course).
   * @param conn Connection to create query for.
   * @param mode Mode to execute queries in.
   */
  toQuery(toConnection &conn,queryMode mode=Normal);
  /** Destroy query.
   */
  virtual ~toQuery();

  /** Execute an SQL statement using this query.
   * @param sql SQL to run.
   * @param params Parameters to pass to query.
   */
  void execute(toSQL &sql,const std::list<toQValue> &params);
  /** Execute an SQL statement using this query.
   * @param sql SQL to run.
   * @param params Parameters to pass to query.
   */
  void execute(const QString &sql,const std::list<toQValue> &params);

  /** Connection object of this object.
   */
  toConnection &connection(void)
  { return Connection; }
  /** Actual database connection that this query is currently using.
   */
  toConnectionSub *connectionSub(void)
  { return ConnectionSub; }
  /** Parameters of the current query.
   */
  std::list<toQValue> &params(void)
  { return Params; }
  /** SQL to run. Observe that this string is in UTF8 format.
   */
  QCString sql(void)
  { return SQL; }

  /** Read a value from the query. Convert the value NULL to the string {null}.
   * @return Value read.
   */
  toQValue readValue(void);
  /** Read a value from the query. Nulls are returned as empty @ref toQValue.
   * @return Value read.
   */
  toQValue readValueNull(void);
  /** Check if end of query is reached.
   * @return True if end of query is reached.
   */
  bool eof(void);

  /** Get the number of rows processed by the query.
   */
  int rowsProcessed(void)
  { return Query->rowsProcessed(); }
  /** Get a list of descriptions for the columns. This function is relatively slow.
   */
  std::list<queryDescribe> describe(void)
  { return Query->describe(); }
  /** Get the number of columns in the resultset of the query.
   */
  int columns(void)
  { return Query->columns(); }

  /** Execute a query and return all the values returned by it.
   * @param conn Connection to run query on.
   * @param sql SQL to run.
   * @param params Parameters to pass to query.
   * @return A list of @ref toQValues:s read from the query.
   */
  static std::list<toQValue> readQuery(toConnection &conn,
				       toSQL &sql,
				       std::list<toQValue> &params);
  /** Execute a query and return all the values returned by it.
   * @param conn Connection to run query on.
   * @param sql SQL to run.
   * @param params Parameters to pass to query.
   * @return A list of @ref toQValues:s read from the query.
   */
  static std::list<toQValue> readQuery(toConnection &conn,
				       const QString &sql,
				       std::list<toQValue> &params);
  /** Execute a query and return all the values returned by it.
   * @param conn Connection to run query on.
   * @param sql SQL to run.
   * @param arg1 Parameters to pass to query.
   * @return A list of @ref toQValues:s read from the query.
   */
  static std::list<toQValue> readQuery(toConnection &conn,toSQL &sql,
				       const QString &arg1=QString::null,const QString &arg2=QString::null,
				       const QString &arg3=QString::null,const QString &arg4=QString::null,
				       const QString &arg5=QString::null,const QString &arg6=QString::null,
				       const QString &arg7=QString::null,const QString &arg8=QString::null,
				       const QString &arg9=QString::null);
  /** Execute a query and return all the values returned by it.
   * @param conn Connection to run query on.
   * @param sql SQL to run.
   * @param arg1 Parameters to pass to query.
   * @return A list of @ref toQValues:s read from the query.
   */
  static std::list<toQValue> readQuery(toConnection &conn,const QString &sql,
				       const QString &arg1=QString::null,const QString &arg2=QString::null,
				       const QString &arg3=QString::null,const QString &arg4=QString::null,
				       const QString &arg5=QString::null,const QString &arg6=QString::null,
				       const QString &arg7=QString::null,const QString &arg8=QString::null,
				       const QString &arg9=QString::null);
  /** Execute a query and return all the values returned by it.
   * @param conn Connection to run query on.
   * @param sql SQL to run.
   * @param params Parameters to pass to query.
   * @return A list of @ref toQValues:s read from the query.
   */
  static std::list<toQValue> readQueryNull(toConnection &conn,
					   toSQL &sql,
					   std::list<toQValue> &params);
  /** Execute a query and return all the values returned by it.
   * @param conn Connection to run query on.
   * @param sql SQL to run.
   * @param params Parameters to pass to query.
   * @return A list of @ref toQValues:s read from the query.
   */
  static std::list<toQValue> readQueryNull(toConnection &conn,
					   const QString &sql,
					   std::list<toQValue> &params);
  /** Execute a query and return all the values returned by it.
   * @param conn Connection to run query on.
   * @param sql SQL to run.
   * @param arg1 Parameters to pass to query.
   * @return A list of @ref toQValues:s read from the query.
   */
  static std::list<toQValue> readQueryNull(toConnection &conn,toSQL &sql,
					   const QString &arg1=QString::null,const QString &arg2=QString::null,
					   const QString &arg3=QString::null,const QString &arg4=QString::null,
					   const QString &arg5=QString::null,const QString &arg6=QString::null,
					   const QString &arg7=QString::null,const QString &arg8=QString::null,
					   const QString &arg9=QString::null);
  /** Execute a query and return all the values returned by it.
   * @param conn Connection to run query on.
   * @param sql SQL to run.
   * @param arg1 Parameters to pass to query.
   * @return A list of @ref toQValues:s read from the query.
   */
  static std::list<toQValue> readQueryNull(toConnection &conn,const QString &sql,
					   const QString &arg1=QString::null,const QString &arg2=QString::null,
					   const QString &arg3=QString::null,const QString &arg4=QString::null,
					   const QString &arg5=QString::null,const QString &arg6=QString::null,
					   const QString &arg7=QString::null,const QString &arg8=QString::null,
					   const QString &arg9=QString::null);
};

/** A short representation of a @ref toQuery::queryDescribe
 */
typedef toQuery::queryDescribe toQDescribe;
/** A short representation of list<toQuery::queryDescribe>
 */
typedef std::list<toQDescribe> toQDescList;

/** Represent a database connection in TOra. Observe that this can mean several actual
 * connections to the database as queries that ae expected to run a long time are sometimes
 * executed in their own connection to make sure the interface doesn't lock up for a long time.
 */
class toConnection {
  toConnectionProvider &Provider;
  QString User;
  QString Password;
  QString Host;
  QString Database;
  QString Mode;
  QString Version;
  std::list<QWidget *> Widgets;
  std::list<QCString> InitStrings;
  std::list<toConnectionSub *> Connections;
  bool NeedCommit;

public:

  /** Contain information about a tablename.
   */
  struct tableName {
    /** The object name
     */
    QString Name;
    /** The schema that owns it
     */
    QString Owner;
    /** Synonym name, only available if this is a synonym. The tablename & owner will
     * point to what it is a synonym for.
     */
    QString Synonym;

    bool operator < (const tableName &) const;
    bool operator == (const tableName &) const;
  };

  /** Contain information about a column in a table.
   */
  struct columnDesc {
    /** Column name
     */
    QString Name;
    /** Column Description
     */
    QString Comment;
  };

  /** This class is an abstract baseclass to actually implement the comunication with the
   * database. 
   * (See also @ref toQuery::queryImpl and @ref toConnectionProvider)
   */
  class connectionImpl {
    toConnection *Connection;
  public:
    /** Get the parent connection object of this connection.
     */
    toConnection &connection(void)
    { return *Connection; }
    /** Create a new connection implementation for a connection.
     * @param conn Connection to implement.
     */
    connectionImpl(toConnection *conn)
    { Connection=conn; };
    /** Destructor.
     */
    virtual ~connectionImpl()
    { }

    /** Commit the supplied actual database connection.
     */
    virtual void commit(toConnectionSub *) = 0;
    /** Rollback the supplied actual database connection.
     */
    virtual void rollback(toConnectionSub *) = 0;

    /** Create a new connection to the database.
     */
    virtual toConnectionSub *createConnection(void) = 0;
    /** Close a connection to the database.
     */
    virtual void closeConnection(toConnectionSub *) = 0;

    /** Get the version of the database connected to.
     */
    virtual QString version(toConnectionSub *) = 0;
    /** Create a copy of this implementation.
     */
    virtual connectionImpl *clone(toConnection *newConn) const = 0;

    /** Extract available objects to query for connection.
     * @return List of available objects.
     */
    virtual std::list<tableName> tableNames(void);
    /** Extract available columns to query for a table.
     * @param table Table to get column for.
     * @return List of columns for table.
     */
    virtual std::list<toConnection::columnDesc> columnDesc(const tableName &table);

    /** Create a new query implementation for this connection.
     * @return A query implementation, allocated with new.
     */
    virtual toQuery::queryImpl *createQuery(toQuery *query,toConnectionSub *conn) = 0;
    /** Execute a query on an actual connection without caring about the result.
     * @param conn Connection to execute on.
     * @param sql SQL to execute.
     * @param params Parameters to pass to query.
     */
    virtual void execute(toConnectionSub *conn,const QCString &sql,toQList &params) = 0;
  };

private:

  void addConnection(void);
  std::list<toConnectionSub *> &connections(void)
  { return Connections; }

  connectionImpl *Connection;

  std::map<tableName,std::list<columnDesc> > ColumnCache;
  std::list<tableName> TableNames;

  toConnectionSub *mainConnection(void);
  toConnectionSub *longConnection(void);
  void freeConnection(toConnectionSub *);
  void readObjects(void);
public:
  /** Create a new connection.
   * @param provider Which database provider to use for this connection.
   * (See @ref to toDatabaseConnection)
   * @param user User to connect to the database with.
   * @param password Password to connect with.
   * @param host Host to connect to the database with.
   * @param database Database to connect to.
   * @param mode Mode to connect to the database with.
   */
  toConnection(const QString &provider,const QString &user,const QString &password,
	       const QString &host,const QString &database,const QString &mode=QString::null);
  /** Create a copy of a connection.
   * @param conn Connection to copy.
   */
  toConnection(const toConnection &conn);
  /** Destroy connection.
   */
  virtual ~toConnection();

  /** Try to close all the widgets associated with this connection.
   * @return True if all widgets agreed to close.
   */
  bool closeWidgets(void);
  /** Get username of connection.
   */
  const QString &user() const
  { return User; }
  /** Get password of connection.
   */
  const QString &password() const
  { return Password; }
  /** Get host of connection.
   */
  const QString &host() const
  { return Host; }
  /** Get database of connection.
   */
  const QString &database() const
  { return Database; }
  /** Get mode of connection.
   */
  const QString &mode() const
  { return Mode; }
  /** Get version of connection.
   */
  const QString &version() const
  { return Version; }
  /** Get provider of connection.
   */
  const QString &provider() const;

  /** Get a description of this connection.
   */
  virtual QString description(void) const;

  /** Set if this connection needs to be commited.
   */
  void setNeedCommit(bool needCommit=true)
  { NeedCommit=needCommit; }
  /**
   * Get information about if the connection has uncommited data.
   *
   * @return Whether uncommited data is available.
   */
  bool needCommit(void) const
  { return NeedCommit; }
  /**
   * Commit connection. This will also close all extra connections except one.
   */
  virtual void commit(void);
  /**
   * Rollback connection. This will also close all extra connections except one.
   */
  virtual void rollback(void);

  /** Execute a statement without caring about the result.
   * @param sql SQL to execute
   * @param params Parameters to pass to query.
   */
  void execute(toSQL &sql,
	       toQList &params);
  /** Execute a statement without caring about the result.
   * @param sql SQL to execute
   * @param params Parameters to pass to query.
   */
  void execute(const QString &sql,
	       toQList &params);
  /** Execute a statement without caring about the result.
   * @param sql SQL to execute
   * @param arg1 Parameters to pass to query.
   */
  void execute(toSQL &sql,
	       const QString &arg1=QString::null,const QString &arg2=QString::null,
	       const QString &arg3=QString::null,const QString &arg4=QString::null,
	       const QString &arg5=QString::null,const QString &arg6=QString::null,
	       const QString &arg7=QString::null,const QString &arg8=QString::null,
	       const QString &arg9=QString::null);
  /** Execute a statement without caring about the result.
   * @param sql SQL to execute
   * @param arg1 Parameters to pass to query.
   */
  void execute(const QString &sql,
	       const QString &arg1=QString::null,const QString &arg2=QString::null,
	       const QString &arg3=QString::null,const QString &arg4=QString::null,
	       const QString &arg5=QString::null,const QString &arg6=QString::null,
	       const QString &arg7=QString::null,const QString &arg8=QString::null,
	       const QString &arg9=QString::null);

  /** Execute a statement without caring about the result on all open database connections.
   * @param sql SQL to execute
   * @param params Parameters to pass to query.
   */
  void allExecute(toSQL &sql,
		  toQList &params);
  /** Execute a statement without caring about the result on all open database connections.
   * @param sql SQL to execute
   * @param params Parameters to pass to query.
   */
  void allExecute(const QString &sql,
		  toQList &params);
  /** Execute a statement without caring about the result on all open database connections.
   * @param sql SQL to execute
   * @param arg1 Parameters to pass to query.
   */
  void allExecute(toSQL &sql,
		  const QString &arg1=QString::null,const QString &arg2=QString::null,
		  const QString &arg3=QString::null,const QString &arg4=QString::null,
		  const QString &arg5=QString::null,const QString &arg6=QString::null,
		  const QString &arg7=QString::null,const QString &arg8=QString::null,
		  const QString &arg9=QString::null);
  /** Execute a statement without caring about the result on all open database connections.
   * @param sql SQL to execute
   * @param arg1 Parameters to pass to query.
   */
  void allExecute(const QString &sql,
		  const QString &arg1=QString::null,const QString &arg2=QString::null,
		  const QString &arg3=QString::null,const QString &arg4=QString::null,
		  const QString &arg5=QString::null,const QString &arg6=QString::null,
		  const QString &arg7=QString::null,const QString &arg8=QString::null,
		  const QString &arg9=QString::null);

  /**
   * Add a widget that uses this connection. This is needed to ensure that all widgets
   * that make use of a connection are destroyed when the connection is closed. Usually
   * tool windows need to call this function.
   *
   * @param widget The widget to add to the connection.
   */
  void addWidget(QWidget *widget)
  { Widgets.insert(Widgets.end(),widget); }
  /**
   * Remove a widget from this connection. Should be called by the destructor of the
   * widget that has called addWidget.
   *
   * @see addWidget
   * @param widget Widget to remove from the widget list.
   */
  void delWidget(QWidget *widget);

  /**
   * Add a statement to be run uppon making new connections.
   * @param sql Statement to run.
   */
  void addInit(const QString &sql);
  /**
   * Remove a statement that was added using @ref addInit.
   */
  void delInit(const QString &sql);

  /**
   * Get the tables available for the current user. This function caches the responses
   * and should be fairly fast after the first call. Do not modify the returned list.
   * @return A list of tables available for the current user, including psynonyms and
   *         views.
   */
  std::list<tableName> &tables(void);
  /**
   * Get a list of the available columns for a table. This function caches the responses
   * and should be fairly fast after the first call. Do not modify the returned list.
   * @return A list of the columns for a table.
   */
  std::list<columnDesc> &columns(const tableName &table);
  /**
   * Clear the object and column cache.
   */
  void clearCache(void);
  /**
   * Get the real object name of a synonym.
   */
  const tableName &realName(const QString &object);

  friend class toQuery;
};

/** This class is used to implement new database providers.
 */

class toConnectionProvider {
  static std::map<QString,toConnectionProvider *> *Providers;
  QString Provider;
  static void checkAlloc(void);
public:
  /** Create a new provider with the specified name.
   * @param provider Name of the provider.
   */
  toConnectionProvider(const QString &provider);
  /** Destructor.
   */
  virtual ~toConnectionProvider();
  /** Get name of provider.
   */
  const QString &provider(void) const
  { return Provider; }

  /** Create an implementation of a connection to this database.
   * @return A connection implementation created with new.
   */
  virtual toConnection::connectionImpl *connection(toConnection *conn)=0;
  /** List the available modes for this database.
   * @return A list of modes that the connection implementation understands.
   */
  virtual std::list<QString> modes(void);
  /** List the available hosts this database provider knows about.
   * @return A list of hosts.
   */
  virtual std::list<QString> hosts(void);
  /** List the available databases this provider knows about for a given host.
   * @param host Host to return connections for.
   * @param user That might be needed.
   * @param password That might be needed.
   * @return A list of databases available for a given host.
   */
  virtual std::list<QString> databases(const QString &host,
				       const QString &user,
				       const QString &pwd)=0;

  /** Get the provider object for a given provider name.
   * @param provider Name of provider.
   * @return Reference to provider object.
   */
  static toConnectionProvider &fetchProvider(const QString &provider);
  /** Get a list of names for providers.
   */
  static std::list<QString> providers();
  /** Get a list of modes for a given provider.
   */
  static std::list<QString> modes(const QString &provider);
  /** Implement a connection for a given provider.
   * @param provider Provider to implement.
   * @param conn Connection to create implementation for.
   */
  static toConnection::connectionImpl *connection(const QString &provider,toConnection *conn);
  /** Get a list of hosts this provider knows about.
   */
  static std::list<QString> hosts(const QString &provider);
  /** Get a list of databases for a given provider and host.
   * @param provider Provider to fetch databases for.
   * @param host Host to fetch databases for.
   * @param user That might be needed.
   * @param password That might be needed.
   * @return List of known databases.
   */
  static std::list<QString> databases(const QString &provider,const QString &host,
				      const QString &user,const QString &pwd);
};

#endif
