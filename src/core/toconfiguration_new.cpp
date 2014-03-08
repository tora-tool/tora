
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

#include "core/toconfiguration_new.h"
#include "core/utils.h"
#include "core/toconf.h"
#include "ts_log/ts_log_utils.h"

#include <QtCore/QObject>
#include <QtCore/QLocale>
#include <QtCore/QSettings>
#include <QtCore/QDir>

toConfigurationNew::toConfigurationNew(QObject *parent)
	: QObject(parent)
{	
    setQSettingsEnv();
}

toConfigurationNew::~toConfigurationNew()
{
}

void toConfigurationNew::registerConfigContext(QString const& contextName, QMetaEnum const& fields, ToConfiguration::ConfigContext const* context)
{
	Q_ASSERT_X( !m_contextMap.contains(contextName), qPrintable(__QHERE__), qPrintable(QString("Context %1 is already registered").arg(contextName)));
	m_contextMap.insert(contextName, fields);

	for(int idx = 0; idx < fields.keyCount(); idx++)
	{
		int intVal = fields.value(idx);
		QString strVal = fields.key(idx);
		Q_ASSERT_X( !m_configMap.contains(intVal), qPrintable(__QHERE__), qPrintable(QString("Context %1 value %2(%3) is already registered").arg(contextName).arg(strVal).arg(intVal)));
		m_configMap.insert(intVal, QVariant());
		m_configContextPtrMap.insert(intVal, context);

		Q_ASSERT_X( !m_optionToEnumMap.contains(strVal), qPrintable(__QHERE__), qPrintable(QString("Context %1 value %2(%3) is already registered").arg(contextName).arg(strVal).arg(intVal)));
		m_optionToEnumMap.insert(strVal, intVal);
	}

}

QVariant toConfigurationNew::option(int option)
{
	Q_ASSERT_X( m_configMap.contains(option), qPrintable(__QHERE__), qPrintable(QString("Unknown enum: %1").arg(option)));
	if (m_configMap.value(option).isNull())
	{
		ToConfiguration::ConfigContext const* ctx = m_configContextPtrMap.value(option);
		QVariant defaultValue= ctx->defaultValue(option);
		QVariant oldValue= ctx->toraIIValue(option);
		if (!oldValue.isNull() && oldValue.canConvert(defaultValue.type()) && oldValue.convert(defaultValue.type()))
			m_configMap[option] = oldValue;
		else
			m_configMap[option] = defaultValue;
	}
	return m_configMap.value(option);
}

QVariant toConfigurationNew::option(QString const& o)
{
	if (!m_optionToEnumMap.contains(o))
		throw OptionNotFound();
	int opt = m_optionToEnumMap.value(o);
	return option(opt);
}

void toConfigurationNew::saveAll()
{

}

// a static one
void toConfigurationNew::setQSettingsEnv()
{
    if (QCoreApplication::organizationName().isEmpty())
        QCoreApplication::setOrganizationName(TOORGNAME);
    if (QCoreApplication::organizationDomain().isEmpty())
        QCoreApplication::setOrganizationDomain(TOORGDOMAIN);
    if (QCoreApplication::applicationName().isEmpty())
        QCoreApplication::setApplicationName(TOAPPNAME);
}

QString toConfigurationNew::sharePath()
{
#ifdef Q_OS_WIN32
	QString appl = QCoreApplication::applicationDirPath();
	appl += QDir::separator();
	appl += "i18n";
	appl += QDir::separator();
	return appl;
#else
	return QString::fromAscii(SHARE_DIR);
#endif
}

template<>
void toConfigurationNew::setOption <QVariant>(int option, QVariant const& newVal)
{
#ifdef QT_DEBUG
	ToConfiguration::ConfigContext const* ctx = m_configContextPtrMap.value(option);
	QVariant val = ctx->defaultValue(option);
	Q_ASSERT_X(newVal.type() == val.type(), qPrintable(__QHERE__), qPrintable(QString("Option enum: %1 oldtype: %2 newtype %3")
			.arg(option)
			.arg(val.typeName())
			.arg(newVal.typeName())
			));
#endif
	m_configMap[option] = newVal;
}

template<> TORA_EXPORT
void toConfigurationNew::setOption <QString>(int option, QString const& newVal)
{
#ifdef QT_DEBUG
	ToConfiguration::ConfigContext const* ctx = m_configContextPtrMap.value(option);
	QVariant val = ctx->defaultValue(option);
	Q_ASSERT_X(QVariant::String == val.type(), qPrintable(__QHERE__), qPrintable(QString("Option enum: %1 oldtype: %2 newtype QString")
			.arg(option)
			.arg(val.typeName())
			));
#endif
	m_configMap[option] = QVariant(newVal);
}

template<> TORA_EXPORT
void toConfigurationNew::setOption <int>(int option, int const&newVal)
{
#ifdef QT_DEBUG
	ToConfiguration::ConfigContext const* ctx = m_configContextPtrMap.value(option);
	QVariant val = ctx->defaultValue(option);
	Q_ASSERT_X(QVariant::Int == val.type(), qPrintable(__QHERE__), qPrintable(QString("Option enum: %1 oldtype: %2 newtype int")
			.arg(option)
			.arg(val.typeName())
	));
#endif
	m_configMap[option] = QVariant(newVal);
}

template<> TORA_EXPORT
void toConfigurationNew::setOption <bool>(int option, bool const&newVal)
{
#ifdef QT_DEBUG
	ToConfiguration::ConfigContext const* ctx = m_configContextPtrMap.value(option);
	QVariant val = ctx->defaultValue(option);
	Q_ASSERT_X(QVariant::Bool == val.type(), qPrintable(__QHERE__), qPrintable(QString("Option enum: %1 oldtype: %2 newtype bool")
			.arg(option)
			.arg(val.typeName())
	));
#endif
	m_configMap[option] = QVariant(newVal);
}

void toConfigurationNew::logUnknownOption(QString const&)
{

}
