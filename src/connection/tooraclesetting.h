
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

#ifndef TOORACLESETTING_H
#define TOORACLESETTING_H

#include "core/tora_export.h"
#include "core/toconfenum.h"
#include "core/totool.h"

#include "ui_tooraclesettingui.h"

namespace ToConfiguration
{
	class Oracle : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Oracle() : ConfigContext("Oracle", ENUM_REF(Oracle,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			ConfDateFormat = 5000 // #define CONF_DATE_FORMAT
			, ConfTimestampFormat // #define CONF_TIMESTAMP_FORMAT
			, MaxLong             // #define CONF_MAX_LONG
			, PlanTable           // #define CONF_PLAN_TABLE
			, KeepPlans           // #define CONF_KEEP_PLANS  // default: false
			, VSqlPlans           // #define CONF_VSQL_PLANS  // default: true
			, SharedPlan          // #define CONF_SHARED_PLAN // default: false
			// Extractor. Controls the method used to produce database object extracts
			// as well as extraction settings (which information should be extracted).
			, UseDbmsMetadata      // #define CONF_USE_DBMS_METADATA
			, IncludeStorage       // #define CONF_EXT_INC_STORAGE
			, SkipOrgMon           // #define CONF_EXT_SKIP_ORGMON
			, SkipStorageExceptTablespace // #define CONF_EXT_SKIP_STORAGE_EX_TABLESPACE
			, IncludeParallel      // #define CONF_EXT_INC_PARALLEL
			, IncludePartition     // #define CONF_EXT_INC_PARTITION
			, IncludeCode          // #define CONF_EXT_INC_CODE
			, IncludeHeader        // #define CONF_EXT_INC_HEADER
			, IncludePrompt        // #define CONF_EXT_INC_PROMPT
		};
		virtual QVariant defaultValue(int option) const;
		static QString planTable(QString const& schema);
	};
};

/** Instance of this class is returned by dll class toOracleProvider.
  * But even though this class is part of the main program
  *
  * NOTE: therefore we can not set ::trotl::g_OCIPL_MAX_LONG from this class.
  * the class toOracleProvider is not visible from this context.
  * Whole application must be restarted to apply the changes.
  */
class TORA_EXPORT toOracleSetting 
    : public QWidget
    , public Ui::toOracleSettingUI
    , public toSettingTab
{
    Q_OBJECT;

public:
    toOracleSetting(QWidget *parent);

public slots:
    virtual void saveSetting(void);
    void createPlanTable(void);

    void dbmsMetadataClicked(bool);
private:
    static ToConfiguration::Oracle s_oracleConf;
};

#endif
