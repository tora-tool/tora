#ifndef __TOMARKEDTEXT_2_H
#define __TOMARKEDTEXT_2_H

class toMultiLineEdit : public QMultiLineEdit {
  Q_OBJECT
public:
  toMultiLineEdit(QWidget *parent,const char *name)
    : QMultiLineEdit(parent,name)
  { }
};

#endif
