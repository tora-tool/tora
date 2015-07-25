
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TO_EDIT_GLOBALS_H
#define TO_EDIT_GLOBALS_H

#include <QtCore/qglobal.h>

//! \brief A helper namespace for Search related tools
namespace Search
{
//enum SearchMode
//{
//    SearchPlaintext = 0,
//    SearchRegexp
//};
//
//enum SearchDirection
//{
//    SearchUndefined = -1,
//    SearchForward = 0,
//    SearchBackward = 1
//};
    enum SearchFlag
    {
        Plaintext = 0,
        Regexp = 1,
        CaseSensitive = 2,
        WholeWords = 4,
        Forward = 8,
        Backward = 16,
        Search = 32,
        Replace = 64,
        ReplaceAll = 128
    };
    Q_DECLARE_FLAGS(SearchFlags, SearchFlag);
    Q_DECLARE_OPERATORS_FOR_FLAGS(SearchFlags);

}; // namespace Search

#endif

