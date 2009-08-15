
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

#ifndef TOSQLEDIT_H
#define TOSQLEDIT_H

#include "config.h"
#include "totemplate.h"
#include "totool.h"

#include <QString>

class QComboBox;
class QLineEdit;
class toTreeWidget;
class QAction;
class toMarkedText;
class toWorksheet;

class toSQLEdit : public toToolWidget
{
    Q_OBJECT;

    toTreeWidget *Statements;
    QLineEdit    *Name;
    toMarkedText *Description;
    QComboBox    *Version;
    toWorksheet  *Editor;
    QAction      *TrashButton;
    QAction      *CommitButton;
    QString       LastVersion;
    QString       Filename;

    bool splitVersion(const QString &split,
                      QString &provider,
                      QString &version);
    void connectList(bool connect);

protected:
    void updateStatements(const QString &def = QString::null);

    bool checkStore(bool);
    virtual bool close(bool del);

    void selectionChanged(const QString &ver);
    void changeSQL(const QString &name, const QString &ver);

public:
    toSQLEdit(QWidget *parent, toConnection &connection);
    virtual ~toSQLEdit();
    void commitChanges(bool);

public slots:
    void loadSQL(void);
    void saveSQL(void);
    void deleteVersion(void);
    void selectionChanged(void);
    void changeVersion(const QString &);
    void commitChanges(void)
    {
        commitChanges(true);
    }
    void editSQL(const QString &);
    void newSQL();
};


class toSQLTemplateItem : public toTemplateItem
{
    QString Name;

public:
    toSQLTemplateItem(toTreeWidget *parent);
    toSQLTemplateItem(toSQLTemplateItem *parent,
                      const QString &name);
    virtual void expand(void);
    virtual void collapse(void);
    virtual QWidget *selectedWidget(QWidget *parent);

    virtual QString allText(int col) const;
};


class toSQLTemplate : public toTemplateProvider
{

public:
    toSQLTemplate()
            : toTemplateProvider("SQL Dictionary") { }
    virtual void insertItems(toTreeWidget *parent, QToolBar *)
    {
        new toSQLTemplateItem(parent);
    }
};

#endif
