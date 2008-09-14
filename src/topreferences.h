/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOPREFERENCES_H
#define TOPREFERENCES_H

#include "config.h"

#include <qvariant.h>
#include <qdialog.h>

#include <map>

#include "tohelp.h"
#include "ui_topreferencesui.h"


class toPreferences : public QDialog,
            public Ui::toPreferencesUI,
            public toHelpContext
{

    Q_OBJECT;

    QWidget *Shown;

    std::map<QListWidgetItem *, QWidget *> Tabs;

    void addWidget(QListWidgetItem *, QWidget *);

public:
    toPreferences(QWidget* parent = 0,
                  const char* name = 0,
                  bool modal = false,
                  Qt::WFlags fl = 0);


    static void displayPreferences(QWidget *parent);

public slots:
    void saveSetting(void);
    void selectTab(QListWidgetItem *item);
    void help(void);
};

#endif // TOPREFERENCES_H
