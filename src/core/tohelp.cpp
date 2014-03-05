
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

#include "core/tohelp.h"
#include "core/utils.h"
#include "core/toconf.h"
#include "core/tohtml.h"
#include "core/tomainwindow.h"
#include "core/totool.h"
#include "core/tohelpsetup.h"
#include "core/toconfiguration_new.h"
#include "ts_log/ts_log_utils.h"

#include <QtGui/QMessageBox>
#include <QtCore/QSettings>

QVariant ToConfiguration::Help::defaultValue(int option) const
{
	switch(option)
	{
	// Paths
	case AdditionalHelpMap:			return QVariant(QMap<QString, QVariant>());
	default:
		Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Help un-registered enum value: %1").arg(option)));
		return QVariant();
	}
}


toHelp *toHelp::Window;

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

ToConfiguration::Help toHelpTool::s_helpConfig;

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
    QMap<QString, QVariant> hMap = toConfigurationNewSingle::Instance().option(ToConfiguration::Help::AdditionalHelpMap).toMap();
    QMapIterator<QString, QVariant> i(hMap);
    while (i.hasNext())
    {
        i.next();
        Dsc[i.key()] = i.value().toString();
    }

    splitter->setStretchFactor(splitter->indexOf(tabs), 0);
    setGeometry(x(), y(), (std::max)(width(), 640), (std::max)(height(), 480));

    QTreeWidgetItem * lastParent = NULL;
    for (std::map<QString, QString>::iterator i = Dsc.begin(); i != Dsc.end(); i++)
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
            toHtml file(Utils::toReadFile(filename));
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
    a->setShortcut(Utils::toKeySequence(tr("F1", "Dialog|Help")));
    connect(a,
            SIGNAL(triggered()),
            &HelpTool,
            SLOT(displayHelp()));
}

void toHelp::changeContent(QTreeWidgetItem * item, QTreeWidgetItem *)
{
    disconnect(Help, SIGNAL(textChanged(void)),
               this, SLOT(removeSelection(void)));

    if (item && !item->text(2).isEmpty())
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
            /* We have to find file with index information. This file should be called
             * either toc.html or index.htm. Note that we cannot use QFile::exists()
             * to check existance of file in qt resources so that case is hardcoded.
             */
            if (filename.startsWith("qrc:"))
                filename.append(QString::fromLatin1("toc.html"));
            else
            {
                QFile file;
                // Oracle 11g has an index.htm file with manual index
                file.setFileName(filename + "index.htm");
                if (file.exists())
                    filename.append(QString::fromLatin1("index.htm"));
                else
                {
                    // toc.html is here for backwards compatibility (say Oracle 9i)
                    // Can probably be removed later (written on 2011-01-23)
                    file.setFileName(filename + "toc.html");
                    if (file.exists())
                        filename.append(QString::fromLatin1("toc.html"));
                    else
                    {
                        // If neither toc.html nor index.htm was found in manual directory - do nothing
                        continue;
                    }
                }
            }

            try
            {
                toHtml file(Utils::toReadFile(filename));
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
                                Utils::toPush(Context, dsc.simplified());
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
                                for (std::list<QString>::iterator i = Context.begin(); i != Context.end(); i++)
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
                                    for (int i = 0; i < words.count(); i++)
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
                            Utils::toPop(Context);
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
