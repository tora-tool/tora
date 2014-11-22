
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

#include "tests/test4window.h"
#include "editor/tosqltext.h"

#include <QStatusBar>
#include <QScrollArea>
#include <QSplitter>
#include <QToolBar>

Test4Window::Test4Window(const QString &sql)
	: Ui::Test4Window()
{
	Ui::Test4Window::setupUi(this);

	QToolBar *toolBar = QMainWindow::addToolBar("TB");
	toolBar->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
	toolBar->setFocusPolicy(Qt::NoFocus);
	
	pushButton = new QPushButton("Oracle", toolBar);
	pushButton->setFlat(true);
	toolBar->addWidget(pushButton);
	connect(pushButton, SIGNAL(released()), this, SLOT(setLexer()));
	
	QAction *oracle = new QAction("Oracle", this);
	QAction *mysql = new QAction("MySQL", this);
	oracle->setCheckable(true);
	oracle->setChecked(true);
	mysql->setCheckable(true);
	mysql->setChecked(false);
	QActionGroup* group = new QActionGroup(this);
	oracle->setActionGroup(group);
	mysql->setActionGroup(group);
	menuGrammar->addAction(oracle);
	menuGrammar->addAction(mysql);
	
	editorLeft = new toSqlText(this);
	leftVerticalLayout->addWidget(editorLeft);
	editorLeft->setText(sql);

	editorRight = new toSqlText(this);
	editorRight->setHighlighter(toSqlText::Oracle);
	rightVerticalLayout->addWidget(editorRight);
	editorRight->setText(sql);
	editorRight->setMarginType(2, QsciScintilla::TextMarginRightJustified);
	editorRight->setMarginWidth(2, QString::fromAscii("009"));

	connect(oracle, SIGNAL(triggered()), this, SLOT(setOracle()));
	connect(mysql, SIGNAL(triggered()), this, SLOT(setMySQL()));

	connect(actionLoad, SIGNAL(triggered()), this, SLOT(load()));
	connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	QMainWindow::show();
}

void Test4Window::load()
{
	QString fn = Utils::toOpenFilename("*.sql", this);
	if (!fn.isEmpty())
	  {
	    QString data = Utils::toReadFile(fn);
	    editorLeft->setText(data);
	    editorRight->setText(data);
	  }
}

void Test4Window::setLexer()
{
	QString txt = pushButton->text();
	if(txt == "Oracle")
	{
		editorRight->setHighlighter(toSqlText::Oracle);
		pushButton->setText("Mysql");
	} else {
		editorRight->setHighlighter(toSqlText::Mysql);
		pushButton->setText("Oracle");
	}
}

void Test4Window::closeEvent(QCloseEvent *event)
{
	//	toCustomLexer *lexer = this->findChild<toCustomLexer *>();
	//	if( lexer)
	//	{
	//		delete lexer;
	//	}
	QMainWindow::closeEvent(event);
}
