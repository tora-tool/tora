/****************************************************************************
** Form implementation generated from reading ui file 'tosyntaxsetup.ui'
**
** Created: Sat Jan 20 20:47:42 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
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
    QToolTip::add(  KeywordUpper, tr( "Convert all keywords to uppercase when displaying syntaxhighlighted text." ) );

    PushButton1 = new QPushButton( Options, "PushButton1" );
    PushButton1->setGeometry( QRect( 90, 75, 70, 32 ) ); 
    PushButton1->setText( tr( "&Choose" ) );

    CodeExample = new QLabel( Options, "CodeExample" );
    CodeExample->setGeometry( QRect( 10, 80, 80, 16 ) ); 
    CodeExample->setText( tr( "Code Font" ) );
    QToolTip::add(  CodeExample, tr( "The font to display when editing SQL." ) );

    GroupBox2 = new QGroupBox( this, "GroupBox2" );
    GroupBox2->setGeometry( QRect( 10, 10, 200, 220 ) ); 
    GroupBox2->setTitle( tr( "Sy&ntax Components" ) );

    SelectColor = new QPushButton( GroupBox2, "SelectColor" );
    SelectColor->setGeometry( QRect( 100, 180, 90, 32 ) ); 
    SelectColor->setText( tr( "&Pick" ) );

    ExampleColor = new QLabel( GroupBox2, "ExampleColor" );
    ExampleColor->setGeometry( QRect( 10, 190, 80, 16 ) ); 
    ExampleColor->setText( tr( "" ) );
    QToolTip::add(  ExampleColor, tr( "Example of current color." ) );

    SyntaxComponent = new QListBox( GroupBox2, "SyntaxComponent" );
    SyntaxComponent->setGeometry( QRect( 10, 20, 180, 150 ) ); 
    QToolTip::add(  SyntaxComponent, tr( "Part of syntax to change color." ) );

    Example = new toHighlightedText(this,"Example");
    Example->setGeometry( QRect( 10, 240, 380, 150 ) );

    // signals and slots connections
    connect( PushButton1, SIGNAL( clicked() ), this, SLOT( selectFont(void) ) );
    connect( SelectColor, SIGNAL( clicked() ), this, SLOT( selectColor(void) ) );
    connect( KeywordUpper, SIGNAL( toggled(bool) ), this, SLOT( changeUpper(bool) ) );
    connect( SyntaxHighlighting, SIGNAL( toggled(bool) ), this, SLOT( changeHighlight(bool) ) );
    connect( SyntaxComponent, SIGNAL( selectionChanged(QListBoxItem*) ), this, SLOT( changeLine(QListBoxItem *) ) );

    // tab order
    setTabOrder( SyntaxComponent, SelectColor );
    setTabOrder( SelectColor, SyntaxHighlighting );
    setTabOrder( SyntaxHighlighting, KeywordUpper );
    setTabOrder( KeywordUpper, PushButton1 );
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

void toSyntaxSetupUI::changeUpper(bool)
{
    qWarning( "toSyntaxSetupUI::changeUpper(void): Not implemented yet!" );
}

void toSyntaxSetupUI::changeHighlight(bool)
{
    qWarning( "toSyntaxSetupUI::changeHighlight(void): Not implemented yet!" );
}

