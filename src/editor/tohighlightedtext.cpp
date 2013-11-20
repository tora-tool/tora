
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
#include "editor/tocomplpopup.h"
#include "core/toconnectiontraits.h"
#include "core/toconfiguration.h"
#include "core/tologger.h"
#include "core/utils.h"

#include <QtCore/QtDebug>
#include <QtGui/QListWidget>
#include <QtGui/QVBoxLayout>

#include <Qsci/qsciapis.h>
#include <Qsci/qsciabstractapis.h>
#include <Qsci/qscilexersql.h>

toHighlightedText::toHighlightedText(QWidget *parent, const char *name)
    : toMarkedText(parent)
	, highlighterType(QtSql)
    //TODO, syntaxColoring(toConfigurationSingle::Instance().highlightType())
	// FIXME: disabled due repainting issues
	//, m_currentLineMarginHandle(QsciScintilla::markerDefine(QsciScintilla::RightArrow))
	, m_bookmarkMarginHandle(QsciScintilla::markerDefine(QsciScintilla::RightTriangle))
	, m_bookmarkHandle(QsciScintilla::markerDefine(QsciScintilla::Background))
	, defaultCompletion()
	, m_analyzerNL(NULL)
	, m_analyzerOracle(NULL)
	, m_parserTimer(new QTimer(this))
	, m_parserThread(new QThread(this))
	, m_haveFocus(true)
{
#if defined(Q_OS_WIN)
	mono = QFont("Courier New", 10);
#elif defined(Q_OS_MAC)
	mono = QFont("Courier", 12);
#else
	mono = QFont(Utils::toStringToFont(toConfigurationSingle::Instance().codeFontName()));
#endif

    // set default keywords for code completion
    QFile api(":/templates/completion.api");
    if (!api.open(QIODevice::ReadOnly | QIODevice::Text))
        QMessageBox::warning(this, tr("Init error"),
                             tr("Cannot read code completion API from %1").arg(api.fileName()));
    else
    {
        while (!api.atEnd())
        {
            QString s(api.readLine());
            defaultCompletion.append(s.trimmed());
        }
    }

    // Setup QsciScintilla stuff
    QsciScintilla::setFolding(QsciScintilla::BoxedFoldStyle);
    // enable syntax coloring if "Syntax highlighting" is on in editor preferences
    //setSyntaxColoring(toConfigurationSingle::Instance().highlight());
    //    updateSyntaxColor(toSyntaxAnalyzer::DebugBg);
    //    updateSyntaxColor(toSyntaxAnalyzer::ErrorBg);
    //    updateSyntaxColor(toSyntaxAnalyzer::CurrentLineMarker);
    //    updateSyntaxColor(toSyntaxAnalyzer::StaticBg);

    // highlight caret line
    QsciScintilla::setCaretLineVisible(true);
    // TODO setCaretLineBackgroundColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::CurrentLineMarker));
#ifdef SCI_LEXER
    // This is only required until transparency fixes in QScintilla go into stable release
    QsciScintilla::SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, QsciScintilla::SC_ALPHA_NOALPHA);
#else
    QsciScintilla::SendScintilla(QsciScintilla::SCI_SETCARETLINEBACKALPHA, 100);
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
    //complAPI = new QsciAPIs(lexer);
    connect (this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(positionChanged(int, int)));
    complTimer = new QTimer(this);
    connect( complTimer, SIGNAL(timeout()), this, SLOT(autoCompleteFromAPIs()) );
    popup = new toComplPopup(this);
    popup->hide();
    connect(popup->list(),
            SIGNAL(itemClicked(QListWidgetItem*)),
            this,
            SLOT(completeFromAPI(QListWidgetItem*)));
    connect(popup->list(),
            SIGNAL(itemActivated(QListWidgetItem*)),
            this,
            SLOT(completeFromAPI(QListWidgetItem*)));
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
    // It uses qscintilla lowlevel API to handle "word unde cursor"
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
    if (complAPI)
        //delete complAPI;
    if (popup)
        delete popup;
}

void toHighlightedText::positionChanged(int row, int col)
{
    if (col > 0 && this->text(row)[col - 1] == '.')
    {
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

//static QString UpperIdent(const QString &str)
//{
//    if (str.length() > 0 && str[0] == '\"')
//        return str;
//    else
//        return str.toUpper();
//}

void toHighlightedText::autoCompleteFromAPIs()
{
    complTimer->stop(); // it's a must to prevent infinite reopening

    QListWidget *list = popup->list();
    QString partial;
    QStringList compleList = this->getCompletionList(partial);

    if (compleList.count() == 0)
        return;

    if (compleList.count() == 1 && compleList.first() == partial)
        this->completeWithText(compleList.first());
    else
    {
        long position, posx, posy;
        int curCol, curRow;
        this->getCursorPosition(&curRow, &curCol);
        position = this->SendScintilla(SCI_GETCURRENTPOS);
        posx = this->SendScintilla(SCI_POINTXFROMPOSITION, 0, position);
        posy = this->SendScintilla(SCI_POINTYFROMPOSITION, 0, position) +
               this->SendScintilla(SCI_TEXTHEIGHT, curRow);
        QPoint p(posx, posy);
        p = mapToGlobal(p);
        popup->move(p);
        list->clear();
        list->addItems(compleList);
        if (!partial.isNull() && partial.length() > 0)
        {
            int i;
            for (i = 0; i < list->model()->rowCount(); i++)
            {
                if (list->item(i)->text().indexOf(partial) == 0)
                {
                    list->item(i)->setSelected(true);
                    list->setCurrentItem(list->item(i));
                    break;
                }
            }
        }

        // if there's no current selection, select the first
        // item. that way arrow keys work as intended.
        QList<QListWidgetItem *> selected = list->selectedItems();
        if (selected.size() < 1 && list->count() > 0)
        {
            list->item(0)->setSelected(true);
            list->setCurrentItem(list->item(0));
        }

        popup->show();
        popup->setFocus();
    }
}

//bool toHighlightedTextEditor::invalidToken(int line, int col)
//{
//    bool ident = true;
//    if (line < 0)
//    {
//        line = 0;
//        col = 0;
//    }
//    while (line < lines())
//    {
//        QString cl = text(line);
//        while (col < int(cl.length()))
//        {
//            QChar c = cl[col];
//            if (!Utils::toIsIdent(c))
//                ident = false;
//            if (!ident && !c.isSpace())
//                return c == '.';
//            col++;
//        }
//        line++;
//        col = 0;
//    }
//    return false;
//}

/**
 * Sets the syntax colouring flag.
 */
//void toHighlightedTextEditor::setSyntaxColoring(bool val)
//{
//    syntaxColoring = val;
//    if (syntaxColoring)
//    {
//        QsciScintilla::setLexer(lexer);
//        //        updateSyntaxColor(toSyntaxAnalyzer::Default);
//        //        updateSyntaxColor(toSyntaxAnalyzer::Comment);
//        //        updateSyntaxColor(toSyntaxAnalyzer::Number);
//        //        updateSyntaxColor(toSyntaxAnalyzer::Keyword);
//        //        updateSyntaxColor(toSyntaxAnalyzer::String);
//        //        updateSyntaxColor(toSyntaxAnalyzer::DefaultBg);
//        //        updateSyntaxColor(toSyntaxAnalyzer::CurrentLineMarker);
//
//        update();
//    }
//    else
//    {
//        QsciScintilla::setLexer(NULL);
//    }
//}

/**
 * Sets the syntax colours for given type
 */
//void toHighlightedTextEditor::updateSyntaxColor(toSyntaxAnalyzer::infoType t)
//{
//    QColor col = DefaultAnalyzer.getColor(t);
//
//    switch (t)
//    {
//    case toSyntaxAnalyzer::Default:
//        lexer->setColor(col, QsciLexerSQL::Default);
//        //lexer->setColor(col, QsciLexerSQL::CommentLineHash);
//        break;
//    case toSyntaxAnalyzer::Comment:
//        lexer->setColor(col, QsciLexerSQL::Comment);
//        lexer->setColor(col, QsciLexerSQL::CommentLine);
//        lexer->setColor(col, QsciLexerSQL::PlusPrompt);
//        lexer->setColor(col, QsciLexerSQL::PlusComment);
//        lexer->setColor(col, QsciLexerSQL::CommentDoc);
//        lexer->setColor(col, QsciLexerSQL::CommentDocKeyword);
//        break;
//    case toSyntaxAnalyzer::Number:
//        lexer->setColor(col, QsciLexerSQL::Number);
//        break;
//    case toSyntaxAnalyzer::Keyword:
//        lexer->setColor(col, QsciLexerSQL::Keyword);
//        lexer->setColor(col, QsciLexerSQL::PlusKeyword);
//        lexer->setColor(col, QsciLexerSQL::Operator);
//        break;
//    case toSyntaxAnalyzer::String:
//        lexer->setColor(col, QsciLexerSQL::DoubleQuotedString);
//        lexer->setColor(col, QsciLexerSQL::SingleQuotedString);
//        break;
//    case toSyntaxAnalyzer::DefaultBg:
//        lexer->setPaper(col);
//        //lexer->setPaper(col, QsciLexerSQL::Default);
//        break;
//    case toSyntaxAnalyzer::ErrorBg:
//        setMarkerBackgroundColor(col, m_errorHandle);
//        break;
//    case toSyntaxAnalyzer::DebugBg:
//        setMarkerBackgroundColor(col, m_debugHandle);
//        break;
//    case toSyntaxAnalyzer::StaticBg:
//        setMarkerBackgroundColor(col, m_staticHandle);
//        break;
//    case toSyntaxAnalyzer::CurrentLineMarker:
////         setMarkerBackgroundColor(col, m_currentLineMarginHandle);
//        // TODO/FIXME?: make it configurable - color.
//        setMarkerBackgroundColor(DefaultAnalyzer.getColor(toSyntaxAnalyzer::CurrentLineMarker).lighter(100),
//                                 m_bookmarkHandle);
//        break;
//    default:
//        break;
//    }
//}


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

void toHighlightedText::tableAtCursor(toCache::ObjectRef &table, bool mark)
{
    try
    {
    	//toConnection const& conn = toConnection::currentConnection(this);
        int curline, curcol;
        getCursorPosition (&curline, &curcol);
        int pos = positionFromLineIndex(curline, curcol);

        QString word = super::wordAtLineIndex(curline, curcol);
        // TODO parse also schema "DOT" ...
        if (mark)
        {
        	//setSelection(tokens.line(), tokens.offset(), lastTokens.line(), lastTokens.offset());
        }
        table.second = word;
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
    int curline, curcol;
    // used as a flag to prevent completion popup when there is
    // an orphan comma. In short - be less agressive on popup.
    bool showDefault = false;
    QStringList toReturn;
    getCursorPosition (&curline, &curcol);

    QString line = text(curline);

    if (isReadOnly() || curcol == 0 || !toConfigurationSingle::Instance().codeCompletion())
        return toReturn;

    //throw QString("QStringList toHighlightedTextEditor::getCompletionList ... not implemented yet.");

    //toSQLParse::editorTokenizer tokens(this, curcol, curline);
    //if (curcol > 0 && line[curcol - 1] != '.')
    //{
    //    partial = tokens.getToken(false);
    //    showDefault = true;
    //}
    //else
    //{
    //    partial = "";
    //}

    //QString name = tokens.getToken(false);
    //QString owner;
    //if (name == ".")
    //{
    //    name = tokens.getToken(false);
    //}

    //QString token = tokens.getToken(false);

    //if (token == ".")
    //    owner = tokens.getToken(false);
    //else
    //{
    //    QString cmp = UpperIdent(name);
    //    QString lastToken;
    //    while ((invalidToken(tokens.line(), tokens.offset() + token.length()) || UpperIdent(token) != cmp || lastToken == ".") && token != ";" && token != "~~~" && !token.isEmpty())
    //    {
    //        lastToken = token;
    //        token = tokens.getToken(false);
    //    }

    //    if (token == ";" || token.isEmpty())
    //    {
    //        tokens.setLine(curline);
    //        tokens.setOffset(curcol);
    //        token = tokens.getToken();
    //        while ((invalidToken(tokens.line(), tokens.offset()) || (UpperIdent(token) != cmp && lastToken != ".")) && token != ";" && !token.isEmpty())
    //            token = tokens.getToken();
    //        lastToken = token;
    //        tokens.getToken(false);
    //    }
    //    if (token != ";" && !token.isEmpty())
    //    {
    //        token = tokens.getToken(false);
    //        if (token != "TABLE" && token != "UPDATE" && token != "FROM" && token != "INTO" && (Utils::toIsIdent(token[0]) || token[0] == '\"'))
    //        {
    //            name = token;
    //            token = tokens.getToken(false);
    //            if (token == ".")
    //                owner = tokens.getToken(false);
    //        }
    //        else if (token == ")")
    //            return toReturn;
    //    }
    //}
    //if (!owner.isEmpty())
    //{
    //    name = owner + QString::fromLatin1(".") + name;
    //}
    //if (!name.isEmpty())
    //{
    //    try
    //    {
    //        toConnection &conn = toConnection::currentConnection(this);
    //        ///toConnection::objectName object = conn.realName(name, false);
    //        ///if(object.Type == "DATABASE")
    //        ///{
    //        ///    std::list<toConnection::objectName> list = conn.tables(object);
    //        ///    Q_FOREACH(toConnection::objectName table, list)
    //        ///    {
    //        ///        QString t = conn.quote(table.Name, false);
    //        ///        if(t.indexOf(*partial) == 0)
    //        ///            toReturn.append(t);
    //        ///    }
    //        ///}
    //        ///else
    //        ///{
    //        ///    ///const toQDescList &desc = conn.columns(object);
    //        ///    for (toQDescList::const_iterator i = desc.begin(); i != desc.end(); i++)
    //        ///    {
    //        ///        QString t;
    //        ///        int ind = (*i).Name.indexOf("(");
    //        ///        if (ind < 0)
    //        ///            ind = (*i).Name.indexOf("RETURNING") - 1; //it could be a function or procedure without parameters. -1 to remove the space
    //        ///        if (ind >= 0)
    //        ///            t = conn.quote((*i).Name.mid(0, ind), false) + (*i).Name.mid(ind);
    //        ///        else
    //        ///            t = conn.quote((*i).Name, false);
    //        ///        if (t.indexOf(*partial) == 0)
    //        ///            toReturn.append(t);
    //        ///    }
    //        ///}
    //    }
    //    catch (QString const &e)
    //    {
    //        TLOG(2, toDecorator, __HERE__) << "toHighlightedTextEditor::getCompletionList:" << e << std::endl;
    //    }
    //    catch (...)
    //    {
    //        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
    //        TLOG(2, toDecorator, __HERE__) << "toHighlightedTextEditor::getCompletionList: Unknown error.";
    //    }
    //}

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

void toHighlightedText::completeFromAPI(QListWidgetItem* item)
{
    if (item)
        this->completeWithText(item->text());
    popup->hide();
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
	//emit parsingStarted();
	//	int len = editor()->length();
	//	if(bufferLength < len)
	//	{
	//		bufferLength = Utils::toNextPowerOfTwo(len);
	//		bufferText = (char*) realloc(bufferText, bufferLength);
	//	}
	//	editor()->SendScintilla(QsciScintillaBase::SCI_GETTEXT, bufferLength, bufferText);
	//	bufferText[len] = '\0';
	//  emit parsingRequested(bufferText, len);
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

