#ifndef TOCONFIGURATION_NEW_H
#define TOCONFIGURATION_NEW_H

#include "core/tora_export.h"
#include "core/toconfenum.h"
#include "editor/tostyle.h"

#include <loki/Singleton.h>

#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtGui/QApplication>

namespace ToConfiguration {
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
    template <class T> void setOption(int option, T const&);

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
    QMap<unsigned, QVariant> m_configMap;
    QMap<unsigned, ToConfiguration::ConfigContext const*> m_configContextPtrMap;
    QMap<QString, QMetaEnum> m_contextMap;
    QMap<QString, ToConfiguration::ConfigContext const*> m_contextSetPtrMap;
    QMap<QString, int> m_optionToEnumMap;
};

typedef Loki::SingletonHolder<toConfigurationNew> toConfigurationNewSingle;

#endif
