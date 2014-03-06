
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

#include "core/toeditorsetting.h"
#include "editor/tostyle.h"
#include "parsing/tosyntaxanalyzer.h"
#include "ts_log/ts_log_utils.h"

#include <QtCore/QDebug>

QVariant ToConfiguration::Editor::defaultValue(int option) const
{
	switch(option)
	{
	case SyntaxHighlightingInt:          return QVariant((int) 0);
	case EditorTypeInt:               return QVariant((int) 0);
	case UseMaxTextWidthMarkBool:      return QVariant((bool) false);
	case MaxTextWidthMarkInt:         return QVariant((int)75);
	case KeywordUpperBool:             return QVariant((bool) false);
	case ObjectNamesUpperBool:         return QVariant((bool) false);
	case CodeCompleteBool:             return QVariant((bool) true);
	case CompleteSortBool:             return QVariant((bool) true);
	case UseEditorShortcutsBool:       return QVariant((bool) false);
	case EditorShortcutsMap:    return QVariant(QMap<QString, QVariant>());
	case AutoIndentBool:               return QVariant((bool) true);
	case UseSpacesForIndentBool:                return QVariant((bool) false);
	case TabStopInt:                  return QVariant((int) 8);
	case ConfTextFont:                 return QVariant(QString(""));
	case ConfCodeFont:                 return QVariant(QString(""));
	case ListTextFont:                 return QVariant(QString(""));
	case Extensions:               return QVariant(QString("SQL (*.sql *.pkg *.pkb), Text (*.txt), All (*)"));
	case EditStyleMap:
	{
		toStylesMap retval;
		QMetaEnum StyleNameEnum(ENUM_REF(toSyntaxAnalyzer,WordClassEnum));
		QsciLexerSQL *l = new QsciLexerSQL(NULL);
		for (int idx = 0; idx < StyleNameEnum.keyCount(); idx++)
		{
			QString keyNameFg = QString::fromAscii("EditStyle") + StyleNameEnum.key(idx) + "Fg";
			QString keyNameBg = QString::fromAscii("EditStyle") + StyleNameEnum.key(idx) + "Bg";
			QString keyNameFo = QString::fromAscii("EditStyle") + StyleNameEnum.key(idx) + "Fo";
			QColor fg, bg;
			QFont fo;
			fg = l->color((int)StyleNameEnum.value(idx));
			bg = l->paper((int)StyleNameEnum.value(idx));
			fo = QFont("");

			QString styleName = StyleNameEnum.key(idx);
			int styleNameEnum = StyleNameEnum.value(idx);
			retval.insert(styleNameEnum, toStyle(fg, bg, fo));
		}
		delete l;
		return QVariant::fromValue(retval);
	}
	default:
		Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Editor un-registered enum value: %1").arg(option)));
		return QVariant();
	}
}
