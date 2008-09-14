/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTLABEL_H
#define TORESULTLABEL_H

#include "config.h"
#include "tobackground.h"
#include "toresult.h"

#include <qlabel.h>

class toNoBlockQuery;
class toSQL;

/** This widget displays the result of a query where each item in the stream
 * is added to a label separated by an optional string.
 */

class toResultLabel : public QLabel, public toResult
{
    Q_OBJECT

    toNoBlockQuery *Query;
    toBackground Poll;

    QString Separator;

public:
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultLabel(QWidget *parent, const char *name = NULL);
    /** Destruct object
     */
    ~toResultLabel();

    /** Reimplemented for internal reasons.
     */
    virtual void query(const QString &sql, const toQList &param);

    /** Handle any connection by default
     */
    virtual bool canHandle(toConnection &)
    {
        return true;
    }

    /** Get separator string to use between result.
     */
    const QString &separator(void) const
    {
        return Separator;
    }
    /** Set separator string to use between result.
     */
    void setSeparator(const QString &sep)
    {
        Separator = sep;
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
};

#endif
