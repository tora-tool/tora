/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTTABLEVIEWEDIT_H
#define TORESULTTABLEVIEWEDIT_H

#include "config.h"
#include "toresulttableview.h"
#include "toqvalue.h"
#include "tosql.h"
#include "toresult.h"
#include "toconnection.h"
#include "toresultmodel.h"

#include <QObject>
#include <QAbstractTableModel>
#include <QTableView>
#include <QModelIndex>
#include <QList>
#include <QHeaderView>
#include <QMenu>


class toResultTableViewEdit : public toResultTableView
{
    Q_OBJECT;

    enum ChangeKind
    {
        Add,
        Delete,
        Update
    };

    struct ChangeSet
    {
        ChangeKind         kind;         /* sql change mode */
        QString            columnName;   /* column name */
        int                column;       /* the real column number
                                          * after adjusting for
                                          * numbercolumn */
        toQValue           newValue;     /* data after the change */
        toResultModel::Row row;          /* data before the change */
    };

    // keep a history of changes to commit.
    // this is a fifo -- don't sort or insert. just append.
    QList<struct ChangeSet> Changes;

    QString Owner;
    QString Table;

    void commitDelete(ChangeSet &change, toConnection &conn);
    void commitAdd(ChangeSet &change, toConnection &conn);
    void commitUpdate(ChangeSet &change, toConnection &conn);

public:
    /**
     * Creates a new tableview for editing data
     *
     * @param readable use readable headers
     * @param numberColumn display vertical header
     * @param parent qobject parent
     * @param name sets objectName property
     */
    toResultTableViewEdit(bool readable,
                          bool numberColumn,
                          QWidget *parent,
                          const char *name = 0);
    virtual ~toResultTableViewEdit(void);


    /**
     * Reimplemented to create query and new model.
     *
     */
    virtual void query(const QString &, const toQList &params);


    /**
     * Reimplemented to create query and new model.
     *
     */
    void query(const QString &sql)
    {
        toQList p;
        query(sql, p);
    }


    /**
     * True if data has been modified.
     *
     */
    bool changed(void)
    {
        return Changes.size() > 0;
    }


signals:
    /**
     * Parameter is true after changes, false after save or load.
     *
     */
    void changed(bool edit);


public slots:
    /**
     * Append change to Changes
     *
     */
    void recordChange(const QModelIndex &,
                      const toQValue &,
                      const toResultModel::Row &);


    /**
     * Append a new row to Changes
     *
     */
    void recordAdd(const toResultModel::Row &);


    /**
     * Record a deletion in Changes
     *
     */
    void recordDelete(const toResultModel::Row &);


    /**
     * Calls Model to add new record.
     *
     */
    void addRecord(void);


    /**
     * Calls Model to add new record.
     *
     */
    void duplicateRecord(void);


    /**
     * Calls Model to delete current record
     *
     */
    void deleteRecord(void);


    /**
     * Writes cached changes to database.
     *
     * @param status Should display a status message.
     * @return success
     */
    bool commitChanges(bool status = true);


    /**
     * Handle connection toolbar's commit and rollback.
     *
     * @param conn Connection that is commited.
     * @param cmt true for commit, false for rollback
     */
    void commitChanges(toConnection &conn, bool cmt);


    /**
     * Clears Changes
     *
     */
    void revertChanges(void);


    /**
     * Handle signal from model when rows are added
     *
     */
    void handleNewRows(const QModelIndex &parent, int start, int end);
};


#endif
