// This defines the specialisation of QListBox that handles the Scintilla
// double-click callback.
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


#include <qmap.h>
#include <qpixmap.h>
#include <qstring.h>

#include "Platform.h"


class SciListBox;


class ListBoxQt : public ListBox
{
public:
    ListBoxQt();

    CallBackAction cb_action;
    void *cb_data;

    virtual void SetFont(Font &font);
    virtual void Create(Window &parent, int, Point, int, bool unicodeMode);
    virtual void SetAverageCharWidth(int);
    virtual void SetVisibleRows(int);
    virtual int GetVisibleRows() const;
    virtual PRectangle GetDesiredRect();
    virtual int CaretFromEdge();
    virtual void Clear();
    virtual void Append(char *s, int type = -1);
    virtual int Length();
    virtual void Select(int n);
    virtual int GetSelection();
    virtual int Find(const char *prefix);
    virtual void GetValue(int n, char *value, int len);
    virtual void Sort();
    virtual void RegisterImage(int type, const char *xpm_data);
    virtual void ClearRegisteredImages();
    virtual void SetDoubleClickAction(CallBackAction action, void *data);
    virtual void SetList(const char *list, char separator, char typesep);

    static QString backdoor;

private:
    SciListBox *slb;
    int visible_rows;
    bool utf8;

    typedef QMap<int, QPixmap> xpmMap;
    xpmMap xset;
};
