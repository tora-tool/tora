
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
#include "core/utils.h"
#include "editor/tostyle.h"
#include "parsing/tosyntaxanalyzer.h"
#include "ts_log/ts_log_utils.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>

QVariant ToConfiguration::Editor::defaultValue(int option) const
{
    switch (option)
    {
        case SyntaxHighlightingInt:
            return QVariant((int) 0);
        case EditorTypeInt:
            return QVariant((int) 0);
        case UseMaxTextWidthMarkBool:
            return QVariant((bool) false);
        case MaxTextWidthMarkInt:
            return QVariant((int)75);
        case KeywordUpperBool:
            return QVariant((bool) false);
        case ObjectNamesUpperBool:
            return QVariant((bool) false);
        case CodeCompleteBool:
            return QVariant((bool) true);
        case CompleteSortBool:
            return QVariant((bool) true);
        case CodeCompleteDelayInt:
        	return QVariant(500);
        case UseEditorShortcutsBool:
            return QVariant((bool) false);
        case EditorShortcutsMap:
            return QVariant(QMap<QString, QVariant>());
        case AutoIndentBool:
            return QVariant((bool) true);
        case UseSpacesForIndentBool:
            return QVariant((bool) false);
        case TabStopInt:
            return QVariant((int) 8);
        case ConfTextFont:
            return QVariant(QString(""));
        case ConfCodeFont:
            {
                QFont fo;
                QFont mono;
#if defined(Q_OS_WIN)
                mono = QFont("Courier New", 10);
#elif defined(Q_OS_MAC)
                mono = QFont("Courier", 12);
#else
                // TODO
#endif
                mono.setStyleHint(QFont::Monospace, QFont::NoAntialias);
                fo = mono.resolve(fo);
                QString fontName = fo.toString();

                return QVariant(fontName);
            }
        case ListTextFont:
            return QVariant(QString(""));
        case Extensions:
            return QVariant(QString("SQL (*.sql *.pkg *.pkb), Text (*.txt), All (*)"));
        case EditStyleMap:
            {
                static toStylesMap retval;
                if (!retval.isEmpty())
                    return QVariant::fromValue(retval);
                QMetaEnum StyleNameEnum(ENUM_REF(toSyntaxAnalyzer,WordClassEnum));
                QsciLexerSQL *l = new QsciLexerSQL(NULL);
                for (int idx = 0; idx < StyleNameEnum.keyCount(); idx++)
                {
                    QColor fg = l->color((int)StyleNameEnum.value(idx));
                    QColor bg = l->paper((int)StyleNameEnum.value(idx));
                    QFont fo = Utils::toStringToFont(defaultValue(ConfCodeFont).toString());

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

void ToConfiguration::Editor::saveUserType(QSettings &s, QVariant &val, int key) const
{
    Q_ASSERT_X( key == EditStyleMap && val.canConvert<toStylesMap>(), qPrintable(__QHERE__), qPrintable(QString("Unknown key to store: %1").arg(key)));
    QMetaEnum StyleNameEnum(ENUM_REF(toSyntaxAnalyzer,WordClassEnum));
    toStylesMap dMap = defaultValue(EditStyleMap).value<toStylesMap>();
    toStylesMap wMap = val.value<toStylesMap>();
    Q_FOREACH(int k, wMap.keys())
    {
        const char* i = StyleNameEnum.valueToKey(k);
        if (i != NULL)
        {
            QString keyNameFg = QString(i) + "Fg";
            QColor confFg = wMap.value(k).FGColor;
            QColor defaFg = dMap.value(k).FGColor;
            if ( confFg != defaFg)
                s.setValue(keyNameFg, confFg.name());

            QString keyNameBg = QString(i) + "Bg";
            QColor confBg = wMap.value(k).BGColor;
            QColor defaBg = dMap.value(k).BGColor;
            if ( confBg != defaBg)
                s.setValue(keyNameBg, confBg.name());

            QString keyNameFo = QString(i) + "Fo";
            QFont  confFo = wMap.value(k).Font;
            QFont  defaFo = dMap.value(k).Font;
            if ( confFo.toString() != defaFo.toString())
                s.setValue(keyNameFo, confFo.toString());
        }
    }
};

void ToConfiguration::Editor::loadUserType(QSettings &s, QVariant &val, int key) const
{
    Q_ASSERT_X( key == EditStyleMap, qPrintable(__QHERE__), qPrintable(QString("Unknown key to store: %1").arg(key)));
    QMetaEnum StyleNameEnum(ENUM_REF(toSyntaxAnalyzer,WordClassEnum));
    toStylesMap dMap = defaultValue(EditStyleMap).value<toStylesMap>();
    Q_FOREACH(int k, dMap.keys())
    {
        const char* i = StyleNameEnum.valueToKey(k);
        if (i != NULL)
        {
            toStyle style = dMap.value(k);
            QString keyNameFg = QString(i) + "Fg";
            QColor confFg(s.value(keyNameFg).toString());
            if (confFg.isValid())
                style.FGColor = confFg;

            QString keyNameBg = QString(i) + "Bg";
            QColor confBg(s.value(keyNameBg).toString());
            if ( confBg.isValid())
                style.BGColor = confBg;

            QString keyNameFo = QString(i) + "Fo";
            QString confFo = s.value(keyNameFo).toString();
            if (!confFo.isEmpty() && QFont(confFo).exactMatch())
                style.Font = QFont(confFo);

            dMap.insert(k, style);
        }
    }
    val = QVariant::fromValue(dMap);
}

ToConfiguration::Editor s_editorConfig;
