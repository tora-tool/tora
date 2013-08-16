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

#include "trotl_export.h"
#include "trotl_common.h"
#include "trotl_lob.h"
#include "trotl_describe.h"
#include "trotl_stat.h"

#include <assert.h>

namespace trotl
{

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

BindParLob::BindParLob(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindPar(pos, stmt, ct)
{
	//valuep = new unsigned char [ _cnt * ( sizeof(OCILobLocator*) ) ];
	valuep = (void**) calloc(_cnt, sizeof(OCILobLocator*));
	value_sz = sizeof(OCILobLocator*);

	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub2*)rlenp)[i] = (ub2) value_sz;
	}

	descAlloc();
}

BindParLob::BindParLob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): BindPar(pos, stmt, decl)
{
	//valuep = new unsigned char [ decl.bracket[1] * ( OCI_NUMBER_SIZE + 1) ];
	valuep = (void**) calloc(_cnt, sizeof(OCILobLocator*));
	value_sz = sizeof(OCILobLocator*);

	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub4*)rlenp)[i] = (ub4) value_sz;
	}

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
	//std::cerr << "~BindParLob4" << std::endl;
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
		if(((OCILobLocator**)valuep)[i])
		{
			//std::cerr << "void descFree(void): " << i << std::endl;
			sword res = OCICALL(OCIDescriptorFree( ((void**)valuep)[i], OCI_DTYPE_LOB));
			oci_check_error(__TROTL_HERE__, _env, res);
		}
	}
	//std::cerr << "void descFree(void) done" << std::endl;
}

// TODO add ind skip here
void BindParLob::bind_hook()
{
	sword res = OCICALL(OCIBindArrayOfStruct(bindp, _stmt._errh, sizeof(OCILobLocator*), 0, 0, 0 ));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
}
// TODO add ind skip here
void BindParLob::define_hook()
{
//	sword res = OCICALL(OCIDefineArrayOfStruct(defnpp , _env._errh, sizeof(OCILobLocator*), 0, 0, 0 ));
//	oci_check_error(__TROTL_HERE__, _env, res);
}

bool BindParLob::isTemporary(unsigned _row) const
{
	bOOlean flag;
	sword res = OCICALL(OCILobIsTemporary(_env, _env._errh, ((OCILobLocator**)valuep)[_row], &flag));
	oci_check_error(__TROTL_HERE__, _env._errh, res);
	return !!flag;
}

ub4 BindParLob::getChunkSize(unsigned _row) const
{
	ub4 retval;
	sword res = OCICALL(OCILobGetChunkSize(_stmt._conn._svc_ctx, _env._errh, ((OCILobLocator**)valuep)[_row], &retval));
	oci_check_error(__TROTL_HERE__, _env._errh, res);
	return retval;

}

BindParClob::BindParClob(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindParLob(pos, stmt, ct)
{
	dty = SQLT_CLOB;
	_type_name = ct->typeName();
};

BindParClob::BindParClob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindParLob(pos, stmt, decl)
{
	dty = SQLT_CLOB;
	_type_name = "CLOB";
};

BindParBlob::BindParBlob(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindParLob(pos, stmt, ct)
{
	dty = SQLT_BLOB;
	_type_name = ct->typeName();
};

BindParBlob::BindParBlob(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindParLob(pos, stmt, decl)
{
	dty = SQLT_BLOB;
	_type_name = "BLOB";
};

BindParCFile::BindParCFile(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindParLob(pos, stmt, ct)
{
	dty = SQLT_CFILEE;
	_type_name = ct->typeName();
};

BindParCFile::BindParCFile(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindParLob(pos, stmt, decl)
{
	dty = SQLT_CFILEE;
	_type_name = "CFILE";
};

BindParBFile::BindParBFile(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindParLob(pos, stmt, ct)
{
	dty = SQLT_BFILEE;
	_type_name = ct->typeName();
};

BindParBFile::BindParBFile(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindParLob(pos, stmt, decl)
{
	dty = SQLT_BFILEE;
	_type_name = "BFILE";
};

SqlLob::SqlLob(OciConnection &conn) : _conn(conn), _loc(NULL)
{
	sword res = OCICALL(OCIDescriptorAlloc(_conn._env, (void**)&_loc, OCI_DTYPE_LOB, 0, NULL));
	oci_check_error(__TROTL_HERE__, _conn._env, res);
};

SqlLob::~SqlLob()
{
	// According to the Oracle 10g documentation we should try to free
	// implicit created temporary LOBs as soon as possible.
	if (isTemporary())
	{
		sword res = OCICALL(OCILobFreeTemporary(_conn._svc_ctx, _conn._env._errh, _loc));
		oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	}

	sword res = OCICALL(OCIDescriptorFree(_loc, OCI_DTYPE_LOB));
	oci_check_error(__TROTL_HERE__, _conn._env, res);
};

bool SqlLob::operator==(const SqlLob& other) const
{
	bOOlean is_equal;
	sword res = OCICALL(OCILobIsEqual(_conn._env, _loc, other._loc, &is_equal));
	oci_check_error(__TROTL_HERE__, _conn._env, res);
	return !!is_equal;
};

void SqlLob::clear()
{
	ub4 lobEmpty = 0;
	sword res = OCICALL(OCIAttrSet(_loc, OCI_DTYPE_LOB, &lobEmpty, 0, OCI_ATTR_LOBEMPTY, _conn._env._errh));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

ub4	SqlLob::getChunkSize()
{
	ub4 size;
	sword res = OCICALL( OCILobGetChunkSize(_conn._svc_ctx, _conn._env._errh, _loc, &size));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return size;
};

oraub8	SqlLob::length()
{
	if(!is_not_null())
		return 0;

	bool done = false;
	oraub8 len;
	while(!done)
	{
		sword res = OCICALL(OCILobGetLength2(_conn._svc_ctx, _conn._env._errh, _loc, &len));

		if( res != OCI_SUCCESS )
		{
			sb4 errorcode;
			sword res2 = OCICALL(OCIErrorGet(_conn._env._errh, 1, NULL, &errorcode, NULL, 0, OCI_HTYPE_ERROR));
			assert(res2 == OCI_SUCCESS);

			if(errorcode == 3127) // ORA-03127: no new operations allowed until the active operation ends
			{
				std::cerr << "ORA-03127: no new operations allowed until the active operation ends" << std::endl;
				MSLEEP(100);
			}
			else
			{
				oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
			}
		}
		else
		{
			done = true;
		}
	}

	return len;
};

bool	SqlLob::isOpen() const
{
	bOOlean flag;
	sword res = OCICALL(OCILobIsOpen(_conn._svc_ctx, _conn._env._errh, _loc, &flag));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return !!flag;
};

bool	SqlLob::isTemporary() const
{
	bOOlean flag;
	sword res = OCICALL(OCILobIsTemporary(_conn._env, _conn._env._errh, _loc, &flag));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return !!flag;
};

void SqlLob::copy(const SqlLob& src, oraub8 amount, oraub8 dst_offset, oraub8 src_offset)
{
	sword res = OCICALL(OCILobCopy2(_conn._svc_ctx, _conn._env._errh, _loc, src._loc, amount, dst_offset, src_offset));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

void SqlLob::append(const SqlLob& src)
{
	sword res = OCICALL(OCILobAppend(_conn._svc_ctx, _conn._env._errh, _loc, src._loc));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

void SqlLob::trim(oraub8 newlen)
{
	sword res = OCICALL(OCILobTrim2(_conn._svc_ctx, _conn._env._errh, _loc, newlen));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

oraub8	SqlLob::erase(oraub8 offset, oraub8 amount)
{
	sword res = OCICALL(OCILobErase2(_conn._svc_ctx, _conn._env._errh, _loc, &amount, offset));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

void SqlLob::enableBuffering()
{
	sword res = OCICALL(OCILobEnableBuffering(_conn._svc_ctx, _conn._env._errh, _loc));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
};

void SqlLob::disableBuffering()
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
	bool done = false;
	while(!done)
	{
		sword res = OCICALL(OCILobOpen(
		                            lob._conn._svc_ctx,
		                            lob._conn._env._errh,
		                            lob._loc,
		                            mode
		                    ));
		if( res != OCI_SUCCESS )
		{
			sb4 errorcode;
			sword res2 = OCICALL(OCIErrorGet(_lob._conn._env._errh, 1, NULL, &errorcode, NULL, 0, OCI_HTYPE_ERROR));
			assert(res2 == OCI_SUCCESS);

			if(errorcode == 3127) // ORA-03127: no new operations allowed until the active operation ends
			{
				std::cerr << "ORA-03127: no new operations allowed until the active operation ends" << std::endl;
				MSLEEP(100);
			}
			else
			{
				oci_check_error(__TROTL_HERE__, _lob._conn._env._errh, res);
			}
		}
		else
		{
			done = true;
		}
	}
};

SqlOpenLob::~SqlOpenLob()
{
	bool done = false;
	while(!done)
	{
		sword res = OCICALL(OCILobClose(
		                            _lob._conn._svc_ctx,
		                            _lob._conn._env._errh,
		                            _lob._loc
		                    ));

		if( res != OCI_SUCCESS )
		{
			sb4 errorcode;
			sword res2 = OCICALL(OCIErrorGet(_lob._conn._env._errh, 1, NULL, &errorcode, NULL, 0, OCI_HTYPE_ERROR));
			assert(res2 == OCI_SUCCESS);

			if(errorcode == 3127) // ORA-03127: no new operations allowed until the active operation ends
			{
				std::cerr << "ORA-03127: no new operations allowed until the active operation ends" << std::endl;
				MSLEEP(100);
			}
			else
			{
				oci_check_error(__TROTL_HERE__, _lob._conn._env._errh, res);
			}
		}
		else
		{
			done = true;
		}
	}
};

oraub8	SqlBlob::write(const dvoid* bufp, oraub8 buflen, oraub8 offset/*=1*/, oraub8 amount)
{
	sword res = OCICALL(OCILobWrite2(_conn._svc_ctx, _conn._env._errh, _loc,
	                                 &amount, NULL, offset, (dvoid*)bufp, buflen,
	                                 OCI_ONE_PIECE/*ub1 piece*/, NULL/*dvoid* ctxp*/, NULL/*sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/,
	                                 0, 0));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

oraub8	SqlBlob::write_append(const dvoid* bufp, oraub8 buflen, oraub8 amount)
{
	sword res = OCICALL(OCILobWriteAppend2(_conn._svc_ctx, _conn._env._errh, _loc,
	                                       &amount, NULL, (dvoid*)bufp, buflen,
	                                       OCI_ONE_PIECE, /* ub1 piece */
	                                       NULL, /* dvoid* ctxp */
	                                       NULL, /* sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,ub4*len,ub1*piece)*/
	                                       0, 0  /* ub2 csid, ub1 csfrm */
	                                      ));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

oraub8	SqlBlob::read(dvoid* bufp, oraub8 buflen, oraub8 offset, oraub8 amount)
{
	sword res = OCICALL(OCILobRead2(_conn._svc_ctx, _conn._env._errh, _loc,
	                                &amount, NULL, offset, bufp, buflen,
	                                OCI_ONE_PIECE, /* ub1 piece */
	                                NULL, /* dvoid* ctxp */
	                                NULL, /* sb4 (*cbfp)(dvoid*ctxp,CONST dvoid*bufp,ub4*len,ub1*piece) */
	                                0, 0  /* ub2 csid, ub1 csfrm */
	                               ));
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

oraub8	SqlClob::write(const dvoid* bufp, oraub8 buflen, oraub8 offset, oraub8 amount, ub2 csid, ub1 csfrm)
{
	sword res = OCICALL(OCILobWrite2(_conn._svc_ctx, _conn._env._errh, _loc,
	                                 &amount, /* *byte_amtp - The number of bytes to write to the database. For CLOB and NCLOB it is used only when char_amtp is zero. */
	                                 NULL, /* *char_amtp - The maximum number of characters to write to the database. */
	                                 offset,
	                                 (dvoid*)bufp, buflen,
	                                 OCI_ONE_PIECE, /* ub1 piece */
	                                 NULL, /* dvoid* ctxp */
	                                 NULL, /* sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,oraub8*len,ub1*piece) */
	                                 csid, csfrm));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

oraub8	SqlClob::write_append(const dvoid* bufp, oraub8 buflen, oraub8 amount, ub2 csid, ub1 csfrm)
{
	sword res = OCICALL(OCILobWriteAppend2(_conn._svc_ctx, _conn._env._errh, _loc,
	                                       &amount, /* *byte_amtp - The number of bytes to write to the database. For CLOB and NCLOB it is used only when char_amtp is zero. */
	                                       NULL,    /* *char_amtp - The maximum number of characters to write to the database. */
	                                       (dvoid*)bufp, buflen,
	                                       OCI_ONE_PIECE, /* ub1 piece*/
	                                       NULL, /* dvoid* ctxp */
	                                       NULL, /* sb4 (*cbfp)(dvoid*ctxp,dvoid*bufp,oraub8*len,ub1*piece) */
	                                       csid, csfrm));
	oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
	return amount;
};

/*
 * NOTE: !! offset has to be >= 1. Oracle's LOBs start at 1st byte.
 */
oraub8	SqlClob::read(dvoid* bufp, oraub8 buflen, oraub8 offset, oraub8 amount, oraub8 *chars, ub2 csid, ub1 csfrm)
{
	oraub8 char_amt = 0;
	bool done = false;
	while(!done)
	{
		sword res = OCICALL(OCILobRead2(
		                            _conn._svc_ctx,
		                            _conn._env._errh,
		                            _loc,
		                            &amount,
		                            &char_amt,
		                            offset,
		                            bufp,
		                            buflen,
		                            OCI_ONE_PIECE, /* ub1 piece */
		                            NULL, /* dvoid* ctxp */
		                            NULL, /* sb4 (*cbfp)(dvoid*ctxp,CONST dvoid*bufp,oraub8*len,ub1*piece) */
		                            0 /* csid */,
		                            0 /* csfrm */
		                    ));
		if( res != OCI_SUCCESS )
		{
			sb4 errorcode;
			sword res2 = OCICALL(OCIErrorGet(_conn._env._errh, 1, NULL, &errorcode, NULL, 0, OCI_HTYPE_ERROR));
			assert(res2 == OCI_SUCCESS);

			if(errorcode == 24804) // ORA-24804: Lob read/write functions called while another OCI LOB read/write streaming is in progress
			{
				std::cerr << "ORA-24804: Lob read/write functions called while another OCI LOB read/write streaming is in progress" << std::endl;
				MSLEEP(100);
			}
			else
			{
				oci_check_error(__TROTL_HERE__, _conn._env._errh, res);
			}
		}
		else
		{
			done = true;
		}
	}

	if(chars)
		*chars = char_amt;
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
