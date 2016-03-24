
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

#include "core/toconnectiontraits.h"

class toOracleTraits: public toConnectionTraits
{
    public:
        /** Return a string representation to address an object.
        * @param name The name to be quoted.
        * @param quoteLowercase Enclose in quotes when identifier has lowercase letters
        * @return String addressing table.
        */
        QString quote(const QString &name) const override;

        /** Perform the opposite of @ref quote.
        * @param name The name to be un-quoted.
        * @return String addressing table.
        */
        QString unQuote(const QString &name) const override;

        QString quoteVarchar(const QString &name) const override;

        QString formatDate(const QVariant &value) const override;

        /** Generate SQL statement for Schema switch
         * @param Schema/(Database) name
         * @return SQL statement
         */
        QString schemaSwitchSQL(QString const&) const override;

        /** Check if connection provider supports table level comments.
         *  @return bool return true if database supports table level comments
         *  See toSQL: toResultCols:TableComment
         */
        bool hasTableComments() const override
        {
            return true;
        }

        bool hasAsyncBreak() const override
        {
            return true;
        }

        virtual QList<QString> primaryKeys(toConnection &, toCache::ObjectRef const&) const;
};
