
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

#undef  TORA_MAKE_DLL
#ifndef TORA_DLL
#define TORA_DLL
#endif
#ifndef TROTL_DLL
#define TROTL_DLL
#endif

#include "core/toconnectionprovider.h"
#include "core/tologger.h"
#include "core/tooracleconst.h"
#include "core/toconfiguration.h"

#include "connection/absfact.h"

#include "connection/tooracledatatype.h"
#include "main/tooraclesetting.h"
#include "connection/tooracleconnection.h"
#include "connection/tooracletraits.h"

#include "trotl.h"
#include "trotl_convertor.h"

#include <QtCore/QSettings>
#include <QtCore/QDir>

class toOracleConnectionImpl;
class toOracleConnectionSub;

class toOracleProvider : public  toConnectionProvider
{
    public:
        toOracleProvider(toConnectionProviderFinder::ConnectionProvirerParams const& p);

        /** see: @ref toConnectionProvider::initialize() */
        bool initialize() override;

        /** see: @ref toConnectionProvider::name() */
        QString const& name() const override
        {
            return m_name;
        };

        QString const& displayName() const override
        {
            return m_display_name;
        };

        /** see: @ref toConnectionProvider::hosts() */
        QList<QString> hosts() const override;

        /** see: @ref toConnectionProvider::defaultConnection() */
        QMap<QString,QString> defaultConnection() const override;

        /** see: @ref toConnectionProvider::databases() */
        QList<QString> databases(const QString &host, const QString &user, const QString &pwd) const override;

        /** see: @ref toConnectionProvider::options() */
        QList<QString> options() const override;

        /** see: @ref toConnectionProvider::configurationTab() */
        QWidget *configurationTab(QWidget *parent) override;

        /** see: @ref toConnection */
        toConnection::connectionImpl* createConnectionImpl(toConnection&) override;

        /** see: @ref toConnection */
        toConnectionTraits* createConnectionTrait(void) override;

    private:
        static QString m_name, m_display_name;
        ::trotl::OciEnv *_envp;
        qint64 toMaxLong;
        QByteArray dateFormat;
};

QString toOracleProvider::m_name = ORACLE_PROVIDER;
QString toOracleProvider::m_display_name = ORACLE_PROVIDER;

toOracleProvider::toOracleProvider(toConnectionProviderFinder::ConnectionProvirerParams const& p)
    : toConnectionProvider(p)
    , _envp(0)
    , toMaxLong(0L)
{
}

bool toOracleProvider::initialize()
{
    try
    {
        QString nls = getenv("NLS_LANG");
        if (nls.isEmpty())
            nls = "AMERICAN_AMERICA.AL32UTF8";
        else
        {
            int pos = nls.lastIndexOf('.');
            if (pos > 0)
                nls = nls.left(pos);
            nls += ".AL32UTF8";
        }
        qputenv("NLS_LANG", nls.toLatin1());

        toMaxLong = toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::MaxLong).toInt();
        toMaxLong = toMaxLong == -1 ? 0x80000000 : toMaxLong;
        ::trotl::g_OCIPL_MAX_LONG = toMaxLong;

        //::trotl::g_OCIPL_BULK_ROWS = toConfigurationSingle::Instance().

        dateFormat = toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::ConfDateFormat).toString().toLatin1();
        ::trotl::g_TROTL_DEFAULT_DATE_FTM = const_cast<char*>(dateFormat.constData());
        ::trotl::OciEnvAlloc *_envallocp = new ::trotl::OciEnvAlloc;

        _envp = new ::trotl::OciEnv(*_envallocp);
        // Catch trotl OCI exception and re-throw toConnection::exception
    }
    catch (const ::trotl::OciException &exc)
    {
        TLOG(5, toNoDecorator, __HERE__)
                << "Error loading Oracle provider: "
                << exc.what()
                << std::endl;
        //ThrowException(exc);
        return false;
    }
    return true;
}

QList<QString> toOracleProvider::hosts() const
{
    return QList<QString>{};
}

QMap<QString,QString> toOracleProvider::defaultConnection() const
{
	QMap<QString,QString> retval;
	retval.insert("HOST", "localhost");
	retval.insert("PORT", "1521");
	retval.insert("DB", "XE");
	retval.insert("USER", "SYSTEM");
	return retval;
	//return QMap<QString,QString>{{"HOST", "localhost"}, {"PORT", "1521"}, {"DB", "XE"}, {"USER", "SYSTEM"}}; Qt >= 5.2 only
}

QList<QString> toOracleProvider::databases(const QString &host, const QString &user, const QString &pwd) const
{
    QSet<QString> tnsnames; // List of locations for the file tnsnames.ora
    QString str;
#ifdef Q_OS_WIN32
    {
        QSettings settings("HKEY_LOCAL_MACHINE\\Software\\ORACLE", QSettings::NativeFormat);
        foreach(QString key, settings.childGroups())
        {
            QString sHome = settings.value(key + '/' + "ORACLE_HOME").toString();
            if ( sHome.isEmpty())
                continue;

            QString sHomeName = settings.value(key + '/' + "ORACLE_HOME_NAME").toString();
            QString version = settings.value(key + '/' + "VERSION").toString();

            QDir dHome(sHome);
            if ( !dHome.exists())
                continue;

            QFileInfo tnsPath(dHome.absolutePath() + "/network/admin/tnsnames.ora");
            if ( !tnsPath.exists() || !tnsPath.isFile() || !tnsPath.isReadable())
                continue;

            TLOG(5, toNoDecorator, __HERE__) << "add tnsnames: " << tnsPath.absoluteFilePath()  << std::endl;
            tnsnames.insert(tnsPath.absoluteFilePath());
        }
    }
#else
    {


        str = QDir::homePath() + QDir::separator() + ".tnsnames.ora";
        QFileInfo home(str);
        if (home.exists() && home.isFile() && home.isReadable())
        {
            TLOG(5, toNoDecorator, __HERE__) << "add tnsnames: " << home.absoluteFilePath()  << std::endl;
            tnsnames.insert(home.absoluteFilePath());
        }


        str = "/etc/tnsnames.ora";
        QFileInfo etc(str);
        if (etc.exists() && etc.isFile() && etc.isReadable())
        {
            TLOG(5, toNoDecorator, __HERE__) << "add tnsnames: " << etc.absoluteFilePath()  << std::endl;
            tnsnames.insert(etc.absoluteFilePath());
        }
    }
#endif

    if (getenv("TNS_ADMIN"))
    {
        str = getenv("TNS_ADMIN");
        str += "/tnsnames.ora";
    }
    else if ( getenv("ORACLE_HOME"))
    {
        str = getenv("ORACLE_HOME");
        str += "/network/admin/tnsnames.ora";
    }
    else
    {
        str.truncate(0);
    }

    QFileInfo ohome_net_adm(str);
    if (!str.isEmpty() && ohome_net_adm.exists() && ohome_net_adm.isFile() && ohome_net_adm.isReadable())
    {
        TLOG(5, toNoDecorator, __HERE__) << "add tnsnames: " << ohome_net_adm.absoluteFilePath()  << std::endl;
        tnsnames.insert(ohome_net_adm.absoluteFilePath());
    }

    QList<QString> ret;

    foreach(QString filename, tnsnames)
    {
        // This block of code parses TNS file and extract all DB's from it
        QFile file(filename);
        int begname = -1;
        int parambeg = -1;
        int pos = 0;
        int param = 0;
        QString tns_file;

        try
        {
            // we read the tns file as a QString this should be safe for all possible encodings this file might have,
            // unlike if we read it as a byte data using QByteArray.
            tns_file = Utils::toReadFile(filename);
            TLOG(0, toDecorator, __HERE__)
                    << "--------------------------------------------------------------------------------" << std::endl
                    << "File read: " << filename << std::endl;
        }
        catch ( QString const &e )
        {
            // If we can't open the file we throw an exception with information why it failed
            TOMessageBox::warning(
                Utils::toQMainWindow(),
                QT_TRANSLATE_NOOP("toReadFile", "File error"),
                QT_TRANSLATE_NOOP("toReadFile", QString("Couldn't open %1 for readonly: %2").arg(filename).arg(e)));
            goto next;
        }

        // Parse the TNS file and get a list of database instances we have in it
        while (pos < tns_file.size())
        {
            if (tns_file[pos] == '#')
            {
                // Comment line can be skipped, move to end of line and continue with a next one
                while (pos < tns_file.size() && tns_file[pos] != '\n')
                    pos++;
            }
            else if (tns_file[pos] == '=')
            {
                // If there is an equal sign we process this line
                // For every parameter in brackets we increment the param variable so that if there is 0 it means we are in lowest level which is
                // name of database in tns file
                if (param == 0 && begname >= 0)
                {
                    QString line = tns_file.mid(begname, pos - begname);
                    // If there is some garbage in name of DB, such as newlines, we probably parsed something we didn't want to,
                    // we should skip this
                    if (line.contains('\n'))
                        TLOG(0, toDecorator, __HERE__) << "Garbage TNS database name skipped: " << line << std::endl;
                    else if (ret.contains(line))
                        TLOG(0, toDecorator, __HERE__) << "Duplicate TNS database name skipped: " << line << std::endl;
                    else if (begname >= 0 && host.isEmpty()) // host IS empty if provider is ORACLETNS_
                        ret.insert(ret.end(), line);
                }
            }
            else if (tns_file[pos] == '(')
            {
                // We entered some subparameter of tns record
                begname = -1;
                parambeg = pos + 1;
                param++;
            }
            else if (tns_file[pos] == ')')
            {
                // We left a subparameter of tns record
                /* 
                //Don't understand this code - we parse tnsnames.ora, we connect via tnsnames, what's the point to have DB SIDs in list?
                if (parambeg >= 0 && host.isEmpty())
                {
                    // If the database has no name but has some SID, we use its SID instead:
                    QString tmp = tns_file.mid(parambeg, pos - parambeg);
                    tmp.replace(QRegExp(QString::fromLatin1("\\s+")), QString());
                    if (tmp.toLower().startsWith(QString::fromLatin1("sid=")))
                        ret.insert(ret.end(), tmp.mid(4));
                }
                */
                begname = -1;
                parambeg = -1;
                param--;
            }
            else if (!tns_file[pos].isSpace() && begname < 0)
            {
                begname = pos;
            }
            pos++;
        }
    next:
        ;;
    }

    // This returns only a list of databases that are in TNS file, with no other information, which is likely somewhat ineffective given that we already processed
    // whole file and have all that information.
    //! \todo The information of all databases should be stored in some structure in memory that contains all stuff that is in TNS file, so that we for example
    //        know the hostname of database, port and so on.
    qSort(ret);
    return ret;
}

QList<QString> toOracleProvider::options() const
{
    QList<QString> ret;
    ret << "*SQL*Net"
        << "-"
        << "SYS_OPER"
        << "SYS_DBA"
        << "SYS_ASM";
    return ret;
}

QWidget* toOracleProvider::configurationTab(QWidget *parent)
{
    return new toOracleSetting(parent);
}

toConnection::connectionImpl* toOracleProvider::createConnectionImpl(toConnection &con)
{
    return new toOracleConnectionImpl(con, *_envp);
}

// TODO return reference into static instance of toOracleTraits
toConnectionTraits* toOracleProvider::createConnectionTrait(void)
{
    static toOracleTraits* t = new toOracleTraits();
    return t;
}

Util::RegisterInFactory<toOracleProvider, ConnectionProvirerFactory> regToOracleInstantProvider(ORACLE_PROVIDER);
