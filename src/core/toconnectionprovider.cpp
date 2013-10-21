
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
#include "core/tooracleconst.h"
#include "connection/absfact.h"

void toConnectionProviderRegistry::load(toConnectionProviderFinder::ConnectionProvirerParams const& providerParams)
{
    QString const& finderName = providerParams.value("KEY").toString();
    QString const& providerName = providerParams.value("PROVIDER").toString();

    if( m_registry.contains(providerName))
    {
        throw QString("Provider already loaded: %1").arg(providerName);
    }

    /* load all dlls. */
    std::auto_ptr<toConnectionProviderFinder> finder = ConnectionProviderFinderFactory::Instance().create(finderName.toStdString(), 0);
    if(!finder.get())
      throw QString("Unknown finder to create: %1").arg(finderName);

    finder->load(providerParams);

    /* dlls are loaded. Create instance of the toConnectioProvider */
    std::auto_ptr<toConnectionProvider> pProvider = ConnectionProvirerFactory::Instance().create(providerName.toStdString(), providerParams);
    if(!pProvider.get())
      throw QString("Unknown provider to create: %1").arg(providerName);

    if( pProvider->initialize())
    {
        m_registry.insert(providerName, pProvider.release());
    }
};

toConnectionProvider& toConnectionProviderRegistry::get(QString const& providerName)
{
    if( !m_registry.contains(providerName))
    {
        throw QString("Connection provider not loaded: %1").arg(providerName);
    }

    return *m_registry.value(providerName);
};

toConnectionProvider const& toConnectionProviderRegistry::get(QString const& providerName) const
{
    if( !m_registry.contains(providerName))
    {
        throw QString("Connection provider not loaded: %1").arg(providerName);
    }
    return *m_registry.value(providerName);
};

QList<QString> toConnectionProviderRegistry::providers() const
{
    return m_registry.keys();
}
