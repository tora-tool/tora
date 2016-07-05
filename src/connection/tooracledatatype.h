
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

#pragma once

#include "core/toqvalue.h"
#include "core/tologger.h"
#include "core/utils.h"

#include "trotl.h"
#include "trotl_convertor.h"

#define MAXTOMAXLONG 30000
#define MAXLOBSHOWN 64

class toOracleClob: public toQValue::complexType
{
    public:
        toOracleClob(trotl::OciConnection &_conn)
            : toQValue::complexType()
            , data(_conn)
            , _length(0)
            , _displayData()
            , _toolTipData()
        {};
        /* virtual */
        bool isBinary() const
        {
            return false;
        }
        /* virtual */ bool isLarge() const
        {
            return true;
        }

        QString const& displayData() const throw() override;
        QString editData() const throw() override;
        QString userData() const throw() override;

        QString const& tooltipData() const throw() override
        {
            if (!_toolTipData.isNull())
                return _toolTipData;
            _toolTipData = QString("Datape: Oracle [N]CLOB\n" "Size: %1B\n").arg(getLength());
            return _toolTipData;
        }

        QString const& dataTypeName() const override
        {
            static const QString clob("CLOB");
            return clob;
        }

        QByteArray read(unsigned offset) const override;

        void write(QByteArray const &data) override {}

        virtual ~toOracleClob()
        {
            TLOG(1, toDecorator, __HERE__) << "toOracleClob DELETED:" << this << std::endl;
        }

        mutable trotl::SqlClob data;
    protected:
        oraub8 getLength() const
        {
            if (!_length)
                _length = data.length();
            return _length;
        };

        mutable oraub8 _length; // NOTE: OCILobGetLength makes one roundtrip to the server
        mutable QString _displayData;
        mutable QString _toolTipData;
        toOracleClob(toOracleClob const&);
        toOracleClob operator=(toOracleClob const&);
        //TODO copying prohibited
};
//Q_DECLARE_METATYPE(toOracleClob*)

class toOracleBlob: public toQValue::complexType
{
    public:
        toOracleBlob(trotl::OciConnection &_conn)
            : toQValue::complexType()
            , data(_conn)
            , _length(0)
            , _displayData()
            , _toolTipData()
        {};

        bool isBinary() const
        {
            return true;
        }
        bool isLarge() const
        {
            return true;
        }

        QString const& displayData() const throw() override;

        QString editData() const throw() override;

        QString userData() const throw();

        QString const& tooltipData() const throw() override
        {
            if (!_toolTipData.isNull())
                return _toolTipData;
            _toolTipData = QString("Datape: Oracle BLOB\nSize: %1B\n").arg(data.length());
            return _toolTipData;
        }

        QString const& dataTypeName() const override
        {
            static const QString blob("blob");
            return blob;
        }

        QByteArray read(unsigned offset) const override;

        void write(QByteArray const &data) override
        {
        }

        virtual ~toOracleBlob()
        {
            TLOG(1, toDecorator, __HERE__) << "toOracleBlob DELETED:" << this << std::endl;
        }

        mutable trotl::SqlBlob data;
    protected:
        oraub8 getLength() const
        {
            if (!_length)
                _length = data.length();
            return _length;
        };

        mutable oraub8 _length; // NOTE: OCILobGetLength makes one roundtrip to the server
        mutable QString _displayData;
        mutable QString _toolTipData;
        toOracleBlob(toOracleBlob const&);
        toOracleBlob operator=(toOracleBlob const&);
        //TODO copying prohibited
};
//Q_DECLARE_METATYPE(toOracleBlob*)

class toOracleCollection: public toQValue::complexType
{
    public:
        toOracleCollection(trotl::OciConnection &_conn)
            : toQValue::complexType()
            , data(_conn)
            , _dataTypeName()
        {};
        /* virtual */
        bool isBinary() const
        {
            return false;
        }
        /* virtual */ bool isLarge() const
        {
            return false;
        }


        /* virtual */ QString const& displayData() const throw()
        {
            static const QString collection("{collection}");
            return collection;
        }

        /* virtual */ QString editData() const throw()
        {
            return QString::fromUtf8(((::trotl::tstring)data).c_str());
        }

        /* virtual */ QString userData() const throw()
        {
            static const QString collection("{collection}");
            return collection;
        }

        /* virtual */ QString const& tooltipData() const throw()
        {
            return dataTypeName();
        }

        /* virtual */ QString const& dataTypeName() const
        {
            if ( _dataTypeName.isNull())
                _dataTypeName = QString::fromUtf8(data._data_type_name.c_str());
            return _dataTypeName;
        }
        /* virtual */ QByteArray read(unsigned offset) const
        {
            return QByteArray();
        }
        /* virtual */ void write(QByteArray const &data)
        {
        }
        /* virtual */ ~toOracleCollection()
        {
            TLOG(1, toDecorator, __HERE__) << "toOracleCollection DELETED:" << this << std::endl;
        }

        mutable trotl::SqlCollection data;
    protected:
        mutable QString _dataTypeName;
        toOracleCollection(toOracleCollection const&);
        toOracleCollection operator=(toOracleCollection const&);
        //TODO copying prohibited
};
//Q_DECLARE_METATYPE(toOracleCollection*)

class toOracleCursor: public toQValue::complexType
{
    public:
        toOracleCursor() {};

        /* virtual */
        bool isBinary() const
        {
            return false;
        }
        /* virtual */ bool isLarge() const
        {
            return true;
        }


        /* virtual */ QString const& displayData() const throw()
        {
            static const QString cursor("{cursor}");
            return cursor;
        }

        /* virtual */ QString editData() const throw()
        {
            std::string s;
            data >> s;
            return QString::fromUtf8(s.c_str());
        }

        /* virtual */ QString userData() const throw()
        {
            return QString("cursor");
        }

        /* virtual */ QString const& tooltipData() const throw()
        {
            return dataTypeName();
        }

        /* virtual */ QString const& dataTypeName() const
        {
            static const QString cursor("cursor SQLT_RSET");
            return cursor;
        }
        /* virtual */ QByteArray read(unsigned offset) const
        {
            return QByteArray();
        }
        /* virtual */ void write(QByteArray const &data)
        {
        }
        /* virtual */ ~toOracleCursor()
        {
            TLOG(1, toDecorator, __HERE__) << "toOracleCursor DELETED:" << this << std::endl;
        }

        mutable trotl::SqlCursor data;
    protected:
        toOracleCursor(toOracleCursor const&);
        toOracleCursor operator=(toOracleCursor const&);
        //TODO copying prohibited
};
//Q_DECLARE_METATYPE(toOracleCursor*)
