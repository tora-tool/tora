
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOCONNECTIONPROVIDER_H
#define TOCONNECTIONPROVIDER_H

#include "core/toconnection.h"

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QList>

#include "loki/Factory_alt.h"

/**
   How Tora connects to a database

   1. Each connect provider has its own "finder" class.
      This "finder" class searches for presence of the vendors client libraries like oci.dll.
      Each "finder" is registered with the factory. The main window loops over the keys
      in the factory and tries to find all the installed database clients.

   2. The connection provider itself is loaded from shared library. This library has dependency
      on the database client libraries, env. variables, etc.
*/

/** Singleton holding all the references to @ref toConnectionProviderPrivate subclasses */
class toConnectionProviderRegistry;
class toConnectionProviderRegistrySing;

/** Abstract base class for connection provider implementation, see connection/absfact.h */
class toConnectionProvider;

/** Represents a database connection in TOra. Observe that this can mean several actual
 * connections to the database as queries that are expected to run a long time are sometimes
 * executed in their own connection to make sure the interface doesn't lock up for a long time.
 */
class toConnection;
class toConnectionSub;     // "real" database connection
class toConnectionTraits;   // class representing the "nature" of particular connection provider

class toConnectionSubLoan;

/** Each subclass of @ref toConnectionProviderFinder is registered with the factory @ConnectionProvirerFinderFact.
    A name of the connection provider is used as a key. This class has two main methods:
    find() - returns a list of possible database client instalations
    load() - loads database client shared library and Tora's connection provider plugin.
    See @ref toConnectionProviderRegistry.

*/
class toConnectionProviderFinder
{
public:
    /** Each location of database client location is described by a set of parameters
        like name, path, ORACLE_HOME, ORACLE_HOME_NAME, TNS_ADMIN, INFORMIXDIR, or whatever.
    */
    typedef QMap<QString, QVariant> ConnectionProvirerParams;

    virtual QString name() const = 0;

    /** Return list of possible client locations
     */
    virtual QList<ConnectionProvirerParams> find() = 0;

    /**
       Load connection providers library
    */
    virtual void load(ConnectionProvirerParams const&) = 0;

    inline toConnectionProviderFinder(unsigned int i)
    {
    };

    virtual ~toConnectionProviderFinder() {};

    inline toConnectionProviderFinder() {};
};

/** Declare datatype for finder factory */
typedef Util::GenericFactory<toConnectionProviderFinder, LOKI_TYPELIST_1(unsigned int)> ConnectionProvirerFinderFact;
/** This singleton represents the only instance of the factory */
typedef Loki::SingletonHolder<ConnectionProvirerFinderFact> ConnectionProviderFinderFactory;

/** This class holds pointers into all the dynamically loaded connection providers.
*/
class toConnectionProviderRegistry
{
public:
    toConnectionProvider& get(QString const &provider);
    toConnectionProvider const& get(QString const &provider) const;

    void load(toConnectionProviderFinder::ConnectionProvirerParams const& provider);

    QList<QString> providers() const;

protected:
    /** only singleton @ref toConnectionProviderRegistrySing can create a instance of this class */
    friend class toConnectionProviderRegistrySing;
    friend struct ::Loki::CreateUsingNew<toConnectionProviderRegistry>;
    friend struct ::Loki::CreateUsingMalloc<toConnectionProviderRegistry>;
    toConnectionProviderRegistry() {};
private:
    QMap<QString, toConnectionProvider*> m_registry;
};

class toConnectionProviderRegistrySing: public Loki::SingletonHolder<toConnectionProviderRegistry> {};

/** Abstract class to be subclassed in all the dynamically loaded connection providers.
    See absfact.h
*/
class toConnectionProvider
{
public:
    virtual ~toConnectionProvider() {};

    /** initialize connection provider
    */
    virtual bool initialize() = 0;

    /** @return name of connection provider class (the key in toConnectionProviderRegistry)
    */
    virtual QString const& name () const = 0;

    /** @return name of connection provider class (as shown in new connection combobox
     */
    virtual QString const& displayName () const = 0;

    /** List the available hosts this database provider knows about.
    * @return A list of hosts.
    */
    virtual QList<QString> hosts() = 0;

    /** List the available databases this provider knows about for a given host.
    * @param host Host to return connections for.
    * @param user That might be needed.
    * @param password That might be needed.
    * @return A list of databases available for a given host.
    */
    virtual QList<QString> databases(const QString &host, const QString &user, const QString &pwd) = 0;

    /** Get a list of options available for the connection. An option with the name
    * "-" indicates a break should be made to separate the rest of the options from the previous
    * options. An option preceded by "*" means selected by default. The * shoul be stripped before
    * before passing it to the connection call.
    */
    virtual QList<QString> options() = 0;

    /**
    * Create and return configuration tab for this connectiontype. The returned widget should also
    * be a childclass of @ref toSettingTab.
    *
    * @return A pointer to the widget containing the setup tab for this tool or NULL of
    * no settings are available.
    */
    virtual QWidget *configurationTab(QWidget *parent) = 0;

    /** create @ref toConnection's helper class. */
    virtual toConnection::connectionImpl* createConnectionImpl(toConnection&) = 0;

    /** create @ref toConnection's helper class. */
    virtual toConnectionTraits* createConnectionTrait(void) = 0;

protected:
    /** only singleton @ref ConnectionProvirerFact can create a instance of this class
    */
    friend struct ::Loki::CreateUsingNew<toConnectionProvider>;
    friend struct ::Loki::CreateUsingMalloc<toConnectionProvider>;

    toConnectionProvider(toConnectionProviderFinder::ConnectionProvirerParams const&) {};
    toConnectionProvider();
};

/** \warning this class represents a list of all the connections providers found
 *	those who are really loaded are held in @ref toConnectionProviderRegistrySing
 */
typedef QList<toConnectionProviderFinder::ConnectionProvirerParams> toProvidersList;
typedef Loki::SingletonHolder<toProvidersList> toProvidersListSing;

#endif
