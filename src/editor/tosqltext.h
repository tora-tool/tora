
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

#ifndef TOSQLTEXT_H
#define TOSQLTEXT_H

#include "editor/toscintilla.h"
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
class toSqlTextWorker;

/**
 * A editor which supports syntax highlighting.
 *
 * This needs to be heavily re-implemented/simplified to use QScintilla syntax
 * coloring. For now it only stubs used API from previous version of
 * toHighlightedTextEditor. The rest of the API comes unchanged from toScintilla
 * which is now derived from QScintilla.
 */
class toSqlText : public toScintilla
{
    Q_OBJECT;
    Q_ENUMS(HighlighterTypeEnum);
    typedef toScintilla super;
public:
    friend class toComplPopup;

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
    toSqlText(QWidget *parent, const char *name = NULL);

    /**
     * Cleaning up done here
     */
    virtual ~toSqlText();

	virtual void setHighlighter(toSqlText::HighlighterTypeEnum);
    HighlighterTypeEnum getHighlighter(void);

    /**
     * Overriden to set font for lexer as well.
     * @param font the font to set
     */
    void setFont (const QFont & font);

    /** Get the tablename currently under the cursor.
     * @param owner Filled with owner or table or QString::null if no owner specified.
     * @param table Filled with tablename
     */
    void tableAtCursor(toCache::ObjectRef &table);

    toSyntaxAnalyzer* analyzer();

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

    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

    void scheduleParsing();
    void unScheduleParsing();

private:
    HighlighterTypeEnum highlighterType;

    toSyntaxAnalyzer *m_analyzerNL, *m_analyzerOracle, *m_currentAnalyzer;
    QMap<int,QString> styleNames;
    QFont mono; // line numbers font

    // toHighlightedTextWorker related variables
    QTimer *m_parserTimer;
	QThread *m_parserThread;
	toSqlTextWorker *m_worker;
	bool m_haveFocus; // this flag handles situation when bg thread response is rececived after focus was lost
};

Q_DECLARE_METATYPE(toSqlText::HighlighterTypeEnum)

/* Utility class for @ref toCustomLexer
 * Instance of this class "lives" within background thread
 * and dispatches signals from/to the main thread
 *
 * NOTE: this class could by nested, but QT does not support it
 */
class toSqlTextWorker: public QObject {
	Q_OBJECT;
	friend class toSqlText;
signals:
	void finished();
	void processed();
	void error(QString err);

public:
	toSqlTextWorker(QObject *parent = 0);
	~toSqlTextWorker();

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
