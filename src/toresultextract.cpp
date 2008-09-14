/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "toextract.h"
#include "tohighlightedtext.h"
#include "toresultextract.h"
#include "tosql.h"
#include "totool.h"


toResultExtract::toResultExtract(bool prompt, QWidget *parent, const char *name)
        : toWorksheetWidget(parent, name, toCurrentConnection(parent))
{
    Prompt = prompt;
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
