
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

#include "tests/test4window.h"

#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconfiguration.h"

#include <QtGui/QApplication>
#include <QtCore/QLibrary>

int main(int argc, char **argv)
{
	int p = 0;
	
	toConfigurationNew::setQSettingsEnv();

	QApplication app(argc, argv);
	QStringList args = app.arguments();
	
	QLibrary parsing("parsing");
	parsing.load();

	QString sql;
	if(args.count() >= 2 && QFile::exists(args[1]))
	{
		sql = Utils::toReadFile(args[1]);
	}
	else
	{
		QFile complexSql(":/complex05.sql");
		bool r = complexSql.open(QIODevice::ReadOnly | QIODevice::Text);
		QByteArray bytes = complexSql.readAll();
		sql = QString(bytes);
	}

	new Test4Window(sql);
	int ret = qApp->exec();
	return ret;
}

