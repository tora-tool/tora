
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
#include "topiechart.h"
#include "tomain.h"
#include "toresult.h"
#include "totool.h"

#include <math.h>
#include <time.h>

#include <qapplication.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qtooltip.h>
#include <QMdiArea>

#include <QPaintEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QMenu>
#include <QPrintDialog>

#include "icons/chart.xpm"
#include "icons/print.xpm"


toPieChart::toPieChart(QWidget *parent, const char *name, Qt::WFlags f)
        : QWidget(parent, f)
{

    if (name)
        setObjectName(name);

    Legend = true;
    DisplayPercent = false;

    setWindowIcon(QPixmap(const_cast<const char**>(chart_xpm)));
    setMinimumSize(60, 60);
    Menu = NULL;

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(createPopupMenu(const QPoint &)));

    setMinimumSize(60, 60);
    // Use list font
    QString str(toConfigurationSingle::Instance().listFont());
    if (!str.isEmpty())
    {
        QFont font(toStringToFont(str));
        setFont(font);
    }
}

toPieChart::~toPieChart()
{
}

#define FONT_ALIGN Qt::AlignLeft|Qt::AlignTop|Qt::TextExpandTabs

void toPieChart::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
        openCopy();
}

void toPieChart::openCopy(void)
{
    QWidget *newWin = new toPieChart(this,
                                     toMainWidget()->workspace(),
                                     NULL,
                                     0);

    newWin->setAttribute(Qt::WA_DeleteOnClose);
    newWin->show();
    newWin->raise();
    newWin->setFocus();
    toMainWidget()->updateWindowsMenu();
}

void toPieChart::setValues(std::list<double> &values, std::list<QString> &labels)
{
    Values = values;
    Labels = labels;

    emit newValues(values, labels);
    update();
}


toPieChart::toPieChart(toPieChart *pie,
                       QWidget *parent,
                       const char *name,
                       Qt::WFlags f)
        : QWidget(parent, f | Qt::Window),
        Values(pie->Values),
        Labels(pie->Labels),
        Postfix(pie->Postfix),
        Legend(pie->Legend),
        DisplayPercent(pie->DisplayPercent),
        Title(pie->Title)
{

    if (name)
        setObjectName(name);

    setWindowIcon(QPixmap(const_cast<const char**>(chart_xpm)));
    Menu = NULL;

    setWindowTitle(Title);

    setMinimumSize(60, 60);
    // Use list font
    QString str(toConfigurationSingle::Instance().listFont());
    if (!str.isEmpty())
    {
        QFont font(toStringToFont(str));
        setFont(font);
    }
}

void toPieChart::createPopupMenu(const QPoint &pos)
{
    if (!Menu)
    {
        Menu = new QMenu(this);

        Menu->addAction(QIcon(print_xpm),
                        tr("&Print..."),
                        this,
                        SLOT(editPrint()));

        Menu->addAction(tr("&Open in new window"),
                        this,
                        SLOT(openCopy()));
    }

    Menu->exec(QCursor::pos());
}

bool toPieChart::event(QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        QString t = findLabel(helpEvent->pos());
        if (!t.isEmpty())
            QToolTip::showText(helpEvent->globalPos(), t);
        else
            QToolTip::hideText();
    }
    return QWidget::event(event);
}

void toPieChart::paintChart(QPainter *p, QRect rect)
{
    QFontMetrics fm = p->fontMetrics();

    int right = rect.width();
    int bottom = rect.height();

    double tot = 0;
    {
        for (std::list<double>::iterator i = Values.begin();i != Values.end();i++)
            tot += *i;
    }

    if (!Title.isEmpty())
    {
        p->save();
        QFont f = p->font();
        f.setBold(true);
        p->setFont(f);
        QRect bounds = fm.boundingRect(0, 0, rect.width(), rect.height(), FONT_ALIGN, Title);
        p->drawText(0, 2, rect.width(), bounds.height(), Qt::AlignHCenter | Qt::AlignTop, Title);
        p->restore();
        p->translate(0, bounds.height() + 2);
        bottom -= bounds.height() + 2;
    }

    if (Legend)
    {
        int lwidth = 0;
        int lheight = 0;

        toResult *Result = dynamic_cast<toResult *>(this);

        std::list<double>::iterator j = Values.begin();
        {
            for (std::list<QString>::iterator i = Labels.begin();i != Labels.end();i++)
            {
                QString sizstr;
                if (j != Values.end())
                {
                    if (DisplayPercent)
                        sizstr.sprintf("%0.1f", 100*(*j) / tot);
                    else
                        sizstr = toQValue::formatNumber(*j);
                    sizstr += Postfix;
                }
                if (!(*i).isEmpty())
                {
                    QString str = toTranslateMayby(Result ? Result->sqlName() : QString::fromLatin1("toPieChart"), *i);
                    str += QString::fromLatin1(" (");
                    str += sizstr;
                    str += QString::fromLatin1(")");
                    QRect bounds = fm.boundingRect(0, 0, 10000, 10000, FONT_ALIGN, str);
                    if (lwidth < bounds.width())
                        lwidth = bounds.width();
                    lheight += bounds.height();
                }
            }
            if (j != Values.end())
                j++;
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
        right = lx;
        p->save();
        p->setBrush(Qt::white);
        p->drawRect(lx, ly, lwidth, lheight);
        p->restore();
        lx += 12;
        ly += 2;
        int cp = 0;
        j = Values.begin();
        for (std::list<QString>::iterator i = Labels.begin();i != Labels.end();i++)
        {
            QString sizstr;
            if (j != Values.end())
            {
                if (DisplayPercent)
                    sizstr.sprintf("%0.1f", 100*(*j) / tot);
                else
                    sizstr = toQValue::formatNumber(*j);
                sizstr += Postfix;
            }

            if (!(*i).isEmpty())
            {
                QString str = toTranslateMayby(Result ? Result->sqlName() : QString::fromLatin1("toPieChart"), *i);
                str += QString::fromLatin1(" (");
                str += sizstr;
                str += QString::fromLatin1(")");

                QRect bounds = fm.boundingRect(lx, ly, 100000, 100000, FONT_ALIGN, str);
                p->drawText(bounds, FONT_ALIGN, str);
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

            if (j != Values.end())
                j++;
        }
    }

    if (tot == 0)
    {
        p->drawText(QRect(2, 2, right - 4, bottom - 4),
                    Qt::AlignCenter//  | Qt::WordBreak
                    , tr("All values are 0 in this chart"));
        return ;
    }

    int cp = 0;
    int pos = 0;
    unsigned int count = 0;
    ChartRect = p->combinedTransform().mapRect(QRect(2, 2, right - 4, bottom - 4));
    Angels.clear();
    for (std::list<double>::iterator i = Values.begin();i != Values.end();i++)
    {
        count++;
        int size = int(*i * 5760 / tot);
        if (count == Values.size())
            size = 5760 - pos;

        if (size > 0)
        {
            p->save();
            QBrush brush(toChartBrush(cp));
            p->setBrush(brush.color());
            p->drawPie(2, 2, right - 4, bottom - 4, pos, size);
            if (brush.style() != Qt::SolidPattern)
            {
                p->setBrush(QBrush(Qt::white, brush.style()));
                p->drawPie(2, 2, right - 4, bottom - 4, pos, size);
            }
            p->restore();
            pos += size;
        }
        Angels.insert(Angels.end(), pos);
        cp++;
    }
}

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

QString toPieChart::findLabel(QPoint p)
{
    if (ChartRect.contains(p))
    {
        QPoint center = ChartRect.center();
        p -= center;
        int iang;
        if (p.x() != 0)
        {
            double angle = atan(double( -p.y()) / p.x());
            iang = int(angle * 180 * 16 / M_PI);
            if (p.x() < 0)
                iang += 180 * 16;
            if (iang < 0)
                iang += 360 * 16;
        }
        else if (p.y() <= 0)
            iang = 90 * 16;
        else
            iang = 270 * 16;

        std::list<int>::iterator i = Angels.begin();
        std::list<QString>::iterator j = Labels.begin();
        while (i != Angels.end() && j != Labels.end())
        {
            if ((*i) > iang)
                return *j;
            i++;
            j++;
        }
    }
    return QString::null;
}

void toPieChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    paintChart(&p, QRect(0, 0, width(), height()));
}

void toPieChart::editPrint(void)
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

toPieConnector::toPieConnector(toPieChart *pieChart, toLineChart *lineChart)
{
    PieChart = pieChart;
    LineChart = lineChart;
    Flow = false;
    LastStamp = 0;

    connect(PieChart, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(LineChart, SIGNAL(destroyed()), this, SLOT(deleteLater()));
    connect(PieChart, SIGNAL(newValues(std::list<double> &, std::list<QString> &)),
            this, SLOT(newValues(std::list<double> &, std::list<QString> &)));
}

void toPieConnector::newValues(std::list<double> &values, std::list<QString> &labels)
{
    std::map<QString, double> reorderMap;
    std::list<double>::iterator val = values.begin();
    std::list<QString>::iterator lab = labels.begin();
    while (val != values.end() && lab != labels.end())
    {
        reorderMap[*lab] = *val;
        val++;
        lab++;
    }

    std::list<QString> newlabs = LineChart->labels();
    std::list<double> reordVals;
    std::map<QString, double>::iterator rv;
    lab = newlabs.begin();

    while (lab != newlabs.end())
    {
        rv = reorderMap.find(*lab);
        if (rv != reorderMap.end())
        {
            reordVals.insert(reordVals.end(), (*rv).second);
            reorderMap.erase(rv);
        }
        else
            reordVals.insert(reordVals.end(), 0);
        lab++;
    }
    if (reorderMap.begin() != reorderMap.end())
    {
        rv = reorderMap.begin();
        while (rv != reorderMap.end())
        {
            newlabs.insert(newlabs.end(), (*rv).first);
            reordVals.insert(reordVals.end(), (*rv).second);
            rv++;
        }
        LineChart->setLabels(newlabs);
    }

    QString nowstr;
    try
    {
        nowstr = toNow(toCurrentConnection(PieChart));
    }
    catch (...)
        {}

    if (Flow)
    {
        time_t now = time(NULL);
        if (now != LastStamp)
        {
            if (LastValues.size() > 0)
            {
                std::list<double> dispVal;
                std::list<double>::iterator i = reordVals.begin();
                std::list<double>::iterator j = LastValues.begin();
                while (i != reordVals.end() && j != LastValues.end())
                {
                    dispVal.insert(dispVal.end(), (*i - *j) / (now - LastStamp));
                    i++;
                    j++;
                }
                LineChart->addValues(dispVal, nowstr);
            }
            LastValues = reordVals;
            LastStamp = now;
        }
    }
    else
        LineChart->addValues(reordVals, nowstr);
}
