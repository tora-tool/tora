//***************************************************************************
/*
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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include "tosyntaxsetup.ui.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include "tohighlightedtext.h"

/* 
 *  Constructs a toSyntaxSetupUI which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
toSyntaxSetupUI::toSyntaxSetupUI( QWidget* parent,  const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "toSyntaxSetupUI" );
    resize( 400, 401 ); 
    setCaption( tr( "Form1" ) );

    Options = new QGroupBox( this, "Options" );
    Options->setGeometry( QRect( 220, 10, 170, 220 ) ); 
    Options->setTitle( tr( "Options" ) );

    SyntaxHighlighting = new QCheckBox( Options, "SyntaxHighlighting" );
    SyntaxHighlighting->setGeometry( QRect( 10, 20, 150, 19 ) ); 
    SyntaxHighlighting->setText( tr( "S&yntax Highlighting" ) );
    QToolTip::add(  SyntaxHighlighting, tr( "Enable syntax highlighting, or use simple text editor" ) );

    KeywordUpper = new QCheckBox( Options, "KeywordUpper" );
    KeywordUpper->setGeometry( QRect( 10, 50, 150, 19 ) ); 
    KeywordUpper->setText( tr( "&Keyword Upper" ) );
    QToolTip::add(  KeywordUpper, tr( "Convert all keywords to uppercase when displaying syntaxhighlighted text. \n"
				      "Only available for monospaced fonts." ) );

    PushButton1 = new QPushButton( Options, "PushButton1" );
    PushButton1->setGeometry( QRect( 90, 75, 70, 32 ) ); 
    PushButton1->setText( tr( "&Choose" ) );

    CodeExample = new QLabel( Options, "CodeExample" );
    CodeExample->setGeometry( QRect( 10, 80, 80, 16 ) ); 
    CodeExample->setText( tr( "Code Font" ) );
    QToolTip::add(  CodeExample, tr( "The font to display when editing SQL." ) );

    PushButton2 = new QPushButton( Options, "PushButton2" );
    PushButton2->setGeometry( QRect( 90, 105, 70, 32 ) ); 
    PushButton2->setText( tr( "&Choose" ) );

    ResultExample = new QLabel( Options, "CodeExample" );
    ResultExample->setGeometry( QRect( 10, 110, 80, 16 ) ); 
    ResultExample->setText( tr( "List Font" ) );
    QToolTip::add(  CodeExample, tr( "The font to display in result lists." ) );

    GroupBox2 = new QGroupBox( this, "GroupBox2" );
    GroupBox2->setGeometry( QRect( 10, 10, 200, 220 ) ); 
    GroupBox2->setTitle( tr( "Sy&ntax Components" ) );

    SelectColor = new QPushButton( GroupBox2, "SelectColor" );
    SelectColor->setGeometry( QRect( 100, 180, 90, 32 ) ); 
    SelectColor->setText( tr( "&Pick" ) );

    ExampleColor = new QLabel( GroupBox2, "ExampleColor" );
    ExampleColor->setGeometry( QRect( 10, 190, 80, 16 ) ); 
    QToolTip::add(  ExampleColor, tr( "Example of current color." ) );

    SyntaxComponent = new QListBox( GroupBox2, "SyntaxComponent" );
    SyntaxComponent->setGeometry( QRect( 10, 20, 180, 150 ) ); 
    QToolTip::add(  SyntaxComponent, tr( "Part of syntax to change color." ) );

    Example = new toHighlightedText(this,"Example");
    Example->setGeometry( QRect( 10, 240, 380, 150 ) );

    // signals and slots connections
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( selectFont(void) ) );
    connect( PushButton2, SIGNAL( clicked() ), this, SLOT( selectResultFont(void) ) );
    connect( SelectColor, SIGNAL( clicked() ), this, SLOT( selectColor(void) ) );
    connect( KeywordUpper, SIGNAL( toggled(bool) ), this, SLOT( changeUpper(bool) ) );
    connect( SyntaxHighlighting, SIGNAL( toggled(bool) ), this, SLOT( changeHighlight(bool) ) );
    connect( SyntaxComponent, SIGNAL( selectionChanged(QListBoxItem*) ), this, SLOT( changeLine(QListBoxItem *) ) );

    // tab order
    setTabOrder( SyntaxComponent, SelectColor );
    setTabOrder( SelectColor, SyntaxHighlighting );
    setTabOrder( SyntaxHighlighting, KeywordUpper );
    setTabOrder( KeywordUpper, PushButton1 );
    setTabOrder( PushButton1, PushButton2 );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
toSyntaxSetupUI::~toSyntaxSetupUI()
{
    // no need to delete child widgets, Qt does it all for us
}

void toSyntaxSetupUI::changeLine(QListBoxItem *)
{
    qWarning( "toSyntaxSetupUI::changeLine(QListBoxItem *): Not implemented yet!" );
}

void toSyntaxSetupUI::selectColor(void)
{
    qWarning( "toSyntaxSetupUI::selectColor(void): Not implemented yet!" );
}

void toSyntaxSetupUI::selectFont(void)
{
    qWarning( "toSyntaxSetupUI::selectFont(void): Not implemented yet!" );
}

void toSyntaxSetupUI::selectResultFont(void)
{
    qWarning( "toSyntaxSetupUI::selectResultFont(void): Not implemented yet!" );
}

void toSyntaxSetupUI::changeUpper(bool)
{
    qWarning( "toSyntaxSetupUI::changeUpper(void): Not implemented yet!" );
}

void toSyntaxSetupUI::changeHighlight(bool)
{
    qWarning( "toSyntaxSetupUI::changeHighlight(void): Not implemented yet!" );
}

