/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOABOUT_H
#define TOABOUT_H

#include "config.h"
#include "ui_toaboutui.h"

class toAbout : public QDialog, public Ui::toAboutUI
{
    Q_OBJECT

public:
    enum AboutType
    {
        About,
        License
    };

    toAbout(AboutType, QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~toAbout();

};

#endif
