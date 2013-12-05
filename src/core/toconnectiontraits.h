
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

#ifndef TOCONNECTIONTRAITS_H
#define TOCONNECTIONTRAITS_H

#include "core/tocache.h"

#include <QtCore/QString>
#include <QtCore/QList>

class toConnection;

/**
 * This class provides various Database specific logic.
 * Each DB connection provider should subclass this.
 */
class toConnectionTraits
{
public:
    /** Return a string representation to address an object.
    * @param name The name to be quoted.
    * @param quoteLowercase Enclose in quotes when identifier has lowercase letters
    * @return String addressing table.
    */
    virtual QString quote(const QString &name) const = 0;

    /** Perform the opposite of @ref quote.
    * @param name The name to be un-quoted.
    * @return String addressing table.
    */
    virtual QString unQuote(const QString &name) const = 0;

    /** Check if connection provider supports table level comments.
     *  @return bool return true if database supports table level comments
     *  See toSQL: toResultCols:TableComment
     */
    virtual bool hasTableComments() const = 0;

    /** Check if connection provider supports asynchronous query break.
     *  @return bool return true if db query can be canceled from bg thread
     */
    virtual bool hasAsyncBreak() const = 0;

    /**
     * Return list of primary key columns for a table
     * By default return an empty list => table can not be modified using toResultTableViewEdit
     */
    virtual QList<QString> primaryKeys(toConnection &, toCache::ObjectRef const&) const { return QList<QString>(); };

    virtual ~toConnectionTraits() {};
};

#endif

