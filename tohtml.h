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

  struct {
    const char *Name;
    const char *Value;
  } Qualifiers[TO_HTML_MAX_QUAL];

  int QualifierNum;
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

  // name should be in lowercase.

  const char *value(const QCString &name);
  bool isTag(void)
  { return IsTag; }
  bool open(void)
  { return Open; }

  // &# characters are expanded before returned.

  QCString text(void);

  // The tag is converted to lowercase before it is returned

  const char *tag(void)
  { return Tag; }

  bool eof(void);

  bool search(const QString &str);
};

#endif
