
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

#ifndef TOQMYSQLSETTING_H
#define TOQMYSQLSETTING_H

#include "core/tosettingtab.h"
#include "core/toconfenum.h"
#include "ts_log/ts_log_utils.h"

#include "ui_toqmysqlsettingui.h"

namespace ToConfiguration
{
    class MySQL : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            MySQL() : ConfigContext("MySQL", ENUM_REF(MySQL,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                BreakConnectionsBool = 6000
                                       , BeforeCreateActionInt  // #define CONF_CREATE_ACTION
            };
            virtual QVariant defaultValue(int option) const
            {
                switch (option)
                {
                    case BreakConnectionsBool :
                        return QVariant((bool)false);
                    case BeforeCreateActionInt:
                        return QVariant((int)0);
                    default:
                        Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context MySQL un-registered enum value: %1").arg(option)));
                        return QVariant();
                }
            }
    };
};

class toQMySqlSetting
    : public QWidget
    , public Ui::toMySQLSettingUI
    , public toSettingTab
{
        Q_OBJECT;
    public:
        toQMySqlSetting(QWidget *parent);

        virtual void saveSetting(void);
    private:
};

#endif
