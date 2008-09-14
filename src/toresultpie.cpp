/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultpie.h"
#include "tosql.h"


toResultPie::toResultPie(QWidget *parent, const char *name)
        : toPieChart(parent, name)
{
    Query = NULL;
    Columns = 0;
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
    Started = false;
    LabelFirst = false;
}

void toResultPie::start(void)
{
    if (!Started)
    {
        try
        {
            connect(timer(), SIGNAL(timeout()), this, SLOT(refresh()));
        }
        TOCATCH
        Started = true;
    }
}

void toResultPie::stop(void)
{
    if (Started)
    {
        try
        {
            disconnect(timer(), SIGNAL(timeout()), this, SLOT(refresh()));
        }
        TOCATCH
        Started = false;
    }
}

void toResultPie::query(const QString &sql, const toQList &param)
{
    if (!handled() || Query)
        return ;

    start();
    if (!setSQLParams(sql, param))
        return ;

    try
    {
        Query = new toNoBlockQuery(connection(), toQuery::Background, sql, param);
        Poll.start(100);
    }
    TOCATCH
}

void toResultPie::poll(void)
{
    try
    {
        if (Query && Query->poll())
        {
            if (!Columns)
                Columns = Query->describe().size();
            while (Query->poll() && !Query->eof())
            {
                QString val;
                QString lab;
                if (Columns > 1)
                {
                    if (LabelFirst)
                    {
                        lab = Query->readValueNull();
                        val = Query->readValueNull();
                    }
                    else
                    {
                        val = Query->readValueNull();
                        lab = Query->readValueNull();
                    }
                    for (int i = 2;i < Columns;i++)
                        Query->readValueNull();
                }
                else
                    val = Query->readValueNull();
                if (!Filter.isEmpty() && !Filter.exactMatch(lab))
                    continue;
                if (!ValueFilter.isEmpty() && !ValueFilter.exactMatch(val))
                    continue;
                Values.insert(Values.end(), val.toDouble());
                if (Columns > 1)
                    Labels.insert(Labels.end(), lab);
            }
            if (Query->eof())
            {
                setValues(Values, Labels);
                Values.clear();
                Labels.clear();
                delete Query;
                Query = NULL;
                Columns = 0;
                Poll.stop();
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
