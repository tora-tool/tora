#ifndef __TOHTML_H
#define __TOHTML_H

#include <qstring.h>
#include <qcstring.h>
#include <map>

#define TO_HTML_MAX_QUAL 100

class toHtml {
  char *Data;
  size_t Length;
  size_t Position;

  char LastChar;

  struct strOfsPair {
    const char *Name;
    const char *Value;
  };

  int QualifierNum;
  strOfsPair Qualifiers[TO_HTML_MAX_QUAL];
  bool Open;
  bool IsTag;
  const char *Tag;
  const char *Text;

  void skipSpace(void);
  const char *mid(size_t start,size_t size);
public:
  toHtml(const QCString &data);
  ~toHtml();

  // Observe that nextTag destroys the data it has parsed, so you
  // can't search it later. Searhing must be done on an new toHtml,
  // several serches can be made though on the same toHtml.

  void nextTag(void);

  const char *value(const QCString &name);
  bool isTag(void)
  { return IsTag; }
  bool open(void)
  { return Open; }
  QCString text(void);
  const char *tag(void)
  { return Tag; }

  bool eof(void);

  bool search(const QString &str);
};

#endif
