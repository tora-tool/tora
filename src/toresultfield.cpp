/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
                Unapplied += Query->readValue();
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
                    if (toConfigurationSingle::Instance().autoIndentRo())
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
