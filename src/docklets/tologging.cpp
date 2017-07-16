
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

#include "docklets/tologging.h"
#include "ts_log/toostream.h"
#include "core/tologger.h"
#include "core/utils.h"
#include <QPlainTextEdit>
#include "widgets/tosearchreplace.h"

REGISTER_VIEW("Logging", toLoggingDocklet);

toLoggingDocklet::toLoggingDocklet(QWidget *parent,
                                   toWFlags flags)
    : toDocklet(tr("Logging"), parent, flags)
    , toEditWidget()
    , log(toLoggingWidgetSingle::Instance())
{
    toEditWidget::FlagSet.Copy = true;
    setObjectName("Logging Docklet");

//    if( (log = toMainWidget()->getLoggingWidget()) == NULL)
//        log = new QPlainTextEdit("NO LOGGING...", this);
//    else
//        log->setParent(this);
    log.setParent(this);
    log.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    setFocusProxy(&log);

    QWidget *w = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout();
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(&log);
    w->setLayout(l);

    setWidget(w);

//    FlagSet.Save = true;
//    FlagSet.Print = true;
    FlagSet.Copy = true;
    FlagSet.Search = true;
    FlagSet.SelectAll = true;

}


QIcon toLoggingDocklet::icon() const
{
    return style()->standardIcon(QStyle::SP_ComputerIcon);
}


QString toLoggingDocklet::name() const
{
    return tr("Logging");
}

void toLoggingDocklet::focusInEvent (QFocusEvent *e)
{
    toDocklet::focusInEvent(e);
}
void toLoggingDocklet::focusOutEvent (QFocusEvent *e)
{
    //toEditWidget::lostFocus();
    toDocklet::focusOutEvent(e);
}

void toLoggingDocklet::editCopy()
{
    log.copy();
}

void toLoggingDocklet::editSelectAll()
{
    log.selectAll();
}

bool toLoggingDocklet::searchNext()
{
    throw QString("TODO toLoggingDocklet::searchNext");
#if 0
    if (!m_search->isVisible())
    {
        m_search->show();
        m_search->setReadOnly(log.isReadOnly());
    }
#endif
    return true;
}

void toLoggingDocklet::searchReplace()
{
    throw QString("TODO toLoggingDocklet::searchReplace");
#if 0
    m_search->setVisible(!m_search->isVisible());
    m_search->setReadOnly(log.isReadOnly());
#endif
}

void toLoggingDocklet::handleSearching(Search::SearchFlags flags)
{
    QTextDocument::FindFlags f;
    if (flags & Search::WholeWords)
        f |= QTextDocument::FindWholeWords;
    if (flags & Search::CaseSensitive)
        f |= QTextDocument::FindCaseSensitively;

    throw QString("TODO toTextView::handleSearching");
#if 0
    /*bool ret =*/ log.find(m_search->searchText(), f);
#endif
}

void toLoggingDocklet::setEditorFocus()
{
    log.setFocus(Qt::OtherFocusReason);
}
