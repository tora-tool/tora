
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

#ifndef __TOSYNTAXSETUP_H__
#define __TOSYNTAXSETUP_H__


#include "ui_tosyntaxsetupui.h"
#include "core/totool.h"

#include <map>

#include <QtCore/QString>
#include <QtCore/QMetaEnum>
#include <QtGui/QFont>
#include <QtGui/QColor>

class QListWidgetItem;
class toSyntaxAnalyzer;

class toStyle : public QObject
{
	Q_OBJECT;
public:
	toStyle() // make QMap happy
	: QObject(NULL)
	{}

	toStyle(const toStyle &other)
	: QObject(NULL)
	, FGColor(other.FGColor)
	, BGColor(other.BGColor)
	, Font(other.Font)
	{}

	toStyle& operator =(const toStyle &other)
	{
		FGColor = other.FGColor;
		BGColor = other.BGColor;
		Font =other.Font;
		return *this;
	}

	toStyle(QColor const& fg, QColor const& bg, QFont const& fo)
	: QObject(NULL)
	, FGColor(fg)
	, BGColor(bg)
	, Font(fo)
	{}

	QColor FGColor;
	QColor BGColor;
	QFont Font;
};

class toSyntaxSetup : public QWidget
    , public Ui::toSyntaxSetupUI
    , public toSettingTab
{

    Q_OBJECT;

    QString ListFontName;
    QListWidgetItem *Current;
    QMetaEnum ColorsEnum;
    QMap<int, toStyle> Styles;  // enum toSyntaxAnalyzer::WordClass => toStyle
    toSyntaxAnalyzer* Analyzer;
public:
    toSyntaxSetup(QWidget *parent = 0, const char *name = 0, Qt::WFlags fl = 0);
    virtual void saveSetting(void);

public slots:
    virtual void changeLine(QListWidgetItem *);
    virtual void selectColor(void);
    virtual void selectFont(void);
    virtual void selectText(void);
    virtual void selectResultFont(void);
    virtual void changeUpper(bool val)
    {
        //TODO Example->setKeywordUpper(val);
        Example->update();
    }
    virtual void changeHighlight(bool val)
    {
        //TODO Example->setSyntaxColoring(val);
        Example->update();
    }
    void openEditorShortcutsDialog();
private:
    int wordClass() const;         // returns enum toSyntaxAnalyzer::wordClass
    void checkFixedWidth(const QFont &fnt);
};

#endif
