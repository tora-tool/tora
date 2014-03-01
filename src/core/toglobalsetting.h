
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

#ifndef TOGLOBALSETTING_H
#define TOGLOBALSETTING_H

#include "core/totool.h"
#include "core/toconfenum.h"

#include "ui_toglobalsettingui.h"
#include "ui_totoolsettingui.h"
#include "ui_connectioncolorsdialogui.h"

typedef QMap<QString, QString> ConnectionColors;
typedef QMapIterator<QString, QString> ConnectionColorsIterator;

class ConnectionColorsDialog : public QDialog, public Ui::ConnectionColorsDialog
{
    Q_OBJECT

public:
    ConnectionColorsDialog(QWidget * parent);

private:
    void newItem(const QString & color, const QString & desc);
private slots:
    void addItem();
    void deleteItem();
    void accept();
};

namespace ToConfiguration {
	class Global : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Global() : ConfigContext("Global", ENUM_REF(Global,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			// Paths
			CustomSQL = 2000        // #define CONF_SQL_FILE
			, HelpDirectory         // #define CONF_HELP_PATH
			, DefaultSession        // #define CONF_DEFAULT_SESSION
			, CacheDirectory        // #define CONF_CACHE_DIR
			, OracleHomeDirectory   // #define CONF_ORACLE_HOME
			, MysqlHomeDirectory    // #define CONF_MYSQL_HOME
			, PgsqlHomeDirectory    // #define CONF_PGSQL_HOME
			, GraphvizHomeDirectory
			// Options (1st column)
			, ChangeConnectionBool  // #define CONF_CHANGE_CONNECTION
			, SavePasswordBool      // #define CONF_SAVE_PWD
			, IncludeDbCaptionBool  // #define CONF_DB_TITLE
			, RestoreSessionBool    // #define CONF_RESTORE_SESSION
			, ToadBindingsBool      // #define CONF_TOAD_BINDINGS
			, CacheDiskBool         // #define CONF_CACHE_DISK
			, DisplayGridlinesBool  // #define CONF_DISPLAY_GRIDLINES
			, MultiLineResultsBool  // #define CONF_MULTI_LINE_RESULTS
			, MessageStatusbarBool  // #define CONF_MESSAGE_STATUSBAR
			, ColorizedConnectionsBool
			, ColorizedConnectionsMap
			// Options (2nd column)
			, StatusMessageInt      // #define CONF_STATUS_MESSAGE
			, HistorySizeInt        // #define CONF_STATUS_SAVE
			, ChartSamplesInt       // #define CONF_CHART_SAMPLES
			, DisplaySamplesInt     // #define CONF_DISPLAY_SAMPLES
			, SizeUnit              // #define CONF_SIZE_UNIT
			, RefreshIntervalInt    // #define CONF_REFRESH
			, DefaultListFormat     // #define CONF_DEFAULT_FORMAT
			, Style                 // #define CONF_STYLE
			, Translation           // #define CONF_LOCALE (Translation)
		};
		virtual QVariant defaultValue(int) const;
	};
};

class toGlobalSetting : public QWidget
    , public Ui::toGlobalSettingUI
    , public toSettingTab
{
    Q_OBJECT;

public:
	toGlobalSetting(QWidget* parent = 0, const char* name = 0,
			Qt::WFlags fl = 0);

public slots:
    virtual void saveSetting(void);

    virtual void sqlBrowse(void);
    virtual void helpBrowse(void);
    virtual void sessionBrowse(void);
    virtual void cacheBrowse(void);

    void oracleBrowse(void);
    void mysqlBrowse(void);
    void pqsqlBrowse(void);
    void graphvizBrowse(void);

    void ColorizedConnectionsConfigure_clicked();
};

class toToolSetting : public QWidget
    , public Ui::toToolSettingUI
    , public toSettingTab
{
    Q_OBJECT;

public:
    toToolSetting(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

public slots:
    virtual void saveSetting(void);
    virtual void changeEnable(void);
};

#endif
