
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

#include "widgets/toresultcombo.h"
#include "widgets/toresultschema.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/tosql.h"
#include "core/totool.h"

toResultCombo::toResultCombo(QWidget *parent, const char *name)
    : QComboBox(parent)
    , SelectionPolicy(None)
    , SelectedFound(false)
    , Query(NULL)
{
    setObjectName(name);
    connect(this, SIGNAL(activated(int)),
            this, SLOT(slotChangeSelected(void)));
    setSizeAdjustPolicy(QComboBox::AdjustToContents);
    //queryingUserlist = false;
}

toResultCombo::~toResultCombo()
{
    if (Query)
        delete Query;
}

void toResultCombo::query(const QString &sql, toQueryParams const& param)
{
    if (!setSqlAndParams(sql, param))
        return ;

    try
    {
        SelectedFound = false;
        blockSignals(true); // Do not emit currentIndexChanged when 1st field is inserted
        clear();
        addItems(Additional);
        for (int i = 0; i < Additional.count(); i++)
            if (Additional[i] == Selected)
                setCurrentIndex(i);
        {
            //Q_ASSERT_X(Query == NULL , qPrintable(__QHERE__), "toResultCombo query while BG is running");
            if (Query)
            {
                Query->disconnect(this);
                delete Query;
                Query = NULL;
            }
            Query = new toEventQuery(this, connection(), sql, param, toEventQuery::READ_ALL);
            auto c1 = connect(Query, &toEventQuery::dataAvailable, this, &toResultCombo::receiveData);
            auto c2 = connect(Query, &toEventQuery::done, this, [=](toEventQuery *q, unsigned long) { slotQueryDone(); });
            //connect(Query, SIGNAL(done(toEventQuery*, unsigned long)), this, SLOT(slotQueryDone()));
            Query->start();
        }
    }
    TOCATCH
}

void toResultCombo::clearAdditional()
{
    Additional.clear();
}

void toResultCombo::additionalItem(const QString &item)
{
    Additional << item;
}

bool toResultCombo::canHandle(const toConnection &)
{
    return true;
}

void toResultCombo::setSelected(const QString &sel)
{
    Selected = sel;
    SelectedFound = !Additional.contains(Selected);
}

void toResultCombo::setSelectedData(const QString &sel)
{
    SelectedData = sel;
}

QString toResultCombo::selected(void)
{
    return Selected;
}

void toResultCombo::setSelectionPolicy(selectionPolicy pol)
{
    SelectionPolicy = pol;
}

void toResultCombo::refresh(void)
{
    toResult::refresh();
}

void toResultCombo::receiveData(void)
{
    try
    {
        if (!Utils::toCheckModal(this))
            return ;

        if (Query)
        {
            while (Query->hasMore())
            {
                QString t = (QString)Query->readValue();
                QStringList l;
                for (int i = 1; i < Query->describe().size(); ++i)
                {
                    QString v = (QString)Query->readValue();
                    l.append(v);
                }
                addItem(t, QVariant(l));
                if (t == Selected)
                {
                    setCurrentIndex(count() - 1);
                    SelectedFound = true;
                }
                if (l.contains(SelectedData))
                {
                    setCurrentIndex(count() - 1);
                    SelectedFound = true;
                }
            }
        }
    }
    catch (const QString &exc)
    {
        blockSignals(false);
        emit currentIndexChanged(currentText());
        emit done();
        Utils::toStatusMessage(exc);
    }
}

void toResultCombo::slotQueryDone(void)
{
    blockSignals(false);
    switch (SelectionPolicy)
    {
        case First:
            if (currentIndex() != 0)
                setCurrentIndex(0);
            break;
        case Last:
            if (currentIndex() != count() - 1)
                setCurrentIndex(count() - 1);
            break;
        case LastButOne:
            if (currentIndex() != count() - 2)
                setCurrentIndex(count() - 2);
            break;
        case None:
            break;
    };
    setFont(font()); // Small hack to invalidate size hint of combobox which should resize to needed size.
    updateGeometry();

    emit currentIndexChanged(currentText());
    emit done();
    delete Query;
    Query = NULL;
} // queryDone

void toResultCombo::slotChangeSelected(void)
{
    Selected = currentText();
    SelectedFound = !Additional.contains(Selected);
}
