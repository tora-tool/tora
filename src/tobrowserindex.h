/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOBROWSERINDEX_H
#define TOBROWSERINDEX_H

#include "config.h"
#include "ui_tobrowserindexui.h"
#include "toextract.h"
#include "totool.h"

#include <map>
#include <list>

class toConnection;

class toBrowserIndex : public QDialog, public Ui::toBrowserIndexUI, public toConnectionWidget
{
private:
    Q_OBJECT

    toExtract Extractor;

    std::map<QString, std::list<QString> > OriginalDescription;
    std::map<QString, std::list<QString> > NewDescription;

    QString Owner;
    QString Table;
    QString Current;
    std::map<QString, QString> IndexType;
    std::map<QString, QString> IndexCols;

    QString table(void);

    toBrowserIndex(toConnection &conn, const QString &owner, const QString &table, const QString &index, QWidget *parent, const char *name = NULL);

    void registerIndex(const QString &type, const QString &ltype, std::list<QString>::iterator beg, std::list<QString>::iterator end);

    void describeTable(const QString &table);
    void saveChanges();

    QString sql(void);
private:
    void execute();
public:
    static void modifyIndex(toConnection &conn, const QString &owner, const QString &table, QWidget *parent, const QString &index = QString::null);
    static void addIndex(toConnection &conn, const QString &owner, const QString &table, QWidget *parent);
public slots:
    virtual void displaySQL();
    virtual void addIndex();
    virtual void delIndex();
    virtual void changeIndex();
    virtual void changeTable(const QString &table);
};

#endif
