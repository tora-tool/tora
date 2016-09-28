
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

#include <dtl/dtl.hpp>

#include <QStatusBar>
#include <QScrollArea>
#include <QSplitter>
#include <QToolBar>

#define declareStyle(style,color, paper, font) \
    styleNames[style] = tr(#style);            \
    super::lexer()->setColor(color, style);    \
    super::lexer()->setPaper(paper, style);    \
    super::lexer()->setFont(font, style);

Test9Window::Test9Window(const QString &sql1, const QString &sql2)
    : Ui::Test9Window()
{
    using dtl::Diff;
    typedef char   elem;
    typedef std::string sequence;

    Ui::Test9Window::setupUi(this);

    QToolBar *toolBar = QMainWindow::addToolBar("TB");
    toolBar->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    toolBar->setFocusPolicy(Qt::NoFocus);

    {
        editorLeft = new toSqlText(this);
        leftHorizontalLayout->addWidget(editorLeft);
        editorLeft->setText(sql1);
        editorLeft->enableToolTips();
        editorLeft->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        editorLeft->viewport()->installEventFilter(this);
    }

    {
        editorRight = new toSqlText(this);
        rightHorizontalLayout->addWidget(editorRight);
        editorRight->setText(sql2);
        editorRight->setMarginType(2, QsciScintilla::TextMarginRightJustified);
        editorRight->setMarginWidth(2, QString::fromLatin1("009"));
        editorRight->enableToolTips();
        editorRight->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        editorRight->viewport()->installEventFilter(this);
    }

    {
        QFont mono(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(Editor::ConfCodeFont).toString()));
        editorDiff = new toScintilla(this);
        bottomVerticalLayout->addWidget(editorDiff);
        editorDiff->viewport()->installEventFilter(this);
        editorDiff->setFont(mono);

        sequence A(sql1.toStdString());
        sequence B(sql2.toStdString());

        Diff< elem, sequence > d(A, B);
        d.compose();
        std::cout << "editDistance:" << d.getEditDistance() << endl;

        auto ses = d.getSes();
        auto seq = d.getSes().getSequence();
    }

    connect(actionLoad, SIGNAL(triggered()), this, SLOT(load()));
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    connect(verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollVerticaly(int)));

    QMainWindow::show();
}

void Test9Window::load()
{
    QString fn = Utils::toOpenFilename("*.sql", this);
    if (!fn.isEmpty())
    {
        QString data = Utils::toReadFile(fn);
        editorLeft->setText(data);
        editorRight->setText(data);
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
