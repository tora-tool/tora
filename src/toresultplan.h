/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTPLAN_H
#define TORESULTPLAN_H

#include "config.h"
#include "tobackground.h"
#include "tosqlparse.h"
#include "toresultview.h"

#include <map>
#include <stdio.h>

class toNoBlockQuery;

/** This widget displays the execution plan of a statement. The statement
 * is identified by the first parameter which should be the address as gotten
 * from the @ref toSQLToAddress function.
 */

class toResultPlan : public toResultView
{
    Q_OBJECT

    QString Ident;
    std::map <QString, toTreeWidgetItem *> Parents;
    std::map <QString, toTreeWidgetItem *> Last;
    std::list<QString> Statements;
    toTreeWidgetItem *TopItem;
    toTreeWidgetItem *LastTop;
    bool Reading;
    toNoBlockQuery *Query;
    toBackground Poll;
    QString User;
    void checkException(const QString &);
    void oracleSetup(void);

    void oracleNext(void);
    void addStatements(std::list<toSQLParse::statement> &stats);
public:
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultPlan(QWidget *parent, const char *name = NULL);

    /** Destruct object
     */
    ~toResultPlan();

    /** Reimplemented for internal reasons. If you prepend "SAVED:" a saved plan is read
     * with the identified_by set to the string following the initial "SAVED:" string.
     */
    virtual void query(const QString &sql, const toQList &param);
    /** Reimplemented for internal reasons.
     */
    void query(const QString &sql)
    {
        toQList p;
        query(sql, p);
    }
    /** Support Oracle
     */
    virtual bool canHandle(toConnection &conn);
private slots:
    void poll();
};

#endif
