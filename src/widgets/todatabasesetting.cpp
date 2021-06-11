
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

#include "widgets/todatabasesetting.h"
#include "core/todatabaseconfig.h"
#include "core/toconfiguration.h"
#include "core/toqvalue.h"

#include <QColorDialog>
#include <QtCore/QString>
#include <QFileDialog>

#include <QtCore/QDebug>
#include <QComboBox>
#include <QtGlobal>

void toDatabaseSetting::numberFormatChange()
{
    NumberDecimalsInt->setEnabled(NumberFormatInt->currentIndex() == 2);
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

toDatabaseSetting::toDatabaseSetting(QWidget *parent, const char *name)
    : QWidget(parent)
    , toSettingTab("database.html")
{
    if (name)
        setObjectName(name);
    setupUi(this);

    connect(IndicateEmptyBool, SIGNAL(clicked(bool)), IndicateEmptyColor, SLOT(setEnabled(bool)));
#if QT_VERSION_MAJOR < 6
    connect(NumberFormatInt, SIGNAL(activated(int)), this, SLOT(numberFormatChange()));
#else
    connect(NumberFormatInt, qOverload<int>(&QComboBox::activated), this, [=](int) { this->numberFormatChange(); });
#endif
    connect(IndicateEmptyColor, &QPushButton::clicked, this, [=]() { this->IndicateEmptyColor_clicked(); });

    toSettingTab::loadSettings(this);

    int mxNumber = toConfigurationNewSingle::Instance().option(ToConfiguration::Database::InitialFetchInt).toInt();
    if (mxNumber <= 0)
        FetchAllBool->setChecked(true);
    FetchAllBool->setEnabled(true);

    int mxContent = toConfigurationNewSingle::Instance().option(ToConfiguration::Database::MaxContentInt).toInt();
    if (mxContent <= 0)
    {
        MaxContentInt->setValue(InitialFetchInt->value());
        UnlimitedContentBool->setChecked(true);
    }
    UnlimitedContentBool->setEnabled(true);

    if (NumberFormatInt->currentIndex() == 2)
        NumberDecimalsInt->setEnabled(true);
    else
        NumberDecimalsInt->setEnabled(false);

    QColor nullColor;
    nullColor.setNamedColor(toConfigurationNewSingle::Instance().option(ToConfiguration::Database::IndicateEmptyColor).toString());
    QPalette palette = IndicateEmptyColor->palette();
    palette.setColor(IndicateEmptyColor->backgroundRole(), nullColor);
    IndicateEmptyColor->setPalette(palette);

}

void toDatabaseSetting::saveSetting(void)
{
    toSettingTab::saveSettings(this);

    if (FetchAllBool->isChecked())
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Database::InitialFetchInt, -1);
    else
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Database::InitialFetchInt, InitialFetchInt->value());

    if (UnlimitedContentBool->isChecked())
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Database::MaxContentInt, -1);
    else
    {
        int num = InitialFetchInt->value();
        int maxnum = MaxContentInt->value();
        if (num < 0)
            maxnum = num;
        else if (num >= maxnum)
            maxnum = num + 1;
        if (maxnum != MaxContentInt->text().toInt())
            TOMessageBox::information(this, tr("Invalid values"),
                                      tr("Doesn't make sense to have max content less than initial\n"
                                         "fetch size. Will adjust value to be higher."),
                                      tr("&Ok"));
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Database::MaxContentInt, maxnum);
    }

    toQValue::setNumberFormat(NumberFormatInt->currentIndex(), NumberDecimalsInt->value());
}

ToConfiguration::Database toDatabaseSetting::s_databaseConfig;
