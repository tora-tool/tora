
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

#include "editor/tohighlightedtext.h"
#include "core/toconnectiontraits.h"
#include "core/toconfiguration.h"
#include "core/tologger.h"
#include "core/utils.h"

#include <QtCore/QtDebug>
#include <QtGui/QListWidget>
#include <QtGui/QVBoxLayout>

#include <Qsci/qsciapis.h>
#include <Qsci/qsciabstractapis.h>
//#include <Qsci/qscilexersql.h>

toHighlightedText::toHighlightedText(QWidget *parent, const char *name)
    : toMarkedText(parent)
	, highlighterType(QtSql)
    //TODO, syntaxColoring(toConfigurationSingle::Instance().highlightType())
	// FIXME: disabled due repainting issues
	//, m_currentLineMarginHandle(QsciScintilla::markerDefine(QsciScintilla::RightArrow))
	, m_bookmarkMarginHandle(QsciScintilla::markerDefine(QsciScintilla::RightTriangle))
	, m_bookmarkHandle(QsciScintilla::markerDefine(QsciScintilla::Background))
	, m_analyzerNL(NULL)
	, m_analyzerOracle(NULL)
	, m_parserTimer(new QTimer(this))
	, m_parserThread(new QThread(this))
	, m_haveFocus(true)
	, m_complAPI(NULL)
{

#if defined(Q_OS_WIN)
	mono = QFont("Courier New", 10);
#elif defined(Q_OS_MAC)
	mono = QFont("Courier", 12);
#else
	mono = QFont(Utils::toStringToFont(toConfigurationSingle::Instance().codeFontName()));
#endif

    // Setup QsciScintilla stuff
    QsciScintilla::setFolding(QsciScintilla::BoxedFoldStyle);
    QsciScintilla::setAutoCompletionThreshold(1);
    QsciScintilla::setAutoCompletionSource(QsciScintilla::AcsAPIs);

    // highlight caret line
    QsciScintilla::setCaretLineVisible(true);
    // TODO setCaretLineBackgroundColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::CurrentLineMarker));
#ifdef SCI_LEXER
    // This is only required until transparency fixes in QScintilla go into stable release
    QsciScintilla::SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, QsciScintilla::SC_ALPHA_NOALPHA);
#else
    QsciScintilla::SendScintilla(/*QsciScintilla::*/SCI_SETCARETLINEBACKALPHA, 100);
#endif

    // handle "max text width" mark
    if (toConfigurationSingle::Instance().useMaxTextWidthMark())
    {
    	QsciScintilla::setEdgeColumn(toConfigurationSingle::Instance().maxTextWidthMark());
        // TODO setEdgeColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::CurrentLineMarker).darker(150));
    	QsciScintilla::setEdgeMode(QsciScintilla::EdgeLine);
    }
    else
    	QsciScintilla::setEdgeMode(QsciScintilla::EdgeNone);

    QsciScintilla::setAutoIndent(true);

    QsciScintilla::setMarginType(2, TextMarginRightJustified);
    QsciScintilla::setMarginWidth(2, QString::fromAscii("009"));

	m_parserTimer->setInterval(5000);   // every 5s
	m_parserTimer->setSingleShot(true); // repeat only if bg thread responded
	m_parserThread->setObjectName("ParserThread");
	m_worker = new toHighlightedTextWorker(NULL);
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
    complTimer = new QTimer(this);
    connect( complTimer, SIGNAL(timeout()), this, SLOT(autoCompleteFromAPIs()) );
	connect(&toHighlighterTypeButtonSingle::Instance(),
			SIGNAL(toggled(int)),
			this,
			SLOT(setHighlighter(int)));
	m_parserThread->start();
	setHighlighter(highlighterType);
	scheduleParsing();
}

void toHighlightedText::keyPressEvent(QKeyEvent * e)
{
    // handle editor shortcuts with TAB
    // It uses qscintilla lowlevel API to handle "word under cursor"
    // This code is taken from sqliteman.com
    if (toConfigurationSingle::Instance().useEditorShortcuts()
            && e->key() == Qt::Key_Tab)
    {
        int pos = SendScintilla(SCI_GETCURRENTPOS);
        int start = SendScintilla(SCI_WORDSTARTPOSITION, pos, true);
        int end = SendScintilla(SCI_WORDENDPOSITION, pos, true);
        SendScintilla(SCI_SETSELECTIONSTART, start, true);
        SendScintilla(SCI_SETSELECTIONEND, end, true);
        QString key(selectedText());
        EditorShortcutsMap shorts(toConfigurationSingle::Instance().editorShortcuts());
        if (shorts.contains(key))
        {
            removeSelectedText();
            insert(shorts.value(key).toString());
            SendScintilla(SCI_SETCURRENTPOS,
                          SendScintilla(SCI_GETCURRENTPOS) +
                          shorts.value(key).toString().length());
            pos = SendScintilla(SCI_GETCURRENTPOS);
            SendScintilla(SCI_SETSELECTIONSTART, pos, true);
            SendScintilla(SCI_SETSELECTIONEND, pos, true);
            return;
        }
        SendScintilla(SCI_SETSELECTIONSTART, pos, true);
        SendScintilla(SCI_SETSELECTIONEND, pos, true);
    }
    toMarkedText::keyPressEvent(e);
}

toHighlightedText::~toHighlightedText()
{
	m_parserThread->quit();
	m_parserThread->wait();
	delete m_parserThread;
}

void toHighlightedText::positionChanged(int row, int col)
{
	if (col > 0)
	{
		int position = this->SendScintilla(SCI_GETCURRENTPOS);
		position = SendScintilla(QsciScintilla::SCI_POSITIONBEFORE, position);
		char c = getByteAt(position);
		// TODO use getWCharAt and handle multibyte characters here
		if (c == '.')
			complTimer->start(500);
	}
	else
	{
		if (complTimer->isActive())
			complTimer->stop();
	}
// FIXME: disabled due repainting issues
    // current line marker (margin arrow)
//    markerDeleteAll(m_currentLineMarginHandle);
//    markerAdd(row, m_currentLineMarginHandle);
}

void toHighlightedText::autoCompleteFromAPIs()
{
    complTimer->stop(); // it's a must to prevent infinite reopening
	{
		toMarkedText::autoCompleteFromAPIs();
		return;
	}
}

void toHighlightedText::openFilename(const QString & file)
{
#pragma message WARN("TODO/FIXME: marks! toHighlightedTextEditor::openFilename")
//    toMarkedTextEditor::openFilename(file);
//
//    m_bookmarks.clear();
//    markerDeleteAll(m_bookmarkHandle);
//    markerDeleteAll(m_bookmarkMarginHandle);
//    setErrors(QMap<int, QString>());
}

#define declareStyle(style,color, paper, font) styleNames[style] = tr(#style); \
super::lexer()->setColor(color, style); \
super::lexer()->setPaper(paper, style); \
super::lexer()->setFont(font, style);

void toHighlightedText::setHighlighter(HighlighterTypeEnum h)
{
	// TODO handle bgthread working here
	QsciLexer *lexer = super::lexer();
	highlighterType = h;
	switch(highlighterType)
	{
	case None:
		if (m_analyzerNL == NULL)
			m_analyzerNL = new toSyntaxAnalyzerNL(this);
		m_currentAnalyzer = m_analyzerNL;
		m_worker->setAnalyzer(m_currentAnalyzer);
		setLexer(NULL);
		break;
	case QtSql:
		if (m_analyzerNL == NULL)
			m_analyzerNL = new toSyntaxAnalyzerNL(this);
		m_currentAnalyzer = m_analyzerNL;
		m_worker->setAnalyzer(m_currentAnalyzer);
		setLexer(m_currentAnalyzer ? m_currentAnalyzer->createLexer(this) : NULL);
		break;
	case Oracle:
		if( m_analyzerOracle == NULL)
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
	QString txt = QLatin1String(ENUM_NAME(toHighlightedText, HighlighterTypeEnum , highlighterType));
	TLOG(8, toDecorator, __HERE__) << " Lexer: " << txt << std::endl;

	QMetaEnum m_enum = toSyntaxAnalyzer::staticMetaObject.enumerator(toSyntaxAnalyzer::staticMetaObject.indexOfEnumerator("WordClassEnum"));
	for(int idx = 0; idx < m_enum.keyCount(); idx++)
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

	if(lexer) // delete the "old" lexer - if any
		delete lexer;

	if(super::lexer())
	{
		declareStyle(OneLine,
				QColor(Qt::black),
				QColor(toHighlightedText::lightCyan),
				mono);
		declareStyle(OneLineAlt,
				QColor(Qt::black),
				QColor(toHighlightedText::lightMagenta),
				mono);
		m_complAPI = super::lexer()->apis();
	} else {
		m_complAPI = NULL;
	}
	setFont(Utils::toStringToFont(toConfigurationSingle::Instance().codeFontName()));
	//update(); gets called by setFont
}

void toHighlightedText::setHighlighter(int h) // slot
{	
	QWidget *focus = qApp->focusWidget();

	if(focus == this)
		setHighlighter((HighlighterTypeEnum)h);
	TLOG(9, toDecorator, __HERE__) << " for: " << focus->metaObject()->className() << std::endl;
}

#ifdef QT_DEBUG
void toHighlightedText::reportFocus()
{
	QWidget *focus = qApp->focusWidget();
	TLOG(9, toDecorator, __HERE__) << (focus ? focus->metaObject()->className() : QString("NULL")) << std::endl;
}
#endif

void toHighlightedText::setFont (const QFont & font)
{
    // Only sets font lexer - one for all styles
    // this may (or may not) need to be changed in a future
	QsciLexer *lexer = super::lexer();
    if (lexer)
    {
        lexer->setDefaultFont(font);
        lexer->setFont(font);

        /* this is workaround against qscintilla 1.6 setFont(font) bug */
        if( qobject_cast<QsciLexerSQL*>(lexer))
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

void toHighlightedText::tableAtCursor(toCache::ObjectRef &table)
{
	/**
	 * Theoretically I could use here SQLLexer::Lexer API if it was implemented for current database.
	 * QScintilla API can not be used:
	 *   - SCI_WORDENDPOSITION, SCI_WORDENDPOSITION do ignore punctuation chars like ('.')
	 *   - Also toMarkedText::wordAtPosition() can not be used as string tokenizer
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
			QStringList words; words << QString::null << QString::null;
        	// pointer onto "current" word in the words list;
        	int idx = -1;
        	CharClassify::cc cls =	m_charClasifier.GetClass(getByteAt(line_start));
        	while(true)
        	{
        		// Query character class
        		CharClassify::cc new_cls =	m_charClasifier.GetClass(getByteAt(p));
        		if (new_cls != CharClassify::ccWord)
        		{
        			int style = SendScintilla(QsciScintilla::SCI_GETSTYLEAT, p) & 0x1f;
        			if( style == QsciLexerSQL::DoubleQuotedString || style == QsciLexerSQL::SingleQuotedString)
        				new_cls = CharClassify::ccWord; // override character class if it's quoted
        		}
        		// Check class change
        		if ( new_cls != cls)
        		{
        			// new token starts
            	    SendScintilla(QsciScintilla::SCI_GETTEXTRANGE, r, p, buf);
        			QString s = convertTextS2Q(buf);
        			r = p;
        			if (cls == CharClassify::ccPunctuation || cls == CharClassify::ccWord)
        			{
        				words << s;
        				if (p >= pos && idx == -1)
        					idx = words.size() -1;
        			}
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
						words << s;
						if (p >= pos && idx == -1)
							idx = words.size() -1;
					}
					break;
				}
        	}
        	delete []buf;
        	words << QString::null << QString::null;
            QString sa = words.at((std::max)(idx-2, 0));
            QString sb = words.at((std::max)(idx-1, 0)); // prev word
            QString sc = words.at((std::max)(idx, 0));  // "current" word
            QString sd = words.at((std::min)(idx+1, words.size() -1)); // next word
            QString se = words.at((std::min)(idx+2, words.size() -1));
            if ( sc == ".")
            {
            	table.first  = sb;
            	table.second = sd;
            } else if ( sb == "." && !sa.isEmpty()) {
            	table.first  = sa;
            	table.second = sc;
            } else if ( sd == "." && !se.isEmpty()) {
            	table.first  = sc;
            	table.second = se;
            } else {
            	table.second = sc;
            }
        }
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    }
}

toSyntaxAnalyzer* toHighlightedText::analyzer()
{
	return m_currentAnalyzer;
}

void toHighlightedText::handleBookmark()
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);

    if (m_bookmarks.contains(curline))
    {
        markerDelete(curline, m_bookmarkHandle);
        markerDefine(curline, m_bookmarkMarginHandle);
        m_bookmarks.removeAll(curline);
    }
    else
    {
        markerAdd(curline, m_bookmarkHandle);
        markerAdd(curline, m_bookmarkMarginHandle);
        m_bookmarks.append(curline);
    }
    qSort(m_bookmarks);
}

void toHighlightedText::gotoPrevBookmark()
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    --curline;

    int newline = -1;
    foreach(int i, m_bookmarks)
    {
        if (curline < i)
            break;
        newline = i;
    }
    if (newline >= 0)
        setCursorPosition(newline, 0);
}

void toHighlightedText::gotoNextBookmark()
{
    int curline, curcol;
    getCursorPosition (&curline, &curcol);
    ++curline;

    int newline = -1;
    foreach(int i, m_bookmarks)
    {
        if (curline > i)
            continue;
        newline = i;
        break;
    }
    if (newline >= 0)
        setCursorPosition(newline, 0);
}

QStringList toHighlightedText::getCompletionList(QString &partial)
{
    QStringList toReturn;
#if 0
    int curline, curcol;
    // used as a flag to prevent completion popup when there is
    // an orphan comma. In short - be less agressive on popup.
    bool showDefault = false;
    getCursorPosition (&curline, &curcol);

    QString line = text(curline);

    if (isReadOnly() || curcol == 0 || !toConfigurationSingle::Instance().codeCompletion())
        return toReturn;

    //throw QString("QStringList toHighlightedTextEditor::getCompletionList ... not implemented yet.");

    toSQLParse::editorTokenizer tokens(this, curcol, curline);
    if (curcol > 0 && line[curcol - 1] != '.')
    {
        partial = tokens.getToken(false);
        showDefault = true;
    }
    else
    {
        partial = "";
    }

    QString name = tokens.getToken(false);
    QString owner;
    if (name == ".")
    {
        name = tokens.getToken(false);
    }

    QString token = tokens.getToken(false);

    if (token == ".")
        owner = tokens.getToken(false);
    else
    {
        QString cmp = UpperIdent(name);
        QString lastToken;
        while ((invalidToken(tokens.line(), tokens.offset() + token.length()) || UpperIdent(token) != cmp || lastToken == ".") && token != ";" && token != "~~~" && !token.isEmpty())
        {
            lastToken = token;
            token = tokens.getToken(false);
        }

        if (token == ";" || token.isEmpty())
        {
            tokens.setLine(curline);
            tokens.setOffset(curcol);
            token = tokens.getToken();
            while ((invalidToken(tokens.line(), tokens.offset()) || (UpperIdent(token) != cmp && lastToken != ".")) && token != ";" && !token.isEmpty())
                token = tokens.getToken();
            lastToken = token;
            tokens.getToken(false);
        }
        if (token != ";" && !token.isEmpty())
        {
            token = tokens.getToken(false);
            if (token != "TABLE" && token != "UPDATE" && token != "FROM" && token != "INTO" && (Utils::toIsIdent(token[0]) || token[0] == '\"'))
            {
                name = token;
                token = tokens.getToken(false);
                if (token == ".")
                    owner = tokens.getToken(false);
            }
            else if (token == ")")
                return toReturn;
        }
    }
    if (!owner.isEmpty())
    {
        name = owner + QString::fromLatin1(".") + name;
    }
    if (!name.isEmpty())
    {
        try
        {
            toConnection &conn = toConnection::currentConnection(this);
            toConnection::objectName object = conn.realName(name, false);
            if(object.Type == "DATABASE")
            {
                std::list<toConnection::objectName> list = conn.tables(object);
                Q_FOREACH(toConnection::objectName table, list)
                {
                    QString t = conn.quote(table.Name, false);
                    if(t.indexOf(*partial) == 0)
                        toReturn.append(t);
                }
            }
            else
            {
                ///const toQDescList &desc = conn.columns(object);
                for (toQDescList::const_iterator i = desc.begin(); i != desc.end(); i++)
                {
                    QString t;
                    int ind = (*i).Name.indexOf("(");
                    if (ind < 0)
                        ind = (*i).Name.indexOf("RETURNING") - 1; //it could be a function or procedure without parameters. -1 to remove the space
                    if (ind >= 0)
                        t = conn.quote((*i).Name.mid(0, ind), false) + (*i).Name.mid(ind);
                    else
                        t = conn.quote((*i).Name, false);
                    if (t.indexOf(*partial) == 0)
                        toReturn.append(t);
                }
            }
        }
        catch (QString const &e)
        {
            TLOG(2, toDecorator, __HERE__) << "toHighlightedTextEditor::getCompletionList:" << e << std::endl;
        }
        catch (...)
        {
            TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
            TLOG(2, toDecorator, __HERE__) << "toHighlightedTextEditor::getCompletionList: Unknown error.";
        }
    }

    // if is toReturn empty fill it with keywords...
    if (showDefault && toReturn.count() == 0)
    {
        for (int i = 0; i < defaultCompletion.size(); ++i)
        {
            if (defaultCompletion.at(i).startsWith(partial, Qt::CaseInsensitive))
                toReturn.append(defaultCompletion.at(i));
        }
    }

    toReturn.sort();
#endif
    return toReturn;
}

void toHighlightedText::completeWithText(QString itemText)
{
    int curline, curcol, start, end;
    getCursorPosition (&curline, &curcol);
    QString line = text(curline);

	throw QString("void toHighlightedTextEditor::completeWithText ... not implemented yet.");
    //toSQLParse::editorTokenizer tokens(this, curcol, curline);
    //if (line[curcol - 1] != '.')
    //{
    //    tokens.getToken(false);
    //    start = tokens.offset();
    //}
    //else
    //{
    //    start = curcol;
    //}
    //if (line[curcol].isSpace())
    //{
    //    end = curcol;
    //}
    //else
    //{
    //    tokens.getToken(true);
    //    if (tokens.line() != curline)
    //        end = line.length();
    //    else
    //        end = tokens.offset();
    //}

    disconnect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));
    setSelection(curline, start, curline, end);
    this->removeSelectedText();
    this->insert(itemText);
    this->setCursorPosition(curline, start + itemText.length());
    connect (this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));
}

void toHighlightedText::focusInEvent(QFocusEvent *e)
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

void toHighlightedText::focusOutEvent(QFocusEvent *e)
{
	m_haveFocus = false;
	toHighlighterTypeButtonSingle::Instance().setDisabled(true);
	toHighlighterTypeButtonSingle::Instance().setValue(None);
	unScheduleParsing();
	super::focusOutEvent(e);
}

void toHighlightedText::scheduleParsing()
{
	if(m_haveFocus && !m_parserTimer->isActive())
		m_parserTimer->start();
}

void toHighlightedText::unScheduleParsing()
{
	if(m_parserTimer->isActive())
		m_parserTimer->stop();
}

void toHighlightedText::process()
{
	emit parsingRequested(text());
}

void toHighlightedText::processed()
{
	if(!m_haveFocus) // response was received after the focus was lost
		return;

	Style style = OneLine;
	unsigned lastLine = 0;
	Q_FOREACH(const toSyntaxAnalyzer::statement &r, m_worker->statements)
	{
		// "clear" line numbers before the statement
		while(lastLine < r.lineFrom)
		{
			setMarginText(lastLine++, QString(), Default);
		}

		// "draw" line numbers for the sql statement
		for(unsigned int i=r.lineFrom, j=1; i <= r.lineTo; ++i, ++j)
		{
			setMarginText(i, QString::number(j), style);
		}
		lastLine = r.lineTo + 1;
		style = style == OneLine ? OneLineAlt : OneLine;
	}

	// "clear" line numbers after the last statement
	while(lastLine < lines())
	{
		setMarginText(lastLine++, QString(), Default);
	}

	//emit parsingFinished();
	scheduleParsing();
}

toHighlightedTextWorker::toHighlightedTextWorker(QObject *parent)
	: QObject(parent)
	, analyzer(NULL)
{

}

toHighlightedTextWorker::~toHighlightedTextWorker()
{
}

void toHighlightedTextWorker::process(QString text)
{
	statements.clear();
	if(analyzer)
	{
		statements = analyzer->getStatements(text);
	}
	emit processed();
}

void toHighlightedTextWorker::setAnalyzer(toSyntaxAnalyzer *analyzer)
{
	this->analyzer = analyzer;
}

toHighlighterTypeButton::toHighlighterTypeButton(QWidget *parent, const char *name)
	: toToggleButton(toHighlightedText::staticMetaObject.enumerator(toHighlightedText::staticMetaObject.indexOfEnumerator("HighlighterTypeEnum"))
	, parent
	, name
	)
{
}

toHighlighterTypeButton::toHighlighterTypeButton()
	: toToggleButton(toHighlightedText::staticMetaObject.enumerator(toHighlightedText::staticMetaObject.indexOfEnumerator("HighlighterTypeEnum"))
	, NULL
	)
{
}

QColor toHighlightedText::lightCyan =  QColor(Qt::cyan).light(180);
QColor toHighlightedText::lightMagenta = QColor(Qt::magenta).light(180);

