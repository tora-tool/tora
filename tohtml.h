//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

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
