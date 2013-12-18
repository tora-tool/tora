
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "core/toquery.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectiontraits.h"
#include "core/tosql.h"

#include <QtGui/QApplication>

toQuery::toQuery(toConnectionSubLoan &conn, const toSQL &sql, toQueryParams const& params)
    : m_ConnectionSubLoan(conn)
    , m_Params(params)
	, m_SQL(sql(conn.ParentConnection).toAscii())
    , m_Query(NULL)
	, m_eof(false)
	, m_rowsProcessed(0)
{
	init();
}

toQuery::toQuery(toConnectionSubLoan &conn, QString const& sql, toQueryParams const& params)
    : m_ConnectionSubLoan(conn)
    , m_Params(params)
    , m_SQL(sql)
    , m_Query(NULL)
	, m_eof(false)
	, m_rowsProcessed(0)
{
	init();
}

void toQuery::init()
{
    try
    {
    	// Try to switch the current db schema
    	if (m_ConnectionSubLoan.SchemaInitialized == false && !m_ConnectionSubLoan.Schema.isEmpty())
    	{
	        QString sql = m_ConnectionSubLoan.ParentConnection.getTraits().schemaSwitchSQL(m_ConnectionSubLoan.Schema);
	        if (!sql.isEmpty())
	        {
	        	m_Query = m_ConnectionSubLoan->createQuery(this);
	        	m_ConnectionSubLoan->setQuery(this);
	        	m_Query->execute(sql);
	        	delete m_Query;
	        }
	        m_ConnectionSubLoan.SchemaInitialized = true;
	        m_ConnectionSubLoan->setSchema(m_ConnectionSubLoan.Schema); // assign value in toConnectionSub from toConnectionSubLoan
    	}

    	// Try to switch the current NLS_SETTINGS (not implemented yet)
    	if( m_ConnectionSubLoan->isInitialized() == false)
    	{
    		Q_FOREACH(QString sql, m_ConnectionSubLoan.ParentConnection.initStrings())
			{
    	        m_Query = m_ConnectionSubLoan->createQuery(this);
    	        m_ConnectionSubLoan->setQuery(this);
    	        m_Query->execute(sql);
    	        delete m_Query;
			}
    		m_ConnectionSubLoan->setInitialized(true);
    	}

        m_Query = m_ConnectionSubLoan->createQuery(this);
        m_ConnectionSubLoan->setQuery(this);
        m_Query->execute();
    }
    catch (...)
    {
        if (m_Query)
            delete m_Query;
        m_ConnectionSubLoan->setQuery(NULL);
        m_Query = NULL;
        throw;
    }
}

toQuery::~toQuery()
{
    if (m_Query)
        delete m_Query;

//    try {
//		QString sql = toSQL::string("Global:CurrentSchema", connection());
//		if (!sql.isEmpty())
//		{
//			m_Query = m_ConnectionSubLoan->createQuery(this);
//			m_ConnectionSubLoan->setQuery(this);
//			m_Query->execute(sql);
//			QString schema = m_Query->readValue();
//			m_ConnectionSubLoan->setSchema(schema);
//			delete m_Query;
//		}
//    } catch (...) {
//    }

    if (m_ConnectionSubLoan->query() == this)
    	m_ConnectionSubLoan->setQuery(NULL);
}

bool toQuery::eof(void)
{
	bool retval = m_Query ? m_Query->eof() : true;

	// eof value was flip over (do not call this each time)
	if (retval && !m_eof) //
	{
		m_eof = retval;
	    try {
			QString sql = toSQL::string("Global:CurrentSchema", connection());
			if (!sql.isEmpty())
			{
				queryImpl* Query = m_ConnectionSubLoan->createQuery(this);
				//m_ConnectionSubLoan->setQuery(this);
				Query->execute(sql);
				QString schema = Query->readValue();
				m_ConnectionSubLoan->setSchema(schema);
				delete Query;
			}
	    } catch (...) {
	    }
	    m_rowsProcessed = m_Query->rowsProcessed();
	    if (m_Query)
	    	delete m_Query;
	    m_Query = NULL;
	}

    return retval;
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
