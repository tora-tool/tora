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

#ifdef WIN32
#include "trotl_parser.h"
#include <stdio.h>
#define snprintf _snprintf
//#define gmtime gmtime_s
//#define localtime localtime_s
#endif

#include "trotl_int.h"

namespace trotl {

// Register Bind datatypes in factory(Bind - PL/SQL) 
// Util::RegisterInFactory<BindParInt,    BindParFactTwoParmSing> regBindInt("int");
// Util::RegisterInFactory<BindParUInt,   BindParFactTwoParmSing> regBindUInt("uint");
// Util::RegisterInFactory<BindParLong,   BindParFactTwoParmSing> regBindLong("long");
// Util::RegisterInFactory<BindParULong,  BindParFactTwoParmSing> regBindULong("ulong");
// Util::RegisterInFactory<BindParDouble, BindParFactTwoParmSing> regBindDouble("double");
// Util::RegisterInFactory<BindParFloat,  BindParFactTwoParmSing> regBindFloat("float");
// Util::RegisterInFactory<BindParNumber, BindParFactTwoParmSing> regBaseNumber("number");
Util::RegisterInFactory<prefferedNumericType, BindParFactTwoParmSing> regBindInt("int");
Util::RegisterInFactory<prefferedNumericType, BindParFactTwoParmSing> regBindUInt("uint");
Util::RegisterInFactory<prefferedNumericType, BindParFactTwoParmSing> regBindLong("long");
Util::RegisterInFactory<prefferedNumericType, BindParFactTwoParmSing> regBindULong("ulong");
Util::RegisterInFactory<prefferedNumericType, BindParFactTwoParmSing> regBindDouble("double");
Util::RegisterInFactory<prefferedNumericType, BindParFactTwoParmSing> regBindFloat("float");
Util::RegisterInFactory<prefferedNumericType, BindParFactTwoParmSing> regBaseNumber("number");

// Register Bind datatypes in factory(Define - SELECT)
//See note at trotl_var.h:BindParNumber 
Util::RegisterInFactory<prefferedNumericType, DefineParFactTwoParmSing, int> regDefineNumberNUM(SQLT_NUM);
Util::RegisterInFactory<prefferedNumericType, DefineParFactTwoParmSing, int> regDefineNumberVNU(SQLT_VNU);

tstring BindParNumber::get_string(unsigned int row) const
{
	if(!indp[row])
	{
		text str_buf[64];
		ub4 str_len = sizeof(str_buf) / sizeof(*str_buf);

		sword res = OCICALL(OCINumberToText(
				_env._errh,
				(OCINumber*) &((char*)valuep)[row * value_sz ],		//(OCINumber*)valuep,
				(const oratext*) g_TROTL_DEFAULT_NUM_FTM,
				strlen(g_TROTL_DEFAULT_NUM_FTM),
				0,							// CONST OraText *nls_params,
				0,							// ub4 nls_p_length,
				(ub4*)&str_len,
				str_buf
		));
		oci_check_error(__TROTL_HERE__, _env._errh, res);

		return tstring((const char*)str_buf, str_len);
	} else {
		return "";
	}
}

SqlNumber::SqlNumber(OciEnv &env, const char* str, const char* fmt, const char* nls_fmt) : _env(env)
{
	//while(str && istspace(*str)) ++str; 

	if (str && *str) {
		sword res = OCICALL(OCINumberFromText(_env._errh,
						      (const text*)str, (ub4)strlen(str),
						      (const text*)fmt, (ub4)strlen(fmt),
						      (const text*)nls_fmt, (ub4)strlen(nls_fmt),
						      (OCINumber*)&_val));
		oci_check_error(__TROTL_HERE__, _env._errh, res);
		_ind.set();
	} else
		_ind.clear();
}
  
//SqlNumber::SqlNumber(const oraclenumber* pnum)
//{
//	memcpy(&_val.exp, &pnum->exp, pnum->len);
//	_val.len = pnum->len;
//	_ind.set();
//}

//tstring number_to_str(const oraclenumber& val, OCIError* errh, const char* fmt, const char* nls_fmt)
//{
//  text buffer[128];
//  ub4 l = sizeof(buffer);
//  
//  sword res = OCICALL(OCINumberToText(errh,
//				(OCINumber*)&val,
//				(const text*)fmt, (ub4)strlen(fmt),
//				(const text*)nls_fmt, (ub4)strlen(nls_fmt),
//				&l, buffer));
//  
//  oci_check_error(__TROTL_HERE__, errh, res);
//  
//  //	const char* p = (const char*)buffer;
//  //	while(istspace(*p))
//  //		++p;
//
//  return (const char*)buffer; //p
//}

}
