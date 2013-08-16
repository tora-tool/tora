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

#include <QtGui/QApplication>

#include "core/utils.h"
#include "core/tosyntaxanalyzer.h"
#include "core/todefaultkeywords.h"
#include "core/toconfiguration.h"


toSyntaxAnalyzer::toSyntaxAnalyzer(const char **keywords)
{
    for (int i = 0; keywords[i]; i++)
    {
        std::list<const char *> &curKey = Keywords[(unsigned char)char(toupper(*keywords[i]))];
        curKey.insert(curKey.end(), keywords[i]);
    }
    ColorsUpdated = false;
}

toSyntaxAnalyzer::posibleHit::posibleHit(const char *text)
{
    Pos = 1;
    Text = text;
}

QColor toSyntaxAnalyzer::getColor(toSyntaxAnalyzer::infoType typ)
{
    if (!ColorsUpdated)
    {
        updateSettings();
        ColorsUpdated = true;
    }
    return Colors[typ];
}

/*
#define ISIDENT(c) (isalnum(c)||(c)=='_'||(c)=='%'||(c)=='$'||(c)=='#')

std::list<toSyntaxAnalyzer::highlightInfo> toSyntaxAnalyzer::analyzeLine(const QString &str,
        toSyntaxAnalyzer::infoType in,
        toSyntaxAnalyzer::infoType &out)
{
    std::list<highlightInfo> highs;
    std::list<posibleHit> search;

    bool inWord;
    bool wasWord = false;
    int multiComment = -1;
    int inString = -1;
    QChar endString;

    if (in == String)
    {
        inString = 0;
        endString = '\'';
    }
    else if (in == Comment)
    {
        multiComment = 0;
    }

    char c;
    char nc = str[0].toLatin1();
    for (int i = 0;i < int(str.length());i++)
    {
        std::list<posibleHit>::iterator j = search.begin();

        c = nc;
        if (int(str.length()) > i)
            nc = str[i + 1].toLatin1();
        else
            nc = ' ';

        bool nextSymbol = ISIDENT(nc);
        if (multiComment >= 0)
        {
            if (c == '*' && nc == '/')
            {
                highs.insert(highs.end(), highlightInfo(multiComment, Comment));
                highs.insert(highs.end(), highlightInfo(i + 2));
                multiComment = -1;
            }
        }
        else if (inString >= 0)
        {
            if (c == endString)
            {
                highs.insert(highs.end(), highlightInfo(inString, String));
                highs.insert(highs.end(), highlightInfo(i + 1));
                inString = -1;
            }
        }
        else if (c == '\'' || c == '\"')
        {
            inString = i;
            endString = str[i];
            search.clear();
            wasWord = false;
        }
        else if (c == '-' && nc == '-')
        {
            highs.insert(highs.end(), highlightInfo(i, Comment));
            highs.insert(highs.end(), highlightInfo(str.length() + 1));
            out = Default;
            return highs;
        }
        else if (c == '/' && nc == '/')
        {
            highs.insert(highs.end(), highlightInfo(i, Comment));
            highs.insert(highs.end(), highlightInfo(str.length() + 1));
            out = Default;
            return highs;
        }
        else if (c == '/' && nc == '*')
        {
            multiComment = i;
            search.clear();
            wasWord = false;
        }
        else
        {
            std::list<posibleHit> newHits;
            while (j != search.end())
            {
                posibleHit &cur = (*j);
                if (cur.Text[cur.Pos] == toupper(c))
                {
                    cur.Pos++;
                    if (!cur.Text[cur.Pos] && !nextSymbol)
                    {
                        newHits.clear();
                        highs.insert(highs.end(), highlightInfo(i - cur.Pos, Keyword));
                        highs.insert(highs.end(), highlightInfo(i + 1));
                        break;
                    }
                    newHits.insert(newHits.end(), cur);
                }
                j++;
            }
            search = newHits;
            if (ISIDENT(c))
                inWord = true;
            else
                inWord = false;

            if (!wasWord && inWord)
            {
                std::list<const char *> &curKey = Keywords[(unsigned char)char(toupper(c))];
                for (std::list<const char *>::iterator j = curKey.begin();
                        j != curKey.end();j++)
                {
                    if (strlen(*j) == 1)
                    {
                        if (!nextSymbol)
                        {
                            highs.insert(highs.end(), highlightInfo(i, Keyword));
                            highs.insert(highs.end(), highlightInfo(i));
                        }
                    }
                    else
                        search.insert(search.end(), posibleHit(*j));
                }
            }
            wasWord = inWord;
        }
    }
    if (inString >= 0)
    {
        if (endString == '\'')
        {
            out = String;
            highs.insert(highs.end(), highlightInfo(inString, String));
        }
        else
        {
            out = Default;
            //highs.insert(highs.end(), highlightInfo(inString, Error));
            highs.insert(highs.end(), highlightInfo(inString, Comment));
        }
        highs.insert(highs.end(), highlightInfo(str.length() + 1));
    }
    else if (multiComment >= 0)
    {
        highs.insert(highs.end(), highlightInfo(multiComment, Comment));
        highs.insert(highs.end(), highlightInfo(str.length() + 1));
        out = Comment;
    }
    else
        out = Default;

    return highs;
}
*/

toSyntaxAnalyzer::infoType toSyntaxAnalyzer::typeString(const QString &str)
{
    if (str == "Default")
        return Default;
    if (str == "Comment")
        return Comment;
    if (str == "Number")
        return Number;
    if (str == "Keyword")
        return Keyword;
    if (str == "String")
        return String;
    if (str == "Background")
        return DefaultBg;
    if (str == "Error background")
        return ErrorBg;
    if (str == "Debug background")
        return DebugBg;
    if (str == "Current line highlight")
        return CurrentLineMarker;
    if (str == "Static check background")
        return StaticBg;
    throw qApp->translate("toSyntaxAnalyzer", "Unknown type");
}

QString toSyntaxAnalyzer::typeString(infoType typ)
{
    switch (typ)
    {
    case Default:
        return "Default";
    case Comment:
        return "Comment";
    case Number:
        return "Number";
    case Keyword:
        return "Keyword";
    case String:
        return "String";
    case DefaultBg:
        return "Background";
    case ErrorBg:
        return "Error background";
    case DebugBg:
        return "Debug background";
    case CurrentLineMarker:
        return "Current line highlight";
    case StaticBg:
        return "Static check background";
    }
    throw qApp->translate("toSyntaxAnalyzer", "Unknown type");
}

void toSyntaxAnalyzer::updateSettings(void)
{
    try
    {
        Colors[Default] = toConfigurationSingle::Instance().syntaxDefault();
        Colors[Comment] = toConfigurationSingle::Instance().syntaxComment();
        Colors[Number] = toConfigurationSingle::Instance().syntaxNumber();
        Colors[Keyword] = toConfigurationSingle::Instance().syntaxKeyword();
        Colors[String] = toConfigurationSingle::Instance().syntaxString();
        Colors[DefaultBg] = toConfigurationSingle::Instance().syntaxDefaultBg();
        Colors[ErrorBg] = toConfigurationSingle::Instance().syntaxErrorBg();
        Colors[DebugBg] = toConfigurationSingle::Instance().syntaxDebugBg();
        Colors[CurrentLineMarker] = toConfigurationSingle::Instance().syntaxCurrentLineMarker();
        Colors[StaticBg] = toConfigurationSingle::Instance().syntaxStaticBg();
    }
    TOCATCH
}



toSyntaxAnalyzer &toSyntaxAnalyzer::defaultAnalyzer(void)
{
    return DefaultAnalyzer;
}

bool toSyntaxAnalyzer::reservedWord(const QString &str)
{
    if (str.length() == 0)
        return false;
    QString t = str.toUpper();
    std::list<const char *> &curKey = Keywords[(unsigned char)char(str[0].toLatin1())];
    for (std::list<const char *>::iterator i = curKey.begin(); i != curKey.end(); i++)
        if (t == (*i))
            return true;
    return false;
}

