
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

#include "core/toresult.h"
#include "core/utils.h"
#include "core/totabwidget.h"
#include "core/totool.h"
#include "core/toconfiguration.h"

#include <QtCore/QTimer>

void toResultObject::slotConnectionChanged(void)
{
    Result->connectionChanged();
}

void toResultObject::setup(void)
{
    QObject *obj = dynamic_cast<QObject *>(Result);
    if (!obj)
    {
        Utils::toStatusMessage(tr("Internal error, toResult is not a descendant of toResult"));
        return ;
    }
    try
    {
        QObject::connect(toToolWidget::currentTool(obj), SIGNAL(connectionChange()), this, SLOT(slotConnectionChanged()));
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
    try
    {
        if (Result->Handled)
            Result->Handled = Result->canHandle(Result->connection());
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        Result->Handled = false;
    }
    if (!Result->Handled)
        Result->changeHandle();
}

toResult::toResult()
    : Slots(this)
    , Handled(true)
    , NeedsRefresh(true)
    , QueryReady(false)
    , FromSQL(false)
    , IsCriticalTab(true)
    , Params()
{
    //see EventDispatcherWin32Private::registerTimer time should be either 0 or >20
    //otherwise the application hungs windows - bacause QT starts a new thread with RT priority
	//
	// Few more comments on this:
	//  - Tora uses multiple inheritance(subclass of the toResult also inherits from QWidget)
	//  - QT does not allow to inherit from QObject twice
	//  - when this is processed the vptr points into toResult's table, the object is not fully created yet
	//    the dynamic_cast does not work yet
	//
	//  - toResultObject::setup uses dynamic_cast and connects "this" to some signals
    QTimer::singleShot(0, &Slots, SLOT(setup()));
}

void toResult::clearParams(void)
{
    Params.clear();
    QueryReady = false;
}

void toResult::refresh()
{
    NeedsRefresh = true;
    query((const QString &)SQL, Params);
}

void toResult::refreshWithParams(toQueryParams const& params)
{
    query((const QString)SQL, params);
    // Grr the whole toResult is a mess
    // Params can also be set from query -> setSqlAndParams
    Params = params;
}

bool toResult::canHandle(toConnection &)
{
    return false;
}

toQueryParams const& toResult::params(void)
{
    return Params;
}

bool toResult::handled(void)
{
    return Handled;
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
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        Handled = false;
    }
    if (last != Handled)
        changeHandle();
}

void toResult::setDisableTab(bool en)
{
    IsCriticalTab = en;
}

void toResult::setSQL(const QString &sql)
{
    SQL = sql;
}

void toResult::setSQL(const toSQL &sql)
{
    setSQLName(sql.name());
    FromSQL = true;
    try
    {
        Params.clear();
        setSQL(toSQL::string(sql, connection()));
        setHandle(true);
    }
    catch (QString const& e)
    {
        TLOG(8, toDecorator, __HERE__) << e << std::endl;
        setHandle(false);
    }
}

QString toResult::sql(void)
{
    return SQL;
}

void toResult::removeSQL()
{
	setSQLName("");
	SQL = "";
	FromSQL = false;
	Params.clear();
	// setHandle(false); //  ibre5041 this hide the whole tab. Use this only if something fails
}

QString toResult::sqlName(void)
{
    return Name;
}

void toResult::setSQLName(const QString &name)
{
    Name = name;
}

toTimer *toResult::timer(void)
{
    return toToolWidget::currentTool(dynamic_cast<QWidget *>(this))->timer();
}

void toResult::connectionChanged(void)
{
    NeedsRefresh = true;
    if (FromSQL)
    {
        try
        {
            if (QueryReady)
                query(toSQL::string(sqlName().toLatin1(), connection()), Params);
            else if (FromSQL)
                SQL = toSQL::string(sqlName().toLatin1(), connection());
            setHandle(true);
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
            setHandle(false);
        }
    }
    else
        setHandle(true);

}

void toResult::setParams(toQueryParams const& par)
{
    Params = par;
    QueryReady = true;
}

bool toResult::setSqlAndParams(const QString &sql, toQueryParams const& par)
{
    bool force = NeedsRefresh;
    NeedsRefresh = false;

    if (toConfigurationSingle::Instance().dontReread() && SQL == sql && Params == par && force == false)
        return false;

    // Enter the inner loop if and only if the NeedsRefresh was false
    /*
      NOTE: it code is here just to prove that QT's container's iterators are sick
      result of comparsion between const and non-const iterator is undefined
      uncomment this is you think that this function gives unpredictible results.
    */
    // if (toConfigurationSingle::Instance().dontReread() && force == false)
    // {
    // 	    if (SQL == sql && par.size() == Params.size())

    // 	    {
    // 		    bool c = par == Params;
    // 		    int ii = par.size();
    // 		    int jj = Params.size();
    // 		    toQueryParams::const_iterator i = par.constBegin();
    // 		    toQueryParams::const_iterator j = Params.constBegin();
    // 		    while (i != par.constEnd() && j != Params.constEnd())
    // 		    {
    // 			    QString si(*i);
    // 			    QString sj(*j);
    // 			    if (QString(*i) != QString(*j))
    // 				    break;
    // 			    i++;
    // 			    j++;
    // 		    }
    // 		    bool c1 = i == par.constEnd();
    // 		    bool c2 = j == Params.constEnd();
    // 		    if (i == par.constEnd() && j == Params.constEnd())
    // 			    return force;
    // 	    }
    // }
    SQL = sql;
    Params = par;
    QueryReady = true;
    return true;
}

toConnection &toResult::connection(void)
{
    return toConnection::currentConnection(dynamic_cast<QWidget *>(this));
}

void toResult::changeHandle(void)
{
    if (!IsCriticalTab)
        return;

    QWidget *widget = dynamic_cast<QWidget *>(this);

    if (!widget)
        return;

    widget->setEnabled(handled());

    // find totabwidget
    QWidget *parent = widget;
    while (parent && parent->metaObject()->className() != QString("toTabWidget"))
        parent = parent->parentWidget();

    if (toTabWidget *tw = dynamic_cast<toTabWidget *>(parent))
        tw->setTabShown(widget, handled());
}
