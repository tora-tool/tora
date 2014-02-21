
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

#include "core/totool.h"
#include "core/utils.h"
#include "core/tologger.h"
#include "core/toconfiguration_new.h"
#include "core/toconnection.h"
#include "core/toconnectionregistry.h"
#include "core/toworkspace.h"
#include "core/totimer.h"
#include "core/tomainwindow.h"
#include "core/toconfiguration.h"
#include "core/toglobalevent.h"

#include <QtGui/QToolBar>
#include <QtGui/QMenu>
#include <QtGui/QBoxLayout>
#include <QtGui/QAction>

// A little magic to get lrefresh to work and get a check on qApp
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

QString const& toTool::name() const
{
	return Name;
}

QString const& toTool::key() const
{
    return Key;
}

const int toTool::priority() const
{
    return Priority;
}

void toSettingTab::processChildWidgets(QWidget *widget)
{
	static QRegExp any(".*");
	QList<QWidget*> lst = widget->findChildren<QWidget*>(any);
	Q_FOREACH(QWidget *w, lst)
	{
		qDebug() << w->objectName();
		if (w->objectName() == "qt_spinbox_lineedit") // internal widget inside QSpinBox
			continue;
		if (QComboBox *combo = qobject_cast<QComboBox*>(w))
		{
			try
			{
				QVariant v = toConfigurationNewSingle::Instance().option(combo->objectName());
			} catch (...) {
				qDebug() << w->objectName() << '*';
				combo->setDisabled(true);
			}
		} else if (QSpinBox *spin = qobject_cast<QSpinBox*>(w))
		{
			try
			{
				QVariant v = toConfigurationNewSingle::Instance().option(spin->objectName());
				if (v.type() == QVariant::Int)
				{
					spin->setValue(v.toInt());
				} else {
					spin->setDisabled(true);
				}
			} catch (...) {
				qDebug() << w->objectName() << '#';
				spin->setDisabled(true);
			}
		} else if (QLineEdit *edit = qobject_cast<QLineEdit*>(w))
		{
			try
			{
				QVariant v = toConfigurationNewSingle::Instance().option(edit->objectName());
			} catch (...) {
				qDebug() << w->objectName() << '&';
				edit->setDisabled(true);
			}
		} else if (QCheckBox *checkbox = qobject_cast<QCheckBox*>(w))
		{
			try
			{
				QVariant v = toConfigurationNewSingle::Instance().option(checkbox->objectName());
				if (v.type() == QVariant::Bool)
				{
					checkbox->setChecked(v.toBool());
				} else {
					checkbox->setDisabled(true);
				}
			} catch (...) {
				qDebug() << w->objectName() << '%';
				checkbox->setDisabled(true);
			}
		}

	}
}

toConnectionWidget::toConnectionWidget(toConnection &conn, QWidget *widget)
    : Connection(&conn), Widget(widget)
{
    Connection->addWidget(Widget);
}

toConnectionWidget::toConnectionWidget(QWidget *widget)
    : Widget(widget)
{
    Connection = &toConnection::currentConnection(widget->parentWidget());
}

void toConnectionWidget::setConnection(toConnection &conn)
{
	toConnection *oldConnection(Connection);
    Connection = &conn;               // 1st change "registration" to toConnection
	oldConnection->delWidget(Widget); // then notify old connection about the change
    Connection->addWidget(Widget);
    if (toConfigurationSingle::Instance().changeConnection())
    	toConnectionRegistrySing::Instance().changeConnection(conn);
}

toConnectionWidget::~toConnectionWidget()
{
	Connection->delWidget(Widget);
}

toConnection& toConnectionWidget::connection()
{
    if (Connection)
        return *Connection;
    if (!Widget)
        throw qApp->translate("toConnectionWidget", "toConnectionWidget not inherited with a QWidget");
    return toConnection::currentConnection(Widget->parentWidget());
}

const toConnection& toConnectionWidget::connection() const
{
	return *Connection;
}

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

    Timer = NULL;

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

toTimer *toToolWidget::timer(void)
{
    if (!Timer)
        Timer = new toTimer(this);
    return Timer;
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
		Action = new QAction(QIcon(Tool.pictureXPM()), windowTitle(), this);
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
    if (toConfigurationSingle::Instance().includeDbCaption())
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

    if(this->windowTitle() != title)
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
	if (tool == this && toConfigurationSingle::Instance().changeConnection())
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
    Key.sprintf("%05d%s", priority, name);
    ToolsRegistrySing::Instance().insert(Key, this);
}

toTool::~toTool()
{
    ToolsRegistrySing::Instance().erase(ToolsRegistrySing::Instance().find(Key));
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

toToolWidget* toTool::createWindow()
{	
	toToolWidget *newWin = NULL;
	try
	{
		toConnection &conn = toConnectionRegistrySing::Instance().currentConnection();

		if (!canHandle(conn))
			throw QString("The tool %1 doesn't support the current database").arg(QString(name()));
		
		newWin = toolWindow(NULL, conn);
		// Some tools like toOutput return NULL if there is one window for session opened
		if (newWin)
			toWorkSpaceSingle::Instance().addToolWidget(newWin);
	}
	TOCATCH;

	return newWin;
}


bool toTool::canHandle(const toConnection &conn)
{
#ifdef TORA3_CONNECTION
    return (conn.providerIs("Oracle"));
#endif
    return true;
}

QAction* toTool::getAction(void)
{
    if (toolAction)
        return toolAction;

    toolAction = new QAction(toMainWindow::lookup());
    if (toolbarImage())
        toolAction->setIcon(QIcon(QPixmap(pictureXPM())));
    if (menuItem())
        toolAction->setText(menuItem());
    if (toolbarTip())
        toolAction->setToolTip(toolbarTip());

    connect(toolAction, SIGNAL(triggered()), this, SLOT(createWindow()));

    return toolAction;
}

void toTool::enableAction(bool en)
{
    getAction()->setEnabled(en);
}

void toTool::enableAction(toConnection const& conn)
{
    enableAction(canHandle(conn));
}

void toTool::setActionVisible(toConnection &conn)
{
    getAction()->setVisible(canHandle(conn));
}

void toTool::setActionVisible(bool en)
{
    getAction()->setVisible(en);
}

QToolBar* toToolsRegistry::toolsToolbar(QToolBar *toolbar) const
{
    if(!toolbar)
        return toolbar;

    try
    {
        int lastPriorityPix = 0;
        ToolsMap &cfgTools = toConfigurationSingle::Instance().tools();

        for (super::const_iterator i = super::begin(); i != super::end(); ++i)
        {
            toTool *pTool = i.value();
            QAction *toolAct = pTool->getAction();
            const QPixmap *pixmap = pTool->toolbarImage();

            // set the tools for the first run
            if (!cfgTools.contains(i.key()))
                cfgTools[i.key()] = true;
            // only enabled tools are set
            if (cfgTools[i.key()] == false)
                continue;

            int priority = pTool->priority();
            if (priority / 100 != lastPriorityPix / 100 && pixmap)
            {
                toolbar->addSeparator();
                lastPriorityPix = priority;
            }

            if (pixmap)
                toolbar->addAction(toolAct);

        } // for tools
    }
    TOCATCH;
    return toolbar;
}

QMenu* toToolsRegistry::toolsMenu(QMenu *menu) const
{
    if(!menu)
        return menu;

    try
    {
        int lastPriorityMenu = 0;

        ToolsMap &cfgTools = toConfigurationSingle::Instance().tools();

        for (super::const_iterator i = super::begin(); i != super::end(); ++i)
        {
            toTool *pTool = i.value();
            QAction *toolAct = pTool->getAction();
            const QPixmap *pixmap = pTool->toolbarImage();
            const char *menuName = pTool->menuItem();

            // set the tools for the first run
            if (!cfgTools.contains(i.key()))
                cfgTools[i.key()] = true;
            // only enabled tools are set
            if (cfgTools[i.key()] == false)
                continue;

            int priority = pTool->priority();
            if (priority / 100 != lastPriorityMenu / 100 && menuName)
            {
                menu->addSeparator();
                lastPriorityMenu = priority;
            }

            if (menuName)
                menu->addAction(toolAct);
        } // for tools
    }
    TOCATCH;
    return menu;
}
