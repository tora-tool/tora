#ifndef TOAWR_H
#define TOAWR_H
 
#include "totool.h"
 
class toResultView;
class QLineEdit;
class toConnection;
 
class toAWR : public toToolWidget {
  Q_OBJECT;
 
  toResultView *Result;
  QLineEdit *Statement;
  QAction *updateAct;
public:
  toAWR(/*toTool *tool,*/ QWidget *parent,toConnection &connection);
  virtual ~toAWR();

private slots:
  void execute(void);
};
 
#endif
 
