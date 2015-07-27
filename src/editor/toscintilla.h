
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

#ifndef TOSCINTILLA_H
#define TOSCINTILLA_H

#include <Qsci/qsciscintilla.h>
#include "core/toeditwidget.h"
#include "core/utils.h"
#include "editor/toeditglobals.h"

#include <QtCore/QString>
#include <QtCore/QPoint>
#include <QtGui/QContextMenuEvent>

class QMenu;
class QMouseEvent;
class QFocusEvent;
class QKeyEvent;
class QsciPrinter;
class QFileSystemWatcher;

/**
 * Long story short - QScintilla::paint emits cursorPositionChanged
 * If it's handler displays QMessageBox(via calling Utils::toStatusMessage) QScintilla::paint gets recursively called
 * And Tora segfaults
 *
 * A sequence of:
 *  message.text = err.value();
 *  QTimer::singleShot(0, &message, SLOT(notify()));
 * Will display an error message AFTER QScintilla::paint finishes
 */
class QSciMessage : public QObject
{
        Q_OBJECT;
    public:
        QString text;
    public slots:
        void notify();
};

/**
 * This is the enhanced editor used in TOra. It mainly offers integration in the TOra
 * menus and print support. It is based on QsciScintilla which is API compatible
 * with QTextEdit class. toEditWidget is abstract class representing copy/paste interface.
 */
class toScintilla: public QsciScintilla //, public toEditWidget
{
        Q_OBJECT;
        typedef QsciScintilla super;
        friend class toEditWidget;
        friend class toBaseEditor;
    public:
        /** Create an editor.
         * @param parent Parent of this widget.
         * @param name Name of this widget.
         */
        toScintilla(QWidget *parent, const char *name = NULL);

        virtual ~toScintilla();

        long currentPosition() const;

        /** Get word at position. This function is now public.
         * re-implemented from QScintilla
         */
        QString wordAtPosition(int position, bool onlyWordCharacters = true) const;

        /** Get word at line and index. This function is now public.
         */
        QString wordAtLineIndex(int line, int index) const;

        QString convertTextS2Q(const char *s) const;

        int NextWordStart(int pos, int delta);
        int NextWordEnd(int pos, int delta);

#if 0
// TODO: this part is waiting for QScintilla backend feature (yet unimplemented).
        /*! \brief Set the selection mode for editor.
        \param aType SC_SEL_STREAM = 0, SC_SEL_RECTANGLE = 1, SC_SEL_LINES = 2
        */
        void setSelectionType(int aType = SC_SEL_STREAM);
#endif

        /** Find the line and column of an index into the string of the entire editor.
         * @param index Index to get line and col from.
         * @param line Will be filled out with the line.
         * @param col Will be filled out with the col.
         */
        void findPosition(int index, int &line, int &col);

        void gotoPosition(int pos);

        int positionAfter(int pos, int offset);

        using QsciScintilla::setSelection;

        void setSelection(int posFrom, int posTo);

        /** Insert text and optionallly mark inserted text.
         * @param str String to insert.
         * @param mark True if mark inserted as selected.
         */
        virtual void insert(const QString &str, bool select = false);

        bool findText(const QString &searchText, const QString &replaceText, Search::SearchFlags flags);
        void findStop();

        char getByteAt(int pos);
        int getStyleAt(int pos);
        int getLevelAt(int line);
        wchar_t getWCharAt(int pos);

    public slots:
        void setWordWrap(bool);

    protected:
        void print(const QString &fname);

        // QsciScintilla API reimplementation

        //! \reimp
        virtual void mousePressEvent(QMouseEvent *e);

        //! \reimp
        virtual void keyPressEvent(QKeyEvent *e);

        //! \reimp
        virtual void focusInEvent(QFocusEvent *e);

        //! \reimp
        virtual void focusOutEvent(QFocusEvent *e);

        //! \reimp
        virtual void contextMenuEvent(QContextMenuEvent *);

        //! \reimp
        virtual void copy();

        //! \reimp
        virtual void paste();

        // Copied from Scintilla CharClassify.h (does not support UTF8)
        class CharClassify
        {
            public:
                CharClassify();

                enum cc { ccSpace, ccNewLine, ccWord, ccPunctuation };
                void SetDefaultCharClasses(bool includeWordClass);
                void SetCharClasses(const unsigned char *chars, cc newCharClass);
                int GetCharsOfClass(cc charClass, unsigned char *buffer);
                cc GetClass(unsigned char ch) const
                {
                    return static_cast<cc>(charClass[ch]);
                }
                bool IsWord(unsigned char ch) const
                {
                    return static_cast<cc>(charClass[ch]) == ccWord;
                }

            private:
                enum { maxChar=256 };
                unsigned char charClass[maxChar];    // not type cc to save space
        };

    signals:
        void displayMenu(QMenu *);

        void gotFocus();
        void lostFocus();

    protected:
        virtual void newLine(void);
        virtual void dropEvent(QDropEvent *);

        /**
         * This function is called to create a right mouse button popup menu
         * at the specified position. If you want to create a custom popup menu,
         * reimplement this function and return the created popup menu. Ownership
         * of the popup menu is transferred to the caller.
         */
        virtual QMenu *createPopupMenu(const QPoint& pos);

        QString getSelectionAsHTML();
        QString getSelectionAsRTF();

        static CharClassify m_charClasifier;
        QSciMessage m_message;
    private slots:

        //! \brief Handle line numbers in the editor on text change
        void slotLinesChanged();

    private:

        QPoint DragStart;

        QString m_searchText;
        Search::SearchFlags m_flags;

        //! Highlight all occurrences of m_searchText QScintilla indicator
        const int m_searchIndicator;
};

#endif

