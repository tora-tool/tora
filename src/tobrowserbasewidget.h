
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

#ifndef TOBROWSERBASEWIDGET_H
#define TOBROWSERBASEWIDGET_H

#include <QTabWidget>
#include <QMap>

class toResult;


/*! \brief Base class for all toBrowser data viewers.
All "browsers" (the tabs with data, columns, scripts etc.)
are inherited from QTabWidget.

Tab's widgets are TOra objects only inherited from toResult
class.

This base class provides common handling of the tab-switching
and view-updating. The updating is cached - see m_cache.

See e.g. toBrowserTableWidget etc. as references.

\author Petr Vanek <petr@scribus.info>
*/
class toBrowserBaseWidget : public QTabWidget
{
    Q_OBJECT

    public:
        //! \param parent Parent cannot be NULL/0 to handle toSQL correctly!
        toBrowserBaseWidget(QWidget * parent);

        /*! \brief Update internal schema and object name.
        Update data if it's different from previously used schema/object.
        See updateData().
        */
        virtual void changeParams(const QString & schema, const QString & object, const QString & type = "");

        /*! \brief Change connection for widget. It has to handle visibility
        of tabs depending on DB features.
        This method should be reimplemented in all child classes to customize
        its impact on child's tab members. All reimplementations should call
        parent's changeConnection() to clear the cache;
        \warning If you don't call toBrowserBaseWidget::changeConnection(),
                 the m_tabs cleaning, before new addTab() you can get assertions.
        */
        virtual void changeConnection();

        //! Returns currently used schema.
        QString schema() { return m_schema; };
        //! Returns type of currently used object.
        QString type() { return m_type; };
        //! Returns currently used object name.
        QString object() { return m_object; };

        /*! \brief Check all editable subwidgets for editation in progress.
        \retval bool true on "all is save". See toResultData::maybeSave().
        */
        bool maybeSave();

        /*! \brief Add given widget/page as a new tab with label.
        It adds page into m_tabs structure as well.
        \param page a toResult and QWidget inherited object.
        \param label a text label for tab caption.
        */
        virtual void addTab(QWidget * page, const QString & label);

    private:
        QString m_schema;
        QString m_type;
        QString m_object;

        /*! Some kind of cache to save some fetching from DB.
        Content of this map is checked everytime user changes
        current tab (see tabWidget_currentChanged()).
        The updateData(), the real updating routine, is called
        only when there is no index-reference or the m_schema
        or m_object have been changed from outside.
        Structure:
         - QString is the currentTab's widget objectName for QTabWidget access.
         - QPair's string1 cached schema name
         - QPair's string2 cached object name
        */
        QMap<QString, QPair<QString,QString> > m_cache;

        /*! Keep pages in this map. It's used in updateData()
        as a quick access shorcut.
        */
        QMap<QString, toResult*> m_tabs;

        /*! \brief Performs data refresh.
        Call toResult child its changeParams() if it's really needed.
        \param ix index of the required tab/widget. See m_tabs.
        */
        void updateData(const QString & ix);

    protected slots:
        /*! \brief Handle current tab change.
        It updates the m_cache cache structure too.
        It calls updateData() if it's required
        */
        virtual void tabWidget_currentChanged(int ix);
};

#endif
