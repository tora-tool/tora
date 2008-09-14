/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTCOLS_H
#define TORESULTCOLS_H

#include "config.h"
#include "toconnection.h"
#include "toresulttableview.h"

class QCheckBox;
class QLabel;
class toResultColsComment;
class QScrollArea;


/**
 * This widget displays information about the returned columns of an
 * object specified by the first and second parameter in the
 * query. The sql is not used in the query.
 *
 */
class toResultCols : public QWidget, public toResult
{
    Q_OBJECT;

    QLabel              *Title;
    QLabel              *Comment;
    toResultColsComment *EditComment;
    QCheckBox           *Edit;
    toResultTableView   *Columns;
    bool                 Header;
    QScrollArea         *ColumnComments; /* container for column
                                          * comment widgets */
    QString              TableName;      /* quoted owner and table */

public:
    /**
     * Create the widget.
     *
     * @param parent Parent widget.
     * @param name Name of widget.
     * @param f Widget flags.
     */
    toResultCols(QWidget *parent, const char *name = NULL, Qt::WFlags f = 0);


    /**
     * Reimplemented from toResult
     */
    virtual void query(const QString &sql, const toQList &param);


    /**
     * Handle any connection by default
     *
     */
    virtual bool canHandle(toConnection &)
    {
        return true;
    }


    /**
     * Display header of column view
     *
     */
    void displayHeader(bool disp);


private slots:
    void editComment(bool val);
};

#endif
