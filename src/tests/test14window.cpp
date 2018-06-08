
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

#include "tests/test14window.h"
#include "editor/tosqltext.h"
#include "core/tologger.h"
#include "docklets/toastwalk.h"

#include "dotgraph.h"

#include <QStatusBar>
#include <QScrollArea>
#include <QSplitter>
#include <QToolBar>

Test14Window::Test14Window(const QString &sql)
    : Ui::Test14Window()
    , toContextMenuHandler()
{
    Ui::Test14Window::setupUi(this);

    Ui::Test14Window::xdotText->setText(sql);

    connect(actionOpen, SIGNAL(triggered()), this, SLOT(load()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(Ui::Test14Window::queryView, SIGNAL(selected(const QMap<QString,QString>&)), this, SLOT(elementSelected(const QMap<QString,QString>&)));
    QMainWindow::show();

    parseAct = new QAction(tr("Parse"), this);
    parseAct->setShortcut(Qt::CTRL + Qt::Key_K);
    connect(parseAct, SIGNAL(triggered()), this, SLOT(parse(void)));
}

void Test14Window::load()
{
    QString fn = Utils::toOpenFilename("*.xdot", this);
    if (!fn.isEmpty())
    {
        QString data = Utils::toReadFile(fn);
        Ui::Test14Window::xdotText->setText(data);
    }
}

void Test14Window::parse()
{
    QString lastText = Ui::Test14Window::xdotText->text();

    if (lastText.isEmpty())
        return;

    try
    {
        //DotGraph G;
        //G.loadXDotText(lastText);
//        std::unique_ptr <SQLParser::Statement> stat = StatementFactTwoParmSing::Instance().create("OracleDML", lastText, "");
//        TLOG(8, toDecorator, __HERE__)
//        << "Parsing ok:" << std::endl
//        << stat->root()->toStringRecursive().toStdString() << std::endl;
//
//        TLOG(8, toDecorator, __HERE__) << stat->root()->toLispStringRecursive() << std::endl;

        Ui::Test14Window::queryView->initEmpty();

        Ui::Test14Window::queryView->loadXDotText(lastText);

        //m_widget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        //setFocusProxy(m_widget); // TODO ?? What is this??
        //setWidget(m_widget); // TODO ?? What is this??
        //toASTWalk(stat.get(), Ui::Test14Window::queryView->graph());
        Ui::Test14Window::queryView->prepareSelectSinlgeElement();
    }
    catch ( SQLParser::ParseException const &e)
    {

    }
    catch (QString const& s)
    {
        TLOG(8, toDecorator, __HERE__) << s << std::endl;
    }
    catch (std::exception const& e)
    {

    }
    catch (...)
    {

    }
}

void Test14Window::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}

void Test14Window::handle(QObject* obj, QMenu* menu)
{
    QString name = obj->objectName();

    if (toSqlText* t = dynamic_cast<toSqlText*>(obj))
    {
        Q_UNUSED(t);
        menu->addSeparator();
        menu->addAction(parseAct);
    }
}
