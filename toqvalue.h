//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef __TOQVALUE_H
#define __TOQVALUE_H

#include <list>

#include <qstring.h>

/** This function is used to represent values that are passed to and from queries
 */
class toQValue {
  enum {
    intType,
    doubleType,
    stringType,
    nullType
  } Type;
  union {
    int Int;
    double Double;
    QString *String;
  } Value;
public:
  /** Create null value.
   */
  toQValue(void);
  /** Create integer value.
   * @param i Value.
   */
  toQValue(int i);
  /** Create string value.
   * @param str Value.
   */
  toQValue(const QString &str);
  /** Create double value.
   * @param d Value.
   */
  toQValue(double d);
  /** Destruct query.
   */
  ~toQValue();
    
  /** Create a copy of a value.
   */
  toQValue(const toQValue &copy);
  /** Assign this value from another value.
   */
  const toQValue &operator = (const toQValue &copy);

  /** Check if this is an int value.
   */
  bool isInt(void) const;
  /** Check if this is a double value.
   */
  bool isDouble(void) const;
  /** Check if this is a string value.
   */
  bool isString(void) const;
  /** Check if this value is null.
   */
  bool isNull(void) const;

  /** Get utf8 format of this value.
   */
  QCString utf8Value(void) const;
  /** Get integer representation of this value.
   */
  int toInt(void) const;
  /** Get double representation of this value.
   */
  double toDouble(void) const;

  /** Convert value to a string.
   */
  operator QString() const;
};

/** A short representation of list<toQuery::queryValue>
 */
typedef std::list<toQValue> toQList;

#endif
