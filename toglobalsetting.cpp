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

TO_NAMESPACE;

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfontdialog.h>
#include <qcombobox.h>
#include <qvalidator.h>
#include <qfiledialog.h>

#include "toconf.h"
#include "totool.h"
#include "tomain.h"
#include "toglobalsetting.h"

#include "toglobalsetting.moc"

toGlobalSettings::toGlobalSettings( QWidget* parent,  const char* name, bool modal, WFlags fl )
  : QFrame( parent, name, modal, fl )
{
  if (!name)
    setName( "toGlobalSettings" );
  setCaption( tr( "Global settings"  ) );

  GroupBox5 = new QGroupBox( this, "GroupBox5" );
  GroupBox5->setGeometry( QRect( 10, 265, 380, 135 ) ); 
  GroupBox5->setTitle( tr( "Database settings"  ) );

  TextLabel1_3 = new QLabel( GroupBox5, "TextLabel1_3" );
  TextLabel1_3->setGeometry( QRect( 10, 20, 140, 20 ) ); 
  TextLabel1_3->setText( tr( "&Explain plan table"  ) );
  QToolTip::add(  TextLabel1_3, tr( "What table to use as the destination for the EXPLAIN TABLE command" ) );

  TextLabel1_3_2 = new QLabel( GroupBox5, "TextLabel1_3_2" );
  TextLabel1_3_2->setGeometry( QRect( 10, 60, 140, 20 ) ); 
  TextLabel1_3_2->setText( tr( "C&heckpoint name"  ) );
  QToolTip::add(  TextLabel1_3_2, tr( "Checkpoint name to use when TOra needs to make temporary changes to the database (Explain plan is one example)" ) );

  TextLabel7 = new QLabel( GroupBox5, "TextLabel7" );
  TextLabel7->setGeometry( QRect( 10, 100, 140, 20 ) ); 
  TextLabel7->setText( tr( "&Default date format"  ) );
  QToolTip::add(  TextLabel7, tr( "Set this dateformat after connection to the database." ) );

  PlanCheckpoint = new QLineEdit( GroupBox5, "PlanCheckpoint" );
  PlanCheckpoint->setGeometry( QRect( 170, 60, 200, 23 ) ); 
  PlanCheckpoint->setText(toTool::globalConfig(CONF_PLAN_CHECKPOINT,
					       DEFAULT_PLAN_CHECKPOINT));

  DefaultDate = new QLineEdit( GroupBox5, "DefaultDate" );
  DefaultDate->setGeometry( QRect( 170, 100, 200, 23 ) ); 
  DefaultDate->setText(toTool::globalConfig(CONF_DATE_FORMAT,
					    DEFAULT_DATE_FORMAT));

  PlanTable = new QLineEdit( GroupBox5, "PlanTable" );
  PlanTable->setGeometry( QRect( 170, 20, 200, 23 ) ); 
  PlanTable->setText(toTool::globalConfig(CONF_PLAN_TABLE,
					  DEFAULT_PLAN_TABLE));

  GroupBox1 = new QGroupBox( this, "GroupBox1" );
  GroupBox1->setGeometry( QRect( 10, 10, 210, 175 ) ); 
  GroupBox1->setTitle( tr( "Query settings"  ) );

  TextLabel1_2_3 = new QLabel( GroupBox1, "TextLabel1_2_3" );
  TextLabel1_2_3->setGeometry( QRect( 9, 140, 120, 20 ) ); 
  TextLabel1_2_3->setText( tr( "&Number of columns"  ) );
  QToolTip::add(  TextLabel1_2_3, tr( "Maximum number of columns in the result of a query" ) );

  TextLabel1_2_2 = new QLabel( GroupBox1, "TextLabel1_2_2" );
  TextLabel1_2_2->setGeometry( QRect( 10, 60, 120, 20 ) ); 
  TextLabel1_2_2->setText( tr( "&Max column size"  ) );
  QToolTip::add(  TextLabel1_2_2, tr( "Max length of data fetched in one column" ) );

  TextLabel1_2 = new QLabel( GroupBox1, "TextLabel1_2" );
  TextLabel1_2->setGeometry( QRect( 10, 100, 120, 20 ) ); 
  TextLabel1_2->setText( tr( "&Column size display"  ) );
  QToolTip::add(  TextLabel1_2, tr( "Max width of a column in the result table from a query" ) );

  TextLabel1 = new QLabel( GroupBox1, "TextLabel1" );
  TextLabel1->setGeometry( QRect( 10, 20, 120, 20 ) ); 
  TextLabel1->setText( tr( "&Initial fetch"  ) );
  QToolTip::add(  TextLabel1, tr( "Number of rows to initially fetch when executing a query" ) );

  MaxNumber = new QLineEdit( GroupBox1, "MaxNumber" );
  MaxNumber->setGeometry( QRect( 140, 20, 60, 23 ) ); 
  MaxNumber->setText(toTool::globalConfig(CONF_MAX_NUMBER,
					  DEFAULT_MAX_NUMBER));
  MaxNumber->setAlignment( int( QLineEdit::AlignRight ) );
  QIntValidator *valid=new QIntValidator(MaxNumber);
  MaxNumber->setValidator(valid);

  MaxColSize = new QLineEdit( GroupBox1, "MaxColSize" );
  MaxColSize->setGeometry( QRect( 140, 60, 60, 23 ) ); 
  MaxColSize->setText(toTool::globalConfig(CONF_MAX_COL_SIZE,
					   DEFAULT_MAX_COL_SIZE));
  MaxColSize->setAlignment( int( QLineEdit::AlignRight ) );
  valid=new QIntValidator(MaxColSize);
  MaxColSize->setValidator(valid);

  MaxColDisp = new QLineEdit( GroupBox1, "MaxColDisp" );
  MaxColDisp->setGeometry( QRect( 140, 100, 60, 23 ) ); 
  MaxColDisp->setText(toTool::globalConfig(CONF_MAX_COL_DISP,
					   DEFAULT_MAX_COL_DISP));
  MaxColDisp->setAlignment( int( QLineEdit::AlignRight ) );
  valid=new QIntValidator(MaxColDisp);
  MaxColDisp->setValidator(valid);

  MaxColNum = new QLineEdit( GroupBox1, "MaxColNum" );
  MaxColNum->setGeometry( QRect( 140, 140, 60, 23 ) ); 
  MaxColNum->setText(toTool::globalConfig(CONF_MAX_COL_NUM,
					  DEFAULT_MAX_COL_NUM));
  MaxColNum->setAlignment( int( QLineEdit::AlignRight ) );
  valid=new QIntValidator(MaxColNum);
  MaxColNum->setValidator(valid);

  GroupBox6 = new QGroupBox( this, "GroupBox6" );
  GroupBox6->setGeometry( QRect( 230, 10, 160, 175 ) ); 
  GroupBox6->setTitle( tr( "Options"  ) );

  SavePassword = new QCheckBox( GroupBox6, "SavePassword" );
  SavePassword->setGeometry( QRect( 10, 20, 130, 20 ) ); 
  SavePassword->setText( tr( "&Save last password"  ) );
  QToolTip::add(  SavePassword, tr( "Save the last password used when connecting" ) );
  if (!toTool::globalConfig(CONF_SAVE_PWD,"").isEmpty())
    SavePassword->setChecked(true);

  QLabel *label=new QLabel(tr("&Refresh"),GroupBox6);
  label->setGeometry(QRect(10,60,45,20));
  QToolTip::add(label,tr("Default refresh time"));

  RefreshList = toRefreshCreate(GroupBox6);
  RefreshList->setGeometry(QRect(60,60,90,20));
  label->setBuddy(RefreshList);

#ifdef ENABLE_STYLE
  label=new QLabel(tr("S&tyle"),GroupBox6);
  label->setGeometry(QRect(10,100,45,20));
  QToolTip::add(label,tr("Select the theme for the controls of the window"));

  StyleList = new QComboBox(GroupBox6);
  StyleList->setGeometry(QRect(60,100,90,20));
  StyleList->insertItem("CDE");
  StyleList->insertItem("Motif");
  StyleList->insertItem("Motif Plus");
  StyleList->insertItem("Platinum");
  StyleList->insertItem("SGI");
  StyleList->insertItem("Windows");
  QString str=toGetSessionType();
  if (str=="CDE")
    StyleList->setCurrentItem(0);
  else if (str=="Motif")
    StyleList->setCurrentItem(1);
  else if (str=="Motif Plus")
    StyleList->setCurrentItem(2);
  else if (str=="Platinum")
    StyleList->setCurrentItem(3);
  else if (str=="SGI")
    StyleList->setCurrentItem(4);
  else if (str=="Windows")
    StyleList->setCurrentItem(5);
  label->setBuddy(StyleList);
#endif

  if (!toMonolithic()) {
    GroupBox7 = new QGroupBox( this, "GroupBox5" );
    GroupBox7->setGeometry( QRect( 10, 195, 380, 60 ) ); 
    GroupBox7->setTitle( tr( "Plugin directory"  ) );

    PluginDirectory = new QLineEdit( GroupBox7, "PluginDirectory" );
    PluginDirectory->setGeometry( QRect( 10, 20, 270, 23 ) ); 
    PluginDirectory->setText(toTool::globalConfig(CONF_PLUGIN_DIR,
						  DEFAULT_PLUGIN_DIR));

    SelectDir = new QPushButton( GroupBox7, "FileChoose" );
    SelectDir->setGeometry( QRect( 290, 14, 80, 32 ) ); 
    SelectDir->setText( tr( "&Browse"  ) );
    connect(SelectDir,SIGNAL(clicked(void)),this,SLOT(chooseFile(void)));
  }

  // tab order
  setTabOrder( MaxNumber, MaxColSize );
  setTabOrder( MaxColSize, MaxColDisp );
  setTabOrder( MaxColDisp, MaxColNum );
  if (toMonolithic())
    setTabOrder( MaxColNum, PlanTable );
  else {
    setTabOrder( MaxColNum, PluginDirectory );
    setTabOrder( PluginDirectory, SelectDir );
    setTabOrder( SelectDir, PlanTable );
  }
  setTabOrder( PlanTable, PlanCheckpoint );
  setTabOrder( PlanCheckpoint, DefaultDate );
  setTabOrder( DefaultDate, SavePassword );
  setTabOrder( SavePassword,  RefreshList );
#ifdef ENABLE_STYLE
  setTabOrder( RefreshList, StyleList );
#endif

  // buddies
  TextLabel1_3->setBuddy( PlanTable );
  TextLabel1_3_2->setBuddy( PlanCheckpoint );
  TextLabel1_2_3->setBuddy( MaxColNum );
  TextLabel1_2_2->setBuddy( MaxColSize );
  TextLabel1_2->setBuddy( MaxColDisp );
  TextLabel1->setBuddy( MaxNumber );
  TextLabel7->setBuddy( DefaultDate );
}

void toGlobalSettings::saveSetting(void)
{
  toTool::globalSetConfig(CONF_MAX_NUMBER,MaxNumber->text());
  toTool::globalSetConfig(CONF_MAX_COL_SIZE,MaxColSize->text());
  toTool::globalSetConfig(CONF_MAX_COL_DISP,MaxColDisp->text());
  toTool::globalSetConfig(CONF_MAX_COL_NUM,MaxColNum->text());
  toTool::globalSetConfig(CONF_PLAN_TABLE,PlanTable->text());
  toTool::globalSetConfig(CONF_PLAN_CHECKPOINT,PlanCheckpoint->text());
  toTool::globalSetConfig(CONF_DATE_FORMAT,DefaultDate->text());
  toTool::globalSetConfig(CONF_REFRESH,RefreshList->currentText());
#ifdef ENABLE_STYLE
  toTool::globalSetConfig(CONF_STYLE,StyleList->currentText());
  toSetSessionType(StyleList->currentText());
#endif
  if (!toMonolithic()) {
    toTool::globalSetConfig(CONF_PLUGIN_DIR,PluginDirectory->text());
  }

  if (SavePassword->isChecked())
    toTool::globalSetConfig(CONF_SAVE_PWD,"Yes");
  else
    toTool::globalSetConfig(CONF_SAVE_PWD,"");
}

void toGlobalSettings::chooseFile(void)
{
  QString str=QFileDialog::getExistingDirectory(PluginDirectory->text(),this);
  if (!str.isEmpty())
    PluginDirectory->setText(str);
}
