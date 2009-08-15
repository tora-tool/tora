
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
#include "toextract.h"
#include "tohighlightedtext.h"
#include "toresultextract.h"
#include "tosql.h"
#include "totool.h"
#include "tomain.h"


toResultExtract::toResultExtract(bool prompt, QWidget *parent, const char *name)
        : toWorksheetWidget(parent, name, toCurrentConnection(parent))
{
    Prompt = prompt;
}

toResultExtract::toResultExtract(QWidget * parent)
    : toWorksheetWidget(parent, "toResultExtract", toMainWidget()->currentConnection())
{
    Prompt = false;
}

static toSQL SQLObjectTypeMySQL("toResultExtract:ObjectType",
                                "SELECT IF(LOCATE('.',CONCAT(:f1,:f2))>0,'INDEX','TABLE')",
                                "Get type of an object by name",
                                "4.1",
                                "MySQL");

static toSQL SQLObjectType("toResultExtract:ObjectType",
                           "SELECT Object_Type FROM sys.All_Objects\n"
                           " WHERE Owner = :f1<char[101]> AND Object_Name = :f2<char[101]>",
                           "");

void toResultExtract::query(const QString &sql, const toQList &param)
{
    if (!setSQLParams(sql, param))
        return ;

    try
    {
        toQList::iterator i = params().begin();
        QString owner;
        QString name;
        if (i != params().end())
        {
            owner = *i;
            i++;
        }
        toConnection &conn = toToolWidget::connection();
        if (i == params().end())
        {
            name = owner;
            if (toIsOracle(conn))
                owner = conn.user().toUpper();
            else
                owner = conn.user();
        }
        else
        {
            name = *i;
            i++;
        }

        QString type;
        if (i == params().end())
        {
            toQuery query(conn, SQLObjectType, owner, name);

            if (query.eof())
                throw tr("Object not found");

            type = query.readValue();
        }
        else
            type = *i;

        std::list<QString> objects;

        if (toIsOracle(conn))
        {
            if (type == QString::fromLatin1("TABLE") ||
                    type == QString::fromLatin1("TABLE PARTITION"))
            {
                objects.insert(objects.end(), QString::fromLatin1("TABLE FAMILY:") + owner + QString::fromLatin1(".") + name);
                objects.insert(objects.end(), QString::fromLatin1("TABLE REFERENCES:") + owner + QString::fromLatin1(".") + name);
            }
            else if (type.startsWith(QString::fromLatin1("PACKAGE")) && Prompt)
            {
                objects.insert(objects.end(), QString::fromLatin1("PACKAGE:") + owner + QString::fromLatin1(".") + name);
                objects.insert(objects.end(), QString::fromLatin1("PACKAGE BODY:") + owner + QString::fromLatin1(".") + name);
            }
            else
                objects.insert(objects.end(), type + QString::fromLatin1(":") + owner + QString::fromLatin1(".") + name);
        }
        else
            objects.insert(objects.end(), type + QString::fromLatin1(":") + owner + QString::fromLatin1(".") + name);

        toExtract extract(conn, NULL);
        extract.setCode(true);
        extract.setHeading(false);
        extract.setPrompt(Prompt);
        editor()->setText(extract.create(objects));
    }
    TOCATCH
}

bool toResultExtract::canHandle(toConnection &conn)
{
    try
    {
        return toExtract::canHandle(conn) && !toSQL::string(SQLObjectType, conn).isEmpty();
    }
    catch (...)
    {
        return false;
    }
}
