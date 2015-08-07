
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

#ifndef __TOHELPCONTEXT_H__
#define __TOHELPCONTEXT_H__

#include <QtCore/QString>

/**
 * This class is used to indicate a help context of an object. When
 * the current context is to be determined you start at the widget
 * that currently has focus and walk upwards to it's parent until you
 * find a widget that also inherits this class which specify the
 * context of all underlying widgets.
 */
class toHelpContext
{
    public:
        /** Create a context.
         * @param file File in the help directory that describes the current context.
         */
        toHelpContext(const QString &file) : Context(file) { }

        virtual ~toHelpContext() { }
        /**
         * Get this context
         * @return The location of a file that describes the current context.
         */
        virtual const QString &context(void) const
        {
            return Context;
        }
    private:
        /** The current context. */
        QString Context;
};

#endif
