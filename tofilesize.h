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

#ifndef __TOFILESIZE_H
#define __TOFILESIZE_H

#include <qbuttongroup.h>
#include <qwidget.h>

class QRadioButton;
class QSpinBox;

/**
 * A widget to get a size value for Oracle objects.
 */
class toFilesize : public QButtonGroup {
  Q_OBJECT

  /**
   * @internal
   * Value of widget
   */
  QSpinBox *Value;
  /**
   * @internal
   * MB unit indicator button
   */
  QRadioButton *MBSize;
  /**
   * @internal
   * KB unit indicator button
   */
  QRadioButton *KBSize;
  /**
   * Setup this widget
   */
  void setup(void);
private slots:
  /**
   * @internal
   * Change the unit type callback.
   */
  void changeType(bool);
  /**
   * @internal
   * Size value changed callback.
   */ 
  void changedSize(void)
  { emit valueChanged(); }
signals:
  /**
   * Emitted when value was changed in the widget.
   */
  void valueChanged(void); 
public:
  /**
   * Create a filesize widget.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toFilesize(QWidget* parent=0,const char* name=0);
  /**
   * Create a filesize widget.
   * @param title Title of size selection frame.
   * @param parent Parent widget.
   * @param name Name of widget.
   */
  toFilesize(const QString &title,QWidget* parent=0,const char* name=0);
  /**
   * Set the value of the widget in KB.
   * @param sizeInKB Size in KB.
   */
  void setValue(int sizeInKB);
  /**
   * Get value in KB.
   * @return Size selected in KB.
   */
  int value(void);
  /**
   * Get an oracle size specification string.
   * @return String describing size in valid Oracle format.
   */
  QString sizeString(void);
};

#endif
