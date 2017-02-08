
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

#include "connection/tooraclequery.h"

#define MAXTOMAXLONG 30000
#define MAXLOBSHOWN 64

#define ORA_CATCH                                 \
    catch (const ::trotl::OciException &exc)      \
    {                                             \
        ReThrowException(exc);                    \
    }                                             \

#if 0
// TODO do not know how to mark toConnectionSub to Broken
        if (exc.is_critical())                    \
            conn->Broken = true;                  \

#endif

QString const& toOracleClob::displayData() const throw()
{
    if (!_displayData.isNull())
        return _displayData;
    try
    {
        ::trotl::SqlOpenLob clob_open(data, OCI_LOB_READONLY);
        char buffer[MAXLOBSHOWN];
        oraub8 chars_read = 0;
        unsigned bytes_read = data.read(&buffer[0], sizeof(buffer), 1, sizeof(buffer), &chars_read);

        TLOG(4, toDecorator, __HERE__) << "Just read CLOB: \"" << buffer << "\"" << std::endl;

        if (data._dirname.empty())
            _displayData = QString("{clob}"); // CLOB is real CLOB
        else
            // CLOB is real CFILE (having dirname and filename)
            _displayData = QString("{cfile:%1/%2}").arg(data._dirname.c_str()).arg(data._filename.c_str());

        _displayData += QString::fromUtf8(buffer, bytes_read);
        if (chars_read != data.length())
            _displayData += "...<truncated>";
    }
    ORA_CATCH
    return _displayData;
}

QString toOracleClob::editData() const throw()
{
    QString retval;
    try
    {
        ::trotl::SqlOpenLob clob_open(data, OCI_LOB_READONLY);

        if (data._dirname.empty())
            retval = QString(
                    "Datatype: Oracle [N]CLOB\n"
                    "Size: %1 Chars\n").arg(getLength());
        else
            retval = QString(
                    "Datatype: Oracle CFILE\n"
                    "Directory: %1\n"
                    "Filename: %2\n"
                    "Size: %3B\n")
                    .arg(data._dirname.c_str())
                    .arg(data._filename.c_str())
                    .arg(getLength());
        char buffer[524288];
        unsigned offset = 0;
        unsigned to_read = 16 * data.getChunkSize();
        oraub8 bytes_read = 0, chars_read = 0;

        while (offset < MAXTOMAXLONG)
        {
            oraub8 cr = 0, br = 0;
            br = data.read(&buffer[0], sizeof(buffer), offset + 1, to_read, &cr);
            offset += cr;
            chars_read += cr;
            bytes_read += br;
            if (br == 0) // end of LOB reached
                break;
            retval += QString::fromUtf8(buffer, br);
        }

        if (offset != data.length())
            retval += "\n...<TRUNCATED>";
    }
    ORA_CATCH
    return retval;
}

QString toOracleClob::userData() const throw()
{
    QString retval;
    try
    {
        ::trotl::SqlOpenLob clob_open(data, OCI_LOB_READONLY);
        char buffer[524288];
        unsigned offset = 0;
        unsigned to_read = 16 * data.getChunkSize();
        oraub8 bytes_read = 0, chars_read = 0;

        while (true)
        {
            oraub8 cr = 0, br = 0;
            br = data.read(&buffer[0], sizeof(buffer), offset + 1, to_read, &cr);
            offset += cr;
            chars_read += cr;
            bytes_read += br;
            if (br == 0) // end of LOB reached
                break;
            retval += QString::fromUtf8(buffer, br);
        }

        if (offset != data.length())
            retval += "\n...<TRUNCATED>";
    }
    ORA_CATCH
    return retval;
}


QByteArray toOracleClob::read(unsigned offset) const
{
    unsigned chunksize = data.getChunkSize();
    char *buffer = (char*)malloc( chunksize ); // TODO use alloc here(or _alloc on MSVC)
    unsigned int bytes_read;
    try
    {
        ::trotl::SqlOpenLob clob_open(data, OCI_LOB_READONLY);
        bytes_read = data.read(buffer, chunksize, offset + 1, chunksize);
    }
    catch (const ::trotl::OciException &exc)
    {
        //if (exc.is_critical())
        //    conn->Broken = true;
        free(buffer);
        ReThrowException(exc);
    }
    QByteArray retval(buffer, bytes_read);
    free(buffer);
    return retval;
}

QString const& toOracleBlob::displayData() const throw()
{
    if (!_displayData.isNull())
        return _displayData;
    try
    {
        ::trotl::SqlOpenLob blob_open(data, OCI_LOB_READONLY);
        unsigned char buffer[MAXLOBSHOWN / 2];
        if (data._dirname.empty())
            _displayData = QString("{blob}"); // BLOB is real BLOB
        else
            // BLOB is BFILE (having dirname and filename)
            _displayData = QString("{bfile:%1/%2}").arg(data._dirname.c_str()).arg(data._filename.c_str());

        unsigned bytes_read = data.read(&buffer[0], sizeof(buffer), 1, sizeof(buffer));

        for (unsigned i = 0; i < bytes_read; ++i)
        {
            char sbuff[4];
            snprintf(sbuff, sizeof(sbuff), " %.2X", buffer[i]);
            _displayData += sbuff;
        }

        if (bytes_read >= MAXLOBSHOWN / 2)
            _displayData += "...<truncated>";
    }
    ORA_CATCH
    return _displayData;
}

QString toOracleBlob::editData() const throw()
{
    QString retval;
    try
    {
        ::trotl::SqlOpenLob clob_open(data, OCI_LOB_READONLY);
        if (data._dirname.empty())
            retval = QString(
                    "Datatype: Oracle BLOB\n"
                    "Size: %1B\n").arg(getLength());
        else
            retval = QString(
                    "Datatype: Oracle BFILE\n"
                    "Directory: %1\n"
                    "Filename: %2\n"
                    "Size: %3B\n")
                    .arg(data._dirname.c_str())
                    .arg(data._filename.c_str())
                    .arg(getLength());
        unsigned char buffer[MAXTOMAXLONG];
        ub4 chunk_size = data.getChunkSize();
        unsigned offset = 0;

        while (offset < MAXTOMAXLONG)
        {
            unsigned to_read = (std::min)(MAXTOMAXLONG - offset, chunk_size);
            unsigned bytes_read = data.read(&buffer[offset], MAXTOMAXLONG - offset, offset + 1, to_read);

            if (bytes_read == 0) // end of LOB reached
                break;

            for (unsigned i = 0; i < bytes_read; ++i)
            {
                char sbuff[4];
                snprintf(sbuff, sizeof(sbuff), " %.2X", buffer[i]);
                retval += sbuff;
                if ( (offset + i) % 32 == 31)
                    retval += "\n";
            }

            offset += bytes_read;
        }

        if (offset == MAXTOMAXLONG)
            retval += "\n...<TRUNCATED>";
    }
    ORA_CATCH
    return retval;
}

QString toOracleBlob::userData() const throw()
{
    return QString("Datape: Oracle BLOB\nSize: %1B\n").arg(data.length());
}

QByteArray toOracleBlob::read(unsigned offset) const
{
    unsigned chunksize = data.getChunkSize();
    char *buffer = (char*)malloc( chunksize ); // TODO use alloc here(or _alloc on MSVC)
    unsigned int bytes_read;
    try
    {
        ::trotl::SqlOpenLob blob_open(data, OCI_LOB_READONLY);
        bytes_read = data.read(buffer, chunksize, offset + 1, chunksize);
    }
    catch (const ::trotl::OciException &exc)
    {
        //if (exc.is_critical())
        //    conn->Broken = true;
        free(buffer);
        ReThrowException(exc);
    }
    QByteArray retval(buffer, bytes_read);
    free(buffer);
    return retval;
}
