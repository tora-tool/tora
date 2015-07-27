
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

#include "connection/tooraclesetting.h"
#include "core/toconfiguration_new.h"

#include "core/tologger.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toconnectionregistry.h"

QVariant ToConfiguration::Oracle::defaultValue(int option) const
{
    switch (option)
    {
        case ConfDateFormat:
            return QVariant(QString("YYYY-MM-DD HH24:MI:SS"));
        case ConfTimestampFormat:
            return QVariant(QString("YYYY-MM-DD HH24:MI:SSXFF"));
        case MaxLong:
            return QVariant((int)30000);
        case PlanTable:
            return QVariant(QString("PLAN_TABLE"));
        case KeepPlansBool:
            return QVariant((bool)false);
        case VSqlPlansBool:
            return QVariant((bool)true);
        case SharedPlanBool:
            return QVariant((bool)false);
        case UseDbmsMetadataBool:
            return QVariant((bool)true);
        case IncludeStorageBool:
            return QVariant((bool)true);
        case SkipOrgMonBool:
            return QVariant((bool)false);
        case SkipStorageExceptTablespaceBool:
            return QVariant((bool)false);
        case IncludeParallelBool:
            return QVariant((bool)true);
        case IncludePartitionBool:
            return QVariant((bool)true);
        case IncludeCodeBool:
            return QVariant((bool)true);
        case IncludeHeaderBool:
            return QVariant((bool)true);
        case IncludePromptBool:
            return QVariant((bool)true);
        default:
            Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Oracle un-registered enum value: %1").arg(option)));
            return QVariant();
    }
}

QString ToConfiguration::Oracle::planTable(QString const& schema)
{
    bool sharedPlan = toConfigurationNewSingle::Instance().option(SharedPlanBool).toBool();
    QString planTable = toConfigurationNewSingle::Instance().option(PlanTable).toString();

    if (sharedPlan || planTable.contains('.') || schema.isNull())
        return planTable;

    return schema + '.' + planTable;
}

toOracleSetting::toOracleSetting(QWidget *parent)
    : QWidget(parent)
    , toSettingTab("database.html#oracle")
{

    setupUi(this);
    toSettingTab::loadSettings(this);
    int len = toConfigurationNewSingle::Instance().option(ToConfiguration::Oracle::MaxLong).toInt();
    if (len >= 0)
    {
        MaxLongInt->setText(QString::number(len));
        MaxLongInt->setValidator(new QIntValidator(MaxLongInt));
        UnlimitedBool->setChecked(false);
    }
    UnlimitedBool->setEnabled(true);

    connect(UseDbmsMetadataBool, SIGNAL(toggled(bool)), this, SLOT(dbmsMetadataClicked(bool)));
    dbmsMetadataClicked(UseDbmsMetadataBool->isChecked());
    try
    {
        // Check if connection exists
        toConnectionRegistrySing::Instance().currentConnection();
        CreatePlanTable->setEnabled(true);
    }
    catch (...)
    {
        CreatePlanTable->setEnabled(false);
    }
}


void toOracleSetting::saveSetting()
{
    toSettingTab::saveSettings(this);

#pragma message WARN("TODO/FIXME: apply new NLS_DATE_FORMAT/NLS_TIMESTAMP_FORMAT here")

    if (UnlimitedBool->isChecked())
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Oracle::MaxLong, -1);
}

void toOracleSetting::dbmsMetadataClicked(bool)
{
    IncludeStorageBool->setDisabled(UseDbmsMetadataBool->isChecked());
    SkipOrgMonBool->setDisabled(UseDbmsMetadataBool->isChecked());
    SkiptStorExTablespaceBool->setDisabled(UseDbmsMetadataBool->isChecked());
    IncludeParallelBool->setDisabled(UseDbmsMetadataBool->isChecked());
    IncludePartitionBool->setDisabled(UseDbmsMetadataBool->isChecked());
    IncludeCodeBool->setDisabled(UseDbmsMetadataBool->isChecked());
    IncludeHeaderBool->setDisabled(UseDbmsMetadataBool->isChecked());
    IncludePromptBool->setDisabled(UseDbmsMetadataBool->isChecked());
}

void toOracleSetting::createPlanTable()
{
    throw QString(__QHERE__ + "No implemeted yet");
#ifdef TORA3_QUERY
    try
    {
        toConnectionSubLoan connSub(toConnection::currentConnection());
        connSub->execute(toSQL::string(SQLCreatePlanTable, conn).arg(ExplainPlan->text()));
    }
    TOCATCH;
#endif
}

ToConfiguration::Oracle toOracleSetting::s_oracleConf;
