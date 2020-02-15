
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

#pragma once

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVariant>

namespace SQLParser
{
class Token;
}

class toIndent : public QObject
{
    Q_OBJECT;
    Q_PROPERTY(int IndentLineWidthInt MEMBER IndentLineWidthInt)
    Q_PROPERTY(int IndentWidthInt     MEMBER IndentWidthInt)
    Q_PROPERTY(bool ReUseNewlinesBool MEMBER ReUseNewlinesBool)

    Q_PROPERTY(bool BreakOnSelectBool MEMBER BreakOnSelectBool)
    Q_PROPERTY(bool BreakOnFromBool   MEMBER BreakOnFromBool)
    Q_PROPERTY(bool BreakOnWhereBool  MEMBER BreakOnWhereBool)
    Q_PROPERTY(bool BreakOnGroupBool  MEMBER BreakOnGroupBool)
    Q_PROPERTY(bool BreakOnOrderBool  MEMBER BreakOnOrderBool)
    Q_PROPERTY(bool BreakOnModelBool  MEMBER BreakOnModelBool)
    Q_PROPERTY(bool BreakOnPivotBool  MEMBER BreakOnPivotBool)
    Q_PROPERTY(bool BreakOnLimitBool  MEMBER BreakOnLimitBool)
    Q_PROPERTY(bool BreakOnJoinBool   MEMBER BreakOnJoinBool)
    Q_PROPERTY(bool WidthModeBool     MEMBER WidthModeBool)

public:
    enum Mode {
        LineBreaksOnly,
        WidthMode,
        NarrowMode
    };

    // default constructor user in worksheet
    toIndent(int adjustment = 0);

    // constructor used in Preferences GUI
    toIndent(QMap<QString, QVariant> const&, int adjustment = 0);

    virtual ~toIndent() {};

    QString indent(QString const&);

protected:
    void setup();
    void tagToken(SQLParser::Token const*);

    int adjustment; // number of spaces before leading select, not used yet

    int IndentLineWidthInt;
    int IndentWidthInt;
    /*----------------------------*/
    bool ReUseNewlinesBool;
    bool BreakOnSelectBool;
    bool BreakOnFromBool;
    bool BreakOnWhereBool;
    bool BreakOnGroupBool;
    bool BreakOnOrderBool;
    bool BreakOnModelBool;
    bool BreakOnPivotBool;
    bool BreakOnLimitBool;
    bool BreakOnJoinBool;
    /*----------------------------*/
    bool WidthModeBool;

    static QSet<QString> KEYWORDS; // keywords - should have spaces around them
    static QSet<QString> JOIN;     // keywords that can start join
};
