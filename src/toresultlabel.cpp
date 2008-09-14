/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultlabel.h"
#include "tosql.h"
#include "totool.h"

//Added by qt3to4:
#include <QLabel>

toResultLabel::toResultLabel(QWidget *parent, const char *name)
        : QLabel(parent)
{
    setObjectName(name);
    Query = NULL;
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
}

toResultLabel::~toResultLabel()
{
    delete Query;
}

void toResultLabel::query(const QString &sql, const toQList &param)
{
    if (!setSQLParams(sql, param))
        return ;

    try
    {
        clear();
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

void toResultLabel::poll(void)
{
    try
    {
        if (!toCheckModal(this))
            return ;
        if (Query && Query->poll())
        {
            QStringList res;
            while (!Query->eof())
            {
                res << Query->readValue();
            }
            setText(res.join(QString::fromLatin1("/")));
            delete Query;
            Query = NULL;
            Poll.stop();
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
