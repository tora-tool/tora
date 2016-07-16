
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

#include "core/toglobalconfiguration.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtCore/QDate>

#include "core/toconf.h"
#include "core/utils.h"

#ifdef TORA_EXPERIMENTAL
#include "core/tomemory.h"
#endif

QVariant ToConfiguration::Global::defaultValue(int option) const
{
    switch (option)
    {
            // Paths
        case CustomSQL:
            {
                QFileInfo torasql(QDir::homePath(), ".torasql");
                return QVariant(torasql.absoluteFilePath());
            }
        case HelpDirectory:
            return QVariant(QString("qrc:/help/toc.html"));
        case DefaultSession:
            {
                QFileInfo toratse(QDir::homePath(), ".tora.tse");
                return QVariant(toratse.absoluteFilePath());
            }
        case CacheDirectory:
            {
                QDir dirname;
#ifdef Q_OS_WIN32
                QString TEMP(qgetenv("TEMP"));
                QFileInfo TEMPInfo(TEMP);
                if (TEMPInfo.isDir() && TEMPInfo.exists())
                    dirname = TEMPInfo.absoluteFilePath();
                else
#endif
                    dirname = QDir::homePath();
                QFileInfo toraCache(dirname, ".tora_cache");
                return QVariant(toraCache.absoluteFilePath());
            }
        case OracleHomeDirectory:
            {
                QString OH(qgetenv("ORACLE_HOME"));
                QFileInfo OHInfo(OH);
                if (OHInfo.isDir() && OHInfo.exists())
                    return QVariant(OHInfo.absoluteFilePath());
                return QVariant(QString(""));
            }
        case MysqlHomeDirectory:
            return QVariant(QString(""));
        case PgsqlHomeDirectory:
            return QVariant(QString(""));
        case GraphvizHomeDirectory:
            {
                QString defaultGvHome;
#if defined(Q_OS_WIN32)
                defaultGvHome = "C:/Program Files/Graphviz 2.28/bin";
#elif defined(Q_OS_WIN64)
                defaultGvHome = "C:/Program Files(x86)/Graphviz 2.28/bin";
#else
                defaultGvHome = "/usr/bin";
#endif
                return QVariant(defaultGvHome);
            }
            // Options: (1st column)
        case ChangeConnectionBool:
            return QVariant((bool)true);
        case SavePasswordBool:
            return QVariant((bool)false);
        case IncludeDbCaptionBool:
            return QVariant((bool)true);
        case RestoreSessionBool:
            return QVariant((bool)false);
        case ToadBindingsBool:
            return QVariant((bool)false);
        case CacheDiskBool:
            return QVariant((bool)true);
        case DisplayGridlinesBool:
            return QVariant((bool)true);
        case MultiLineResultsBool:
            return QVariant((bool)false);
        case MessageStatusbarBool:
            return QVariant((bool)false);
        case ColorizedConnectionsBool:
            return QVariant((bool)true);
        case ColorizedConnectionsMap:
            {
                QMap<QString, QVariant> retval;
                retval["#FF0000"] = QVariant("Production");
                retval["#00FF00"] = QVariant("Development");
                retval["#0000FF"] = QVariant("Testing");
                return retval;
            }
        case UpdateStateInt:        // 0 update is not running, 1 update is running, 2 TOra crashed, do not try again on startup
        	return QVariant((int)0);
        case UpdateLastDate:        // last date update was run
        	return QVariant(QDate());
        case UpdatesCheckBool:
        	return QVariant((bool)true);
            // Options: (2nd column)
        case StatusMessageInt:
            return QVariant((int)5);
        case HistorySizeInt:
            return QVariant((int)10);
        case ChartSamplesInt:
            return QVariant((int)100);
        case DisplaySamplesInt:
            return QVariant((int)-1);
        case SizeUnit:
            return QVariant(QString("MB"));
        case RefreshInterval:
            return QVariant(QString("None"));  // None - 0
        case DefaultListFormatInt:
            return QVariant((int)0);
        case Style:
            return QVariant(Utils::toGetSessionType());
        case Translation:
            return QVariant(QLocale().name());
        case ClipboardCHeadersBool:
            return QVariant((bool)true);
        case ClipboardRHeadersBool:
            return QVariant((bool)false);
#ifdef TORA_EXPERIMENTAL
        case RamThresholdInt:
            //return QVariant((int)getTotalSystemMemory()*2/3);
            return QVariant((int)2000);
#endif
        default:
            Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Global un-registered enum value: %1").arg(option)));
            return QVariant();
    }
};

QVariant ToConfiguration::Global::toraIIValue(int option) const
{
    QSettings s(TOORGNAME, "TOra"); // "old" Tora 2.x config namespace
    QVariant retval;

    switch (option)
    {
            // Paths
        case CustomSQL:
            s.beginGroup("preferences");
            retval = s.value("SQLDictionary");
            break;
            //case HelpDirectory:
            //case DefaultSession:
            //case CacheDirectory:
        case OracleHomeDirectory:
            s.beginGroup("preferences");
            retval = s.value("ORACLE_HOME");
            break;
        case MysqlHomeDirectory:
            s.beginGroup("preferences");
            retval = s.value("MYSQL_HOME");
            break;
        case PgsqlHomeDirectory:
            s.beginGroup("preferences");
            retval = s.value("PGSQL_HOME");
            break;
        case GraphvizHomeDirectory:
            s.beginGroup("preferences");
            retval = s.value("GV_HOME");
            break;
            // Options: (1st column)
            //case ChangeConnectionBool:	return QVariant((bool)true);
        case SavePasswordBool:
            s.beginGroup("preferences");
            retval = s.value("SavePassword");
            break;
            //case IncludeDbCaptionBool:	return QVariant((bool)true);
            //case RestoreSessionBool:	return QVariant((bool)false);
            //case ToadBindingsBool:		return QVariant((bool)false);
            //case CacheDiskBool:		    s.beginGroup("preferences"); retval = s.value("CacheDiskUse"); break;
            //case DisplayGridlinesBool:	return QVariant((bool)true);
            //case MultiLineResultsBool:	return QVariant((bool)false);
            //case MessageStatusbarBool:	return QVariant((bool)false);
            //case ColorizedConnectionsBool:	return QVariant((bool)true);
            //case ColorizedConnectionsMap:
            // Options: (2nd column)
            //case StatusMessageInt:		return QVariant((int)5);
            //case HistorySizeInt:		return QVariant((int)10);
            //case ChartSamplesInt:		return QVariant((int)100);
            //case DisplaySamplesInt:     return QVariant((int)-1);
            //case SizeUnit:			return QVariant(QString("MB"));
            //case RefreshInterval:	return QVariant(QString("None"));  // None - 0
            //case DefaultListFormatInt:		return QVariant((int)0);
            //case Style:			return QVariant(Utils::toGetSessionType());
            //case Translation:		return QVariant(QLocale().name());
        default:
            s.beginGroup("preferences");
            retval = QVariant();
            break;
    }
    s.endGroup();
    return retval;
}

ToConfiguration::Global s_global;
