
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

#pragma once

#include "result/tomvc.h"
#include "widgets/totreeview.h"

class toEventQuery;

namespace Views
{
class toTreeView;
}

namespace ResutLock
{
    struct Traits : public MVCTraits
    {
        static const bool AlternatingRowColorsEnabled = true;
        static const int  ShowRowNumber = NoRowNumber;
        static const int  ColumnResize = RowColumResize;

        typedef Views::toTreeView View;
    };

    class MVC
            : public TOMVC
              <
              ::ResutLock::Traits,
              Views::DefaultTreeViewPolicy,
              ::DefaultDataProviderPolicy
              >
    {
        Q_OBJECT;
    public:
        typedef TOMVC<
                ::ResutLock::Traits,
                Views::DefaultTreeViewPolicy,
                ::DefaultDataProviderPolicy
                  > _s;
        MVC(QWidget *parent) : _s(parent)
        {};
        virtual ~MVC() {};
    };
}

/**
 * A result table displaying information about locks
 */
class toResultLock
        : public ResutLock::MVC
//        , public toResult
{
        Q_OBJECT
    public:
        toResultLock(QWidget *parent, const char *name = "toResultLock");
        ~toResultLock();

        /** Support Oracle
         */
        //bool canHandle(const toConnection &conn) /* TODO does not called - override */;

    private:
};
