#ifndef __TOMAINWINDOW_H
#define __TOMAINWINDOW_H

class toMainWindow : public QMainWindow {
  Q_OBJECT
public:
  toMainWindow()
    : QMainWindow(0,"Main Window")
  { }
};

#endif
