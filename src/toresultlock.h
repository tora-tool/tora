/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTLOCK_H
#define TORESULTLOCK_H

#include "config.h"

#include <map>

#include "tobackground.h"
#include "toresultview.h"

class toNoBlockQuery;

class toResultLock : public toResultView
{
    Q_OBJECT

    toBackground Poll;
    toNoBlockQuery *Query;
    toTreeWidgetItem *LastItem;
    std::map<int, bool> Checked;
public:
    toResultLock(QWidget *parent, const char *name = NULL);
    ~toResultLock();

    virtual void query(const QString &sql, const toQList &param);

    virtual void query(const QString &sql)
    {
        toQList p;
        query(sql, p);
    }
    /** Support Oracle
     */
    virtual bool canHandle(toConnection &conn);
private slots:
    void poll(void);
};

#endif
