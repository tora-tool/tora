
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

#ifndef TOBROWSERBASEWIDGET_H
#define TOBROWSERBASEWIDGET_H

#include "core/tocache.h"

#include <QTabWidget>
#include <QtCore/QMap>
#include <QtCore/QPair>

#include "loki/Factory_alt.h"

class toResult;
class toResult2;

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
        QString schema()
        {
            return m_schema;
        };
        //! Returns type of currently used object.
        QString type()
        {
            return m_type;
        };
        //! Returns currently used object name.
        QString object()
        {
            return m_object;
        };

        /*! \brief Check all editable subwidgets for editation in progress.
        \retval bool true on "all is save". See toResultData::maybeSave().
        */
        bool maybeSave();

        /*! \brief Add given widget/page as a new tab with label.
        It adds page into m_tabs structure as well.
        \param page a toResult and QWidget inherited object.
        \param label a text label for tab caption.
        */
        virtual int addTab(QWidget * page, const QString & label);

        /*! \brief Set type of object being handled
        This is required for some result displaying classes (for example extractor). If
        type is not set TOra cannot distinguish which object should be used when
        different types of object use the same name.
        \param type type of object. Must be in uppercase and must match name of type used in database.
        */
        void setType(const QString & type);
    signals:
        void selected(const QString&);
    private:
        typedef QPair<toResult*,toResult2*> PRESULT; // intermediate class, till toResult is made obsolete

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
        QMap<QString, QPair<QString, QString> > m_cache;

        /*! Keep pages in this map. It's used in updateData()
        as a quick access shorcut.
        */
        QMap<QString, PRESULT> m_tabs;

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

/** Declare datatype for factory */
typedef Util::GenericFactory<toBrowserBaseWidget, LOKI_TYPELIST_1(QWidget *), toCache::CacheEntryType> toBrowserWidgetFact;
/** This singleton represents the only instance of the factory */
typedef Loki::SingletonHolder<toBrowserWidgetFact> toBrowserWidgetFactory;

#endif
