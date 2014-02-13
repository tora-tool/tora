
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

#ifndef TOSGATRACE_H
#define TOSGATRACE_H

#include "core/totool.h"
#include "core/toconfenum.h"
#include "ts_log/ts_log_utils.h"

#include <QtCore/QString>

#define TOSQL_LONGOPS "toSGATrace:LongOps"

class QComboBox;
class QMenu;
class QTabWidget;
class toConnection;
class toMain;
class toResultSchema;
class toResultTableView;
class toSGAStatement;
class toTool;

namespace ToConfiguration
{
	class SgaTrace : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		SgaTrace() : ConfigContext("SgaTrace", ENUM_REF(SgaTrace,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			AutoUpdate   = 11000  // #define CONF_AUTO_UPDATE
		};
		QVariant defaultValue(int option) const
		{
			switch(option)
			{
			case AutoUpdate: return QVariant((bool)true);
			default:
				Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Editor un-registered enum value: %1").arg(option)));
				return QVariant();
			}
		}
	};
};

class toSGATrace : public toToolWidget
{
    Q_OBJECT;
public:
    toSGATrace(QWidget *parent, toConnection &connection);

public slots:
    void changeSchema(const QString &str);
    void changeItem(void);
    void changeRefresh(const QString &str);
    void refresh(void);
private:
    virtual void slotWindowActivated(toToolWidget*) {};
    
    toResultTableView *Trace;
    QTabWidget *ResultTab;

    QAction       *FetchAct;
    toResultSchema *Schema;
    QComboBox     *Type;
    QComboBox     *Refresh;
    QComboBox     *Limit;

    toSGAStatement *Statement;
    QString CurrentSchema;

    void updateSchemas(void);    
};

#endif
