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

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_rid.h"
#include "trotl_describe.h"
#include "trotl_parser.h"
#include "trotl_stat.h"

namespace trotl
{

// // Register Bind datatypes in factory(Bind - PL/SQL)
Util::RegisterInFactory<BindParRid, BindParFactTwoParmSing> regBindRid("rid");
Util::RegisterInFactory<BindParRid, BindParFactTwoParmSing> regBindURid("urid");

// // Register Bind datatypes in factory(Define - SELECT)
Util::RegisterInFactory<BindParRid, DefineParFactTwoParmSing, int> regDefineRid(SQLT_RDD);

BindParRid::BindParRid(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindPar(pos, stmt, ct)
{
	_errh.alloc(stmt._env);
	valuep = (void**) calloc(_cnt, sizeof(OCIRowid*));
	value_sz = sizeof(OCIRowid*);
	dty = SQLT_RDD;
	_type_name = ct->typeName();
	descAlloc();
}

BindParRid::BindParRid(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): BindPar(pos, stmt, decl)
{
	_errh.alloc(stmt._env);
	valuep = (void**) calloc(_cnt, sizeof(OCIRowid*));
	value_sz = sizeof(OCIRowid*);
	dty = SQLT_RDD;
	_type_name = "ROWID";
	descAlloc();
};

void BindParRid::descAlloc(void)
{
	for(unsigned i=0; i<_cnt; ++i)
	{
		//sword res = OCICALL(OCIDescriptorAlloc(_env, &((void**)valuep)[i], OCI_DTYPE_ROWID, 0, NULL));
		sword res = OCICALL(OCIDescriptorAlloc(_env, &valuep[i], OCI_DTYPE_ROWID, 0, NULL));
		oci_check_error(__TROTL_HERE__, _env, res);
	}
	// sword res = OCICALL(OCIArrayDescriptorAlloc(
	// 					    (void *)_env,
	// 					    (void **)&valuep,
	// 					    OCI_DTYPE_ROWID,
	// 					    _cnt,
	// 					    0,
	// 					    (void **)0));	
	// oci_check_error(__TROTL_HERE__, _env, res);
}

void BindParRid::descFree(void)
{
	// for(unsigned i=0; i<_cnt; ++i)
	// {
	// 	if(((void**)valuep)[i])
	// 	{
	// 		sword res = OCICALL(OCIDescriptorFree( ((void**)valuep)[i], OCI_DTYPE_ROWID));
	// 		oci_check_error(__TROTL_HERE__, _env, res);
	// 	}
	// }
}

// TODO add ind skip here
void BindParRid::bind_hook()
{
//	sword res = OCICALL(OCIBindArrayOfStruct(bindp, stmt._errh, sizeof(OCIRowid*), 0, 0, 0 ));
//	oci_check_error(__TROTL_HERE__, stmt._errh, res);
}

// TODO add ind skip here
void BindParRid::define_hook()
{
//	sword res = OCICALL(OCIDefineArrayOfStruct(defnpp , _env._errh, sizeof(OCIRowid*), 0, 0, 0 ));
//	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
}

void BindParRid::fetch_hook(ub4 iter, ub4 idx, ub1 piece, ub4, sb2 ind)
{
	descFree();
	descAlloc();
}
	
tstring BindParRid::get_string(unsigned int i) const
{
	if(indp[i] == OCI_IND_NULL)
		return "";
	OraText buffer[4001];
	ub2 outbfl = sizeof(buffer);
	sword res = OCICALL(OCIRowidToChar(((OCIRowid**)valuep)[i], buffer, &outbfl, _stmt._errh));
	oci_check_error(__TROTL_HERE__, _stmt._errh, res);
	return tstring((char*)buffer, outbfl);
}

SqlRid::SqlRid(OciEnv &env) : _env(env)
{
	sword res = OCICALL(OCIDescriptorAlloc(_env, (dvoid**)&_rid, OCI_DTYPE_ROWID, 0, NULL));
	oci_check_error(__TROTL_HERE__, _env, res);
};

SqlRid::~SqlRid()
{
	sword res = OCICALL(OCIDescriptorFree(_rid, OCI_DTYPE_ROWID));
	oci_check_error(__TROTL_HERE__, _env, res);
};

};
