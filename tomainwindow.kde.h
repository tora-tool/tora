#ifndef __TOMAINWINDOW_KDE_H
#define __TOMAINWINDOW_KDE_H

class toMainWindow : public KDockMainWindow {
  Q_OBJECT
public:
  toMainWindow()
    : KDockMainWindow(0,"Main Window")
  { }
};

#endif
