/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-20010 Numerous Other Contributors
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

#include "core/toquery.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/tosql.h"

#include <QtGui/QApplication>

toQuery::toQuery(toConnectionSubLoan &conn, const toSQL &sql, toQueryParams const& params)
    : m_ConnectionSub(conn)
    , m_Params(params)
	, m_SQL(sql(conn.ParentConnection).toAscii())
    , m_Query(NULL)
{
	init();
}

toQuery::toQuery(toConnectionSubLoan &conn, QString const& sql, toQueryParams const& params)
    : m_ConnectionSub(conn)
    , m_Params(params)
    , m_SQL(sql)
    , m_Query(NULL)
{
	init();
}

void toQuery::init()
{
    try
    {
    	if ( m_ConnectionSub.InitMode == toConnectionSubLoan::INIT_SESSION)
    	{
    		Q_FOREACH(QString sql, m_ConnectionSub.ParentConnection.initStrings())
			{
    	        m_Query = m_ConnectionSub->createQuery(this);
    	        m_ConnectionSub->setQuery(this);
    	        m_Query->execute(sql);
			}
    	}
		  
        m_Query = m_ConnectionSub->createQuery(this);
        m_ConnectionSub->setQuery(this);
        m_Query->execute();
    }
    catch (...)
    {
        if (m_Query)
            delete m_Query;
        m_ConnectionSub->setQuery(NULL);
        m_Query = NULL;
        throw;
    }
}

toQuery::~toQuery()
{
    Utils::toBusy busy;
    if (m_Query)
        delete m_Query;
    try
    {
        if (m_ConnectionSub->query() == this)
            m_ConnectionSub->setQuery(NULL);
        //conn if (Mode != Test && Connection && Connection->ConnectionPool)
        //conn    Connection->ConnectionPool->release(ConnectionSub);
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
}

bool toQuery::eof(void)
{
    return m_Query->eof();
}

toQList toQuery::readQuery(toConnection &conn, toSQL const& sql, toQueryParams const& params)
{
    Utils::toBusy busy;
    toConnectionSubLoan loan(conn);
    toQuery query(loan, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

toQList toQuery::readQuery(toConnection &conn, QString const& sql, toQueryParams const& params)
{
    Utils::toBusy busy;
    toConnectionSubLoan loan(conn);
    toQuery query(loan, sql, params);
    toQList ret;
    while (!query.eof())
        ret.insert(ret.end(), query.readValue());
    return ret;
}

//toQList toQuery::readQuery(QString const& sql, toQueryParams const& params)
//{
//    Utils::toBusy busy(m_ShowBusy);
//    m_SQL = sql;
//    m_Params = params;
//    m_Query->execute();
//    toQList ret;
//    while (!eof())
//        ret.insert(ret.end(), readValue());
//    return ret;
//}

toQValue toQuery::readValue(void)
{
    if (connection().Abort)
        throw qApp->translate("toQuery", "Query aborted");
    return m_Query->readValue();
}

toQColumnDescriptionList toQuery::describe(void)
{
    return m_Query->describe();
}
