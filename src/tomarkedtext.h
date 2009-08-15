
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

#ifndef TOMARKEDTEXT_H
#define TOMARKEDTEXT_H

#include "config.h"
#include "toeditwidget.h"

#include <map>

#include <qglobal.h>
#include <qstring.h>
#include <qtimer.h>

#include <Qsci/qsciscintilla.h>

#include <QContextMenuEvent>
#include <QString>
#include <QDropEvent>
#include <QTimerEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPoint>

#include "tosearchreplace.h"

class QMenu;
class QsciPrinter;


/**
 * This is the enhanced editor used in TOra. It mainly offers integration in the TOra
 * menues and print support. It is based on QsciScintilla which is API compatible
 * with QTextEdit class.
 */

class toMarkedText : public QsciScintilla, public toEditWidget
{
    Q_OBJECT

    /** Filename of the file in this buffer.
     */
    QString Filename;

    bool Search;
    bool SearchFailed;
    bool SearchForward;
    QString SearchString;
    QString LastSearch;

    int CursorTimerID;

    QPoint DragStart;

    Search::SearchDirection m_searchDirection;
    //! Text to search
    QString m_searchText;
    //! Highligh all occurrences of m_searchText QScintilla indicator
    int m_searchIndicator;

    bool findText(Search::SearchDirection direction);

    void searchFound(int line, int col);
    void incrementalSearch(bool forward, bool next);
protected:
    /** Reimplemented for internal reasons.
     */
    virtual void mousePressEvent(QMouseEvent *e);

    /** Reimplemented for internal reasons.
     */
    virtual void keyPressEvent(QKeyEvent *e);
public:
    /** Create an editor.
     * @param parent Parent of this widget.
     * @param name Name of this widget.
     */
    toMarkedText(QWidget *parent, const char *name = NULL);

    virtual ~toMarkedText();

    /**
     * Returns the default tab width.
     */
    static int defaultTabWidth();

    /**
     * Sets the default tab width.
     * @param width the new default tab width;
     */
    static void setDefaultTabWidth(int width);

    /**
     * Returns default setting for tabs.
     *
     */
    static bool defaultTabSpaces(void);

    /**
     * If true, editor will use spaces not tabs.
     *
     */
    static void setDefaultTabSpaces(bool on);

    /** Insert text and optionallly mark inserted text.
     * @param str String to insert.
     * @param mark True if mark inserted as selected.
     */
    virtual void insert(const QString &str, bool select = false);

    /** Erase the contents of the editor.
     */
    virtual void clear(void)
    {
        Filename = "";
        redoEnabled(false);
        undoEnabled(false);
        setEdit();
        QsciScintilla::clear();
        setModified(false);
    }

    /** Get location of the current selection. This function is now public. See the
     * Qt documentation for more information.
     */
    bool getSelection (int * line1, int * col1, int * line2, int * col2)
    {
        QsciScintilla::getSelection(line1, col1, line2, col2);

        return hasSelectedText();
    }

#if 0
// TODO: this part is waiting for QScintilla backend feature (yet unimplemented).
    /*! \brief Set the selection mode for editor.
    \param aType SC_SEL_STREAM = 0, SC_SEL_RECTANGLE = 1, SC_SEL_LINES = 2
    */
    void setSelectionType(int aType=SC_SEL_STREAM);
#endif

    /** Get filename of current file in editor.
     * @return Filename of editor.
     */
    virtual QString filename(void) const
    {
        return Filename;
    }
    /** Open a file for editing.
     * @param file File to open for editing.
     */
    virtual void openFilename(const QString &file);
    /** Set the current filename of the file in editor.
     * @param str String containing filename.
     */
    virtual void setFilename(const QString &str)
    {
        Filename = str;
    }
    /** Update user interface with availability of copy/paste etc.
     */
    virtual void setEdit(void);
    /** Reimplemented for internal reasons.
     */
    virtual void focusInEvent (QFocusEvent *e);
    /** Reimplemented for internal reasons.
     */
    virtual void focusOutEvent (QFocusEvent *e);
    /** Print this editor.
     */
    virtual void editPrint(void);
    /** Reimplemented for internal reasons.
     */
    virtual bool editOpen(QString suggestedFile = QString::null);
    /** Reimplemented for internal reasons.
     */
    virtual bool editSave(bool askfile);

    virtual bool searchNext(const QString & text);
    virtual bool searchPrevious(const QString & text);

    /** Replace entry with new data
     */
    virtual void searchReplace(const QString &newData);
    virtual void searchReplaceAll(const QString &newData);
    /** Check if data can be modified by search
     * @param all If true can replace all, otherwise can replace right now.
     */
    virtual bool searchCanReplace(bool all);

    /** Start incremental search mode, or search next if started.
     */
    void incrementalSearch(bool forward);

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
    /** Find the line and column of an index into the string of the entire editor.
     * @param index Index to get line and col from.
     * @param line Will be filled out with the line.
     * @param col Will be filled out with the col.
     */
    void findPosition(int index, int &line, int &col);

signals:
    void displayMenu(QMenu *);
    // emitted when a new file is opened
    void fileOpened(void);
    void fileOpened(QString file);
    void fileSaved(QString file);

protected:
    virtual void newLine(void);
    virtual void dropEvent(QDropEvent *);
    virtual void timerEvent(QTimerEvent *);
    void contextMenuEvent(QContextMenuEvent *);

    /**
     * This function is called to create a right mouse button popup menu
     * at the specified position. If you want to create a custom popup menu,
     * reimplement this function and return the created popup menu. Ownership
     * of the popup menu is transferred to the caller.
     */
    virtual QMenu *createPopupMenu(const QPoint& pos);

private slots:
    void setTextChanged();
    void setCopyAvailable(bool yes);
    //! \brief Handle line numbers in the editor on text change
    void linesChanged();

public slots:
    /**
     * Reimplemented for internal reasons.
     */
    virtual void editUndo(void)
    {
        undo();
    }

    /**
     * Reimplemented for internal reasons.
     */
    virtual void editRedo(void)
    {
        redo();
    }

    /**
     * Reimplemented for internal reasons.
     */
    virtual void editCut(void)
    {
        cut();
    }

    /**
     * Reimplemented for internal reasons.
     */
    virtual void editCopy(void)
    {
        copy();
    }

    /**
     * Reimplemented for internal reasons.
     */
    virtual void editPaste(void)
    {
        paste();
    }

    /**
     * Reimplemented for internal reasons.
     */
    virtual void editSelectAll(void)
    {
        selectAll();
    }

    /**
     * QsciScintilla's selectAll doesn't always work like we want.
     *
     */
    virtual void selectAll(bool select = TRUE)
    {
        // selectAll(true) doesn't work if there's a selection already.
        if (QsciScintilla::hasSelectedText())
            QsciScintilla::selectAll(false);

        QsciScintilla::selectAll();
    }

    void setWordWrap(bool);
};

#endif
