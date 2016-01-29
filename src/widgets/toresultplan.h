
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

#ifndef TORESULTPLAN_H
#define TORESULTPLAN_H


#include "widgets/toresultview.h"
#include "widgets/topushbutton.h"
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
        Q_ENUMS(ExplainTypeEnum);
    public:

        enum ExplainTypeEnum
        {
            Explain   = 10,
			XPLAN = 20,
        };


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
        virtual bool canHandle(const toConnection &conn);

    protected:
        void showEvent(QShowEvent * event);
        void hideEvent(QHideEvent * event);

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

        QSharedPointer<toConnectionSubLoan> LockedConnection;

        void checkException(const QString &);
        void oracleSetup(void);

        // TODO: toSQLParse disabled void addStatements(std::list<toSQLParse::statement> &stats);
        void connectSlotsAndStart();

        void cleanup();
    private slots:
        void slotPoll();
        void slotQueryDone();
        void slotChildComboReady();
        void slotChildComboChanged(int NewIndex);
        void slotErrorHanler(toEventQuery*, toConnection::exception  const &);
};

/**
 * Subclass toToggleButton and iterate over values of HighlighterTypeEnum
 */
class toExplainTypeButton : public toToggleButton
{
        Q_OBJECT;
    public:
        toExplainTypeButton(QWidget *parent, const char *name = 0);
        toExplainTypeButton();
};

class toPlanTreeItem
{
public:
    explicit toPlanTreeItem(const QList<QVariant> &data, toPlanTreeItem *parentItem = 0);
    ~toPlanTreeItem();

    void appendChild(toPlanTreeItem *child);

    toPlanTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    toPlanTreeItem *parentItem();

    QString id() const;
private:
    QList<toPlanTreeItem*> m_childItems;
    QList<QVariant> m_itemData;
    toPlanTreeItem *m_parentItem;
};

class toResultPlanNewModel : public QAbstractItemModel
{
        Q_OBJECT
		friend class toResultPlanNew;
    public:
		toResultPlanNewModel(toEventQuery *query, QObject *parent = 0);
		virtual ~toResultPlanNewModel();

		virtual QVariant data(const QModelIndex &index, int role) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

		virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
		virtual QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
		virtual QModelIndex parent(const QModelIndex &index) const override;
		virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

        QModelIndex rootIndex() const;

    signals:
		/**
		 * Emitted when query is finished.
		 */
		void queryDone(toEventQuery*);

    private slots:
		void slotPoll(toEventQuery*);
		void slotQueryDone(toEventQuery*);

    protected:
        void cleanup();
        toEventQuery *Query;
        toPlanTreeItem *rootItem, *sqlidItem;
        QList<toPlanTreeItem*> stack;

        struct HeaderDesc
        {
            QString           name;        /* column name */
            Qt::Alignment     align;       /* alignment */
        };

        typedef QList<HeaderDesc> HeaderList;
        HeaderList Headers;
        bool HeadersRead;
};

class toResultPlanNew : public QWidget//, public toResult
{
	// Note: it thiss class also subclasses toResult, then QTabWidget will NOT show it
        Q_OBJECT
        //Q_ENUMS(ExplainTypeEnum);
    public:
        explicit toResultPlanNew(QWidget *parent = 0);

        void queryCursorPlan(toQueryParams const& params);
        void queryPlanTable(toQueryParams const& params);
        void queryXPlan(toQueryParams const& params);
        bool handled(void) { return true; };
    private slots:
		void queryDone(toEventQuery*);
		void explainDone(toEventQuery*);
    private:
        QPointer<toResultPlanNewModel> model;
        QTreeView *planTreeView;
        QPlainTextEdit *planTreeText;
        // used for sequence explain plan for, select * from plan_table
        QSharedPointer<toConnectionSubLoan> LockedConnection;
        bool explaining;
        QString planId;
        QPointer<toEventQuery> explainQuery;
};

// this one will be usually parented by QStatusBar
typedef Loki::SingletonHolder<toExplainTypeButton, Loki::CreateUsingNew, Loki::NoDestroy> toExplainTypeButtonSingle;

#endif
