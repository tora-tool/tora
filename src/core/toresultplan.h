
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

#ifndef TORESULTPLAN_H
#define TORESULTPLAN_H


#include "core/toresultview.h"
//#include "core/tosqlparse.h"

class toEventQuery;
class toResultCombo;

/** This widget displays the execution plan of a statement. The statement
 * is identified by the first parameter which should be the address as gotten
 * from the @ref toSQLToAddress function.
 */

class toResultPlan : public toResultView
{
    Q_OBJECT;

public:
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultPlan(QWidget *parent, const char *name = NULL);

    /** Destruct object
     */
    ~toResultPlan() {};

    /** Reimplemented for internal reasons. If you prepend "SAVED:" a saved plan is read
     * with the identified_by set to the string following the initial "SAVED:" string.
     * If you prepend SGA: the rest is interpreted as cursor address:hash_value in V$SQL_PLAN.
     */
    virtual void query(const QString &sql, toQueryParams const& param);

    /** Support Oracle
     */
    virtual bool canHandle(toConnection &conn);

private:
    QString Ident;
    std::map <QString, toTreeWidgetItem *> Parents;
    std::map <QString, toTreeWidgetItem *> Last;
    std::list<QString> Statements;
    toTreeWidgetItem *TopItem;
    toTreeWidgetItem *LastTop;
    bool Explaining; // true when explaining the query, false when fetching the plan
    QPointer<toEventQuery> Query;
    QString Schema;
    toResultCombo *CursorChildSel; //Used for Oracle for V$SQL_PLAN to choose plans among cursor children
    void checkException(const QString &);
    void oracleSetup(void);

    //void oracleNext();
    // TODO: toSQLParse disabled void addStatements(std::list<toSQLParse::statement> &stats);
    void connectSlotsAndStart();

private slots:
    void slotPoll();
    void slotQueryDone();
    void slotChildComboReady();
    void slotChildComboChanged(int NewIndex);
    void slotErrorHanler(toEventQuery*, toConnection::exception  const &);
};

#endif
