/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "tohelp.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "tosearchreplace.h"

#include <q3accel.h>
#include <qcheckbox.h>
#include <q3multilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>


toSearchReplace::toSearchReplace(QWidget *parent)
        : QDialog(parent/*, "SearchReplace"*/), toHelpContext(QString::fromLatin1("searchreplace.html"))
{
    setupUi(this);

    Q3Accel *a = new Q3Accel(this);
    a->connectItem(a->insertItem(toKeySequence(tr("F1", "Search|Help"))),
                   this,
                   SLOT(displayHelp()));
    Replace->setAccel( QKeySequence(tr("Ctrl+Shift+R", "Edit|Replace")) );
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
        Target->searchReplace(ReplaceText->text());
        searchNext();
    }
}

void toSearchReplace::replaceAll(void)
{
    if (Target && Target->searchCanReplace(true))
    {
        while (Target->searchCanReplace(false))
        {
            Target->searchReplace(ReplaceText->text());
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
    QString searchText = SearchText->text();
    do
    {
        ok = true;
        if (Exact->isOn())
        {
            found = text.find(searchText, pos, !IgnoreCase->isOn());
            foundLen = searchText.length();
        }
        else
        {
            QRegExp re(searchText, !IgnoreCase->isOn(), false);
            found = re.indexIn(text, pos);
            foundLen = re.matchedLength();
        }
        if (found == -1)
        {
            return false;
        }
        if (WholeWord->isOn())
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
    bool ena = SearchText->text().length() > 0;
    SearchNext->setEnabled(ena);
    Search->setEnabled(ena);
}

bool toSearchReplace::searchNextAvailable(void)
{
    return SearchNext->isEnabled();
}
