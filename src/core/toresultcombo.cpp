
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

#include "core/toresultcombo.h"
#include "core/toresultschema.h"
#include "core/utils.h"
#include "core/toconf.h"
#include "core/toconnection.h"
#include "core/toeventquery.h"
#include "core/tosql.h"
#include "core/totool.h"

toResultCombo::toResultCombo(QWidget *parent, const char *name)
    : QComboBox(parent)
    , Query(NULL)
    , SelectionPolicy(None)
	, SelectedFound(false)
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
        clear();
        SelectedFound = false;
        blockSignals(true); // Do not emit currentIndexChanged when 1st field is inserted
        addItems(Additional);
        for (int i = 0; i < Additional.count(); i++)
            if (Additional[i] == Selected)
                setCurrentIndex(i);
        {
		Q_ASSERT_X(Query == NULL , qPrintable(__QHERE__), "toResultCombo query while BG is running");
            	Query = new toEventQuery(this, connection(), sql, param, toEventQuery::READ_ALL);
            	connect(Query, SIGNAL(dataAvailable(toEventQuery*)), this, SLOT(slotPoll()));
            	connect(Query, SIGNAL(done(toEventQuery*)), this, SLOT(slotQueryDone()));
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

void toResultCombo::slotPoll(void)
{
    try
    {
        if (!Utils::toCheckModal(this))
            return ;

        if (Query)
        {
            while (Query->hasMore())
            {
                QString t = Query->readValue();
                QStringList l;
                for (unsigned i = 1; i < Query->describe().size(); ++i)
                {
                    QString v = Query->readValue();
                    l.append(v);
                }
                addItem(t, QVariant(l));
//                if (queryingUserlist)
//                {
//                    userList.append(new toCacheEntryUser(t));
//                }
                if (t == Selected)
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
    switch(SelectionPolicy)
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

// If we were querying user list - save it to cache
//    if (queryingUserlist)
//    {
//        connection().getCache().updateUserList(userList, toCache::USERS);
//        userList.clear();
//    }
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
