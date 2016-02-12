
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

#include "editor/tomemoeditor.h"
#include "core/utils.h"
#include "widgets/toresultview.h"
#include "editor/toscintilla.h"

#include <QtGui/QKeyEvent>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QtCore/QSettings>
#include <QCheckBox>

#include "icons/commit.xpm"
#include "icons/copy.xpm"
#include "icons/wordwrap.xpm"
#include "icons/cut.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/forward.xpm"
#include "icons/next.xpm"
#include "icons/paste.xpm"
#include "icons/previous.xpm"
#include "icons/rewind.xpm"

void toMemoEditor::openFile(void)
{
    Editor->editOpen();
}

void toMemoEditor::saveFile(void)
{
    Editor->editSave(true);
}

toMemoEditor::toMemoEditor(QWidget *parent,
                           const QString &str,
                           int row,
                           int col,
                           bool sql,
                           bool modal,
                           bool navigation)
    : QDialog(parent),
      Row(row),
      Col(col)
{
    setModal(modal);

    setWindowTitle("Memo Editor");

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    Toolbar = Utils::toAllocBar(this, tr("Memo Editor"));
    vbox->addWidget(Toolbar);

    if (sql)
        Editor = new toMemoEditor::toMemoSQL(this);
    else
        Editor = new toMemoEditor::toMemoText(this);
    vbox->addWidget(Editor);
    Editor->sciEditor()->setReadOnly(Row < 0 || Col < 0 || listView());
    Editor->setFocus();

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    vbox->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    if (Row >= 0 && Col >= 0 && !listView())
    {
        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(commit_xpm))),
                           tr("Save changes"),
                           this,
                           SLOT(store(void)));

        Toolbar->addSeparator();

        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(fileopen_xpm))),
                           tr("Open file"),
                           this,
                           SLOT(openFile()));

        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(filesave_xpm))),
                           tr("Save file"),
                           this,
                           SLOT(saveFile()));

        Toolbar->addSeparator();

        QAction *act =
            Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(cut_xpm))),
                               tr("Cut to clipboard"),
                               Editor,
                               SLOT(cut()));

        connect(Editor,
                SIGNAL(copyAvailable(bool)),
                act,
                SLOT(setEnabled(bool)));
        act->setEnabled(false);
    }
    else
    {
        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(filesave_xpm))),
                           tr("Save changes"),
                           this,
                           SLOT(saveFile(void)));
        Toolbar->addSeparator();
    }

    QAction *act =
        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(copy_xpm))),
                           tr("Copy to clipboard"),
                           Editor,
                           SLOT(copy()));
    connect(Editor,
            SIGNAL(copyAvailable(bool)),
            act,
            SLOT(setEnabled(bool)));
    act->setEnabled(false);

    QAction * WordWrapAct = new QAction(QIcon(QPixmap(const_cast<const char**>(wordwrap_xpm))),
                                        tr("Word Wrap"), Toolbar);
    WordWrapAct->setCheckable(true);
    connect(WordWrapAct, SIGNAL(toggled(bool)),
            Editor, SLOT(setWordWrap(bool)));
    Toolbar->addAction(WordWrapAct);

    QAction * XMLWrapAct = new QAction(QIcon(":/icons/xmlwrap.png"), tr("XML Format"), Toolbar);
    XMLWrapAct->setCheckable(true);
    connect(XMLWrapAct, SIGNAL(toggled(bool)),
            Editor, SLOT(setXMLWrap(bool)));
    Toolbar->addAction(XMLWrapAct);


    if (Row >= 0 && Col >= 0 && !listView())
    {
        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(paste_xpm))),
                           tr("Paste from clipboard"),
                           Editor,
                           SLOT(paste()));
    }

    toListView *lst = listView();
    if (lst || navigation)
    {
        Toolbar->addSeparator();

        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(rewind_xpm))),
                           tr("First column"),
                           this,
                           SLOT(firstColumn()));

        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(previous_xpm))),
                           tr("Previous column"),
                           this,
                           SLOT(previousColumn()));

        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(next_xpm))),
                           tr("Next column"),
                           this,
                           SLOT(nextColumn()));

        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(forward_xpm))),
                           tr("Last column"),
                           this,
                           SLOT(lastColumn()));
    }

    Toolbar->addSeparator();

    Null = new QCheckBox(tr("NULL"), Toolbar);
    Toolbar->addWidget(Null);
    connect(Null, SIGNAL(toggled(bool)), this, SLOT(null(bool)));
    Null->setEnabled(!Editor->sciEditor()->isReadOnly());
    Null->setFocusPolicy(Qt::StrongFocus);

    setText(str);

    Label = new QLabel(Toolbar);
    Label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    Label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum));

    if (lst)
    {
        connect(parent,
                SIGNAL(currentChanged(toTreeWidgetItem *)),
                this,
                SLOT(changeCurrent(toTreeWidgetItem *)));
        Label->setText(QString::fromLatin1("<B>") +
                       lst->headerItem()->text(Col) +
                       QString::fromLatin1("</B>"));
    }
    Toolbar->addWidget(Label);

    readSettings();

    connect(this,
            SIGNAL(finished(int)),
            this,
            SLOT(writeSettings()));

    if (!modal)
        show();
}

void toMemoEditor::readSettings()
{
    QSettings settings;
    settings.beginGroup("memoEditor");
    resize(settings.value("size", QSize(400, 300)).toSize());
}

void toMemoEditor::writeSettings() const
{
    QSettings settings;
    settings.beginGroup("memoEditor");
    settings.setValue("size", size());
}

void toMemoEditor::setText(const QString &str)
{
    Editor->sciEditor()->setText(str);
    Null->setChecked(str.isNull());
    Editor->sciEditor()->setModified(false);
}

void toMemoEditor::null(bool nul)
{
    Editor->sciEditor()->setModified(true);
    Editor->setDisabled(nul);
}

QString toMemoEditor::text(void)
{
    return Editor->sciEditor()->text();
}

void toMemoEditor::store(void)
{
    if (Editor->sciEditor()->isReadOnly())
        return ;
    if (Editor->sciEditor()->isModified())
    {
        if (!Editor->isEnabled())
            emit changeData(Row, Col, QString::null);
        else
            emit changeData(Row, Col, Editor->sciEditor()->text());
    }
    accept();
}

void toMemoEditor::changePosition(int row, int cols)
{
    if (Editor->sciEditor()->isModified())
    {
        if (!Editor->isEnabled())
            emit changeData(Row, Col, QString::null);
        else
            emit changeData(Row, Col, Editor->sciEditor()->text());
        Editor->sciEditor()->setModified(false);
    }

    toListView *lst = listView();
    if (lst)
        Label->setText(QString::fromLatin1("<B>") +
                       lst->headerItem()->text(Col) +
                       QString::fromLatin1("</B>"));

    Row = row;
    Col = cols;
}

toListView *toMemoEditor::listView(void)
{
    return dynamic_cast<toListView *>(parentWidget());
}

void toMemoEditor::firstColumn(void)
{
    toListView *lst = listView();
    if (lst)
    {
        toTreeWidgetItem *cur = lst->currentItem();
        if (!cur)
            return ;

        if (Col == 0)
        {
            toTreeWidgetItem *item = lst->firstChild();
            toTreeWidgetItem *next = item;
            while (next && next != cur)
            {
                item = next;
                if (item->firstChild())
                    next = item->firstChild();
                else if (item->nextSibling())
                    next = item->nextSibling();
                else
                {
                    next = item;
                    do
                    {
                        next = next->parent();
                    }
                    while (next && !next->nextSibling());
                    if (next)
                        next = next->nextSibling();
                }
            }
            lst->setCurrentItem(item);
        }
        else
        {
            Col = 0;
            Label->setText("<B>" + lst->headerItem()->text(Col) + "</B>");

            toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(cur);
            toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(cur);
            if (resItem)
                setText(resItem->allText(Col));
            else if (chkItem)
                setText(chkItem->allText(Col));
            else
                setText(cur->text(Col));
        }
    }
}

void toMemoEditor::previousColumn(void)
{
    toListView *lst = listView();
    if (lst)
    {
        toTreeWidgetItem *cur = lst->currentItem();
        if (!cur)
            return ;

        if (Col == 0)
        {
            Col = lst->columns() - 1;
            toTreeWidgetItem *item = lst->firstChild();
            toTreeWidgetItem *next = item;
            while (next && next != cur)
            {
                item = next;
                if (item->firstChild())
                    next = item->firstChild();
                else if (item->nextSibling())
                    next = item->nextSibling();
                else
                {
                    next = item;
                    do
                    {
                        next = next->parent();
                    }
                    while (next && !next->nextSibling());
                    if (next)
                        next = next->nextSibling();
                }
            }
            if (item != cur)
            {
                lst->setCurrentItem(item);
                cur = NULL;
            }
        }
        else
            Col--;

        if (cur)
        {
            toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(cur);
            toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(cur);
            if (resItem)
                setText(resItem->allText(Col));
            else if (chkItem)
                setText(chkItem->allText(Col));
            else
                setText(cur->text(Col));
        }
        Label->setText(QString::fromLatin1("<B>") +
                       lst->headerItem()->text(Col) +
                       QString::fromLatin1("</B>"));
    }
}

void toMemoEditor::nextColumn(void)
{
    toListView *lst = listView();
    if (lst)
    {
        toTreeWidgetItem *cur = lst->currentItem();
        if (!cur)
            return ;

        if (Col == lst->columns() - 1)
        {
            Col = 0;
            toTreeWidgetItem *next = cur;
            if (cur->firstChild())
                next = cur->firstChild();
            else if (cur->nextSibling())
                next = cur->nextSibling();
            else
            {
                next = cur;
                do
                {
                    next = next->parent();
                }
                while (next && !next->nextSibling());
                if (next)
                    next = next->nextSibling();
            }
            if (next)
            {
                lst->setCurrentItem(next);
                cur = NULL;
            }
        }
        else
            Col++;
        if (cur)
        {
            toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(cur);
            toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(cur);
            if (resItem)
                setText(resItem->allText(Col));
            else if (chkItem)
                setText(chkItem->allText(Col));
            else
                setText(cur->text(Col));
        }
        Label->setText(QString::fromLatin1("<B>") +
                       lst->headerItem()->text(Col) +
                       QString::fromLatin1("</B>"));
    }
}

void toMemoEditor::lastColumn(void)
{
    toListView *lst = listView();
    if (lst)
    {
        toTreeWidgetItem *cur = lst->currentItem();
        if (!cur)
            return ;

        if (Col == lst->columns() - 1)
        {
            toTreeWidgetItem *next = cur;
            if (cur->firstChild())
                next = cur->firstChild();
            else if (cur->nextSibling())
                next = cur->nextSibling();
            else
            {
                next = cur;
                do
                {
                    next = next->parent();
                }
                while (next && !next->nextSibling());
                if (next)
                    next = next->nextSibling();
            }
            if (next)
                lst->setCurrentItem(next);
        }
        else
        {
            Col = lst->columns() - 1;
            Label->setText(QString::fromLatin1("<B>") +
                           lst->headerItem()->text(Col) +
                           QString::fromLatin1("</B>"));
            toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(cur);
            toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(cur);
            if (resItem)
                setText(resItem->allText(Col));
            else if (chkItem)
                setText(chkItem->allText(Col));
            else
                setText(cur->text(Col));
        }
    }
}

void toMemoEditor::changeCurrent(toTreeWidgetItem *)
{
    toListView *lst = listView();
    if (lst)
    {
        toTreeWidgetItem *cur = lst->currentItem();
        if (!cur)
            return ;

        toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(cur);
        toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(cur);
        if (resItem)
            setText(resItem->allText(Col));
        else if (chkItem)
            setText(chkItem->allText(Col));
        else
            setText(cur->text(Col));
    }
}
