/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
