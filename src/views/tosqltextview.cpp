
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

#include "views/tosqltextview.h"

#include <QtGui/QFocusEvent>
#include <QtGui/QTextDocument>

#include "editor/tosqltext.h"
#include "core/utils.h"
#include "widgets/tosearchreplace.h"

using namespace Views;

toSqlTextView::toSqlTextView(QWidget *parent /* = 0*/, const char *name /* = 0*/)
    : QWidget(parent)
    , toEditWidget()
    , m_model(NULL)
    , m_model_column(0)
{
    if (name)
        setObjectName(name);

    toEditWidget::FlagSet.Save = true;
    toEditWidget::FlagSet.Paste = false;
    toEditWidget::FlagSet.SelectAll = true;
    toEditWidget::FlagSet.SelectBlock = false;

    m_view = new toSqlText(this);
    m_view->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_search = new toSearchReplace(this);
    m_search->SearchMode->hide();

    QVBoxLayout *l = new QVBoxLayout();
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(m_view);
    l->addWidget(m_search);
    setLayout(l);

    connect(m_search, SIGNAL(searchNext(Search::SearchFlags)), this, SLOT(handleSearching(Search::SearchFlags)));
    connect(m_search, SIGNAL(windowClosed()), this, SLOT(setEditorFocus()));
}

void toSqlTextView::setReadOnly(bool ro)
{
    m_view->setReadOnly(ro);
}

void toSqlTextView::setText(const QString &t)
{
    m_view->setText(t);
}

void toSqlTextView::setFilename(const QString &f)
{
    m_filename = f;
}

bool toSqlTextView::editSave(bool)
{
    QString fn = Utils::toSaveFilename(m_filename, QString::fromLatin1("*.sql"), this);
    if (!fn.isEmpty())
    {
        return Utils::toWriteFile(fn, m_view->text());
    }
    return false;
}

QString toSqlTextView::editText()
{
    return m_view->text();
}

void toSqlTextView::editCopy(void)
{
    m_view->copy();
}

void toSqlTextView::editSelectAll(void)
{
    m_view->selectAll();
}

void toSqlTextView::focusInEvent (QFocusEvent *e)
{
    QWidget::focusInEvent(e);
}

void toSqlTextView::setModel(QAbstractItemModel *model)
{
    m_model = model;
    modelReset();
    connect(model, SIGNAL(modelReset()), this, SLOT(modelReset()));
    connect(model, SIGNAL(rowsInserted(const QModelIndex &, int , int)), this, SLOT(rowsInserted(const QModelIndex &, int, int)));
    connect(model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
    setReadOnly(true);
    m_search->setReadOnly(true);
}

void toSqlTextView::modelReset()
{
    m_view->clear();
    for(int row = 0; row < m_model->rowCount(); row++)
    {
        QModelIndex index = m_model->index(row, m_model_column);
        m_view->append(m_model->data(index).toString());
        // new line
        switch (m_view->eolMode())
        {
//            case QsciScintilla::EolWindows:
//                m_view->append("\n\r");
//                break;
//            case QsciScintilla::EolMac:
//                m_view->append("\n");
//                break;
            default:
                // Unix is default one
                m_view->append("\r");
                break;
        }
    }
}

void toSqlTextView::rowsInserted(const QModelIndex &parent, int first, int last)
{

}

void toSqlTextView::rowsRemoved(const QModelIndex &parent, int first, int last)
{

}

bool toSqlTextView::searchNext()
{
    if (!m_search->isVisible())
    {
        m_search->show();
        m_search->setReadOnly(true);
    }
    return true;
}

void toSqlTextView::handleSearching(Search::SearchFlags flags)
{
    QTextDocument::FindFlags f;
    if (flags & Search::WholeWords)
        f |= QTextDocument::FindWholeWords;
    if (flags & Search::CaseSensitive)
        f |= QTextDocument::FindCaseSensitively;

    //bool ret = m_view->find(m_search->searchText(), f);
}

void toSqlTextView::setEditorFocus()
{
    m_view->setFocus(Qt::OtherFocusReason);
}
