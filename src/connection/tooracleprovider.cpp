/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#undef  TORA_MAKE_DLL
#ifndef TORA_DLL
#define TORA_DLL
#endif
#ifndef TROTL_DLL
#define TROTL_DLL
#endif

#include "core/toconnectionprovider.h"
#include "core/tologger.h"
#include "core/tomainwindow.h"
#include "core/toconfiguration.h"
#include "core/tooracleconst.h"

#include "connection/absfact.h"

#include "connection/tooracledatatype.h"
#include "connection/tooraclesetting.h"
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
    virtual bool initialize();

    /** see: @ref toConnectionProvider::name() */
    virtual QString const& name() const
    {
        return m_name;
    };

    virtual QString const& displayName() const
    {
        return m_display_name;
    };

    /** see: @ref toConnectionProvider::hosts() */
    virtual QList<QString> hosts();

    /** see: @ref toConnectionProvider::databases() */
    virtual QList<QString> databases(const QString &host, const QString &user, const QString &pwd);

    /** see: @ref toConnectionProvider::options() */
    virtual QList<QString> options();

    /** see: @ref toConnectionProvider::configurationTab() */
    virtual QWidget *configurationTab(QWidget *parent);

    /** see: @ref toConnection */
    virtual toConnection::connectionImpl* createConnectionImpl(toConnection&);

    /** see: @ref toConnection */
    virtual toConnectionTraits* createConnectionTrait(void);

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

        toMaxLong = toConfigurationSingle::Instance().maxLong() == -1 ? 0x80000000 : toConfigurationSingle::Instance().maxLong();
        ::trotl::g_OCIPL_MAX_LONG = toMaxLong;

        //::trotl::g_OCIPL_BULK_ROWS = toConfigurationSingle::Instance().

        dateFormat = toConfigurationSingle::Instance().dateFormat().toAscii();
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

QList<QString> toOracleProvider::hosts()
{
    QList<QString> ret = QList<QString>() << QString::null << "SQL*Net";
    return ret;
}

QList<QString> toOracleProvider::databases(const QString &host, const QString &user, const QString &pwd)
{
    QSet<QString> tnsnames; // List of locations for the file tnsnames.ora
    QString str;
#ifdef Q_OS_WIN32
    {
        QSettings settings("HKEY_LOCAL_MACHINE\\Software\\ORACLE", QSettings::NativeFormat);
        foreach(QString key, settings.childGroups())
        {
            QString sHome = settings.value(key + '/' + "ORACLE_HOME").toString();
            if( sHome.isEmpty())
                continue;

            QString sHomeName = settings.value(key + '/' + "ORACLE_HOME_NAME").toString();
            QString version = settings.value(key + '/' + "VERSION").toString();

            QDir dHome(sHome);
            if( !dHome.exists())
                continue;

            QFileInfo tnsPath(dHome.absolutePath() + "/network/admin/tnsnames.ora");
            if( !tnsPath.exists() || !tnsPath.isFile() || !tnsPath.isReadable())
                continue;

            TLOG(5, toNoDecorator, __HERE__) << "add tnsnames: " << tnsPath.absoluteFilePath()  << std::endl;
            tnsnames.insert(tnsPath.absoluteFilePath());
        }
    }
#else
    {


        str = QDir::homePath() + QDir::separator() + ".tnsnames.ora";
        QFileInfo home(str);
        if(home.exists() && home.isFile() && home.isReadable())
        {
            TLOG(5, toNoDecorator, __HERE__) << "add tnsnames: " << home.absoluteFilePath()  << std::endl;
            tnsnames.insert(home.absoluteFilePath());
        }


        str = "/etc/tnsnames.ora";
        QFileInfo etc(str);
        if(etc.exists() && etc.isFile() && etc.isReadable())
        {
            TLOG(5, toNoDecorator, __HERE__) << "add tnsnames: " << etc.absoluteFilePath()  << std::endl;
            tnsnames.insert(etc.absoluteFilePath());
        }
    }
#endif

    if(getenv("TNS_ADMIN"))
    {
        str = getenv("TNS_ADMIN");
        str += "/tnsnames.ora";
    }
    else if( getenv("ORACLE_HOME"))
    {
        str = getenv("ORACLE_HOME");
        str + "/network/admin/tnsnames.ora";
    }
    else
    {
        str.truncate(0);
    }

    QFileInfo ohome_net_adm(str);
    if(!str.isEmpty() && ohome_net_adm.exists() && ohome_net_adm.isFile() && ohome_net_adm.isReadable())
    {
        TLOG(5, toNoDecorator, __HERE__) << "add tnsnames: " << ohome_net_adm.absoluteFilePath()  << std::endl;
        tnsnames.insert(ohome_net_adm.absoluteFilePath());
    }

    QList<QString> ret;

    foreach(QString filename, tnsnames)
    {
        QFile file(filename);
        int begname = -1;
        int parambeg = -1;
        int pos = 0;
        int param = 0;
        QByteArray barray;
        const char *buf;

        try
        {
            barray = Utils::toReadFileB(filename);
            buf = barray.constData();
            TLOG(0, toDecorator, __HERE__)
                    << "--------------------------------------------------------------------------------" << std::endl
                    << "File read: " << filename << std::endl;
        }
        catch( QString const &e )
        {
            TOMessageBox::warning(
                Utils::toQMainWindow(),
                QT_TRANSLATE_NOOP("toReadFileB", "File error"),
                QT_TRANSLATE_NOOP("toReadFileB", QString("Couldn't open %1 for readonly: %2").arg(filename).arg(e)));
            goto next;
        }

        while (pos < barray.size())
        {
            if (buf[pos] == '#')
            {
                while (pos < barray.size() && buf[pos] != '\n')
                    pos++;
            }
            else if (buf[pos] == '=')
            {
                if (param == 0)
                {
                    if (begname >= 0 && !host.isEmpty())
                        ret.insert(ret.end(), QString::fromLatin1(buf + begname, pos - begname));
                }
            }
            else if (buf[pos] == '(')
            {
                begname = -1;
                parambeg = pos + 1;
                param++;
            }
            else if (buf[pos] == ')')
            {
                if (parambeg >= 0 && host.isEmpty())
                {
                    QString tmp = QString::fromLatin1(buf + parambeg, pos - parambeg);
                    tmp.replace(QRegExp(QString::fromLatin1("\\s+")), QString::null);
                    if (tmp.toLower().startsWith(QString::fromLatin1("sid=")))
                        ret.insert(ret.end(), tmp.mid(4));
                }
                begname = -1;
                parambeg = -1;
                param--;
            }
            else if (!isspace(buf[pos]) && begname < 0)
            {
                begname = pos;
            }
            pos++;
        }
next:
        ;;
    } // foreach(QString str, tnsnames)

    qSort(ret);
    return ret;
}

QList<QString> toOracleProvider::options()
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
