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
#endif

#include "trotl_rid.h"

namespace trotl {

// // Register Bind datatypes in factory(Bind - PL/SQL)
Util::RegisterInFactory<BindParRid, BindParFactTwoParmSing> regBindRid("rid");
Util::RegisterInFactory<BindParRid, BindParFactTwoParmSing> regBindURid("urid");


// // Register Bind datatypes in factory(Define - SELECT)
Util::RegisterInFactory<BindParRid, DefineParFactTwoParmSing, int> regDefineNumberRid(SQLT_RDD);

BindParRid::BindParRid(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): BindPar(pos, stmt, decl)
{
	//valuep = new unsigned char [ decl.bracket[1] * ( OCI_NUMBER_SIZE + 1) ];
	valuep = malloc( _cnt * sizeof(OCIRowid*) );
	////memset(valuep, 0x5a, _cnt * sizeof(OCIRowid*) );
	
	dty = SQLT_RDD;
	value_sz = sizeof(OCIRowid*);
	type_name = "ROWID";

	descAlloc();
};

BindParRid::BindParRid(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : BindPar(pos, stmt, ct)
{
	//valuep = new unsigned char [ _cnt * ( sizeof(OCIRowid*) ) ];
	valuep = malloc( _cnt * sizeof(OCIRowid*) );
	////memset(valuep, 0x5a, _cnt * sizeof(OCIRowid*) );
	
	dty = SQLT_RDD;
	value_sz = sizeof(OCIRowid*);
	type_name = ct.get_type_str();
	
	descAlloc();
}
	
BindParRid::~BindParRid()
{
	descFree();
	if(valuep)
	{
		free(valuep);
		valuep = NULL;
	}
}

void BindParRid::descAlloc(void)
{
	for(unsigned i=0; i<_cnt; ++i)
	{
		sword res = OCICALL(OCIDescriptorAlloc(_env, &((void**)valuep)[i], OCI_DTYPE_ROWID, 0, NULL));
		oci_check_error(__HERE__, _env, res);
	}
}

void BindParRid::descFree(void)
{
	for(unsigned i=0; i<_cnt; ++i)
	{
		if(((void**)valuep)[i])
		{
			sword res = OCICALL(OCIDescriptorFree( ((void**)valuep)[i], OCI_DTYPE_ROWID));
			oci_check_error(__HERE__, _env, res);
		}
	}
}

// TODO add ind skip here
void BindParRid::bind_hook(SqlStatement &stmt)
{
//	sword res = OCICALL(OCIBindArrayOfStruct(bindp, stmt._errh, sizeof(OCIRowid*), 0, 0, 0 ));
//	oci_check_error(__HERE__, stmt._errh, res);
}

// TODO add ind skip here
void BindParRid::define_hook(SqlStatement &stmt)
{
//	sword res = OCICALL(OCIDefineArrayOfStruct(defnpp , _env._errh, sizeof(OCIRowid*), 0, 0, 0 ));
//	oci_check_error(__HERE__, _stmt._errh, res);
}

tstring BindParRid::get_string(unsigned int i) const
{
	if(indp[i] == OCI_IND_NULL)
		return "";
	OraText buffer[4001];
	ub2 outbfl = sizeof(buffer);
	sword res = OCICALL(OCIRowidToChar(((OCIRowid**)valuep)[i], buffer, &outbfl, _stmt._errh));
	oci_check_error(__HERE__, _stmt._errh, res);
	return std::string((char*)buffer, outbfl);
}

SqlRid::SqlRid(OciEnv &env) : _env(env)
{
	sword res = OCICALL(OCIDescriptorAlloc(_env, (dvoid**)&_rid, OCI_DTYPE_ROWID, 0, NULL));
	oci_check_error(__HERE__, _env, res);
};

SqlRid::~SqlRid()
{
	sword res = OCICALL(OCIDescriptorFree(_rid, OCI_DTYPE_ROWID));
	oci_check_error(__HERE__, _env, res);
};

};
