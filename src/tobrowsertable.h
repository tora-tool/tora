
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
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
