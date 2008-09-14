/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTDEPEND_H
#define TORESULTDEPEND_H

#include "config.h"
#include "tobackground.h"
#include "toresultview.h"

class toNoBlockQuery;

/** This widget displays information about the dependencies of an object
 * specified by the first and second parameter in the query. The sql is not
 * used in the query. It will also recurs through all dependencies of the
 * objects depended on.
 */

class toResultDepend : public toResultView
{
    Q_OBJECT

    /** Check if an object already exists.
     * @param owner Owner of object.
     * @param name Name of object.
     * @return True if object exists.
     */
    bool exists(const QString &owner, const QString &name);

    toNoBlockQuery *Query;
    toBackground Poll;
    toTreeWidgetItem *Current;
public:
    /** Create the widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toResultDepend(QWidget *parent, const char *name = NULL);

    /** Object destructor.
     */
    ~toResultDepend();
    /** Reimplemented for internal reasons.
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
public slots:
    void poll(void);
};

#endif
