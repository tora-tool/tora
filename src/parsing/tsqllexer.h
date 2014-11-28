#ifndef __TSQLLEXER__
#define __TSQLLEXER__

#include "core/tora_export.h"
#include "loki/Factory_alt.h"

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QVariant>
#include <boost/iterator/iterator_facade.hpp>

#include "parsing/TokenAttr.hpp"

using namespace SQLLexer::BlkCtx;

namespace SQLLexer
{

	class Lexer;

	class TORA_EXPORT Position
	{
	public:
		inline Position(unsigned line, unsigned linePos);
		inline Position(Position const&other);

		inline unsigned getLine() const;
		inline unsigned getLinePos() const;
		inline QString toString() const;

		inline Position& operator=(const Position& other);
		inline bool operator== (const Position &other) const;
		inline bool operator!= (const Position &other) const;
		inline bool operator< (const Position &other) const;
		inline bool operator> (const Position &other) const;

		inline bool operator<= (const Position &other) const;

	private:
		unsigned _mLine, _mLinePos;
	};

	/*
	 * Token - an element in Lexer stream
	 */
	class TORA_EXPORT Token : public QObject // inherit from QObject for QPointer
	{
		Q_OBJECT;
		Q_ENUMS(TokenType);
	public:

		enum TokenType
		{
			// Special node types
			X_UNASSIGNED = 0
			, X_EOF
			, X_EOL
			, X_FAILURE                // invalid character not understood by lexer
			, X_ONE_LINE               // SQLPlus command
			// White space "hidden" channel
			, X_WHITE                  // "white" char space, tab, newline
			, X_COMMENT                // single line comment, preferably without newline part
			, X_COMMENT_ML             // Multi-line comment (the correct one)
			, X_COMMENT_ML_END         // Partially edited multi-line comment, usually ends with EOF
			// Leaf node
			, L_RESERVED               // Reserved word
			, L_IDENTIFIER
			, L_NUMBER
			, L_BIND_VAR
			, L_BIND_VAR_WITH_PARAMS   // Tora(otl) specific bind variable :n<int>
			, L_BUILDIN                // Buildin function name
			, L_STRING                 // single/double quoted string
			, L_LPAREN                 // '('
			, L_RPAREN                 // ')'
			, L_SELECT_INTRODUCER      // SELECT, WITH
			, L_DML_INTRODUCER         // INSERT, UPDATE
			, L_DDL_INTRODUCER         // CREATE, ALTER
			, L_PL_INTRODUCER          // DECLARE, BEGIN, CREATE, PACKAGE, PROCEDURE
			, L_OTHER_INTRODUCER       // ALTER, EXPLAIN, ...
		};

		inline Token();
		inline Token(const Position &pos, unsigned len, unsigned origType, const TokenType& tokentype);
		inline Token(const Token& other);

		virtual ~Token() {};

		inline const Position& getPosition() const;
		inline unsigned getLength() const;
		inline const TokenType& getTokenType() const;
		inline unsigned getOrigTokenType() const;

		inline BlockContextEnum getBlockContext() const;
		inline void setBlockContext(BlockContextEnum);
		
		inline const QString& getText() const;
		inline void setText(const QString&);

		inline Token& operator=(const Token& other);
		inline operator const Position&() const;

	protected:
		Position _mPosition;
		unsigned _mLength, _mOrigType;
		BlockContextEnum _mBlockContext;
		QString  _mText;
		TokenType _mTokenType;
	};

	class Lexer
	{
	public:
		class token_const_iterator;

		inline Lexer(const QString &statement, const QString &name);
		virtual ~Lexer() {};

		/* To be overridden by subclasses */
		virtual QString firstWord() = 0;
		virtual QString wordAt(Position const &) = 0;
		virtual token_const_iterator findStartToken(token_const_iterator const &) = 0;		
		virtual token_const_iterator findEndToken(token_const_iterator const &) = 0;
		virtual void setStatement(const char *s, unsigned len = -1) = 0;
		virtual void setStatement(const QString &s) = 0;

		inline bool operator== (const Lexer& other) const;

		class Exception: public ::std::exception
		{
		public:
		    inline virtual const char* what() const throw();
		};

		class token_const_iterator : public boost::iterator_facade
		<
			token_const_iterator
			, Token const
			, boost::bidirectional_traversal_tag
		>
		{
		public:

			token_const_iterator(token_const_iterator const& other);
			explicit token_const_iterator(Lexer const& l, int i);
			~token_const_iterator();

			inline token_const_iterator consumeUntil(Token::TokenType);
			inline token_const_iterator consumeUntil(QSet<Token::TokenType>);

			inline token_const_iterator consumeUntil(unsigned);
			inline token_const_iterator consumeUntil(QSet<unsigned>);
			inline token_const_iterator consumeWS();

			token_const_iterator& operator=(const token_const_iterator& other);

		private:
			friend class boost::iterator_core_access;
			inline void increment();
			inline void decrement();
			inline bool equal(token_const_iterator const& other) const;
			inline Token const& dereference() const;

			mutable Token _mToken;
			int _mLastIndex, _mCurrentIndex;
			Lexer const& _mLexer;
		}; // nested class token_const_iterator

		inline token_const_iterator begin() const;
		inline token_const_iterator end() const;

		protected:
		/** To be overridden by subclasses */
		virtual int size() const = 0;
		virtual const Token& LA(int pos) const = 0;

		QString _mStatement, _mName;
		bool _mEof;
	};

}; // namespace SQLParser

#include "tsqllexer.inl"

typedef TORA_EXPORT Util::GenericFactory<SQLLexer::Lexer, LOKI_TYPELIST_2(const QString &, const QString&)> LexerFactTwoParm;
class TORA_EXPORT LexerFactTwoParmSing: public ::Loki::SingletonHolder<LexerFactTwoParm> {};

#endif
