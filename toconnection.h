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
 *      software in the executable aside from Oracle client libraries.
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

#include <qstring.h>
#include "otlv32.h"
#include <list>

class QWidget;

/**
 * This class defines a connection to a database.
 *
 * Actually it can represent several connections since it has the ability to
 * clone the connection if for some reason or other you need another connection
 * to the database.
 *
 * The actuall interface to the database is the excellent OTL library which
 * reference is beyond the scope of this documentation but it is available online
 * at this location http://www.geocities.com/skuchin/otl/home.htm.
 *
 * Connections can be made in two modes, using SQL*Net or not. When connecting using
 * SQL*Net a connection is made in OTL using the connectstring user/password@db. If
 * not using SQL*Net the ORACLE_SID environment variable is temporarily changed to
 * the db and then the connection is made using user/password.
 *
 * All connections to Oracle in TOra are made using UTF-8 encoding. This means that
 * whenever you convert a QString to pass it to or from Oracle you should use the
 * fromUtf8 or utf8 functions and NOT just convert it to const char * which will
 * convert it to latin1 which will not work.
 */

class toConnection {
  /**
   * The actual connection to the database.
   */
  otl_connect *Connection;
  /**
   * Wether to use SQL*Net or not for this connection.
   */
  bool SqlNet;
  /**
   * Username of this connection.
   */
  QString User;
  /**
   * Password of this connection.
   */
  QString Password;
  /**
   * Host or database if you like for this connection.
   */
  QString Host;
  /**
   * The version detected of this database.
   */
  QString Version;
  /**
   * The mode of this connection.
   */
  QString Mode;
  /**
   * The current opened widgets that use this connection. This is not handled 
   * automatically and cause really ugly bugs if the widget doesn't register
   * itself.
   */
  list<QWidget *> Widgets;
  /**
   * Indicate if commit is needed on this connection.
   */
  bool NeedCommit;
  /**
   * Make a new connection for this connection.
   *
   * @return A pointer to the new connection, must be freed.
   */
  otl_connect *newConnection(void);
  /**
   * This string contains statements that are to be run when a new connection
   * is made.
   */
  list<QString> InitStrings;
  /**
   * Setup the connection object.
   */
  void setup(void);
  /**
   * List of unused extra connections.
   */
  list<otl_connect *> FreeConnect;
public:
  /**
   * Create a new connection.
   *
   * @param sqlNet Indicate if SQL*Net is to be used for this connection.
   * @param iuser User to connect with.
   * @param ipassword Password to connect with.
   * @param ihost Host or database to connect to.
   * @param mode Mode of connection, can be empty, SYS_OPER or SYS_DBA to indicate
   *             which options to use when connecting.
   */
  toConnection(bool sqlNet,
	       const QString &iuser,const QString &ipassword,const QString &ihost,
	       const QString &mode=QString::null);
  /**
   * Create a copy of a connection
   */
  toConnection(const toConnection &conn);
  ~toConnection();
  /**
   * Close the widgets that are using this connection.
   * @return Returns true if all widgets agreed to close, otherwise false is returned.
   */
  bool closeWidgets(void);
  /**
   * Get user.
   *
   * @return User of connection.
   */
  const QString &user() const
  { return User; }
  /**
   * Get password.
   *
   * @return Password of connection.
   */
  const QString &password() const
  { return Password; }
  /**
   * Get host.
   *
   * @return Host of connection.
   */
  const QString &host() const
  { return Host; }
  /**
   * Get mode.
   *
   * @return Mode of connection.
   */
  const QString &mode() const
  { return Mode; }
  /**
   * Get version.
   *
   * @return Version of connection.
   */
  const QString &version() const
  { return Version; }
  /**
   * Get information about if the connection has uncommited data.
   *
   * @return Wether uncommited data is available.
   */
  bool needCommit(void) const
  { return NeedCommit; }
  /**
   * Commit connection. This will also close all extra connections except one.
   */
  void commit(void);
  /**
   * Rollback connection. This will also close all extra connections except one.
   */
  void rollback(void);
  /**
   * Set if commit is needed.
   *
   * @param needCommit Wether commit is needed.
   */
  void setNeedCommit(bool needCommit=true)
  { NeedCommit=needCommit; }
  /**
   * Get a connectstring for this connection.
   *
   * @return pw If this is true a connectstring that can be used with otl_connect is
   *            returned, otherwise a descriptive string of the connection without
   *            the password is returned.
   */
  QString connectString(bool pw=false) const;
  /**
   * Get the main connection of this connection.
   *
   * @return A reference to the connection.
   */
  otl_connect &connection();
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
   * Get a longrunning connection. Will create a new connection or return a free
   * connection already available. It can also return the main connection if TOra
   * is configured to not create new connections for long running queries. The
   * connection should be returned using the @ref longOperationFree function.
   *
   * @return A pointer to a connection.
   */
  otl_connect *longOperation(void);
  /**
   * Return a connection that is no longer needed. This can be reused or discarded
   * when no longer needed. It will not be removed until either commit or rollback
   * is issued.
   * @param conn Connection to return.
   */
  void longOperationFree(otl_connect *conn);
  /**
   * Get a list of the other sessions that this connection has made and not closed
   * yet.
   */
  list<otl_connect *> &otherSessions(void)
  { return FreeConnect; }
};

#endif
