/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
class QToolButton;
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
    QToolButton  *TrashButton;
    QToolButton  *CommitButton;
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
