
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
#include "core/tooracleconst.h"
#include "core/toconfiguration_new.h"
#include "core/toglobalsetting.h"
#include "core/toconf.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>

#include <oci.h>
#if defined(Q_OS_LINUX) || defined (Q_OS_MAC)
#include <unistd.h>
#endif

#if defined(Q_OS_LINUX)
#   define PROVIDER_LIB "libporacle.so"
#   define TROTL_LIB    "libtrotl.so"
#elif defined(Q_OS_WIN32)
#   define PROVIDER_LIB "poracle.dll"
#   define TROTL_LIB    "trotl.dll"
#elif defined(Q_WS_MAC)
#   define PROVIDER_LIB "libporacle.dylib"
#   define TROTL_LIB    "libtrotl.dylib"
#endif

class toOracleInstantFinder : public  toConnectionProviderFinder
{
public:
    inline toOracleInstantFinder(unsigned int i) : toConnectionProviderFinder(i) {};

    virtual QString name() const
    {
        return QString::fromAscii(ORACLE_INSTANTCLIENT);
    };

    /** Return list of possible client locations
     */
    virtual QList<ConnectionProvirerParams> find();

    /**
     * Load connection providers library
	 */
    virtual void load(ConnectionProvirerParams const&);

protected:
    static QList<QString> const m_libname;
    static bool m_OCILoaded;

    static QString clientVersion(QFileInfo path);

    void check(ConnectionProvirerParams const&);
    void setEnv(ConnectionProvirerParams const&);
    void loadLib(ConnectionProvirerParams const&);

private:
    static QSet<QString> const m_paths;
};

QSet<QString> const toOracleInstantFinder::m_paths = QSet<QString>()
#if defined(Q_OS_LINUX)
        << QString::fromAscii("/usr/lib/oracle/11.2/client64/lib")
        << QString::fromAscii("/usr/lib/oracle/11.2/client/lib")
        << QString::fromAscii("/usr/lib/oracle/11.1/client64/lib")
        << QString::fromAscii("/usr/lib/oracle/11.1/client/lib")
        << QString::fromAscii("/usr/lib/oracle/10.2.0.5/client64/lib")
        << QString::fromAscii("/usr/lib/oracle/10.2.0.5/client/lib")
        << QString::fromAscii("/usr/lib/oracle/10.2.0.4/client64/lib")
        << QString::fromAscii("/usr/lib/oracle/10.2.0.4/client/lib")
        << QString::fromAscii("/usr/lib/oracle/10.2.0.3/client64/lib")
        << QString::fromAscii("/usr/lib/oracle/10.2.0.3/client/lib")
        << QString::fromAscii("/opt/instantclient*")
        << QString::fromAscii("/usr/lib")
#elif  defined(Q_OS_WIN32)
        << QString::fromAscii("C:\\instantclient*")
        << QString::fromAscii("D:\\instantclient*")
        << QString::fromAscii("E:\\instantclient*")
        << QString::fromAscii("C:\\oracle\\instantclient*")
        << QString::fromAscii("D:\\oracle\\instantclient*")
        << QString::fromAscii("E:\\oracle\\instantclient*")
        << QString::fromAscii("D:\\devel\\instantclient*")
#endif
		<< QDir::currentPath() 
        ;

QList<QString> const toOracleInstantFinder::m_libname = QList<QString>()
#if defined(Q_OS_LINUX)
        << QString::fromAscii("libclntsh.so*")
#elif defined(Q_WS_MAC)
        << QString::fromAscii("libclntsh.*dylib")
#elif defined(Q_OS_WIN32) // Note both 32 and 64 bit build
        << QString::fromAscii("OCI.dll")
#endif
        ;

bool toOracleInstantFinder::m_OCILoaded = false;

QString toOracleInstantFinder::clientVersion(QFileInfo path)
{
    QFileInfo ocilib(path);
    typedef sword (*tpOCIClientVersion) ( sword*, sword *, sword *, sword *, sword*);
    tpOCIClientVersion pOCIClientVersion = 0;
    sword ret, major, minor, update, patch, port;
    QString retval;
    QTextStream s(&retval);

    Utils::toLibrary::LHandle hmodule = Utils::toLibrary::loadLibrary(path); // TODO this one thows, the others don't
    if( !hmodule)
        return QString::null;

    pOCIClientVersion = (tpOCIClientVersion)  Utils::toLibrary::lookupSymbol( hmodule, "OCIClientVersion");
    if( !pOCIClientVersion)
    {
        retval = "pre-10g client";
        goto EXIT;
    }

    ret = pOCIClientVersion(&major, &minor, &update, &patch, &port);
    if( ret == OCI_ERROR)
    {
        retval = "error calling OCIClientVersion";
        goto EXIT;
    }

    s << major << '.' << minor << '.' << update << '.' << patch << '.' << port;
EXIT:
    bool success = Utils::toLibrary::unLoadLibrary(path, hmodule);
    return retval;
};

QList<toConnectionProviderFinder::ConnectionProvirerParams>  toOracleInstantFinder::find()
{
	using namespace ToConfiguration;

    QList<ConnectionProvirerParams> retval;
    QSet<QString> possibleOracleHomes;
    ConnectionProvirerParams ohome;

    do
    {
    	QString cHome = toConfigurationNewSingle::Instance().option(Global::OracleHomeDirectory).toString();
    	if( cHome.isEmpty())
    		continue;
    	QDir dHome(cHome);
    	if( !dHome.exists())
    		continue;
    	if( possibleOracleHomes.contains(dHome.absolutePath()))
    		continue;
    	TLOG(5, toNoDecorator, __HERE__) << "searching: " << dHome.absolutePath()  << std::endl;
    	possibleOracleHomes.insert(dHome.absolutePath());
    }
    while(false);

    do
    {
        QString sHome = getenv("ORACLE_HOME");
        if( sHome.isEmpty())
            continue;
        QDir dHome(sHome);
        if( !dHome.exists())
            continue;
        if( possibleOracleHomes.contains(dHome.absolutePath()))
            continue;
        TLOG(5, toNoDecorator, __HERE__) << "searching: " << dHome.absolutePath()  << std::endl;
        possibleOracleHomes.insert(dHome.absolutePath());
    }
    while(false);

    {
        QStringList slEnvPaths;
#if defined(Q_OS_WIN32)
        QStringList slPath = QString(getenv("PATH")).split(';', QString::SkipEmptyParts );
        slEnvPaths.append(slPath);
#elif defined(Q_OS_LINUX)
        QStringList slLDPath = QString(getenv("LD_LIBRARY_PATH")).split(';', QString::SkipEmptyParts );
        slEnvPaths.append(slLDPath);
        QStringList slLDPath32 = QString(getenv("LD_LIBRARY_PATH32")).split(';', QString::SkipEmptyParts );
        slEnvPaths.append(slLDPath32);
        QStringList slLDPath64 = QString(getenv("LD_LIBRARY_PATH64")).split(';', QString::SkipEmptyParts );
        slEnvPaths.append(slLDPath64);
#endif
        foreach(QString sHome, slEnvPaths)
        {
            if( sHome.isEmpty())
                continue;
            QDir dHome(sHome);
            if( !dHome.exists())
                continue;
            if( possibleOracleHomes.contains(dHome.absolutePath()))
                continue;
            /* we not want Oracle instalation to be listed between instant clients */
#if defined(Q_OS_WIN32)
            if( dHome.exists("sqlldr.exe"))
                continue;
#endif
            TLOG(5, toNoDecorator, __HERE__) << "searching: " << dHome.absolutePath()  << std::endl;
            possibleOracleHomes.insert(dHome.absolutePath());
        }
    }

    // populate the list of the possible oracle homes
    foreach( QString p, m_paths)
    {
        /** special case, path contains a wildcard
         */
        if( p.contains("*"))
        {
            /** chdir into parent dir */
            QString sParentPath = p.left( p.lastIndexOf(QDir::separator()));
            QDir dParentPath(sParentPath);
            if( !dParentPath.exists())
                continue;

            QStringList filter = QStringList() << p.mid( p.lastIndexOf(QDir::separator()) + 1);
            QStringList sSubdirPaths = dParentPath.entryList( filter);
            foreach( QString s, sSubdirPaths)
            {
                QDir dSubDirPath(dParentPath);
                dSubDirPath.cd(s);
                dSubDirPath.setFilter(QDir::Files);
                if( !dSubDirPath.exists() && !dSubDirPath.isReadable())
                    continue;
                TLOG(5, toNoDecorator, __HERE__) << "searching: " << dSubDirPath.absolutePath()  << std::endl;
                possibleOracleHomes.insert(dSubDirPath.absolutePath());
            }

        }
        else     // dir path not contains wildcard
        {
            QDir dLibPath(p);
            if( !dLibPath.exists() && !dLibPath.isReadable())
                continue;
            TLOG(5, toNoDecorator, __HERE__) << "searching: " << dLibPath.absolutePath()  << std::endl;
            possibleOracleHomes.insert(dLibPath.absolutePath());
        }
    }

    foreach( QString s, possibleOracleHomes)
    {
        QDir d(s);
        QStringList sLibraries = d.entryList( m_libname);
        QString version;
        foreach( QString sLibrary, sLibraries)
        {
            QString sLibPath = QDir::toNativeSeparators(d.canonicalPath()) + QDir::separator() + sLibrary;

            if(!Utils::toLibrary::isValidLibrary(sLibPath))
            {
                TLOG(5, toNoDecorator, __HERE__) << "skipping: " << sLibPath << std::endl;
                continue;
            }

            //version = clientVersion(sLibPath);
            TLOG(5, toNoDecorator, __HERE__) << "adding: " << sLibPath << ':' << version << std::endl;
            ohome.insert("PATH", QDir::toNativeSeparators(d.canonicalPath()));
            ohome.insert("ORACLE_HOME", QDir::toNativeSeparators(d.canonicalPath()));
            ohome.insert("LIBPATH", sLibPath);
            ohome.insert("VERSION", version);
            ohome.insert("KEY", name());
            ohome.insert("IS INSTANT", true);
            ohome.insert("PROVIDER", ORACLE_PROVIDER);
            retval.append(ohome);
            ohome.clear();
        }
    }

    return retval;
}

void toOracleInstantFinder::check(ConnectionProvirerParams const &params)
{
    if( params.value("KEY") != name())
        throw QString("toOracleInstantFinder::load - invalid key");

    if( m_OCILoaded == true)
        throw QString("toOracleInstantFinder::load - Oracle client libs are already loaded");
}

void toOracleInstantFinder::setEnv(ConnectionProvirerParams const &params)
{
    QString ohome = params.value("ORACLE_HOME").toString();
	qputenv("ORACLE_HOME", ohome.toLocal8Bit());
    TLOG(5, toNoDecorator, __HERE__) << "export ORACLE_HOME=" << ohome << std::endl;

    qputenv("NLS_LANG", "AMERICAN_AMERICA.AL32UTF8"); // TODO hardcoded ATM
    TLOG(5, toNoDecorator, __HERE__) << "export NLS_LANG=AMERICAN_AMERICA.AL32UTF8" << std::endl;
}

void toOracleInstantFinder::loadLib(ConnectionProvirerParams const &params)
{
    QFileInfo libPath(params.value("LIBPATH").toString());
#ifdef Q_OS_WIN32
    TLOG(5, toNoDecorator, __HERE__) << "Loading:" << libPath.absoluteFilePath() << std::endl;
    Utils::toLibrary::LHandle hmoduleOCI = Utils::toLibrary::loadLibrary(libPath);
    if ( hmoduleOCI)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " TROTL_LIB  << std::endl;
    Utils::toLibrary::LHandle hmoduleTrotl = Utils::toLibrary::loadLibrary(QFileInfo(TROTL_LIB));
    if ( hmoduleTrotl)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " PROVIDER_LIB << std::endl;
    Utils::toLibrary::LHandle hmodulePOracle = Utils::toLibrary::loadLibrary(QFileInfo(PROVIDER_LIB));
    if ( hmodulePOracle)
    	TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;
    else
    	TLOG(5, toNoDecorator, __HERE__) << "Failed" << std::endl;

#else
    /* Steps to load libclntsh.so on Linux
    All these approaches fail:
    - setenv("LD_LIBRARY_PATH", ..);
      does not work. glibc preserves copy of this variable and ignores any further changes
    - dlopen("/opt/instantclient_11_1/libclntsh.so.11.1");
      fails. libclntsh.so.11.1 depends on libnnz.so
    - dlopen("/opt/instantclient_11_1/libnns.so");
      fails. libnnz.so depends on libclntsh.so.11.1. There is cyclic dependency between these libraries.
    - chdir("/opt/instantclient_11_1");
      dlopen("/opt/instantclient_11_1/libclntsh.so.11.1");
      This succeeds, but further call to OCIEnvInit fails with: ORA-01804.
      For some courious reason Oracle client thinks, that it is a thick one and searches for "rdbms" subdir.

    This approach works correctly:
    - libtrotl.so has compiled in this library search path: "$ORIGIN/instantclient:$$ORIGIN/instantclient".
    - libtrotl.so has dependency on libclntsh.so.11.1.
    - ln -sf /opt/instantclient_11_1 ./instantclient
    - dlopen("libtrotl.so")
    */
    int retval;
    QDir toraHome(QDir::homePath() + QDir::separator() + QChar('.') + QString::fromAscii(TOAPPNAME));
    QString providerPath(toraHome.absolutePath() + QDir::separator() + QString::fromAscii("poracle"));
    TLOG(5, toNoDecorator, __HERE__) << "Creating plugin path:" << providerPath << std::endl;
    if( !toraHome.mkpath(providerPath))
	    throw QString("Could not create: %1").arg(providerPath);

    if( !QDir::setCurrent(providerPath))
	    throw QString("Could change cwd: %1").arg(toraHome.absolutePath());
    
    TLOG(5, toNoDecorator, __HERE__) << "Re-creating symlink:" << libPath.absolutePath() << std::endl;
    retval = unlink("instantclient");
    if(retval)
        TLOG(5, toNoDecorator, __HERE__) << strerror(retval) << std::endl;

    retval = symlink(libPath.absolutePath().toStdString().c_str(), "instantclient");
    if(retval)
        TLOG(5, toNoDecorator, __HERE__) << strerror(retval) << std::endl;

    retval = unlink(TROTL_LIB);
    retval = symlink(PROVIDERS_PATH "/../" TROTL_LIB, TROTL_LIB);
    if(retval)
	    TLOG(5, toNoDecorator, __HERE__) << strerror(retval) << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " TROTL_LIB  << std::endl;
    Utils::toLibrary::LHandle hmoduleTrotl = Utils::toLibrary::loadLibrary(QFileInfo(TROTL_LIB));
    if ( hmoduleTrotl)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " PROVIDER_LIB << std::endl;
    Utils::toLibrary::LHandle hmodulePOracle = Utils::toLibrary::loadLibrary(QFileInfo(QDir(PROVIDERS_PATH), PROVIDER_LIB));
    if ( hmodulePOracle)
    	TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;
    else
    	TLOG(5, toNoDecorator, __HERE__) << "Failed" << std::endl;

    if( !QDir::setCurrent(toraHome.absolutePath()))
    	throw QString("Could change cwd: %1").arg(toraHome.absolutePath());

#endif
}

void toOracleInstantFinder::load(ConnectionProvirerParams const &params)
{
    check(params);
    setEnv(params);
    loadLib(params);
    m_OCILoaded = true;
}

Util::RegisterInFactory<toOracleInstantFinder, ConnectionProviderFinderFactory> regToOracleInstantFind(ORACLE_INSTANTCLIENT);


class toOracleFinder : public  toOracleInstantFinder
{
public:
    inline toOracleFinder(unsigned int i) : toOracleInstantFinder(i) {};

    virtual QString name() const
    {
        return QString::fromAscii(ORACLE_TNSCLIENT);
    };

    /** Return list of possible client locations
     */
    virtual QList<ConnectionProvirerParams> find();

    /**
       Load connection providers library
    */
    virtual void load(ConnectionProvirerParams const&);
    void loadLib(ConnectionProvirerParams const &params);
};

QList<toConnectionProviderFinder::ConnectionProvirerParams> toOracleFinder::find()
{
	using namespace ToConfiguration;

    QList<ConnectionProvirerParams> retval;
    QSet<QString> possibleOracleHomes;
    ConnectionProvirerParams ohome;

    do
    {
    	QString cHome = toConfigurationNewSingle::Instance().option(Global::OracleHomeDirectory).toString();
    	if( cHome.isEmpty())
    		continue;
    	QDir dHome(cHome);
    	if( !dHome.exists())
    		continue;
    	if( possibleOracleHomes.contains(dHome.absolutePath()))
    		continue;
    	TLOG(5, toNoDecorator, __HERE__) << "searching: " << dHome.absolutePath()  << std::endl;
    	possibleOracleHomes.insert(dHome.absolutePath());
    }
    while(false);

    do
    {
    	QString sHome = qgetenv("ORACLE_HOME");
        if( sHome.isEmpty())
            continue;
        QDir dHome(sHome);
        if( !dHome.exists())
            continue;
        if( possibleOracleHomes.contains(dHome.absolutePath()))
            continue;
        TLOG(5, toNoDecorator, __HERE__) << "searching: " << dHome.absolutePath()  << std::endl;
        possibleOracleHomes.insert(dHome.absolutePath());
    }
    while(false);

#ifdef Q_OS_WIN32
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
        if( possibleOracleHomes.contains(dHome.absolutePath()))
            continue;

        TLOG(5, toNoDecorator, __HERE__) << "searching oh: " << dHome.absolutePath()  << std::endl;

        QString sLibPath = QDir::toNativeSeparators(dHome.absolutePath() + "/bin/" + m_libname.first());
        if( !Utils::toLibrary::isValidLibrary(sLibPath))
            continue;

        TLOG(5, toNoDecorator, __HERE__) << "adding: " << sLibPath << ':' << version << std::endl;

        ohome.insert("PATH", sHome);
        ohome.insert("ORACLE_HOME", sHome);
        ohome.insert("LIBPATH", sLibPath);
        ohome.insert("NAME", sHomeName);
        ohome.insert("VERSION", version);
        ohome.insert("KEY", name());
        ohome.insert("IS INSTANT", false);
        ohome.insert("PROVIDER", ORACLE_PROVIDER);
        possibleOracleHomes.insert(sHome);
        retval.append(ohome);
        ohome.clear();
    }
#endif

    foreach( QString s, possibleOracleHomes)
    {
    	QDir ohDir(s);
    	QDir ohLibDir(s + QDir::separator() + QString::fromAscii("lib"));
    	QStringList sLibraries = ohLibDir.entryList( m_libname);
    	QString version;
    	foreach( QString sLibrary, sLibraries)
    	{
    		QString sLibPath = QDir::toNativeSeparators(ohLibDir.canonicalPath()) + QDir::separator() + sLibrary;

    		if(!Utils::toLibrary::isValidLibrary(sLibPath))
    		{
    			TLOG(5, toNoDecorator, __HERE__) << "skipping: " << sLibPath << std::endl;
    			continue;
    		}

    		//version = clientVersion(sLibPath);
    		TLOG(5, toNoDecorator, __HERE__) << "adding: " << sLibPath << ':' << version << std::endl;
    		ohome.insert("PATH", QDir::toNativeSeparators(ohDir.canonicalPath()));
    		ohome.insert("ORACLE_HOME", QDir::toNativeSeparators(ohDir.canonicalPath()));
    		ohome.insert("LIBPATH", sLibPath);
    		ohome.insert("VERSION", version);
    		ohome.insert("KEY", name());
    		ohome.insert("IS INSTANT", false);
    		ohome.insert("PROVIDER", ORACLE_PROVIDER);
    		retval.append(ohome);
    		ohome.clear();
    	}
    }

    return retval;
}

void toOracleFinder::load(ConnectionProvirerParams const &params)
{
    check(params);
    setEnv(params);
    loadLib(params);
    m_OCILoaded = true;
}

void toOracleFinder::loadLib(ConnectionProvirerParams const &params)
{
#ifdef Q_OS_WIN32
    QFileInfo libPath(params.value("LIBPATH").toString());
    TLOG(5, toNoDecorator, __HERE__) << "Loading:" << libPath.absoluteFilePath() << std::endl;
    Utils::toLibrary::LHandle hmoduleOCI = Utils::toLibrary::loadLibrary(libPath);
    if ( hmoduleOCI)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " TROTL_LIB  << std::endl;
    Utils::toLibrary::LHandle hmoduleTrotl = Utils::toLibrary::loadLibrary(QFileInfo(TROTL_LIB));
    if ( hmoduleTrotl)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " PROVIDER_LIB << std::endl;
    Utils::toLibrary::LHandle hmodulePOracle = Utils::toLibrary::loadLibrary(QFileInfo(PROVIDER_LIB));
    if ( hmodulePOracle)
    	TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;
    else
    	TLOG(5, toNoDecorator, __HERE__) << "Failed" << std::endl;
#else
    QFileInfo libPath(params.value("LIBPATH").toString());
    TLOG(5, toNoDecorator, __HERE__) << "Loading:" << libPath.absoluteFilePath() << std::endl;
    Utils::toLibrary::LHandle hmoduleOCI = Utils::toLibrary::loadLibrary(libPath);
    if ( hmoduleOCI)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " TROTL_LIB  << std::endl;
    Utils::toLibrary::LHandle hmoduleTrotl = Utils::toLibrary::loadLibrary(QFileInfo(PROVIDERS_PATH "/../" TROTL_LIB));
    if ( hmoduleTrotl)
        TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;

    TLOG(5, toNoDecorator, __HERE__) << "Loading: " PROVIDER_LIB << std::endl;
    Utils::toLibrary::LHandle hmodulePOracle = Utils::toLibrary::loadLibrary(QFileInfo(QDir(PROVIDERS_PATH), PROVIDER_LIB));
    if ( hmodulePOracle)
    	TLOG(5, toNoDecorator, __HERE__) << "OK" << std::endl;
    else
    	TLOG(5, toNoDecorator, __HERE__) << "Failed" << std::endl;
#endif
}

Util::RegisterInFactory<toOracleFinder, ConnectionProviderFinderFactory> regToOracleFind(ORACLE_TNSCLIENT);
