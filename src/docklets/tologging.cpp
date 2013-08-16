
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

#include "docklets/tologging.h"
#include "core/tologger.h"
#include "core/utils.h"
#include "core/tomain.h"
#include "editor/tosearchreplace.h"

#include <QtGui/QPlainTextEdit>

REGISTER_VIEW("Logging", toLoggingDocklet);

toLoggingDocklet::toLoggingDocklet(QWidget *parent,
                                   Qt::WindowFlags flags)
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

    m_search = new toSearchReplace(this);
    m_search->hide();

    setFocusProxy(&log);

    QWidget *w = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout();
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(&log);
    l->addWidget(m_search);
    w->setLayout(l);

    setWidget(w);


//    FlagSet.Save = true;
//    FlagSet.Print = true;
    FlagSet.Copy = true;
    FlagSet.Search = true;
    FlagSet.SelectAll = true;

    toEditWidget::receivedFocus();

    connect(m_search, SIGNAL(searchNext(Search::SearchFlags)),
            this, SLOT(handleSearching(Search::SearchFlags)));
    connect(m_search, SIGNAL(windowClosed()),
            this, SLOT(setEditorFocus()));
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
	toEditWidget::receivedFocus();
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
    if (!m_search->isVisible())
    {
        m_search->show();
        m_search->setReadOnly(log.isReadOnly());
    }
    return true;
}

void toLoggingDocklet::searchReplace()
{
    m_search->setVisible(!m_search->isVisible());
    m_search->setReadOnly(log.isReadOnly());
}

void toLoggingDocklet::handleSearching(Search::SearchFlags flags)
{
    QTextDocument::FindFlags f;
    if (flags & Search::WholeWords)
        f |= QTextDocument::FindWholeWords;
    if (flags & Search::CaseSensitive)
        f |= QTextDocument::FindCaseSensitively;

    bool ret = log.find(m_search->searchText(), f);
}

void toLoggingDocklet::setEditorFocus()
{
    log.setFocus(Qt::OtherFocusReason);
}
