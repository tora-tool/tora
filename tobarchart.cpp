/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include <qpainter.h>
#include <qworkspace.h>

#include "tobarchart.h"
#include "tomain.h"
#include "utils.h"

#include "tobarchart.moc"

toBarChart::toBarChart(QWidget *parent, const char *name, WFlags f)
        : toLineChart(parent, name, f)
{
    setMinValue(0);
}

#define FONT_ALIGN AlignLeft|AlignTop|ExpandTabs

void toBarChart::paintChart(QPainter *p, QRect &rect)
{
    QFontMetrics fm = p->fontMetrics();

    if (!Zooming)
    {
        if (MinAuto)
        {
            bool first = true;
            std::list<std::list<double> >::reverse_iterator i = Values.rbegin();
            if (i != Values.rend())
            {
                for (std::list<double>::iterator j = (*i).begin();j != (*i).end();j++)
                {
                    if (first)
                    {
                        first = false;
                        zMinValue = *j;
                    }
                    else if (zMinValue > *j)
                        zMinValue = *j;
                }
            }
        }
        if (MaxAuto)
        {
            bool first = true;
            std::list<double> total;
            std::list<bool>::iterator e = Enabled.begin();
            {
                for (std::list<std::list<double> >::iterator i = Values.begin();i != Values.end();i++)
                {
                    std::list<double>::iterator k = total.begin();
                    if (e == Enabled.end() || *e)
                    {
                        for (std::list<double>::iterator j = (*i).begin();j != (*i).end();j++)
                        {
                            if (k == total.end())
                            {
                                total.insert(total.end(), *j);
                                k = total.end();
                            }
                            else
                            {
                                *k += *j;
                                k++;
                            }
                        }
                    }
                    if (e != Enabled.end())
                        e++;
                }
            }
            for (std::list<double>::iterator i = total.begin();i != total.end();i++)
            {
                if (first)
                {
                    first = false;
                    zMaxValue = *i;
                }
                else if (zMaxValue < *i)
                    zMaxValue = *i;
            }
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

    std::list<QPointArray> Points;
    int cp = 0;
    int samples = countSamples();
    int zeroy = int(rect.height() - 2 - ( -zMinValue / (zMaxValue - zMinValue) * (rect.height() - 4)));
    if (samples > 1)
    {
        const QWMatrix &mtx = p->worldMatrix();
        p->setClipRect(int(mtx.dx() + 2), int(mtx.dy() + 2), rect.width() - 3, rect.height() - 3);
        if (Zooming)
            p->drawText(2, 2, rect.width() - 4, rect.height() - 4,
                        AlignLeft | AlignTop, tr("Zoom"));
        std::list<bool>::reverse_iterator e = Enabled.rbegin();
        for (std::list<std::list<double> >::reverse_iterator i = Values.rbegin();i != Values.rend();i++)
        {
            if (e == Enabled.rend() || *e)
            {
                std::list<double> &val = *i;
                int count = 0;
                int skip = SkipSamples;
                QPointArray a(samples + 10);
                int x = rect.width() - 2;
                for (std::list<double>::reverse_iterator j = val.rbegin();j != val.rend() && x >=
                        2;
                        j++)
                {
                    if (skip > 0)
                        skip--;
                    else
                    {
                        int val = int(rect.height() - 2 - ((*j - zMinValue) / (zMaxValue - zMinValue) * (rect.height() - 4)));
                        x = rect.width() - 2 - count * (rect.width() - 4) / (samples - 1);
                        a.setPoint(count, x, val);
                        count++;
                        if (count >= samples)
                            break;
                    }
                }
                a.resize(count*2);
                Points.insert(Points.end(), a);
            }
            cp++;
            if (e != Enabled.rend())
                e++;
        }
    }

    std::map<int, int> Bottom;
    std::list<bool>::reverse_iterator e = Enabled.rbegin();
    for (std::list<QPointArray>::iterator i = Points.begin();i != Points.end();)
    {
        while (e != Enabled.rend() && !*e)
        {
            cp--;
            e++;
        }
        if (e != Enabled.rend())
            e++;
        cp--;

        QPointArray a = *i;
        int lx = 0;
        int lb = 0;
        for (unsigned int j = 0;j < a.size() / 2;j++)
        {
            int x, y;
            a.point(j, &x, &y);
            if (Bottom.find(x) == Bottom.end())
                Bottom[x] = 0;
            if (lx != x)
                lb = Bottom[x];
            a.setPoint(a.size() - 1 - j, x, zeroy - lb);
            y -= lb;
            a.setPoint(j, x, y);
            Bottom[x] = zeroy - y;
            lx = x;
        }

        p->save();
        QBrush brush(toChartBrush(cp));
        p->setBrush(brush.color());
        p->drawPolygon(a);
        if (brush.style() != QBrush::SolidPattern)
        {
            p->setBrush(QBrush(Qt::white, brush.style()));
            p->drawPolygon(a);
        }
        p->restore();
        i++;
    }
}

toBarChart::toBarChart (toBarChart *chart, QWidget *parent, const char *name, WFlags f)
        : toLineChart(chart, parent, name, f)
{}

toLineChart *toBarChart::openCopy(QWidget *parent)
{
    toBarChart *newWin = new toBarChart(this,
                                        parent ? parent : toMainWidget()->workspace(),
                                        NULL, parent ? 0 : WDestructiveClose);
    if (!parent)
    {
        newWin->show();
        newWin->raise();
        newWin->setFocus();

        toMainWidget()->windowsMenu();

#if QT_VERSION < 0x030100
        // This is a really ugly workaround for a Qt layout bug
        QWidget *tmp = NULL;
        QWidget *tmp2 = NULL;

#if QT_VERSION < 0x030200

        for (unsigned int i = 0;i < toMainWidget()->workspace()->windowList().count();i++)
        {
            QWidget *widget = toMainWidget()->workspace()->windowList().at(i);
#else

        for (unsigned int i = 0;i < toMainWidget()->workspace()->windowList(QWorkspace::CreationOrder).count();i++)
        {
            QWidget *widget = toMainWidget()->workspace()->windowList(QWorkspace::CreationOrder).at(i);
#endif

            if (newWin != widget)
                tmp2 = widget;
            else
                tmp = newWin;
            if (tmp2 && tmp)
                break;
        }
        if (tmp2 && tmp)
        {
            tmp2->setFocus();
            tmp->setFocus();
        }
#endif

    }
    return newWin;
}

