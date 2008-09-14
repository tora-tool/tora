
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TO_DATATYPE
#define TO_DATATYPE

#include "config.h"

#include <list>
#include <QWidget>

#include "toextract.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class toConnection;

/**
 * Widget used to represent a datatype selection for a column.
 *
 */
class toDatatype : public QWidget
{
    Q_OBJECT;

    std::list<toExtract::datatype> Datatypes;

    QComboBox *Type;
    QLabel    *LeftParenthesis;
    QSpinBox  *Size;
    QLabel    *Comma;
    QSpinBox  *Precision;
    QLabel    *RightParenthesis;
    QLineEdit *Custom;
    bool       PreferCustom;

    void setupLabels();
    void setup(toConnection &conn);

public:
    /**
     * Create the widget and specify connection and the datatype.
     *
     * @param conn Connection to specify datatype for.
     * @param def Initial datatype to use.
     * @param parent The parent widget.
     * @param name Name of widget.
     */
    toDatatype(toConnection &conn,
               const QString &def,
               QWidget *parent,
               const char *name = NULL);

    /**
     * Create the widget and specify connection.
     *
     * @param conn Connection to specify datatype for.
     * @param parent The parent widget.
     * @param name Name of widget.
     */
    toDatatype(toConnection &conn, QWidget *parent, const char *name = NULL);

    /**
     * Get a string representation of the type.
     */
    QString type() const;

public slots:
    /**
     * Change the type.
     *
     * @param type The entire type specification (Including posible
     * size and precision)
     */
    void setType(const QString &type);

    /**
     * Indicate if you would prefer to enter a custom datatype or if
     * posible use the simpler controls.
     *
     * @param prefer If true always use custom controls.
     */
    void setCustom(bool prefer);

private slots:
    void changeType(int);
};

#endif
