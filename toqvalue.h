//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
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
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef TOQVALUE_H
#define TOQVALUE_H

#include <list>

#include <qstring.h>

/** This function is used to represent values that are passed to and from queries
 */
class toQValue {
  enum queryType {
    intType,
    doubleType,
    stringType,
    nullType
  } Type;

  union queryValue {
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
