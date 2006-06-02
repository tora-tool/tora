/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#ifndef TORESULTCOLS_H
#define TORESULTCOLS_H

#include "config.h"
#include "toconnection.h"
#include "toresultview.h"
#include "toresultitem.h"

#include <qvbox.h>
#include <qlineedit.h>

class QCheckBox;
class QLabel;
class toResultColsItem;
class toResultLong;
class toResultColsComment;

/** This widget displays information about the returned columns of an object
 * specified by the first and second parameter in the query. The sql is not
 * used in the query.
 */

class toResultCols : public QVBox, public toResult
{
    Q_OBJECT

    class resultCols;
    class resultColsEdit;
    friend class resultCols;
    friend class resultColsEdit;
class resultColsEdit : public toResultItem
    {
        QString Table;
        bool Cached;
    public:
        resultColsEdit(QWidget *parent)
                : toResultItem(1, false, parent)
        { }
        virtual QWidget *createValue(QWidget *parent);
        virtual void setValue(QWidget *widget, const QString &title, const QString &value);
        void describe(toQDescList &desc, const QString &table, bool cached);
        friend class resultCols;
    };
class resultCols : public toListView
    {
        resultColsEdit *Edit;
        QString Owner;
        QString Name;
    public:
        resultCols(QWidget *parent, const char *name = NULL);
        void editComment(bool val);
        void describe(toQDescList &desc);
        void query(const toConnection::objectName &, bool);
        void query(const QString &table, const QString &owner, const QString &name);
        friend class toResultCols;
    };

    QLabel *Title;
    QLabel *Comment;
    toResultColsComment *EditComment;
    QCheckBox *Edit;
    resultCols *Columns;
    toResultLong *MySQLColumns;
    bool Header;

    virtual void query(const QString &sql, const toQList &param, bool nocache);
public:
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     * @param f Widget flags.
     */
    toResultCols(QWidget *parent, const char *name = NULL, WFlags f = 0);
    /** Reimplemented for internal reasons.
     */
    virtual void query(const QString &sql, const toQList &param)
    {
        query(sql, param, false);
    }
    /** Handle any connection by default
     */
    virtual bool canHandle(toConnection &)
    {
        return true;
    }

    /** Display header of column view
     */
    void displayHeader(bool disp);

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

    friend class toResultColsItem;
public slots:
    /** Erase last parameters
     */
    virtual void clearParams(void)
    {
        toResult::clearParams();
    }
    /** Reimplemented for internal reasons.
     */
    virtual void refresh(void)
    {
        query(sql(), params(), true);
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
    void editComment(bool val);
};

#endif
