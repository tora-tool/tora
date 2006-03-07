/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "utils.h"

#include "toconnection.h"
#include "todatatype.h"
#include "toextract.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include "todatatype.moc"

toDatatype::toDatatype(toConnection &conn,
                       const QString &def,
                       QWidget *parent,
                       const char *name)
        : QHBox(parent, name)
{
    setup(conn);
    setType(def);
}

toDatatype::toDatatype(toConnection &conn,
                       QWidget *parent,
                       const char *name)
        : QHBox(parent, name)
{
    setup(conn);
    setType("VARCHAR(32)");
}

void toDatatype::setup(toConnection &conn)
{
    toExtract extractor(conn, this);
    Datatypes = extractor.datatypes();

    Type = new QComboBox(this);
    for (std::list<toExtract::datatype>::iterator i = Datatypes.begin();i != Datatypes.end();i++)
        Type->insertItem((*i).name());
    LeftParenthesis = new QLabel(tr("<B>(</B>"), this);
    LeftParenthesis->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    Size = new QSpinBox(this);
    Size->setMinValue(0);
    Comma = new QLabel(tr("<B>,</B>"), this);
    Comma->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    Precision = new QSpinBox(this);
    Precision->setMinValue(0);
    RightParenthesis = new QLabel(tr("<B>)</B>"), this);
    RightParenthesis->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
    Custom = new QLineEdit(this);
    Custom->hide();
    PreferCustom = false;

    connect(Type, SIGNAL(activated(int)), this, SLOT(changeType(int)));
}

QString toDatatype::type() const
{
    QString type;
    if (Custom->isShown())
    {
        type = Custom->text();
    }
    else
    {
        type = Type->currentText();
        bool par = false;
        if (Size->isShown())
        {
            type += "(";
            par = true;
            type += Size->text();
        }
        if (Precision->isShown())
        {
            if (!par)
            {
                type += "(";
                par = true;
            }
            else
                type += ",";
            type += Precision->text();
        }
        if (par)
            type += ")";
    }
    return type;
}

void toDatatype::setType(const QString &type)
{
    try
    { // Intentionally not using the SQL parser for this
        enum {
            Initial,
            AtType,
            AtSize,
            AtPrecision,
            AtEnd
        } state = Initial;
        int startType = -1;
        int endType = -1;
        int size = -1;
        int precision = -1;
        bool valid = !PreferCustom;
        bool endoftoken = false;
        if (valid)
        {
            for (unsigned int pos = 0;pos < type.length();pos++)
            {
                QChar c = type.at(pos);
                if (c.isSpace())
                {
                    endoftoken = true;
                    continue;
                }
                switch (state)
                {
                case Initial:
                    state = AtType;
                    startType = pos;
                    endoftoken = false;
                    break;
                case AtType:
                    if (c == '(')
                    {
                        state = AtSize;
                        endoftoken = false;
                    }
                    else if (endoftoken)
                    {
                        valid = false;
                    }
                    else
                        endType = pos;
                    break;
                case AtSize:
                    if (c.isDigit() && (!endoftoken || size == -1))
                    {
                        if (size == -1)
                            size = ((char)c) - '0';
                        else
                            size = size * 10 + ((char)c) - '0';
                        endoftoken = false;
                    }
                    else if (size == -1)
                        valid = false;
                    else if (c == ')')
                    {
                        endoftoken = false;
                        state = AtEnd;
                    }
                    else if (c == ',')
                    {
                        endoftoken = false;
                        state = AtPrecision;
                    }
                    else
                        valid = false;
                    break;
                case AtPrecision:
                    if (c.isDigit() && (!endoftoken || precision == -1))
                    {
                        if (precision == -1)
                            precision = ((char)c) - '0';
                        else
                            precision = precision * 10 + ((char)c) - '0';
                        endoftoken = false;
                    }
                    else if (precision == -1)
                        valid = false;
                    else if (c == ')')
                    {
                        endoftoken = false;
                        state = AtEnd;
                    }
                    else
                        valid = false;
                    break;
                case AtEnd:
                    valid = false;
                    break;
                }
                if (!valid)
                    break;
            }
        }
        if (valid)
        {
            if (endType == -1)
                endType = type.length();
            if (startType >= 0)
            {
                QString actualtype = type.mid(startType, endType - startType + 1).upper();
                valid = false;
                for (int i = 0;i < Type->count();i++)
                {
                    if (actualtype == Type->text(i))
                    {
                        Type->show();
                        Type->setCurrentItem(i);
                        valid = true;
                        break;
                    }
                }
                if (valid)
                {
                    valid = false;
                    for (std::list<toExtract::datatype>::iterator i = Datatypes.begin();i != Datatypes.end();i++)
                    {
                        if ((*i).name() == actualtype)
                        {
                            valid = true;
                            if ((*i).hasLength())
                            {
                                if (size > (*i).maxLength())
                                    valid = false;
                                else
                                {
                                    Size->show();
                                    Size->setMaxValue((*i).maxLength());
                                    if (size != -1)
                                        Size->setValue(size);
                                    else
                                        Size->setValue((*i).maxLength());
                                }
                            }
                            else if (size != -1)
                                valid = false;
                            else
                                Size->hide();
                            if ((*i).hasPrecision())
                            {
                                if (precision > (*i).maxPrecision())
                                    valid = false;
                                else
                                {
                                    Precision->show();
                                    Precision->setMaxValue((*i).maxPrecision());
                                    if (precision != -1)
                                        Precision->setValue(precision);
                                    else
                                        Precision->setValue((*i).maxPrecision());
                                }
                            }
                            else if (precision != -1)
                                valid = false;
                            else
                                Precision->hide();
                            break;
                        }
                    }
                }
            }
        }
        if (!valid)
        {
            Custom->setText(type);
            Custom->show();
            Precision->hide();
            Size->hide();
            Type->hide();
        }
        else
            Custom->hide();

        setupLabels();
    }
    TOCATCH
}

void toDatatype::setupLabels()
{
    bool show = Size->isShown() || Precision->isShown();
    RightParenthesis->setShown(show);
    LeftParenthesis->setShown(show);
    Comma->setShown(Size->isShown() && Precision->isShown());
}

void toDatatype::setCustom(bool prefer)
{
    PreferCustom = prefer;
    setType(type());
}

void toDatatype::changeType(int id)
{
    QString type = Type->text(id);
    for (std::list<toExtract::datatype>::iterator i = Datatypes.begin();i != Datatypes.end();i++)
    {
        if ((*i).name() == type)
        {
            if ((*i).hasLength())
            {
                Size->setShown(true);
                Size->setMaxValue((*i).maxLength());
            }
            else
                Size->setShown(false);
            if ((*i).hasPrecision())
            {
                Precision->setShown(true);
                Precision->setMaxValue((*i).maxPrecision());
            }
            else
                Precision->setShown(false);
            break;
        }
    }
    setupLabels();
}
