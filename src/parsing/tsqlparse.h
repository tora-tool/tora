#ifndef __TSQLPARSE__
#define __TSQLPARSE__

#include <QString>
#include <QList>
#include <QMap>
#include <QSet>
#include <QPointer>
#include <QVariant>
#include <QVector>
#include <ostream>
#include "Loki/Factory_alt.h"

#include <boost/iterator/iterator_facade.hpp>

#include <iostream>
#include "tsqlparser_export.h"

namespace SQLParser
{
	class TSQLPARSER_EXPORT Token;
	class TSQLPARSER_EXPORT Statement;
	class TSQLPARSER_EXPORT ParseException;
	
	class TSQLPARSER_EXPORT Position
	{
	public:
		Position(unsigned line, unsigned linePos): _mLine(line), _mLinePos(linePos) {};
		Position(Position const&other): _mLine(other._mLine), _mLinePos(other._mLinePos) {};
		
		inline unsigned getLine() const { return _mLine; };
		inline unsigned getLinePos() const { return _mLinePos; };
		inline QString toString() const { return QString("[%1,%2]").arg(_mLine).arg(_mLinePos); };
		inline bool operator== (const Position &other) const
		{
			return _mLine==other._mLine && _mLinePos==other._mLinePos;
		}

		inline bool operator!= (const Position &other) const
		{
			return !(*this == other);
		};
	
		inline bool operator< (const Position &other) const
		{
			return (_mLine < other._mLine) || (_mLine == other._mLine && _mLinePos < other._mLinePos);
		};

		inline bool operator> (const Position &other) const
		{
			return (_mLine > other._mLine) || (_mLine == other._mLine && _mLinePos > other._mLinePos);
		};
	private:		
		unsigned _mLine, _mLinePos;
	};
	  
	class TSQLPARSER_EXPORT Token : public QObject // inherit from QObject for QPointer
	{
	public:
		// TreeModel methods		
		inline int columnCount() const
		{
			return 1;
		};
		inline int childCount() const
		{
			return _mChildren.size();
		};
		inline QVariant data(int) const
		{
			return QVariant(toString());
		};
		inline Token* child(int row) const
		{
			return _mChildren.value(row);
		};
		inline int row() const
		{
			if (_mParent.isNull())
				return 0;			
			return _mParent->_mChildren.indexOf(const_cast<Token*>(this));			
		};
		inline Token* parent() const
		{
			return _mParent;
		};
		
		enum TokenType
		{
			// Special node types
			X_UNASSIGNED = 0,
			X_ROOT,
			X_FAILURE,
			X_COMMENT,
			// Leaf node
			L_RESERVED,
			L_TABLENAME,
			L_SCHEMANAME,
			L_FUNCTIONNAME,
			L_PROCEDURENAME,
			L_PACKAGENAME,
			L_BINDVARNAME,
			L_CURSORNAME,
			L_EXCEPTIONNAME,
			L_TABLEALIAS,
			L_IDENTIFIER,
			L_JOIN,
			L_ON,
			L_JOINING_CLAUSE,
			L_USING,
			// Leaf node (PLSQL only)
			L_VARIABLE,
			L_DATATYPE,
			L_PARAMETER, 
			// Structural node types			
			S_SUBQUERY_FACTORED,
			S_SUBQUERY_NESTED,
			L_SUBQUERY_ALIAS,
			S_UNION,
			S_WITH,
			S_COLUMN_LIST,
			S_FROM,
			S_WHERE,
			S_COND_AND,
			S_COND_OR,
			S_IDENTIFIER,
			S_TABLE_REF
		};

		const static char* TokenType2Text[];

		enum UsageType
		{
			Unknown,
			Declaration,
			Usage,
			UsageL // Used in LVALUE
		};
		
		Token(Token *parent, const Position &pos, const QString &str, const TokenType& tokentype = X_UNASSIGNED)
			: _mParent(parent)
			, _mPosition(pos)
			, _mStr(str)
			, _mTokenType(tokentype) // will be overwriten by descendent
			, _mUsageType(Unknown)
		{};

		const Position& getPosition() const { return _mPosition; };
		operator const Position&() const { return getPosition(); };

		const QString& toString() const
		{
			return _mStr;
		};

		operator const QString&() const { return toString(); };
		QString toStringFull() const
		{
			QString retval(toString());
			foreach(QPointer<Token> space, _mSpaces)
			{
				retval += space->toString();
			}
			return retval;
		};
		QString toStringRecursive() const
		{
			QString retval( isLeaf() ? toStringFull() : "");
			foreach(QPointer<Token> child, _mChildren)
			{
				retval += child->toStringRecursive();
			}
			return retval;
		};

		inline bool isLeaf() const { return _mChildren.isEmpty(); };
		
		inline const TokenType& getTokenType() const { return _mTokenType; };
		inline const char* getTokenTypeString() const { return TokenType2Text[_mTokenType]; };

		inline const QString& getTokenATypeName() const { return _mTokenATypeName; };
		inline void setTokenATypeName(QString const& name) { _mTokenATypeName = name; };

		inline const UsageType& getTokenUsageType() const { return _mUsageType; };

		inline void appendChild(QPointer<Token> child) { _mChildren.append(child); };
		inline void appendSpacer(QPointer<Token> space) { _mSpaces.append(space); };

		inline QList<QPointer<Token> > const& getChildren() const
		{
			return _mChildren;
		};

	protected:
		friend class Statement;
		
		QPointer<Token> _mParent;
		const Position _mPosition;
		const QString _mStr;
		const TokenType _mTokenType;
		const UsageType _mUsageType;
		QString _mTokenATypeName; //ANTLR token type - for debugging purposes only
		// TODO use only one of them
		QList<QPointer<Token> > _mChildren;
		QList<QPointer<Token> > _mSpaces;		
	};

	class TSQLPARSER_EXPORT ParseException: public ::std::exception
	{
	public:
		virtual const char* what() const throw() { return ""; };
	private:
		
	};

	class TSQLPARSER_EXPORT Identifier
	{
	public:
		QString toString(int fields = -1) const
		{
			QString retval;
			foreach(const Token* token, _mFields)
			{
				if(fields == 0)
					break;
				if(!retval.isEmpty())
					retval += '.';
				retval += token->toString();
				if(fields > 0)
					fields--;
			}
			return retval;
		}
		int length() const { return _mFields.size(); };
		Identifier(QVector<Token*> const &a) : _mFields(a)  {};
		Identifier() {};
		virtual ~Identifier() {};
		//private:
		QVector<Token*> _mFields;
	};
	
	class TSQLPARSER_EXPORT Statement //: public QObject
	{
		friend class ParseException;
	public:
		enum StatementType
		{
			S_SELECT,
			S_UPDATE,
			S_DELETE,
			S_INSERT,
			S_CREATE,
			S_DROP,
			S_ALTER,
			S_BEGIN,
			S_DECLARE,
			S_OTHER
		};

		enum ParserState
		{
			P_ERROR = -1,
			P_INIT = 0,
			P_LEXER, // lexed
			P_PARSER // parsed
		};
		
		Statement(const QString &statement, const QString &name)
			: _mStatement(statement)
			, _mname(name)
			, _mState(P_INIT)
			, _mAST(NULL)
			, _mEnd(NULL)
		{};

		~Statement() {};
		
		void dumpTree ();
		virtual void tree2Dot(std::ostream&) const = 0;
		
		// TreeModel methods
		inline const Token* root() const
		{
			return _mAST;
		};

		inline const QSet<QString>& tables() const
		{
			return _mTablesSet;
		};

		inline const QSet<QString>& aliases() const
		{
			return _mAliasesSet;
		};

		inline const QList<QString> allDeclarations() const
		{
			return _mDeclarations.keys();
		};

		inline const QList<const Token*> declarations(QString const& alias) const
		{
			return _mDeclarations.values(alias);
		};

		inline const QMap<QString, const Token*> declarations() const
		{
			return _mDeclarations;
		};
		
		class token_const_iterator : public boost::iterator_facade
		<
			token_const_iterator ,
			Token const,
			boost::forward_traversal_tag
		>
		{
		public:
			token_const_iterator() : m_token(0), m_stack()
			{};

			explicit token_const_iterator(Token* p) : m_token(p), m_stack()
			{
				m_stack.push_back(p);				
			};

			token_const_iterator(token_const_iterator const& other)
				: m_token(other.m_token)
				, m_stack(other.m_stack)
			{};
			
			~token_const_iterator() {};

			unsigned depth() const
			{
				return m_stack.size();
			};

		private:
			friend class boost::iterator_core_access;
			
			TSQLPARSER_EXPORT void increment();
			
			TSQLPARSER_EXPORT bool equal(token_const_iterator const& other) const;
			
			inline Token const& dereference() const { return *m_token; }
			
			Token const* m_token;
			QList<Token const*> m_stack;
		}; // class token_const_iterator
		
		token_const_iterator begin() const;
		token_const_iterator end() const;

		class token_const_iterator_to_root : public boost::iterator_facade
		<
			token_const_iterator_to_root ,
			Token const,
			boost::forward_traversal_tag
		>
		{
		public:
			token_const_iterator_to_root() : m_token(0) {};

			explicit token_const_iterator_to_root(Token const* p) : m_token(p) {};

			token_const_iterator_to_root(token_const_iterator_to_root const& other) : m_token(other.m_token) {};
			
			~token_const_iterator_to_root() {};

			inline operator Token const* () { return m_token; };
		private:
			friend class boost::iterator_core_access;
			
			TSQLPARSER_EXPORT void increment();
			
			TSQLPARSER_EXPORT bool equal(token_const_iterator_to_root const& other) const;
			
			inline Token const& dereference() const { return *m_token; }
			
			Token const* m_token;
		}; // class token_const_iterator_to_root

	protected:
		QString _mStatement, _mname;
		QMap<Position,Position*> _mPosition2pToken;
		StatementType _mStatementType;
		ParserState _mState;
		virtual void parse() = 0;
		QPointer<Token> _mAST;
		mutable Token *_mEnd;
		QSet<QString> _mTablesSet, _mAliasesSet;
		QVector<Token*> _mTablesList;
		QMap<QString, const Token*> _mDeclarations;
	public:
		QVector<Identifier> _mIdentifiers;
	};

}; // namespace SQLParser

typedef TSQLPARSER_EXPORT Util::GenericFactory<SQLParser::Statement, LOKI_TYPELIST_2(const QString &, const QString&)> StatementFactTwoParm;
class TSQLPARSER_EXPORT StatementFactTwoParmSing: public ::Loki::SingletonHolder<StatementFactTwoParm> {};

#endif
