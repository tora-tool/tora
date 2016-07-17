
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

#include "main/tooraclesetting.h"

#include "core/toconfiguration.h"

#include "core/tologger.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toconnectionsubloan.h"
#include "core/toconnectionregistry.h"
#include "core/tosql.h"
#include "core/toqvalue.h"
#include "core/toquery.h"

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
        toConnection &conn = toConnectionRegistrySing::Instance().currentConnection();
        CreatePlanTable->setEnabled(conn.providerIs("Oracle"));
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
}

void toOracleSetting::createPlanTable()
{
    try
    {
        Utils::toBusy busy;
        toConnection &conn = toConnectionRegistrySing::Instance().currentConnection();
        if (!conn.providerIs("Oracle"))
            return;
        toConnectionSubLoan connSub(conn);
        toQuery createPlanTable(connSub, toSQL::string(toSQL::TOSQL_CREATEPLAN, conn).arg(PlanTable->text().trimmed()), toQueryParams());
        createPlanTable.eof();
    }
    TOCATCH;
}

ToConfiguration::Oracle toOracleSetting::s_oracleConf;
