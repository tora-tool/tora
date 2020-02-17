
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "templates/totemplate.h"

#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconfiguration.h"
#include "core/toconnection.h"
#include "widgets/toscintilla.h"
#include "core/toeventquery.h"
#include "core/totool.h"
#include "tools/toresultview.h"
#include "widgets/tohelp.h"

#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QTimer>

#include <QtGui/QHideEvent>
#include <QtGui/QShowEvent>
#include <QTextEdit>
#include <QtCore/QFileInfo>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QToolBar>

#include "icons/totemplate.xpm"


typedef QMap<QString, QVariant> TemplatesMap;
typedef QMapIterator<QString, QVariant> TemplatesMapIterator;

static TemplatesMap DefaultText(void)
{
    TemplatesMap def;

#if !defined(TO_NO_ORACLE)
    def["PL/SQL Functions"] = QVariant(":/templates/sqlfunctions.tpl");
    def["Optimizer Hints"] = QVariant(":/templates/hints.tpl");
    def["Log4PL/SQL"] = QVariant(":/templates/log4plsql.tpl");
#endif

    return def;
}

QVariant ToConfiguration::Template::defaultValue(int option) const
{
    switch (option)
    {
        case TemplateMap:
            {
                QMap <QString, QVariant> retval;
                return QVariant(retval);
            }
        default:
            Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Template un-registered enum value: %1").arg(option)));
            return QVariant();
    }
};

toTemplateEdit::toTemplateEdit(std::map<QString, QString> &pairs,
                               QWidget *parent, const char *name) :
    QDialog(parent),//, name, true, Qt::WStyle_Maximize),
    toHelpContext(QString::fromLatin1("template.html#editor")),
    TemplateMap(pairs)
{
    setupUi(this);
    toHelp::connectDialog(this);
    LastTemplate = TemplateMap.end();
    updateFromMap();
    Description->setWrapMode(QsciScintilla::WrapWord);
}

void toTemplateEdit::connectList(bool conn)
{
    if (conn)
        connect(Templates, SIGNAL(selectionChanged()), this,
                SLOT(changeSelection()));
    else
        disconnect(Templates, SIGNAL(selectionChanged()), this,
                   SLOT(changeSelection()));
}

toTreeWidgetItem *toTemplateEdit::findLast(void)
{
    QString name = (*LastTemplate).first;
    return Templates->toFindItem(name);
}
void toTemplateEdit::allocateItem(void)
{
    QStringList lst = Name->text().split(QString(":"));
    int li = 0;
    toTreeWidgetItem *parent = NULL;
    for (toTreeWidgetItem *item = Templates->firstChild(); item && li
            < lst.count();)
    {
        if (item->text(0) == lst[li])
        {
            li++;
            parent = item;
            item = item->firstChild();
        }
        else
            item = item->nextSibling();
    }
    while (li < lst.count())
    {
        if (parent)
            parent = new toTreeWidgetItem(parent, lst[li]);
        else
            parent = new toTreeWidgetItem(Templates, lst[li]);
        li++;
    }
}

bool toTemplateEdit::clearUnused(toTreeWidgetItem *first, const QString &pre)
{
    bool ret = false;
    while (first)
    {
        toTreeWidgetItem *delitem = first;
        QString str = pre;
        if (!str.isEmpty())
            str += ":";
        str += first->text(0).toLatin1();
        if (first->firstChild() && clearUnused(first->firstChild(), str))
            delitem = NULL;
        if (delitem && TemplateMap.find(str) != TemplateMap.end())
            delitem = NULL;
        first = first->nextSibling();
        if (!delitem)
            ret = true;
        else
            delete delitem;
    }
    return ret;
}

void toTemplateEdit::updateFromMap(void)
{
    try
    {
        while (Templates->firstChild())
            delete Templates->firstChild();
        toTreeWidgetItem *last = NULL;
        int lastLevel = 0;
        QStringList lstCtx;
        for (std::map<QString, QString>::iterator i = TemplateMap.begin(); i != TemplateMap.end(); i++)
        {
            QStringList ctx = (*i).first.split(":");
            if (last)
            {
                while (last && lastLevel >= int(ctx.count()))
                {
                    last = last->parent();
                    lastLevel--;
                }
                while (last && lastLevel >= 0 && !Utils::toCompareLists(lstCtx, ctx, (unsigned int)lastLevel))
                {
                    last = last->parent();
                    lastLevel--;
                }
            }
            if (lastLevel < 0)
                throw qApp->translate("toTemplateEdit", "Internal error, lastLevel < 0");
            while (lastLevel < int(ctx.count()) - 1)
            {
                if (last)
                    last = new toTreeWidgetItem(last, ctx[lastLevel]);
                else
                    last = new toTreeWidgetItem(Templates, ctx[lastLevel]);
                last->setOpen(true);
                lastLevel++;
            }
            if (last)
                last = new toTreeWidgetItem(last, ctx[lastLevel]);
            else
                last = new toTreeWidgetItem(Templates, ctx[lastLevel]);
            last->setOpen(true);
            if (i == LastTemplate)
                last->setSelected(true);
            lstCtx = ctx;
            lastLevel++;
        }
    }
    catch (const QString &str)
    {
        Utils::toStatusMessage(str);
        reject();
    }
}

void toTemplateEdit::remove(void)
{
    if (LastTemplate != TemplateMap.end())
    {
        toTreeWidgetItem *item = findLast();
        TemplateMap.erase(LastTemplate);
        LastTemplate = TemplateMap.end();
        Name->setText(QString::null);
        Description->setText(QString::null);
        if (item)
        {
            connectList(false);
            clearUnused(Templates->firstChild(), "");
            connectList(true);
        }
    }
}

void toTemplateEdit::preview(void)
{
    Preview->setText(Description->text());
}

QString toTemplateEdit::name(toTreeWidgetItem *item)
{
    QString str = item->text(0);
    for (item = item->parent(); item; item = item->parent())
    {
        str.prepend(":");
        str.prepend(item->text(0));
    }
    return str;
}

void toTemplateEdit::newTemplate(void)
{
    changeSelection();
    LastTemplate = TemplateMap.end();
    Description->setText(QString::null);
    toTreeWidgetItem *item = Templates->selectedItem();
    if (item)
    {
        connectList(false);
        Templates->setSelected(item, false);
        connectList(true);
        item = item->parent();
    }
    QString str;
    if (item)
    {
        str = name(item);
        str += ":";
    }
    Name->setText(str);
}

void toTemplateEdit::changeSelection(void)
{
    if (LastTemplate != TemplateMap.end())
    {
        if (Name->text() != (*LastTemplate).first || Description->text() != (*LastTemplate).second)
        {
            TemplateMap.erase(LastTemplate);
            TemplateMap[Name->text()] = Description->text();
            allocateItem();
        }
    }
    else if (!Name->text().isEmpty())
    {
        TemplateMap[Name->text()] = Description->text();
        allocateItem();
    }
    LastTemplate = TemplateMap.end();

    toTreeWidgetItem *item = Templates->selectedItem();
    if (item)
    {
        QString str = name(item);
        LastTemplate = TemplateMap.find(str);
        if (LastTemplate != TemplateMap.end())
        {
            Name->setText((*LastTemplate).first);
            Description->setText((*LastTemplate).second);
            Preview->setText((*LastTemplate).second);
        }
        else
        {
            Name->setText(QString::null);
            Description->clear();
            Preview->setText(QString::null);
        }
    }
    else
        LastTemplate = TemplateMap.end();
    clearUnused(Templates->firstChild(), "");
}


toTemplateAddFile::toTemplateAddFile(QWidget *parent, const char *name)
    : QDialog(parent)
{

    if (name)
        setObjectName(name);

    setupUi(this);
    OkButton->setEnabled(false);
    toHelp::connectDialog(this);
}


void toTemplateAddFile::browse()
{
    QFileInfo file(Filename->text());
    QString filename = Utils::toOpenFilename(file.dir().path(), QString("*.tpl"), this);
    if (!filename.isEmpty())
        Filename->setText(filename);
}


void toTemplateAddFile::valid()
{
    if (Filename->text().isEmpty() || Root->text().isEmpty())
        OkButton->setEnabled(false);
    else
        OkButton->setEnabled(true);
}


toTemplateSetting::toTemplateSetting(toTool *tool, QWidget *parent, const char *name)
    : QWidget(parent)
    , toSettingTab("template.html#setup")
    , Tool(tool)
{

    setupUi(this);
    TemplatesMap def = DefaultText();

    TemplatesMap tMap = toConfigurationNewSingle::Instance().option(ToConfiguration::Template::TemplateMap).toMap();
    TemplatesMapIterator i(tMap);
    while (i.hasNext())
    {
        i.next();
        new toTreeWidgetItem(FileList, i.key(), i.value().toString());
        if (def.find(i.key()) != def.end())
            def.erase(def.find(i.key()));
    }
    TemplatesMapIterator j(def);
    while (j.hasNext())
    {
        j.next();
        new toTreeWidgetItem(FileList, j.key(), j.value().toString());
    }
    toSettingTab::loadSettings(this);
}


void toTemplateSetting::saveSetting(void)
{
    TemplatesMap m;
    TemplatesMap def = DefaultText();
    for (toTreeWidgetItem *item = FileList->firstChild(); item; item = item->nextSibling())
    {
        // save only user addons
        if (!DefaultText().contains(item->text(0)))
            m[item->text(0)] = item->text(1);
    }

    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Template::TemplateMap, m);
    toSettingTab::saveSettings(this);
}


void toTemplateSetting::addFile(void)
{
    toTemplateAddFile file(this);
    if (file.exec())
        new toTreeWidgetItem(FileList, file.Root->text(), file.Filename->text());
}


void toTemplateSetting::editFile(void)
{
    toTreeWidgetItem *item = FileList->selectedItem();
    if (item)
    {
        try
        {
            QString file = item->text(1);
            std::map<QString, QString> pairs;
            try
            {
                Utils::toLoadMap(file, pairs);
            }
            catch (...)
            {
                if (TOMessageBox::warning(this,
                                          qApp->translate("toTemplatePrefs", "Couldn't open file."),
                                          qApp->translate("toTemplatePrefs", "Couldn't open file. Start on new file?"),
                                          qApp->translate("toTemplatePrefs", "&Ok"),
                                          qApp->translate("toTemplatePrefs", "Cancel")) == 1)
                    return ;
            }
            toTemplateEdit edit(pairs, this);
            if (edit.exec())
            {
                edit.changeSelection();
                if (!Utils::toSaveMap(file, pairs))
                    throw qApp->translate("toTemplatePrefs", "Couldn't write file");
            }
        }
        catch (const QString &str)
        {
            TOMessageBox::warning(this,
                                  qApp->translate("toTemplatePrefs", "Couldn't open file"),
                                  str,
                                  qApp->translate("toTemplatePrefs", "&Ok"));
        }
    }
}


void toTemplateSetting::delFile(void)
{
    delete FileList->selectedItem();
}


class toTemplateTool : public toTool
{
        TODock *Dock;
        toTemplate *Window;
    protected:
        virtual const char **pictureXPM(void)
        {
            return const_cast<const char**>(totemplate_xpm);
        }
    public:
        toTemplateTool()
            : toTool(410, "Project Manager")
        {
            Dock = NULL;
            Window = NULL;
            toTemplateProvider::setToolKey(key());
        }
        virtual const char *menuItem()
        {
            return "Project Manager";
        }
        void toggleWindow()
        {
            if (!Dock || !Window)
            {
                Dock = Utils::toAllocDock(qApp->translate("toTemplateTool", "Template"), QString::null, *toolbarImage());
                // fixes warning from QMainWindow::saveState
                Dock->setObjectName("Template");
                Window = new toTemplate(Dock);
            }
            else if (Dock->isHidden())
            {
                Dock->show();
                Window->showResult(true);
            }
            else
            {
                Dock->hide();
                Window->showResult(false);
            }
        }
        virtual toToolWidget *toolWindow(QWidget *, toConnection &)
        {
            toggleWindow();
            return NULL;
        }
        void closeWindow()
        {
            Dock = NULL;
            Window = NULL;
            toTemplateProvider::setShown(false);
        }
        virtual QWidget *configurationTab(QWidget *parent)
        {
            return new toTemplateSetting(this, parent);
        }
        virtual bool canHandle(const toConnection &)
        {
            return true;
        }
        virtual void closeWindow(toConnection &connection) {};
    private:
        static ToConfiguration::Template s_templateConfig;
};

ToConfiguration::Template toTemplateTool::s_templateConfig;

static toTemplateTool TemplateTool;

void toTemplate::hideEvent(QHideEvent *e)
{
    toTemplateProvider::setShown(false);
    QWidget::hideEvent(e);
}

void toTemplate::showEvent(QShowEvent *e)
{
    toTemplateProvider::setShown(true);
    QWidget::showEvent(e);
}

QWidget *toTemplate::parentWidget(toTreeWidgetItem *item)
{
    try
    {
        return templateWidget(item)->frame();
    }
    catch (...)
    {
        return NULL;
    }
}

toTemplate *toTemplate::templateWidget(toTreeWidget *obj)
{
    QObject *lst = obj;
    while (lst)
    {
        toTemplate *tpl = dynamic_cast<toTemplate *>(lst);
        if (tpl)
            return tpl;
        lst = lst->parent();
    }
    throw tr("Not a toTemplate parent");
}

toTemplate *toTemplate::templateWidget(toTreeWidgetItem *item)
{
    return templateWidget(item->listView());
}

class toTemplateResult : public QWidget
{
        toTemplate *Template;
    public:
        toTemplateResult(TODock *parent, toTemplate *temp)
            : QWidget(parent), Template(temp)
        {
            QVBoxLayout *box = new QVBoxLayout;
            box->setSpacing(0);
            box->setContentsMargins(0, 0, 0, 0);
            setLayout(box);
        }

        virtual ~toTemplateResult()
        {
            Template->closeFrame();
        }
};

toTemplate::toTemplate(TODock *parent)
    : QWidget(parent), toHelpContext(QString::fromLatin1("template.html"))
{
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);

    Toolbar = Utils::toAllocBar(this, tr("Template Toolbar"));
    vbox->addWidget(Toolbar);

    List = new toListView(this);
    List->addColumn(tr("Template"));
    List->setRootIsDecorated(true);
    List->setSorting(0);
    List->setShowSortIndicator(false);
    List->setTreeStepSize(10);
    List->setSelectionMode(toTreeWidget::Single);
    List->setResizeMode(toTreeWidget::AllColumns);
    Result = Utils::toAllocDock(tr("Template result"),
                                QString::null,
                                *TemplateTool.toolbarImage());
    // fixes warning from QMainWindow::saveState
    Result->setObjectName("templateResult");
    vbox->addWidget(List);
    Frame = new toTemplateResult(Result, this);

    connect(List, SIGNAL(expanded(toTreeWidgetItem *)), this, SLOT(expand(toTreeWidgetItem *)));
    connect(List, SIGNAL(collapsed(toTreeWidgetItem *)), this, SLOT(collapse(toTreeWidgetItem *)));
    connect(List,
            SIGNAL(clicked(toTreeWidgetItem *)),
            this,
            SLOT(selected(toTreeWidgetItem *)));

    if (toTemplateProvider::Providers)
        for (std::list<toTemplateProvider *>::iterator i = toTemplateProvider::Providers->begin();
                i != toTemplateProvider::Providers->end();
                i++)
            (*i)->insertItems(List, Toolbar);

    Toolbar->addWidget(new Utils::toSpacer());

    WidgetExtra = NULL;
    setWidget(NULL);

    setFocusProxy(List);
    Utils::toAttachDock(parent, this, Qt::LeftDockWidgetArea);
    Utils::toAttachDock(Result, Frame, Qt::BottomDockWidgetArea);

    setLayout(vbox);
}

toTemplate::~toTemplate()
{
    TemplateTool.closeWindow();
    delete Result;
}

void toTemplate::hideTemplates()
{
    TemplateTool.toggleWindow();
}

void toTemplate::showResult(bool show)
{
    if (!Result)
        return ;

    if (show)
    {
        Result->show();
    }
    else
    {
        Result->hide();
    }
}

void toTemplate::closeFrame(void)
{
    Result = NULL;
    Frame = NULL;
}

void toTemplate::expand(toTreeWidgetItem *item)
{
    try
    {
        toTemplateItem *ti = dynamic_cast<toTemplateItem *>(item);
        if (ti)
            ti->expand();
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
}

void toTemplate::selected(toTreeWidgetItem *item)
{
    try
    {
        toTemplateItem *ti = dynamic_cast<toTemplateItem *>(item);
        if (ti)
        {
            ti->setSelected(true);
            ti->selected();
        }
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
}

QWidget *toTemplate::frame(void)
{
    if (Result->isHidden())
        showResult(true);
    return Frame;
}

void toTemplate::collapse(toTreeWidgetItem *item)
{
    toTemplateItem *ti = dynamic_cast<toTemplateItem *>(item);
    if (ti)
        ti->collapse();
}


QWidget *toTemplateItem::selectedWidget(QWidget *)
{
    return NULL;
}

void toTemplateItem::setSelected(bool sel)
{
    try
    {
        toTemplate *temp = toTemplate::templateWidget(this);
        if (sel && temp)
        {
            QWidget *frame = toTemplate::parentWidget(this);
            if (frame)
            {
                QWidget *w = selectedWidget(frame);
                if (w)
                    frame->layout()->addWidget(w);
                temp->setWidget(w);
            }
        }
    }
    TOCATCH
    toResultViewItem::setSelected(sel);
}

QWidget *toTemplateText::selectedWidget(QWidget *parent)
{
    QWidget *ret = new QTextEdit(Note, parent);
    if (parent->layout())
        parent->layout()->addWidget(ret);
    return ret;
}

void toTemplate::setWidget(QWidget *widget)
{
    if (!widget)
    {
        widget = new QTextEdit(frame());
        frame()->layout()->addWidget(widget);
    }

    if (WidgetExtra != widget)
    {
        widget->show();
        if (WidgetExtra)
            delete WidgetExtra;

        WidgetExtra = widget;
    }
}

class toTextTemplate : toTemplateProvider
{
    public:
        toTextTemplate()
            : toTemplateProvider("Text")
        { }
        void addFile(toTreeWidget *parent, const QString &root, const QString &file);
        virtual void insertItems(toTreeWidget *parent, QToolBar *toolbar);
};

void toTextTemplate::insertItems(toTreeWidget *parent, QToolBar *)
{
    TemplatesMap def = DefaultText();

    TemplatesMap tMap = toConfigurationNewSingle::Instance().option(ToConfiguration::Template::TemplateMap).toMap();
    TemplatesMapIterator i(tMap);
    while (i.hasNext())
    {
        i.next();
        addFile(parent, i.key(), i.value().toString());
        if (def.find(i.key()) != def.end())
            def.erase(def.find(i.key()));
    }
    TemplatesMapIterator j(def);
    while (j.hasNext())
    {
        j.next();
        addFile(parent, j.key(), j.value().toString());
    }
}

void toTextTemplate::addFile(toTreeWidget *parent, const QString &root, const QString &file)
{
    std::map<QString, QString> pairs;
    try
    {
        Utils::toLoadMap(file, pairs);
        toTemplateItem *last = new toTemplateItem(*this, parent, root);
        int lastLevel = 0;
        QStringList lstCtx;
        for (std::map<QString, QString>::iterator i = pairs.begin(); i != pairs.end(); i++)
        {
            QStringList ctx = (*i).first.split(QString::fromLatin1(":"));
            if (last)
            {
                while (last && lastLevel >= int(ctx.count()))
                {
                    last = dynamic_cast<toTemplateItem *>(last->parent());
                    lastLevel--;
                }
                while (last && lastLevel >= 0 && !Utils::toCompareLists(lstCtx, ctx, (unsigned int)lastLevel))
                {
                    last = dynamic_cast<toTemplateItem *>(last->parent());
                    lastLevel--;
                }
            }
            if (lastLevel < 0)
                throw qApp->translate("toTemplate", "Internal error, lastLevel < 0");
            while (lastLevel < int(ctx.count()) - 1)
            {
                last = new toTemplateItem(last, ctx[lastLevel]);
                lastLevel++;
            }
            last = new toTemplateText(last, ctx[lastLevel], (*i).second);
            lstCtx = ctx;
            lastLevel++;
        }
    }
    TOCATCH
}

toTemplateSQL::toTemplateSQL(toConnection &conn, toTemplateItem *parent,
                             const QString &name, const QString &sql)
    : toTemplateItem(parent, name), Object(this), Connection(&conn), SQL(sql)
{
    setExpandable(true);
}

toTemplateSQL::toTemplateSQL(toTemplateItem *parent,
                             const QString &name, const QString &sql)
    : toTemplateItem(parent, name), Object(this), SQL(sql)
{
    Connection = NULL;
    setExpandable(true);
}

toTemplateSQLObject::toTemplateSQLObject(toTemplateSQL *parent)
    : Parent(parent)
{
    Query = NULL;
}

void toTemplateSQL::expand(void)
{
    while (firstChild())
        delete firstChild();
    Object.expand();
}

void toTemplateSQLObject::expand(void)
{
    try
    {
        delete Query;
        Query = NULL;
        Query = new toEventQuery(this
                                 , Parent->connection()
                                 , Parent->SQL
                                 , toQueryParams()
                                 , toEventQuery::READ_ALL
                                );
        connect(Query, &toEventQuery::dataAvailable, this, &toTemplateSQLObject::receiveData);
        connect(Query, &toEventQuery::done, this, [=](toEventQuery*, unsigned long){ queryDone(); });
        Query->start();
    }
    TOCATCH
}

void toTemplateSQLObject::receiveData(toEventQuery*)
{
    if (QApplication::activeModalWidget()) // Template is never in widget
        return;

    try
    {
        toQColumnDescriptionList desc = Query->describe();
        while (Query->hasMore())
        {
            toTreeWidgetItem *item = Parent->createChild((QString)Query->readValue());
            for (int j = 1; j < desc.size(); j++)
                item->setText(j, (QString)Query->readValue());
        }
    }
    catch (const QString &str)
    {
        delete Query;
        Query = NULL;
        Utils::toStatusMessage(str);
    }
} // poll

void toTemplateSQLObject::queryDone(void )
{
    delete Query;
    Query = NULL;
} // queryDone

toTemplateSQLObject::~toTemplateSQLObject()
{
    delete Query;
}

static toTextTemplate TextTemplate;
