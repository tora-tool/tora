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



#ifndef __TOGLOBALSETTINGS_H
#define __TOGLOBALSETTINGS_H

#include <qvariant.h>
#include <qframe.h>

#include "totool.h"

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;

class toGlobalSettings : public QFrame, public toSettingTab {
  Q_OBJECT

  QGroupBox *GroupBox5;
  QLabel *TextLabel1_3;
  QLabel *TextLabel1_3_2;
  QGroupBox *GroupBox1;
  QLabel *TextLabel1_2_3;
  QLabel *TextLabel1_2_2;
  QLabel *TextLabel1_2;
  QLabel *TextLabel1;
  QLabel *TextLabel7;
  QGroupBox *GroupBox6;
  QLabel *TextLabel1_3_3;

  QLineEdit *PlanCheckpoint;
  QLineEdit *PlanTable;
  QLineEdit *MaxNumber;
  QLineEdit *MaxColSize;
  QLineEdit *MaxColDisp;
  QLineEdit *MaxColNum;
  QLineEdit *DefaultDate;
  QCheckBox *SavePassword;
  QCheckBox *SyntaxHigh;
  QCheckBox *KeywordUpper;
  QString Text;
  QPushButton *SelectFont;
  QComboBox *StyleList;
  QComboBox *RefreshList;

public:
  toGlobalSettings( QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags fl = 0 );

  virtual void saveSetting(void);

public slots:
  void changeFont(void);
};

#endif
