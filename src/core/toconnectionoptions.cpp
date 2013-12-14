
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

#include "core/toconnectionoptions.h"
#include "core/toconnection.h"

toConnectionOptions::toConnectionOptions(const QString &_prov,
		const QString &_host,
		const QString &_database,
		const QString &_user,
		const QString &_pass,
		const QString &_schema,
		const QString &_color,
		int _port,
		QSet<QString> _options)
	: provider (_prov)
	, host     (_host)
	, database (_database)
	, username (_user)
	, password (_pass)
	, schema   (_schema)
	, color    (_color)
	, port     (_port)
	, options  (_options)
{}

toConnectionOptions::toConnectionOptions(toConnectionOptions const& other)
	: provider (other.provider)
	, host     (other.host)
	, database (other.database)
	, username (other.username)
	, password (other.password)
	, schema   (other.schema)
	, color    (other.color)
	, port     (other.port)
	, options  (other.options)
{
}

toConnectionOptions& toConnectionOptions::operator=(const toConnectionOptions &other)
{
	provider = other.provider;
	host     = other.host;
	database = other.database;
	username = other.username;
	password = other.password;
	schema   = other.schema;
	color    = other.color;
	port     = other.port;
	options  = other.options;
	return *this;
}

bool toConnectionOptions::operator==(const toConnectionOptions &other) const
{
	if ( provider != other.provider
			|| host != other.host
			|| database != other.database
			|| username != other.username
			|| schema != other.schema
			|| color != other.color
			|| port != other.port)
		return false;

	if (options.size() != other.options.size())
		return false;

	QStringList opts1 = options.toList(); qSort(opts1);
	QStringList opts2 = other.options.toList(); qSort(opts2);
	for(int i = 0; i < opts1.size(); i++)
	{
		if (opts1.at(i) != opts2.at(i))
			return false;
	}
	return true;
}

bool toConnectionOptions::operator==(const toConnection &conn) const
{
    return conn.provider() == provider
    		&& conn.host() == (host + ":" + QString::number(port))
    		&& conn.database() == database
    		&& conn.user() == username
    		&& conn.color() == color
    		&& (schema.isEmpty() || (conn.defaultSchema() == schema));
}

template<> bool qMapLessThanKey<toConnectionOptions>(const toConnectionOptions &key1, const toConnectionOptions &key2)

{
	if (key1.provider != key2.provider)
    	return key1.provider > key2.provider;
	if (key1.database != key2.database)
    	return key1.database > key2.database;
	if (key1.port != key2.port)
    	return key1.port > key2.port;
	if (key1.host != key2.host)
    	return key1.host > key2.host;
	if (key1.username != key2.username)
    	return key1.username > key2.username;
	if (key1.schema != key2.schema)
    	return key1.schema > key2.schema;
	if (key1.color != key2.color)
    	return key1.color > key2.color;
	if (key1.options.size() != key2.options.size())
		return key1.options.size() > key2.options.size();
	QStringList opts1 = key1.options.toList(); qSort(opts1);
	QStringList opts2 = key2.options.toList(); qSort(opts2);
	for(int i = 0; i < opts1.size(); i++)
	{
		if (opts1.at(i) != opts2.at(i))
			return opts1.at(i) > opts2.at(i);
	}
	return false;
}

QString toConnectionOptions::toString() const
{
	return QString("%1/%2@%3s:schema=%4;color=%5;port=%6;options=???")
			.arg(username)
			.arg(password)
			.arg(database)
			.arg(schema)
			.arg(color)
			.arg(port);
}
