
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

#ifndef TOHIGHLIGHTEDTEXT_H
#define TOHIGHLIGHTEDTEXT_H

#include "editor/tomarkedtext.h"
#include "core/topushbutton.h"
#include "core/tocache.h"
#include "parsing/tosyntaxanalyzer.h"

#include "loki/Singleton.h"

#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtCore/QStringList>
#include <QtGui/QKeyEvent>
#include <QtGui/QWidget>

class QListWidgetItem;
class QsciAbstractAPIs;

class toComplPopup;
class toHighlightedTextWorker;

/**
 * A editor which supports syntax highlighting.
 *
 * This needs to be heavily re-implemented/simplified to use QScintilla syntax
 * coloring. For now it only stubs used API from previous version of
 * toHighlightedTextEditor. The rest of the API comes unchanged from toMarkedText
 * which is now derived from QScintilla.
 */
class toHighlightedText : public toMarkedText
{
    Q_OBJECT;
    Q_ENUMS(HighlighterTypeEnum);
public:
    friend class toComplPopup;
    typedef toMarkedText super;

    enum HighlighterTypeEnum {
    	None   = 10,
    	Oracle = 20,
    	Mysql  = 30,
    	QtSql  = 40
    };

    // Styles alternating colors for linenumbers
    enum Style {
            Default = 30,
            OneLine = 150,
            OneLineAlt = 151,
            MaxStyle
    };
    static QColor lightCyan;
    static QColor lightMagenta;


    /** Create a new editor.
     * @param parent Parent of widget.
     * @param name Name of widget.
     */
    toHighlightedText(QWidget *parent, const char *name = NULL);

    /**
     * Cleaning up done here
     */
    virtual ~toHighlightedText();

    /*! \brief Inherited from toMarkedText to clear all required editor
    markers;
    */
    virtual void openFilename(const QString &file);

	void setHighlighter(toHighlightedText::HighlighterTypeEnum);
    HighlighterTypeEnum getHighlighter(void);

    /**
     * Overriden to set font for lexer as well.
     * @param font the font to set
     */
    void setFont (const QFont & font);

    // ------------------ API used by TOra classes ----------------------
    // NOTE: currently all stubs

    /**
     * Set keyword upper flag. If this is set keywords will be converted
     * to uppercase when painted.
     *
     * NOTE: this may be quite tricky to implement - have to check
     *       how the Scintilla Lexers are working
     *
     * @param val New value of keyword to upper flag.
     */
    //void setKeywordUpper(bool val) {}

    /**
     * Sets the syntax colouring flag.
     */
    //void setSyntaxColoring(bool val);

    /** Get the tablename currently under the cursor.
     * @param owner Filled with owner or table or QString::null if no owner specified.
     * @param table Filled with tablename
     */
    void tableAtCursor(toCache::ObjectRef &table);

    toSyntaxAnalyzer* analyzer();

    //void updateSyntaxColor(toSyntaxAnalyzer::infoType t);
public slots:
    void handleBookmark();
    void gotoPrevBookmark();
    void gotoNextBookmark();

    // Override QScintilla
    virtual void autoCompleteFromAPIs();

    void positionChanged(int row, int col);

private slots:
    void setHighlighter(int);
    void process();
	void processed();

#ifdef QT_DEBUG
    // This function should diagnose focus "stealing"
    void reportFocus();
#endif

signals:
	void parsingRequested(QString);

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

    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

    void scheduleParsing();
    void unScheduleParsing();
private:
    HighlighterTypeEnum highlighterType;

    QsciAbstractAPIs* m_complAPI;
    QTimer* complTimer;

    toSyntaxAnalyzer *m_analyzerNL, *m_analyzerOracle, *m_currentAnalyzer;
    QMap<int,QString> styleNames;
    QFont mono; // line numbers font

    // toHighlightedTextWorker related variables
    QTimer *m_parserTimer;
	QThread *m_parserThread;
	toHighlightedTextWorker *m_worker;
	bool m_haveFocus; // this flag handles situation when bg thread response is rececived after focus was lost

    //! \brief A handler for current line highlighting - margin
    // FIXME: disabled due repainting issues
    // int m_currentLineMarginHandle;
    //! \brief A handler for bookrmarks - line highlighted
    int m_bookmarkHandle;
    //! \brief A handler for bookrmarks - margin
    int m_bookmarkMarginHandle;
    //! \brief Bookrmarks handler list used for navigation (next/prev)
    QList<int> m_bookmarks;
};

Q_DECLARE_METATYPE(toHighlightedText::HighlighterTypeEnum)

/* Utility class for @ref toCustomLexer
 * Instance of this class "lives" within background thread
 * and dispatches signals from/to the main thread
 *
 * NOTE: this class could by nested, but QT does not support it
 */
class toHighlightedTextWorker: public QObject {
	Q_OBJECT;
	friend class toHighlightedText;
signals:
	void finished();
	void processed();
	void error(QString err);

public:
	toHighlightedTextWorker(QObject *parent = 0);
	~toHighlightedTextWorker();

public slots:
	void process(QString);

protected:
	void setAnalyzer(toSyntaxAnalyzer*);

	toSyntaxAnalyzer::statementList statements;
	toSyntaxAnalyzer *analyzer;
};

/**
 * Subclass toToggleButton and iterate over values of HighlighterTypeEnum
 */
class toHighlighterTypeButton : public toToggleButton
{
	Q_OBJECT;
public:
	toHighlighterTypeButton(QWidget *parent, const char *name = 0);
	toHighlighterTypeButton();
};

// this one will be usually parented by QStatusBar
typedef Loki::SingletonHolder<toHighlighterTypeButton, Loki::CreateUsingNew, Loki::NoDestroy> toHighlighterTypeButtonSingle;

#endif
