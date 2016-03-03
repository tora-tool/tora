/** \file
 * This is the include file for the ANTLR3 sample project "dynamic-scope"
 * which shows how to use a C grammar and call it from C code.
 */
#ifndef	_T_MINI_TRAITS_H
#define	_T_MINI_TRAITS_H

// First thing we always do is include the ANTLR3 generated files, which
// will automatically include the antlr3 runtime header files.
// The compiler must use -I (or set the project settings in VS2005)
// to locate the antlr3 runtime files and -I. to find this file
//
#include <antlr3.hpp>

namespace Antlr3Mini {
	class SQLMiniLexer;
	class SQLMiniParser;

	class S1Lexer;
	class S1Parser;

	class S2Lexer;
	class S2Parser;

	class S3Lexer;
	class S3Parser;
};

namespace Antlr3Mini {
//code for overriding
template<class ImplTraits>
class UserTraits : public antlr3::CustomTraitsBase<ImplTraits>
{
public:
};

typedef antlr3::Traits<SQLMiniLexer, SQLMiniParser, UserTraits> SQLMiniLexerTraits;
typedef SQLMiniLexerTraits SQLMiniParserTraits;

typedef antlr3::Traits<S1Lexer, S1Parser, UserTraits>               S1LexerTraits;
typedef antlr3::Traits<S1Lexer, S1Parser, UserTraits>               S1ParserTraits;

typedef antlr3::Traits<S2Lexer, S2Parser, UserTraits>               S2LexerTraits;
typedef antlr3::Traits<S2Lexer, S2Parser, UserTraits>               S2ParserTraits;

typedef antlr3::Traits<S3Lexer, S3Parser, UserTraits>               S3LexerTraits;
typedef antlr3::Traits<S3Lexer, S3Parser, UserTraits>               S3ParserTraits;

};

#endif
