
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

#include "widgets/toabout.h"
#include "core/tocache.h"
#include "core/toconf.h"
#include "core/toconfiguration.h"
#include "core/todatabaseconfig.h"
#include "core/toglobalconfiguration.h"
#include "core/toconnection.h"
#include "core/toconnection.h"
#include "core/toconnectionprovider.h"
#include "core/toconnectionsubloan.h"
#include "core/tologger.h"
#include "core/toquery.h"
#include "core/toqvalue.h"
#include "core/toraversion.h"
#include "widgets/tosplash.h"
#include "core/tosql.h"
#include "core/utils.h"
#include "editor/tomarkededitor.h"
#include "core/tooracleconst.h"

#include <QtCore/QDateTime>
#include <QApplication>
#include <QMessageBox>
#include <QtCore/QString>
#include <QtCore/QTranslator>
#include <QStyleFactory>
#include <QtCore/QLibrary>

#include <memory>

static void usage()
{
    printf("Usage:\n\n  test2 connectstring [dbschema]\n\n");
    exit(2);
}

int main(int argc, char **argv)
{
    toConfigurationNew::setQSettingsEnv();

    /*! \warning: Keep the code before QApplication init as small
      as possible. There could be serious display issues when
      you construct some Qt classes before QApplication.
      It's the same for global static stuff - some instances can
      break it (e.g. qscintilla lexers etc.).
    */
    QApplication app(argc, argv);

    QString style(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::Style).toString());
    if (!style.isEmpty())
        QApplication::setStyle(QStyleFactory::create(style));

    try
    {
        toQValue::setNumberFormat(
            toConfigurationNewSingle::Instance().option(ToConfiguration::Database::NumberFormatInt).toInt(),
            toConfigurationNewSingle::Instance().option(ToConfiguration::Database::NumberDecimalsInt).toInt()
        );

        qRegisterMetaType<toQColumnDescriptionList>("toQColumnDescriptionList&");
        qRegisterMetaType<ValuesList>("ValuesList&");
        qRegisterMetaType<toConnection::exception>("toConnection::exception");

        if (argc == 1)
            usage();

        QString connect = QString::fromLatin1(argv[1]);
        QString user, password, database, schema, table;

        QStringList slashList, atList = connect.split("@", QString::SkipEmptyParts);
        if ( atList.size() == 1)
            database = QString::fromLatin1(qgetenv("ORACLE_SID"));
        if ( atList.size() > 2)
            usage();

        if ( atList.at(0).contains("/"))
        {
            slashList = atList.at(0).split("/", QString::SkipEmptyParts);
            user = slashList.at(0);
            password = slashList.at(1);
            database = atList.at(1);
        }
        else
        {
            slashList = atList.at(1).split("/", QString::SkipEmptyParts);
            user = atList.at(0);
            database = slashList.at(0);
            password = slashList.at(1);
        }
        if ( argc == 2)
        {
            schema = user;
        }
        else
        {
            schema = QString::fromLatin1(argv[2]);
        }

        if ( argc == 4)
        {
            table = QString::fromLatin1(argv[3]) ;
        }
        else
        {
            table = QString::fromLatin1("T_CHILD6");
        }

        // List of all connection provider finders
        std::vector<std::string> finders = ConnectionProviderFinderFactory::Instance().keys();
        // Resulting list of all the providers found
        QList<toConnectionProviderFinder::ConnectionProvirerParams> allProviders;
        // Loop over all finders and all ther find method each of them can return more locations
        for (std::vector<std::string>::const_iterator i = finders.begin(); i != finders.end(); ++i)
        {
            TLOG(5, toDecorator, __HERE__) << "Looking for client: " << *i << std::endl;
            std::auto_ptr<toConnectionProviderFinder> finder = ConnectionProviderFinderFactory::Instance().create(*i, 0);
            QList<toConnectionProviderFinder::ConnectionProvirerParams> l = finder->find();
            allProviders.append(l);
        }
	QDir oHome = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::OracleHomeDirectory).toString();
        foreach(toConnectionProviderFinder::ConnectionProvirerParams const& params, allProviders)
        {
	    if ( params.value("PROVIDER").toString() != ORACLE_PROVIDER)
	      continue;
	    QDir pHome(params.value("ORACLE_HOME").toString());
	    if (oHome != pHome)
	      continue;
	    QString providerName = params.value("PROVIDER").toString();
            if (providerName == "Oracle")
            {
                toConnectionProviderRegistrySing::Instance().load(params);
                break;
            }
        }
        foreach(toConnectionProviderFinder::ConnectionProvirerParams const& params, allProviders)
        {
            QString providerName = params.value("PROVIDER").toString();
	    if (params.value("PROVIDER").toString() != ORACLE_PROVIDER)
	      continue;
	    if (params.value("IS INSTANT").toBool() == true)
	      continue;
	    if (toConnectionProviderRegistrySing::Instance().providers().contains(providerName))
	      continue;	    
            if (providerName == "Oracle")
            {
                toConnectionProviderRegistrySing::Instance().load(params);
                break;
            }
        }

        QSet<QString> options;
        QPointer<toConnection> oraCon = new toConnection(
            QString("Oracle"),
            user,
            password,
            "",
            database,
            "",
            "",
            options);
        TLOG(0,toDecorator,__HERE__) << "Version: " << oraCon->version() << std::endl;

        // uncomment this if you want to run cache refresh in synchronous mode
        // i.e. run it in the current main thread
        // toCache::cacheObjects co(*oraCon);
        // co.run();

        // uncomment this if you want to run cache refresh in async mode
        // i.e. run it the background thread
        //oraCon->getCache().readCache();  // rereadCache()
        //oraCon->getCache().wait4BGThread(); // wait for bg thread to finish.

        QString ConstrainsSQL = QString::fromLatin1(
                                    " SELECT                                                 \n"
                                    "       c.constraint_name                                \n" // c1
                                    "       -- max(a.constraint_name) as constraint_name     \n"
                                    "       -- , c.constraint_name                           \n"
                                    "       , max(r.constraint_name) as r_constraint_name    \n" // c2
                                    "       , max(c.owner)           as owner                \n" // c3
                                    "       , max(c.table_name)      as table_name           \n" // c4
                                    "       , c.column_name          as column_name          \n" // c5
                                    "       , max(r.owner)           as r_owner              \n" // c6
                                    "       , max(r.table_name)      as r_table_name         \n" // c7
                                    "       , max(r.column_name)     as r_column_name        \n" // c8
                                    "       , max(a.constraint_type)                         \n" // c9
                                    " FROM sys.all_constraints a                             \n"
                                    " JOIN sys.all_cons_columns c ON (c.constraint_name = a.constraint_name AND c.owner = a.owner)                                 \n"
                                    " JOIN sys.all_cons_columns r ON (r.constraint_name = a.r_constraint_name AND r.owner = a.r_owner AND r.position = c.position) \n"
                                    " WHERE                                                  \n"
                                    "          a.owner =                   :f1<char[101]>    \n"
                                    "      AND a.table_name =              :f2<char[101]>    \n"
                                    "      AND a.constraint_type = 'R'                       \n"
                                    " GROUP BY ROLLUP (c.constraint_name, c.column_name)     \n"
                                );

        QString ReferencesSQL = QString::fromLatin1(
                                    " SELECT                                                 \n"
                                    "       c.constraint_name                                \n" // c1
                                    "       -- max(a.constraint_name) as constraint_name     \n"
                                    "       -- , c.constraint_name                           \n"
                                    "       , max(r.constraint_name) as r_constraint_name    \n" // c2
                                    "       , max(c.owner)           as owner                \n" // c3
                                    "       , max(c.table_name)      as table_name           \n" // c4
                                    "       , c.column_name          as column_name          \n" // c5
                                    "       , max(r.owner)           as r_owner              \n" // c6
                                    "       , max(r.table_name)      as r_table_name         \n" // c7
                                    "       , max(r.column_name)     as r_column_name        \n" // c8
                                    "       , max(a.constraint_type)                         \n" // c9
                                    " FROM sys.all_constraints a                             \n"
                                    " JOIN sys.all_cons_columns c ON (c.constraint_name = a.constraint_name AND c.owner = a.owner)                                 \n"
                                    " JOIN sys.all_cons_columns r ON (r.constraint_name = a.r_constraint_name AND r.owner = a.r_owner AND r.position = c.position) \n"
                                    " WHERE                                                  \n"
                                    "          a.r_owner =                   :f1<char[101]>  \n"
                                    "      AND r.table_name =                :f2<char[101]>  \n"
                                    "      AND a.constraint_type = 'R'                       \n"
                                    " GROUP BY ROLLUP (c.constraint_name, c.column_name)     \n"
                                );

        //QList<toCache::CacheEntry const*> tables = oraCon->getCache().getEntriesInSchema(schema, toCache::TABLE);
        //Q_FOREACH(toCache::CacheEntry const*e, tables)
        {
            toConnectionSubLoan conn(*oraCon);
            toQValue c1, c2, c3, c4, c5, c6, c7, c8, c9;
            //toCacheEntryTable const* f = static_cast<toCacheEntryTable const*>(e);
            //oraCon->getCache().describeEntry(f);
            toQuery QueryC(conn, ConstrainsSQL, toQueryParams() << schema.toUpper() << table.toUpper());
            while (!QueryC.eof())
            {
                c1 = QueryC.readValue();
                c2 = QueryC.readValue();
                c3 = QueryC.readValue();
                c4 = QueryC.readValue();
                c5 = QueryC.readValue();
                c6 = QueryC.readValue();
                c7 = QueryC.readValue();
                c8 = QueryC.readValue();
                c9 = QueryC.readValue();

                if ( c5.isNull() && c1.isNull())
                {
                    // Here collect FK?details (column list for compound keys)
                }

                if ( c5.isNull() && !c1.isNull())
                {
                    TLOG(0, toNoDecorator, __HERE__)
                            << c3.displayData() << '.' << c4.displayData()
                            << " => "
                            << c6.displayData() << '.' << c7.displayData()
                            << std::endl;
                }
            }

            toQuery QueryR(conn, ReferencesSQL, toQueryParams() << schema.toUpper() << table.toUpper());
            while (!QueryR.eof())
            {
                c1 = QueryR.readValue();
                c2 = QueryR.readValue();
                c3 = QueryR.readValue();
                c4 = QueryR.readValue();
                c5 = QueryR.readValue();
                c6 = QueryR.readValue();
                c7 = QueryR.readValue();
                c8 = QueryR.readValue();
                c9 = QueryR.readValue();

                if ( c5.isNull() && !c1.isNull())
                {
                    TLOG(0, toNoDecorator, __HERE__)
                            << c3.displayData() << '.' << c4.displayData()
                            << " => "
                            << c6.displayData() << '.' << c7.displayData()
                            << std::endl;
                }
            }

        }

        delete oraCon;
    }
    catch (const QString &str)
    {
        std::cerr << "Unhandled exception:"<< std::endl << std::endl << qPrintable(str) << std::endl;
        TOMessageBox::critical(NULL,
                               qApp->translate("main", "Unhandled exception"),
                               str,
                               qApp->translate("main", "Exit"));
    }
    return 1;
}
