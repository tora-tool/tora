
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

#ifndef TOEDITWIDGET_H
#define TOEDITWIDGET_H

class QWidget;

/** This is an abstract base class for any widget (Not included in object) that wants to
 * access copy, cut, paste, open, save commands of the user interface.
 *
 * TODO COPY enable/disable still does not work
 */
class toEditWidget
{
    public:

        struct FlagSetStruct
        {
            unsigned Open      : 1;
            unsigned Save      : 1;
            unsigned Print     : 1;
            unsigned Undo      : 1;
            unsigned Redo      : 1;
            unsigned Cut       : 1;
            unsigned Copy      : 1;
            unsigned Paste     : 1;
            unsigned Search    : 1;
            unsigned SelectAll : 1;
            unsigned SelectBlock : 1;
            unsigned ReadAll   : 1;
            FlagSetStruct()
                : Open(false)
                , Save(false)
                , Print(false)
                , Undo(false)
                , Redo(false)
                , Cut(false)
                , Copy(false)
                , Paste(false)
                , Search(false)
                , SelectAll(false)
                , SelectBlock(false)
                , ReadAll(false)
            {}
        } FlagSet;

    public:
        /** Empty constructor, all functions are disabled.
         */
        toEditWidget();

        virtual ~toEditWidget();

        /** Perform an open on this widget. Default NOP.
         * @param file The file to open, if not specified prompt for file.
         */
        virtual bool editOpen(const QString &file = QString::null) = 0;

        /** Perform a save on this widget. Default NOP.
         * @param askfile Ask filename even if default filename is available.
         */
        virtual bool editSave(bool askfile) = 0;

        /** Print this widgets contents. Default NOP.
         */
        virtual void editPrint(void) = 0;

        /** Perform undo. Default NOP.
         */
        virtual void editUndo(void) = 0;

        /** Perform redo. Default NOP.
         */
        virtual void editRedo(void) = 0;

        /** Perform cut. Default NOP.
         */
        virtual void editCut(void) = 0;

        /** Perform copy. Default NOP.
         */
        virtual void editCopy(void) = 0;

        /** Perform paste. Default NOP.
         */
        virtual void editPaste(void) = 0;

        /** Select all contents. Default NOP.
         */
        virtual void editSelectAll(void) = 0;

        /** Read all available data. Default NOP.
         */
        virtual void editReadAll(void) = 0;

        /*! \brief Return all current text
        This virtual function can be re-implemented subclasses(like toScintilla).
        So far it is used in toCodeOutline only.
        */
        virtual QString editText() = 0;

        /*! \brief Search for next occrence of text
        This is pure virtual as it has to be implemented in all
        separated children of toEditWidget.
        */
        virtual bool searchNext() = 0;
        /*! \brief Replace current entry with new data
        This is pure virtual as it has to be implemented in all
        separated children of toEditWidget.
        */
        virtual void searchReplace() = 0;

        static toEditWidget* findEdit(QWidget *widget);
};

#endif
