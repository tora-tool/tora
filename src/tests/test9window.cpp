
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

#include "tests/test9window.h"
#include "editor/tosqltext.h"
#include "editor/todifftext.h"
#include "core/toconfiguration.h"
#include "core/toeditorconfiguration.h"

//#include <dtl/dtl.hpp>
//#include "Qsci/qscilexerdiff.h"

#include <QStatusBar>
#include <QScrollArea>
#include <QSplitter>
#include <QToolBar>

class Line
{
public:
    Line(QString _txt, unsigned _number) : number(_number), txt(_txt) {};
    Line(const Line& other) : number(other.number) , txt(other.txt) {};

    Line& operator=(const Line& other)
    {
        number = other.number;
        txt = other.txt;
        return *this;
    }

    unsigned number;
    QString txt;
};

inline bool operator==(const Line& lhs, const Line& rhs)
{
    return lhs.txt == rhs.txt;
}
inline bool operator!=(const Line& lhs, const Line& rhs){ return !(lhs == rhs); }

Test9Window::Test9Window(const QString &sql1, const QString &sql2)
    : Ui::Test9Window()
    , oldSql(sql1)
    , newSql(sql2)
{
    //using dtl::Diff;
    using namespace ToConfiguration;

    //typedef wchar_t elem;
    //typedef std::wstring sequence;
    typedef Line elem;
    typedef std::vector<Line> sequence;

    Ui::Test9Window::setupUi(this);

    QToolBar *toolBar = QMainWindow::addToolBar("TB");
    toolBar->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    toolBar->setFocusPolicy(Qt::NoFocus);

    {
        editorLeft = new toSqlText(this);
        leftLayout->addWidget(editorLeft);
        editorLeft->setText(oldSql);
        editorLeft->enableToolTips();
        editorLeft->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        editorLeft->viewport()->installEventFilter(this);
    }

    {
        editorRight = new toSqlText(this);
        rightLayout->addWidget(editorRight);
        editorRight->setText(newSql);
        editorRight->setMarginType(2, QsciScintilla::TextMarginRightJustified);
        editorRight->setMarginWidth(2, QString::fromLatin1("009"));
        editorRight->enableToolTips();
        editorRight->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        editorRight->viewport()->installEventFilter(this);
    }

#if 0
    {
        QFont mono(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfCodeFont).toString()));
        editorDiff = new toScintilla(this);
        bottomVerticalLayout->addWidget(editorDiff);
        editorDiff->viewport()->installEventFilter(this);
        editorDiff->setFont(mono);
        editorDiff->setLexer(new QsciLexerDiff(editorDiff));

        sequence A; // sql1.toStdWString());
        sequence B; // sql2.toStdWString());

        Diff< elem, sequence > d(A, B);
        d.compose();
        std::cout << "editDistance:" << d.getEditDistance() << endl;

        auto seq = d.getSes().getSequence();
        char *txt = new char[seq.size() * 2];
        unsigned pos = 0;

        QString qs;
#if 0
        foreach(auto &e, seq)
        {
            char mb[MB_LEN_MAX];
            wchar_t wc = e.first;
            auto info  = e.second;
            long long beforeIdx = info.beforeIdx;
            long long afterIdx  = info.afterIdx;
            dtl::edit_t type    = info.type;

            int i = wctomb(mb,wc);

            txt[pos++] = mb[0];
            switch(type)
            {
                case dtl::SES_DELETE:
                    txt[pos++] = QsciLexerDiff::LineRemoved;
                    break;
                case dtl::SES_COMMON:
                    txt[pos++] = QsciLexerDiff::Default;
                    break;
                case dtl::SES_ADD:
                    txt[pos++] = QsciLexerDiff::LineAdded;
                    break;
                default:
                    txt[pos++] = QsciLexerDiff::Comment;
            }

            QString s = QString::fromWCharArray(&wc, 1);
			qs += s;
        }
        editorDiff->addStyledText(seq.size()*2, txt);
#endif

    }
#endif

    editorDiff = new toDiffText(this);
    bottomLayout->addWidget(editorDiff);
    editorDiff->viewport()->installEventFilter(this);

    editorDiff->setText(oldSql, newSql);

    connect(actionLoad_Old, SIGNAL(triggered()), this, SLOT(load_old()));
    connect(actionLoad_New, SIGNAL(triggered()), this, SLOT(load_new()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    connect(verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollVerticaly(int)));

    QMainWindow::show();
}

void Test9Window::load_old()
{
    QString fn = Utils::toOpenFilename("*.sql", this);
    if (!fn.isEmpty())
    {
        QString oldSql = Utils::toReadFile(fn);
        editorLeft->setText(oldSql);
        editorDiff->setText(oldSql, newSql);
    }
}

void Test9Window::load_new()
{
    QString fn = Utils::toOpenFilename("*.sql", this);
    if (!fn.isEmpty())
    {
        newSql = Utils::toReadFile(fn);
        editorRight->setText(newSql);
        editorDiff->setText(oldSql, newSql);
    }
}

void Test9Window::scrollVerticaly(int value)
{
    editorLeft->verticalScrollBar()->setSliderPosition(value);
    editorRight->verticalScrollBar()->setSliderPosition(value);
}

bool Test9Window::eventFilter(QObject *obj, QEvent* evt)
{
    if (evt->type() == QEvent::Wheel && (editorRight->viewport() == obj || editorLeft->viewport() == obj))
    {
        evt->ignore();
        return true;
    }
    if (evt->type() == QEvent::Wheel && editorDiff->viewport() == obj)
    {
        evt->accept();
    }
    // return false to continue event propagation
    // for all events
    return false;
}

void Test9Window::closeEvent(QCloseEvent *event)
{
    //	toCustomLexer *lexer = this->findChild<toCustomLexer *>();
    //	if( lexer)
    //	{
    //		delete lexer;
    //	}
    QMainWindow::closeEvent(event);
}

void Test9Window::wheelEvent(QWheelEvent* event)
{
    verticalScrollBar->event(event);
    event->accept();
}
