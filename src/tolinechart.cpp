
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
#include "tolinechart.h"
#include "ui_tolinechartsetupui.h"
#include "tomain.h"
#include "toresultview.h"
#include "totool.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qspinbox.h>
#include <qvalidator.h>
#include <QMdiArea>

#include <QString>
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QScrollBar>
#include <QMenu>
#include <QPrintDialog>

#include "icons/print.xpm"
#include "icons/chart.xpm"

double toLineChart::round(double round, bool up)
{
    double base = 1.0E-5;
    double mult = 1;
    if (round < 0)
    {
        mult = -1;
        round = -round;
        up = !up;
    }
    for (;;base *= 10)
    {
        if (up)
        {
            if (base >= round)
                return mult * base;
            else if (base*2.5 >= round)
                return mult*base*2.5;
            else if (base*5 >= round)
                return mult*base*5;
        }
        else if (base > round)
        {
            if (base / 2 <= round)
                return mult*base / 2;
            else if (base / 4 <= round)
                return mult*base / 4;
            else if (base / 10 <= round)
                return mult*base / 10;
            else
                return 0;
        }
    }
}

void toLineChart::setSamples(int samples)
{
    Samples = samples;

    if (Samples > 0)
    {
        while (int(XValues.size()) > Samples)
            XValues.erase(XValues.begin());

        for (std::list<std::list<double> >::iterator i = Values.begin();i != Values.end();i++)
            while (int((*i).size()) > Samples)
                (*i).erase((*i).begin());
    }
    update();
}

toLineChart::toLineChart(QWidget *parent, const char *name, Qt::WFlags f)
        : QWidget(parent, f)
{
    if (name)
        setObjectName(name);

    setWindowIcon(QPixmap((const char**)chart_xpm));
    Menu = NULL;
    MinAuto = MaxAuto = true;
    MinValue = MaxValue = 0;
    Legend = true;
    Last = false;
    Grid = 5;
    AxisText = true;
    MousePoint[0] = MousePoint[1] = QPoint( -1, -1);
    DisplaySamples = toConfigurationSingle::Instance().displaySamples();

    clearZoom();

    setSamples(toConfigurationSingle::Instance().chartSamples());

    setMinimumSize(80, 50);

    // Use list font
    QString str(toConfigurationSingle::Instance().listFont());
    if (!str.isEmpty())
    {
        QFont font(toStringToFont(str));
        setFont(font);
    }

    Horizontal = new QScrollBar(Qt::Horizontal, this);
    Horizontal->hide();
    Vertical = new QScrollBar(Qt::Vertical, this);
    Vertical->hide();
    connect(Vertical, SIGNAL(valueChanged(int)), this, SLOT(verticalChange(int)));
    connect(Horizontal, SIGNAL(valueChanged(int)), this, SLOT(horizontalChange(int)));

    toMainWidget()->addChart(this);
}

void toLineChart::addValues(std::list<double> &value, const QString &xValue)
{
    if (int(XValues.size()) == Samples && Samples > 0)
        XValues.erase(XValues.begin());
    XValues.insert(XValues.end(), xValue);

    if (Samples > 0)
        for (std::list<std::list<double> >::iterator i = Values.begin();i != Values.end();i++)
            if (int((*i).size()) == Samples)
                (*i).erase((*i).begin());

    std::list<double>::iterator j = value.begin();
    for (std::list<std::list<double> >::iterator i = Values.begin();i != Values.end() && j != value.end();i++)
    {
        (*i).insert((*i).end(), *j);
        j++;
    }
    while (j != value.end())
    {
        std::list<double> t;
        t.insert(t.end(), *j);
        Values.insert(Values.end(), t);
        j++;
    }

    emit valueAdded(value, xValue);

    update();
}

QRect toLineChart::fixRect(QPoint p1, QPoint p2)
{
    if (p1.x() < Chart.x())
        p1.setX(Chart.x());
    if (p2.x() < Chart.x())
        p2.setX(Chart.x());
    if (p1.x() > Chart.x() + Chart.width())
        p1.setX(Chart.x() + Chart.width());
    if (p2.x() > Chart.x() + Chart.width())
        p2.setX(Chart.x() + Chart.width());

    if (p1.y() < Chart.y())
        p1.setY(Chart.y());
    if (p2.y() < Chart.y())
        p2.setY(Chart.y());
    if (p1.y() > Chart.y() + Chart.height())
        p1.setY(Chart.y() + Chart.height());
    if (p2.y() > Chart.y() + Chart.height())
        p2.setY(Chart.y() + Chart.height());

    return QRect(std::min(p1.x(), p2.x()),
                 std::min(p1.y(), p2.y()),
                 abs(p1.x() - p2.x()),
                 abs(p1.y() - p2.y()));
}

#define FONT_ALIGN Qt::AlignLeft|Qt::AlignTop|Qt::TextExpandTabs

void toLineChart::paintTitle(QPainter *p, QRect &rect)
{
    if (!Title.isEmpty())
    {
        p->save();
        QFont f = p->font();
        f.setBold(true);
        p->setFont(f);
        QFontMetrics fm = p->fontMetrics();
        QRect bounds = fm.boundingRect(0, 0, rect.width(), rect.width(), FONT_ALIGN, Title);
        p->drawText(0, 2, rect.width(), bounds.height(), Qt::AlignHCenter | Qt::AlignTop, Title);
        p->restore();
        p->translate(0, bounds.height() + 2);
        rect.setTop(rect.top() + bounds.height() + 2);
    }
    if (Last)
    {
        QString str;
        for (std::list<std::list<double> >::iterator i = Values.begin();i != Values.end();i++)
        {
            if ((*i).begin() != (*i).end())
            {
                if (!str.isEmpty())
                    str += QString::fromLatin1("\n");
                str += toQValue::formatNumber(*(*i).rbegin());
                str += YPostfix;
            }
        }
        if (!str.isEmpty())
        {
            QFontMetrics fm = p->fontMetrics();
            QRect bounds = fm.boundingRect(0, 0, rect.width(), rect.height(), FONT_ALIGN, str);
            p->drawText(0, 2, rect.width(), bounds.height(), Qt::AlignHCenter | Qt::AlignTop, str);
            p->translate(0, bounds.height());
            rect.setTop(rect.top() + bounds.height());
        }
    }
}

void toLineChart::paintLegend(QPainter *p, QRect &rect)
{
    QFontMetrics fm = p->fontMetrics();
    if (Legend)
    {
        int lwidth = 0;
        int lheight = 0;

        toResult *Result = dynamic_cast<toResult *>(this);

        {
            std::list<bool>::iterator j = Enabled.begin();
            for (std::list<QString>::iterator i = Labels.begin();i != Labels.end();i++)
            {
                QString t = toTranslateMayby(Result ? Result->sqlName() : QString::fromLatin1("toLineChart"), *i);
                if (!t.isEmpty() && *i != " " && (j == Enabled.end() || *j))
                {
                    QRect bounds = fm.boundingRect(0, 0, 10000, 10000, FONT_ALIGN, t);
                    if (lwidth < bounds.width())
                        lwidth = bounds.width();
                    lheight += bounds.height();
                }
                if (j != Enabled.end())
                    j++;
            }
        }
        if (lheight > 0)
        {
            lheight += 4;
            lwidth += 14;
        }
        int lx = rect.width() - lwidth - 2;
        int ly = 2;
        if (lx < 50)
            lx = 50;
        p->save();
        p->setBrush(Qt::white);
        p->drawRect(lx, ly, lwidth, lheight);
        p->restore();
        rect.setRight(lx - 2);
        lx += 12;
        ly += 2;
        int cp = 0;
        std::list<bool>::iterator j = Enabled.begin();
        for (std::list<QString>::iterator i = Labels.begin();i != Labels.end();i++)
        {
            QString t = toTranslateMayby(Result ? Result->sqlName() : QString::fromLatin1("toLineChart"), *i);
            QRect bounds = fm.boundingRect(lx, ly, 100000, 100000, FONT_ALIGN, t);
            if (!t.isEmpty() && t != " " && (j == Enabled.end() || *j))
            {
                p->drawText(bounds, FONT_ALIGN, t);
                p->save();
                QBrush brush(toChartBrush(cp));
                p->setBrush(brush.color());
                p->drawRect(lx - 10, ly + bounds.height() / 2 - fm.ascent() / 2, 8, fm.ascent());
                if (brush.style() != Qt::SolidPattern)
                {
                    p->setBrush(QBrush(Qt::white, brush.style()));
                    p->drawRect(lx - 10, ly + bounds.height() / 2 - fm.ascent() / 2, 8, fm.ascent());
                }
                p->restore();
                ly += bounds.height();
            }
            cp++;
            if (j != Enabled.end())
                j++;
        }
    }
}

void toLineChart::paintAxis(QPainter *p, QRect &rect)
{
    QFontMetrics fm = p->fontMetrics();

    bool leftAxis = true;
    if ((zMaxValue == 0 || zMaxValue == round(0, true)) && zMinValue == 0)
        leftAxis = false;

    if (AxisText)
    {
        int yoffset = 0;
        QString minstr;
        QString maxstr;
        QRect ybounds;
        if (leftAxis)
        {
            minstr = toQValue::formatNumber(zMinValue);
            maxstr = toQValue::formatNumber(zMaxValue);
            maxstr += YPostfix;
            QRect bounds = fm.boundingRect(0, 0, 100000, 100000, FONT_ALIGN, minstr);
            yoffset = bounds.height();
            ybounds = fm.boundingRect(0, 0, 100000, 100000, FONT_ALIGN, maxstr);
            if (yoffset < ybounds.height())
                yoffset = ybounds.height();
        }

        QString maxXstr;
        QString minXstr;
        int xoffset = 0;
        if (XValues.size() > 1)
        {

            int count = 0;
            for (std::list<QString>::reverse_iterator i = XValues.rbegin();i != XValues.rend();i++)
            {
                if (count == SkipSamples)
                    maxXstr = *i;
                else if (count <= SkipSamples + UseSamples - 1 || UseSamples < 0)
                    minXstr = *i;
                count++;
            }

            QRect bounds = fm.boundingRect(0, 0, 100000, 100000, FONT_ALIGN, minXstr);
            xoffset = bounds.height();
            bounds = fm.boundingRect(0, 0, 100000, 100000, FONT_ALIGN, maxXstr);
            if (xoffset < bounds.height())
                xoffset = bounds.height();

            bool xscroll = false;
            if ((UseSamples < count && UseSamples >= 0) || SkipSamples > 0)
            {
                xscroll = true;
                xoffset += Horizontal->sizeHint().height();
            }

            bool yscroll = false;
            if (Zooming && (MinValue != zMinValue || MaxValue != zMaxValue))
            {
                yscroll = true;
                yoffset += Vertical->sizeHint().width();
            }

            if (zMinValue != 0 || zMaxValue != 0)
            {
                p->save();
                p->rotate( -90);
                p->drawText(xoffset - rect.height() + 2, fm.ascent() + 1, minstr);
                p->drawText( -2 - ybounds.width(), fm.ascent() + 1, maxstr);
                p->restore();
            }
            else
                yoffset = (yscroll ? Horizontal->sizeHint().height() : 0);

            p->drawText(yoffset + 2,
                        rect.height() - xoffset - 2 + (xscroll ? Horizontal->sizeHint().height() : 0),
                        rect.width() - 4 - yoffset,
                        xoffset - (xscroll ? Horizontal->sizeHint().height() : 0),
                        Qt::AlignLeft | Qt::AlignTop, minXstr);
            p->drawText(yoffset + 2,
                        rect.height() - xoffset - 2 + (xscroll ? Horizontal->sizeHint().height() : 0),
                        rect.width() - 4 - yoffset,
                        xoffset - (xscroll ? Horizontal->sizeHint().height() : 0),
                        Qt::AlignRight | Qt::AlignTop, maxXstr);
            p->translate(yoffset, 0);

            rect.setLeft(yoffset);
            rect.setBottom(rect.bottom() - xoffset);

            if (xscroll)
            {
                Horizontal->setGeometry(yoffset + 2,
                                        rect.bottom() - 1,
                                        rect.width() - 3,
                                        Horizontal->sizeHint().height());
                disconnect(Horizontal, SIGNAL(valueChanged(int)), this, SLOT(horizontalChange(int)));
                Horizontal->setMinimum(0);
                Horizontal->setMaximum(count - UseSamples);
                Horizontal->setValue(count - UseSamples - SkipSamples);
                connect(Horizontal, SIGNAL(valueChanged(int)), this, SLOT(horizontalChange(int)));
//                 Horizontal->setSteps(1, UseSamples); - qt4 no equivalent found
                Horizontal->show();
            }
            else
                Horizontal->hide();

            if (yscroll)
            {
                int part = int(100.0 * (MaxValue - MinValue) / (zMaxValue - zMinValue));
                Vertical->setGeometry(yoffset + 2 - Vertical->sizeHint().width(),
                                      rect.top() + 2,
                                      Vertical->sizeHint().width(),
                                      rect.height() - 3);
                disconnect(Vertical, SIGNAL(valueChanged(int)), this, SLOT(verticalChange(int)));
                Vertical->setMinimum(0);
                Vertical->setMaximum(part - 100);

                Vertical->setValue(int(100*(MaxValue - zMaxValue) / (zMaxValue - zMinValue)));

                connect(Vertical, SIGNAL(valueChanged(int)), this, SLOT(verticalChange(int)));
//                 Vertical->setSteps(10, 100);
                Vertical->show();
            }
            else
                Vertical->hide();

        }
    }

    p->save();
    p->setBrush(Qt::white);
    Chart = QRect(rect.left() + 2, rect.top() + 2, rect.width() - 3, rect.height() - 3);
    p->drawRect(2, 2, rect.width() - 3, rect.height() - 3);
    p->restore();
    if (Grid > 1)
    {
        p->save();
        p->setPen(Qt::gray);
        for (int i = 1;i < Grid;i++)
        {
            int ypos = (rect.height() - 4) * i / Grid + 2;
            int xpos = (rect.width() - 4) * i / Grid + 2;
            p->drawLine(3, ypos, rect.width() - 3, ypos);
            p->drawLine(xpos, 3, xpos, rect.height() - 3);
        }
        p->restore();
    }
}

void toLineChart::paintChart(QPainter *p, QRect &rect)
{
    if (!Zooming)
    {
        if (MinAuto || MaxAuto)
        {
            bool first = true;
            std::list<bool>::iterator k = Enabled.begin();
            for (std::list<std::list<double> >::iterator i = Values.begin();i != Values.end();i++)
            {
                if (k == Enabled.end() || *k)
                {
                    for (std::list<double>::iterator j = (*i).begin();j != (*i).end();j++)
                    {
                        if (first)
                        {
                            zMinValue = *j;
                            zMaxValue = *j;
                            first = false;
                        }
                        else if (zMaxValue < *j)
                            zMaxValue = *j;
                        else if (zMinValue > *j)
                            zMinValue = *j;
                    }
                }
                if (k != Enabled.end())
                    k++;
            }
            zMaxValue = round(zMaxValue, true);
            zMinValue = round(zMinValue, false);
        }
        if (!MinAuto)
            zMinValue = MinValue;
        else
        {
            zMinValue = round(zMinValue, false);
            MinValue = zMinValue;
        }
        if (!MaxAuto)
            zMaxValue = MaxValue;
        else
        {
            zMaxValue = round(zMaxValue, true);
            MaxValue = zMaxValue;
        }
    }

    paintTitle(p, rect);
    paintLegend(p, rect);
    paintAxis(p, rect);

    int cp = 0;
    int samples = countSamples();
    if (samples > 1)
    {
        const QMatrix &mtx = p->worldMatrix();
        p->setClipRect(int(mtx.dx() + 2), int(mtx.dy() + 2), rect.width() - 3, rect.height() - 3);
        if (Zooming)
            p->drawText(2, 2, rect.width() - 4, rect.height() - 4,
                        Qt::AlignLeft | Qt::AlignTop, tr("Zoom"));
        std::list<bool>::iterator k = Enabled.begin();
        for (std::list<std::list<double> >::iterator i = Values.begin();i != Values.end();i++)
        {
            if (k == Enabled.end() || *k)
            {
                p->save();
                QBrush brush(toChartBrush(cp));
                Qt::PenStyle pens;
                switch (brush.style())
                {
                case Qt::BDiagPattern:
                    pens = Qt::DashLine;
                    break;
                case Qt::FDiagPattern:
                    pens = Qt::DotLine;
                    break;
                case Qt::DiagCrossPattern:
                    pens = Qt::DashDotLine;
                    break;
                case Qt::CrossPattern:
                    pens = Qt::DashDotDotLine;
                    break;
                default:
                    pens = Qt::SolidLine;
                }
                p->setPen(QPen(brush.color(), pens));
                std::list<double> &val = *i;
                int count = 0;
                bool first = true;
                int lval = 0;
                int lx = rect.width() - 2;
                int skip = SkipSamples;
                for (std::list<double>::reverse_iterator j = val.rbegin();j != val.rend() && lx >=
                        2;
                        j++)
                {
                    if (skip > 0)
                        skip--;
                    else
                    {
                        int val = int(rect.height() - 2 - ((*j - zMinValue) / (zMaxValue - zMinValue) * (rect.height() - 4)));
                        if (!first)
                        {
                            int x = lx;
                            x = rect.width() - 2 - (count + 1) * (rect.width() - 4) / samples;
                            p->drawLine(x, val, lx, lval);
                            lx = x;
                        }
                        else
                            first = false;
                        lval = val;
                        count++;
                        if (count >= samples)
                            break;
                    }
                }
                p->restore();
            }
            cp++;
            if (k != Enabled.end())
                k++;
        }
    }
}

void toLineChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setFont(font());
    QRect rect(0, 0, width(), height());
    paintChart(&p, rect);
    MousePoint[1] = QPoint( -1, -1);
}

void toLineChart::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && MousePoint[0] != QPoint( -1, -1))
    {
        if (MousePoint[1] != QPoint( -1, -1))
        {
            QRect rect = fixRect(MousePoint[0], MousePoint[1]);
            QPainter p(this);
//FIXME qt4:             p.setRasterOp(NotROP);
            p.drawRect(rect);
        }
        if (MousePoint[0].x() != e->x() &&
                MousePoint[0].y() != e->y())
        {
            QRect rect = fixRect(MousePoint[0], e->pos());
            int samples = countSamples();
            UseSamples = samples * rect.width() / Chart.width() + 1;
            if (UseSamples < 2)
                UseSamples = 2;
            SkipSamples += samples * (Chart.width() + Chart.x() - rect.x() - rect.width()) / Chart.width();
            Zooming = true;

            double t = (zMaxValue - zMinValue) * (Chart.y() + Chart.height() - rect.y() - rect.height()) / Chart.height() + zMinValue;
            zMaxValue = (zMaxValue - zMinValue) * (Chart.y() + Chart.height() - rect.y()) / Chart.height() + zMinValue;
            zMinValue = t;
            update();
        }
        MousePoint[1] = MousePoint[0] = QPoint( -1, -1);
    }
    else if (e->button() == Qt::RightButton)
    {
        if (Chart.contains(e->pos()))
        {
            clearZoom();
            update();
        }
    }
}

int toLineChart::countSamples(void)
{
    int samples = UseSamples;
    if (UseSamples <= 1)
    {
        if (Samples < 0)
            samples = XValues.size();
        else
            samples = Samples;
    }
    return samples;
}

void toLineChart::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        MousePoint[0] = e->pos();
    else if (e->button() == Qt::RightButton)
    {
        if (!Chart.contains(e->pos()) || !Zooming)
        {
            if (!Menu)
            {
                Menu = new QMenu(this);
                Menu->addAction(QIcon(QPixmap(const_cast<const char**>(print_xpm))),
                                tr("&Print..."),
                                this,
                                SLOT(editPrint()));
                Menu->addAction(tr("&Open in new window"), this, SLOT(openCopy()));

                Menu->addSeparator();

                Menu->addAction(tr("&Chart Manager..."), this, SLOT(chartSetup()));
                Menu->addAction(tr("&Properties..."), this, SLOT(setup()));

                Menu->addSeparator();

                Menu->addAction(tr("Clear Chart"), this, SLOT(clear()));
                addMenues(Menu);
            }
            Menu->popup(e->globalPos());
        }
    }
}

void toLineChart::chartSetup(void)
{
    toMainWidget()->setupChart(this);
}

void toLineChart::setup(void)
{
    QDialog dialog(this);
    Ui::toLineChartSetupUI setup; //, NULL, true);
    setup.setupUi(&dialog);

    setup.MinValue->setText(toQValue::formatNumber(MinValue));
    setup.MaxValue->setText(toQValue::formatNumber(MaxValue));
    setup.AutoMax->setChecked(MaxAuto);
    setup.AutoMin->setChecked(MinAuto);
    setup.ShowAxis->setChecked(AxisText);
    setup.ShowLast->setChecked(Last);
    setup.ShowLegend->setChecked(Legend);
    setup.Grids->setValue(Grid);

    setup.Enabled->addColumn(tr("Enabled charts"));
    std::list<QString>::iterator i = Labels.begin();
    std::list<bool>::iterator j = Enabled.begin();
    toTreeWidgetItem *item = NULL;
    setup.Enabled->setSorting( -1);
    setup.Enabled->setSelectionMode(toTreeWidget::Multi);
    while (i != Labels.end())
    {
        item = new toResultViewItem(setup.Enabled, item, *i);
        if (j != Enabled.end())
        {
            if (*j)
                item->setSelected(true);
            j++;
        }
        else
            item->setSelected(true);
        i++;
    }

    setup.MaxValue->setValidator(new QDoubleValidator(setup.MaxValue));
    setup.MinValue->setValidator(new QDoubleValidator(setup.MinValue));
    if (Samples < 0)
    {
        setup.UnlimitedSamples->setChecked(true);
        setup.Samples->setValue(XValues.size());
    }
    else
        setup.Samples->setValue(Samples);

    if (DisplaySamples < 0)
    {
        setup.AllSamples->setChecked(true);
        setup.DisplaySamples->setValue(setup.Samples->value());
    }
    else
        setup.DisplaySamples->setValue(DisplaySamples);

    if (dialog.exec())
    {
        MinValue = setup.MinValue->text().toDouble();
        MaxValue = setup.MaxValue->text().toDouble();
        MaxAuto = setup.AutoMax->isChecked();
        MinAuto = setup.AutoMin->isChecked();
        AxisText = setup.ShowAxis->isChecked();
        Last = setup.ShowLast->isChecked();
        Legend = setup.ShowLegend->isChecked();

        if (setup.AllSamples->isChecked())
        {
            if (DisplaySamples == UseSamples)
                UseSamples = DisplaySamples = -1;
            else
                DisplaySamples = -1;
        }
        else
        {
            if (DisplaySamples == UseSamples)
                UseSamples = DisplaySamples = setup.DisplaySamples->value();
            else
                DisplaySamples = setup.DisplaySamples->value();
        }

        if (setup.UnlimitedSamples->isChecked())
            setSamples( -1);
        else
            setSamples(setup.Samples->value());


        Grid = setup.Grids->value();

        std::list<bool> ena;
        for (toTreeWidgetItem *item = setup.Enabled->firstChild();item;item = item->nextSibling())
            ena.insert(ena.end(), item->isSelected());

        Enabled = ena;

        update();
    }
}

void toLineChart::editPrint(void)
{
    TOPrinter printer;
    QPrintDialog dialog(&printer, this);
    dialog.setMinMax(1, 1);
    if (dialog.exec())
    {
        printer.setCreator(tr(TOAPPNAME));
        QPainter painter(&printer);
        QRect rect(printer.pageRect());
        paintChart(&painter, rect);
    }
}

void toLineChart::clearZoom(void)
{
    Zooming = false;
    SkipSamples = 0;
    UseSamples = DisplaySamples;
    zMinValue = -1;
    zMaxValue = -1;
}

void toLineChart::mouseMoveEvent(QMouseEvent *e)
{
    if (MousePoint[0] != QPoint( -1, -1))
    {
        QPainter p(this);
//FIXME qt4        p.setRasterOp(NotROP);
        if (MousePoint[1] != QPoint( -1, -1))
            p.drawRect(fixRect(MousePoint[0], MousePoint[1]));
        MousePoint[1] = e->pos();
        p.drawRect(fixRect(MousePoint[0], MousePoint[1]));
    }
}


void toLineChart::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        openCopy();
}

toLineChart *toLineChart::openCopy(QWidget *parent)
{
    toLineChart *newWin = new toLineChart(this,
                                          parent ? parent : toMainWidget()->workspace(),
                                          NULL,
                                          (Qt::WindowFlags) (parent ? 0 : Qt::WA_DeleteOnClose));
    if (!parent)
    {
        newWin->show();
        newWin->raise();
        newWin->setFocus();
        toMainWidget()->updateWindowsMenu();
    }
    return newWin;
}

toLineChart::~toLineChart()
{
    toMainWidget()->removeChart(this);
}

toLineChart::toLineChart (toLineChart *chart, QWidget *parent, const char *name, Qt::WFlags f)
        : QWidget(parent, f)
{

    if (name)
        setObjectName(name);

    Menu = NULL;
    Values = chart->Values;
    XValues = chart->XValues;
    Labels = chart->Labels;
    Legend = chart->Legend;
    Last = false;
    Grid = 5;
    AxisText = true;
    MinValue = chart->MinValue;
    MinAuto = chart->MinAuto;
    MaxValue = chart->MaxValue;
    MaxAuto = chart->MaxAuto;
    Samples = chart->Samples;
    Title = chart->Title;
    YPostfix = chart->YPostfix;
    DisplaySamples = chart->DisplaySamples;
    Enabled = chart->Enabled;

    setWindowTitle(Title);
    setWindowIcon(QPixmap((const char**)chart_xpm));

    clearZoom();

    setMinimumSize(80, 50);

    Horizontal = new QScrollBar(Qt::Horizontal, this);
    Horizontal->hide();
    Vertical = new QScrollBar(Qt::Vertical, this);
    Vertical->hide();
    connect(Vertical, SIGNAL(valueChanged(int)), this, SLOT(verticalChange(int)));
    connect(Horizontal, SIGNAL(valueChanged(int)), this, SLOT(horizontalChange(int)));

    // Use list font
    QString str(toConfigurationSingle::Instance().listFont());
    if (!str.isEmpty())
    {
        QFont font(toStringToFont(str));
        setFont(font);
    }

    toMainWidget()->addChart(this);
}

void toLineChart::exportData(std::map<QString, QString> &ret, const QString &prefix)
{
    int id = 0;
    {
        for (std::list<QString>::iterator i = Labels.begin();i != Labels.end();i++)
        {
            id++;
            ret[prefix + ":Labels:" + QString::number(id).toLatin1()] = *i;
        }
    }
    id = 0;
    {
        for (std::list<QString>::iterator i = XValues.begin();i != XValues.end();i++)
        {
            id++;
            ret[prefix + ":XValues:" + QString::number(id).toLatin1()] = *i;
        }
    }
    id = 0;
    for (std::list<std::list<double> >::iterator i = Values.begin();i != Values.end();i++)
    {
        QString value;

        for (std::list<double>::iterator j = (*i).begin();j != (*i).end();j++)
        {
            if (!value.isNull())
                value += QString::fromLatin1(",");
            value += QString::number(*j);
        }
        id++;
        ret[prefix + ":Values:" + QString::number(id).toLatin1()] = value;
    }
    ret[prefix + ":Title"] = Title;
}

void toLineChart::importData(std::map<QString, QString> &ret, const QString &prefix)
{
    int id;
    std::map<QString, QString>::iterator i;

    id = 1;
    Labels.clear();
    while ((i = ret.find(prefix + ":Labels:" + QString::number(id).toLatin1())) != ret.end())
    {
        Labels.insert(Labels.end(), (*i).second);
        id++;
    }

    id = 1;
    XValues.clear();
    while ((i = ret.find(prefix + ":XValues:" + QString::number(id).toLatin1())) != ret.end())
    {
        XValues.insert(XValues.end(), (*i).second);
        id++;
    }

    id = 1;
    Values.clear();
    QRegExp comma(QString::fromLatin1(","));
    while ((i = ret.find(prefix + ":Values:" + QString::number(id).toLatin1())) != ret.end())
    {
        QStringList lst = (*i).second.split(comma);
        std::list<double> vals;
        for (int j = 0;j < lst.count();j++)
            vals.insert(vals.end(), lst[j].toDouble());

        Values.insert(Values.end(), vals);
        id++;
    }
    Samples = id - 2;
    Title = ret[prefix + ":Title"];
    update();
}

void toLineChart::horizontalChange(int val)
{
    SkipSamples = XValues.size() - UseSamples - val;
    update();
}

void toLineChart::verticalChange(int val)
{
    double size = (zMaxValue - zMinValue);
    zMaxValue = MaxValue - size / 100 * val;
    if (val == Vertical->maximum())
        zMinValue = MinValue;
    else
        zMinValue = MaxValue - size / 100 * (val + 100);

    update();
}
