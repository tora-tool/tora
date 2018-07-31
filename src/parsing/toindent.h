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
    Q_PROPERTY(bool WidthModeBool     MEMBER WidthModeBool)

public:
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
    /*----------------------------*/
    bool WidthModeBool;

    static QSet<QString> KEYWORDS; // keywords - should have spaces around them
};
