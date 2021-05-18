
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
#include "tools/toresulttableview.h"
#include "core/toconfiguration.h"
#include "core/toeditorconfiguration.h"
#include "widgets/tosearchreplace.h"

#include <QPlainTextEdit>
#include <QtCore/QMimeData>

REGISTER_VIEW("Bind Values", toBindVarsDocklet);

class PlainTextEdit: public QPlainTextEdit
{
public:
    using QPlainTextEdit::QPlainTextEdit;
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void insertFromMimeData(const QMimeData *source) override;
};

toBindVarsDocklet::toBindVarsDocklet(QWidget *parent)
    : super(tr("Bind Values"), parent)
    , toEditWidget()
    , editor(new PlainTextEdit(this))
{
    toEditWidget::FlagSet.Copy = true;
    toEditWidget::FlagSet.Paste = true;
    setObjectName("BindVars Docklet");

    QFont fixed(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::ConfCodeFont).toString()));
    fixed.setPointSize(8);
    editor->setFont(fixed);
    editor->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    setFocusProxy(editor);

    QWidget *w = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout();
    l->setSpacing(0);
    l->setContentsMargins(0, 0, 0, 0);
    l->addWidget(editor);
    w->setLayout(l);

    setWidget(w);

    FlagSet.Copy = true;
    FlagSet.Paste = true;
    FlagSet.Search = true;
    FlagSet.SelectAll = true;
}


QIcon toBindVarsDocklet::icon() const
{
    return style()->standardIcon(QStyle::SP_TrashIcon);
}


QString toBindVarsDocklet::name() const
{
    return tr("Bind Values");
}

void toBindVarsDocklet::focusInEvent (QFocusEvent *e)
{
    super::focusInEvent(e);
    toEditWidget::gotFocus();
}
void toBindVarsDocklet::focusOutEvent (QFocusEvent *e)
{
    super::focusOutEvent(e);
    toEditWidget::lostFocus();
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

bool toBindVarsDocklet::handleSearching(QString const& search, QString const& replace, Search::SearchFlags flags)
{
    QTextDocument::FindFlags f;
    if (flags & Search::WholeWords)
        f |= QTextDocument::FindWholeWords;
    if (flags & Search::CaseSensitive)
        f |= QTextDocument::FindCaseSensitively;

    return editor->find(search, f);
}

bool PlainTextEdit::canInsertFromMimeData(const QMimeData *source) const
{
    if (source->hasFormat("application/x-tora"))
        return true;
    return QPlainTextEdit::canInsertFromMimeData(source);
}
void PlainTextEdit::insertFromMimeData(const QMimeData *source)
{
    if (source->hasFormat("application/x-tora"))
    {
        toResultTableView *view = NULL;
        QHash<QString, QStringList> values;
        QString spointer(source->data("application/x-tora"));
        if (toResultTableView::Registry.contains(spointer))
            view = toResultTableView::Registry[spointer];
        else
            return;
        toConnection &conn = toConnection::currentConnection(view);
        QModelIndexList indexes = view->selectionModel()->selectedIndexes();
        foreach(QModelIndex index, indexes)
        {
            QString header = view->model()->headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString();
            QString value = view->model()->data(index, Qt::EditRole).toString();
            if (value.isEmpty())
                continue;
            QStringList &list = values[header];  // will create and insert an empty QStringList if necessary
	    if (list.contains(value))
		continue;
	    list.append(value);
        }
        foreach(QString name, values.keys())
        {
            toParamGet::setDefault(conn, QString(":%1").arg(name), values[name]);
        }
    }
    QPlainTextEdit::insertFromMimeData(source);
};
