/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOCURRENT_H
#define TOCURRENT_H

#include "config.h"
#include "totool.h"
#include "tosql.h"
#include "tobackground.h"

#include "toresultview.h"

class QTabWidget;
class toNoBlockQuery;
class toResultParam;
class toResultStats;
class toResultView;
class toResultTableView;

class toCurrent : public toToolWidget
{
    Q_OBJECT;

    struct update
    {
        bool IsRole;

        toTreeWidgetItem *Parent;
        QString Type;
        QString SQL;
        QString Role;

        update()
        {
            IsRole = false;
            Parent = NULL;
        }

        update(bool isrole,
               toTreeWidgetItem *parent,
               const QString &type,
               const QString &sql,
               const QString &role)
                : IsRole(isrole),
                Parent(parent),
                Type(type),
                SQL(sql),
                Role(role)
        {
        }
    };

    std::list<update> Updates;

    QTabWidget        *Tabs;
    toResultTableView *Version;
    toListView        *Grants;
    toResultTableView *ResourceLimit;
    toResultParam     *Parameters;
    toResultStats     *Statistics;
    toBackground       Poll;

    update CurrentUpdate;
    toNoBlockQuery *Query;

    virtual void addList(bool isrole,
                         toTreeWidgetItem *parent,
                         const QString &typ,
                         const toSQL &sql,
                         const QString &role = QString::null);

protected:
    void closeEvent(QCloseEvent *event);

public:
    toCurrent(QWidget *parent, toConnection &connection);
    virtual ~toCurrent();

public slots:
    void refresh(void);
    void poll(void);
};

#endif
