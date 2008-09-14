/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"
#include "toresultschema.h"
#include "toconnection.h"

#include <QSettings>


toResultSchema::toResultSchema(toConnection &conn,
                               QWidget *parent,
                               const char *name)
    : toResultCombo(parent, name)
{
    setSQL(toSQL::sql(toSQL::TOSQL_USERLIST));

    ConnectionKey =
        conn.provider() + "-" +
        conn.host() + "-" +
        conn.database();

    QString sel = conn.schema();

    if(sel.isEmpty())
    {
        QSettings s;
        sel = s.value("schema/" + ConnectionKey).toString();
    }

    if(sel.isEmpty())
    {
        if (toIsMySQL(conn))
            sel = conn.database();
        else if (toIsOracle(conn) || toIsSapDB(conn))
            sel = conn.user().toUpper();
        else
            sel = conn.user();
    }

    conn.setSchema(sel);
    setSelected(sel);
    connect(this,
            SIGNAL(currentIndexChanged(const QString &)),
            this,
            SLOT(updateLastSchema(const QString &)));
}


#define CHANGE_CURRENT_SCHEMA QString("ALTER SESSION SET CURRENT_SCHEMA = %1")
#define CHANGE_CURRENT_SCHEMA_PG QString("SET search_path TO %1,\"$user\",public")

void toResultSchema::update() {
    update(toResultCombo::currentText());
}


void toResultSchema::update(const QString &schema) {
    if(schema.isEmpty())
        return;

    try {
        toConnection &conn = connection();

        if (toIsOracle(conn)) {
            /* remove current schema initstring */
            conn.delInit(CHANGE_CURRENT_SCHEMA.arg(conn.user()));

            /* set the new one with selected schema */
            QString sql = CHANGE_CURRENT_SCHEMA.arg(schema);
            conn.allExecute(sql);

            conn.addInit(sql);
        }
        else if (toIsMySQL(conn)) {
            conn.allExecute(QString("USE `%1`").arg(schema));
            conn.setDatabase(schema);
        }
        else if (toIsPostgreSQL(conn))
            conn.allExecute(CHANGE_CURRENT_SCHEMA_PG.arg(schema));
        else
            throw QString("No support for changing schema for this database");

        conn.setSchema(schema);
    }
    TOCATCH;
}


void toResultSchema::updateLastSchema(const QString &schema) {
    QSettings s;
    s.setValue("schema/" + ConnectionKey, schema);
}
