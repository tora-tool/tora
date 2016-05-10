
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

#include "docklets/tobindvars.h"
#include "tools/toparamget.h"
#include "core/toconfiguration.h"
#include "core/toeditorconfiguration.h"
#include <QPlainTextEdit>
#include "widgets/tosearchreplace.h"

REGISTER_VIEW("BindVars", toBindVarsDocklet);

toBindVarsDocklet::toBindVarsDocklet(QWidget *parent,
                                   toWFlags flags)
    : toDocklet(tr("BindVars"), parent, flags)
    , toEditWidget()
    , editor(new QPlainTextEdit(this))
{
    toEditWidget::FlagSet.Copy = true;
    toEditWidget::FlagSet.Paste = true;
    setObjectName("BindVars Docklet");

    QFont fixed(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::ConfCodeFont).toString()));
    fixed.setPointSize(8);
    editor->setFont(fixed);
    editor->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_search = new toSearchReplace(this);
    m_search->hide();

    setFocusProxy(editor);

    QWidget *w = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout();
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(editor);
    l->addWidget(m_search);
    w->setLayout(l);

    setWidget(w);

    FlagSet.Copy = true;
    FlagSet.Paste = true;
    FlagSet.Search = true;
    FlagSet.SelectAll = true;

    connect(m_search, SIGNAL(searchNext(Search::SearchFlags)),
            this, SLOT(handleSearching(Search::SearchFlags)));
    connect(m_search, SIGNAL(windowClosed()),
            this, SLOT(setEditorFocus()));
}


QIcon toBindVarsDocklet::icon() const
{
    return style()->standardIcon(QStyle::SP_ComputerIcon);
}


QString toBindVarsDocklet::name() const
{
    return tr("BindVars");
}

void toBindVarsDocklet::focusInEvent (QFocusEvent *e)
{
    toDocklet::focusInEvent(e);
}
void toBindVarsDocklet::focusOutEvent (QFocusEvent *e)
{
    //toEditWidget::lostFocus();
    toDocklet::focusOutEvent(e);
}

void toBindVarsDocklet::editCopy()
{
    editor->copy();
}

void toBindVarsDocklet::editPaste()
{
    editor->paste();
}

void toBindVarsDocklet::editSelectAll()
{
    editor->selectAll();
}

bool toBindVarsDocklet::searchNext()
{
    if (!m_search->isVisible())
    {
        m_search->show();
        m_search->setReadOnly(editor->isReadOnly());
    }
    return true;
}

void toBindVarsDocklet::searchReplace()
{
    m_search->setVisible(!m_search->isVisible());
    m_search->setReadOnly(editor->isReadOnly());
}

void toBindVarsDocklet::handleSearching(Search::SearchFlags flags)
{
    QTextDocument::FindFlags f;
    if (flags & Search::WholeWords)
        f |= QTextDocument::FindWholeWords;
    if (flags & Search::CaseSensitive)
        f |= QTextDocument::FindCaseSensitively;

    /*bool ret =*/ editor->find(m_search->searchText(), f);
}

void toBindVarsDocklet::setEditorFocus()
{
    editor->setFocus(Qt::OtherFocusReason);
}
