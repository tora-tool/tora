// This module implements the specialisation of QListBox that handles the
// Scintilla double-click callback.
//
// Copyright (c) 2007
// 	Phil Thompson <phil@river-bank.demon.co.uk>
// 
// This file is part of QScintilla.
// 
// This copy of QScintilla is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option) any
// later version.
// 
// QScintilla is supplied in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
// 
// You should have received a copy of the GNU General Public License along with
// QScintilla; see the file LICENSE.  If not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include "ListBoxQt.h"

#include <stdlib.h>

#include "SciClasses.h"


// The backdoor through which the full text of the selected item is passed.
QString ListBoxQt::backdoor;


ListBoxQt::ListBoxQt()
    : cb_action(0), cb_data(0), slb(0), visible_rows(5), utf8(false)
{
}


void ListBoxQt::SetFont(Font &font)
{
    QFont *f = reinterpret_cast<QFont *>(font.GetID());

    if (f)
        slb->setFont(*f);
}


void ListBoxQt::Create(Window &parent, int, Point, int, bool unicodeMode)
{
    utf8 = unicodeMode;

    // The parent we want is the QsciScintillaBase, not the text area.
    id = slb = new SciListBox(reinterpret_cast<QWidget *>(parent.GetID())->parentWidget(), this);
}


void ListBoxQt::SetAverageCharWidth(int)
{
    // We rely on sizeHint() for the size of the list box rather than make
    // calculations based on the average character width and the number of
    // visible rows.
}


void ListBoxQt::SetVisibleRows(int vrows)
{
    // We only pretend to implement this.
    visible_rows = vrows;
}


int ListBoxQt::GetVisibleRows() const
{
    return visible_rows;
}


PRectangle ListBoxQt::GetDesiredRect()
{
    PRectangle rc(0, 0, 100, 100);

    if (slb)
    {
        QSize sh = slb->sizeHint();

        rc.right = sh.width();
        rc.bottom = sh.height();
    }

    return rc;
}


int ListBoxQt::CaretFromEdge()
{
    int dist = 0;

    // Find the width of the biggest image.
    for (xpmMap::const_iterator it = xset.begin(); it != xset.end(); ++it)
    {
        int w = it.value().width();

        if (dist < w)
            dist = w;
    }

    if (slb)
        dist += slb->frameWidth();

    // Fudge factor - adjust if required.
    dist += 3;

    return dist;
}


void ListBoxQt::Clear()
{
    Q_ASSERT(slb);

    slb->clear();
}


void ListBoxQt::Append(char *s, int type)
{
    Q_ASSERT(slb);

    QString qs;

    if (utf8)
        qs = QString::fromUtf8(s);
    else
        qs = QString::fromLatin1(s);

    xpmMap::const_iterator it;

    if (type < 0 || (it = xset.find(type)) == xset.end())
        slb->addItem(qs);
    else
        slb->addItemPixmap(it.value(), qs);
}


int ListBoxQt::Length()
{
    Q_ASSERT(slb);

    return slb->count();
}


void ListBoxQt::Select(int n)
{
    Q_ASSERT(slb);

    slb->setCurrentRow(n);
}


int ListBoxQt::GetSelection()
{
    Q_ASSERT(slb);

    return slb->currentRow();
}


int ListBoxQt::Find(const char *prefix)
{
    Q_ASSERT(slb);

    return slb->find(prefix);
}


void ListBoxQt::GetValue(int n, char *value, int len)
{
    Q_ASSERT(slb);

    backdoor = slb->text(n);

    if (backdoor.isEmpty() || len <= 0)
        value[0] = '\0';
    else
    {
        const char *s;
        int slen;

        QByteArray bytes;

        if (utf8)
            bytes = backdoor.toUtf8();
        else
            bytes = backdoor.toLatin1();

        s = bytes.data();
        slen = bytes.length();

        // Copy everything up to the first space.  We assume everything
        // afterwards is additional descriptive information which shouldn't
        // be inserted into the text.
        while (slen-- && len-- && *s != ' ')
            *value++ = *s++;

        *value = '\0';
    }
}


void ListBoxQt::Sort()
{
    Q_ASSERT(slb);

    slb->sortItems();
}


void ListBoxQt::RegisterImage(int type,const char *xpm_data)
{
    xset.insert(type, *reinterpret_cast<const QPixmap *>(xpm_data));
}


void ListBoxQt::ClearRegisteredImages()
{
    xset.clear();
}


void ListBoxQt::SetDoubleClickAction(CallBackAction action, void *data)
{
    cb_action = action;
    cb_data = data;
}


void ListBoxQt::SetList(const char *list, char separator, char typesep)
{
    char *words;

    Clear();

    if ((words = qstrdup(list)) != NULL)
    {
        char *startword = words;
        char *numword = NULL;

        for (int i = 0; words[i] != '\0'; i++)
        {
            if (words[i] == separator)
            {
                words[i] = '\0';

                if (numword)
                    *numword = '\0';

                Append(startword, numword ? atoi(numword + 1) : -1);

                startword = words + i + 1;
                numword = NULL;
            }
            else if (words[i] == typesep)
            {
                numword = words + i;
            }
        }

        if (startword)
        {
            if (numword)
                *numword = '\0';

            Append(startword, numword ? atoi(numword + 1) : -1);
        }

        delete[] words;
    }
}


// The ListBox methods that need to be implemented explicitly.

ListBox::ListBox()
{
}


ListBox::~ListBox()
{
}


ListBox *ListBox::Allocate()
{
    return new ListBoxQt();
}
