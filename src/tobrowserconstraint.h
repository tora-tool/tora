/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOBROWSERCONSTRAINT_H
#define TOBROWSERCONSTRAINT_H

#include "config.h"
#include "ui_tobrowserconstraintui.h"
#include "toextract.h"
#include "totool.h"

#include <map>
#include <list>

class toBrowserConstraint : public QDialog, public Ui::toBrowserConstraintUI, public toConnectionWidget
{
private:
    Q_OBJECT

    QString Owner;
    QString Table;

    toExtract Extractor;

    std::map<QString, std::list<QString> > OriginalDescription;
    std::map<QString, std::list<QString> > NewDescription;

    QString Current;
    QString BeforeUnquote;  // Hack for not incurring differences with weird MySQL quoting
    QString AfterUnquote;
    std::map<QString, QString> Constraints;

    toBrowserConstraint(toConnection &conn, const QString &owner, const QString &table, QWidget *parent, const char *name = NULL);

    QString table();
    void parseConstraint(const QString &definition);

    void saveChanges();
    void describeTable(const QString &table);

    QString sql(void);
public:
    static void modifyConstraint(toConnection &conn, const QString &owner, const QString &table, QWidget *parent);
public slots:
    virtual void displaySQL();
    virtual void addConstraint();
    virtual void delConstraint();
    virtual void changeConstraint();
    virtual void changeRefered();
    virtual void changeType(int type);
    virtual void changeTable(const QString &table);
};

#endif
