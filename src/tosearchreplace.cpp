
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

#include "utils.h"

#include "tohelp.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "tosearchreplace.h"

#include "icons/close.xpm"

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <QAction>
#include <QTextEdit>


toSearchReplace::toSearchReplace(QWidget *parent)
        : QDialog(parent),
        toHelpContext(QString::fromLatin1("searchreplace.html"))
{
    setupUi(this);
    hideButton->setIcon(QPixmap(const_cast<const char**>(close_xpm)));

    QAction *action = new QAction(this);
    action->setShortcut(QKeySequence::HelpContents);
    connect(action, SIGNAL(triggered()), this, SLOT(displayHelp()));

    action = new QAction(Replace);
    action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_R);
    Replace->addAction(action);

    toEditWidget::addHandler(this);
    receivedFocus(toMainWidget()->editWidget());
    searchChanged("");

    connect(SearchNext, SIGNAL(clicked()), this, SLOT(searchNext()));
    connect(SearchPrevious, SIGNAL(clicked()), this, SLOT(searchPrevious()));
    connect(Replace, SIGNAL(clicked()), this, SLOT(replace()));
    connect(ReplaceAll, SIGNAL(clicked()), this, SLOT(replaceAll()));
    connect(hideButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(SearchText, SIGNAL(editTextChanged(const QString &)),
             this, SLOT(searchChanged(const QString &)));
}

void toSearchReplace::displayHelp(void)
{
    toHelp::displayHelp();
}

QString toSearchReplace::currentSearchText()
{
    return SearchText->currentText();
}

Search::SearchMode toSearchReplace::searchMode()
{
    if (SearchMode->currentIndex() == 0)
        return Search::SearchPlaintext;
    else
        return Search::SearchRegexp;
}

bool toSearchReplace::wholeWords()
{
    return WholeWords->isChecked();
}

bool toSearchReplace::caseSensitive()
{
    return MatchCase->isChecked();
}

void toSearchReplace::receivedFocus(toEditWidget *widget)
{
#ifdef Q_WS_MAC
    // Dialog is not writable without this magic on macosx.
    // TODO/FIXME: it would be great to reqrite S&R mechanism
    // to include it in edit widgets instead of main window...
    QWidget * w = dynamic_cast<QWidget*>(widget);
    if (w)
        setParent(w, Qt::Dialog);
//     else
//         qDebug() << widget << "not set as parent" << w;
#endif

    Target = widget;
    bool anySearch = Target && Target->searchEnabled();
    SearchPrevious->setEnabled(anySearch);
    SearchNext->setEnabled(anySearch);
    Replace->setEnabled(Target && Target->searchCanReplace(false));
    ReplaceAll->setEnabled(Target && Target->searchCanReplace(true));
}

void toSearchReplace::searchNext(void)
{
    QString t(SearchText->currentText());
    if (Target)
    {
        Target->searchNext(t);
        Replace->setEnabled(Target->searchCanReplace(false));
        ReplaceAll->setEnabled(Target->searchCanReplace(true));
    }
    if (t.length() > 0 && SearchText->findText(t) == -1)
        SearchText->addItem(t);
}

void toSearchReplace::searchPrevious(void)
{
    QString t(SearchText->currentText());
    if (Target)
    {
        Target->searchPrevious(t);
        Replace->setEnabled(Target->searchCanReplace(false));
        ReplaceAll->setEnabled(Target->searchCanReplace(true));
    }
    if (t.length() > 0 && SearchText->findText(t) == -1)
        SearchText->addItem(t);
}

void toSearchReplace::replace(void)
{
    QString t(ReplaceText->currentText());
    if (Target)
        Target->searchReplace(t);
    if (t.length() > 0 && ReplaceText->findText(t) == -1)
        ReplaceText->addItem(t);
}

void toSearchReplace::replaceAll(void)
{
    QString t(ReplaceText->currentText());
    if (Target)
        Target->searchReplaceAll(t);
    if (t.length() > 0 && ReplaceText->findText(t) == -1)
        ReplaceText->addItem(t);
}

void toSearchReplace::show()
{
    SearchText->setFocus();
    toMarkedText * w = dynamic_cast<toMarkedText*>(Target);
    if (w && w->hasSelectedText())
        SearchText->lineEdit()->setText(w->selectedText());
    SearchText->lineEdit()->selectAll();
    QDialog::show();
}

void toSearchReplace::searchChanged(const QString & text)
{
    bool ena = text > 0;
    SearchNext->setEnabled(ena);
    SearchPrevious->setEnabled(ena);
}
