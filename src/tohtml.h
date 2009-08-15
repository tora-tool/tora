
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOHTML_H
#define TOHTML_H

#include "config.h"

#include <map>

#include <qstring.h>

#define TO_HTML_MAX_QUAL 100

/** A small and hopefully pretty efficient parser of HTML tags. No attempt what
 * so ever is made trying to check the validity of the text so if you forget
 * a closing > you will probably end up with an exception.
 *
 * A simple traverser counting the number of paragraphs might look like this.
 *
<pre>
int paragraphs++;
toHtml count("Paragraphs<P>And some</P><P>Dobedoo");
while(!count.eof()) {
  count.nextToken();
  if (count.isTag()&&count.open()&&QString(count.tag())=="p")
    paragraphs++;
}
</pre>
 */

class QChar;

class toHtml
{
    /** Actual data of file.
     */
    QString Data;
    /** Length of the data.
     */
    int Length;
    /** Current position in data.
     */
    int Position;

    /** Last character that was at the end of text written over by call to @ref mid.
     * This is a real cludge, but what don't you do for performance.
     */
    QChar LastChar;

    /** List of qualifiers for tab. This is not a list because of performance as well.
     * Not really a lot of memory wasted this way, in fact it is probably more efficient
     * than a dynamic list.
     */
    struct
    {
        /** Name of qualifier. Always lowercase, points into @ref Data.
         */
        QString Name;
        /** Value of qualifier. Points into @ref Data.
         */
        QString Value;
    }
    Qualifiers[TO_HTML_MAX_QUAL];

    /** Number of qualifiers used.
     */
    int QualifierNum;
    /** If last token read was a tag or plain text.
     */
    bool IsTag;
    /** Last tag read was an open tag. Only valid if @ref isTag is true.
     */
    bool Open;
    /** Name of tag. NULL terminated, points into @ref Data.
     */
    QString Tag;
    /** Text if not tag. NULL terminated, points into @ref Data.
     */
    QString Text;

    /** Walk forward in @ref Data until not whitespace.
     */
    void skipSpace(void);
    /** Extract a string from @ref Data. Insert 0 at end of string, store overwritten char
     * in LastChar.
     * @param start Start position of string to extract.
     * @param size Size of string to extract.
     * @return Pointer to string (Points into @ref Data)
     */
    QString mid(int start, int size);
public:
    /** Create a HTML parser.
     * @param data Data to parse.
     */
    toHtml(const QString &data);
    ~toHtml();

    /** Read the next token which can be either a open tag, close tag or text.
     *
     * Observe that nextToken destroys the data it has parsed, so you
     * can't search it later. Searhing must be done on an new toHtml,
     * several serches can be made though on the same toHtml.
     */

    void nextToken(void);

    /** Get value of a qualifier.
     * @param name Name of qualifier in lowercase.
     * @return Value of qualifier or NULL.
     */
    QString value(const QString &name);
    /** Return true if current token is a tag.
     * @return True if tag.
     */
    bool isTag(void)
    {
        return IsTag;
    }
    /** Return true if open tag. Only valid if @ref isTag is true.
     * @return True if open tag.
     */
    bool open(void)
    {
        return Open;
    }

    /** Get bread text if available. Expands &# characters are expanded before returned.
     * Only valid if @ref isTag returns false.
     * @return The parsed text.
     */

    QString text(void);

    /** Get name of tag in lowercase. Only valid if @ref isTag returns true.
     * @return Name of tag (Excluding / if close tag).
     */
    QString tag(void)
    {
        return Tag;
    }

    /** Check if at end of data
     * @return True if at end of data.
     */
    bool eof(void);

    /** Search data for a simple text.
     * @param data Data to search.
     * @param str Substring to search for in data.
     */
    static bool search(const QString &data, const QString &str);

    /** Escape text to be HTML.
     */
    static QString escape(const QString &html);
};

#endif
