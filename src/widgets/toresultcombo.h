#pragma once
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


#include "core/toresult.h"

#include <QComboBox>
#include <QtCore/QStringList>
#include <QtCore/QPointer>

class toEventQuery;
class toSQL;

/** This widget displays the result of a query where each field is added as an item
 * to a combobox.
 */
class toResultCombo : public QComboBox, public toResult
{
    private:
        Q_OBJECT;

    public:
        enum selectionPolicy
        {
            LastButOne = -1,
            Last = 0,
            First = 1,
            None
        };

        /** Create the widget.
         * @param parent Parent widget.
         * @param name Name of widget.
         */
        toResultCombo(QWidget *parent, const char *name = NULL);

        /** Destruct object
         */
        virtual ~toResultCombo();

        /** Reimplemented abstract method
         */
        virtual void query(const QString &sql, toQueryParams const& param) override; // from toResult

        /** Handle any connection by default
         */
        virtual bool canHandle(const toConnection &) override; // from toResult

        /** Clear list of additional items.
         */
        void clearAdditional();

        /** Item to add before the query is read. Can be called several times.
         */
        void additionalItem(const QString &item);

        /** Set selected. When the result is read and this value is encountered that item is selected.
         */
        void setSelected(const QString &sel);

        /** Set selected. When the result is read and this non-displayed is encountered that item is selected.
         */
        void setSelectedData(const QString &sel);

        /** Get selected item value. Might not be same as currentText since that item might not have been read yet.
         */
        QString selected(void);

        void setSelectionPolicy(selectionPolicy pol);

    signals:
        /** Done reading the query.
         */
        void done(void);

    public slots:
        /** Reimplemented for internal reasons.
         */
        void refresh(void) override;

    protected slots:
        virtual void slotQueryDone(void);

    private slots:
        void receiveData(void);
        void slotChangeSelected(void);

    protected:
        selectionPolicy SelectionPolicy;
        QString Selected, SelectedData;
        QStringList Additional;
        bool SelectedFound;       // Selected field was either retrieved from DB(true) or is member of Additional fields(false)
        toEventQuery *Query;
};
