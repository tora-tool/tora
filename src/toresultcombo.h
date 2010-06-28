
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

#ifndef TORESULTCOMBO_H
#define TORESULTCOMBO_H

#include "config.h"

#include <qcombobox.h>
#include <qstringlist.h>

#include <loki/SmartPtr.h>

#include "tobackground.h"
#include "toresult.h"

class toNoBlockQuery;
class toSQL;

using Loki::SmartPtr;

/** This widget displays the result of a query where each field is added as an item
 * to a combobox.
 */

class toResultCombo : public QComboBox, public toResult
{
    Q_OBJECT

    SmartPtr<toNoBlockQuery> Query;
    toBackground Poll;

    QString Selected;
    QStringList Additional;

public:
    enum selectionPolicy {
	    LastButOne = -1,
	    Last = 0,
	    First = 1,
	    None
    };
    selectionPolicy SelectionPolicy;
    
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultCombo(QWidget *parent, const char *name = NULL);
    /** Destruct object
     */
    ~toResultCombo();

    /** Reimplemented for internal reasons.
     */
    virtual void query(const QString &sql, const toQList &param);

    /** Clear list of additional items.
     */
    virtual void clearAdditional()
    {
        Additional.clear();
    }
    /** Item to add before the query is read. Can be called several times.
     */
    virtual void additionalItem(const QString &item)
    {
        Additional << item;
    }

    /** Handle any connection by default
     */
    virtual bool canHandle(toConnection &)
    {
        return true;
    }

    /** Set selected. When the result is read and this value is encountered that item is selected.
     */
    void setSelected(const QString &sel)
    {
        Selected = sel;
    }
    /** Get selected item value. Might not be same as currentText since that item might not have been read yet.
     */
    QString selected(void)
    {
        return Selected;
    }

    void setSelectionPolicy(selectionPolicy pol)
    {
	    SelectionPolicy = pol;
    }
    
    // Why are these needed?
#if 1
    /** Set the SQL statement of this list
     * @param sql String containing statement.
     */
    void setSQL(const QString &sql)
    {
        toResult::setSQL(sql);
    }
    /** Set the SQL statement of this list. This will also affect @ref Name.
     * @param sql SQL containing statement.
     */
    void setSQL(const toSQL &sql)
    {
        toResult::setSQL(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const QString &sql)
    {
        toResult::query(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const toSQL &sql)
    {
        toResult::query(sql);
    }
    /** Set new SQL and run query.
     * @param sql New sql.
     * @see setSQL
     */
    void query(const toSQL &sql, toQList &par)
    {
        toResult::query(sql, par);
    }
#endif
signals:
    /** Done reading the query.
     */
    void done(void);
public slots:
    /** Reimplemented for internal reasons.
     */
    virtual void refresh(void)
    {
        toResult::refresh();
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1)
    {
        toResult::changeParams(Param1);
    }
    /** Reimplemented For internal reasons.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2)
    {
        toResult::changeParams(Param1, Param2);
    }
    /** Reimplemented for internal reasons.
     */
    virtual void changeParams(const QString &Param1, const QString &Param2, const QString &Param3)
    {
        toResult::changeParams(Param1, Param2, Param3);
    }
private slots:
    void poll(void);
    void changeSelected(void);
};

#endif
