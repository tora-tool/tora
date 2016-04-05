#include "ts_log/ts_log_utils.h"

#include <QtCore/QObject>
#include <QtCore/QSet>

namespace SQLLexer
{
	inline SQLLexer::Position::Position(unsigned line, unsigned linePos)
		: _mLine(line)
		, _mLinePos(linePos)
	{};

	inline SQLLexer::Position::Position(Position const&other)
		: _mLine(other._mLine)
		, _mLinePos(other._mLinePos)
	{};

	inline unsigned SQLLexer::Position::getLine() const
	{
		return _mLine;
	};
	inline unsigned SQLLexer::Position::getLinePos() const
	{
		return _mLinePos;
	};
	inline QString SQLLexer::Position::toString() const
	{
		return QString("[%1,%2]").arg(_mLine).arg(_mLinePos);
	};

	inline SQLLexer::Position& SQLLexer::Position::operator=(const Position& other)
	{
		_mLine = other._mLine;
		_mLinePos = other._mLinePos;
		return *this;
	}
	inline bool SQLLexer::Position::operator== (const Position &other) const
	{
		return _mLine == other._mLine && _mLinePos == other._mLinePos;
	}

	inline bool SQLLexer::Position::operator!= (const Position &other) const
	{
		return !(*this == other);
	};

	inline bool SQLLexer::Position::operator< (const Position &other) const
	{
		return (_mLine < other._mLine) || (_mLine == other._mLine && _mLinePos < other._mLinePos);
	};

	inline bool SQLLexer::Position::operator> (const Position &other) const
	{
		return (_mLine > other._mLine) || (_mLine == other._mLine && _mLinePos > other._mLinePos);
	};

	inline bool SQLLexer::Position::operator<= (const Position &other) const
	{
		return operator==(other) || operator<(other);
	}
	
	inline Token::Token()
		: _mPosition(SQLLexer::Position(0,0))
		, _mLength(0)
		, _mOrigType(0) // TOKEN_INVALID
		, _mBlockContext(BlkCtx::NONE)
		, _mTokenType(X_UNASSIGNED)
	{};

	inline Token::Token(const SQLLexer::Position &pos, unsigned len, unsigned origType, const TokenType& tokentype)
		: _mPosition(pos)
		, _mLength(len)
		, _mOrigType(origType)
		, _mBlockContext(BlkCtx::NONE) // NONE
		, _mTokenType(tokentype)
	{};

	inline Token::Token(const Token& other)
		: _mPosition(other._mPosition)
		, _mLength(other._mLength)
		, _mOrigType(other._mOrigType)
		, _mBlockContext(other._mBlockContext)
		, _mTokenType(other._mTokenType)
	{
	};

	inline const SQLLexer::Position& Token::getPosition() const
	{
		return _mPosition;
	};

	inline unsigned Token::getLength() const
	{
		return _mLength;
	}

	inline const Token::TokenType& Token::getTokenType() const
	{
		return _mTokenType;
	};

	inline unsigned Token::getOrigTokenType() const
	{
		return _mOrigType;
	};

  	inline BlockContextEnum Token::getBlockContext() const
	{
		return _mBlockContext;
	};

	inline void Token::setBlockContext(BlockContextEnum b)
	{
		_mBlockContext = b;
	}
	
	inline const QString& Token::getText() const
	{
		return _mText;
	};

	inline void Token::setText(const QString &t)
	{
		_mText = t;
	};

	inline Token& Token::operator=(const Token& other)
	{
		_mPosition = other._mPosition;
		_mLength = other._mLength;
		_mTokenType = other._mTokenType;
		_mOrigType = other._mOrigType;
		_mBlockContext = other._mBlockContext;
		_mText = other._mText;
		return *this;
	};

	inline Token::operator const SQLLexer::Position&() const
	{
		return getPosition();
	};

	inline Lexer::Lexer(const QString &statement, const QString &name)
        : _mStatement(statement)
        , _mName(name)
        , _mEof(false)
	{};

	inline bool Lexer::operator== (const Lexer& other) const
	{
		return this == &other;
	}

	inline Lexer::token_const_iterator Lexer::begin() const
	{
		return token_const_iterator(*this, 1);
	}

	inline Lexer::token_const_iterator Lexer::end() const
	{
		return token_const_iterator(*this, size());
	}

	const char* Lexer::Exception::what() const throw()
	{
		return "";
	};

	inline Lexer::token_const_iterator::token_const_iterator(token_const_iterator const& other)
    	: _mToken(other._mToken)
    	, _mLastIndex(other._mLastIndex)
    	, _mCurrentIndex(other._mCurrentIndex)
    	, _mLexer(other._mLexer)
	{};

	inline Lexer::token_const_iterator::token_const_iterator(Lexer const& l, int i)
    	: _mLastIndex(l.size()) // add +1 for EOF token (_LT(i) is slow/ get(i) does not return EOF)
    	, _mCurrentIndex(i)
    	, _mLexer(l)
	{};

	inline Lexer::token_const_iterator::~token_const_iterator()
	{};

	inline void Lexer::token_const_iterator::increment()
	{
		_mCurrentIndex = (std::min)(_mLastIndex, _mCurrentIndex + 1);
	}

	inline void Lexer::token_const_iterator::decrement()
	{
		_mCurrentIndex = (std::max)(1, _mCurrentIndex - 1);
	}

	inline bool Lexer::token_const_iterator::equal(token_const_iterator const& other) const
	{
		return _mCurrentIndex == other._mCurrentIndex;
	}

	inline Token const& Lexer::token_const_iterator::dereference() const
	{
		_mToken = _mLexer.LA(_mCurrentIndex);
		return _mToken;
	}

	inline Lexer::token_const_iterator Lexer::token_const_iterator::consumeUntil(Token::TokenType type)
	{
		Lexer::token_const_iterator retval(*this);
		while( retval->getTokenType() != Token::X_EOF)
		{
			if( std::next(retval)->getTokenType() == type)
				break;
			retval++;
		}
		return retval;
	}

	inline Lexer::token_const_iterator Lexer::token_const_iterator::consumeUntil(QSet<Token::TokenType> s)
	{
		Lexer::token_const_iterator retval(*this);
		while( retval->getTokenType() != Token::X_EOF)
		{
			if( s.contains(std::next(retval)->getTokenType()))
				break;
			retval++;
		}
		return retval;
	}

	inline Lexer::token_const_iterator Lexer::token_const_iterator::consumeUntil(unsigned type)
	{
		Lexer::token_const_iterator retval(*this);
		while( retval->getTokenType() != Token::X_EOF)
		{
			if( std::next(retval)->getOrigTokenType() == type)
				break;
			retval++;
		}
		return retval;
	}

	inline Lexer::token_const_iterator Lexer::token_const_iterator::consumeUntil(QSet<unsigned> s)
	{
		Lexer::token_const_iterator retval(*this);
		while( retval->getTokenType() != Token::X_EOF)
		{
			if( s.contains(std::next(retval)->getOrigTokenType()))
				break;
			retval++;
		}
		return retval;
	}

	inline Lexer::token_const_iterator Lexer::token_const_iterator::consumeWS()
	{
		Lexer::token_const_iterator retval(*this);
		bool spaceMatched = false; // if pointing onto non-WS consume until next non-WS
		while(true)
		{
			switch (retval->getTokenType())
			{
			case Token::X_EOL:
			case Token::X_WHITE:
			case Token::X_COMMENT:
			case Token::X_COMMENT_ML:
			case Token::X_COMMENT_ML_END:
				retval++;
				continue;
			default:
				if (spaceMatched)
					return retval;
				else
					retval++;
			}
			spaceMatched = true;
		}
		return retval; // never reached
	}

	inline Lexer::token_const_iterator& Lexer::token_const_iterator::operator=(const token_const_iterator& other)
	{
		_mToken = other._mToken;
		_mLastIndex = other._mLastIndex;
		_mCurrentIndex = other._mCurrentIndex;
		Q_ASSERT_X(_mLexer == other._mLexer, qPrintable(__QHERE__), "Invalid iterator assignment");
		return *this;
	}

};
