/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOHELPSETUP_H
#define TOHELPSETUP_H

#include "config.h"

#include <QProgressBar>
#include <QWidget>

#include "ui_tohelpui.h"
#include "ui_tohelpaddfileui.h"
#include "ui_tohelpsetupui.h"
#include "totool.h"


class toHelpAddFile : public QDialog, public Ui::toHelpAddFileUI
{
    Q_OBJECT;

public:
    toHelpAddFile(QWidget *parent, const char *name = 0);

public slots:
    virtual void browse(void);
    virtual void valid(void);
};


class toHelpPrefs : public QWidget,
            public Ui::toHelpSetupUI,
            public toSettingTab
{

    Q_OBJECT;

    toTool *Tool;

public:
    toHelpPrefs(toTool *tool, QWidget *parent, const char *name = 0);

public slots:
    virtual void saveSetting(void);
    virtual void addFile(void);
    virtual void delFile(void);
    virtual void oracleManuals(void);
};

#endif
