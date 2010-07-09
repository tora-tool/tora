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
  AND ANY EXPRES7S OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
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

#ifdef WIN32
#include "trotl_parser.h"
#include <stdio.h>
#define snprintf _snprintf
//#define gmtime gmtime_s
//#define localtime localtime_s
#endif

#include "trotl_lob.h"

namespace trotl {

// Register Bind datatypes in factory(Bind - PL/SQL)
Util::RegisterInFactory<BindParClob, BindParFactTwoParmSing> regBindClob("clob");
Util::RegisterInFactory<BindParBlob, BindParFactTwoParmSing> regBindBlob("blob");
Util::RegisterInFactory<BindParCFile, BindParFactTwoParmSing> regBindCFile("cfile");
Util::RegisterInFactory<BindParBFile, BindParFactTwoParmSing> regBindBFile("bfile");

// Register Bind datatypes in factory(Define - SELECT)
Util::RegisterInFactory<BindParClob, DefineParFactTwoParmSing, int> regDefineNumberClob(SQLT_CLOB);
Util::RegisterInFactory<BindParBlob, DefineParFactTwoParmSing, int> regDefineNumberBlob(SQLT_BLOB);
Util::RegisterInFactory<BindParCFile, DefineParFactTwoParmSing, int> regDefineNumberCFile(SQLT_CFILEE);
Util::RegisterInFactory<BindParBFile, DefineParFactTwoParmSing, int> regDefineNumberBFile(SQLT_BFILEE);

BindParLob::BindParLob(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : BindPar(pos, stmt, ct)
{
	//valuep = new unsigned char [ _cnt * ( sizeof(OCILobLocator*) ) ];
	valuep = malloc( _cnt * sizeof(OCILobLocator*)  );

	////memset(valuep, 0x5a, _cnt * sizeof(OCILobLocator*) );

	value_sz = sizeof(OCILobLocator*);
	descAlloc();
}

BindParLob::BindParLob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): BindPar(pos, stmt, decl)
{
	//valuep = new unsigned char [ decl.bracket[1] * ( OCI_NUMBER_SIZE + 1) ];
	valuep = malloc( _cnt * sizeof(OCILobLocator*)  );

	////memset(valuep, 0x5a, _cnt * sizeof(OCILobLocator*) );

	value_sz = sizeof(OCILobLocator*);
	descAlloc();
};

BindParLob::~BindParLob()
{
	descFree();
	//std::cerr << "~BindParLob1" << std::endl;
	if(valuep)
	{
		//  TODO
		//	*** glibc detected *** /home/ivan/workspace/trotl/debug/src/.libs/lt-test_28: corrupted double-linked list: 0x080a68b0 ***
		//
		//std::cerr << "~BindParLob2" << std::endl;
		//delete[] valuep;
		free(valuep);
		//std::cerr << "~BindParLob3" << std::endl;
		valuep = NULL;
	}
	std::cerr << "~BindParLob4" << std::endl;
}

void BindParLob::descAlloc(void) //TODO OCI_DTYPE_FILE for BFILE, CFILE
{
	for(unsigned i=0; i<_cnt; ++i)
	{
		sword res = OCICALL(OCIDescriptorAlloc(_env, &((void**)valuep)[i], OCI_DTYPE_LOB, 0, NULL));
		oci_check_error(__TROTL_HERE__, _env, res);
	}
}

void BindParLob::descFree(void)
{
	for(unsigned i=0; i<_cnt; ++i)
	{
		if(((void**)valuep)[i])
		{
			//std::cerr << "void descFree(void)" << std::endl;
			sword res = OCICALL(OCIDescriptorFree( ((void**)valuep)[i], OCI_DTYPE_LOB));
			oci_check_error(__TROTL_HERE__, _env, res);
		}
	}
	//std::cerr << "void descFree(void) done" << std::endl;
}

// TODO add ind skip here
void BindParLob::bind_hook(SqlStatement &stmt)
{
	sword res = OCICALL(OCIBindArrayOfStruct(bindp, stmt._errh, sizeof(OCILobLocator*), 0, 0, 0 ));
	oci_check_error(__TROTL_HERE__, stmt._errh, res);
}
// TODO add ind skip here
void BindParLob::define_hook(SqlStatement &stmt)
{
//	sword res = OCICALL(OCIDefineArrayOfStruct(defnpp , _env._errh, sizeof(OCILobLocator*), 0, 0, 0 ));
//	oci_check_error(__TROTL_HERE__, _env, res);
}

BindParClob::BindParClob(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : BindParLob(pos, stmt, ct)
{
	dty = SQLT_CLOB;
	type_name = ct.get_type_str();
};

BindParClob::BindParClob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindParLob(pos, stmt, decl)
{
	dty = SQLT_CLOB;
	type_name = "CLOB";
};

BindParBlob::BindParBlob(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : BindParLob(pos, stmt, ct)
{
	dty = SQLT_BLOB;
	type_name = ct.get_type_str();
};

BindParBlob::BindParBlob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindParLob(pos, stmt, decl)
{
	dty = SQLT_BLOB;
	type_name = "BLOB";
};

BindParCFile::BindParCFile(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : BindParLob(pos, stmt, ct)
{
	dty = SQLT_CFILEE;
	type_name = ct.get_type_str();
};

BindParCFile::BindParCFile(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindParLob(pos, stmt, decl)
{
	dty = SQLT_CFILEE;
	type_name = "CFILE";
};

BindParBFile::BindParBFile(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : BindParLob(pos, stmt, ct)
{
	dty = SQLT_BFILEE;
	type_name = ct.get_type_str();
};

BindParBFile::BindParBFile(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindParLob(pos, stmt, decl)
{
	dty = SQLT_BFILEE;
	type_name = "BFILE";
};

SqlLob::SqlLob(OciConnection &conn) : _conn(conn), _loc(NULL)
{
	sword res = OCICALL(OCIDescriptorAlloc(_conn._env, (dvoid**)&_loc, OCI_DTYPE_LOB, 0, NULL));
	oci_check_error(__TROTL_HERE__, _conn._env, res);
};

SqlLob::~SqlLob()
{
	// According to the Oracle 10g documentation we should try to free
	// implicit created temporary LOBs as soon as possible.
	if (is_temporary()) {
		sword res = OCICALL(OCILobFreeTemporary(_conn._svc_ctx, _conn._env._errh, _loc));
		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	}

	sword res = OCICALL(OCIDescriptorFree(_loc, OCI_DTYPE_LOB));
	oci_check_error(__TROTL_HERE__, _conn._env, res);
};

boolean SqlLob::operator==(const SqlLob& other) const
{
	boolean is_equal;
	sword res = OCICALL(OCILobIsEqual(_conn._env, _loc, other._loc, &is_equal));
	oci_check_error(__TROTL_HERE__, _conn._env, res);
	return is_equal;
};

void SqlLob::clear()
{
	ub4 lobEmpty = 0;
	sword res = OCICALL(OCIAttrSet(_loc, OCI_DTYPE_LOB, &lobEmpty, 0, OCI_ATTR_LOBEMPTY, _conn._env._errh));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

ub4	SqlLob::get_chunk_size()
{
	ub4 size;
	sword res = OCICALL( OCILobGetChunkSize(_conn._svc_ctx, _conn._env._errh, _loc, &size));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return size;
};

ub4	SqlLob::get_length()
{
	if(is_not_null())
	{
		ub4 len;
		sword res = OCICALL(OCILobGetLength(_conn._svc_ctx, _conn._env._errh, _loc, &len));
		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
		return len;
	} else {
		return 0;
	}
};

boolean	SqlLob::is_open() const
{
	boolean flag;
	sword res = OCICALL(OCILobIsOpen(_conn._svc_ctx, _conn._env._errh, _loc, &flag));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return flag;
};

boolean	SqlLob::is_temporary() const
{
	boolean flag;
	sword res = OCICALL(OCILobIsTemporary(_conn._env, _conn._env._errh, _loc, &flag));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return flag;
};

void SqlLob::copy(const SqlLob& src, ub4 amount, ub4 dst_offset, ub4 src_offset)
{
	sword res = OCICALL(OCILobCopy(_conn._svc_ctx, _conn._env._errh, _loc, src._loc, amount, dst_offset, src_offset));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

void SqlLob::append(const SqlLob& src)
{
	sword res = OCICALL(OCILobAppend(_conn._svc_ctx, _conn._env._errh, _loc, src._loc));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

void SqlLob::trim(ub4 newlen)
{
	sword res = OCICALL(OCILobTrim(_conn._svc_ctx, _conn._env._errh, _loc, newlen));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

ub4	SqlLob::erase(ub4 offset, ub4 amount)
{
	sword res = OCICALL(OCILobErase(_conn._svc_ctx, _conn._env._errh, _loc, &amount, offset));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

void SqlLob::enable_buffering()
{
	sword res = OCICALL(OCILobEnableBuffering(_conn._svc_ctx, _conn._env._errh, _loc));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

void SqlLob::disable_buffering()
{
	sword res = OCICALL(OCILobDisableBuffering(_conn._svc_ctx, _conn._env._errh, _loc));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

void SqlLob::flush(ub4 flag)
{
	sword res = OCICALL(OCILobFlushBuffer(_conn._svc_ctx, _conn._env._errh, _loc, flag));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

SqlOpenLob::SqlOpenLob(SqlLob& lob, ub1 mode) : _lob(lob)
{
	sword res = OCICALL(OCILobOpen(
			lob._conn._svc_ctx,
			lob._conn._env._errh,
			lob._loc,
			mode
	));
	oci_check_error(__TROTL_HERE__, lob._conn._env._errh, res);
};

SqlOpenLob::~SqlOpenLob()
{
	sword res = OCICALL(OCILobClose(
			_lob._conn._svc_ctx,
			_lob._conn._env._errh,
			_lob._loc
	));
	oci_check_error(__TROTL_HERE__, _lob._conn._env._errh, res);
};

ub4	SqlBlob::write(const dvoid* bufp, ub4 buflen, ub4 offset/*=1*/, ub4 amount)
{
	sword res = OCICALL(OCILobWrite(_conn._svc_ctx, _conn._env._errh, _loc,
			&amount, offset, (dvoid*)bufp, buflen,
			OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
			0, 0));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

ub4	SqlBlob::write_append(const dvoid* bufp, ub4 buflen, ub4 amount)
{
	sword res = OCICALL(OCILobWriteAppend(_conn._svc_ctx, _conn._env._errh, _loc,
			&amount, (dvoid*)bufp, buflen,
			OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
			0, 0));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

ub4	SqlBlob::read(dvoid* bufp, ub4 buflen, ub4 offset, ub4 amount)
{
	sword res = OCICALL(OCILobRead(_conn._svc_ctx, _conn._env._errh, _loc,
			&amount, offset, bufp, buflen,
			NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,CONST dvoid*bufp,ub4*len,ub1*piece)*/,
			0, 0));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

SqlTempBlob::SqlTempBlob(OciConnection& conn, OCIDuration dur) : SqlBlob(conn)
{
	sword res = OCICALL(OCILobCreateTemporary(conn._svc_ctx, conn._env._errh, _loc,
			OCI_DEFAULT, SQLCS_IMPLICIT, OCI_TEMP_BLOB, FALSE, dur));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

SqlTempBlob::~SqlTempBlob()
{
	// already implemented in ~OciLob
	// sword res = OCICALL(OCILobFreeTemporary(_conn._svc_ctx, _conn._env._errh, _loc));
	// oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

ub4	SqlClob::write(const dvoid* bufp, ub4 buflen, ub4 offset, ub4 amount, ub2 csid, ub1 csfrm)
{
	sword res = OCICALL(OCILobWrite(_conn._svc_ctx, _conn._env._errh, _loc,
			&amount, offset, (dvoid*)bufp, buflen,
			OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
			csid, csfrm));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

ub4	SqlClob::write_append(const dvoid* bufp, ub4 buflen, ub4 amount, ub2 csid, ub1 csfrm)
{
	sword res = OCICALL(OCILobWriteAppend(_conn._svc_ctx, _conn._env._errh, _loc,
			&amount, (dvoid*)bufp, buflen,
			OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
			csid, csfrm));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

/*
 * NOTE: !! offset has to be >= 1. Oracle's LOBs start at 1st byte.
 */
ub4	SqlClob::read(dvoid* bufp, ub4 buflen, ub4 offset, ub4 amount, ub2 csid, ub1 csfrm)
{
	sword res = OCICALL(OCILobRead(
			_conn._svc_ctx,
			_conn._env._errh,
			_loc,
			&amount,
			offset,
			bufp,
			buflen,
			NULL, /*dvoid* ctxp*/
			NULL, /*sb4 (*cbfp)(dvoid*ctxp,CONST dvoid*bufp,ub4*len,ub1*piece)*/
			0 /* csid */,
			0 /* csfrm */
	));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

SqlTempClob::SqlTempClob(OciConnection& conn, OCIDuration dur) : SqlClob(conn)
{
	sword res = OCICALL(OCILobCreateTemporary(conn._svc_ctx, conn._env._errh, _loc,
			OCI_DEFAULT, SQLCS_IMPLICIT, OCI_TEMP_CLOB, FALSE, dur));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
}

SqlTempClob::~SqlTempClob()
{
	// already implemented in ~OciLob
	// sword res = OCICALL(OCILobFreeTemporary(_conn._svc_ctx, _conn._env._errh, _loc));
	// oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
}

};
