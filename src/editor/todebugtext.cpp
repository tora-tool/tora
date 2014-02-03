
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

#include "editor/todebugtext.h"
#include "core/tologger.h"

toDebugText::toDebugText(QWidget *parent, const char *name)
    : toHighlightedText(parent)
	, m_errorMarginHandle(QsciScintilla::markerDefine(QsciScintilla::Circle))
	, m_errorHandle(QsciScintilla::markerDefine(QsciScintilla::Background))
	, m_debugMarginHandle(QsciScintilla::markerDefine(QsciScintilla::Rectangle))
	, m_debugHandle(QsciScintilla::markerDefine(QsciScintilla::Background))
	, m_staticMarginHandle(QsciScintilla::markerDefine(QsciScintilla::Circle))
	, m_staticHandle(QsciScintilla::markerDefine(QsciScintilla::Background))
{
	connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(setStatusMessage(void )));
}

toDebugText::~toDebugText()
{
}

bool toDebugText::hasErrors()
{
    if ( Errors.empty() )
        return (false);
    else
        return (true);
}

void toDebugText::nextError(void)
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    for (QMap<int, QString>::iterator i = Errors.begin(); i != Errors.end(); i++)
    {
        if (i.key() > curline)
        {
            setCursorPosition(i.key(), 0);
            break;
        }
    }
}

void toDebugText::previousError(void)
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    curcol = -1;
    for (QMap<int, QString>::iterator i = Errors.begin(); i != Errors.end(); i++)
    {
        if (i.key() >= curline)
        {
            if (curcol < 0)
                curcol = i.key();
            break;
        }
        curcol = i.key();
    }
    if (curcol >= 0)
        setCursorPosition(curcol, 0);
}

/* Sets errors/static observations markers in margins and on text
*/
void toDebugText::setErrors(const QMap<int, QString> &errors, bool errorsGiven)
{
    int handle, marginHandle;

    if (errorsGiven)
    {
        handle = m_errorHandle;
        marginHandle = m_errorMarginHandle;
    }
    else
    {
        handle = m_staticHandle;
        marginHandle = m_staticMarginHandle;
    }

    Errors = errors;
    setStatusMessage();
    markerDeleteAll(handle);
    markerDeleteAll(marginHandle);
    for (QMap<int, QString>::const_iterator i = Errors.begin(); i != Errors.end(); i++)
    {
        if (i.key() < 0)
        {
            TLOG(2, toDecorator, __HERE__)
                    << "toHighlightedTextEditor::setErrors errLine:" << i.key() << " value:" << i.value() << std::endl;
            continue;
        }

        // Some general errors are not attached to any line. Oracle returns lineno 0 for those.
        // TOra will be showing such errors in error list but no line will be highlighted.
        if (i.key() > 0)
        {
            markerAdd(i.key() - 1, handle);
            markerAdd(i.key() - 1, marginHandle);
        }
    }
} // setErrors

void toDebugText::setStatusMessage(void)
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    QMap<int, QString>::iterator err = Errors.find(curline);
    if (err == Errors.end())
    	m_message.text = QString::null;
    else
    	m_message.text = err.value();

    // Call Utils::statusMessage asynchronously (see QSciMessage)
    QTimer::singleShot(0, &m_message, SLOT(notify()));
}

void toDebugText::setCurrentDebugLine(int current)
{
	setCursorPosition (current, 0);
	markerDeleteAll(m_debugHandle);
	markerDeleteAll(m_debugMarginHandle);
	if (current >= 0)
	{
		markerAdd(current, m_debugHandle);
		markerAdd(current, m_debugMarginHandle);
	}
}
