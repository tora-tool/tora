/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TORESULTSTORAGE_H
#define TORESULTSTORAGE_H

#include "config.h"
#include "tobackground.h"
#include "toresultview.h"

#include <QSplitter>
#include <QPaintEvent>

class toNoBlockQuery;

class toStorageExtent : public QWidget
{
    Q_OBJECT;

public:
    struct extentName
    {
        QString Owner;
        QString Table;
        QString Partition;
        int Size;

        extentName(void)
        {
            Size = 0;
        }
        extentName(const QString &owner, const QString &table, const QString &partition, int size);
        bool operator < (const extentName &) const;
        bool operator == (const extentName &) const;
    };

struct extentTotal : public extentName
    {
        int Extents;
        extentTotal(const QString &owner, const QString &table, const QString &partition, int size)
                : extentName(owner, table, partition, size)
        {
            Extents = 1;
        }
    };

struct extent : public extentName
    {
        int File;
        int Block;

        extent(void)
        {
            File = Block = 0;
        }
        extent(const QString &owner, const QString &table, const QString &partition,
               int file, int block, int size);
        bool operator < (const extent &) const;
        bool operator == (const extent &) const;
    };

private:
    std::list<extent> Extents;
    extentName Highlight;
    QString Tablespace;

    std::map<int, int> FileOffset;
    int Total;
public:
    toStorageExtent(QWidget *parent, const char *name = NULL);
    void highlight(const QString &owner, const QString &table, const QString &partition);

    void setTablespace(const QString &tablespace);
    void setFile(const QString &tablespace, int file);

    std::list<extentTotal> objects(void);
protected:
    virtual void paintEvent(QPaintEvent *);
};

class toResultTableView;

class toResultExtent : public QSplitter, public toResult
{
    Q_OBJECT;

    toStorageExtent   *Graph;
    toResultTableView *List;
public:
    toResultExtent(QWidget *parent, const char *name = NULL);

    /** Support Oracle
     */
    virtual bool canHandle(toConnection &conn);
public slots:
    /** Perform a query.
     * @param sql Execute an SQL statement.
     * @param params Parameters needed as input to execute statement.
     */
    virtual void query(const QString &sql, const toQList &params);
};

class toResultStorage : public toResultView
{
    Q_OBJECT;

    bool ShowCoalesced;
    QString Unit;
    toBackground Poll;

    toNoBlockQuery *Tablespaces;
    toNoBlockQuery *Files;

    bool OnlyFiles;
    bool AvailableGraph;
    std::list<QString> TablespaceValues;
    std::list<QString> FileValues;

    QString CurrentSpace;
    QString CurrentFile;

    void saveSelected(void);
    void updateList(void);
public:
    toResultStorage(bool availableGraph, QWidget *parent, const char *name = NULL);
    ~toResultStorage();

    void showCoalesced(bool shw)
    {
        ShowCoalesced = shw;
    }

    void setOnlyFiles(bool only);
    bool onlyFiles(void) const
    {
        return OnlyFiles;
    }

    QString currentTablespace(void);
    QString currentFilename(void);
    /** Support Oracle
     */
    virtual bool canHandle(toConnection &conn);
public slots:
    virtual void query(void);
private slots:
    void poll(void);
};

#endif
