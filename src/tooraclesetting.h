/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOORACLESETTING_H
#define TOORACLESETTING_H

#include "config.h"
#include "totool.h"

#include "ui_tooraclesettingui.h"


class toOracleSetting : public QWidget,
            public Ui::toOracleSettingUI,
            public toSettingTab
{

    Q_OBJECT;

public:
    toOracleSetting(QWidget *parent);

public slots:
    virtual void saveSetting(void);
    virtual void createPlanTable(void);
};

#endif
