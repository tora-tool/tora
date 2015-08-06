
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

#include "editor/tosqltext.h"
#include "core/toconnection.h"
#include "core/toconnectiontraits.h"
#include "core/tologger.h"
#include "core/utils.h"
#include "core/toconfiguration_new.h"
#include "editor/toworksheettext.h"

#include <QListWidget>
#include <QVBoxLayout>

#include <Qsci/qsciapis.h>
#include <Qsci/qsciabstractapis.h>
//#include <Qsci/qscilexersql.h>

toSqlText::toSqlText(QWidget *parent, const char *name)
    : toScintilla(parent)
    , highlighterType(QsciSql)
    // FIXME: disabled due repainting issues
    //, m_currentLineMarginHandle(QsciScintilla::markerDefine(QsciScintilla::RightArrow))
    , m_analyzerNL(NULL)
    , m_analyzerOracle(NULL)
    , m_parserTimer(new QTimer(this))
    , m_parserThread(new QThread(this))
    , m_haveFocus(true)
{
    using namespace ToConfiguration;
#if defined(Q_OS_WIN)
    mono = QFont("Courier New", 10);
#elif defined(Q_OS_MAC)
    mono = QFont("Courier", 12);
#else
    mono = QFont(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfCodeFont).toString()));
#endif

    // Setup QsciScintilla stuff
    // QsciScintilla::setFolding(QsciScintilla::BoxedFoldStyle);

    QsciScintilla::setMarginType(2, TextMarginRightJustified);
    QsciScintilla::setMarginWidth(2, QString("009"));

    m_parserTimer->setInterval(5000);   // every 5s
    m_parserTimer->setSingleShot(true); // repeat only if bg thread responded
    m_parserThread->setObjectName("ParserThread");
    m_worker = new toSqlTextWorker(NULL);
    m_worker->moveToThread(m_parserThread);
    connect(m_parserTimer, SIGNAL(timeout()), this, SLOT(process()));
    connect(this, SIGNAL(parsingRequested(QString)),  m_worker, SLOT(process(QString)));
    connect(m_worker, SIGNAL(processed()), this, SLOT(processed()));
    connect(m_worker, SIGNAL(finished()),  m_parserThread, SLOT(quit()));
    connect(m_worker, SIGNAL(finished()),  m_worker, SLOT(deleteLater()));
    connect(m_parserThread, SIGNAL(finished()),  m_parserThread, SLOT(deleteLater()));

    // Connect signals&slots
    connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(setStatusMessage(void )));
    connect (this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));

    connect(&toHighlighterTypeButtonSingle::Instance(),
            SIGNAL(toggled(int)),
            this,
            SLOT(setHighlighter(int)));
    m_parserThread->start();
    setHighlighter(highlighterType);
    scheduleParsing();
}

void toSqlText::keyPressEvent(QKeyEvent * e)
{
#if TORA_INCREMENTAL_SEARCH
    //    if (Search)
    //    {
    //        bool ok = false;
    //        if (e->modifiers() == Qt::NoModifier && e->key() == Qt::Key_Backspace)
    //        {
    //            int len = SearchString.length();
    //            if (len > 0)
    //                SearchString.truncate(len - 1);
    //            ok = true;
    //        }
    //        else if (e->key() != Qt::Key_Escape)
    //        {
    //            QString t = e->text();
    //            if (t.length())
    //            {
    //                SearchString += t;
    //                ok = true;
    //            }
    //            else if (e->key() == Qt::Key_Shift ||
    //                     e->key() == Qt::Key_Control ||
    //                     e->key() == Qt::Key_Meta ||
    //                     e->key() == Qt::Key_Alt)
    //            {
    //                ok = true;
    //            }
    //        }
    //
    //        if (ok)
    //        {
    //            incrementalSearch(m_searchDirection, false);
    //            e->accept();
    //            return ;
    //        }
    //        else
    //        {
    //            Search = false;
    //            LastSearch = SearchString;
    //            Utils::toStatusMessage(QString::null);
    //        }
    //    }
#endif
    super::keyPressEvent(e);
}

toSqlText::~toSqlText()
{
    m_parserThread->quit();
    m_parserThread->wait();
    delete m_parserThread;
}

#define declareStyle(style,color, paper, font) styleNames[style] = tr(#style); \
    super::lexer()->setColor(color, style); \
    super::lexer()->setPaper(paper, style); \
    super::lexer()->setFont(font, style);

void toSqlText::setHighlighter(HighlighterTypeEnum h)
{
    using namespace ToConfiguration;

    // TODO handle bgthread working here
    QsciLexer *lexer = super::lexer();
    highlighterType = h;
    switch (highlighterType)
    {
        case None:
            if (m_analyzerNL == NULL)
                m_analyzerNL = new toSyntaxAnalyzerNL(this);
            m_currentAnalyzer = m_analyzerNL;
            m_worker->setAnalyzer(m_currentAnalyzer);
            setLexer(NULL);
            break;
        case QsciSql:
            if (m_analyzerNL == NULL)
                m_analyzerNL = new toSyntaxAnalyzerNL(this);
            m_currentAnalyzer = m_analyzerNL;
            m_worker->setAnalyzer(m_currentAnalyzer);
            setLexer(m_currentAnalyzer ? m_currentAnalyzer->createLexer(this) : NULL);
            break;
        case Oracle:
            if ( m_analyzerOracle == NULL)
                m_analyzerOracle = new toSyntaxAnalyzerOracle(this);
            m_currentAnalyzer = m_analyzerOracle;
            m_worker->setAnalyzer(m_currentAnalyzer);
            setLexer(m_currentAnalyzer ? m_currentAnalyzer->createLexer(this) : NULL);
            break;
        case Mysql:
            m_currentAnalyzer = NULL;
            m_worker->setAnalyzer(NULL);
            setLexer(m_currentAnalyzer ? m_currentAnalyzer->createLexer(this) : NULL);
    }
#ifdef QT_DEBUG
    QString txt = QLatin1String(ENUM_NAME(toSqlText, HighlighterTypeEnum , highlighterType));
    TLOG(8, toDecorator, __HERE__) << " Lexer: " << txt << std::endl;

    QMetaEnum m_enum = toSyntaxAnalyzer::staticMetaObject.enumerator(toSyntaxAnalyzer::staticMetaObject.indexOfEnumerator("WordClassEnum"));
    for (int idx = 0; idx < m_enum.keyCount(); idx++)
    {
        unsigned ival = m_enum.value(idx);
        QString  sval = m_enum.key(idx);
        TLOG(8, toNoDecorator, __HERE__) << "  Analyzer:" << sval << '(' << ival << ')' <<std::endl;
        if (super::lexer() == NULL)
            break;
        QColor c = super::lexer()->color(ival);
        QColor p = super::lexer()->paper(ival);
        QFont  f = super::lexer()->font(ival);
        TLOG(8, toNoDecorator, __HERE__) << "  Style:" << sval << std::endl
                                         << "   Fore:" << c.name() << '(' << c.red() << ' ' << c.green() << ' ' << c.blue() << ' ' << c.alpha() << ')' << std::endl
                                         << "   Back:" << p.name() << '(' << p.red() << ' ' << p.green() << ' ' << p.blue() << ' ' << p.alpha() << ')' << std::endl
                                         << "   Font:" << f.toString() << std::endl;
    }
#endif

    if (lexer) // delete the "old" lexer - if any
        delete lexer;

    if (super::lexer())
    {
        declareStyle(OneLine,
                     QColor(Qt::black),
                     QColor(toSqlText::lightCyan),
                     mono);
        declareStyle(OneLineAlt,
                     QColor(Qt::black),
                     QColor(toSqlText::lightMagenta),
                     mono);
    }

    setFont(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfCodeFont).toString()));
    //update(); gets called by setFont
}

void toSqlText::setHighlighter(int h) // slot
{
    QWidget *focus = qApp->focusWidget();

    if (focus == this)
        setHighlighter((HighlighterTypeEnum)h);
    TLOG(9, toDecorator, __HERE__) << " for: " << focus->metaObject()->className() << std::endl;
}

#ifdef QT_DEBUG
void toSqlText::reportFocus()
{
    QWidget *focus = qApp->focusWidget();
    TLOG(9, toDecorator, __HERE__) << (focus ? focus->metaObject()->className() : QString("NULL")) << std::endl;
}
#endif

void toSqlText::setFont (const QFont & font)
{
    // Only sets font lexer - one for all styles
    // this may (or may not) need to be changed in a future
    QsciLexer *lexer = super::lexer();
    if (lexer)
    {
        lexer->setDefaultFont(font);
        lexer->setFont(font);

        /* this is workaround against qscintilla 1.6 setFont(font) bug */
        if ( qobject_cast<QsciLexerSQL*>(lexer))
        {
            lexer->setFont(font, QsciLexerSQL::Default);
            lexer->setFont(font, QsciLexerSQL::Comment);
            lexer->setFont(font, QsciLexerSQL::CommentLine);
            lexer->setFont(font, QsciLexerSQL::PlusComment);
            lexer->setFont(font, QsciLexerSQL::CommentLineHash);
            lexer->setFont(font, QsciLexerSQL::CommentDocKeyword);
            lexer->setFont(font, QsciLexerSQL::CommentDocKeywordError);
            lexer->setFont(font, QsciLexerSQL::DoubleQuotedString);
            lexer->setFont(font, QsciLexerSQL::SingleQuotedString);
            lexer->setFont(font, QsciLexerSQL::PlusPrompt);
        }
        update();
    }
    super::setFont(font);
}

void toSqlText::tableAtCursor(toSqlText::Word &schema, toSqlText::Word &table)
{
    /**
     * Theoretically I could use here SQLLexer::Lexer API if it was implemented for current database.
     * QScintilla API can not be used:
     *   - SCI_WORDENDPOSITION, SCI_WORDENDPOSITION do ignore punctuation chars like ('.')
     *   - Also toScintilla::wordAtPosition() can not be used as string tokenizer
     *   - The closest implementation is Scintilla's MoveNextWordStart/MoveNextWordEnd but these methods are not accessible
     *
     *   This code loops over chars in the line buffer and queries the char-class and style for each char.
     *   If the class changes a new word is emitted into "words" buffer. The word under cursor is pointed by "idx"
     */
    try
    {
        //toConnection const& conn = toConnection::currentConnection(this);
        int curline, curcol;
        getCursorPosition (&curline, &curcol);
        int pos = positionFromLineIndex(curline, curcol);
        int line_end = SendScintilla(QsciScintilla::SCI_GETLINEENDPOSITION, curline);
        int line_start = positionFromLineIndex(curline, 0);
        {
            // Line buffer
            char *buf = new char[line_end - line_start + 1];
            // Buffer markers, r = word start, p = word end
            int p = line_start, r = line_start;
            QList<Word> words; // Pair word(QString), start position, end position
            words << Word() << Word();
            // pointer onto "current" word in the words list;
            int idx = -1;
            CharClassify::cc cls =	m_charClasifier.GetClass(getByteAt(line_start));
            while (true)
            {
                // Query character class
                CharClassify::cc new_cls =	m_charClasifier.GetClass(getByteAt(p));
                if (new_cls != CharClassify::ccWord)
                {
                    int style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, p) & 0x1f;
                    if ( style == QsciLexerSQL::DoubleQuotedString || style == QsciLexerSQL::SingleQuotedString)
                        new_cls = CharClassify::ccWord; // override character class if it's quoted
                }
                // Check class change
                if ( new_cls != cls)
                {
                    // new token starts
                    SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, r, p, buf);
                    QString s = convertTextS2Q(buf);
                    if (cls == CharClassify::ccPunctuation || cls == CharClassify::ccWord)
                    {
                        words << std::make_tuple(s, r, p);
                        if (p >= pos && idx == -1)
                            idx = words.size() -1;
                    }
                    r = p;
                    cls = new_cls;
                }
                // Advance to next char
                p = SendScintilla(QsciScintilla::SCI_POSITIONAFTER, p);
                // Check line end
                if (p >= line_end)
                {
                    // possibly emit the last word in the buffer
                    if (cls == CharClassify::ccPunctuation || cls == CharClassify::ccWord)
                    {
                        SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, r, p, buf);
                        QString s = convertTextS2Q(buf);
                        words << std::make_tuple(s, r, p);
                        if (p >= pos && idx == -1)
                            idx = words.size() -1;
                    }
                    break;
                }
            }
            delete []buf;
            words << Word() << Word();
            auto sa = words.at((std::max)(idx-2, 0));
            auto sb = words.at((std::max)(idx-1, 0)); // prev word
            auto sc = words.at((std::max)(idx, 0));  // "current" word
            auto sd = words.at((std::min)(idx+1, words.size() -1)); // next word
            auto se = words.at((std::min)(idx+2, words.size() -1));
            if ( sc.text() == ".")
            {
            	schema = sb;
            	table = sd;
            }
            else if ( sb.text() == "." && !sa.text().isEmpty())
            {
                schema  = sa;
                table = sc;
            }
            else if ( sd.text() == "." && !se.text().isEmpty())
            {
                schema = sc;
                table = se;
            }
            else
            {
            	table = sc;
            }
        }
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
}

void toSqlText::tableAtCursor(toCache::ObjectRef &objectRef)
{
	Word schema, table;
	tableAtCursor(schema,table);
	objectRef.first = schema.text();
	objectRef.second = table.text();
}

toSyntaxAnalyzer* toSqlText::analyzer()
{
    return m_currentAnalyzer;
}

void toSqlText::focusInEvent(QFocusEvent *e)
{
#ifdef QT_DEBUG
    QTimer::singleShot(1000, this, SLOT(reportFocus()));
#endif

    m_haveFocus = true;
    toHighlighterTypeButtonSingle::Instance().setEnabled(true);
    toHighlighterTypeButtonSingle::Instance().setValue(highlighterType);
    TLOG(9, toDecorator, __HERE__) << this << " " << highlighterType << std::endl;
    scheduleParsing();
    super::focusInEvent(e);
}

void toSqlText::focusOutEvent(QFocusEvent *e)
{
    m_haveFocus = false;
    toHighlighterTypeButtonSingle::Instance().setDisabled(true);
    toHighlighterTypeButtonSingle::Instance().setValue(None);
    unScheduleParsing();
    super::focusOutEvent(e);
}

void toSqlText::scheduleParsing()
{
    if (m_haveFocus && !m_parserTimer->isActive())
        m_parserTimer->start();
}

void toSqlText::unScheduleParsing()
{
    if (m_parserTimer->isActive())
        m_parserTimer->stop();
}

void toSqlText::process()
{
    emit parsingRequested(text());
}

void toSqlText::processed()
{
    if (!m_haveFocus) // response was received after the focus was lost
        return;

    Style style = OneLine;
    unsigned lastLine = 0;
    Q_FOREACH(const toSyntaxAnalyzer::statement &r, m_worker->statements)
    {
        // "clear" line numbers before the statement
        while (lastLine < r.lineFrom)
        {
            setMarginText(lastLine++, QString(), Default);
        }

        // "draw" line numbers for the sql statement
        for (unsigned int i=r.lineFrom, j=1; i <= r.lineTo; ++i, ++j)
        {
            setMarginText(i, QString::number(j), style);
        }
        lastLine = r.lineTo + 1;
        style = style == OneLine ? OneLineAlt : OneLine;
    }

    // "clear" line numbers after the last statement
    while (lastLine < lines())
    {
        setMarginText(lastLine++, QString(), Default);
    }

    //emit parsingFinished();
    scheduleParsing();
}

toSqlTextWorker::toSqlTextWorker(QObject *parent)
    : QObject(parent)
    , analyzer(NULL)
{

}

toSqlTextWorker::~toSqlTextWorker()
{
}

void toSqlTextWorker::process(QString text)
{
    statements.clear();
    if (analyzer)
    {
        statements = analyzer->getStatements(text);
    }
    emit processed();
}

void toSqlTextWorker::setAnalyzer(toSyntaxAnalyzer *analyzer)
{
    this->analyzer = analyzer;
}

toHighlighterTypeButton::toHighlighterTypeButton(QWidget *parent, const char *name)
    : toToggleButton(toSqlText::staticMetaObject.enumerator(toSqlText::staticMetaObject.indexOfEnumerator("HighlighterTypeEnum"))
                     , parent
                     , name
                    )
{
}

toHighlighterTypeButton::toHighlighterTypeButton()
    : toToggleButton(toSqlText::staticMetaObject.enumerator(toSqlText::staticMetaObject.indexOfEnumerator("HighlighterTypeEnum"))
                     , NULL
                    )
{
}

QColor toSqlText::lightCyan =  QColor(Qt::cyan).light(180);
QColor toSqlText::lightMagenta = QColor(Qt::magenta).light(180);

