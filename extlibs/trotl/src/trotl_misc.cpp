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
#include "trotl_misc.h"
#include "trotl_describe.h"

namespace trotl
{

Util::RegisterInFactory<BindParMisc, DefineParFactTwoParmSing, int> regDefineIntervalDS(SQLT_INTERVAL_DS);
Util::RegisterInFactory<BindParMisc, DefineParFactTwoParmSing, int> regDefineIntervalYM(SQLT_INTERVAL_YM);

BindParMisc::BindParMisc(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct): BindPar(pos, stmt, ct)
{
	valuep = (void**) calloc(_cnt, 128);
	alenp = (ub2*) calloc(_cnt, sizeof(ub2));

	dty = SQLT_STR;
	value_sz = 128;
	_type_name = typeid(tstring).name();
}

BindParMisc::BindParMisc(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : BindPar(pos, stmt, decl)
{
	// an assumtion is that remaining datatypes can hit into 128 chars
	valuep = (void**) calloc(decl.bracket[1], 128);
	alenp = (ub2*) calloc(_cnt, sizeof(ub2));

	dty = SQLT_STR;
	value_sz = 128;
	_type_name = typeid(tstring).name();
}

};

