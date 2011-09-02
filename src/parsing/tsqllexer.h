#ifndef __TSQLLEXER__
#define __TSQLLEXER__

#include <QString>

#include "Loki/Factory_alt.h"

#include <boost/iterator/iterator_facade.hpp>

namespace SQLParser
{
	class Lexer {
	public:
		Lexer(const QString &statement, const QString &name)
			: _mStatement(statement)
			, _mName(name)
			, _mEof(false)
		{};

		virtual QString firstWord() = 0;
		virtual QString currentWord(unsigned line, unsigned column) = 0;
	protected:
		QString _mStatement, _mName;
		bool _mEof;
	};

	
}; // namespace SQLParser

typedef Util::GenericFactory<SQLParser::Lexer, LOKI_TYPELIST_2(const QString &, const QString&)> LexerFactTwoParm;
class LexerFactTwoParmSing: public ::Loki::SingletonHolder<LexerFactTwoParm> {};


#endif
