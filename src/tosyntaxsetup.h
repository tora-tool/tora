/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSYNTAXSETUP_H
#define TOSYNTAXSETUP_H

#include "config.h"
#include "tohelp.h"
#include "tohighlightedtext.h"
#include "ui_tosyntaxsetupui.h"
#include "totool.h"

#include <map>

#include <QString>

class QFont;

class toSyntaxSetup : public QWidget,
            public Ui::toSyntaxSetupUI,
            public toSettingTab
{

    Q_OBJECT;

    QString List;
    QListWidgetItem *Current;
    std::map<QString, QColor> Colors;
    toSyntaxAnalyzer Analyzer;
    QString color();

public:
    toSyntaxSetup(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);
    virtual void saveSetting(void);
    void checkFixedWidth(const QFont &fnt);

public slots:
    virtual void changeLine(QListWidgetItem *);
    virtual void selectColor(void);
    virtual void selectFont(void);
    virtual void selectText(void);
    virtual void selectResultFont(void);
    virtual void changeUpper(bool val)
    {
        Example->setKeywordUpper(val);
        Example->update();
    }
    virtual void changeHighlight(bool val)
    {
        Example->setSyntaxColoring(val);
        Example->update();
    }
};

#endif
