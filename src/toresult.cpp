
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
#include "toresult.h"
#include "totabwidget.h"
#include "totool.h"

#include <qtabwidget.h>
#include <qtimer.h>


toResult::toResult()
        : Slots(this),
        Handled(true),
        Tabs(0),
        TabWidget(0),
        ForceRefresh(false),
        QueryReady(false),
        FromSQL(false),
        DisableTab(true)
{
    QTimer::singleShot(1, &Slots, SLOT(setup()));
}

void toResult::changeHandle(void)
{
    if (!DisableTab)
        return;

    QWidget *widget = dynamic_cast<QWidget *>(this);

    if (!widget)
        return;

    widget->setEnabled(handled());

    // find totabwidget
    QWidget *parent = widget;
    while (parent && parent->metaObject()->className() != QString("toTabWidget"))
        parent = parent->parentWidget();
    toTabWidget *tw = dynamic_cast<toTabWidget *>(parent);

    if (tw)
        tw->setTabShown(widget, handled());
}

void toResult::setHandle(bool ena)
{
    bool last = Handled;
    try
    {
        if (!ena)
            Handled = false;
        else
            Handled = canHandle(connection());
    }
    catch (...)
    {
        Handled = false;
    }
    if (last != Handled)
        changeHandle();
}

void toResult::connectionChanged(void)
{
    ForceRefresh = true;
    if (FromSQL)
    {
        try
        {
            if (QueryReady)
                query(toSQL::string(sqlName().toLatin1(), connection()), (const toQList)Params);
            else if (FromSQL)
                SQL = toSQL::string(sqlName().toLatin1(), connection());
            setHandle(true);
        }
        catch (...)
        {
            setHandle(false);
        }
    }
    else
        setHandle(true);

}

toTimer *toResult::timer(void)
{
    return toCurrentTool(dynamic_cast<QWidget *>(this))->timer();
}

toConnection &toResult::connection(void)
{
    return toCurrentConnection(dynamic_cast<QWidget *>(this));
}

void toResult::query(const QString &sql)
{
    toQList params;
    query(sql, (const toQList)params);
}

void toResult::query(const toSQL &sql)
{
    setSQLName(sql.name());
    FromSQL = true;
    try
    {
        toQList params;
        query((const QString)toSQL::string(sql, connection()), (const toQList)params);
        setHandle(true);
    }
    catch (...)
    {
        setHandle(false);
    }
}

void toResult::query(const toSQL &sql, const toQList &par)
{
    setSQLName(sql.name());
    FromSQL = true;
    try
    {
        query((const QString)toSQL::string(sql, connection()), (const toQList)par);
        setHandle(true);
    }
    catch (...)
    {
        setHandle(false);
    }
}
// #include <QtDebug>
void toResult::setSQL(const toSQL &sql)
{
//     qDebug() << "setSQL";
//     qDebug() << "setSQL" << sql.name();
    setSQLName(sql.name());
    FromSQL = true;

    try
    {
//         qDebug() << "setSQL 1";
        Params.clear();
//         qDebug() << "setSQL 2";
        setSQL(toSQL::string(sql, connection()));
//         qDebug() << "setSQL 3";
        setHandle(true);
    }
    catch (...)
    {
//         qDebug() << "setSQL failed:" << sql.name();
        setHandle(false);
    }
}

void toResult::changeParams(const QString &Param1, const QString &Param2, const QString &Param3)
{
    toQList params;
    toPush(params, toQValue(Param1));
    toPush(params, toQValue(Param2));
    toPush(params, toQValue(Param3));
    query((const QString)SQL, (const toQList)params);
}

void toResult::changeParams(const QString &Param1, const QString &Param2)
{
    toQList params;
    toPush(params, toQValue(Param1));
    toPush(params, toQValue(Param2));
    query((const QString)SQL, (const toQList)params);
}

void toResult::changeParams(const QString &Param1)
{
    toQList params;
    toPush(params, toQValue(Param1));
    query((const QString)SQL, (const toQList)params);
}

void toResultObject::connectionChanged(void)
{
    Result->connectionChanged();
}

void toResultObject::setup(void)
{
    QObject *obj = dynamic_cast<QObject *>(Result);
    if (!obj)
    {
        toStatusMessage(tr("Internal error, toResult is not a descendant of toResult"));
        return ;
    }
    try
    {
        QObject::connect(toCurrentTool(obj), SIGNAL(connectionChange()), this, SLOT(connectionChanged()));
    }
    catch (...)
        {}
    try
    {
        if (Result->Handled)
            Result->Handled = Result->canHandle(Result->connection());
    }
    catch (...)
    {
        Result->Handled = false;
    }
    if (!Result->Handled)
        Result->changeHandle();
}

bool toResult::setSQLParams(const QString &sql, const toQList &par)
{
    bool force = ForceRefresh;
    ForceRefresh = false;
    if (toConfigurationSingle::Instance().dontReread())
    {
        if (SQL == sql && par.size() == Params.size())
        {
            toQList::iterator i = ((toQList &)par).begin();
            toQList::iterator j = Params.begin();
            while (i != ((toQList &)par).end() && j != Params.end())
            {
                if (QString(*i) != QString(*j))
                    break;
                i++;
                j++;
            }
            if (i == ((toQList &)par).end() && j == Params.end())
                return force;
        }
    }
    SQL = sql;
    Params = par;
    QueryReady = true;
    return true;
}

void toResult::refresh()
{
    ForceRefresh = true;
    query((const QString &)SQL, (const toQList &)Params);
}
