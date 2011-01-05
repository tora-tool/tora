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

#include "trotl_string.h"

namespace trotl {

Util::RegisterInFactory<BindParChar,    BindParFactTwoParmSing> regBindChar("char");
Util::RegisterInFactory<BindParVarchar, BindParFactTwoParmSing> regBindVarchar("varchar");

Util::RegisterInFactory<BindParVarchar, DefineParFactTwoParmSing, int> regDefineChar1(SQLT_CHR);
Util::RegisterInFactory<BindParChar,    DefineParFactTwoParmSing, int> regDefineChar2(SQLT_AFC);
Util::RegisterInFactory<BindParVarchar, DefineParFactTwoParmSing, int> regDefineVarchar(SQLT_STR);
  
Util::RegisterInFactory<BindParRaw, DefineParFactTwoParmSing, int> regDefineRaw(SQLT_BIN);

Util::RegisterInFactory<BindParLong, DefineParFactTwoParmSing, int> regDefineLong(SQLT_LNG);
	
	void BindParLong::fetch_hook(ub4 iter, ub4 idx, ub1 piece, ub4 alen, sb2 ind)
	{
		if(piece == OCI_FIRST_PIECE)
		{
			s.at(iter)->clear();
			s.at(iter)->str("");
		}
		*s.at(iter) << std::string((char*)valuep, min(g_OCIPL_MAX_LONG - (unsigned)s.at(iter)->tellp(), alen));
	}

	// This is abandonned piece of code. for some unknown reason callback does not work well on LONG datatype
	// maybe it will be usefull for some other datatypes
	// #define PIECE_SIZE 1000	
	// #define DATA_SIZE 5000
	// 	static text nextpiece[DATA_SIZE];
	// void BindParLong::define_hook()
	// {
	// 	// std::cout << "valuep:" << valuep << std::endl;
	// 	// sword res = OCICALL(OCIDefineDynamic(defnpp, _stmt._errh, (void*)this, (OCICallbackDefine)cdf_fetch_buffer));
	// 	// oci_check_error(__TROTL_HERE__, _env, res);
	// }

	// sb4 BindParLong::cdf_fetch_buffer(dvoid *ctx, OCIDefine *defnp, ub4 iter, dvoid **bufpp, ub4 **alenpp, ub1 *piecep, dvoid **indpp, ub2 **rcpp)
	// {
	// 	BindParLong *me = (BindParLong*)ctx;
	// 	std::cout << "cdf_fetch_buffer called" << std::endl
	// 		  << "dvoid *ctx = "        << ctx   << std::endl
	// 		  << "OCIDefine *defnp = " << defnp << std::endl 
	// 		  << "ub4 iter = " << iter << std::endl 
	// 		  << "dvoid **bufpp = " << bufpp << std::endl 		  
	// 		  << "ub4 **alenpp = " << alenpp << " " << *alenpp << " " << (*alenpp ? **alenpp:0) << std::endl 
	// 		  << "ub1 *piecep = " << piecep << " " << (unsigned)*piecep << std::endl 
	// 		  << "dvoid **indpp =" << indpp << std::endl 
	// 		  << "ub2 **rcpp = " << rcpp << std::endl
	// 		  << "me->valuep = " << (char*)me->valuep << std::endl;
	// 	printf("Getting OCI_%s_PIECE .... \n",
	// 	       (piecep[0]==(ub1)OCI_ONE_PIECE) ? "ONE" :
	// 	       (piecep[0]==(ub1)OCI_FIRST_PIECE) ? "FIRST" :
	// 	       (piecep[0]==(ub1)OCI_NEXT_PIECE) ? "NEXT" :
	// 	       (piecep[0]==(ub1)OCI_LAST_PIECE) ? "LAST" : "???");
	// 	if(me->valuep)
	// 		std::cout << (char*)me->valuep << std::endl;
	// 	if(*bufpp)
	// 		std::cout << (char*)*bufpp << std::endl;
	// 	std::cout << "================================================================================" << std::endl;
		
	// 	static ub4  len = DATA_SIZE;
	// 	static sb2  ind = 0;
	// 	static boolean glGetInd = 0;          /* global var to control return of ind */
	// 	static ub2  rc = 0;
	// 	static boolean glGetRc = 0;           /* global var to control return of rc */
		
	// 	int i;
		
	// 	//*bufpp = (dvoid *) me->valuep;
	// 	*bufpp = (dvoid *) nextpiece;

	// 	if(*bufpp)
	// 	{
	// 		me->s.at(iter)->clear();
	// 		me->s.at(iter)->str("");
				
	// 		*me->s.at(iter) << (char*)*bufpp;
	// 	}
		
	// 	/*verify the contents of fetched pieces*/
	// 	// if(piecep[0]==(ub1)OCI_FIRST_PIECE)
	// 	// 	;
		
	// 	if ((*piecep) == (ub1)OCI_ONE_PIECE)
	// 	 	*piecep = OCI_FIRST_PIECE;

	// 	//if ((*piecep) == (ub1)OCI_FIRST_PIECE)
	// 	*piecep = OCI_LAST_PIECE;

	// 	//len = me->value_sz;
	// 	//*alenpp = (ub4*)me->alenp;
	// 	//*indpp  = me->indp;
		
	// 	len = PIECE_SIZE;
	// 	*alenpp = &len;
		
	// 	ind = 0;
	// 	if (glGetInd)
	// 		*indpp  = (dvoid *) &ind;
	// 	else
	// 		*indpp = (dvoid *)0;
		
	// 	rc = 0;
	// 	if (glGetRc)
	// 		*rcpp  = (ub2 *) &rc;
	// 	else
	// 		*rcpp = (ub2 *)0;
	// 	return OCI_CONTINUE;
	// }
};

