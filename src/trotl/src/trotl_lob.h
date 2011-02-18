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
#include "trotl_conn.h" //TODO see - note bellow
#include "trotl_base.h"
#include "trotl_stat.h"

namespace trotl {

struct TROTL_EXPORT BindParLob: public SqlStatement::BindPar
{
	BindParLob(unsigned int pos, SqlStatement &stmt, ColumnType &ct);
	BindParLob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
	~BindParLob();

	virtual tstring get_string(unsigned int) const
	{
		throw OciException(__TROTL_HERE__, "Invalid datatype in conversion(BindParLob to tstring)\n");
		//return "";
	}

	virtual void bind_hook();
	virtual void define_hook();

	boolean	isTemporary(unsigned row) const;
	ub4 getChunkSize(unsigned row) const;
	
private:
	void descAlloc(void); //TODO OCI_DTYPE_FILE for BFILE, CFILE
	void descFree(void);
protected:
	BindParLob(const BindParLob &other);	
};

struct TROTL_EXPORT BindParClob: public BindParLob
{
	BindParClob(unsigned int pos, SqlStatement &stmt, ColumnType &ct);
	BindParClob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
protected:
	BindParClob(const BindParClob &other);
};

struct TROTL_EXPORT BindParBlob: public BindParLob
{
	BindParBlob(unsigned int pos, SqlStatement &stmt, ColumnType &ct);
	BindParBlob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
protected:
	BindParBlob(const BindParBlob &other);	
};

struct TROTL_EXPORT BindParCFile: public BindParLob
{
	BindParCFile(unsigned int pos, SqlStatement &stmt, ColumnType &ct);
	BindParCFile(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
protected:
	BindParCFile(const BindParCFile &other);	
};

struct TROTL_EXPORT BindParBFile: public BindParLob
{
	BindParBFile(unsigned int pos, SqlStatement &stmt, ColumnType &ct);
	BindParBFile(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);
protected:
	BindParBFile(const BindParBFile &other);	
};

// TODO - OciConnection is not actually needed.
// only Temporary lobs need reference to service context
// base class for the BLOB/CLOB data holders OCIPL::OciLob and OCIPL::OciClob
struct TROTL_EXPORT SqlLob: public SqlValue
{
	SqlLob(OciConnection &conn);
	~SqlLob();

	boolean operator==(const SqlLob& other) const;

	void clear();

	ub4 getChunkSize();

	//TODO OCILobGetLength2 for 64 bit mode
	oraub8	length();

	boolean	isOpen() const;

	boolean	isTemporary() const;

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

	~SqlTempBlob();
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

	~SqlTempClob();
};


// original code starts here


//template<typename TYPE>

//struct OciLob : public SqlValue
//{
//	//	OciLob() : _loc(NULL) {}
//
//	OciLob(OciConnection& conn):_conn(conn)
//	{
//		sword res = OCICALL(OCIDescriptorAlloc(conn._env, (dvoid**)&_loc, OCI_DTYPE_LOB, 0, NULL));
//		oci_check_error(__TROTL_HERE__, conn._env, res);
//	}
//
//	OciLob(OciConnection& conn, OCILobLocator* loc):_conn(conn),_loc(loc)
//	{
//	}
//
//	OciLob(const OciLob& other):_conn(other._conn)
//	{
//		sword res = OCICALL(OCIDescriptorAlloc(_conn._env, (dvoid**)&_loc, OCI_DTYPE_LOB, 0, NULL));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//
//#if ORACLE_MAJOR_VERSION>=8 && ORACLE_MINOR_VERSION>=1
//		res = OCICALL(OCILobLocatorAssign(_conn._svc_ctx, _conn._env._errh, other._loc, &_loc));
//#else
//		res = OCICALL(OCILobAssign(_conn._env, _conn._env._errh, other._loc, &_loc));	// no support for temporary LOBs
//#endif
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//
//		clear();
//	}
//
//	~OciLob()
//	{
//#if ORACLE_MAJOR_VERSION>=8 && ORACLE_MINOR_VERSION>=1
//		// According to the Oracle 10g documentation we should try to free
//		// implicit created temporary LOBs as soon as possible.
//		if (is_temporary()) {
//			sword res = OCICALL(OCILobFreeTemporary(_conn._svc_ctx, _conn._env._errh, _loc));
//			oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		}
//#endif
//		sword res = OCICALL(OCIDescriptorFree(_loc, OCI_DTYPE_LOB));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//	}
//
//	boolean operator==(const OciLob& other) const
//	{
//		boolean is_equal;
//		sword res = OCICALL(OCILobIsEqual(_conn._env, _loc, other._loc, &is_equal));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		return is_equal;
//	}
//
//	void clear()
//	{
//		ub4 lobEmpty = 0;
//		sword res = OCICALL(OCIAttrSet(_loc, OCI_DTYPE_LOB, &lobEmpty, 0, OCI_ATTR_LOBEMPTY, _conn._env._errh));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//	}
//
//	void* get_ref() {return &_loc;}
//
//	//tstring str(bool internal=false) const;
//
//
//	ub4	get_chunk_size()
//	{
//		ub4 len;
//		sword res = OCICALL(OCILobGetLength(_conn._svc_ctx, _conn._env._errh, _loc, &len));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		return len;
//	}
//
//#if ORACLE_MAJOR_VERSION>=8 && ORACLE_MINOR_VERSION>=1
//	ub4	get_length()
//	{
//		ub4 size;
//		sword res = OCICALL(OCILobGetChunkSize(_conn._svc_ctx, _conn._env._errh, _loc, &size));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		return size;
//	}
//
//	boolean	is_open() const
//	{
//		boolean flag;
//		sword res = OCICALL(OCILobIsOpen(_conn._svc_ctx, _conn._env._errh, _loc, &flag));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		return flag;
//	}
//
//	boolean	is_temporary() const
//	{
//		boolean flag;
//		sword res = OCICALL(OCILobIsTemporary(_conn._env, _conn._env._errh, _loc, &flag));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		return flag;
//	}
//#endif
//
//
//void copy(const OciLob& src, ub4 amount, ub4 dst_offset, ub4 src_offset)
//{
//	sword res = OCICALL(OCILobCopy(_conn._svc_ctx, _conn._env._errh, _loc, src._loc, amount, dst_offset, src_offset));
//	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//}
//
//void append(const OciLob& src)
//{
//	sword res = OCICALL(OCILobAppend(_conn._svc_ctx, _conn._env._errh, _loc, src._loc));
//	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//}
//
//void trim(ub4 newlen=0)
//{
//	sword res = OCICALL(OCILobTrim(_conn._svc_ctx, _conn._env._errh, _loc, newlen));
//	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//}
//
//ub4	erase(ub4 offset, ub4 amount)
//{
//	sword res = OCICALL(OCILobErase(_conn._svc_ctx, _conn._env._errh, _loc, &amount, offset));
//	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//	return amount;
//}
//
//
//void enable_buffering()
//{
//	sword res = OCICALL(OCILobEnableBuffering(_conn._svc_ctx, _conn._env._errh, _loc));
//	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//}
//
//void disable_buffering()
//{
//	sword res = OCICALL(OCILobDisableBuffering(_conn._svc_ctx, _conn._env._errh, _loc));
//	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//}
//
//void flush(ub4 flag=OCI_LOB_BUFFER_NOFREE)
//{
//	sword res = OCICALL(OCILobFlushBuffer(_conn._svc_ctx, _conn._env._errh, _loc, flag));
//	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//}
//
//protected:
//	OciConnection&	_conn;
//	OCILobLocator*	_loc;
//
//	friend struct OciOpenLob;
//};
//
///// open and close LOBs using OciOpenLob for exception handling
//// Forgetting to close LOBs results in errors in OciLogin::disconnect().
//struct OciOpenLob
//{
//	OciOpenLob(OciLob& lob, ub1 mode=OCI_LOB_READWRITE):_lob(lob)
//	{
//		sword res = OCICALL(OCILobOpen(lob._conn._svc_ctx, lob._conn._env._errh, lob._loc, mode));
//		oci_check_error(__TROTL_HERE__, lob._conn._env._errh, res);
//	}
//
//	~OciOpenLob()
//	{
//		sword res = OCICALL(OCILobClose(_lob._conn._svc_ctx, _lob._conn._env._errh, _lob._loc));
//		oci_check_error(__TROTL_HERE__, _lob._conn._env._errh, res);
//	}
//
//protected:
//	OciLob&	_lob;
//};
//
//
///// BLOB data holder structure
//struct OciBlob : public OciLob
//{
//	OciBlob(OciConnection& conn) : OciLob(conn) {}
//	OciBlob(const OciLob& other) : OciLob(other) {}
//
//	ub4	write(const dvoid* bufp, ub4 buflen, ub4 offset/*=1*/, ub4 amount)
//	{
//		sword res = OCICALL(OCILobWrite(_conn._svc_ctx, _conn._env._errh, _loc,
//				&amount, offset, (dvoid*)bufp, buflen,
//				OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
//				0, 0));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		return amount;
//	}
//
//	ub4	write_append(const dvoid* bufp, ub4 buflen, ub4 amount)
//	{
//		sword res = OCICALL(OCILobWriteAppend(_conn._svc_ctx, _conn._env._errh, _loc,
//				&amount, (dvoid*)bufp, buflen,
//				OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
//				0, 0));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		return amount;
//	}
//
//	ub4	read(dvoid* bufp, ub4 buflen, ub4 offset, ub4 amount)
//	{
//		sword res = OCICALL(OCILobRead(_conn._svc_ctx, _conn._env._errh, _loc,
//				&amount, offset, bufp, buflen,
//				NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,CONST dvoid*bufp,ub4*len,ub1*piece)*/,
//				0, 0));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		return amount;
//	}
//};
//
//struct OciTempBlob : public OciBlob
//{
//	OciTempBlob(OciConnection& conn, OCIDuration dur=OCI_DURATION_SESSION)
//	:	OciBlob(conn)
//	{
//		sword res = OCICALL(OCILobCreateTemporary(conn._svc_ctx, conn._env._errh, _loc,
//				OCI_DEFAULT, SQLCS_IMPLICIT, OCI_TEMP_BLOB, FALSE, dur));
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//	}
//
//	~OciTempBlob()
//	{
//		/*already implemented in ~OciLob
//	  sword res = OCICALL(OCILobFreeTemporary(_conn._svc_ctx, _conn._env._errh, _loc));
//	  oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		 */
//	}
//};
//
//
///// CLOB data holder structure
//struct OciClob : public OciLob
//{
//	OciClob(OciConnection& conn) : OciLob(conn) {}
//	OciClob(const OciLob& other) : OciLob(other) {}
//
//	ub4	write(const dvoid* bufp, ub4 buflen, ub4 offset, ub4 amount, ub2 csid=0, ub1 csfrm=SQLCS_IMPLICIT)
//	{
//		sword res = OCICALL(OCILobWrite(_conn._svc_ctx, _conn._env._errh, _loc,
//				&amount, offset, (dvoid*)bufp, buflen,
//				OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
//				csid, csfrm));
//
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//
//		return amount;
//	}
//
//	ub4	write_append(const dvoid* bufp, ub4 buflen, ub4 amount, ub2 csid=0, ub1 csfrm=SQLCS_IMPLICIT)
//	{
//		sword res = OCICALL(OCILobWriteAppend(_conn._svc_ctx, _conn._env._errh, _loc,
//				&amount, (dvoid*)bufp, buflen,
//				OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
//				csid, csfrm));
//
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//
//		return amount;
//	}
//
//	ub4	read(dvoid* bufp, ub4 buflen, ub4 offset, ub4 amount, ub2 csid=0, ub1 csfrm=SQLCS_IMPLICIT)
//	{
//		sword res = OCICALL(OCILobRead(_conn._svc_ctx, _conn._env._errh, _loc,
//				&amount, offset, bufp, buflen,
//				NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,CONST dvoid*bufp,ub4*len,ub1*piece)*/,
//				csid, csfrm));
//
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//
//		return amount;
//	}
//};
//
//struct OciTempClob : public OciClob
//{
//	OciTempClob(OciConnection& conn, OCIDuration dur=OCI_DURATION_SESSION)
//	:	OciClob(conn)
//	{
//		sword res = OCICALL(OCILobCreateTemporary(conn._svc_ctx, conn._env._errh, _loc,
//				OCI_DEFAULT, SQLCS_IMPLICIT, OCI_TEMP_CLOB, FALSE, dur));
//
//		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//	}
//
//	~OciTempClob()
//	{
//		/*already implemented in ~OciLob
//	sword res = OCICALL(OCILobFreeTemporary(_conn._svc_ctx, _conn._env._errh, _loc));
//	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
//		 */
//	}
//};
//
///* todo
//   struct OciBFile : public OciLob
//   {
//   OciBFile(OciConnection& conn) : OciLob(conn) {}
//   OciBFile(const OciLob& other) : OciLob(other) {}
//   };
//
//   struct OciCFile : public OciLob
//   {
//   OciCFile(OciConnection& conn) : OciLob(conn) {}
//   OciCFile(const OciLob& other) : OciLob(other) {}
//   };
// */


}

#endif
