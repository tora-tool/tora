#pragma once

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVariant>

class toIndent : QObject
{
    Q_OBJECT;
public:
    // default constructor user in worksheet
    toIndent(int adjustment = 0);

    // constructor used in Preferences GUI
    toIndent(QMap<QString, QVariant> const&, int adjustment = 0);

    virtual ~toIndent() {};

    QString indent(QString const&);

protected:
    void setup();

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
