
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

#ifndef TODATABASESETTING_H
#define TODATABASESETTING_H

#include "core/tosettingtab.h"
#include "core/toconfenum.h"
#include "core/utils.h"

#include "ui_todatabasesettingui.h"

namespace ToConfiguration
{
    class Database : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            Database() : ConfigContext("Database", ENUM_REF(Database,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                ObjectCacheInt   = 4000  // #define CONF_OBJECT_CACHE
                                   , CacheTimeout        // #define CONF_CACHE_TIMEOUT (invisible)
                , AutoCommitBool          // #define CONF_AUTO_COMMIT
                , FirewallModeBool    // #define CONF_FIREWALL_MODE
                , ConnTestIntervalInt // #define CONF_CONN_TEST_INTERVAL
                , CachedConnectionsInt
                , InitialFetchInt     // #define CONF_MAX_NUMBER (InitialFetch)
                , MaxContentInt       // #define CONF_MAX_CONTENT (InitialEditorContent)
                , MaxColDispInt       // #define CONF_MAX_COL_DISP
                , IndicateEmptyBool   // #define CONF_INDICATE_EMPTY
                , IndicateEmptyColor  // #define CONF_INDICATE_EMPTY_COLOR
                , NumberFormatInt     // #define CONF_NUMBER_FORMAT
                , NumberDecimalsInt   // #define CONF_NUMBER_DECIMALS
            };
            virtual QVariant defaultValue(int) const;
    };
};

class toDatabaseSetting : public QWidget
    , public Ui::toDatabaseSettingUI
    , public toSettingTab
{
        Q_OBJECT;

    public:
        toDatabaseSetting(QWidget *parent = 0, const char *name = 0, toWFlags fl = 0);

    public slots:
        virtual void saveSetting(void);
        virtual void numberFormatChange();
        virtual void IndicateEmptyColor_clicked();
    private:
        static ToConfiguration::Database s_databaseConfig;
};

#endif
