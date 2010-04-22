
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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultfield.h"
#include "tosql.h"
#include "tosqlparse.h"
#include "totool.h"


toResultField::toResultField(QWidget *parent, const char *name)
        : toHighlightedText(parent, name)
{
    setReadOnly(true);
    Query = NULL;
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
    whichResultField = 1;
}

toResultField::~toResultField()
{
    delete Query;
}

void toResultField::query(const QString &sql, const toQList &param)
{
    if (!setSQLParams(sql, param))
        return ;

    setFilename(QString::null);

    try
    {
        clear();
        Unapplied = QString::null;
        if (Query)
        {
            delete Query;
            Query = NULL;
        }

        Query = new toNoBlockQuery(connection(), toQuery::Background, sql, param);
        Poll.start(100);
    }
    TOCATCH
}

#define THRESHOLD 10240

void toResultField::poll(void)
{
    try
    {
        if (!toCheckModal(this))
            return ;
        if (Query && Query->poll())
        {
            while (Query->poll() && !Query->eof())
            {
                // For some MySQL statements (say "show create function aaa.bbb") more than one column is returned
                // and it is not possible to control that (or I do not know how to do it). This workaround will get
                // a required field (say 3rd) from a result set returned.
                int fieldNo = whichResultField; // by default this would be set to 1 in constructor
                while (fieldNo > 1)
                {
                    fieldNo--;
                    Query->readValue();
                }
                Unapplied += Query->readValue();

                // Read any remaining columns for queries with specific field to fetch.
                // This is primarily used for MySQL statements like "show create..." which
                // return different uncontrollable number of fields for different users.
                // If remaining fields are not fetched polling thread will loop.
                if (whichResultField > 1)
                    while (!Query->eof())
                        Query->readValue();
            }
            if (Unapplied.length() > THRESHOLD)
            {
                append(Unapplied);
                Unapplied = QString::null;
            }
            if (Query->eof())
            {
                delete Query;
                Query = NULL;
                Poll.stop();
                try
                {
                    // Code is formatted if it is set in preferences (Preferences->Editor Extensions) to
                    // indent (format) a read only code AND! if it is not a MySQL code because current
                    // TOra code parser/indenter does not work correctly with MySQL code (routines)
                    if (toConfigurationSingle::Instance().autoIndentRo() && !toIsMySQL(connection()))
                        setText(toSQLParse::indent(text() + Unapplied));
                    else
                        append(Unapplied);
                }
                TOCATCH
                Unapplied = QString::null;
            }
        }
    }
    catch (const QString &exc)
    {
        delete Query;
        Query = NULL;
        Poll.stop();
        toStatusMessage(exc);
    }
}
