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

#include "trotl_extended_parser.h"

namespace trotl {

  using namespace boost::spirit;
	
  bool ExtendedPlsqlParser::parse (const tstring &statement)
  {
	  //static rule<phrase_scanner_t> C_WHITE   = *(blank_p | eol_p);
	  static rule<phrase_scanner_t> M_WHITE   = +(blank_p | eol_p)
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "space", _1, _2)];
    
	  static rule<phrase_scanner_t> K_SELECT  = as_lower_d["select"]
		  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "keyword", _1, _2)];
	  static rule<phrase_scanner_t> K_FROM    = as_lower_d["from"];
	  static rule<phrase_scanner_t> K_WHERE   = as_lower_d["where"];
	  static rule<phrase_scanner_t> K_INSERT  = as_lower_d["insert"];
	  static rule<phrase_scanner_t> K_INTO    = as_lower_d["into"];
	  static rule<phrase_scanner_t> K_VALUES  = as_lower_d["values"];
	  static rule<phrase_scanner_t> K_MERGE   = as_lower_d["merge"];
	  static rule<phrase_scanner_t> K_DELETE  = as_lower_d["delete"];
	  static rule<phrase_scanner_t> K_DECLARE = as_lower_d["declare"];
	  static rule<phrase_scanner_t> K_BEGIN   = as_lower_d["begin"];
	  static rule<phrase_scanner_t> K_END     = as_lower_d["end"];
	  static rule<phrase_scanner_t> K_CASE    = as_lower_d["case"];
	  static rule<phrase_scanner_t> K_WHEN    = as_lower_d["when"];
	  static rule<phrase_scanner_t> K_THEN    = as_lower_d["then"];
	  static rule<phrase_scanner_t> K_ELSE    = as_lower_d["else"];
	  static rule<phrase_scanner_t> K_ROWID   = as_lower_d["rowid"];
	  static rule<phrase_scanner_t> K_ORDER_BY= as_lower_d["order"] >> as_lower_d["by"];

	  static rule<phrase_scanner_t> K_DISTINCT= as_lower_d["distinct"];
	  static rule<phrase_scanner_t> K_UNIQUE  = as_lower_d["unique"]
		  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "keyword", _1, _2)];
	  static rule<phrase_scanner_t> K_ALL     = as_lower_d["all"]
		  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "keyword", _1, _2)];
	  static rule<phrase_scanner_t> K_AS      = as_lower_d["as"]
		  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "keyword", _1, _2)];
	  static rule<phrase_scanner_t> K_BULK_COLLECT = as_lower_d["bulk"] >> as_lower_d["collect"]
		  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "keyword", _1, _2)];
	  
	  static rule<phrase_scanner_t> keyword_tm =
		  K_SELECT | K_FROM | K_WHERE |
		  K_INSERT | K_INTO | K_VALUES |
		  K_MERGE  | K_DELETE | K_DECLARE |
		  K_BEGIN  | K_END |
		  K_CASE   | K_WHEN | K_THEN | K_ELSE |
		  K_ROWID  |
		  K_ORDER_BY;

	  
	  
	  static rule<phrase_scanner_t> word_tm      = lexeme_d[(alpha_p >> *(alnum_p | ch_p('_') | ch_p('#') | ch_p('$') ))];
	  static rule<phrase_scanner_t> dqstring_tm  = lexeme_d[ch_p('"') >> (alnum_p|ch_p(' ')) >> *(~ch_p('"')) >> ch_p('"')];
	  static rule<phrase_scanner_t> bind_tm      = lexeme_d[(alpha_p >> *(alnum_p | ch_p('_') | ch_p('.')))];
	  static rule<phrase_scanner_t> unumber_tm   = lexeme_d[(+digit_p)];
	  static rule<phrase_scanner_t> number_tm    = (!ch_p('-')) >> unumber_tm;
	  static rule<phrase_scanner_t> float_tm     = lexeme_d[(number_tm >> !(ch_p('.') >> unumber_tm ))];

	  static rule<phrase_scanner_t> operator_tm = ( str_p("**") |
							as_lower_d["not"] | as_lower_d["in"] | as_lower_d["and"] | as_lower_d["or"] |
							( as_lower_d["is"] >> as_lower_d["null"] ) |
							( as_lower_d["is"] >> as_lower_d["not"] >> as_lower_d["null"] ) | 
							ch_p('+')   | ch_p('-') |
							ch_p('*')   | ch_p('/') |
							str_p("||") |
							ch_p('=')   | ch_p('<') |
							ch_p('>')   | str_p("<=") |
							str_p(">=") | str_p("<>") |
							str_p("!=") | str_p("~=") |
							str_p("^=") | str_p(":=")
		  );
	  
 	  static rule<phrase_scanner_t> delim_tm = ( str_p("..") | ch_p(';') /*| ','*/ | '(' | ')' /*| '.' | blank_p | eol_p*/ );
	  
	  static rule<phrase_scanner_t> appos_tm = (ch_p('\'') >> ch_p('\''));
	  static rule<phrase_scanner_t> qstring_tm = ch_p('\'') >> *(~ch_p('\'')|appos_tm) >> ch_p('\'');
	  static rule<phrase_scanner_t> inout_tm = str_p("inout") | str_p("out") | str_p("in");
	  static rule<phrase_scanner_t> label_tm = str_p("<<") >> word_tm >> str_p(">>");

//	  static rule<phrase_scanner_t> end_tm =
		  
	  rule<phrase_scanner_t> keyword_cl = keyword_tm
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "keyword", _1, _2)];
	  
	  rule<phrase_scanner_t> operator_cl = operator_tm
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)];
	  
	  rule<phrase_scanner_t> delim_cl= delim_tm
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)];

	  rule<phrase_scanner_t> number_cl = (float_tm|unumber_tm|number_tm)
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "number", _1, _2)];

	  rule<phrase_scanner_t> word_cl = ((label_tm | ((word_tm|dqstring_tm) >>
					 !( ch_p('.') >> (word_tm|dqstring_tm)) >>
					 !( ch_p('.') >> (word_tm|dqstring_tm)) >>
					 !( ch_p('%') >> (word_tm|dqstring_tm))
				     )) - keyword_tm - operator_tm)
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "word", _1, _2)];

	  rule<phrase_scanner_t> dqstring_cl = dqstring_tm
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "qword", _1, _2)];

	  rule<phrase_scanner_t> qstring_cl = qstring_tm
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "qstring", _1, _2)];

	  rule<phrase_scanner_t> bracket_cl = ( ch_p('[') >> unumber_tm[boost::bind(&BindVarDecl::setname, boost::ref(_bindvar), "bracket", _1, _2)] >> ch_p(']') );

	  rule<phrase_scanner_t> bindvar = (ch_p(':') >>
			  (+alnum_p)[boost::bind(&BindVarDecl::setname, boost::ref(_bindvar), "bindname", _1, _2)] >>
			  ch_p('<') >>
			  (bind_tm)[boost::bind(&BindVarDecl::setname, boost::ref(_bindvar), "bindtype", _1, _2)] >>
			  (!bracket_cl) >>			    
			    !(ch_p(',') >>
			     inout_tm[boost::bind(&BindVarDecl::setname, boost::ref(_bindvar), "inout", _1, _2)] >>
			     (!bracket_cl)
				    )>>		 
			  ch_p('>'))
			  [boost::bind(&ExtendedPlsqlParser::bindvar_callback, boost::ref(*this), "bindvar", _1, _2)];

	  rule<phrase_scanner_t> comment_cl = (confix_p("/*", *(anychar_p | ch_p(':') ) , "*/") | confix_p("--", *anychar_p, eol_p) | confix_p("PROMPT", *anychar_p, eol_p))
	  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "comment", _1, _2)];

	  static rule<phrase_scanner_t> coma = str_p(",")
		  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)];
	  static rule<phrase_scanner_t> dot = str_p(".")
		  [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)];
	  static rule<phrase_scanner_t> asterisk = ch_p('*');

	  static rule<phrase_scanner_t> function;
	  static rule<phrase_scanner_t> expression;
	  
	  static rule<phrase_scanner_t> identifier = ( dqstring_cl | word_cl );
	  static rule<phrase_scanner_t> sql_expression;
	  static rule<phrase_scanner_t> sql_identifier = ( K_ROWID | identifier );

	  expression = (sql_expression | function) >> *(operator_cl >> ( sql_expression | function));
	  function = word_cl >>
		  str_p("(")[boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)] >>
		  expression >> str_p(")") [boost::bind(&ExtendedPlsqlParser::token_callback, boost::ref(*this), "character", _1, _2)];
	  
	  static rule<phrase_scanner_t> column_spec = ( (sql_identifier | number_cl | qstring_cl | function) >> !( dot >> asterisk | dot >> sql_identifier ));
	  static rule<phrase_scanner_t> alias = (!K_AS) >> (!sql_identifier); 
	  static rule<phrase_scanner_t> displayed_column = ( column_spec /* | sql_expression */ ) >> !alias ;
	  static rule<phrase_scanner_t> select_list = asterisk | displayed_column >> *(coma >> displayed_column);
	  	  
	  static rule<phrase_scanner_t> select_expression = K_SELECT >> !( K_DISTINCT | K_UNIQUE | K_ALL ) >> select_list >> !( K_BULK_COLLECT );

	  rule<> plsql = *(keyword_cl | comment_cl | word_cl | dqstring_cl |
			   bindvar | operator_cl | qstring_cl| 
			   number_cl | delim_cl );

	  parse_info<> res = boost::spirit::parse(statement.c_str(),
						  // Begin grammar
						  /*as_lower_d[(*/ select_expression>> end_p /*)]*/
						  //,
						  //  End grammar
						  //M_WHITE
						  //(space_p | blank_p | eol_p)
		  );
	  bool p = res.full;

//	  BOOST_SPIRIT_DEBUG_RULE(K_SELECT);
// 	  BOOST_SPIRIT_DEBUG_RULE(M_WHITE);
// 	  BOOST_SPIRIT_DEBUG_RULE(keyword_tm);
// 	  BOOST_SPIRIT_DEBUG_RULE(comment_cl);
 	  BOOST_SPIRIT_DEBUG_RULE(word_cl);
 	  BOOST_SPIRIT_DEBUG_RULE(expression);
 	  BOOST_SPIRIT_DEBUG_RULE(column_spec);
 	  BOOST_SPIRIT_DEBUG_RULE(displayed_column);
 	  BOOST_SPIRIT_DEBUG_RULE(function);
	  
	  for(unsigned itr2=0; itr2 < _bindvars.size(); ++itr2)
		  _bindvars.at(itr2).print();
//	  std::cout << "Bindvar count: " << _bindvars.size() << std::endl;

	  return p;
  };

  tstring ExtendedPlsqlParser::colorword(const tstring &i, int color)
  {
	  char command[256], normal[256];
	  int attr = BRIGHT;
	  int bg = BLACK;
	  tostream retval;
	  // see: http://www.vias.org/linux-knowhow/lnag_05_05_04.html
	  //	  sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, color + 30, bg + 40);
	  if( color == ExtendedPlsqlParser::RED)
	    sprintf(command, "%c[%d;%dm", 0x1B, color + 30, 1);
	  else
	    sprintf(command, "%c[%d;%dm", 0x1B, color + 30, 22);
	  //	  sprintf(normal, "%c[%d;%d;%dm", 0x1B, RESET, WHITE+30, BLACK+40);
	  sprintf(normal, "%c[0m", 0x1B);
	  
	  retval << command << i << normal;

	  return retval.str();
  };

};

