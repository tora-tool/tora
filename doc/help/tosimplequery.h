#ifndef TOSIMPLEQUERY_H
#define TOSIMPLEQUERY_H

#include "totool.h"

class toResultView;
class QLineEdit;
class toConnection;

class toSimpleQuery : public toToolWidget {
  Q_OBJECT

  toResultView *Result;
  QLineEdit *Statement;
private slots:
  void execute(void);
public:
  toSimpleQuery(QWidget *parent,toConnection &connection);
};

#endif
