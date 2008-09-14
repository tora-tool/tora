
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <QAction>
#include <QTextEdit>


toSearchReplace::toSearchReplace(QWidget *parent)
        : QDialog(parent), toHelpContext(QString::fromLatin1("searchreplace.html"))
{
    setupUi(this);

    QAction *action = new QAction(this);
    action->setShortcut(QKeySequence::HelpContents);
    connect(action, SIGNAL(triggered()), this, SLOT(displayHelp()));

    action = new QAction(Replace);
    action->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_R);
    Replace->addAction(action);

    toEditWidget::addHandler(this);
    receivedFocus(toMainWidget()->editWidget());
}

void toSearchReplace::displayHelp(void)
{
    toHelp::displayHelp();
}

void toSearchReplace::receivedFocus(toEditWidget *widget)
{
    Target = widget;
    bool anySearch = Target && Target->searchEnabled();
    Search->setEnabled(anySearch);
    SearchNext->setEnabled(anySearch);
    Replace->setEnabled(Target && Target->searchCanReplace(false));
    ReplaceAll->setEnabled(Target && Target->searchCanReplace(true));
}

void toSearchReplace::search(void)
{
    if (Target)
    {
        Target->searchTop();
        searchNext();
    }
}

void toSearchReplace::searchNext(void)
{
    if (Target)
    {
        if (Target->searchNext(this))
        {
            Replace->setEnabled(Target->searchCanReplace(false));
            ReplaceAll->setEnabled(Target->searchCanReplace(true));
        }
        else
        {
            toStatusMessage(tr("No more matches found"), false, false);
            Replace->setEnabled(false);
            ReplaceAll->setEnabled(false);
        }
    }
}

void toSearchReplace::replace(void)
{
    if (Target && Target->searchCanReplace(false))
    {
        Target->searchReplace(ReplaceText->toPlainText());
        searchNext();
    }
}

void toSearchReplace::replaceAll(void)
{
    if (Target && Target->searchCanReplace(true))
    {
        while (Target->searchCanReplace(false))
        {
            Target->searchReplace(ReplaceText->toPlainText());
            searchNext();
        }
    }
}

void toSearchReplace::show()
{
    SearchText->setFocus();
    SearchText->selectAll();
    QDialog::show();
}


bool toSearchReplace::findString(const QString &text, int &pos, int &endPos)
{
    bool ok;
    int found;
    int foundLen;
    QString searchText = SearchText->toPlainText();
    do
    {
        ok = true;
        if (Exact->isChecked())
        {
            found = text.indexOf(searchText, pos, IgnoreCase->isChecked() ?
                                 Qt::CaseInsensitive :
                                 Qt::CaseSensitive);
            foundLen = searchText.length();
        }
        else
        {
            QRegExp re(searchText,
                       IgnoreCase->isChecked() ?
                       Qt::CaseInsensitive :
                       Qt::CaseSensitive);
            found = re.indexIn(text, pos);
            foundLen = re.matchedLength();
        }
        if (found == -1)
        {
            return false;
        }
        if (WholeWord->isChecked())
        {
            if (found != 0 && !text[found].isSpace())
                ok = false;
            if (found + foundLen != int(text.length()) && !text[found + foundLen].isSpace())
                ok = false;
            pos = found + 1;
        }
    }
    while (!ok);

    pos = found;
    endPos = found + foundLen;
    return true;
}

void toSearchReplace::searchChanged(void)
{
    bool ena = SearchText->toPlainText().length() > 0;
    SearchNext->setEnabled(ena);
    Search->setEnabled(ena);
}

bool toSearchReplace::searchNextAvailable(void)
{
    return SearchNext->isEnabled();
}
