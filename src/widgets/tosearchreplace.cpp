
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

#include "widgets/tosearchreplace.h"
#include "widgets/tohelp.h"
#include "core/utils.h"
#include "core/tomainwindow.h"
#include "core/toeditmenu.h"
#include "core/toeditwidget.h"

#include "icons/close.xpm"

#include <QAction>
#include <QTextEdit>
#include <QLineEdit>
#include <QKeyEvent>

toSearchReplace::toSearchReplace(QWidget *parent)
    : QWidget(parent)
    , toHelpContext(QString::fromLatin1("searchreplace.html"))
    , m_editWidget(NULL)
{
    setupUi(this);

    QAction *action = new QAction(this);
    action->setShortcut(QKeySequence::HelpContents);
    connect(action, SIGNAL(triggered()), this, SLOT(displayHelp()));

    action = new QAction(Replace);
    action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_R);
    Replace->addAction(action);

    connect(SearchNext, SIGNAL(clicked()), this, SLOT(act_searchNext()));
    connect(SearchPrevious, SIGNAL(clicked()), this, SLOT(act_searchPrevious()));
    connect(Replace, SIGNAL(clicked()), this, SLOT(act_replace()));
    connect(ReplaceAll, SIGNAL(clicked()), this, SLOT(act_replaceAll()));
    connect(SearchText, SIGNAL(editTextChanged(const QString &)), this, SLOT(act_searchChanged(const QString &)));

    connect(SearchText->lineEdit(), SIGNAL(returnPressed()), this, SLOT(act_searchNext()));
    connect(ReplacementText->lineEdit(), SIGNAL(returnPressed()), this, SLOT(act_replace()));

    SearchText->installEventFilter(this);
    ReplacementText->installEventFilter(this);
}

void toSearchReplace::displayHelp(void)
{
    toHelp::displayHelp();
}

QString toSearchReplace::searchText()
{
    return SearchText->currentText();
}

QString toSearchReplace::replaceText()
{
    return ReplacementText->currentText();
}

void toSearchReplace::setReadOnly(bool ro)
{
    ReplacementText->setEnabled(!ro);
    Replace->setEnabled(!ro);
    ReplaceAll->setEnabled(!ro);
}

void toSearchReplace::activate()
{
    SearchText->setFocus();
    SearchText->lineEdit()->selectAll();
    act_searchChanged(SearchText->currentText());
}

void toSearchReplace::registerEdit(toEditWidget *w)
{
    m_editWidget = w;
}

void toSearchReplace::unregisterEdit(toEditWidget *w)
{
    if (m_editWidget == w)
        m_editWidget = NULL;
}

Search::SearchFlags toSearchReplace::sharedFlags()
{
    Search::SearchFlags f;
    f |= SearchMode->currentIndex() == 0 ? Search::Plaintext : Search::Regexp;

    if (WholeWords->isChecked())
        f |= Search::WholeWords;

    if (MatchCase->isChecked())
        f |= Search::CaseSensitive;

    return f;
}

void toSearchReplace::act_searchNext(void)
{
    QString t(SearchText->currentText());
    if (t.length() > 0 && SearchText->findText(t) == -1)
        SearchText->addItem(t);

    if (t.length() && m_editWidget)
    {
        Search::SearchFlags f = Search::Search | Search::Forward | sharedFlags();
        m_editWidget->handleSearching(searchText(), QString(), f);
    }
}

void toSearchReplace::act_searchPrevious(void)
{
    QString t(SearchText->currentText());
    if (t.length() > 0 && SearchText->findText(t) == -1)
        SearchText->addItem(t);

    if (t.length())
    {
        Search::SearchFlags f = Search::Search | Search::Backward | sharedFlags();
        m_editWidget->handleSearching(searchText(), QString(), f);
    }
}

void toSearchReplace::act_replace(void)
{
    QString t(ReplacementText->currentText());
    if (t.length() > 0 && ReplacementText->findText(t) == -1)
        ReplacementText->addItem(t);

    if (SearchText->currentText().length())
    {
        Search::SearchFlags f = Search::Replace | Search::Forward | sharedFlags();
        m_editWidget->handleSearching(searchText(), replaceText(), f);
    }
}

void toSearchReplace::act_replaceAll(void)
{
    QString t(ReplacementText->currentText());
    if (t.length() > 0 && ReplacementText->findText(t) == -1)
        ReplacementText->addItem(t);

    if (SearchText->currentText().length())
    {
        Search::SearchFlags f = Search::ReplaceAll | Search::Forward | sharedFlags();
        m_editWidget->handleSearching(searchText(), replaceText(), f);
    }
}

void toSearchReplace::showEvent(QShowEvent * e)
{
    QWidget::showEvent(e);
}

void toSearchReplace::closeEvent(QCloseEvent *e)
{
    #pragma message WARN("TODO: call toScintilla::findStop here - extend toEditWidget API")
    QWidget::closeEvent(e);
}

bool toSearchReplace::eventFilter(QObject *target, QEvent *event)
{
    if ((target == SearchText || target == ReplacementText) && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape)
        {
            parentWidget()->close(); // Close parent docklet window
            if (QWidget *w = dynamic_cast<QWidget*>(m_editWidget))
            {
                w->setFocus();
            }
            return true;
        }
    }
    return QWidget::eventFilter(target, event);
}

void toSearchReplace::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        event->accept();
        parentWidget()->close(); // Close parent docklet window
        return;
    }
    QWidget::keyPressEvent(event);
}

void toSearchReplace::act_searchChanged(const QString & text)
{
    bool ena = !text.isEmpty();
    SearchNext->setEnabled(ena);
    SearchPrevious->setEnabled(ena);
    Replace->setEnabled(ena);
    ReplaceAll->setEnabled(ena);
}
