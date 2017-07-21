
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

#include "core/totextview.h"
#include "core/toeditmenu.h"
#include "docklets/tosearch.h"
#include "core/utils.h"
#include <QtGui/QFocusEvent>
#include <QtGui/QTextDocument>
#include <QTextBrowser>


toTextView::toTextView(QWidget *parent /* = 0*/, const char *name /* = 0*/)
    : super(parent)
    , toEditWidget()
{
    if (name)
        setObjectName(name);

    toEditWidget::FlagSet.Save = true;
    toEditWidget::FlagSet.Paste = true;
    toEditWidget::FlagSet.SelectAll = true;
    toEditWidget::FlagSet.SelectBlock = true;
}

void toTextView::setFilename(const QString &f)
{
    m_filename = f;
}

bool toTextView::editSave(bool)
{
    QString fn = Utils::toSaveFilename("AWR-from-to", QString::fromLatin1("*.html"), this);
    if (!fn.isEmpty())
    {
        if (fn.contains(".HTML", Qt::CaseInsensitive) || fn.contains(".HTM", Qt::CaseInsensitive))
            return Utils::toWriteFile(fn, toHtml());
        else
            return Utils::toWriteFile(fn, toPlainText());
    }
    return false;
}

QString toTextView::editText()
{
    return toPlainText();
}

void toTextView::editCopy(void)
{
    copy();
}

void toTextView::editSelectAll(void)
{
    selectAll();
}

void toTextView::focusInEvent (QFocusEvent *e)
{
    super::focusInEvent(e);
    toEditWidget::gotFocus();
}

void toTextView::focusOutEvent (QFocusEvent *e)
{
    super::focusInEvent(e);
    toEditWidget::lostFocus();
}

bool toTextView::handleSearching(QString const& search, QString const& replace, Search::SearchFlags flags)
{
    QTextDocument::FindFlags f;
    if (flags & Search::WholeWords)
        f |= QTextDocument::FindWholeWords;
    if (flags & Search::CaseSensitive)
        f |= QTextDocument::FindCaseSensitively;

    return find(search, f);
}

void toTextView::keyPressEvent(QKeyEvent *e)
{
    if (Utils::toCheckKeyEvent(e, toEditMenuSingle::Instance().searchReplaceAct->shortcut())) {
        toSearchReplaceDockletSingle::Instance().activate();
        e->accept();
        return;
    }
    super::keyPressEvent(e);
}
