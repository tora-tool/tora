
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

#include "utils.h"

#include "toconf.h"
#include "tohelp.h"
#include "tohtml.h"
#include "tomain.h"
#include "toresultview.h"
#include "totool.h"
#include "tohelpsetup.h"

#include <QMessageBox>
#include <QSettings>


toHelp *toHelp::Window;

toHelpAddFile::toHelpAddFile(QWidget *parent, const char *name)
        : QDialog(parent)
{

    if (name)
        setObjectName(name);

    setupUi(this);
    OkButton->setEnabled(false);
    toHelp::connectDialog(this);
}


void toHelpAddFile::browse()
{
    QString filename = toOpenFilename(Filename->text(), QString::fromLatin1("toc.htm*"), this);
    if (!filename.isEmpty())
        Filename->setText(filename);
}


void toHelpAddFile::valid()
{
    if (Filename->text().isEmpty() || Root->text().isEmpty())
        OkButton->setEnabled(false);
    else
        OkButton->setEnabled(true);
}


toHelpPrefs::toHelpPrefs(toTool *tool, QWidget *parent, const char *name)
        : QWidget(parent), toSettingTab("additionalhelp.html"), Tool(tool)
{

    if (name)
        setObjectName(name);

    setupUi(this);

//     int tot = Tool->config("Number", "-1").toInt();
//     if (tot != -1)
//     {
//         for (int i = 0;i < tot;i++)
//         {
//             QString num = QString::number(i);
//             QString root = Tool->config(num.toLatin1(), "");
//             num += QString::fromLatin1("file");
//             QString file = Tool->config(num.toLatin1(), "");
//             new toTreeWidgetItem(FileList, root, file);
//         }
//     }
    HelpsMapIterator i(toConfigurationSingle::Instance().additionalHelp());
    while (i.hasNext())
    {
        i.next();
        new toTreeWidgetItem(FileList, i.key(), i.value());
    }
}


void toHelpPrefs::saveSetting()
{
//     int i = 0;
    HelpsMap h;
    for (toTreeWidgetItem *item = FileList->firstChild();item;item = item->nextSibling())
    {
//         QString nam = QString::number(i);
//         Tool->setConfig(nam.toLatin1(), item->text(0));
//         nam += QString::fromLatin1("file");
//         Tool->setConfig(nam.toLatin1(), item->text(1));
//         i++;
        h[item->text(0)] = item->text(1);
    }
//     Tool->setConfig("Number", QString::number(i));
    toConfigurationSingle::Instance().setAdditionalHelp(h);
    delete toHelp::Window;
}


void toHelpPrefs::addFile()
{
    toHelpAddFile file(this);
    if (file.exec())
        new toTreeWidgetItem(FileList, file.Root->text(), file.Filename->text());
}


void toHelpPrefs::delFile()
{
    delete FileList->selectedItem();
}


void toHelpPrefs::oracleManuals()
{
    // For Oracle 11g look for a file /nav/portal_3.htm which contains a list of all books
    QString filename = toOpenFilename(QString::null, QString::fromLatin1("*index.htm*"), this);
    try
    {
        toHtml file(toReadFile(filename));
        QString dsc;
        bool inDsc = false;
        QRegExp isToc(QString::fromLatin1("toc\\.htm?$"));
        while (!file.eof())
        {
            file.nextToken();
            if (file.isTag())
            {
                if (file.open() && file.tag() == "a")
                {
                    QString href = toHelp::path(filename);
                    href += file.value("href");
                    if (!href.isEmpty() &&
                        !dsc.isEmpty() &&
                        href.indexOf(isToc) >= 0 &&
                        !file.value("title").isNull())
                    {
                        new toTreeWidgetItem(FileList, dsc.simplified(), href);
                        inDsc = false;
                        dsc = QString::null;
                    }
                }
                // This one is for oracle manuals with version 11
                else if ((file.open() && file.tag() == "td") && !file.value("id").isNull())
                {
                    dsc.clear();
                    inDsc = true;
                }
                // This one is for old version of oracle manuals
                else if ((file.open() && file.tag() == "dd") || file.tag() == "book")
                {
                    dsc.clear();
                    inDsc = true;
                }
            }
            else if (inDsc)
                dsc += file.text();
        }
    }
    catch (const QString &str)
    {
        TOMessageBox::warning(toMainWidget(), qApp->translate("toHelpPrefs", "File error"), str);
    }
}


QWidget *toHelpTool::configurationTab(QWidget *parent)
{
    return new toHelpPrefs(this, parent);
}

void toHelpTool::displayHelp(void)
{
    QWidget *cur = qApp->focusWidget();
    while (cur)
    {
        QDialog *dlg = dynamic_cast<QDialog *>(cur);
        if (dlg)
        {
            toHelp::displayHelp(dlg);
            return ;
        }
        cur = cur->parentWidget();
    }
    // No dialog found
    toHelp::displayHelp();
}

static toHelpTool HelpTool;

toHelp::toHelp(QWidget *parent, QString name, bool modal)
        : QDialog(parent)
//                  ,
//                  Qt::Window |
//                  Qt::WindowTitleHint |
//                  Qt::WindowSystemMenuHint |
//                  Qt::WindowMinimizeButtonHint |
//                  Qt::WindowMaximizeButtonHint |
//                  Qt::WindowCloseButtonHint)
{
    setupUi(this);
    setModal(modal);

    if (!modal)
        Window = this;

    connect(SearchLine, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(Sections, SIGNAL(currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(changeContent( QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(Result, SIGNAL(currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem *)),
            this, SLOT(changeContent( QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(Help, SIGNAL(textChanged(void)),
            this, SLOT(removeSelection(void)));
    connect(Help, SIGNAL(backwardAvailable(bool)),
            backButton, SLOT(setEnabled(bool)));
    connect(backButton, SIGNAL(clicked(void)),
            Help, SLOT(backward(void)));
    connect(Help, SIGNAL(forwardAvailable(bool)),
            forwardButton, SLOT(setEnabled(bool)));
    connect(forwardButton, SIGNAL(clicked(void)),
            Help, SLOT(forward(void)));

    std::map<QString, QString> Dsc;
    Dsc[tr(TOAPPNAME " manual")] = QString("qrc:/help/toc.html");
    HelpsMapIterator i(toConfigurationSingle::Instance().additionalHelp());
    while (i.hasNext())
    {
        i.next();
        Dsc[i.key()] = i.value();
    }

    splitter->setStretchFactor(splitter->indexOf(tabs), 0);
    setGeometry(x(), y(), std::max(width(), 640), std::max(height(), 480));

    QTreeWidgetItem * lastParent = NULL;
    for (std::map<QString, QString>::iterator i = Dsc.begin();i != Dsc.end();i++)
    {
        try
        {
            QString path = toHelp::path((*i).second);
            QString filename = (*i).second;
            QTreeWidgetItem * parent;
            // TODO: With this "if" removed TOra manual could be added somewhere in the
            //       middle of other help documents which is not OK. This should be
            //       fixed so that TOra manual is always the first one.
            //if ((*i).first == tr("TOra manual"))
            //{
                parent = new QTreeWidgetItem(Sections, QStringList() << (*i).first << "" << filename);
                Sections->expandItem(parent);
                if (!lastParent)
                    lastParent = parent;
            //}
            //else
            //    parent = lastParent = new QTreeWidgetItem(lastParent, QStringList() << (*i).first << "" << filename);
            toHtml file(toReadFile(filename));
            bool inA = false;
            QString dsc;
            QString href;
            QTreeWidgetItem * last = NULL;
            bool inOracle = false; // indicates that we're in an Oracle documentation
            // Following two variables indicate that we're in a specific part of oracle documentation
            bool inHeader = false; // in h2 class=tocheader (1st level content)
            bool inList = false;   // in li (2nd, 3rd... level conent)
            while (!file.eof())
            {
                file.nextToken();

                if (!file.isTag())
                {
                    if (inA)
                    {
                        dsc += file.text();
                        dsc = dsc.simplified();
                    }
                }
                else
                {
                    QString c = file.tag();
                    if (c == "a" && (!inOracle || (inOracle && (inHeader || inList))))
                    {
                        if (file.open())
                        {
                            href = file.value("href");
                            if (!href.isEmpty())
                                inA = true;
                        }
                        else
                        {
                            if (inA &&
                                !dsc.isEmpty() &&
                                !href.isEmpty())
                            {
                                if (href.indexOf("//") < 0 &&
                                    href.indexOf("..") < 0)
                                {
                                    last = new QTreeWidgetItem(parent, QStringList() << dsc);
                                    filename = path;
                                    filename += href;
                                    last->setText(2, filename);
                                }
                                dsc.clear();
                            }
                            inA = false;
                        }
                    }
                    else if (c == "dl" || c == "ul")
                    {
                        if (file.open())
                        {
                            if (!last)
                                last = new QTreeWidgetItem(parent, QStringList() << "--------");
                            parent = last;
                            last = NULL;
                        }
                        else
                        {
                            last = parent;
                            parent = parent->parent();
                            if (!parent)
                                throw tr("Missing parent, unbalanced dl in help file content");
                        }
                    }
                    else if (c == "h2")
                    {
                        if (file.open() &&  file.value("class") == "tocheader")
                            inHeader = true;
                        else
                            inHeader = false;
                    }
                    else if (c == "li")
                    {
                        if (file.open())
                            inHeader = true;
                        else
                            inHeader = false;
                    }
                    else if ((c == "meta") && file.value("name") == "generator")
                    {
                        inOracle = true;
                    }
                }
            }
        }
        TOCATCH
    }

    for (int i = 0; i < Sections->topLevelItemCount(); ++i)
        Manuals->addItem(Sections->topLevelItem(i)->text(0));

//     Progress = new QProgressBar(tabs);
    Progress->setMaximum(Dsc.size());
    Progress->hide();
    Searching = false;

    QSettings s;
    s.beginGroup("helpdialog");
    restoreGeometry(s.value("geometry").toByteArray());
    splitter->restoreState(s.value("splitter").toByteArray());
}

toHelp::~toHelp()
{
    if (Window == this)
        Window = NULL;
}

void toHelp::closeEvent(QCloseEvent * e)
{
    if (Searching)
        return;

    QSettings s;
    s.beginGroup("helpdialog");
    s.setValue("geometry", saveGeometry());
    s.setValue("splitter", splitter->saveState());

    QWidget::closeEvent(e);
}

QString toHelp::path(const QString &path)
{
    QString cur;
    if (path.isNull())
        cur = "qrc:/help/toc.html";
    else
        cur = path;
    cur.replace(QRegExp(QString::fromLatin1("[^/]+$")), QString::null);
    return cur;
}

void toHelp::displayHelp(const QString &context, QWidget *parent)
{
    toHelp *window;
    if (!Window || parent)
        window = new toHelp(parent, tr("Help window"), false);
    else
        window = Window;
    QString file = path();

    file += context;

    window->Help->setSource(file);

    if (parent)
    {
        window->exec();
        delete window;
    }
    else
        window->show();
}

void toHelp::displayHelp(QWidget *parent)
{
    QWidget *cur = qApp->focusWidget();
    while (cur)
    {
        toHelpContext *ctx = dynamic_cast<toHelpContext *>(cur);
        if (ctx && !ctx->context().isEmpty())
        {
            toHelp::displayHelp(ctx->context(), parent);
            return ;
        }
        cur = cur->parentWidget();
    }
    toHelp::displayHelp(QString::fromLatin1("toc.html"), parent);
}

void toHelp::connectDialog(QDialog *dialog)
{
    QAction *a = new QAction(dialog);
    a->setShortcut(toKeySequence(tr("F1", "Dialog|Help")));
    connect(a,
            SIGNAL(triggered()),
            &HelpTool,
            SLOT(displayHelp()));
}

void toHelp::changeContent(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    disconnect(Help, SIGNAL(textChanged(void)),
               this, SLOT(removeSelection(void)));

    if (!item->text(2).isEmpty())
    {
        if (item->text(2).startsWith("qrc:"))
            Help->setSource(item->text(2));
        else
            // NOTE: it would be more correct to get a proper url of local file using
            // QUrl::fromLocalFile but in that case we would have to remove html anchor
            // and then add it back again after getting a file url.
            Help->setSource("file://" + item->text(2));
    }

    connect(Help, SIGNAL(textChanged(void)),
            this, SLOT(removeSelection(void)));
}

void toHelp::search(void)
{
    if (Searching)
        return ;

    Searching = true;
    Result->clear();
    QStringList words = SearchLine->text().toLower().split(QRegExp(QString::fromLatin1("\\s+")));
    if (words.count() == 0)
        return ;
    QRegExp strip(QString::fromLatin1("\\d+-\\d+\\s*,\\s+"));
    QRegExp stripend(QString::fromLatin1(",$"));
    int steps = 1;
    Progress->setValue(0);

    qApp->processEvents();

    QTreeWidgetItem * parent;
    for (int i = 0; i < Sections->topLevelItemCount(); ++i)
    {
        parent = Sections->topLevelItem(i);
        if (Manuals->currentIndex() == 0 || parent->text(0) == Manuals->currentText())
        {
            QString path = toHelp::path(parent->text(2));
            QString filename = path;
            filename.append(QString::fromLatin1("toc.html"));

            try
            {
                toHtml file(toReadFile(filename));
                std::list<QString> Context;
                bool inDsc = false;
                bool aRestart = true;
                QString dsc;
                QString href;
                while (!file.eof())
                {
                    file.nextToken();
                    if (file.isTag())
                    {
                        if (file.open())
                        {
                            if (file.tag() == "a")
                            {
                                href = file.value("href");
                                if (href[0] == '#')
                                    href = "";
                                else if (href.indexOf("..") >= 0)
                                    href = "";
                            }
                            else if (file.tag() == "dd")
                            {
                                inDsc = true;
                                aRestart = false;
                                href = dsc = "";
                            }
                            else if (file.tag() == "dl")
                            {
                                toPush(Context, dsc.simplified());
                                href = dsc = "";
                                inDsc = true;
                            }
                        }
                        else if (file.tag() == "a")
                        {
                            if (!dsc.isEmpty() &&
                                    !href.isEmpty())
                            {
                                QString tmp;
                                for (std::list<QString>::iterator i = Context.begin();i != Context.end();i++)
                                    if (i != Context.begin() && !(*i).isEmpty())
                                    {
                                        tmp += *i;
                                        tmp += QString::fromLatin1(", ");
                                    }
                                tmp += dsc.simplified();
                                QString url = path;
                                url += href;
                                aRestart = true;

                                bool incl = true;
                                {
                                    for (int i = 0;i < words.count();i++)
                                        if (!tmp.contains(words[i], Qt::CaseInsensitive))
                                        {
                                            incl = false;
                                            break;
                                        }
                                }

                                if (incl)
                                {
                                    tmp.replace(strip, QString::fromLatin1(" "));
                                    tmp.replace(stripend, QString::fromLatin1(" "));
                                    QTreeWidgetItem *item = new QTreeWidgetItem(
                                        Result,
                                        QStringList() << tmp.simplified());
                                    item->setText(1, parent->text(0));
                                    item->setText(2, url);
                                }
                                href = "";
                            }
                        }
                        else if (file.tag() == "dl")
                        {
                            toPop(Context);
                        }
                    }
                    else if (inDsc)
                    {
                        dsc += file.text();
                    }
                }
            }
            TOCATCH
        }
        Progress->setValue(steps);
        ++steps;
        if (steps > 10)
            Progress->show();
        qApp->processEvents();
    }
    Progress->hide();
    Searching = false;
}

void toHelp::setSelection(QTreeWidget *lst, const QString &source)
{
    /*    disconnect(lst, SIGNAL(selectionChanged(toTreeWidgetItem *)),
                   this, SLOT(changeContent(toTreeWidgetItem *)));

        bool any = false;

        QString t = source;
        t.replace(QRegExp(QString::fromLatin1("^file:")), QString::fromLatin1(""));

        toTreeWidgetItem *next = NULL;
        for (toTreeWidgetItem *item = lst->firstChild();item;item = next)
        {

            if ((item->text(2) == t) != bool(item->isSelected()))
            {
                if (item->text(2) == t)
                {
                    any = true;
                    lst->setSelected(item, true);
                    lst->ensureItemVisible(item);
                    for (toTreeWidgetItem *parent = item->parent();parent;parent = parent->parent())
                        lst->setOpen(parent, true);
                    break;
                }
                else
                    lst->setSelected(item, false);
            }

            if (item->firstChild())
                next = item->firstChild();
            else if (item->nextSibling())
            {
                next = item->nextSibling();
            }
            else
            {
                next = item;
                do
                {
                    next = next->parent();
                }
                while (next && !next->nextSibling());
                if (next)
                    next = next->nextSibling();
            }
        }

        connect(lst, SIGNAL(selectionChanged(toTreeWidgetItem *)),
                this, SLOT(changeContent(toTreeWidgetItem *)));

        if (!any)
        {
            QString t = source;
            t.replace(QRegExp(QString::fromLatin1("#[^#]*$")), QString::null);
            if (t != source)
                setSelection(lst, t);
        }*/
}

void toHelp::removeSelection(void)
{
    setSelection(Sections, Help->source().toString());
    setSelection(Result, Help->source().toString());
}
