
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

#include "editor/tomodeleditor.h"
#include "core/toglobalevent.h"
#include "core/tomainwindow.h"
#include "core/toqvalue.h"
#include "editor/toscintilla.h"

#include <QtGui/QKeyEvent>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QtCore/QSettings>
#include <QtCore/QAbstractItemModel>
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

void toModelEditor::openFile()
{
    Editor->editOpen();
}

void toModelEditor::saveFile()
{
    QVariant const &data = Model->data(Current, Qt::UserRole);
    if (data.type() == QVariant::UserType)
    {
        QString fn;
        fn = Utils::toSaveFilename(Editor->filename(), QString::null, this);
        QFile file(fn);
        if (!file.open(QIODevice::WriteOnly))
        {
            TOMessageBox::warning(
                toMainWindow::lookup(),
                QT_TRANSLATE_NOOP("toWriteFile", "File error"),
                QT_TRANSLATE_NOOP("toWriteFile", QString("Couldn't open %1 for writing")
                                  .arg(fn).toLatin1().constData())); // TODO test this in MSVC
            return;
        }
        offset = 0;
        bool retval = Utils::toWriteLargeFile(file, *this, /*progressbar*/false, /*parent*/this);
        if (retval)
            toGlobalEventSingle::Instance().addRecentFile(fn);
    }
    else
    {
        Editor->editSave(true);
    }
};

QByteArray toModelEditor::nextData() const
{
    QVariant const &data = Model->data(Current, Qt::UserRole);
    if (data.type() == QVariant::UserType)
    {
        toQValue::complexType *i = data.value<toQValue::complexType*>();
        QByteArray a = i->read(offset);
        offset += a.size();
        return a;
    }
    else
    {
        return QByteArray();
    }
}

toModelEditor::toModelEditor(QWidget *parent,
                             QAbstractItemModel *model,
                             QModelIndex current,
                             bool sql,
                             bool modal) : QDialog(parent),
    Current(current),
    Model(model)
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
        Editor = new toModelEditor::toModelSQL(this);
    else
        Editor = new toModelEditor::toModelText(this);
    vbox->addWidget(Editor);

    Editable = Model->flags(Current) & Qt::ItemIsEditable;
    Editor->sciEditor()->setReadOnly(!Editable);
    Editor->setFocus();
    Editor->setWordWrap(true);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
    vbox->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    if (Editable)
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
    WordWrapAct->setChecked(true);
    connect(WordWrapAct, SIGNAL(toggled(bool)),
            Editor, SLOT(setWordWrap(bool)));
    Toolbar->addAction(WordWrapAct);

    QAction * XMLWrapAct = new QAction(QIcon(":/icons/xmlwrap.png"), tr("XML Format"), Toolbar);
    XMLWrapAct->setCheckable(true);
    connect(XMLWrapAct, SIGNAL(toggled(bool)),
            Editor, SLOT(setXMLWrap(bool)));
    Toolbar->addAction(XMLWrapAct);

    if (Editable)
    {
        Toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(paste_xpm))),
                           tr("Paste from clipboard"),
                           Editor,
                           SLOT(paste()));
    }

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

    Toolbar->addSeparator();

    NullCheck = new QCheckBox(tr("NULL"), Toolbar);
    Toolbar->addWidget(NullCheck);
    connect(NullCheck, SIGNAL(toggled(bool)), this, SLOT(setNull(bool)));
    NullCheck->setEnabled(!Editor->sciEditor()->isReadOnly());
    NullCheck->setFocusPolicy(Qt::StrongFocus);

    Label = new QLabel(Toolbar);
    Label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    Label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                     QSizePolicy::Minimum));
    Toolbar->addWidget(Label);

    readSettings();

    connect(this,
            SIGNAL(finished(int)),
            this,
            SLOT(writeSettings()));

    changePosition(Current);
    if (!modal)
        show();
}

void toModelEditor::readSettings()
{
    QSettings settings;
    settings.beginGroup("memoEditor");
    resize(settings.value("size", QSize(400, 300)).toSize());
}

void toModelEditor::writeSettings() const
{
    QSettings settings;
    settings.beginGroup("memoEditor");
    settings.setValue("size", size());
}

void toModelEditor::setText(const QString &str)
{
    Editor->sciEditor()->setText(str);
    NullCheck->setChecked(str.isNull());
    Editor->sciEditor()->setModified(false);
}

void toModelEditor::setNull(bool nul)
{
    Editor->sciEditor()->setModified(true);
    Editor->setDisabled(nul);
}

void toModelEditor::store()
{
    if (Editor->sciEditor()->isReadOnly())
        return;
    if (Editor->sciEditor()->isModified())
    {
        if (!Editor->isEnabled())
            Model->setData(Current, QVariant(QString::null));
        else
            Model->setData(Current, Editor->sciEditor()->text());
    }
    accept();
}

void toModelEditor::changePosition(QModelIndex index)
{
    Current = index;
	try
	{
		QVariant const &data = Model->data(Current, Qt::UserRole);
		if (data.type() == QVariant::UserType && data.canConvert<toQValue::complexType*>())
		{
			toQValue::complexType *i = data.value<toQValue::complexType*>();
			setText(i->editData());
			return;
		}

		if (Editable)
			setText(Model->data(Current, Qt::EditRole).toString());
		else
		{
			setText(Model->data(Current, Qt::DisplayRole).toString());
		}
	}
	TOCATCH
}

void toModelEditor::firstColumn()
{
    QModelIndex index = Model->index(Current.row(), 1);
    changePosition(index);
    Label->setText("<B>" + Model->headerData(index.column(), Qt::Horizontal).toString() + "</B>");
}

void toModelEditor::previousColumn()
{
    if (Current.column() <= 1)
        return;

    QModelIndex index = Model->index(Current.row(), Current.column() - 1);
    changePosition(index);
    Label->setText("<B>" + Model->headerData(index.column(), Qt::Horizontal).toString() + "</B>");
}

void toModelEditor::nextColumn()
{
    if (Current.column() >= Model->columnCount() - 1)
    {
        printf("ignore\n");
        return;
    }

    QModelIndex index = Model->index(Current.row(), Current.column() + 1);
    changePosition(index);
    Label->setText("<B>" + Model->headerData(index.column(), Qt::Horizontal).toString() + "</B>");
}

void toModelEditor::lastColumn()
{
    QModelIndex index = Model->index(Current.row(), Model->columnCount() - 1);
    changePosition(index);
    Label->setText("<B>" + Model->headerData(index.column(), Qt::Horizontal).toString() + "</B>");
}
