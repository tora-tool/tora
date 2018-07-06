
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

//#include "core/utils.h"
#include "parsing/tsqlparse.h"
#include "parsing/tsqllexer.h"
#include <QtCore/QDateTime>
#include <QApplication>
#include <QtCore/QTextCodec>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtCore/QLibrary>
#include <QtCore/QDebug>

#include <memory>
#include <iostream>
#include "core/utils.h"
#include "core/tologger.h"
#include "parsing/toindent.h"

void indentPriv(SQLParser::Token const* root, QList<SQLParser::Token const*> &list);

int main(int argc, char **argv)
{
    using namespace SQLParser;

    QString filename;
    if ( argc == 2)
    {
        filename = QString::fromLatin1(argv[1]);
    } else {
        return 0;
    }

    QApplication app(argc, argv);
    try
    {
        QString input = Utils::toReadFile(filename);
        QString output = toIndent::indent(input);
    }
    catch (const ParseException &e)
    {
        std::cerr << "ParseException: "<< std::endl << std::endl;
    }
    catch (const QString &str)
    {
        std::cerr << "Unhandled exception: "<< std::endl << std::endl << qPrintable(str) << std::endl;
    }
    return 0;
}
