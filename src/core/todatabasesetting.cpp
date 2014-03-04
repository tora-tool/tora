
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

#include "core/todatabasesetting.h"
#include "core/utils.h"
#include "core/toconfenum.h"
#include "core/toconfiguration_new.h"
#include "core/tomainwindow.h"
#include "core/toqvalue.h"
#include "ts_log/ts_log_utils.h"

#include <QtGui/QColorDialog>
#include <QtCore/QString>
#include <QtGui/QFileDialog>

#include <QtCore/QDebug>

QVariant ToConfiguration::Database::defaultValue(int option) const
{
	switch(option)
	{
	case ObjectCache:              return QVariant((int)1);
	case CacheTimeout: 	           return QVariant((int)7);
	case AutoCommit:               return QVariant((bool)false);
	case FirewallMode:             return QVariant((bool)false);
	case ConnectionTestInterval:   return QVariant((int)900);     //15min
	case CachedConnections:        return QVariant((int)4);
	case MaxNumber:                return QVariant((int)50);
	case MaxContent:               return QVariant((int)100);
	case MaxColDisp:               return QVariant((int)300);
	case IndicateEmpty:            return QVariant((bool)true);
	case IndicateEmptyColor:       return QVariant(QString("#f2ffbc"));
	case NumberFormat:             return QVariant((int)0);
	case NumberDecimals:           return QVariant((int)2);
	default:
		Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Editor un-registered enum value: %1").arg(option)));
		return QVariant();
	}
}

void toDatabaseSetting::numberFormatChange()
{
    Decimals->setEnabled(NumberFormat->currentIndex() == 2);
}

void toDatabaseSetting::IndicateEmptyColor_clicked()
{
    QPalette palette = IndicateEmptyColor->palette();
    QColor c = QColorDialog::getColor(
                   palette.color(IndicateEmptyColor->backgroundRole()),
                   this);

    if (c.isValid())
    {
        palette.setColor(IndicateEmptyColor->backgroundRole(), c);
        IndicateEmptyColor->setPalette(palette);
    }
}

toDatabaseSetting::toDatabaseSetting(QWidget *parent, const char *name, Qt::WFlags fl)
    : QWidget(parent, fl)
    , toSettingTab("database.html")
{
    if (name)
        setObjectName(name);
    setupUi(this);

    MaxColDisp->setValue(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::MaxColDisp).toInt());
    int mxNumber = toConfigurationNewSingle::Instance().option(ToConfiguration::Database::MaxNumber).toInt();
    if (mxNumber <= 0)
        FetchAll->setChecked(true);
    else
        InitialFetch->setValue(mxNumber);

    int mxContent = toConfigurationNewSingle::Instance().option(ToConfiguration::Database::MaxContent).toInt();
    if (mxContent <= 0)
    {
        MaxContent->setValue(InitialFetch->value());
        UnlimitedContent->setChecked(true);
    }
    else
        MaxContent->setValue(mxContent);

//     MaxColDisp->setValidator(new QIntValidator(MaxColDisp));
//     InitialFetch->setValidator(new QIntValidator(InitialFetch));
//     MaxContent->setValidator(new QIntValidator(InitialFetch));q

    //NumberFormat->setCurrentIndex(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::numberFormat());

    //Decimals->setValue(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::numberDecimals());
    if (NumberFormat->currentIndex() == 2)
        Decimals->setEnabled(true);

    //AutoCommit->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::autoCommit());
//     DontReread->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::dontReread());
    ////ObjectCache->setCurrentIndex(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::objectCache());
//     BkgndConnect->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::bkgndConnect());
    //CachedConnections->setValue(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::cachedConnections());
    ////IndicateEmpty->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::indicateEmpty());
    //FirewallMode->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::firewallMode());
    //ConnTestInterval->setValue(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::connTestInterval());

    QColor nullColor;
    nullColor.setNamedColor(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::IndicateEmptyColor).toString());
    QPalette palette = IndicateEmptyColor->palette();
    palette.setColor(IndicateEmptyColor->backgroundRole(), nullColor);
    IndicateEmptyColor->setPalette(palette);

//     int val = toConfigurationNewSingle::Instance().option(ToConfiguration::Database::autoLong();
//     AutoLong->setChecked(val);
//     MoveAfter->setValue(val);
//     KeepAlive->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::keepAlive());

    connect(IndicateEmpty, SIGNAL(clicked(bool)),
            IndicateEmptyColor, SLOT(setEnabled(bool)));
}

// void toUpdateIndicateEmpty(void);

void toDatabaseSetting::saveSetting(void)
{
	toSettingTab::saveSettings(this);
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setObjectCache((toConfiguration::ObjectCacheEnum)ObjectCache->currentIndex());

    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setAutoCommit(AutoCommit->isChecked());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setFirewallMode(FirewallMode->isChecked());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setConnTestInterval(ConnTestInterval->value());
    ///toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setCachedConnections(CachedConnections->value());


    if (FetchAll->isChecked())
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Database::MaxNumber, -1);
    else
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Database::MaxNumber, InitialFetch->value());

    if (UnlimitedContent->isChecked())
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Database::MaxContent, -1);
    else
    {
        int num = InitialFetch->value();
        int maxnum = MaxContent->value();
        if (num < 0)
            maxnum = num;
        else if (num >= maxnum)
            maxnum = num + 1;
        if (maxnum != MaxContent->text().toInt())
            TOMessageBox::information(this, tr("Invalid values"),
                                      tr("Doesn't make sense to have max content less than initial\n"
                                         "fetch size. Will adjust value to be higher."),
                                      tr("&Ok"));
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Database::MaxContent, maxnum);
    }

    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setMaxColDisp(MaxColDisp->value());

//     toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setDontReread(DontReread->isChecked());

//     toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setBkgndConnect(BkgndConnect->isChecked());
//     toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setAutoLong(AutoLong->isChecked() ? MoveAfter->value() : 0);
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setIndicateEmpty(IndicateEmpty->isChecked());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setIndicateEmptyColor(IndicateEmptyColor->palette().color(IndicateEmptyColor->backgroundRole()).name());
//     toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setKeepAlive(KeepAlive->isChecked() ? DEFAULT_KEEP_ALIVE : -1); //FIXME: there was ""



    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setNumberFormat(NumberFormat->currentIndex());
    ////toConfigurationNewSingle::Instance().option(ToConfiguration::Database::setNumberDecimals(Decimals->value());
    toQValue::setNumberFormat(NumberFormat->currentIndex(), Decimals->value());

//     toUpdateIndicateEmpty();
}
