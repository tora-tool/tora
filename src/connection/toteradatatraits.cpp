
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

#include "connection/toteradatatraits.h"
#include "core/utils.h"
#include "core/toconfiguration_new.h"
#include "editor/toworksheettext.h"

/** Return a string representation to address an object.
*
* mrj: todo copied from tooracleconnection
*
*
* Checks if identifier has illegal characters, starts with digit, is a reserved
* word etc. - if so - returns it enclosed with quotes (otherwise returns the same string).
* Note that when identifier name returned from oracle data dictionary is in lowercase
* - it MUST be enclosed with quotes (case insensitive "normal" identifiers are always
* returned in uppercase).
* @param name The name to be quoted.
* @param quoteLowercase Enclose in quotes when identifier has lowercase letters.
*   When processing data returned by dada dictionary quoteLowercase should be true
*   When processing data entered by user quoteLowercase should be false
* @return String addressing table.
*/
QString toTeradataTraits::quote(QString const &name) const
{
    using namespace ToConfiguration;

    bool mustBeQuoted = false;
    // Identifiers starting with digit should be quoted
    if (name.at(0).isDigit())
        mustBeQuoted = true;
    else
    {
        for (int i = 0; i < name.length(); i++)
        {
            if ((name.at(i).toUpper() != name.at(i)) || !Utils::toIsIdent(name.at(i)))
                mustBeQuoted = true;
        }
    }

    // Check if given identified is a reserved word
    int i = 0;
    //TODO add DefaultKeywords into connection provider traits
    //while (ok && (DefaultKeywords[i] != NULL))
    //{
    //  if (name.compare(DefaultKeywords[i], Qt::CaseInsensitive) == 0)
    //      ok = false;
    //  i++;
    //}
    if (mustBeQuoted)
        return QString::fromLatin1("\"") + name + QString::fromLatin1("\"");

    if (toConfigurationNewSingle::Instance().option(Editor::ObjectNamesUpperBool).toBool())
        return name.toUpper();
    else
        return name.toLower();
}

QString toTeradataTraits::unQuote(QString const &str) const
{
    if (str.at(0).toLatin1() == '\"' && str.at(str.length() - 1).toLatin1() == '\"')
        return str.left(str.length() - 1).right(str.length() - 2);
    return str.toUpper();
}

QString toTeradataTraits::schemaSwitchSQL(QString const & schema) const
{
    static const QString ALTER_SESSION("DATABASE \"%1\"");
    return ALTER_SESSION.arg(schema);
}

