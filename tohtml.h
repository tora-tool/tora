#ifndef __TOHTML_H
#define __TOHTML_H

#include <qstring.h>
#include <qcstring.h>
#include <map>

class toHtml {
  QCString Data;
  size_t Position;

  void skipSpace(void);
public:
  struct tag {
    bool Open;
    QString Tag;
    map<QString,QString> Qualifiers;
    QString Text;
  };

  toHtml(const char *file,size_t siz);
  ~toHtml();

  tag nextTag(void);
  bool eof(void)
  { return Position>=Data.length(); }

  bool search(const QString &str);
};

#endif
