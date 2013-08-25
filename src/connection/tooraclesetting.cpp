
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries.
 *
 *      You may link this product with any GPL'd Qt library.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "connection/tooraclesetting.h"
#include "core/toconfiguration.h"
#include "core/tologger.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toconnectionregistry.h"

toOracleSetting::toOracleSetting(QWidget *parent)
    : QWidget(parent)
    , toSettingTab("database.html#oracle")
{

    setupUi(this);
    DefaultDate->setText(toConfigurationSingle::Instance().dateFormat());
    DefaultTimestamp->setText(toConfigurationSingle::Instance().timestampFormat());
    //CheckPoint->setText(toConfigurationSingle::Instance().planCheckpoint());
    ExplainPlan->setText(toConfigurationSingle::Instance().planTable(NULL));
    //OpenCursors->setValue(toConfigurationSingle::Instance().openCursors());
    KeepPlans->setChecked(toConfigurationSingle::Instance().keepPlans());
    VsqlPlans->setChecked(toConfigurationSingle::Instance().vsqlPlans());
    SharedPlan->setChecked(toConfigurationSingle::Instance().sharedPlan());
    int len = toConfigurationSingle::Instance().maxLong();
    if (len >= 0)
    {
        MaxLong->setText(QString::number(len));
        MaxLong->setValidator(new QIntValidator(MaxLong));
        Unlimited->setChecked(false);
    }
    // extractor group options
    cbUseDbmsMetadata->setChecked(toConfigurationSingle::Instance().extractorUseDbmsMetadata());
    cbIncludeStorage->setChecked(toConfigurationSingle::Instance().extractorIncludeSotrage());
    cbSkipOrgMon->setChecked(toConfigurationSingle::Instance().extractorSkipOrgMonInformation());
    cbSkiptStorExTablespace->setChecked(toConfigurationSingle::Instance().extractorSkipStorageExceptTablespaces());
    cbIncludeParallel->setChecked(toConfigurationSingle::Instance().extractorIncludeParallel());
    cbIncludePartition->setChecked(toConfigurationSingle::Instance().extractorIncludePartition());
    cbIncludeCode->setChecked(toConfigurationSingle::Instance().extractorIncludeCode());
    cbIncludeHeader->setChecked(toConfigurationSingle::Instance().extractorIncludeHeader());
    cbIncludePrompt->setChecked(toConfigurationSingle::Instance().extractorIncludePrompt());
    try
    {
        // Check if connection exists
    	toConnectionRegistrySing::Instance().currentConnection();
        CreatePlanTable->setEnabled(true);
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
}


void toOracleSetting::saveSetting()
{
    toConfigurationSingle::Instance().setKeepPlans(KeepPlans->isChecked());
    toConfigurationSingle::Instance().setVsqlPlans(VsqlPlans->isChecked());
    toConfigurationSingle::Instance().setSharedPlan(SharedPlan->isChecked());
    toConfigurationSingle::Instance().setDateFormat(DefaultDate->text());
    toConfigurationSingle::Instance().setTimestampFormat(DefaultTimestamp->text());

#pragma message WARN("TODO/FIXME: apply new NLS_DATE_FORMAT/NLS_TIMESTAMP_FORMAT here")

    //toConfigurationSingle::Instance().setPlanCheckpoint(CheckPoint->text());
    toConfigurationSingle::Instance().setPlanTable(ExplainPlan->text());
    //toConfigurationSingle::Instance().setOpenCursors(OpenCursors->value());
    if (Unlimited->isChecked())
    {
        toConfigurationSingle::Instance().setMaxLong(-1);
    }
    else
    {
        toConfigurationSingle::Instance().setMaxLong(MaxLong->text().toInt());
    }
    // extractor group options
    toConfigurationSingle::Instance().setExtractorUseDbmsMetadata(cbUseDbmsMetadata->isChecked());
    toConfigurationSingle::Instance().setExtractorIncludeSotrage(cbIncludeStorage->isChecked());
    toConfigurationSingle::Instance().setExtractorSkipOrgMonInformation(cbSkipOrgMon->isChecked());
    toConfigurationSingle::Instance().setExtractorSkipStorageExceptTablespaces(cbSkiptStorExTablespace->isChecked());
    toConfigurationSingle::Instance().setExtractorIncludeParallel(cbIncludeParallel->isChecked());
    toConfigurationSingle::Instance().setExtractorIncludePartition(cbIncludePartition->isChecked());
    toConfigurationSingle::Instance().setExtractorIncludeCode(cbIncludeCode->isChecked());
    toConfigurationSingle::Instance().setExtractorIncludeHeader(cbIncludeHeader->isChecked());
    toConfigurationSingle::Instance().setExtractorIncludePrompt(cbIncludePrompt->isChecked());
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
