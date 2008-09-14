/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOGLOBALSETTING_H
#define TOGLOBALSETTING_H

#include "config.h"
#include "ui_todatabasesettingui.h"
#include "ui_toglobalsettingui.h"
#include "tohelp.h"
#include "totool.h"
#include "ui_totoolsettingui.h"

class toGlobalSetting : public QWidget, public Ui::toGlobalSettingUI, public toSettingTab
{
    Q_OBJECT;

public:
    toGlobalSetting(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

public slots:
    virtual void saveSetting(void);

    virtual void pluginBrowse(void);
    virtual void sqlBrowse(void);
    virtual void helpBrowse(void);
    virtual void sessionBrowse(void);
    virtual void cacheBrowse(void);
};

class toDatabaseSetting : public QWidget, public Ui::toDatabaseSettingUI, public toSettingTab
{
    Q_OBJECT;

public:
    toDatabaseSetting(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

public slots:
    virtual void saveSetting(void);
    virtual void numberFormatChange();
    virtual void IndicateEmptyColor_clicked();
};

class toToolSetting : public QWidget, public Ui::toToolSettingUI, public toSettingTab
{
    Q_OBJECT;

public:
    toToolSetting(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);

public slots:
    virtual void saveSetting(void);
    virtual void changeEnable(void);
};

#endif
