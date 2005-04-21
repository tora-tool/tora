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

#ifndef TOBROWSERCONSTRAINT_H
#define TOBROWSERCONSTRAINT_H

#include "config.h"
#include "tobrowserconstraintui.h"
#include "toextract.h"
#include "totool.h"

#include <map>
#include <list>

class toBrowserConstraint : public toBrowserConstraintUI, public toConnectionWidget
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
