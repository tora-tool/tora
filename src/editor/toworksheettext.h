
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

#ifndef TOWORKSHEETTEXT_H
#define TOWORKSHEETTEXT_H

#include "../main/toeditorsetting.h"
#include "editor/tosqltext.h"

class toComplPopup;

class toWorksheetText : public toSqlText
{
        Q_OBJECT;
        Q_ENUMS(EditorTypeEnum);
        typedef toSqlText super;
        friend class toComplPopup;
    public:

        enum EditorTypeEnum
        {
            SciTe   = 100
#ifdef QT_DEBUG
                      , Custom  = 200
                                  , Emacs   = 300
#endif
        };

        /** Create a new editor.
         * @param parent Parent of widget.
         * @param name Name of widget.
         */
        toWorksheetText(QWidget *parent, const char *name = NULL);

        virtual ~toWorksheetText();

        virtual void setHighlighter(toSqlText::HighlighterTypeEnum);

    public slots:
        void setEditorType(int);

        void handleBookmark();
        void gotoPrevBookmark();
        void gotoNextBookmark();

        // Override QScintilla (display custom toComplPopup window)
        virtual void autoCompleteFromAPIs();

        // Insert chosen text
        void completeFromAPI(QListWidgetItem * item);

        void positionChanged(int row, int col);

    protected:
        /*! \brief Override QScintilla event handler to display code completion popup */
        virtual void keyPressEvent(QKeyEvent * e);

        /*! \brief Guess what should be used for code completion
        in this time.
        When SQL parser can decide the editor is in FOO.bar state
        it will suggest "bar" related columns etc.
        When SQL parser couldn't find any suggestion it will list
        keywords/functions from templates/completion.api list.
        \param partial a QString reference with starting char sequence
        */
        QStringList getCompletionList(QString &partial);

        void completeWithText(QString const&);

        virtual void focusInEvent(QFocusEvent *e);
        virtual void focusOutEvent(QFocusEvent *e);

#ifdef TORA3_SESSION
        /** Export data to a map.
         * @param data A map that can be used to recreate the data of a chart.
         * @param prefix Prefix to add to the map.
         */
        virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
        /** Import data
         * @param data Data to read from a map.
         * @param prefix Prefix to read data from.
         */
        virtual void importData(std::map<QString, QString> &data, const QString &prefix);
#endif

    private:
        EditorTypeEnum editorType;
        toComplPopup* popup;

    protected:
        QsciAbstractAPIs* m_complAPI;
        QTimer* complTimer;

        //! \brief A handler for current line highlighting - margin
        // FIXME: disabled due repainting issues
        // int m_currentLineMarginHandle;
        //! \brief A handler for bookrmarks - line highlighted
        int m_bookmarkHandle;
        //! \brief A handler for bookrmarks - margin
        int m_bookmarkMarginHandle;
        //! \brief Bookrmarks handler list used for navigation (next/prev)
        QList<int> m_bookmarks;

        bool m_completeEnabled;
};

/**
 * Subclass toToggleButton and iterate over values of HighlighterTypeEnum
 */
class toEditorTypeButton : public toToggleButton
{
        Q_OBJECT;
    public:
        toEditorTypeButton(QWidget *parent, const char *name = 0);
        toEditorTypeButton();
};

// this one will be usually parented by QStatusBar
typedef Loki::SingletonHolder<toEditorTypeButton, Loki::CreateUsingNew, Loki::NoDestroy> toEditorTypeButtonSingle;

#endif
