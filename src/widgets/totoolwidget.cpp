
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

//
// C++ Implementation: toTool, toToolWidget, toConnectionWidget
//

#include "widgets/totoolwidget.h"
#include "core/totool.h"
#include "core/utils.h"
#include "core/tologger.h"

#include "core/toconnection.h"
#include "core/toconnectionregistry.h"
#include "widgets/toworkspace.h"
#include "core/tomainwindow.h"
#include "core/toconfiguration.h"
#include "core/toglobalevent.h"
#include "core/toglobalconfiguration.h"

#include <QToolBar>
#include <QMenu>
#include <QBoxLayout>
#include <QAction>
#include <QComboBox>
#include <QtDebug>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>

// A little magic to get lrefresh to work and get a check on qApp
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

toToolWidget::toToolWidget(toTool &tool, const QString &ctx, QWidget *parent, toConnection &conn, const char *name)
    : QWidget(parent),
      toHelpContext(ctx),
      toConnectionWidget(conn, this),
      Tool(tool),
      Action(NULL)
{
    if (name)
    {
        setObjectName(name);
        setCaption(name);
    }
    // make sure widget gets deleted
    setAttribute(Qt::WA_DeleteOnClose);
    // have to set the basic layout for widgets. It's requested later
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    connect(&toWorkSpaceSingle::Instance(), SIGNAL(activeToolChaged(toToolWidget*)), this, SLOT(slotWindowActivated(toToolWidget*)));
    connect(&toWorkSpaceSingle::Instance(), SIGNAL(activeToolChaged(toToolWidget*)), this, SLOT(toolActivated(toToolWidget*)));

    if (parent)
    {
        // Voodoo for making connection changing cascade to sub tools.
        try
        {
            toToolWidget *tool = toToolWidget::currentTool(parent);
            if (tool)
                connect(tool, SIGNAL(connectionChange()), this, SLOT(parentConnection()));
        }
        catch (std::exception const &e)
        {
            TLOG(1, toDecorator, __HERE__) << "       Ignored exception:" << std::endl
                                           << e.what() << std::endl;
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "       Ignored exception." << std::endl;
        }
    }
}

void toToolWidget::parentConnection(void)
{
    try
    {
        setConnection(toConnection::currentConnection(parentWidget()));
    }
    TOCATCH
}

toToolWidget::~toToolWidget()
{
    // nobody catches this anyway
    // toGlobalEventSingle::Instance().toolWidgetRemoved(this);
}

QString toToolWidget::currentSchema(QWidget *cur)
{
    while (cur)
    {
    	toToolWidget *widget = dynamic_cast<toToolWidget *>(cur);
        if (widget)
            return widget->schema();
        cur = cur->parentWidget();
    }
    TLOG(2, toDecorator, __HERE__) << "Couldn't find parent toToolWidget. Internal error." << std::endl;
    return "";
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
        setWindowIcon(Utils::connectionColorPixmap(conn.color()));
        emit toolCaptionChanged();
    }
    emit connectionChange();
    toGlobalEventSingle::Instance().setNeedCommit(this, this->hasTransaction());
}

bool toToolWidget::canHandle(const toConnection &conn)
{
    return Tool.canHandle(conn);
}

void toToolWidget::commitChanges()
{
    Q_ASSERT_X(false, qPrintable(__QHERE__), "Commit button should be disabled when tool does not support it");
}

void toToolWidget::rollbackChanges()
{
    Q_ASSERT_X(false, qPrintable(__QHERE__), "Rollback button should be disabled when tool does not support it");
}

QAction *toToolWidget::activationAction()
{
    if (Action == NULL)
    {
        Action = new QAction(QIcon(QPixmap(Tool.pictureXPM())), windowTitle(), this);
        Action->setCheckable(true);
    }
    return Action;
}

#ifdef TORA3_SESSION
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
#endif

void toToolWidget::setCaption(QString const& caption)
{
    QString title;
    if (toConfigurationNewSingle::Instance().option(ToConfiguration::Global::IncludeDbCaptionBool).toBool())
    {
        try
        {
            title = this->connection().description();
            title += QString::fromLatin1(" ");
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        }
    }
    title += caption;

    if (this->windowTitle() != title)
    {
        this->setWindowTitle(title);
        activationAction()->setText(title);
        emit toolCaptionChanged();
    }
}

#ifdef QT_DEBUG
void toToolWidget::focusInEvent(QFocusEvent *e)
{
    QWidget::focusInEvent(e);
}

void toToolWidget::enterEvent(QEvent *e)
{
    QWidget::enterEvent(e);
}

void toToolWidget::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
}

void toToolWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);
}
#endif

void toToolWidget::toolActivated(toToolWidget *tool)
{
    if (tool == this && toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ChangeConnectionBool).toBool())
        toConnectionRegistrySing::Instance().changeConnection(connection());
}

toToolWidget* toToolWidget::currentTool(QObject *cur)
{
    while (cur)
    {
        toToolWidget *tool = dynamic_cast<toToolWidget *>(cur);
        if (tool)
            return tool;
        cur = cur->parent();
    }
    throw qApp->translate("toCurrentTool", "Couldn't find parent tool. Internal error.");
}
