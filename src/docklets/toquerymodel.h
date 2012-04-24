
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

#ifndef TOQUERYMODEL_H
#define TOQUERYMODEL_H

//#include "config.h"
#include "todocklet.h"

#include "toeditwidget.h"

#include <QSortFilterProxyModel>

class DotGraphView;
class QTimerEvent;

class toQueryModel : public toDocklet
{
    Q_OBJECT;

    class editHandlerHolder: public toEditWidget::editHandler
    {
    public:
		editHandlerHolder() : m_current(NULL) {};
	    virtual ~editHandlerHolder() {};
	    virtual void receivedFocus(toEditWidget *widget)
	    {
		    m_current = widget;
		    
	    }
	    virtual void lostFocus(toEditWidget *widget)
	    {
		    m_current = NULL;
	    }
	    toEditWidget *m_current;       	    
    };
    
private:
    DotGraphView *m_widget;

    editHandlerHolder *m_currentEditor;
	QString m_lastText;
    int m_timerID;
public:    
    toQueryModel(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    /**
     * Get the action icon name for this docklet
     *
     */
    virtual QIcon icon() const;

    /**
     * Get the docklet's name
     *
     */
    virtual QString name() const;

    virtual void timerEvent(QTimerEvent *e);
public slots:
    void describeSlot(void);
};


#endif
