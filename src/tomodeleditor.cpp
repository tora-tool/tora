/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "tohighlightedtext.h"
#include "tomarkedtext.h"
#include "tomodeleditor.h"
#include "toresultview.h"
#include "tomain.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qtoolbutton.h>

#include <QKeyEvent>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QSettings>
#include <QAbstractItemModel>

#include "icons/commit.xpm"
#include "icons/copy.xpm"
#include "icons/cut.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/forward.xpm"
#include "icons/next.xpm"
#include "icons/paste.xpm"
#include "icons/previous.xpm"
#include "icons/rewind.xpm"

class toMemoText : public toMarkedText
{
    toModelEditor *MemoEditor;

public:
    toMemoText(toModelEditor *edit, QWidget *parent, const char *name = NULL)
        : toMarkedText(parent, name), MemoEditor(edit)
        { }

    virtual void keyPressEvent(QKeyEvent *e)
    {
        if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toModelEditor", "Ctrl+Return", "Memo Editor|Save changes"))))
        {
            MemoEditor->store();
            e->accept();
        }
        else
        {
            toMarkedText::keyPressEvent(e);
        }
    }
};


class toMemoSQL : public toHighlightedText
{
    toModelEditor *MemoEditor;

public:
    toMemoSQL(toModelEditor *edit, QWidget *parent, const char *name = NULL)
        : toHighlightedText(parent, name), MemoEditor(edit)
    { }

    virtual void keyPressEvent(QKeyEvent *e)
    {
        if (toCheckKeyEvent(e, QKeySequence(qApp->translate("toModelEditor", "Ctrl+Return", "Memo Editor|Save changes"))))
        {
            MemoEditor->store();
            e->accept();
        }
        else
        {
            toHighlightedText::keyPressEvent(e);
        }
    }
};

void toModelEditor::openFile()
{
    Editor->editOpen();
}

void toModelEditor::saveFile()
{
    Editor->editSave(true);
}

toModelEditor::toModelEditor(QWidget *parent,
                             QAbstractItemModel *model,
                             QModelIndex current,
                             bool sql,
                             bool modal) : QDialog(parent),
                                           Model(model),
                                           Current(current)
{
    setModal(modal);
    setMinimumSize(400, 300);
    QDesktopWidget *paramDesktop = new QDesktopWidget;
    setMaximumWidth(paramDesktop->availableGeometry(this).width()*2 / 3);

    setWindowTitle("Memo Editor");

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    Toolbar = toAllocBar(this, tr("Memo Editor"));
    vbox->addWidget(Toolbar);

    if (sql)
        Editor = new toMemoSQL(this, this);
    else
        Editor = new toMemoText(this, this);
    vbox->addWidget(Editor);

    Editable = Model->flags(Current) & Qt::ItemIsEditable;
    Editor->setReadOnly(!Editable);
    Editor->setFocus();

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
    NullCheck->setObjectName(TO_TOOLBAR_WIDGET_NAME);
    Toolbar->addWidget(NullCheck);
    connect(NullCheck, SIGNAL(toggled(bool)), this, SLOT(setNull(bool)));
    NullCheck->setEnabled(!Editor->isReadOnly());
    NullCheck->setFocusPolicy(Qt::StrongFocus);

    Label = new QLabel(Toolbar);
    Label->setObjectName(TO_TOOLBAR_WIDGET_NAME);
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
    Editor->setText(str);
    NullCheck->setChecked(str.isNull());
    Editor->setModified(false);
}

void toModelEditor::setNull(bool nul)
{
    Editor->setModified(true);
    Editor->setDisabled(nul);
}

void toModelEditor::store()
{
    if (Editor->isReadOnly())
        return;
    if (Editor->isModified())
    {
        if (!Editor->isEnabled())
            Model->setData(Current, QVariant(QString::null));
        else
            Model->setData(Current, Editor->text());
    }
    accept();
}

void toModelEditor::changePosition(QModelIndex index)
{
    Current = index;
    if(Editable)
        setText(Model->data(Current, Qt::EditRole).toString());
    else
        setText(Model->data(Current, Qt::DisplayRole).toString());
}

void toModelEditor::firstColumn()
{
    QModelIndex index = Model->index(Current.row(), 1);
    changePosition(index);
    Label->setText("<B>" + Model->headerData(index.column(), Qt::Horizontal).toString() + "</B>");
}

void toModelEditor::previousColumn()
{
    if(Current.column() <= 1)
        return;

    QModelIndex index = Model->index(Current.row(), Current.column() - 1);
    changePosition(index);
    Label->setText("<B>" + Model->headerData(index.column(), Qt::Horizontal).toString() + "</B>");
}

void toModelEditor::nextColumn()
{
    if(Current.column() >= Model->columnCount() - 1) {
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
