/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSEARCHREPLACE_H
#define TOSEARCHREPLACE_H

#include "config.h"
#include "ui_tosearchreplaceui.h"
#include "tohelp.h"
#include "toeditwidget.h"

class toListView;
class toMarkedText;
class toResultContentEditor;

class toSearchReplace : public QDialog, public Ui::toSearchReplaceUI,
            public toHelpContext, public toEditWidget::editHandler
{
    Q_OBJECT

    toEditWidget *Target;
public:
    toSearchReplace(QWidget *parent);
    ~toSearchReplace()
    {
        toEditWidget::delHandler(this);
    }

    virtual void show();
    bool findString(const QString &text, int &pos, int &endPos);
    virtual void receivedFocus(toEditWidget *widget);
    virtual void lostFocus(toEditWidget *)
    {
        receivedFocus(NULL);
    }
    bool searchNextAvailable(void);
public slots:
    virtual void searchNext();
private slots:
    virtual void replaceAll();
    virtual void replace();
    virtual void search();
    virtual void searchChanged();
    virtual void displayHelp();
};

#endif
