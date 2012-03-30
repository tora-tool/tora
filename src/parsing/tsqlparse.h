#ifndef __TSQLPARSE__
#define __TSQLPARSE__

#include <assert.h>
#include <QString>
#include <QList>
#include <QMap>
#include <QSet>
#include <QPointer>
#include <QVariant>
#include <QVector>
#include <ostream>
#include "loki/Factory_alt.h"

#include <boost/iterator/iterator_facade.hpp>

#include <iostream>
#include "tsqlparser_export.h"

namespace SQLParser
{
	class TSQLPARSER_EXPORT Token;
	class TSQLPARSER_EXPORT Statement;
	class TSQLPARSER_EXPORT ParseException;

	/* each instance of T_SELECT can holds transtation map TABLE_ALIAS -> TABLE_REF */
	class TSQLPARSER_EXPORT Translation : public QMap<QString, Token*>
	{
	public:
		virtual ~Translation() {};
	};

	class TSQLPARSER_EXPORT ObjectCache
	{
	public:
		class TableInfo {
		public:
			TableInfo(QString const& schema, QString const& table, QList<QString> columns)
				: _mSchema(schema)
				, _mTable(table)
				, _mColumns(columns)
			{};
			TableInfo(TableInfo const& other)
				: _mSchema(other._mSchema)
				, _mTable(other._mTable)
				, _mColumns(other._mColumns)
			{};

			inline QString const& schemaName() const {  return _mSchema; }
			inline QString const& tableName() const {  return _mTable; };
			inline QList<QString> const& columns() const {  return _mColumns; };
		protected:
			QString _mSchema;
			QString _mTable;
			QList<QString> _mColumns;
		};

		inline bool tableExists(QString const& currentSchema, QString const& schema, QString const& table) const 
		{ 
			return getTable(currentSchema, schema, table) != _mTableMap.end();
		};

		inline bool columnExists(QString const& schema, QString const& table, QString const& column) const 
		{ 
			QMap<QPair<QString, QString>, TableInfo>::const_iterator t;
			if( (t = getTable(schema, schema, table)) != _mTableMap.end())
			{
				return t->columns().contains(column);
			} else {
				return false;
			}
		};

		inline void addTable(TableInfo const& other)
		{
			_mTableMap.insert(QPair<QString,QString>(other.schemaName(), other.tableName()), other);
		}
	protected:
		QMap<QPair<QString, QString>, TableInfo>::const_iterator getTable(QString const& currentSchema, QString const& schema, QString const& table) const;

		QMap<QPair<QString, QString>, TableInfo> _mTableMap;
		QMap<QPair<QString, QString>, QPair<QString,QString> > _mSynonymMap;
	};
	
	/*
	 * Token's position in the source
	 */
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
	  
    /*
     * Token - an element in AST tree hierarchy
     */
	class TSQLPARSER_EXPORT Token : public QObject // inherit from QObject for QPointer
	{
	public:
		// TreeModel methods		
		inline int columnCount()     const { return 1; };
		inline int childCount()      const { return _mChildren.size(); };
		inline QVariant data(int)    const { return QVariant(toString()); };
		inline Token* child(int row) const { return _mChildren.value(row); };
		inline Token* parent()       const { return _mParent; };
		inline int row() const
		{
			if (_mParent == NULL)
				return 0;			
			return _mParent->_mChildren.indexOf(const_cast<Token*>(this));			
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
			S_TABLE_REF,
			S_OPERATOR_BINARY
		};
		
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
			//, _mAlias(NULL)
		{};

		Token(const Token& other)
			: _mParent(other._mParent)
			, _mPosition(other._mPosition)
			, _mStr(other._mStr)
			, _mTokenType(other._mTokenType) // will be overwriten by descendent
			, _mUsageType(other._mUsageType)
			, _mTokenATypeName(other._mTokenATypeName)
			, _mChildren(other._mChildren)
			, _mSpaces(other._mSpaces)
		{
			size_t me = this->size();
			size_t oth = other.size();
			assert( this->size() >= other.size());
		};

		virtual ~Token()
		{};
		virtual size_t size() const { return sizeof(*this); };

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
		QString toStringRecursive(bool spaces = true) const
		{
			QString retval( isLeaf() ?
					(spaces ? toStringFull() : toString()) :
					"");
			foreach(QPointer<Token> child, _mChildren)
			{
				retval += child->toStringRecursive(spaces);
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
		inline void replaceChild(int index, Token* newOne)
		{
			_mChildren.replace(index, newOne);			
			foreach(QPointer<Token> child, newOne->_mChildren)
			{
				child->_mParent = newOne;
			}
		};

		inline QList<QPointer<Token> > const& getChildren() const
		{
			return _mChildren;
		};

		//inline Translation& aliasTranslation() { return _mAliasTranslation; };
		//inline Translation const& aliasTranslation() const { return _mAliasTranslation; };

		//inline Token const* nodeAlias() const { return _mAlias; };
		//inline void setNodeAlias(Token const *t) { _mAlias = t; };

		//inline QSet<Token*>& nodeTables() { return _mTables; };
		//inline const QSet<Token*>& nodeTables() const { return _mTables; };
	protected:
		friend class Statement;
		
		const static char* TokenType2Text[];
		Token* _mParent;
		const Position _mPosition;
		const QString _mStr;
		const TokenType _mTokenType;
		const UsageType _mUsageType;
		QString _mTokenATypeName; //ANTLR token type - for debugging purposes only
		// TODO use only one of them
		QList<QPointer<Token> > _mChildren;
		QList<QPointer<Token> > _mSpaces;
		
	};

	class TSQLPARSER_EXPORT TokenIdentifier: public Token
	{
	public:
		TokenIdentifier(const Token& other)  
		: Token(other)
		, _mResolved(false)
		{
			size_t me = this->size();
			size_t oth = other.size();
			assert( this->size() >= other.size());
		};

		virtual ~TokenIdentifier()
		{};

		virtual size_t size() const { return sizeof(*this); };
		
		QString schemaName () const;
		QString tableName () const;
		QString columnName () const;

		void resolve() const;
		bool resolved() const { return _mResolved; };
		
	    
	protected:
		bool _mResolved;
	};

	class TSQLPARSER_EXPORT TokenTable: public Token
	{
	public:
		TokenTable(const Token& other)  
		: Token(other)
		, _mAlias(NULL)
		{
			size_t me = this->size();
			size_t oth = other.size();
			assert( this->size() >= other.size()); 
		};

		virtual ~TokenTable()
		{};
		
		virtual size_t size() const { return sizeof(*this); };
		
		QString tableName () const;
		QString columnName () const;
	    
		inline Token const* nodeAlias() const { return _mAlias; };
		inline void setNodeAlias(Token const *t) { _mAlias = t; };

		inline QString nodeID() const { return _mNodeID; };
		inline void setNodeID(QString const &id) { _mNodeID = id; };
		
	protected:
		/* Translate table ref SCHEMA.TABLE_A -> table alias "A"
		   this pointer is present only if token is of type SUBQUERY, ROOT, TABLE_REF(schema.tablename)
		*/
		Token const* _mAlias;
		QString _mNodeID;
	};

	class TSQLPARSER_EXPORT TokenSubquery: public Token
	{
	public:
		TokenSubquery(Token *parent, const Position &pos, const QString &str, const TokenType& tokentype = X_UNASSIGNED)
			: Token(parent, pos, str, tokentype)
			, _mAlias(NULL)
		{};

		TokenSubquery(const Token& other) 
		: Token(other)
		, _mAlias(NULL)
		{
			size_t me = this->size();
			size_t oth = other.size();
			assert( this->size() >= other.size());
		};

		virtual ~TokenSubquery() {};
		virtual size_t size() const { return sizeof(*this); };
		
		inline Token const* nodeAlias() const { return _mAlias; };
		inline void setNodeAlias(Token const *t) { _mAlias = t; };

		inline QString nodeID() const { return _mNodeID; };
		inline void setNodeID(QString const &id) const { _mNodeID = id; };

		inline Translation& aliasTranslation() { return _mAliasTranslation; };
		inline Translation const& aliasTranslation() const { return _mAliasTranslation; };

		inline Translation& nodeTables() { return _mTables; };
		inline Translation const& nodeTables() const { return _mTables; };

	protected:
		/* Translate table ref SCHEMA.TABLE_A -> table alias "A"
		   this pointer is present only if token is of type SUBQUERY, ROOT, TABLE_REF(schema.tablename)
		*/
		Token const* _mAlias;

		/* translate table alias "A" -> table ref "SCHEMA.TABLE_A"
		   this map is present only if token is of type SUBQUERY or ROOT
		*/
		Translation _mAliasTranslation;	

		/* Set of tables used in a (sub)query. This list is present only if token is of type SUBQUERY or ROOT
		*/
		Translation _mTables;

		mutable QString _mNodeID;
	};

	class TSQLPARSER_EXPORT ParseException: public ::std::exception
	{
	public:
		virtual const char* what() const throw() { return ""; };
	private:
		
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

		
		/* Call this method after the instance is created.
			It will use ObjectCache to disambiguate column names.
		*/
		virtual void scanTree(ObjectCache *, QString const&) = 0;

		virtual ~Statement() {};		

		void dumpTree ();
		
		// TreeModel methods
		inline const Token* root() const
		{
			return _mAST;
		};

		//inline const QSet<QString>& tables() const
		//{
		//	return _mTablesSet;
		//};

		inline const QVector<Token const*>& tableTokens() const
		{
			return _mTablesList;
		};

		//inline const QSet<QString>& aliases() const
		//{
		//	return _mAliasesSet;
		//};

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
		
		Token const* translateAlias(QString const& alias, Token const *context);
		Token const* getTableRef(QString const& tableName, Token const *context);

		class token_const_iterator : public boost::iterator_facade
		<
			token_const_iterator ,
			Token const,
			boost::forward_traversal_tag
		>
		{
		public:
			token_const_iterator() : m_token(0), m_stack(), m_lastIndex(0)
			{};

			explicit token_const_iterator(Token* p) : m_token(p), m_stack(), m_lastIndex(0)
			{
				m_stack.push_back(p);				
			};

			token_const_iterator(token_const_iterator const& other)
				: m_token(other.m_token)
				, m_stack(other.m_stack)
				, m_lastIndex(other.m_lastIndex)
			{};
			
			~token_const_iterator() {};

			unsigned depth() const
			{
				return m_stack.size();
			};

		private:
			friend class boost::iterator_core_access;
			
			TSQLPARSER_EXPORT void increment();
			TSQLPARSER_EXPORT void decrement();
			
			TSQLPARSER_EXPORT bool equal(token_const_iterator const& other) const;
			
			inline Token const& dereference() const { return *m_token; }
			
			Token const* m_token;
			QList<Token const*> m_stack;
			int m_lastIndex;
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

		class token_iterator_to_root : public boost::iterator_facade
		<
			token_iterator_to_root ,
			Token,
			boost::forward_traversal_tag
		>
		{
		public:
			token_iterator_to_root() : m_token(0) {};

			explicit token_iterator_to_root(Token* p) : m_token(p) {};

			token_iterator_to_root(token_iterator_to_root const& other) : m_token(other.m_token) {};
			
			~token_iterator_to_root() {};

			inline operator Token* () { return m_token; };
		private:
			friend class boost::iterator_core_access;
			
			TSQLPARSER_EXPORT void increment();
			
			TSQLPARSER_EXPORT bool equal(token_iterator_to_root const& other) const;
			
			inline Token& dereference() const { return *m_token; }
			
			Token* m_token;
		}; // class token_const_iterator_to_root

	protected:
		QString _mStatement, _mname;
		QMap<Position,Position*> _mPosition2pToken;
		StatementType _mStatementType;
		ParserState _mState;
		virtual void parse() = 0;
		QPointer<Token> _mAST;
		mutable Token *_mEnd;
		//QSet<QString> _mTablesSet, _mAliasesSet;
		QVector<Token const*> _mTablesList;
		QMap<QString, const Token*> _mDeclarations;
	public:
	};

}; // namespace SQLParser

typedef TSQLPARSER_EXPORT Util::GenericFactory<SQLParser::Statement, LOKI_TYPELIST_2(const QString &, const QString&)> StatementFactTwoParm;
class TSQLPARSER_EXPORT StatementFactTwoParmSing: public ::Loki::SingletonHolder<StatementFactTwoParm> {};

#endif
