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

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_int.h"
#include "trotl_describe.h"
#include "trotl_stat.h"

namespace trotl
{

// Register Bind datatypes in factory(Bind - PL/SQL)
Util::RegisterInFactory<BindParNumber, BindParFactTwoParmSing> regBindInt("int");
Util::RegisterInFactory<BindParNumber, BindParFactTwoParmSing> regBindUInt("uint");
Util::RegisterInFactory<BindParNumber, BindParFactTwoParmSing> regBindLong("long");
Util::RegisterInFactory<BindParNumber, BindParFactTwoParmSing> regBindULong("ulong");
Util::RegisterInFactory<BindParNumber, BindParFactTwoParmSing> regBindDouble("double");
Util::RegisterInFactory<BindParNumber, BindParFactTwoParmSing> regBindFloat("float");
Util::RegisterInFactory<BindParNumber, BindParFactTwoParmSing> regBaseNumber("number");

// Register Bind datatypes in factory(Define - SELECT)
//See note at trotl_var.h:BindParNumber
Util::RegisterInFactory<BindParNumber, DefineParFactTwoParmSing, int> regDefineNumberNUM(SQLT_NUM);
Util::RegisterInFactory<BindParNumber, DefineParFactTwoParmSing, int> regDefineNumberVNU(SQLT_VNU);

tstring TROTL_EXPORT BindParNumber::get_string(unsigned int row) const
{
	if(!indp[row])
	{
		text str_buf[64];
		ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);

		sword res = OCICALL(OCINumberToText(
		                            _env._errh,
		                            (OCINumber*) &((char*)valuep)[row * value_sz ],		//(OCINumber*)valuep,
		                            (const oratext*) g_TROTL_DEFAULT_NUM_FTM,
		                            (ub4)strlen(g_TROTL_DEFAULT_NUM_FTM),
		                            0,							// CONST OraText *nls_params,
		                            0,							// ub4 nls_p_length,
		                            (ub4*)&str_len,
		                            str_buf
		                    ));
		oci_check_error(__TROTL_HERE__, _env._errh, res);

		return tstring((const char*)str_buf, str_len);
	}
	else
	{
		return "";
	}
}

TROTL_EXPORT BindParNumber::BindParNumber(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct) : BindPar(pos, stmt, ct)
{
	_errh.alloc(stmt._env);
	valuep = (void**) calloc(_cnt, (size_t)OCI_NUMBER_SIZE );

	dty =  SQLT_VNU; //dty = SQLT_NUM;
	value_sz = OCI_NUMBER_SIZE;
	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub2*)rlenp)[i] = (ub2) value_sz;
	}
	_type_name = ct->typeName();
}

TROTL_EXPORT BindParNumber::BindParNumber(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): BindPar(pos, stmt, decl)
{
	_errh.alloc(stmt._env);
	valuep = (void**) calloc(_cnt, (size_t)OCI_NUMBER_SIZE );

	dty =  SQLT_VNU; //dty = SQLT_NUM;
	value_sz = OCI_NUMBER_SIZE;
	for(unsigned i = 0; i < _cnt; ++i)
	{
		((ub4*)rlenp)[i] = (ub4) value_sz;
	}

	_type_name = "NUMBER";
};


}
