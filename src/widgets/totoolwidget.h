
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

#ifndef __TOTOOLWIDGET_H__
#define __TOTOOLWIDGET_H__

#include "core/tohelpcontext.h"
#include "widgets/toconnectionwidget.h"

#include <loki/Singleton.h>

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QWidget>

class toTool;

/** Simple baseclass for widgets defining the main tool widget. It is in
 * no way mandatory and all it does is register the widget in the connetion.
 *
 * Tools are common QWidget based objects. The main window - the MDI area -
 * requires them wrapped as QMdiSubWindows. So the tool widget is set as widget()
 * in the toToolWidget constructor.
 *
 * \note: Note that all access to tool widget is done via QMdiSubWindow::widget() in
 * the code. See all available tools' windowActivated() slots for examples.
 *
 * \warning Remember that windowActivated() slot called by toMainWidget()->workspace(),
 * SIGNAL(subWindowActivated(QMdiSubWindow *) has to handle "0" as input parameter
 * so an appropriate test is mandatory.
 */
class toToolWidget : public QWidget
    , public toHelpContext
    , public toConnectionWidget
{
        Q_OBJECT;
    signals:
        /** Emitted when the connection is changed.
         */
        void connectionChange(void);

        /** Emmited when tool window title is changed
         */
        void toolCaptionChanged();

    public:
        /** Create widget and its QMdiSubWindow.
         * @param ctx Help context for this tool.
         * @param parent Parent widget.
         * @param conn Connection of widget.
         * @param name Name of widget.
         */
        toToolWidget(toTool &tool, const QString &ctx, QWidget *parent, toConnection &conn, const char *name = NULL);

        ~toToolWidget();

        /** Get the current connection.
         * @return Reference to connection.
         */
        toConnection & connection()
        {
            return toConnectionWidget::connection();
        }
        toConnection const& connection() const
        {
            return toConnectionWidget::connection();
        }

        virtual QString schema() const
        {
        	return "";
        }

        /** Return the schema name most closely associated with a widget.
         *   Subclass of toToolWidget has to override schema() method
         * @return QString schema name closest to the current.
         */
        static QString currentSchema(QWidget *cur);

        /** Change connection of tool.
         */
        void setConnection(toConnection &conn);

        virtual bool hasTransaction() const
        {
            return false; // by default tool does not support transactions
        }

        /** Get the tool for this tool widget.
         * @return Reference to a tool object.
         */
        toTool& tool(void)
        {
            return Tool;
        }

        /** Check if this tool can handle a specific connection.
         * @param provider Name of connection.
         * @return True if connection is handled.
         */
        virtual bool canHandle(const toConnection &conn);

        void commitChanges();
        void rollbackChanges();

        QAction *activationAction();

#ifdef TORA3_SESSION
        /** Export data to a map.
         * @param data A map that can be used to recreate the data of a chart.
         * @param prefix Prefix to add to the map.
         */
        virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
        /** Import data
         * @param data Data to read from a map.
         * @param prefix Prefix to read data from.
         */
        virtual void importData(std::map<QString, QString> &data, const QString &prefix);
#endif

#ifdef QT_DEBUG
        /* Inherited from QWidget - for debuging purposes only - buggy MDI */
        virtual void focusInEvent(QFocusEvent *);
        virtual void enterEvent(QEvent *);
        virtual void paintEvent(QPaintEvent *);
        virtual void setVisible(bool visible);
#endif
        /** Return the tool widget most closely associated with a widget.
        * @return Pointer to tool widget.
        */
        static toToolWidget* currentTool(QObject *widget);

    protected:
        /* Set or change title of a tool window.
         * @param caption Caption to set to the tool.
         */
        void setCaption(QString const& caption);

    private slots:
        void parentConnection(void);
        virtual void slotWindowActivated(toToolWidget*) = 0;
        void toolActivated(toToolWidget*);
    private:
        toTool &Tool;
        QAction *Action;
};

#endif
