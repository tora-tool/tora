/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "tonoblockquery.h"
#include "toresultbar.h"
#include "tosql.h"
#include "totool.h"

#include <QMenu>


toResultBar::toResultBar(QWidget *parent, const char *name)
        : toBarChart(parent, name)
{
    connect(&Poll, SIGNAL(timeout()), this, SLOT(poll()));
    LastStamp = 0;
    Flow = true;
    Columns = 0;
    Query = NULL;
    Started = false;
}

toResultBar::~toResultBar()
{
    delete Query;
}

void toResultBar::start(void)
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

void toResultBar::stop(void)
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

void toResultBar::query(const QString &sql, const toQList &param, bool first)
{
    if (!handled() || Query)
        return ;

    start();
    setSQLParams(sql, param);

    try
    {
        First = first;
        Query = new toNoBlockQuery(connection(), toQuery::Background, sql, param);
        Poll.start(100);
    }
    TOCATCH
}

void toResultBar::poll(void)
{
    try
    {
        if (Query && Query->poll())
        {
            toQDescList desc;
            if (!Columns)
            {
                desc = Query->describe();
                Columns = desc.size();
            }

            if (First)
            {
                if ( desc.empty() )
                    desc = Query->describe();
                clear();
                std::list<QString> labels;
                for (toQDescList::iterator i = desc.begin();i != desc.end();i++)
                    if (i != desc.begin())
                        labels.insert(labels.end(), (*i).Name);
                setLabels(labels);
            }

            while (Query->poll() && !Query->eof())
            {
                unsigned int num = 0;
                QString lab = Query->readValue();
                num++;
                std::list<double> vals;
                while (!Query->eof() && num < Columns)
                {
                    vals.insert(vals.end(), Query->readValue().toDouble());
                    num++;
                }

                if (Flow)
                {
                    time_t now = time(NULL);
                    if (now != LastStamp)
                    {
                        if (LastValues.size() > 0)
                        {
                            std::list<double> dispVal;
                            std::list<double>::iterator i = vals.begin();
                            std::list<double>::iterator j = LastValues.begin();
                            while (i != vals.end() && j != LastValues.end())
                            {
                                dispVal.insert(dispVal.end(), (*i - *j) / (now - LastStamp));
                                i++;
                                j++;
                            }
                            std::list<double> tmp = transform(dispVal);
                            addValues(tmp, lab);
                        }
                        LastValues = vals;
                        LastStamp = now;
                    }
                }
                else
                {
                    std::list<double> tmp = transform(vals);
                    addValues(tmp, lab);
                }
            }
            if (Query->eof())
            {
                Poll.stop();
                Columns = 0;
                delete Query;
                Query = NULL;
                update();
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

std::list<double> toResultBar::transform(std::list<double> &input)
{
    return input;
}

void toResultBar::connectionChanged(void)
{
    toResult::connectionChanged();
    clear();
}

void toResultBar::addMenues(QMenu *popup)
{
    if (!sqlName().isEmpty())
    {
        popup->addSeparator();
        popup->addAction(tr("Edit SQL..."),
                         this,
                         SLOT(editSQL()));
    }
}

void toResultBar::editSQL(void)
{
    toMainWidget()->editSQL(sqlName());
}
