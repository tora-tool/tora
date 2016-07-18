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

#pragma once

#include "core/tora_export.h"
#include "core/toconfenum.h"
#include <loki/Singleton.h>

#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtCore/QVariant>
#include <QApplication>
#include "tostyle.h"

namespace ToConfiguration
{
    class ConfigContext;
};

class QMetaEnum;

class TORA_EXPORT toConfigurationNew: public QObject
{
        Q_OBJECT;
        friend class ::ToConfiguration::ConfigContext;
    public:
        class OptionNotFound : public std::exception
    {};

        toConfigurationNew(QObject *parent = 0);
        virtual ~toConfigurationNew();

        QVariant option(int option);
        QVariant option(QString const& option);
        template <class T> void setOption(QString const& optionName, T const& newVal)
        {
            if (m_optionToEnumMap.contains(optionName))
            {
                setOption(m_optionToEnumMap.value(optionName), newVal);
            }
            else
            {
                logUnknownOption(optionName);
            }
        }

        template <class T> void setOption(int option, T const&)
        {
        };

        void saveAll();

        /*! \brief Set the QSettings access strings.
        QSettings uses these. It's used in the main.cpp before
        new QApplication instance init.
        The 2nd usage is in the toConfiguration constructor due
        some strange loading -- values are ignored with empty
        QSettings constructor (maybe due the Loki lib?) */
        static void setQSettingsEnv();

        // Application location paths
        static QString sharePath();
    protected:
        void registerConfigContext(QString const& context, QMetaEnum const& fields, ToConfiguration::ConfigContext const*);
    private:
        void logUnknownOption(QString const&);

        QMap<unsigned, QVariant> m_configMap;
        QMap<unsigned, ToConfiguration::ConfigContext const*> m_configContextPtrMap;
        QMap<QString, QMetaEnum> m_contextMap;
        QMap<QString, ToConfiguration::ConfigContext const*> m_contextSetPtrMap;
        QMap<QString, int> m_optionToEnumMap;
        QMap<int, QString> m_enumToOptionMap;
        QSettings m_settings;
};

template<> TORA_EXPORT
void toConfigurationNew::setOption <QVariant>(int option, QVariant const&);

template<> TORA_EXPORT
void toConfigurationNew::setOption <QString>(int option, QString const&);

template<> TORA_EXPORT
void toConfigurationNew::setOption <int>(int option, int const&);

template<> TORA_EXPORT
void toConfigurationNew::setOption <bool>(int option, bool const&);

template<> TORA_EXPORT
void toConfigurationNew::setOption <QDate>(int option, QDate const&);

class TORA_EXPORT toConfigurationNewSingle: public ::Loki::SingletonHolder<toConfigurationNew> {};

