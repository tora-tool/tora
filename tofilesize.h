//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
