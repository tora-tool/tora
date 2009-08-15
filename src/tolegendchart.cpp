
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#include "tolegendchart.h"

#include <qpainter.h>
//Added by qt3to4:
#include <QPaintEvent>


toLegendChart::toLegendChart(int columns, QWidget *parent, const char *name, Qt::WFlags f)
        : QWidget(parent, f)
{
    setObjectName(name);
    Columns = columns < 1 ? 1 : columns;
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
}

void toLegendChart::addLabel(const QString &label)
{
    Labels.insert(Labels.end(), label);
    sizeHint();
    update();
}

void toLegendChart::setLabels(std::list<QString> &labels)
{
    Labels = labels;
    sizeHint();
    update();
}

#define FONT_ALIGN Qt::AlignLeft|Qt::AlignTop|Qt::TextExpandTabs

std::list<int> toLegendChart::sizeHint(int &height, int &items)
{
    QFontMetrics fm = fontMetrics();

    int count = 0;
    {
        for (std::list<QString>::iterator i = Labels.begin();i != Labels.end();i++)
            if (!(*i).isEmpty() && *i != " ")
                count++;
    }

    items = (count + Columns - 1) / Columns;

    height = 0;
    int width = 0;
    int cheight = 0;
    int cur = 0;
    std::list<int> ret;

    for (std::list<QString>::iterator i = Labels.begin();i != Labels.end();i++)
    {
        if (!(*i).isEmpty() && *i != " ")
        {
            if (cur == items)
            {
                ret.insert(ret.end(), width);

                if (cheight > height)
                    height = cheight;
                cheight = 0;
                width = 0;
                cur = 0;
            }
            QRect bounds = fm.boundingRect(0, 0, 10000, 10000, FONT_ALIGN, *i);
            if (width < bounds.width())
                width = bounds.width();
            cheight += bounds.height();
            cur++;
        }
    }
    if (width > 0)
    {
        ret.insert(ret.end(), width);
    }
    if (cheight > height)
        height = cheight;
    return ret;
}

QSize toLegendChart::sizeHint(void)
{
    QFontMetrics fm = fontMetrics();

    int height, items;

    std::list<int> widths = sizeHint(height, items);

    height += 8;
    int width = 8;
    for (std::list<int>::iterator i = widths.begin();i != widths.end();i++)
        width += (*i) + 12;

    setFixedWidth(width);

    if (!Title.isEmpty())
    {
        QRect bounds = fm.boundingRect(0, 0, width, 10000, FONT_ALIGN, Title);
        height += bounds.height() + 2;
    }

    return QSize(width, height);
}

void toLegendChart::paintEvent(QPaintEvent *)
{
    int height, items;
    std::list<int> widths = sizeHint(height, items);

    int width = 4;
    {
        for (std::list<int>::iterator i = widths.begin();i != widths.end();i++)
            width += (*i) + 12;
    }

    QPainter p(this);
    QFontMetrics fm = fontMetrics();

    if (!Title.isEmpty())
    {
        p.save();
        QFont f = p.font();
        f.setBold(true);
        p.setFont(f);
        QRect bounds = fm.boundingRect(0, 0,
                                       toLegendChart::width(),
                                       toLegendChart::height(), FONT_ALIGN, Title);
        p.drawText(0, 2,
                   toLegendChart::width() - 4,
                   toLegendChart::height(), Qt::AlignHCenter | Qt::AlignTop, Title);
        p.restore();
        p.translate(0, bounds.height() + 2);
    }

    int cx = 2;
    int cy = 4;
    p.save();
    p.setBrush(Qt::white);
    p.drawRect(2, 2, width, height + 4);
    p.restore();
    int cur = 0;
    std::list<int>::iterator j = widths.begin();
    int cp = 0;
    for (std::list<QString>::iterator i = Labels.begin();i != Labels.end();i++)
    {
        if (!(*i).isEmpty() && *i != " ")
        {
            if (cur == items)
            {
                cx += *j + 12;
                cy = 4;
                cur = 0;
                j++;
            }
            QRect bounds = fm.boundingRect(cx + 12, cy, 100000, 100000, FONT_ALIGN, *i);
            p.drawText(bounds, FONT_ALIGN, *i);
            p.save();
            QBrush brush(toChartBrush(cp));
            p.setBrush(brush.color());
            p.drawRect(cx + 2, cy + bounds.height() / 2 - fm.ascent() / 2, 8, fm.ascent());
            if (brush.style() != Qt::SolidPattern)
            {
                p.setBrush(QBrush(Qt::white, brush.style()));
                p.drawRect(cx + 2, cy + bounds.height() / 2 - fm.ascent() / 2, 8, fm.ascent());
            }
            p.restore();
            cy += bounds.height();
            cur++;
        }
        cp++;
    }
}
