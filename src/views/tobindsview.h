
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
#include "widgets/totableview.h"

class toEventQuery;

namespace Views
{
class toTableView;
}

class toBindModel: public toTableModelPriv
{
    typedef toTableModelPriv super;
    using toTableModelPriv::toTableModelPriv;

public:

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(QModelIndex const& index) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void appendRows(toQueryAbstr::RowList &);
    void appendRow(toQueryAbstr::Row &);
//    void setHeaders(const toQueryAbstr::HeaderList &);
//    toQueryAbstr::HeaderList & headers(void) { return Headers; }
//    toQueryAbstr::HeaderList const& headers(void) const { return Headers; }
protected:
    int bindsXMLIndex;
};

namespace BindVariables
{
    struct Traits : public MVCTraits
    {
        typedef toBindModel Model;
        static const bool AlternatingRowColorsEnabled = true;
        static const int  ShowRowNumber = NoRowNumber;
        static const int  ColumnResize = RowColumResize;
        static const int  SelectionMode = QAbstractItemView::ExtendedSelection;
        static const bool HideColumns = true;

        typedef Views::toTableView View;
    };

    class MVC
            : public TOMVC
              <
              ::BindVariables::Traits,
              Views::DefaultTableViewPolicy,
              ::DefaultDataProviderPolicy
              >
    {
        Q_OBJECT;
    public:
        typedef TOMVC<
                ::BindVariables::Traits,
                Views::DefaultTableViewPolicy,
                ::DefaultDataProviderPolicy
                  > _s;
        MVC(QWidget *parent) : _s(parent)
        {};
        virtual ~MVC() {};
    };
}

class toBindVariablesView : public BindVariables::MVC
{
    Q_OBJECT;
    typedef BindVariables::MVC super;
public:
    explicit toBindVariablesView(QWidget *parent = 0);
#if 0
    void setSQL(QString const&);
#endif
};
