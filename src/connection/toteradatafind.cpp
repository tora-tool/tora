
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

#include "core/toconnectionprovider.h"
#include "core/tologger.h"
#include "core/utils.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>

#if defined(Q_OS_LINUX)
#   define PROVIDER_LIB "libpteradata.so"
#elif defined(Q_OS_WIN32)
#   define PROVIDER_LIB "pteradata.dll"
#elif defined(Q_WS_MAC)
#   define PROVIDER_LIB "libpteradata.dylib"
#endif

class toTeradataFinder : public  toConnectionProviderFinder
{
    public:
        inline toTeradataFinder(unsigned int i) : toConnectionProviderFinder(i) {};

        virtual QString name() const
        {
            return QString::fromLatin1("Teradata");
        };

        /** Return list of possible client locations
         */
        virtual QList<ConnectionProvirerParams> find();

        /**
           Load connection providers library
        */
        virtual void load(ConnectionProvirerParams const&);

    protected:
        static QList<QString> const m_libname;
        static bool m_CLIv2Loaded;

        void check(ConnectionProvirerParams const&);
        void setEnv(ConnectionProvirerParams const&);
        void loadLib(ConnectionProvirerParams const&);

    private:
        static QSet<QString> const m_paths;
};

QSet<QString> const toTeradataFinder::m_paths = QSet<QString>()
#if defined(Q_OS_LINUX)
        << QString::fromLatin1("/opt/teradata/client/14.00/lib64/")
        << QString::fromLatin1("/opt/teradata/client/14.00/lib/")
        << QString::fromLatin1("/opt/teradata/client/13.00/lib64/")
        << QString::fromLatin1("/opt/teradata/client/13.00/lib/")
#elif  defined(Q_OS_WIN32)
#endif
        ;

QList<QString> const toTeradataFinder::m_libname = QList<QString>()
#if defined(Q_OS_LINUX)
        << QString::fromLatin1("libcliv2.so")
#elif defined(Q_OS_WIN32)
        << QString::fromLatin1("wincli32.dll")
#elif defined(Q_OS_WIN64)
        << QString::fromLatin1("wincli64.dll")
#endif
        ;

bool toTeradataFinder::m_CLIv2Loaded = false;

QList<toConnectionProviderFinder::ConnectionProvirerParams>  toTeradataFinder::find()
{
    QList<ConnectionProvirerParams> retval;
    QSet<QString> possibleRoots;
    ConnectionProvirerParams tRoot;

    do
    {
        QString sRoot = getenv("COPLIB");
        if ( sRoot.isEmpty())
            continue;
        QDir dRoot(sRoot);
        if ( !dRoot.exists() || !dRoot.isReadable())
            continue;
        if ( possibleRoots.contains(dRoot.absolutePath()))
            continue;
        TLOG(5, toNoDecorator, __HERE__) << "searching: " << dRoot.absolutePath()  << std::endl;
        possibleRoots.insert(dRoot.absolutePath());
    }
    while (false);

    foreach(QString sLibPath, m_paths)
    {
        QDir dLibPath(sLibPath);
        if ( !dLibPath.exists() || !dLibPath.isReadable())
            continue;
        TLOG(5, toNoDecorator, __HERE__) << "searching: " << dLibPath.absolutePath()  << std::endl;
        possibleRoots.insert(dLibPath.absolutePath());
    }

#ifdef Q_OS_WIN32
    QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Teradata\\Client", QSettings::NativeFormat);
    foreach(QString key, settings.childGroups())
    {
        QString sRoot = settings.value(key + "/Teradata CLIv2/InstallDLLsPath").toString();
        if ( sRoot.isEmpty())
            continue;

        QString version = settings.value(key + "/Teradata CLIv2/Version").toString();

        QDir dRoot(sRoot);
        if ( !dRoot.exists() || !dRoot.isReadable())
            continue;

        if (! possibleRoots.contains(dRoot.absolutePath()))
            possibleRoots.insert(dRoot.absolutePath());
    }
#endif

    foreach( QString sRoot, possibleRoots)
    {
        QDir dRoot(sRoot);
        QStringList sLibraries = dRoot.entryList( m_libname);

        foreach( QString sLibrary, sLibraries)
        {
            QString sLibPath = QDir::toNativeSeparators(dRoot.canonicalPath()) + QDir::separator() + sLibrary;

            if (!Utils::toLibrary::isValidLibrary(sLibPath))
            {
                TLOG(5, toNoDecorator, __HERE__) << "skipping: " << sLibPath << std::endl;
                continue;
            }

            TLOG(5, toNoDecorator, __HERE__) << "adding: " << sLibPath << std::endl;
            tRoot.insert("PATH", QDir::toNativeSeparators(dRoot.canonicalPath()));
            tRoot.insert("TERADATA_ROOT", QDir::toNativeSeparators(dRoot.canonicalPath()));
            tRoot.insert("LIBPATH", sLibPath);
            tRoot.insert("KEY", name());
            tRoot.insert("PROVIDER", "Teradata");
            retval.append(tRoot);
            tRoot.clear();
        }
    }

    return retval;
}

void toTeradataFinder::check(ConnectionProvirerParams const &params)
{
    if ( params.value("KEY") != name())
        throw QString("toTeradataFinder::load - invalid key");

    if ( m_CLIv2Loaded == true)
        throw QString("toTeradataFinder::load - Teradata client libs are already loaded");
}

void toTeradataFinder::setEnv(ConnectionProvirerParams const &params)
{
    QString sRoot = params.value("TERADATA_ROOT").toString();
    //Utils::toSetEnv("COPLIB", sRoot);
    //TLOG(5, toNoDecorator, __HERE__) << "export COPLIB=" << sRoot << std::endl;
}

void toTeradataFinder::loadLib(ConnectionProvirerParams const &params)
{
    QFileInfo libPath(params.value("LIBPATH").toString());

    TLOG(5, toNoDecorator, __HERE__) << "Loading:" << libPath.absoluteFilePath() << std::endl;
    Utils::toLibrary::LHandle hmoduleCLIv2 = Utils::toLibrary::loadLibrary(libPath);
    if ( hmoduleCLIv2)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " PROVIDER_LIB << std::endl;
#ifdef Q_OS_WIN32
    Utils::toLibrary::LHandle hmodulePTeradata = Utils::toLibrary::loadLibrary(QFileInfo(PROVIDER_LIB));
#else
    Utils::toLibrary::LHandle hmodulePTeradata = Utils::toLibrary::loadLibrary(QFileInfo(QDir(PROVIDERS_PATH), PROVIDER_LIB));
#endif
    if ( hmodulePTeradata)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;
}

void toTeradataFinder::load(ConnectionProvirerParams const &params)
{
    check(params);
    setEnv(params);
    loadLib(params);
    m_CLIv2Loaded = true;
}

Util::RegisterInFactory<toTeradataFinder, ConnectionProviderFinderFactory> regToTeradataFind("Teradata");
