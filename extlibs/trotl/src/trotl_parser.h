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

#ifndef TROTL_PARSER_H_
#define TROTL_PARSER_H_

#include "trotl_export.h"
#include "trotl_common.h"

#include <vector>

namespace trotl
{

//using namespace std;
//using namespace boost::spirit;

//Class holding bind variable data
struct TROTL_EXPORT BindVarDecl
{
public:
	void setname(tstring s, const char *begin, const char *end)
	{
		tstring str(begin,end);

		if(s == "bindname")
		{
			bindname = str;
			_inside_bindvar = true;
		}
		else if(s == "bindtype")
		{
			bindtype = str;
			_bracket_index = 0;
		}
		else if(s == "inout")
		{
			inout = str;
			_bracket_index = 1;
		}
		else if(s == "bracket")
		{
			bracket[_bracket_index] = (atoi(str.c_str())!=0 ? atoi(str.c_str()) : 1);
		}
	};

	void reset()
	{
		bindname.resize(0);
		bindtype.resize(0);
		inout.resize(0);
		bracket[0] = bracket[1] = 1;
		_inside_bindvar = false;
		inout = "in";  // default value if inout ommited(OTL compat)
	};

	void print() const
	{
		// get_log().ts( tstring(__HERE_SHORT__))
		//std::cout << ':' << bindname << '<' << bindtype << bracket[0] << ',' << inout << bracket[1] << '>' << std::endl;
	}

	bool operator==(tstring _varname)
	{
		return _varname == bindname;
	};

	BindVarDecl():_inside_bindvar(false), _bracket_index(0)
	{
		reset();
	};

	tstring bindname, bindtype, inout;
	bool _inside_bindvar;
	unsigned int _bracket_index, bracket[2];
};

class TROTL_EXPORT SimplePlsqlParser
{
public:
	bool parse(const tstring &sql);
	SimplePlsqlParser() : _lastlen(0) { };

	static tstring colorword(const tstring &i, int color);

	//Colors - as difined in linux console
	enum linux_console_attrs
	{
		RESET	= 0,
		BRIGHT = 1,
		DIM = 2,
		UNDERLINE	= 3,
		BLINK	= 4,
		REVERSE = 7,
		HIDDEN = 8
	};

	enum linux_console_colors
	{
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
			_colored << colorword(str, SimplePlsqlParser::WHITE);
		else if(name == "word" || name == "qword")
			_colored << colorword(str, SimplePlsqlParser::GREEN);
		else if(name == "number")
			_colored << colorword(str, SimplePlsqlParser::BLUE);
		else if(name == "eq")
			_colored << colorword(str, SimplePlsqlParser::WHITE);
		else if(name == "string")
			_colored << colorword(str, SimplePlsqlParser::YELLOW);
		else if(name == "comment")
			_colored << colorword(str, SimplePlsqlParser::BLUE);
		//_colored << str;

		_non_colored << str;
	}

	void bindvar_callback(const tstring name, const char *begin, const char * end);

	const tstring getColored() const
	{
		return _colored.str();
	};
	const tstring getNonColored() const
	{
		return _non_colored.str();
	};

	std::vector<BindVarDecl> _bindvars;
private:
	tostream _non_colored, _colored;
	int _lastlen;
	BindVarDecl _bindvar;
};

};

#endif
