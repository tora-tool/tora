
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

#ifndef TORESULTCODE_H
#define TORESULTCODE_H

#include "editor/todebugeditor.h"

#include "core/toresult.h"

class toSQL;

/** This widget displays the SQL used to recreate an object. It uses the
 * @ref toExtract class to do this.
 */
class toResultCode : public toDebugEditor, public toResult
{
    private:
        Q_OBJECT;

        bool Prompt;
        bool m_heading;
        int m_offset; // offset of "CREATE OR REPLACE .. if heading is used
    public:
        /** Create the widget.
         * @param parent Parent widget.
         * @param name Name of widget.
         */
        toResultCode(bool prompt, QWidget *parent, const char *name = NULL);
        //! \brief Dummy constructor for Qt designer usage
        toResultCode(QWidget * parent = 0);

        /** Display generated headers in the extractor.
            The final behavior depends on this flag *and*
            toConfigurationSingle::Instance().extractorIncludeHeader() too.
            See:
            extract.setHeading(m_heading && toConfigurationSingle::Instance().extractorIncludeHeader());
        */
        void setHeading(bool enable)
        {
            m_heading = enable;
        };

        /** Return offset line of the "real" statement start (if headers/prompts are used) */
        unsigned offset() const { return m_offset; }

        /** Ignores sql and extracts object name and owner as parameters.
         */
        virtual void query(const QString &sql, toQueryParams const& param);

        /** Clear result widget */
        virtual void clearData();

        /** Support Oracle
         */
        virtual bool canHandle(const toConnection &conn);
};

#endif
