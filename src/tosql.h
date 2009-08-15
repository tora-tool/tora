
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSQL_H
#define TOSQL_H

#include "config.h"

#include <map>
#include <list>

#include <qstring.h>
//Added by qt3to4:
#include <QString>

class toConnection;

/**
 * This class handles an abstraction of SQL statements used by TOra to extract
 * information and manipulate data. This is usefull in two ways, first off you
 * can edit the strings using a handy editor inside TOra if you find bugs. Also
 * you can use different statements for different Oracle versions. You can also
 * add support for new versions of Oracle without any need for recompilation or
 * programming knowledge.
 *
 * All information about SQL statements are stored in one global static map which
 * contains a name, a description and one or several statements connected to one
 * Oracle version.
 *
 * To determine which SQL to use the statement with the highest
 * version not above the current connection is used. If no statements below or
 * equal to the current version is available the lowest available is used.
 *
 * All strings are specific for a given database provider. No attempt to use
 * strings from other providers will be made.
 */

class toSQL
{
public:
    /**
     * Contains a statement with it's version.
     */
    struct version
    {
        /**
         * The provider this version is made for.
         */
        QString Provider;
        /**
         * Version string
         */
        QString Version;
        /**
         * SQL statement
         */
        QString SQL;
        /** An indication if this SQL has been modified after startup and thus needs to be
         * saved to disk to retain.
         */
        bool Modified;
        /** Create a new version
         * @param provider Provider
         * @param ver Version
         * @param sql Statement
         * @param modified Wether it is modified or not.
         */
        version(const QString &provider, const QString &ver, const QString &sql, bool modified = false)
                : Provider(provider), Version(ver), SQL(sql), Modified(modified)
        { }
    }
    ;

    /**
     * Definition of one toSQL statement with its description and versions.
     */
    struct definition
    {
        /** Description of what this statement is supposed to do.
         */
        QString Description;
        /** Indication of wether this description is changed or not and needs to be saved
         * to disk to retain.
         */
        bool Modified;
        /** List of the different versions of the statement.
         */
        std::list<version> Versions;
    };

    /** Type of map of statement names to statement definitions.
     */
    typedef std::map<QString, definition> sqlMap;
private:
    /** Map of statement names to statement definitions for each provider.
     */
    static sqlMap *Definitions;

    /** Name of this SQL statement
     */
    QString Name;

    /** Internal constructor used by some of the internal functions. Creates an
     * SQL statement with a name but without an entry in the @ref Definitions map.
     */
    toSQL(const QString &name);

    /** Check that @ref Definitions are allocated, if not allocate it.
     */
    static void allocCheck(void);
public:
    /** Name of the SQL to get a userlist
     */
    static const char * const TOSQL_USERLIST;
    /** Name of the SQL to create a plan table
     */
    static const char * const TOSQL_CREATEPLAN;
    /** Update the map with new information.
     * @param name Name of this SQL statement.
     * @param sql Statement to execute for this SQL.
     * @param description Description of this SQL.
     * @param ver Version of database this statement is meant for.
     * @param provider Database provider this string is used for.
     * @param modified Wether this is a modification or an addition to the map.
     * @return True if a new statement was saved, otherwise false.
     */
    static bool updateSQL(const QString &name,
                          const QString &sql,
                          const QString &description,
                          const QString &ver = "8.1",
                          const QString &provider = "Oracle",
                          bool modified = true);

    /** Remove an SQL statement from a map. If the last version is removed
     * from a statement it's definition is also removed.
     * @param name Name to remove.
     * @param ver Version to remove.
     * @param provider Provider to delete.
     * @return True if a version was found to be removed.
     */
    static bool deleteSQL(const QString &name,
                          const QString &ver,
                          const QString &provider = "Oracle");

    /** Get the statement of an SQL.
     * @param name Name of statement.
     * @param conn Connection to get version from.
     * @return String containing the statement.
     * @exception QString with description of problem fetching string.
     */
    static QString string(const QString &name, const toConnection &conn);
    /** Get the statement of an SQL.
     * @param sqlDef SQL to get name of statement from.
     * @param conn Connection to get version from.
     * @return String containing the statement.
     * @exception QString with description of problem fetching string.
     */
    static QString string(const toSQL &sqldef, const toConnection &conn)
    {
        return string(sqldef.Name, conn);
    }

    /** Get description of an SQL.
     * @param name Name of SQL to get name from..
     * @return String containing description.
     * @exception QString with description of problem fetching string.
     */
    static QString description(const QString &name);
    /** Get description of an SQL.
     * @param sqlDef SQL to get name of statement from.
     * @return String containing description.
     * @exception QString with description of problem fetching string.
     */
    static QString description(const toSQL &sql)
    {
        return description(sql.Name);
    }

    /** Get the statement of an SQL.
     * @param name Name of statement.
     * @param conn Connection to get version from.
     * @return String containing the statement.
     * @exception QString with description of problem fetching string.
     */
    static QString sql(const QString &name, const toConnection &conn)
    {
        return string(name, conn).toUtf8();
    }
    /** Get the statement of an SQL.
     * @param sqlDef SQL to get name of statement from.
     * @param conn Connection to get version from.
     * @return String containing the statement.
     * @exception QString with description of problem fetching string.
     */
    static QString sql(const toSQL &sqldef, const toConnection &conn)
    {
        return sql(sqldef.Name, conn);
    }

    /** Get an SQL from a specified name.
     * @param name Name to get SQL for.
     * @return a toSQL object for given name.
     */
    static toSQL sql(const QString &name);

    /**
     * Get all defined names that start with a given substring.
     * @param startWith The string that the name should start with.
     * @return A list of names available.
     */
    static std::list<QString> range(const QString &startWith);

    /** Save SQL definitions to file.
     * @param file Filename to save to.
     * @param all If true all statements will be saved, otherwised only modified are saved.
     * @return True if saved successfully.
     */
    static bool saveSQL(const QString &file, bool all = false);
    /** Load definitions from file.
     * @param file Filename to load from.
     * @exceptions QString describing the problem loading.
     */
    static void loadSQL(const QString &file);

    /** Get the entire map of SQL definitions
     * @return A reference to the map of definitions.
     */
    static const sqlMap &definitions(void)
    {
        allocCheck();
        return *Definitions;
    }

    /** Get the statement of an SQL.
     * @param conn Connection to get version from.
     * @return String containing the statement.
     */
    const QString operator () (const toConnection &conn) const
    {
        return string(Name, conn);
    }

    /** Get name of this SQL.
     * @return Name.
     */
    const QString &name(void) const
    {
        return Name;
    }

    /** Create a new SQL. Observe that the only thing this does is insert the supplied
     * information into the definition map, deleting the SQL will not the information from
     * the defitinion map. Only one description can be made for each statement name.
     * @param name Name of SQL.
     * @param sql Statement of this SQL.
     * @param description Description of statement.
     * @param ver Version this statement applies to.
     * @param provider Provider this string is for.
     */
    toSQL(const QString &name,
          const QString &sql,
          const QString &description = "",
          const QString &ver = "8.1",
          const QString &provider = "Oracle");
};

#endif
