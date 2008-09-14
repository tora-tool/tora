
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#include "utils.h"

#include "toconf.h"
#include "tolistviewformatter.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toparamget.h"
#include "toresultview.h"
#include "tosearchreplace.h"
#include "tosql.h"
#include "totool.h"
#include "toresultlistformat.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qprinter.h>
#include <qregexp.h>
#include <qstyle.h>
#include <QMdiArea>

#include <QKeyEvent>
#include <QList>
#include <QString>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QMimeData>
#include <QColorGroup>

static int MaxColDisp;
static bool Gridlines;

void toResultViewMLine::setText(int col, const QString &text)
{
    toResultViewItem::setText(col, text);
    int pos = 0;
    int lines = 0;
    do
    {
        pos = text.indexOf("\n", pos);
        lines++;
        pos++;
    }
    while (pos > 0);
    if (lines > Lines)
        Lines = lines;
}

void toResultViewMLine::setText(int col, const toQValue &text)
{
    if (text.isDouble())
        toResultViewItem::setText(col, text);
    setText(col, QString(text));
}

void toResultViewMLine::setup(void)
{
    toResultViewItem::setup();
    int margin = listView()->itemMargin() * 2 + 1;
    setHeight((listView()->fontMetrics().height() + 1)*Lines + margin);
}

void toResultViewMLine::paintCell(QPainter *pnt, const QColorGroup & cg,
                                  int column, int width, int alignment)
{
    toResultViewItem::paintCell(pnt, cg, column,
                                std::max(toTreeWidgetItem::width(pnt->fontMetrics(), listView(), column), width),
                                alignment);
    if (Gridlines)
    {
        pnt->setPen(Qt::gray);
        pnt->drawLine(width - 1, 0, width - 1, height());
        pnt->drawLine(0, height() - 1, width - 1, height() - 1);
    }
}

static int TextWidth(const QFontMetrics &fm, const QString &str)
{
    int lpos = 0;
    int pos = 0;
    int maxWidth = 0;
    do
    {
        pos = str.indexOf("\n", lpos);
        QRect bounds = fm.boundingRect(str.mid(lpos, pos - lpos));
        if (bounds.width() > maxWidth)
            maxWidth = bounds.width();
        lpos = pos + 1;
    }
    while (pos >= 0);
    return maxWidth;
}

int toResultViewMLine::realWidth(const QFontMetrics &fm, const toTreeWidget *top, int column, const QString &txt) const
{
    if (!MaxColDisp)
    {
        MaxColDisp = toConfigurationSingle::Instance().maxColDisp();
        Gridlines = toConfigurationSingle::Instance().displayGridlines();
    }
    QString t = text(column);
    if (t.isNull())
        t = txt;
    return std::min(TextWidth(fm, t), MaxColDisp) + top->itemMargin()*2 - fm.minLeftBearing() - fm.minRightBearing() + 1;
}

QString toResultViewItem::firstText(int col) const
{
    if (col >= ColumnCount)
        return QString::null;
    QString txt = ColumnData[col].Data;
    int pos = txt.indexOf('\n');
    if (pos != -1)
        return txt.mid(0, pos) + "...";
    return txt;
}

QString toResultViewItem::text(int col) const
{
    if (col >= ColumnCount)
        return QString::null;
    if (ColumnData[col].Type == keyData::Number)
        return toTreeWidgetItem::text(col);
    return firstText(col);
}

int toResultViewItem::realWidth(const QFontMetrics &fm, const toTreeWidget *top, int column, const QString &txt) const
{
    if (!MaxColDisp)
    {
        MaxColDisp = toConfigurationSingle::Instance().maxColDisp();
        Gridlines = toConfigurationSingle::Instance().displayGridlines();
    }
    QString t = text(column);
    if (t.isNull())
        t = txt;
    QRect bounds = fm.boundingRect(t);
    return std::min(bounds.width(), MaxColDisp) + top->itemMargin()*2 - fm.minLeftBearing() - fm.minRightBearing() + 1;
}

void toResultViewItem::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
#if 0                           // disabled, not overriding correct function anyhow
    // null related background handling
    QColorGroup colNull(cg);
    if ((toConfigurationSingle::Instance().indicateEmpty() && text(column) == "{null}")
            || text(column).isNull())
    {
        QColor nullColor;
        nullColor.setNamedColor(toConfigurationSingle::Instance().indicateEmptyColor());
        colNull.setColor(QColorGroup::Base, nullColor);
    }

    toTreeWidgetItem::paintCell(p, colNull, column, width, align);
    toResultView *view = dynamic_cast<toResultView *>(listView());
    if (view && (itemBelow() == NULL || itemBelow()->itemBelow() == NULL))
        view->addItem();
    if (Gridlines)
    {
        p->setPen(Qt::gray);
        p->drawLine(width - 1, 0, width - 1, height());
        p->drawLine(0, height() - 1, width - 1, height() - 1);
    }
#endif
}

#define ALLOC_SIZE 10

void toResultViewItem::setText(int col, const QString &txt)
{
    if (txt != text(col))
    {
        if (col >= ColumnCount || !ColumnData)
        {
            int ns = (col + ALLOC_SIZE) / ALLOC_SIZE * ALLOC_SIZE;
            keyData *nd = new keyData[ns];
            int i;
            for (i = 0;i < ColumnCount;i++)
                nd[i] = ColumnData[i];
            while (i < ns)
            {
                nd[i].Width = 0;
                nd[i].Type = keyData::String;
                i++;
            }
            delete[] ColumnData;
            ColumnData = nd;
            ColumnCount = ns;
        }

        static QRegExp number(QString::fromLatin1("^-?\\d*\\.?\\d+E?-?\\d*.?.?$"));

        ColumnData[col].Data = txt;

        if (txt == "N/A")
        {
            ColumnData[col].Type = keyData::String;
            ColumnData[col].KeyAsc = "\xff";
            ColumnData[col].KeyDesc = "\x00";
        }
        else if (number.indexIn(txt) >= 0)   // qt4 match()
        {
            ColumnData[col].Type = keyData::Number;

            static char buf[100];
            double val = txt.toFloat();
            if (val < 0)
                sprintf(buf, "\x01%015.5f", val);
            else
                sprintf(buf, "%015.5f", val);
            ColumnData[col].KeyAsc = ColumnData[col].KeyDesc = QString::fromLatin1(buf);
        }
        else
        {
            ColumnData[col].Type = keyData::String;
            ColumnData[col].KeyAsc = ColumnData[col].KeyDesc = ColumnData[col].Data;
        }
        ColumnData[col].Width = realWidth(listView()->fontMetrics(), listView(), col, txt);
    }
    toTreeWidgetItem::setText(col, firstText(col));
}

void toResultViewItem::setText(int col, const toQValue &text)
{
    setText(col, QString(text));
    if (text.isDouble())
        ColumnData[col].Data = QString::number(text.toDouble());
}

toResultViewCheck::toResultViewCheck(toTreeWidget *parent, toTreeWidgetItem *after, const QString &text, toTreeWidgetCheck::Type type)
        :
        toTreeWidgetCheck(parent, after, QString::null, type)
{
    ColumnData = NULL;
    ColumnCount = 0;
    if (!text.isNull())
        setText(0, text);
}

toResultViewCheck::toResultViewCheck(toTreeWidgetItem *parent, toTreeWidgetItem *after, const QString &text, toTreeWidgetCheck::Type type)
        :
        toTreeWidgetCheck(parent, after, QString::null, type)
{
    ColumnData = NULL;
    ColumnCount = 0;
    if (!text.isNull())
        setText(0, text);
}

void toResultViewCheck::setText(int col, const QString &txt)
{
    if (txt != text(col))
    {
        if (col >= ColumnCount || !ColumnData)
        {
            int ns = (col + ALLOC_SIZE) / ALLOC_SIZE * ALLOC_SIZE;
            keyData *nd = new keyData[ns];
            int i;
            for (i = 0;i < ColumnCount;i++)
                nd[i] = ColumnData[i];
            while (i < ns)
            {
                nd[i].Width = 0;
                nd[i].Type = keyData::String;
                i++;
            }
            delete[] ColumnData;
            ColumnData = nd;
            ColumnCount = ns;
        }

        static QRegExp number(QString::fromLatin1("^\\d*\\.?\\d+E?-?\\d*.?.?$"));

        ColumnData[col].Data = txt;

        if (txt == "N/A")
        {
            ColumnData[col].Type = keyData::String;
            ColumnData[col].KeyAsc = "\xff";
            ColumnData[col].KeyDesc = "\x00";
        }
// qt4        else if (number.match(txt) >= 0)
        else if (number.indexIn(txt) >= 0)
        {
            ColumnData[col].Type = keyData::Number;

            static char buf[100];
            double val = txt.toFloat();
            if (val < 0)
                sprintf(buf, "\x01%015.5f", val);
            else
                sprintf(buf, "%015.5f", val);
            ColumnData[col].KeyAsc = ColumnData[col].KeyDesc = QString::fromLatin1(buf);
        }
        else
        {
            ColumnData[col].Type = keyData::String;
            ColumnData[col].KeyAsc = ColumnData[col].KeyDesc = ColumnData[col].Data;
        }
        ColumnData[col].Width = realWidth(listView()->fontMetrics(), listView(), col, txt);
    }
    toTreeWidgetCheck::setText(col, firstText(col));
}

void toResultViewCheck::setText(int col, const toQValue &text)
{
    setText(col, QString(text));
    if (text.isDouble())
        ColumnData[col].Data = QString::number(text.toDouble());
}

void toResultViewMLCheck::setText(int col, const QString &text)
{
    toResultViewCheck::setText(col, text);
    int pos = 0;
    int lines = 0;
    do
    {
        pos = text.indexOf("\n", pos);
        lines++;
        pos++;
    }
    while (pos > 0);
    if (lines > Lines)
        Lines = lines;
}

void toResultViewMLCheck::setText(int col, const toQValue &text)
{
    if (text.isDouble())
        toResultViewCheck::setText(col, text);
    setText(col, QString(text));
}

void toResultViewMLCheck::setup(void)
{
    toResultViewCheck::setup();
    int margin = listView()->itemMargin() * 2;
    setHeight((listView()->fontMetrics().height() + 1)*Lines + margin);
}

void toResultViewMLCheck::paintCell(QPainter *pnt, const QColorGroup & cg,
                                    int column, int width, int alignment)
{
    toResultViewCheck::paintCell(pnt, cg, column,
                                 std::max(toTreeWidgetCheck::width(pnt->fontMetrics(), listView(), column), width),
                                 alignment);
}

int toResultViewMLCheck::realWidth(const QFontMetrics &fm, const toTreeWidget *top, int column, const QString &txt) const
{
    if (!MaxColDisp)
    {
        MaxColDisp = toConfigurationSingle::Instance().maxColDisp();
        Gridlines = toConfigurationSingle::Instance().displayGridlines();
    }
    QString t = text(column);
    if (t.isNull())
        t = txt;
    int wx = top->itemMargin() * 2 - fm.minLeftBearing() - fm.minRightBearing() + 1;
    if (column == 0)
        wx += top->style()->pixelMetric(QStyle::PM_CheckListButtonSize) + 4 + top->itemMargin();

    return std::min(TextWidth(fm, t), MaxColDisp) + wx;
}

int toResultViewCheck::realWidth(const QFontMetrics &fm, const toTreeWidget *top, int column, const QString &txt) const
{
    if (!MaxColDisp)
    {
        MaxColDisp = toConfigurationSingle::Instance().maxColDisp();
        Gridlines = toConfigurationSingle::Instance().displayGridlines();
    }
    QString t = text(column);
    if (t.isNull())
        t = txt;
    QRect bounds = fm.boundingRect(t);
    int wx = top->itemMargin() * 2 - fm.minLeftBearing() - fm.minRightBearing() + 1;
    if (column == 0)
        wx += top->style()->pixelMetric(QStyle::PM_CheckListButtonSize) + 4 + top->itemMargin();

    return std::min(bounds.width(), MaxColDisp) + wx;
}


void toResultViewCheck::paintCell(QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
    toTreeWidgetCheck::paintCell(p, cg, column, width, align);
    toResultView *view = dynamic_cast<toResultView *>(listView());
    if (view && (itemBelow() == NULL || itemBelow()->itemBelow() == NULL))
        view->addItem();
}

QString toResultViewCheck::text(int col) const
{
    if (col >= ColumnCount)
        return QString::null;
    if (ColumnData[col].Type == keyData::Number)
        return toTreeWidgetCheck::text(col);
    return firstText(col);
}

QString toResultViewCheck::firstText(int col) const
{
    if (col >= ColumnCount)
        return QString::null;
    QString txt = ColumnData[col].Data;
    int pos = txt.indexOf('\n');
    if (pos != -1)
        return txt.mid(0, pos) + "...";
    return txt;
}

toListView::toListView(QWidget *parent, const char *name, Qt::WFlags f)
        : toTreeWidget(parent, name, f),
        toEditWidget(false, true, true,
                     false, false,
                     false, false, false,
                     true, true, false)
{
    FirstSearch = false;
    setTreeStepSize(15);
    setSelectionMode(Extended);
    setAllColumnsShowFocus(true);

    setSorting(-1);
    Menu = NULL;
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(displayMenu(const QPoint &)));
    QString str(toConfigurationSingle::Instance().listFont());
    if (!str.isEmpty())
    {
        QFont font(toStringToFont(str));
        setFont(font);
    }
    LastMove = QPoint(-1, -1);
}

toListView::~toListView()
{
}

void toListView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
#if 0                           // todo
    QPoint p = e->pos();
    int col = headerItem()->sectionAt(p.x());
    toTreeWidgetItem *item = itemAt(contentsToViewport(p));
    toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(item);
    toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(item);
    QClipboard *clip = qApp->clipboard();
    if (resItem)
        clip->setText(resItem->allText(col));
    else if (chkItem)
        clip->setText(chkItem->allText(col));
    else if (item)
        clip->setText(item->text(col));

    Q3Header *head = header();
    for (int i = 0;i < columns();i++)
    {
        QString str;
        if (resItem)
            str = resItem->allText(i);
        else if (chkItem)
            str = chkItem->allText(col);
        else if (item)
            str = item->text(i);

        try
        {
            toParamGet::setDefault(toCurrentConnection(this),
                                   head->label(i).lower(), toUnnull(str));
        }
        catch (...) {}
    }
    toTreeWidget::contentsMouseDoubleClickEvent(e);
#endif
}

void toListView::contentsMouseMoveEvent(QMouseEvent *e)
{
#if 0
    if (e->state() == Qt::LeftButton &&
            e->stateAfter() == Qt::LeftButton &&
            LastMove.x() > 0 &&
            LastMove != e->pos())
    {
        QPoint p = e->pos();
        int col = header()->sectionAt(p.x());
        toTreeWidgetItem *item = itemAt(contentsToViewport(p));
        toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(item);
        toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(item);
        QString str;
        if (resItem)
            str = resItem->allText(col);
        else if (chkItem)
            str = chkItem->allText(col);
        else if (item)
            str = item->text(col);
        if (str.length())
        {
            Q3DragObject *d = new Q3TextDrag(str, this);
            d->dragCopy();
        }
    }
    else
    {
        LastMove = e->pos();
        toTreeWidget::contentsMouseMoveEvent(e);
    }
#endif
}

void toListView::contentsMousePressEvent(QMouseEvent *e)
{
#if 0                           // todo
    LastMove = QPoint(-1, -1);
    toTreeWidget::contentsMousePressEvent(e);
#endif
}

void toListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
#if 0                           // todo
    LastMove = QPoint(-1, -1);
    toTreeWidget::contentsMouseReleaseEvent(e);
#endif
}

#if 0
toTreeWidgetItem *toListView::printPage(TOPrinter *printer, QPainter *painter, toTreeWidgetItem *top, int &column, int &level, int pageNo, bool paint)
{
    Q3PaintDeviceMetrics wmetr(this);
    Q3PaintDeviceMetrics metrics(printer);

    double wpscalex = (double(metrics.width()) * wmetr.widthMM() / metrics.widthMM() / wmetr.width());
    double wpscaley = (double(metrics.height()) * wmetr.heightMM() / metrics.heightMM() / wmetr.height());

    painter->save();
    QFont font = painter->font();
    font.setPointSizeFloat(font.pointSizeFloat() / std::max(wpscalex, wpscaley));
    painter->setFont(font);

    painter->scale(wpscalex, wpscaley);

    double mwidth = metrics.width() / wpscalex;
    double mheight = metrics.height() / wpscaley;
    double x = 0;
    if (paint)
    {
        QString numPage(tr("Page: %1").arg(pageNo));
        painter->drawText(0, int(metrics.height() / wpscaley) - header()->height(), int(metrics.width() / wpscalex),
                          header()->height(),
                          Qt::SingleLine | Qt::AlignRight | Qt::AlignVCenter,
                          numPage);
        painter->drawText(0, int(metrics.height() / wpscaley) - header()->height(), int(metrics.width() / wpscalex),
                          header()->height(),
                          Qt::SingleLine | Qt::AlignHCenter | Qt::AlignVCenter,
                          middleString());
        painter->drawText(0, int(metrics.height() / wpscaley) - header()->height(), int(metrics.width() / wpscalex),
                          header()->height(),
                          Qt::SingleLine | Qt::AlignLeft | Qt::AlignVCenter,
                          sqlName());
        painter->drawLine(0, header()->height() - 1, int(mwidth), header()->height() - 1);
    }
    font = toListView::font();
    font.setPointSizeFloat(font.pointSizeFloat() / std::max(wpscalex, wpscaley));
    painter->setFont(font);

    for (int i = column;i < columns();i++)
    {
        double width = columnWidth(i);
        if (width + x >= mwidth)
        {
            if (i == column)
                width = mwidth - x - 1;
            else
                break;
        }
        if (paint)
            painter->drawText(int(x), 0, int(width),
                              header()->height(),
                              Qt::SingleLine | Qt::AlignLeft | Qt::AlignVCenter, header()->label(i));
        x += width;
    }
    if (paint)
        painter->translate(0, header()->height());

    double y = (header()->height() + 1) + header()->height();
    int curLevel = level;
    int tree = rootIsDecorated() ? treeStepSize() : 0;
    int newCol = -1;
    toTreeWidgetItem *item = top;
    while (item && (y < mheight || item == top))
    {
        if (column == 0)
            x = curLevel;
        else
            x = 0;
        painter->translate(x, 0);
        for (int i = column;i < columns();i++)
        {
            double width = columnWidth(i);
            if (width + x >= mwidth)
            {
                if (i == column)
                    width = mwidth - x - 1;
                else
                {
                    newCol = i;
                    break;
                }
            }
            if (i == 0)
                width -= curLevel;
            if (paint)
            {
                item->setSelected(false);
                item->paintCell(painter, qApp->palette().active(), i, int(width), columnAlignment(i));
                painter->translate(width, 0);
            }
            x += width;
        }
        if (paint)
            painter->translate(-x, item->height());
        y += item->height();
        if (item->firstChild())
        {
            item = item->firstChild();
            curLevel += tree;
        }
        else if (item->nextSibling())
            item = item->nextSibling();
        else
        {
            do
            {
                item = item->parent();
                curLevel -= tree;
            }
            while (item && !item->nextSibling());
            if (item)
                item = item->nextSibling();
        }
    }
    if (paint)
        painter->drawLine(0, 0, int(mwidth), 0);
    painter->restore();
    if (newCol >= 0)
    {
        column = newCol;
        return top;
    }
    column = 0;
    level = curLevel;
    return item;
}
#endif

void toListView::editPrint(void)
{
#if 0
    TOPrinter printer;

    std::map<int, int> PageColumns;
    std::map<int, toTreeWidgetItem *> PageItems;

    int column = 0;
    int tree = rootIsDecorated() ? treeStepSize() : 0;
    int page = 1;
    PageColumns[1] = 0;
    toTreeWidgetItem *item = PageItems[1] = firstChild();

    printer.setCreator(tr(TOAPPNAME));
    QPainter painter(&printer);

    while ((item = printPage(&printer, &painter, item, column, tree, page++, false)))
    {
        PageColumns[page] = column;
        PageItems[page] = item;
    }

    printer.setMinMax(1, page - 1);
    printer.setFromTo(1, page - 1);
    if (printer.setup())
    {
        QList<int> pages;
        for (int i = printer.fromPage();i <= printer.toPage() || (printer.toPage() == 0 && i < page);i++)
            pages += i;

        for (QList<int>::iterator pageit = pages.begin();pageit != pages.end();pageit++)
        {
            page = *pageit;
            item = PageItems[page];
            column = PageColumns[page];

            printPage(&printer, &painter, item, column, tree, page, true);
            printer.newPage();
            painter.resetXForm();
            qApp->processEvents();
            QString str = tr("Printing page %1").arg(page);
            toStatusMessage(str, false, false);
        }
        painter.end();
        toStatusMessage(tr("Done printing"), false, false);
    }
#endif
}

void toListView::setDisplayMenu(QMenu *m)
{
    if (Menu)
    {
        delete Menu;
        Menu = NULL;
    }

    Menu = m;
}


void toListView::displayMenu(const QPoint &pos)
{
    toTreeWidgetItem *item = itemAt(pos);

    if (!item)
        return;

    if (!Menu)
    {
        Menu = new QMenu(this);
        displayAct = Menu->addAction(tr("Display in editor..."));

        QMenu *just = new QMenu(tr("Alignment"), this);
        leftAct   = just->addAction(tr("Left"));
        centerAct = just->addAction(tr("Center"));
        rightAct  = just->addAction(tr("Right"));
        connect(just,
                SIGNAL(triggered(QAction *)),
                this,
                SLOT(menuCallback(QAction *)));

        Menu->addSeparator();

        copyAct = Menu->addAction(tr("&Copy field"));
        if (selectionMode() == Multi || selectionMode() == Extended)
        {
            copySelAct  = Menu->addAction(tr("Copy selection"));
            copyHeadAct = Menu->addAction(tr("Copy selection with header"));
        }
        copyTransAct = Menu->addAction(tr("Copy transposed"));
        if (selectionMode() == Multi || selectionMode() == Extended)
        {
            Menu->addSeparator();
            selectAllAct = Menu->addAction(tr("Select all"));
        }

        Menu->addSeparator();

        exportAct = Menu->addAction(tr("Export to file..."));
        if (!Name.isEmpty())
        {
            Menu->addSeparator();
            editAct = Menu->addAction(tr("Edit SQL..."));
        }

        connect(Menu,
                SIGNAL(triggered(QAction *)),
                this,
                SLOT(menuCallback(QAction *)));
        addMenues(Menu);
        emit displayMenu(Menu);
    }

    MenuItem = item;
    MenuColumn = currentColumn();
    Menu->exec(QCursor::pos());
}

void toListView::displayMemo(void)
{
    QString str = menuText();
    if (!str.isEmpty())
        new toMemoEditor(this, str, 0, MenuColumn);
}

void toListView::menuCallback(QAction *action)
{
    if (action == copyAct)
    {
        QClipboard *clip = qApp->clipboard();
        clip->setText(menuText());
    }
    else if (action == copySelAct)
    {
        try
        {
            QString str = exportAsText(false, true);
            if (!str.isNull())
            {
                QClipboard *clip = qApp->clipboard();
                QMimeData drag;
                drag.setHtml(str);
                clip->setMimeData(&drag);
            }
        }
        TOCATCH;
    }
    else if (action == leftAct)
        setColumnAlignment(MenuColumn, Qt::AlignLeft);
    else if (action == centerAct)
        setColumnAlignment(MenuColumn, Qt::AlignCenter);
    else if (action == rightAct)
        setColumnAlignment(MenuColumn, Qt::AlignRight);
    else if (action == copyHeadAct)
    {
        try
        {
            QString str = exportAsText(true, true);
            if (!str.isNull())
            {
                QClipboard *clip = qApp->clipboard();
                QMimeData drag;
                drag.setHtml(str);
                clip->setMimeData(&drag);
            }
        }
        TOCATCH;
    }
    else if (action == selectAllAct)
        selectAll(true);
//     else if(act ==
//     case TORESULT_MEMO:
//         displayMemo();
//         break;
    else if (action == copyTransAct)
        copyTransposed();
    else if (action == editAct)
        toMainWidget()->editSQL(Name);
    else if (action == exportAct)
        editSave(false);
    else
        toStatusMessage("Not yet implemented.");
}

QString toListView::menuText(void)
{
    toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(MenuItem);
    toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(MenuItem);
    QString str;
    if (resItem)
        str = resItem->allText(MenuColumn);
    else if (chkItem)
        str = chkItem->allText(MenuColumn);
    else if (MenuItem)
        str = MenuItem->text(MenuColumn);
    return str;
}

void toListView::focusInEvent(QFocusEvent *e)
{
    receivedFocus();
    toTreeWidget::focusInEvent(e);
}

bool toListView::searchNext(toSearchReplace *search)
{
    toTreeWidgetItem *item = currentItem();

    bool first = FirstSearch;
    FirstSearch = false;

    for (toTreeWidgetItem *next = NULL;item;item = next)
    {
        if (!first)
            first = true;
        else
        {
            for (int i = 0;i < columns();i++)
            {
                int pos = 0;
                int endPos = 0;

                toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(item);
                toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(item);
                QString txt;
                if (resItem)
                    txt = resItem->allText(i);
                else if (chkItem)
                    txt = chkItem->allText(i);
                else if (item)
                    txt = item->text(i);

                if (search->findString(item->text(0), pos, endPos))
                {
                    setCurrentItem(item);
                    return true;
                }
            }
        }

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
    return false;
}

toListView *toListView::copyTransposed(void)
{
    QMdiSubWindow * w = new QMdiSubWindow(toMainWidget()->workspace());

    toListView *lst = new toListView(toMainWidget()->workspace());

    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setWidget(lst);
    toMainWidget()->workspace()->addSubWindow(w);

    // qt4
//     lst->setWFlags(lst->getWFlags() | Qt::WDestructiveClose);
    lst->Name = Name;

    toTreeWidgetItem *next = NULL;
    for (int i = 1;i < columns();i++)
    {
        next = new toResultViewItem(lst, next);
        next->setText(0, headerItem()->text(i));
    }

    next = NULL;
    int col = 1;
    lst->addColumn(headerItem()->text(0));
    for (toTreeWidgetItem *item = firstChild();item;item = next)
    {

        lst->addColumn(item->text(0));
        toTreeWidgetItem *ci = lst->firstChild();
        for (int i = 1;i < columns() && ci;i++)
        {
            ci->setText(col, item->text(i));
            ci = ci->nextSibling();
        }

        if (item->firstChild())
        {
            next = item->firstChild();
        }
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
        col++;
    }
    w->setWindowTitle(Name);
    w->show();
    toMainWidget()->updateWindowsMenu();
    return lst;
}

bool toListView::editSave(bool)
{
    try
    {
        QString delimiter;
        QString separator;
        int type = exportType(separator, delimiter);

        QString nam;
        switch (type)
        {
        case - 1:
            return false;
        default:
            nam = "*.txt";
            break;
        case 2:
            nam = "*.csv";
            break;
        case 3:
            nam = "*.html";
            break;
        case 4:
            nam = "*.sql";
            break;
        }

        QString filename = toSaveFilename(QString::null, nam, this);
        if (filename.isEmpty())
            return false;

        return toWriteFile(filename, exportAsText(true, false, type, separator, delimiter));
    }
    TOCATCH
    return false;
}

void toListView::addMenues(QMenu *) {}

bool toListView::searchCanReplace(bool)
{
    return false;
}

int toListView::exportType(QString &separator, QString &delimiter)
{
    toResultListFormat format(this, NULL);
    if (!format.exec())
        return -1;

    format.saveDefault();

    separator = format.Separator->text();
    delimiter = format.Delimiter->text();

    return format.Format->currentIndex();

}


QString toListView::exportAsText(bool tincludeHeader, bool tonlySelection, int type,
                                 const QString &tsep, const QString &tdel)
{
    QString result;

    includeHeader = tincludeHeader;
    onlySelection = tonlySelection;
    sep = tsep;
    del = tdel;

    if (type < 0)
        type = exportType(sep, del);
    if (type < 0)
        return QString::null;

    std::auto_ptr<toListViewFormatter> pFormatter(toListViewFormatterFactory::Instance().CreateObject(type));
    result =  pFormatter->getFormattedString(*this);

    return result;
}

void toListView::exportData(std::map<QString, QString> &ret, const QString &prefix)
{
    int id = 0;
    for (int i = 0;i < columns();i++)
    {
        id++;
        ret[prefix + ":Labels:" + QString::number(id).toLatin1()] = headerItem()->text(i);
    }
    std::map<toTreeWidgetItem *, int> itemMap;
    toTreeWidgetItem *next = NULL;
    id = 0;
    if (rootIsDecorated())
        ret[prefix + ":Decorated"] = QString::fromLatin1("Yes");
    for (toTreeWidgetItem *item = firstChild();item;item = next)
    {
        id++;
        QString nam = prefix;
        nam += ":Items:";
        nam += QString::number(id).toLatin1();
        nam += ":";
        itemMap[item] = id;
        if (item->parent())
            ret[nam + "Parent"] = QString::number(itemMap[item->parent()]);
        else
            ret[nam + "Parent"] = QString::fromLatin1("0");
        if (item->isOpen())
            ret[nam + "Open"] = QString::fromLatin1("Yes");
        for (int i = 0;i < columns();i++)
        {
            toResultViewItem *resItem = dynamic_cast<toResultViewItem *>(item);
            toResultViewCheck *chkItem = dynamic_cast<toResultViewCheck *>(item);
            QString val;
            if (resItem)
                val = resItem->allText(i);
            else if (chkItem)
                val = resItem->allText(i);
            else
                val = item->text(i);
            ret[nam + QString::number(i).toLatin1()] = val;
        }

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
}

void toListView::importData(std::map<QString, QString> &ret, const QString &prefix)
{
    int id;
    std::map<QString, QString>::iterator i;
    clear();

    id = 1;
    while ((i = ret.find(prefix + ":Labels:" + QString::number(id).toLatin1())) != ret.end())
    {
        addColumn((*i).second);
        id++;
    }

    setRootIsDecorated(ret.find(prefix + ":Decorated") != ret.end());
    setSorting(0);

    std::map<int, toTreeWidgetItem *> itemMap;

    id = 1;
    while ((i = ret.find(prefix + ":Items:" + QString::number(id).toLatin1() + ":Parent")) != ret.end())
    {
        QString nam = prefix + ":Items:" + QString::number(id).toLatin1() + ":";
        int parent = (*i).second.toInt();
        toResultViewItem *item;
        if (parent)
            item = new toResultViewItem(itemMap[parent], NULL);
        else
            item = new toResultViewItem(this, NULL);
        if (!ret[nam + "Open"].isEmpty())
            item->setOpen(true);
        itemMap[id] = item;
        for (int j = 0;j < columns();j++)
            item->setText(j, ret[nam + QString::number(j).toLatin1()]);
        id++;
    }
}

bool toResultView::eof(void)
{
    return !Query || Query->eof();
}

QString toResultView::middleString()
{
    try
    {
        return connection().description();
    }
    catch (...)
    {
        return QString::null;
    }
}

void toResultView::setup(bool readable, bool dispCol)
{
    Query = NULL;
    ReadableColumns = readable;
    NumberColumn = dispCol;
    SortConnected = false;
    if (NumberColumn)
    {
        addColumn(QString::fromLatin1("#"));
        setColumnAlignment(0, Qt::AlignRight);
    }
    Filter = NULL;
    readAllEnabled(true);
    ReadAll = false;
    SortColumn = -2;
    SortAscending = true;
}

toResultView::toResultView(bool readable, bool dispCol, QWidget *parent, const char *name, Qt::WFlags f)
        : toListView(parent, name, f)
{
    setup(readable, dispCol);
}

toResultView::toResultView(QWidget *parent, const char *name, Qt::WFlags f)
        : toListView(parent, name, f)
{
    setup(false, true);
}

#define STOP_RESIZE_ROW 500

toTreeWidgetItem *toResultView::createItem(toTreeWidgetItem *last, const QString &str)
{
    if (childCount() == STOP_RESIZE_ROW && resizeMode() == toTreeWidget::NoColumn)
        for (int i = 0;i < columns();i++)
            setColumnWidthMode(i, toTreeWidget::Manual);

    return new toResultViewItem(this, last, str);
}

void toResultView::addItem(void)
{
    MaxColDisp = toConfigurationSingle::Instance().maxColDisp();
    Gridlines = toConfigurationSingle::Instance().displayGridlines();

    try
    {
        if (Query && !Query->eof())
        {
            RowNumber++;
            int disp = 0;
            toTreeWidgetItem *last = LastItem;
            LastItem = createItem(LastItem, QString::null);
            if (NumberColumn)
            {
                LastItem->setText(0, QString::number(RowNumber));
                disp = 1;
            }
            else
                LastItem->setText(columns(), QString::number(RowNumber));
            for (int j = 0;(j < Query->columns() || j == 0) && !Query->eof();j++)
                LastItem->setText(j + disp, Query->readValue());
            if (Filter && !Filter->check(LastItem))
            {
                delete LastItem;
                LastItem = last;
                RowNumber--;
            }
        }
    }
    TOCATCH
}

void toResultView::query(const QString &sql, const toQList &param)
{
    if (!handled())
        return ;

    if (!setSQLParams(sql, param))
        return ;

    delete Query;
    Query = NULL;
    LastItem = NULL;
    RowNumber = 0;

    clear();

    if (NumberColumn)
    {
        addColumn(QString::fromLatin1("#"));
        setColumnAlignment(0, Qt::AlignRight);
    }

    if (Filter)
        Filter->startingQuery();

    try
    {
        Query = new toQuery(connection(), sql, param);

        //printf("Query: %s \n", (const char*) Query->sql);
        toQDescList description = Query->describe();

        bool hidden = false;

        for (toQDescList::iterator i = description.begin();i != description.end();i++)
        {
            QString name = (*i).Name;
            if (ReadableColumns)
                toReadableColumn(name);

            if (name.length() > 0 && name.at(0) != ' ')
            {
                if (hidden)
                    throw tr("Can only hide last column in query");
                if (name.at(0) == '-')
                {
                    addColumn(toTranslateMayby(sqlName(), name.right(name.length() - 1)));
                    setColumnAlignment(columns() - 1, Qt::AlignRight);
                }
                else
                {
                    addColumn(toTranslateMayby(sqlName(), name));
                    if ((*i).AlignRight)
                        setColumnAlignment(columns() - 1, Qt::AlignRight);
                }
            }
            else
                hidden = true;
        }
        setResizeMode(resizeMode());

        if (NumberColumn)
            setSorting(0);
        else
            setSorting(Query->columns());

        int MaxNumber = toConfigurationSingle::Instance().maxNumber();
        for (int j = 0;j < MaxNumber && !Query->eof();j++)
            addItem();
        if (ReadAll || MaxNumber < 0)
            editReadAll();
    }
    TOCATCH
    updateContents();
}

void toResultView::editReadAll(void)
{
    if (!ReadAll)
        toStatusMessage(tr("Reading all entries"), false, false);
    int i = 0;
    while (!eof())
    {
        addItem();
        i++;
        if (i % 100 == 0)
            qApp->processEvents();
    }
}

toResultView::~toResultView()
{
    if (Query)
        Query->deleteLater();
    delete Filter;
}

void toResultView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_PageDown)
    {
        toTreeWidgetItem *item = firstChild();
        if (item && !eof() && item->height())
        {
            int num = visibleHeight() / item->height();
            while (num > 0)
            {
                addItem();
                num--;
            }
        }
    }
    toTreeWidget::keyPressEvent(e);
}

void toResultView::addMenues(QMenu *menu)
{
    menu->addSeparator();
    ReadAllAct = menu->addAction(tr("Read All"));
}

void toResultView::menuCallback(QAction *cmd)
{
    if (cmd == ReadAllAct)
        editReadAll();
    else
    {
        if (cmd == toListView::exportAct)
            editReadAll();
        toListView::menuCallback(cmd);
    }
}

int toResultView::queryColumns(void) const
{
    return Query ? Query->columns() : 0;
}

void toResultView::setSorting(int col, bool asc)
{
    if (col == SortColumn && asc == SortAscending)
        return ;
    SortColumn = col;
    SortAscending = asc;
    if (((col == 0 && NumberColumn) || (col == columns() && !NumberColumn)) && asc == true)
    {
        col = -1;
        toListView::setSorting(0, true);
        sort();
        QTimer::singleShot(1, this, SLOT(checkHeading()));
    }
    else if (SortConnected)
    {
        SortConnected = false;
//         disconnect(headerItem(), SIGNAL(clicked(int)), this, SLOT(headingClicked(int))); todo
    }
    toListView::setSorting(col, asc);
}

void toResultView::headingClicked(int col)
{
    if (col == SortColumn)
        setSorting(col, !SortAscending);
    else
        setSorting(col, true);
}

void toResultView::checkHeading(void)
{
    SortConnected = true;
//     connect(headerItem(), SIGNAL(clicked(int)), this, SLOT(headingClicked(int))); todo
}

void toResultView::refresh(void)
{
    int lastSort = SortColumn;
    bool lastAsc = SortAscending;
    toResult::refresh();
    if (lastSort >= 0)
        setSorting(lastSort, lastAsc);
}

void toResultFilter::exportData(std::map<QString, QString> &, const QString &) {}

void toResultFilter::importData(std::map<QString, QString> &, const QString &) {}
