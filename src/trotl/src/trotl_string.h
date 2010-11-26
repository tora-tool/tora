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

#ifndef TROTL_STRING_H_
#define TROTL_STRING_H_

#ifdef WIN32
#include "trotl_parser.h"
#include <stdio.h>
#define snprintf _snprintf
//#define gmtime gmtime_s
//#define localtime localtime_s
#endif

#include "trotl_export.h"
#include "trotl_base.h"
#include "trotl_stat.h"

namespace trotl {

/* Another template specialization - wraps null terminated string and maps it to tstring
 **/
// VARCHAR2
// If the value_sz parameter is greater than zero, Oracle obtains the bind variable value
// by reading exactly that many bytes, starting at the buffer address in your program.
// Trailing blanks are stripped, and the resulting value is used in the SQL statement or
// PL/SQL block. If, in the case of an INSERT statement, the resulting value is longer
// than the defined length of the database column, the INSERT fails, and an error is
// returned.
struct TROTL_EXPORT BindParVarchar: public SqlStatement::BindPar
{
  	BindParVarchar(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : SqlStatement::BindPar(pos, stmt, ct)
	{
		/* amount of bytes =  (string length +1 ) * (array length) */
		valuep = (void**) calloc(_cnt, ct._width + 1);
		alenp = (ub2*) calloc(_cnt, sizeof(ub2));
		
		dty = SQLT_STR;
		value_sz = ct._width + 1;
		type_name = typeid(tstring).name();
	}

	BindParVarchar(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl) : SqlStatement::BindPar(pos, stmt, decl)
	{
		// amount of bytes =  (string length +1 ) * (array length)
		valuep = (void**) calloc(decl.bracket[0]+1, decl.bracket[1]);
		alenp = (ub2*) calloc(_cnt, sizeof(ub2));
		
		dty = SQLT_STR;
		value_sz = decl.bracket[0]+1;
		type_name = typeid(tstring).name();
	}

	~BindParVarchar()
	{}

	virtual tstring get_string(unsigned int row) const
	{	  
	  return is_null(row) ? "NULL" : tstring(((char*)valuep)+(row * value_sz));
	}

protected:
	BindParVarchar(const BindParVarchar &other);
};

/* wrapper for Oracle CHAR(X) datatype.
 **/
struct TROTL_EXPORT BindParChar: public SqlStatement::BindPar
{
	BindParChar(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : SqlStatement::BindPar(pos, stmt, ct)
	{
		valuep = (void**) calloc(_cnt, ct._width + 1); // TODO +1 ?? why?
		alenp = (ub2*) calloc(_cnt, sizeof(ub2));
		
		dty = SQLT_CHR;
		value_sz = ct._width;
		type_name = typeid(tstring).name();
	}

	BindParChar(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): SqlStatement::BindPar(pos, stmt, decl)
	{
		valuep = (void**) calloc(decl.bracket[1], decl.bracket[0]);
		alenp = (ub2*) calloc(_cnt, sizeof(ub2));
		
		dty = SQLT_STR;	/* use STR_CHR even if placeholder defined as "char" */
		value_sz = decl.bracket[0];
		type_name = typeid(tstring).name();
	}

	~BindParChar()
	{}

	virtual tstring get_string(unsigned int row) const
	{
		if(!indp[row])
			return tstring(((char*)valuep)+(row * value_sz), (_bind_type != DEFINE_SELECT ? alenp[row] : value_sz) );		
		return "";
	}

protected:	
	BindParChar(const BindParChar &other);
};

/* wrapper for Oracle RAW(X) datatype.
 **/
struct TROTL_EXPORT BindParRaw: public SqlStatement::BindPar
{
	BindParRaw(unsigned int pos, SqlStatement &stmt, ColumnType &ct) : SqlStatement::BindPar(pos, stmt, ct)
	{
		// amount of bytes =  (string length +1 ) * (array length)
		valuep = (void**) calloc(_cnt, ct._width + 1);
		alenp = (ub2*) calloc(_cnt, sizeof(ub2));
		
		dty = SQLT_BIN;
		value_sz = ct._width;
		type_name = ct.get_type_str();
	}

	BindParRaw(unsigned int pos, SqlStatement &stmt, BindVarDecl &decl): SqlStatement::BindPar(pos, stmt, decl)
	{
		valuep = (void**) calloc (decl.bracket[1], decl.bracket[0]);
		alenp = (ub2*) calloc(_cnt, sizeof(ub2));
		
		dty = SQLT_BIN;
		value_sz = decl.bracket[0];
		type_name = "RAW";
	}

	~BindParRaw()
	{}
	
	virtual tstring get_string(unsigned int row) const
	{
		if(!indp[row])
		{
			tostream ret;
//			ret << "0x";
			for(unsigned i=row*value_sz; i<(row+1)*value_sz; ++i)
			{
				char buffer[4];
				snprintf(buffer, sizeof(buffer), "%.2X", ((unsigned char*)valuep)[i]);
				ret << buffer;
			}
			return ret.str();
		}

		return "";
	}

protected:
		BindParRaw(const BindParRaw &other);
};

};

#endif
