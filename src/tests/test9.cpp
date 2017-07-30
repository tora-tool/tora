
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

#include "tests/test9window.h"

#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconfiguration.h"

#include <QApplication>
#include <QtCore/QLibrary>

int main(int argc, char **argv)
{
    int p = 0;

    toConfiguration::setQSettingsEnv();

    QApplication app(argc, argv);
    QStringList args = app.arguments();

    QLibrary parsing("parsing");
    parsing.load();

    QString sql1, sql2;
    if (args.count() >= 3 && QFile::exists(args[1]) && QFile::exists(args[2]))
    {
        sql1 = Utils::toReadFile(args[1]);
        sql2 = Utils::toReadFile(args[2]);
    }
    else
    {
        QByteArray bytes;
        bool r;
        QFile complexSql(":/old.sql");
        r = complexSql.open(QIODevice::ReadOnly | QIODevice::Text);
        bytes = complexSql.readAll();
        sql1 = QString::fromLatin1(bytes);

        QFile complexNewSql(":/new.sql");
        r = complexNewSql.open(QIODevice::ReadOnly | QIODevice::Text);
        bytes = complexNewSql.readAll();
        sql2 = QString::fromLatin1(bytes);
    }

    new Test9Window(sql1, sql2);
    int ret = qApp->exec();
    return ret;
}

