// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Martin Fuchs <martin-fuchs@gmx.net>
//
// forked from OCIPL Version 1.3
// by Ivan Brezina <ivan@cvut.cz>

/*

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in
	the documentation and/or other materials provided with the
	distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TROTL_LOB_H_
#define TROTL_LOB_H_

#include "trotl_export.h"
#include "trotl_common.h"
#include "trotl_handle.h"
#include "trotl_var.h"
#include "trotl_conn.h" //TODO see - note bellow ***

namespace trotl
{

struct TROTL_EXPORT BindParLob: public BindPar
{
	BindParLob(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParLob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
	virtual ~BindParLob();

	virtual tstring get_string(unsigned int) const
	{
		throw_oci_exception(OciException(__TROTL_HERE__, "Invalid datatype in conversion(BindParLob to tstring)\n"));
		return "";
	}

	virtual void bind_hook();
	virtual void define_hook();

	bool isTemporary(unsigned row) const;
	ub4 getChunkSize(unsigned row) const;

private:
	void descAlloc(void); //TODO OCI_DTYPE_FILE for BFILE, CFILE
	void descFree(void);
protected:
	BindParLob(const BindParLob &other);
};

struct TROTL_EXPORT BindParClob: public BindParLob
{
	BindParClob(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParClob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
protected:
	BindParClob(const BindParClob &other);
};

struct TROTL_EXPORT BindParBlob: public BindParLob
{
	BindParBlob(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParBlob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
protected:
	BindParBlob(const BindParBlob &other);
};

struct TROTL_EXPORT BindParCFile: public BindParLob
{
	BindParCFile(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParCFile(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
protected:
	BindParCFile(const BindParCFile &other);
};

struct TROTL_EXPORT BindParBFile: public BindParLob
{
	BindParBFile(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParBFile(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
protected:
	BindParBFile(const BindParBFile &other);
};

// TODO ***- OciConnection is not actually needed.
// only Temporary lobs need reference to service context
// base class for the BLOB/CLOB data holders OCIPL::OciLob and OCIPL::OciClob
struct TROTL_EXPORT SqlLob: public SqlValue
{
	SqlLob(OciConnection &conn);
	virtual ~SqlLob();

	bool operator==(const SqlLob& other) const;

	void clear();

	ub4 getChunkSize();

	oraub8	length();

	bool isOpen() const;

	bool isTemporary() const;

	void copy(const SqlLob& src, oraub8 amount, oraub8 dst_offset, oraub8 src_offset);

	void append(const SqlLob& src);

	void trim(oraub8 newlen=0);

	oraub8 erase(oraub8 offset, oraub8 amount);

	void enableBuffering();

	void disableBuffering();

	void flush(ub4 flag=OCI_LOB_BUFFER_NOFREE);

protected:
	OciConnection&	_conn;
	OCILobLocator*	_loc;

	friend struct SqlOpenLob;
	friend struct ConvertorForRead;
	friend struct ConvertorForWrite;

	SqlLob();
};

// open and close LOBs using OciOpenLob for exception handling
// Forgetting to close LOBs results in errors in OciLogin::disconnect().
struct TROTL_EXPORT SqlOpenLob
{
	SqlOpenLob(SqlLob& lob, ub1 mode=OCI_LOB_READWRITE);

	~SqlOpenLob();

protected:
	SqlLob&	_lob;
};

/// BLOB data holder structure
struct TROTL_EXPORT SqlBlob : public SqlLob
{
	SqlBlob(OciConnection &conn) : SqlLob(conn) {}

	oraub8	write(const dvoid* bufp, oraub8 buflen, oraub8 offset/*=1*/, oraub8 amount);

	oraub8	write_append(const dvoid* bufp, oraub8 buflen, oraub8 amount);

	oraub8	read(dvoid* bufp, oraub8 buflen, oraub8 offset, oraub8 amount);
};

struct TROTL_EXPORT SqlTempBlob : public SqlBlob
{
	SqlTempBlob(OciConnection &conn, OCIDuration dur=OCI_DURATION_SESSION);

	virtual ~SqlTempBlob();
};

// CLOB data holder structure
struct TROTL_EXPORT SqlClob : public SqlLob
{
	SqlClob(OciConnection &conn) : SqlLob(conn) {}

	oraub8	write(const dvoid* bufp, oraub8 buflen, oraub8 offset, oraub8 amount, ub2 csid=0, ub1 csfrm=SQLCS_IMPLICIT);

	oraub8	write_append(const dvoid* bufp, oraub8 buflen, oraub8 amount, ub2 csid=0, ub1 csfrm=SQLCS_IMPLICIT);

	/*
	 * NOTE: !! offset has to be >= 1. Oracle's LOBs start with 1st byte.
	 */
	oraub8	read(dvoid* bufp, oraub8 buflen, oraub8 offset, oraub8 amount, oraub8 *chars=NULL, ub2 csid=0, ub1 csfrm=SQLCS_IMPLICIT);
};

struct TROTL_EXPORT SqlTempClob : public SqlClob
{
	SqlTempClob(OciConnection &conn, OCIDuration dur=OCI_DURATION_SESSION);

	virtual ~SqlTempClob();
};

}

#endif
