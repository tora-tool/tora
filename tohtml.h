#ifndef __TOHTML_H
#define __TOHTML_H

#include <qstring.h>
#include <qcstring.h>
#include <map>

class toHtml {
  QCString Data;
  size_t Position;

  void skipSpace(void);
  char data(void)
  { return data(Position); }
  char data(size_t pos);
  QCString mid(size_t start,size_t size);
public:
  struct tag {
    bool Open;
    QCString Tag;
    map<QString,QString> Qualifiers;
    QCString Text;
  };

  toHtml(const QCString &data);
  ~toHtml();

  tag nextTag(void);
  bool eof(void);

  bool search(const QString &str);
};

#endif
