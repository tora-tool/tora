
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

#ifndef TOWORKINGWIDGET_H
#define TOWORKINGWIDGET_H

#include <QWidget>

class QLabel;
class QPushButton;


/*! \brief Cool widget displayign the "waiting" status for queries.
See its usage in toResultTableView or toScriptSchemaWidget for example.
*/
class toWorkingWidget : public QWidget
{
    Q_OBJECT

    public:
        /*! Interactive allows user to stop running action
        (it has to be handled in code). NonInteractive disables
        the stop widget. */
        enum WorkingWidgetType
        {
            Interactive,
            NonInteractive
        };

        toWorkingWidget(QWidget * parent = 0);

        //! \brief Set the text for display label
        void setText(const QString & text);
        //! \brief Set the widget behaviour. See WorkingWidgetType enum.
        void setType(WorkingWidgetType type = Interactive);

    signals:
        //! \brief This is emitted when user requested "stop/cancel" action.
        void stop();

    public slots:
        /*! \brief Show this widget.
        It just prepare itself for showing if it's in the
        WorkingWidgetType Interactive mode (CurrentType). There has to be
        a forceShow() call connected to QTimer shot to show all
        internal widgets (see toResultTableView for example).
        The widget is shown normally if it's in the NonInteractive mode.
        */
        void show();
        /*! \brief Really show all subwidgets.
        See show() docs.
        */
        void forceShow();

    private:
        // Remember current internal type
        WorkingWidgetType CurrentType;

        // horizontal container for WorkingLabel and WorkingStop
        QWidget *HWorking;

        // label displayed by Working
        QLabel *WorkingLabel;

        // stop button displayed by Working
        QPushButton *WorkingStop;

    private slots:
        //! Handle the stop/cancel button press.
        void stopWorking();
};

#endif
