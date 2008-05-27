//
// C++ Implementation: toTool, toToolWidget, toConnectionWidget
//
// Copyright: See COPYING file that comes with this distribution
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
    : QMdiSubWindow(parent),
        toHelpContext(ctx),
        toConnectionWidget(conn, this),
        Tool(tool)
{

    if (name)
        setObjectName(name);
    setAttribute(Qt::WA_DeleteOnClose);

    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);

    Timer = NULL;

    if (parent)
    {
        QMdiArea *wspace = dynamic_cast<QMdiArea *>(parent);
        if (wspace)
            wspace->addSubWindow(this, Qt::Window);

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
#include <QMdiSubWindow>
void toTool::createWindow(void)
{
    toMain *main = toMainWidget();
    try
    {
        if (!canHandle(main->currentConnection()))
            throw QString("The tool %1 doesn't support the current database").arg(QString(name()));
        QWidget *newWin = toolWindow(main->workspace(), main->currentConnection());

        if (newWin)
        {
            // make sure widget gets deleted
            newWin->setAttribute(Qt::WA_DeleteOnClose);

            const QPixmap *icon = toolbarImage();
            if (icon)
                newWin->setWindowIcon(*icon);
            toToolWidget *tool = dynamic_cast<toToolWidget *>(newWin);
            if (tool && tool->windowTitle().isEmpty())
                toToolCaption(tool, name());

            newWin->show();
            newWin->raise();
            newWin->setFocus();

            main->updateWindowsMenu();

            if (tool)
                main->toolWidgetAdded(tool);

            // Maximize window if only window
            {
                bool max = true;
                for (int i = 0;i < toMainWidget()->workspace()->subWindowList().count();i++)
                {
                    QWidget *widget = toMainWidget()->workspace()->subWindowList().at(i);

                    if (widget && widget != newWin && !widget->isHidden())
                        max = false;
                }
                if (max)
                    newWin->showMaximized();
            }
        }
    }
    TOCATCH
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
