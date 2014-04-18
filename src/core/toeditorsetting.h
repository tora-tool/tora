
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

#ifndef TOEDITORSETTING_H
#define TOEDITORSETTING_H

#include "core/toconfenum.h"

#include <QtCore/QVariant>
#include <QtGui/QColor>
#include <QtGui/QFont>

namespace ToConfiguration
{
	class Editor : public ConfigContext
	{
		Q_OBJECT;
		Q_ENUMS(OptionTypeEnum);
	public:
		Editor() : ConfigContext("Editor", ENUM_REF(Editor,OptionTypeEnum)) {};
		enum OptionTypeEnum {
			SyntaxHighlightingInt  = 3000 // #define CONF_HIGHLIGHT
			, EditorTypeInt            // #define CONF_EDITOR_TYPE
			, UseMaxTextWidthMarkBool
			, MaxTextWidthMarkInt
			, KeywordUpperBool      // #define CONF_KEYWORD_UPPER
			, ObjectNamesUpperBool  // #define CONF_OBJECT_NAMES_UPPER
			, CodeCompleteBool      // #define CONF_CODE_COMPLETION
			, CompleteSortBool      // #define CONF_COMPLETION_SORT
			, UseEditorShortcutsBool
			, EditorShortcutsMap
			, AutoIndentBool        // #define CONF_AUTO_INDENT
			, UseSpacesForIndentBool// #define CONF_TAB_SPACES
			, TabStopInt            // #define CONF_TAB_STOP
			, ConfTextFont          // #define CONF_TEXT
			, ConfCodeFont          // #define CONF_CODE
			, ListTextFont          // #define CONF_LIST
			, Extensions            // #define CONF_EXTENSIONS
			// 2nd tab
			, EditStyleMap          // #define CONF_EDIT_STYLE
		};
		virtual QVariant defaultValue(int option) const;

		virtual void saveUserType(QSettings &s, QVariant &val, int key) const;

	};
}

#endif
