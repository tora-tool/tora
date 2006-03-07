/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#ifndef TOBROWSERINDEX_H
#define TOBROWSERINDEX_H

#include "config.h"
#include "tobrowserindexui.h"
#include "toextract.h"
#include "totool.h"

#include <map>
#include <list>

class toConnection;

class toBrowserIndex : public toBrowserIndexUI, public toConnectionWidget
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
