/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOBROWSERTABLE_H
#define TOBROWSERTABLE_H

#include "config.h"
#include "ui_tobrowsertableui.h"
#include "toextract.h"
#include "totool.h"

#include <list>
#include <QResizeEvent>

class toConnection;
class toDatatype;

class QGridLayout;
class QLineEdit;

class toBrowserTable : public QDialog,
            public Ui::toBrowserTableUI,
            public toConnectionWidget
{

private:
    Q_OBJECT;

    QString Owner;
    QString Table;

    toExtract Extractor;
    std::list<toExtract::columnInfo> Columns;

    std::list<QString> OriginalDescription;

    QWidget *ColumnGrid;
    QGridLayout *ColumnGridLayout;

    int ColumnNumber;

    std::list<QLineEdit *> ColumnNames;
    std::list<toDatatype *> Datatypes;
    std::list<QLineEdit *> Extra;

    toBrowserTable(toConnection &conn,
                   const QString &owner,
                   const QString &table,
                   QWidget *parent,
                   const char *name = NULL);

    void addParameters(std::list<QString> &migrateTable,
                       const std::list<QString> &ctx,
                       const QString &type,
                       const QString &data);
    QString sql(void);

protected:
    virtual void resizeEvent(QResizeEvent *e);

public:
    static void editTable(toConnection &conn,
                          const QString &owner,
                          const QString &table,
                          QWidget *parent);
    static void newTable(toConnection &conn,
                         const QString &owner,
                         QWidget *parent)
    {
        editTable(conn, owner, QString::null, parent);
    }

public slots:
    virtual void displaySQL();
    virtual void addColumn();
    virtual void removeColumn();
    virtual void toggleCustom(bool);

private:
    bool UglyFlag;

private slots:
    virtual void uglyWorkaround();
};

#endif
