
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

#ifndef __TOSETTINGTAB_H__
#define __TOSETTINGTAB_H__

#include "core/tohelpcontext.h"
#include "core/tora_export.h"

/**
 * Abstract baseclass for widgets defining tool settings.
 */
class TORA_EXPORT toSettingTab : public toHelpContext
{
    public:
        /**
         * Default constructor.
         * @param ctx Help context for this setting tab.
         */
        toSettingTab(const QString &ctx)
            : toHelpContext(ctx)
        { }
        /**
         * This function is called to save the contents of the widget when
         * a user has pressed the ok button of the dialog. It should simply
         * save the values in the dialog to the appropriate configuration
         * entry using the @ref toTool::setConfig function.
         */
        virtual void saveSetting(void) = 0;

        /*
         * This function iterates over all child widgets an tries to find matching config option.
         * Based on objectName and Widget type. Then configuration option's values are load from configuration into Widgets
         */
        virtual void loadSettings(QWidget*);

        /*
         * This function iterates over all child widgets an tries to find matching config option.
         * Based on objectName and Widget type. Then configuration option's values are save into configuration from Widgets
         */
        virtual void saveSettings(QWidget*);

};

#endif
