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

#ifndef TROTL_EXTENDED_PARSER_H_
#define TROTL_EXTENDED_PARSER_H_

#include <string>
#include <sstream>
#include <algorithm>

#define BOOST_SPIRIT_USE_OLD_NAMESPACE
#define BOOST_SPIRIT_DEBUG
#include <boost/bind.hpp>
//#include <boost/spirit.hpp>
#include <boost/spirit/include/classic.hpp>
//#include <boost/spirit/actor/clear_actor.hpp>
#include <boost/spirit/include/classic_clear_actor.hpp>
#include <boost/function.hpp>

#include "trotl_export.h"
#include "trotl_common.h"
#include "trotl_parser.h"

namespace trotl {

//using namespace std;
//using namespace boost::spirit;

class TROTL_EXPORT ExtendedPlsqlParser 
{
public:
	bool parse(const tstring &sql);
	ExtendedPlsqlParser() : _lastlen(0) { };

	static tstring colorword(const tstring &i, int color);

	//Colors - as difined in linux console
	enum linux_console_attrs {	  
		RESET	= 0,
		BRIGHT = 1,
		DIM = 2,
		UNDERLINE	= 3,
		BLINK	= 4,
		REVERSE = 7,
		HIDDEN = 8
	};

	enum linux_console_colors {
		BLACK = 0,
		RED = 1,
		GREEN = 2,
		YELLOW = 3,
		BLUE = 4,
		MAGENTA = 5,
		CYAN = 6,
		WHITE = 7
	};

	void token_callback(const tstring name, const char *begin, const char *end)
	{
		const tstring str(begin,end);
		if(_bindvar._inside_bindvar || str.empty())
			return;

		if(name == "character")
			_colored << colorword(str, ExtendedPlsqlParser::WHITE);
		else if(name == "word")
			_colored << colorword(str, ExtendedPlsqlParser::GREEN);
		else if(name == "keyword")
			_colored << colorword(str, ExtendedPlsqlParser::BLUE);
		else if(name == "number")
			_colored << colorword(str, ExtendedPlsqlParser::BLUE);
		else if(name == "eq")
			_colored << colorword(str, ExtendedPlsqlParser::WHITE);
		else if(name == "qstring")
			_colored << colorword(str, ExtendedPlsqlParser::YELLOW);
		else if(name == "comment")
			_colored << str;
		else if(name == "space")
			_colored << '@' << str << '@';		
		else
			_colored << str;
		_non_colored << str;
	}

	void bindvar_callback(const tstring name, const char *begin, const char * end)
	{
		const tstring str(begin, end);
		tstring bindname(begin, end);
		
		if(str.empty())
			return;

		/* replace OTL specific extension with spaces */
		tstring::iterator l = find(bindname.begin(), bindname.end(), '<');
		tstring::iterator r = find(bindname.begin(), bindname.end(), '>');
		std::fill(l, ++r, ' ');

		_colored << colorword(str, ExtendedPlsqlParser::RED);

		// Do not try to find duplicate bindvar names - we use OciBindbyPos
		//if( find(_bindvars.begin(), _bindvars.end(), _bindvar.bindname) == _bindvars.end())
		_bindvars.push_back(_bindvar);

		_bindvar.reset();
		_non_colored << bindname;
	}

	tstring getColored() const { return _colored.str(); };
	tstring getNonColored() const { return _non_colored.str(); };
	
#ifdef WIN32
//	typedef TROTL_EXPORT std::vector<BindVarDecl>;
#endif

	std::vector<BindVarDecl> _bindvars;
private:	
	tostream _non_colored, _colored; 
	int _lastlen;
	BindVarDecl _bindvar;
};

};

#endif
