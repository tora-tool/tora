/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOEDITEXTENSIONS_H
#define TOEDITEXTENSIONS_H

#include "config.h"
#include "toeditwidget.h"

// for settings classes
#include "ui_toeditextensiongotoui.h"
#include "ui_toeditextensionsetupui.h"
#include "totemplate.h"
#include "tosqlparse.h"

#include <algorithm>

class toMarkedText;
class toEditExtensionTool;

class toEditExtensions : public QObject, public toEditWidget::editHandler
{
    Q_OBJECT;

    toMarkedText *Current;

    void intIndent(int level);

public:
    toEditExtensions();
    virtual void receivedFocus(toEditWidget *widget);
    virtual void lostFocus(toEditWidget *widget);

public slots:
    void indentBlock(void);
    void deindentBlock(void);

    void autoIndentBlock(void);
    void autoIndentBuffer(void);

    void quoteBlock(void);
    void unquoteBlock(void);

    void obfuscateBlock(void);
    void obfuscateBuffer(void);

    void searchForward(void);
    void searchBackward(void);

    void upperCase(void);
    void lowerCase(void);

    void gotoLine(void);

    void autoComplete(void);

    void editEnabled(bool);
};


class toEditExtensionGoto : public QDialog, public Ui::toEditExtensionGotoUI
{
    Q_OBJECT;

    toMarkedText *Editor;

public:
    toEditExtensionGoto(toMarkedText *editor);
    void gotoLine();
};


class toEditExtensionSetup : public QWidget,
            public Ui::toEditExtensionSetupUI,
            public toSettingTab
{
    Q_OBJECT;

    toEditExtensionTool *Tool;
    toSQLParse::settings Current;
    bool Ok;
    bool Started;

public:
    toEditExtensionSetup(
        toEditExtensionTool *tool,
        QWidget *parent,
        const char *name = NULL);

    virtual ~toEditExtensionSetup();

public slots:
    void saveCurrent(void);
    virtual void changed(void);
    virtual void saveSetting(void);
};

#endif
