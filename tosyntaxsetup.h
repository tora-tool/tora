#ifndef __TOSYNTAXSETUP_H
#define __TOSYNTAXSETUP_H

#include <map>
#include "totool.h"
#include "tosyntaxsetup.ui.h"

class toSyntaxSetup : public toSyntaxSetupUI, public toSettingTab { 
  QString Text;
  QListBoxItem *Current;
  map<QString,QColor> Colors;
public:
  toSyntaxSetup(QWidget *parent=0,const char *name=0,WFlags fl=0);
  virtual void saveSetting(void);

public slots:
  virtual void changeLine(QListBoxItem *);
  virtual void selectColor(void);
  virtual void selectFont(void);
};

#endif
