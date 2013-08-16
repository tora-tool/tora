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

//TODO - SQLT_VST when should I use this datatype?

#ifndef TROTL_MISC_H_
#define TROTL_MISC_H_

#include "trotl_common.h"
#include "trotl_export.h"
#include "trotl_handle.h"
#include "trotl_var.h"
#include "trotl_describe.h"
#include "trotl_parser.h"

namespace trotl
{

/* Misc datatype - temporary solution
   This class represents any datatype, whose value can be converted into a string
*/
struct TROTL_EXPORT BindParMisc: public BindPar
{
	BindParMisc(unsigned int pos, SqlStatement &stmt, DescribeColumn* ct);
	BindParMisc(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl);

	virtual ~BindParMisc()
	{}

	virtual tstring get_string(unsigned int row) const
	{
		return is_null(row) ? "NULL" : tstring(((char*)valuep)+(row * value_sz));
	}

protected:
	BindParMisc(const BindParMisc &other);
};


};

#endif
