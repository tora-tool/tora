/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOPROJECT_H
#define TOPROJECT_H

#include "config.h"
#include "totemplate.h"

#include <map>

#include <QString>
#include <QAction>


class toProjectTemplateItem : public toTemplateItem
{
    QString Filename;
    int Order;
    void setup(const QString &name, bool open);
    int order(bool asc);

public:
    toProjectTemplateItem(toTemplateItem *item,
                          toTreeWidgetItem *after,
                          QString name,
                          bool open = true);
    toProjectTemplateItem(toTreeWidget *item,
                          QString name = QString::null,
                          bool open = true);

    virtual QString key(int col, bool asc) const;

    const QString &filename(void) const
    {
        return Filename;
    }
    void setFilename(const QString &file);
    bool project(void);
    virtual void selected(void);
    toProjectTemplateItem *previousSibling(void);

    virtual QWidget *selectedWidget(QWidget *parent);
};

class toProject : public QWidget
{
    Q_OBJECT;

    toProjectTemplateItem *Root;
    QAction               *DelFile;
    toTreeWidget            *Project;

    std::map<toTreeWidgetItem *, toProjectTemplateItem *> ItemMap;

    void update(toProjectTemplateItem *sourceparent, toResultViewItem *parent);
    QString generateSQL(toProjectTemplateItem *item);

public:
    toProject(toProjectTemplateItem *top, QWidget *parent);
    ~toProject();

    void selectItem(toProjectTemplateItem *);

public slots:
    void update(void);
    void addFile(void);
    void delFile(void);
    void newProject(void);
    void saveProject(void);
    void generateSQL(void);
    void selectionChanged(void);
    void moveDown(void);
    void moveUp(void);
};


class toProjectTemplate : public QObject, public toTemplateProvider
{
    Q_OBJECT;

    toProjectTemplateItem *Root;
    std::map<QString, QString> Import;

    QAction *AddFile;
    QAction *DelFile;

    toProject *Details;

public:
    toProjectTemplate(void)
            : QObject(0), toTemplateProvider("project")
    {
        setObjectName("projecttemplate");
        Details = NULL;
    }

    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);

    virtual void insertItems(toTreeWidget *parent, QToolBar *toolbar);
    virtual void removeItems(toTreeWidgetItem *item);

    virtual toProject *selectedWidget(QWidget *parent);

    virtual toProjectTemplateItem *root(void)
    {
        return Root;
    }

    friend class toProject;

public slots:
    void addFile();
    void delFile();
    void changeItem(toTreeWidgetItem *item);
};

#endif
