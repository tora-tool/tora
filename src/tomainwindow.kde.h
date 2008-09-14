/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOMAINWINDOW_KDE_H
#define TOMAINWINDOW_KDE_H

#include "config.h"

#include <kapp.h>
#include <kdockwidget.h>

/**
 * A container class that either inherits KDE or Qt main window.
 */

class toMainWindow : public KDockMainWindow
{
    Q_OBJECT
public:
    toMainWindow()
            : KDockMainWindow(0, "Main Window")
    { }
}
;

#endif
