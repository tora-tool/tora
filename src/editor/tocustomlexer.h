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

#ifndef TOCUSTOMLEXER_H_
#define TOCUSTOMLEXER_H_

#include "parsing/tsqllexer.h"

#include <QtCore/QObject>
#include <Qsci/qscilexercustom.h>

class QsciStyle;

class toCustomLexerWorker;

typedef QPair<unsigned, unsigned> StmtLineRange;

class toCustomLexer : public QsciLexerCustom {
        Q_OBJECT;

public:
        enum Style {
                Default = 0,
                Comment,
                CommentMultiline,
                Reserved,
                Builtin,
                String,
                Failure,
                OneLine,
                OneLineAlt,
                MaxStyle
        };

        toCustomLexer(QObject *parent = 0);
        virtual ~toCustomLexer();

        /* override - reimplemented from QsciLexerCustom */
        virtual const char *language() const;
        virtual QString description(int) const;
        virtual void styleText(int start, int end);

signals:
		void parsingStarted();
		void parsingFinished();
		void parsingRequested(const char*, unsigned);

private slots:
		void process();
		void processed();

public slots:
        void setOracle();
        void setMySQL();

private:
        bool pushStyle(int style);
        bool popStyle();
        int  getStyle();
        bool hasStyle(int);
        void resetStyle();

        void schedule();

        char *lineText, *bufferText;
        unsigned lineLength, bufferLength;

        QMap<int,QString> styleNames;
    	QList<int> styleStack;
    	std::auto_ptr <SQLLexer::Lexer> lexer;

    	QThread *thread;
    	toCustomLexerWorker *worker;
};

/* Utility class for @ref toCustomLexer
 * Instance of this class "lives" within background thread
 * and dispatches signals from/to the main thread
 *
 * NOTE: this class could by nested, but QT does not support it
 */
class toCustomLexerWorker: public QObject {
	Q_OBJECT;
	friend class toCustomLexer;
signals:
	void finished();
	void processed();
	void error(QString err);

public:
	toCustomLexerWorker(QObject *parent = 0);
	~toCustomLexerWorker();

public slots:
	void process(const char*, unsigned int len);

protected:
	QList<StmtLineRange> statements;
};

#endif /* TOCUSTOMLEXER_H_ */
