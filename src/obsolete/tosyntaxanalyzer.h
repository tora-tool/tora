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

#ifndef TOSYNTAXANALYZER_H
#define TOSYNTAXANALYZER_H

#include "todefaultkeywords.h"

#include <QtGui/QMap>
#include <QtGui/QColor>
#include <QtGui/QString>

/** This class implements a syntax parser to provide information to
 * a syntax highlighted editor.
 */
class toSyntaxAnalyzer
{
public:
    /** Highlighting categories (joins more categories in qscintilla into
     * simplier ones
     */
    enum infoType
    {
        Default = 0,
        Comment = 1,
        Number = 2,
        Keyword = 3,
        String = 4,
        DefaultBg = 5,
        ErrorBg = 6,
        DebugBg = 7,
        CurrentLineMarker = 8,
        StaticBg = 9
    };
private:
    /** Indicate if colors are updated, can't do this in constructor since QApplication
     * isn't initialized yet.
     */
    bool ColorsUpdated;
    /** Colors allocated for the different @ref infoType values.
     */
    QMap<infoType, QColor> Colors;

    /** Keeps track of possible hits found so far.
     */
    struct posibleHit
    {
        posibleHit(const char *);
        /** Where you are in this word to find a hit.
         */
        int Pos;
        /** The text to hit, points into keywords array.
         */
        const char *Text;
    };
    /** An array of lists of keywords, indexed on the first character.
     */
    std::list<const char *> Keywords[256];
protected:
    /** Check if this is part of a symbol or not.
     */

    //static bool isSymbol(QChar c)
//   {
//       return (c.isLetterOrNumber() || c == '_' || c == '#' || c == '$' || c == '.');
//   }

private:
    /** Get a colordefinition from a @ref infoType value.
     * @param def Color to fill out.
     * @param pos @ref infoType to get color for.
     */
    void readColor(const QColor &def, infoType pos);

    /** Get a string representation of an @ref infoType.
     * @param typ @ref infoType to get string for.
     * @return Description of infotype.
     */
    static QString typeString(infoType typ);

    /** Get an @ref infoType from a string representation of it.
     * @param str Description of @ref infoType.
     * @return @ref infoType described by string.
     */
    static infoType typeString(const QString &str);

    /** Update configuration settings from this class color values.
     */
    void updateSettings(void);
public:
    /** Create a syntax analysed
     * @param keywords A list of keywords.
     */
    toSyntaxAnalyzer(const char **keywords);
    virtual ~toSyntaxAnalyzer()
    { }

    /** Get the character used to quote names of functions etc for the database
     */
    virtual QChar quoteCharacter()
    {
        return '\"';
    }
    /** True if declare keyword starts block.
     */
    virtual bool declareBlock()
//#ifdef TO_NO_ORACLE
//    { return false;
//    }
//#else
    {
        return true;
    }
//#endif

    /** Get a colordefinition for a @ref infoType value.
     * @param typ @ref infoType to get color for.
     * @return Color of that type.
     */
    QColor getColor(infoType typ);

    /** Check if a word is reserved. (Used in toSQLParse)
     * @param word Word to check.
     * @return True if word is reserved.
     */
    bool reservedWord(const QString &word);

    friend class toSyntaxSetup;
    /** Get the default syntax analyzer.
     * @return Reference to the default analyzer.
     */
    static toSyntaxAnalyzer &defaultAnalyzer();
};

static toSyntaxAnalyzer DefaultAnalyzer(DefaultKeywords);

#endif

