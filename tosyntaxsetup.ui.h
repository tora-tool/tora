/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/

#ifndef TOSYNTAXSETUPUI_H
#define TOSYNTAXSETUPUI_H

#include <qvariant.h>
#include <qwidget.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QListBox;
class QListBoxItem;
class QPushButton;
class toHighlightedText;

class toSyntaxSetupUI : public QWidget
{ 
    Q_OBJECT

public:
    toSyntaxSetupUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~toSyntaxSetupUI();

    QGroupBox* Options;
    QCheckBox* SyntaxHighlighting;
    QCheckBox* KeywordUpper;
    QPushButton* PushButton1;
    QLabel* CodeExample;
    QGroupBox* GroupBox2;
    QPushButton* SelectColor;
    QLabel* ExampleColor;
    QListBox* SyntaxComponent;
    toHighlightedText* Example;

public slots:
    virtual void changeLine(QListBoxItem *);
    virtual void selectColor(void);
    virtual void selectFont(void);
    virtual void changeUpper(bool);
    virtual void changeHighlight(bool);

};

#endif // TOSYNTAXSETUPUI_H
