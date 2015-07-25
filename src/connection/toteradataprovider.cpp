
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
#define TORA_DLL

#include "core/toconnectionprovider.h"
#include "connection/absfact.h"
#include "connection/toteradatatraits.h"
#include "core/tologger.h"
#include "core/utils.h"

#include <QtCore/QSettings>
#include <QtCore/QDir>

extern "C" {
#include <coptypes.h>       /* Teradata include files */
#include <coperr.h>
#include <dbcarea.h>
#include <parcel.h>
#include <dbchqep.h>
}

class toTeradataProvider : public  toConnectionProvider
{
    public:
        toTeradataProvider(toConnectionProviderFinder::ConnectionProvirerParams const& p) : toConnectionProvider(p) {};

        /** see: @ref toConnectionProvider::initialize() */
        virtual bool initialize();

        /** see: @ref toConnectionProvider::name() */
        virtual QString const& name() const
        {
            return m_name;
        };

        virtual QString const& displayName() const
        {
            return m_name;
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
        static QString m_name;
};

QString toTeradataProvider::m_name = "Teradata";

bool toTeradataProvider::initialize()
{
    return true;
}

QList<QString> toTeradataProvider::hosts()
{
    QList<QString> ret = QList<QString>() << ":1025"; // default port
    return ret;
}

QList<QString> toTeradataProvider::databases(const QString &host, const QString &user, const QString &pwd)
{
    QList<QString> ret;
    // TODO
    return ret;
}

QList<QString> toTeradataProvider::options()
{
    QList<QString> ret;
    return ret;
}

QWidget* toTeradataProvider::configurationTab(QWidget *parent)
{
    return NULL;
}

toConnection::connectionImpl* toTeradataProvider::createConnectionImpl(toConnection &con)
{
    throw toConnection::exception("toTeradataProvider::createConnectionImpl Not implemented yet");
    return NULL;
}

// TODO return reference into static instance of toTeradataTraits
toConnectionTraits* toTeradataProvider::createConnectionTrait(void)
{
    static toTeradataTraits *t = new toTeradataTraits();
    return t;
}

Util::RegisterInFactory<toTeradataProvider, ConnectionProvirerFactory> regToTeradataProvider("Teradata");
