/****************************************************************************
** Form interface generated from reading ui file 'tosyntaxsetup.ui'
**
** Created: Sat Jan 20 20:47:42 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
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
