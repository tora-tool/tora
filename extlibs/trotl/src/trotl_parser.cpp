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

#include "trotl_export.h"
#include "trotl_common.h"
#include "trotl_parser.h"

#include <boost/bind.hpp>
//#include <boost/spirit.hpp>
#include <boost/spirit/include/classic.hpp>
//#include <boost/spirit/actor/clear_actor.hpp>
#include <boost/spirit/include/classic_clear_actor.hpp>
#include <boost/function.hpp>

#include <algorithm>

namespace trotl
{

using namespace boost::spirit::classic;

bool SimplePlsqlParser::parse (const tstring &statement)
{
	static rule<> word_tm = (alpha_p >> *(alnum_p | ch_p('_') /*| ch_p('%')*/ | ch_p('#') | ch_p('$') ));
	static rule<> quoted_word_tm = ch_p('"') >> (*alnum_p|ch_p(' ')) >> *(~ch_p('"')) >> ch_p('"');
	static rule<> bind_tm = (alpha_p >> *(alnum_p | ch_p('_') | ch_p('.')));
	static rule<> unumber_tm = (+digit_p);
	static rule<> number_tm = (!ch_p('-')) >> unumber_tm;
	static rule<> float_tm = (number_tm >> !(ch_p('.') >> unumber_tm ));

#if 1
	static rule<> operator_tm = ( str_p("**") | str_p("NOT") |
	                              ch_p('+')   | ch_p('-') |
	                              ch_p('*')   | ch_p('/') |
	                              str_p("||") |
	                              ch_p('=')   | ch_p('<') |
	                              ch_p('>')   | str_p("<=") |
	                              str_p(">=") | str_p("<>") |
	                              str_p("!=") | str_p("~=") |
	                              str_p("^=") | str_p(":=")
	                            );
#else // use this branch if you have gcc 4.3.3 see gcc bug 36391
	static rule<> operator1_tm = ( str_p("**") | str_p("NOT") |
	                               ch_p('+')   | ch_p('-') |
	                               ch_p('*')   | ch_p('/') |
	                               str_p("||") | ch_p('='));
	static rule<> operator2_tm = ( ch_p('<') | ch_p('>') |
	                               str_p("<=") | str_p(">=") |
	                               str_p("<>") | str_p("!=") |
	                               str_p("~=") | str_p("^=") | str_p(":=") );
	static rule<> operator_tm = ( operator1_tm | operator2_tm );

#endif

	static rule<> delim_tm= ( str_p("..") | ch_p(';') | ',' | '(' | ')' |  '.' | '[' | ']' | blank_p | eol_p );
	//static rule<> char_tm2= ( /*ch_p('*') | ch_p('.') |*/
	//static rule<> char_tm3= ( ch_p('+') | ch_p('-') | ch_p('/') | ch_p('!') );
	//static rule<> char_tm4= ( /*ch_p('<') | ch_p('>') | c_p('%') |*/
	//static rule<> char_tm= ( char_tm1 | char_tm2 /*| char_tm3*/ | char_tm4 );

// 	  static rule<> char_tm= ( ch_p('=') | ';' | ',' | '|' |
// 				     ch_p('*') | '.' | '(' | ')' |
// 				     ch_p('+') | '-' | '/' | '!' |
// 				     ch_p('<') | '>' | '%' | blank_p | eol_p
// 				     );

	static rule<> appos_tm = (ch_p('\'') >> ch_p('\''));
	static rule<> string_tm = ch_p('\'') >> *(~ch_p('\'')|appos_tm) >> ch_p('\'');
	static rule<> inout_tm = str_p("inout") | str_p("out") | str_p("in");

	static rule<> label_tm = str_p("<<") >> word_tm >> str_p(">>");

	rule<> operator_cl = operator_tm
	                     [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)];

// 	  rule<> char_cl= char_tm
// 	  [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)];

	rule<> delim_cl= delim_tm
	                 [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)];

	rule<> number_cl = (float_tm|unumber_tm|number_tm)
	                   [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "number", _1, _2)];

	rule<> word_cl = (label_tm | ((word_tm|quoted_word_tm) >>
	                              !( ch_p('.') >> (word_tm|quoted_word_tm)) >>
	                              !( ch_p('.') >> (word_tm|quoted_word_tm)) >>
	                              !( ch_p('%') >> (word_tm|quoted_word_tm))
	                             ))
	                 [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "word", _1, _2)];

	rule<> quoted_word_cl = quoted_word_tm
	                        [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "qword", _1, _2)];

	rule<> string_cl = string_tm
	                   [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "string", _1, _2)];

	rule<> bracket_cl = ( ch_p('[') >> unumber_tm[boost::bind(&BindVarDecl::setname, boost::ref(_bindvar), "bracket", _1, _2)] >> ch_p(']') );

	rule<> bindvar = (ch_p(':') >>
	                  (bind_tm)[boost::bind(&BindVarDecl::setname, boost::ref(_bindvar), "bindname", _1, _2)] >>
	                  ch_p('<') >>
	                  (bind_tm)[boost::bind(&BindVarDecl::setname, boost::ref(_bindvar), "bindtype", _1, _2)] >>
	                  (!bracket_cl) >>
	                  !(ch_p(',') >>
	                    inout_tm[boost::bind(&BindVarDecl::setname, boost::ref(_bindvar), "inout", _1, _2)] >>
	                    (!bracket_cl)
	                   )>>
	                  ch_p('>'))
	                 [boost::bind(&SimplePlsqlParser::bindvar_callback, boost::ref(*this), "bindvar", _1, _2)];

	rule<> comment_cl = (confix_p("/*", *(anychar_p | ch_p(':') ) , "*/") | confix_p("--", *anychar_p, eol_p) | confix_p("PROMPT", *anychar_p, eol_p))
	                    [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "comment", _1, _2)];

	//	  rule<> endslash = (eol_p >> ch_p('/') >> (*blank_p) >> eol_p)
	//	  [boost::bind(&token_callback, boost::ref(_words), boost::ref(_bindvar), "endslash", _1, _2)];

	rule<> eq_cl = ( (ch_p(':') >> ch_p('=') ))
	               [boost::bind(&SimplePlsqlParser::token_callback, boost::ref(*this), "eq", _1, _2)];

	bool p = boost::spirit::classic::parse(statement.c_str(),
// Note: as_lower_d wont compile on 64bit linux if BOOST_VERSION < 1.37.0
#if (BOOST_VERSION >= 103700)
	                                       as_lower_d[(
#endif
	                                                       // Begin grammar
	                                                       *(comment_cl | word_cl | quoted_word_cl |
	                                                                       bindvar |operator_cl |string_cl| /*eq_cl |*/
	                                                                       number_cl | delim_cl)
#if (BOOST_VERSION >= 103700)
	                                                       )]
#endif
	                                       //,
	                                       //  End grammar
	                                       //space_p
	                                      ).full;

	for(unsigned itr2=0; itr2 < _bindvars.size(); ++itr2)
		_bindvars.at(itr2).print();
//	  std::cout << "Bindvar count: " << _bindvars.size() << std::endl;

	return p;
};


void SimplePlsqlParser::bindvar_callback(const tstring name, const char *begin, const char * end)
{
	const tstring str(begin, end);
	tstring bindname(begin, end);

	if(str.empty())
		return;

	/* replace OTL specific extension with spaces */
	tstring::iterator l = find(bindname.begin(), bindname.end(), '<');
	tstring::iterator r = find(bindname.begin(), bindname.end(), '>');
	std::fill(l, ++r, ' ');

	_colored << colorword(str, SimplePlsqlParser::RED);

	// Try to find duplicate bindvar names - even if we use OciBindbyPos
	if( find(_bindvars.begin(), _bindvars.end(), _bindvar.bindname) == _bindvars.end())
		_bindvars.push_back(_bindvar);

	_bindvar.reset();
	_non_colored << bindname;
}


tstring SimplePlsqlParser::colorword(const tstring &i, int color)
{
#ifdef __GNUC__
	char command[256], normal[256];
	int attr = BRIGHT;
	int bg = BLACK;
	tostream retval;
	// see: http://www.vias.org/linux-knowhow/lnag_05_05_04.html
	//	  sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, color + 30, bg + 40);
	if( color == SimplePlsqlParser::RED)
		sprintf(command, "%c[%d;%dm", 0x1B, color + 30, 1);
	else
		sprintf(command, "%c[%d;%dm", 0x1B, color + 30, 22);
	//	  sprintf(normal, "%c[%d;%d;%dm", 0x1B, RESET, WHITE+30, BLACK+40);
	sprintf(normal, "%c[0m", 0x1B);

	retval << command << i << normal;

	return retval.str();
#else
	return i;
#endif
};

};

