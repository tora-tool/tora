/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef __TOCONF_H
#define __TOCONF_H

#define CONF_SAVE_PWD		"SavePassword"
#define DEFAULT_SAVE_PWD	""
#define CONF_PASSWORD		"DefaultPassword"
#define DEFAULT_PASSWORD	"manager"
#define CONF_DATABASE		"DefaultDatabase"
#define DEFAULT_DATABASE	""
#define CONF_USER		"DefaultUser"
#define DEFAULT_USER		"system"
#define CONF_SQLNET		"SqlNet"
#define CONF_MAX_NUMBER		"MaxNumber"
#define DEFAULT_MAX_NUMBER	"50"
#define CONF_MAX_COL_SIZE	"MaxColSize"
#define DEFAULT_MAX_COL_SIZE	"30000"
#define CONF_MAX_COL_DISP	"MaxColDisp"
#define DEFAULT_MAX_COL_DISP	"300"
#define CONF_PLAN_TABLE		"PlanTable"
#define DEFAULT_PLAN_TABLE	"PLAN_TABLE"
#define CONF_PLAN_CHECKPOINT	"PlanCheckpoint"
#define DEFAULT_PLAN_CHECKPOINT	"TORA_CHECKPOINT"
#define CONF_TEXT		"ConfText"
#define CONF_DATE_FORMAT	"ConfDateFormat"
#define DEFAULT_DATE_FORMAT	"YYYY-MM-DD HH24:MI:SS"
#define CONF_REFRESH		"Refresh"
#define DEFAULT_REFRESH		"None"
#define CONF_HIGHLIGHT		"SyntaxHighlight"
#define CONF_KEYWORD_UPPER	"KeywordUpper"
#define CONF_COLOR		"KeywordColor"
#define CONF_PLUGIN_DIR		"PluginDir"
#define CONF_SQL_FILE		"SQLDictionary"
#define DEFAULT_SQL_FILE	"$HOME/.torasql"
#define CONF_LONG_SESSION	"LongSeparate"
#define CONF_LIST_SCALE		"ListScale"
#define DEFAULT_LIST_SCALE	"0.8"
#define CONF_DESKTOP_AWARE	"DesktopAware"
#define CONF_STATUS_MESSAGE     "StatusMessage"
#define DEFAULT_STATUS_MESSAGE  "5"
#define CONF_QT_XFT             "Xft"
#define DEFAULT_QT_XFT          "false"
#define CONF_DOCK_TOOLBAR	"DockToolbar"

// Undefining this will leave out the style selection option stuff from the program
#ifndef TO_KDE
#define ENABLE_STYLE
#define ENABLE_QT_XFT
#endif
#define CONF_STYLE		"Style"
#define DEFAULT_STYLE		"Windows"
#ifdef WIN32
#define TOMONOLITHIC
#define DEFAULT_PLUGIN_DIR ""
#endif
#define TOVERSION               "0.10"

#endif
