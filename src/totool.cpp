
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

//
// C++ Implementation: toTool, toToolWidget, toConnectionWidget
//

#include "utils.h"

#include "tobackground.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "totool.h"

#ifndef Q_OS_WIN32
#include <unistd.h>
#endif

#include <qapplication.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtimer.h>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QVBoxLayout>


// A little magic to get lrefresh to work and get a check on qApp

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

toConnectionWidget::toConnectionWidget(toConnection &conn, QWidget *widget)
        : Connection(&conn), Widget(widget)
{
    Connection->addWidget(Widget);
}

void toConnectionWidget::setConnection(toConnection &conn)
{
    if (Connection)
        Connection->delWidget(Widget);
    Connection = &conn;
    Connection->addWidget(Widget);
}

toConnectionWidget::toConnectionWidget(QWidget *widget)
        : Widget(widget)
{
    Connection = NULL;
}

toConnection &toConnectionWidget::connection()
{
    if (Connection)
        return *Connection;
    QWidget *widget = Widget;
    if (!widget)
        throw qApp->translate("toConnectionWidget", "toConnectionWidget not inherited with a QWidget");
    return toCurrentConnection(widget->parentWidget());
}

toToolWidget::toToolWidget(toTool &tool, const QString &ctx, QWidget *parent, toConnection &conn, const char *name)
    : QWidget(parent),
        toHelpContext(ctx),
        toConnectionWidget(conn, this),
        Tool(tool)
{
    if (name)
        setObjectName(name);
    // make sure widget gets deleted
    setAttribute(Qt::WA_DeleteOnClose);
    // have to set the basic layout for widgets. It's requested later
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    Timer = NULL;

    if (parent)
    {
        // Voodoo for making connection changing cascade to sub tools.
        try
        {
            toToolWidget *tool = toCurrentTool(parent);
            if (tool)
                connect(tool, SIGNAL(connectionChange()), this, SLOT(parentConnection()));
        }
        catch (...)
	{}
    }
}

toConnectionWidget::~toConnectionWidget()
{
}

void toToolWidget::parentConnection(void)
{
    try
    {
        setConnection(toCurrentConnection(parentWidget()));
    }
    TOCATCH
}

toToolWidget::~toToolWidget()
{
    toMainWidget()->toolWidgetRemoved(this);
}

void toToolWidget::setConnection(toConnection &conn)
{
    bool connCap = false;
    QString name = connection().description();
    QString capt = windowTitle();
    if (capt.startsWith(name))
    {
        connCap = true;
        capt = capt.mid(name.length());
    }
    toConnectionWidget::setConnection(conn);
    if (connCap)
    {
        capt.prepend(connection().description());
        setWindowTitle(capt);
    }
    emit connectionChange();
}

toTimer *toToolWidget::timer(void)
{
    if (!Timer)
        Timer = new toTimer(this);
    return Timer;
}

void toToolWidget::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    QWidget *par = parentWidget();
    if (!par)
        par = this;
    if (isMaximized() || par->width() >= toMainWidget()->workspace()->width())
        data[prefix + ":State"] = QString::fromLatin1("Maximized");
    else if (isMinimized())
        data[prefix + ":State"] = QString::fromLatin1("Minimized");

    data[prefix + ":X"] = QString::number(par->x());
    data[prefix + ":Y"] = QString::number(par->y());
    data[prefix + ":Width"] = QString::number(par->width());
    data[prefix + ":Height"] = QString::number(par->height());
}

void toToolWidget::importData(std::map<QString, QString> &data, const QString &prefix)
{
    QWidget *par = parentWidget();
    if (!par)
        par = this;
    if (data[prefix + ":State"] == QString::fromLatin1("Maximized"))
    {
        par->showMaximized();
        showMaximized();
    }
    else if (data[prefix + ":State"] == QString::fromLatin1("Minimized"))
    {
        par->showMinimized();
        showMinimized();
    }
    else
    {
        par->showNormal();
        par->setGeometry(data[prefix + ":X"].toInt(),
                         data[prefix + ":Y"].toInt(),
                         data[prefix + ":Width"].toInt(),
                         data[prefix + ":Height"].toInt());

    }
}

std::map<QString, toTool *> *toTool::Tools;
// std::map<QCString, QString> *toTool::Configuration;


const char **toTool::pictureXPM(void)
{
    return (const char **) NULL;  // compiler warning
}

toTool::toTool(int priority, const char *name)
        : Name(name),
        Priority(priority),
        ButtonPicture(0),
        toolAction(0)
{
    if (!Tools)
        Tools = new std::map<QString, toTool *>;
    Key.sprintf("%05d%s", priority, name);
    (*Tools)[Key] = this;
}

toTool::~toTool()
{
    Tools->erase(Key);
    delete ButtonPicture;
}

const QPixmap *toTool::toolbarImage()
{
    if (!ButtonPicture)
    {
        const char **picture = pictureXPM();
        if (picture)
            ButtonPicture = new QPixmap(picture);
    }
    return ButtonPicture;
}

QWidget* toTool::createWindow()
{
    toMain *main = toMainWidget();
    QWidget *newWin = 0;

    try
    {
        if (!canHandle(main->currentConnection()))
            throw QString("The tool %1 doesn't support the current database").arg(QString(name()));
        newWin = toolWindow(main->workspace(), main->currentConnection());

        if (newWin)
        {
            toToolWidget *tool = dynamic_cast<toToolWidget *>(newWin);

            if (tool && tool->windowTitle().isEmpty()) {
                toToolCaption(tool, name());
            }

            // save previous window
            QMdiSubWindow *previous = main->lastActiveWindow();
            QMdiSubWindow *newsub = main->workspace()->addSubWindow(newWin);
            newsub->setFocusProxy(newWin);
            const QPixmap *icon = toolbarImage();
            if (icon)
            {
                newWin->setWindowIcon(*icon); // is this really required?
                newsub->setWindowIcon(*icon);
            }

            if(tool)
                main->toolWidgetAdded(tool);

            // Maximize window if only window
            bool max = true;
            foreach(QMdiSubWindow *window, main->workspace()->subWindowList())
            {
                QWidget *widget = window->widget();
                if (widget && widget != newWin && !widget->isHidden())
                    max = false;
            }

            if(!max && previous)
            {
                // if not max, check previous window. if it's
                // maximized then show this window max
                if(previous->windowState() & Qt::WindowMaximized)
                    max = true;
            }

            if (max)
                newWin->showMaximized();

            // workaround bug in mdi. deactivate subwindow first, then
            // set active
            // WARNING: don't change this call order unles you know what
            // are you doing ;) It's working now. When there is missing
            // "previous" call, the toWorksheet tool do not get focus.
            // Dunno why... it makes me mad.
            main->workspace()->setActiveSubWindow(0);
            main->workspace()->setActiveSubWindow(previous);
            main->workspace()->setActiveSubWindow(newsub);

            // piece of shit mdi doesn't always send window activated
            // signal
            main->windowActivated(newsub);

            newsub->show();
            newsub->raise();
            newsub->setFocus();

            main->updateWindowsMenu();
        }
    }
    TOCATCH;

    return newWin;
}


bool toTool::canHandle(toConnection &conn)
{
    return (toIsOracle(conn));
}

QAction* toTool::getAction(void)
{
    if (toolAction)
        return toolAction;

    toolAction = new QAction(toMainWidget());
    if (toolbarImage())
        toolAction->setIcon(QIcon(QPixmap(pictureXPM())));
    if (menuItem())
        toolAction->setText(menuItem());
    if (toolbarTip())
        toolAction->setToolTip(toolbarTip());

    connect(toolAction, SIGNAL(triggered()), this, SLOT(createWindow()));

    return toolAction;
}


void toTool::customSetup()
{}

QWidget *toTool::configurationTab(QWidget *)
{
    return NULL;
}

toTool *toTool::tool(const QString &key)
{
    if (!Tools)
        Tools = new std::map<QString, toTool *>;
    std::map<QString, toTool *>::iterator i = Tools->find(key);
    if (i == Tools->end())
        return NULL;

    return (*i).second;
}

void toTool::about(QWidget *)
{}

// const QString& toTool::config(const QString &tag, const QString &def) {
//     return toConfigurationSingle::Instance().config(tag, def, Name);
// }
//
// void toTool::setConfig(const QString &tag, const QString &value) {
//     toConfigurationSingle::Instance().setConfig(tag, value, Name);
// }
