#ifndef __TOMARKEDTEXT_3_H
#define __TOMARKEDTEXT_3_H

class toMultiLineEdit : public QtMultiLineEdit {
  Q_OBJECT
public:
  toMultiLineEdit(QWidget *parent,const char *name)
    : QtMultiLineEdit(parent,name)
  { }
};

#endif
