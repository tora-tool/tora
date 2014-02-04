
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

#ifndef TODEBUGTEXT_H
#define TODEBUGTEXT_H

#include "editor/tosqltext.h"

/**
 * A editor which supports syntax highlighting and also marks error lines
 */
class toDebugText : public toSqlText
{
    Q_OBJECT;

public:
    typedef toSqlText super;

    /** Create a new editor.
     * @param parent Parent of widget.
     * @param name Name of widget.
     */
    toDebugText(QWidget *parent, const char *name = NULL);

    /**
     * Cleaning up done here
     */
    virtual ~toDebugText();

    // ------------------ API used by TOra classes ----------------------
    // NOTE: currently all stubs

    /** Set current debug line. Will be indicated with a different background. Used from debug tool.
     * @param current Current line.
     */
    void setCurrentDebugLine(int current);

    /** Returns true if the editor has any errors.
     */
    bool hasErrors();

    /** Set the error list map.
     * @param errors A map of linenumbers to errorstrings. These will be displayed in the
     *               statusbar if the cursor is placed on the line.
     */
    void setErrors(const QMap<int, QString> &errors, bool errorsGiven = true);

public slots:
    /** Go to next error.
     */
    void nextError(void);
    /** Go to previous error.
     */
    void previousError(void);

private slots:
	void setStatusMessage(void);

private:
    /** Map of rows with errors and their error message.
     */
    QMap<int, QString> Errors;

    //! \brief A handler for debug - line highlighted
    int m_debugHandle;
    //! \brief A handler for debug - margin
    int m_debugMarginHandle;
    //! \brief A handler for code error - line highlighted
    int m_errorHandle;
    //! \brief A handler for code error - margin
    int m_errorMarginHandle;
    //! \brief A handler for static check observations - line highlighted
    int m_staticHandle;
    //! \brief A handler for static check observations - margin
    int m_staticMarginHandle;
};

#endif
