
// ANTLR runtime includes
#include "antlr3commontoken.h"
#include "antlr3string.h"
#include "antlr3input.h"
#include "antlr3lexer.h"

#include "OracleSQLLexer.h"
#include "OracleSQLParser.h"

#include "tsqlparse.h"

#include <QPair>

namespace SQLParser
{
	/* each instance of T_SELECT can holds transtation map TABLE_ALIAS -> TABLE_REF */
	class TSQLPARSER_EXPORT Translation : public QMap<QString, Token*>
	{
	public:
		virtual ~Translation() {};
	};

	/* Vertex represents "table". Table's scope is either: "global" Token* == ROOT
	   or "local" Token* points into some nested/factored subquery
	*/
	class TSQLPARSER_EXPORT Vertex : public QPair<Translation*, Token*>	
	{
	};

	/* Edge connects two Vertexes */
	class TSQLPARSER_EXPORT Edge : public QPair<Vertex*, Vertex*>
	{
	};
	
	class OracleSQLToken: public Token
	{
	public:
		OracleSQLToken (Token *parent, const Position &pos, const QString &str, unsigned tokentype, const char* tokentypestring, unsigned usagetype = T_UNKNOWN);
		Translation _mTranslation; // table alias "A" -> table ref "SCHEMA.TABLE_A"
		
	};

	OracleSQLToken::OracleSQLToken (Token *parent, const Position &pos, const QString &str, unsigned tokentype, const char* tokentypestring, unsigned usagetype)
		: Token(parent, pos, str)
	{
		_mTokenATypeName = tokentypestring;

		UsageType &_mUsageTypeRef = const_cast<Token::UsageType&>(_mUsageType);
		switch(usagetype)
		{
		case 0:
		case T_UNKNOWN:
			_mUsageTypeRef = Unknown;
			break;
		case T_USE:
			_mUsageTypeRef = Usage;
			break;
		case T_DECL:
			_mUsageTypeRef = Declaration;
			break;
		}

		TokenType &_mTokenTypeRef = const_cast<Token::TokenType&>(_mTokenType);
		switch(tokentype) {
		case T_UNKNOWN:
			_mTokenTypeRef = X_UNASSIGNED;
			break;
		case T_RESERVED:
			_mTokenTypeRef = L_RESERVED;
			break;
		case T_TABLE_NAME:
			_mTokenTypeRef = L_TABLENAME;
			break;
		case T_TABLE_ALIAS:
			// Resolve grammar ambiguity: SELECT * FROM A INNER JOIN B; (=> INNER is not a table alias)
			// The same for NATURAL JOIN, CROSS JOIN, LEFT/RIGHT OUTER JOIN
			if( usagetype == T_DECL &&  (!str.compare("INNER", Qt::CaseInsensitive) ||
						     !str.compare("CROSS", Qt::CaseInsensitive) ||							     
						     !str.compare("NATURAL", Qt::CaseInsensitive) ||
						     !str.compare("LEFT", Qt::CaseInsensitive) ||
						     !str.compare("RIGHT", Qt::CaseInsensitive)
				    )
				)
			{
				_mTokenTypeRef = L_RESERVED;
				_mUsageTypeRef = Unknown;
				_mTokenATypeName = "T_RESERVED";
				break;
			}
			switch(parent->getTokenType())
			{
			case S_WITH:
				_mTokenTypeRef = L_SUBQUERY_ALIAS;
				break;
			default:				
				_mTokenTypeRef = L_TABLEALIAS;
				break;
			}
			break;
		case T_SCHEMA_NAME:
			_mTokenTypeRef = L_SCHEMANAME;
			break;
		case T_TABLE_REF:
			_mTokenTypeRef = S_TABLE_REF;
			break;
		case T_FUNCTION_NAME:
			_mTokenTypeRef = L_FUNCTIONNAME;
			break;
		case T_PACKAGE_NAME:
			_mTokenTypeRef = L_PACKAGENAME;
			break;
		case T_BINDVAR_NAME:
			_mTokenTypeRef = L_BINDVARNAME; 
			break; 
		case T_COLUMN_ALIAS:
		case T_COLUMN_NAME:
			_mTokenTypeRef = L_IDENTIFIER;
			break;
		case T_WITH:
			_mTokenTypeRef = S_WITH;
			break;
		case T_UNION:
			_mTokenTypeRef = S_UNION;
			break;
		case T_SELECT:
			switch(parent->getTokenType())
			{
			case S_WITH:
				_mTokenTypeRef = S_SUBQUERY_FACTORED;
				break;
			case S_FROM:
			case S_COLUMN_LIST:
			case S_WHERE:
			case S_COND_AND:
			case S_COND_OR:
			case S_UNION:				
				_mTokenTypeRef = S_SUBQUERY_NESTED;
				break;
			case X_ROOT:
				_mTokenTypeRef = X_ROOT;
				break;
			default:
				_mTokenTypeRef = X_FAILURE;
			}
			break;
		case T_TABLE_CAST:
			_mTokenTypeRef = S_SUBQUERY_NESTED;
			break;
		case T_FROM:
			_mTokenTypeRef = S_FROM;
			break;
		case T_JOINING_CLAUSE:
			_mTokenTypeRef = L_JOINING_CLAUSE;
			break;
		case T_WHERE:
			_mTokenTypeRef = S_WHERE;
			break;
		case T_IDENTIFIER:
			_mTokenTypeRef = S_IDENTIFIER;
			break;
		} // switch(tokentype)

	};

	class OracleIdentifier: public Identifier
	{
	public:
		OracleIdentifier(QVector<Token*> const &a) : Identifier(a), _mTableRef(NULL) {};
		OracleIdentifier() : Identifier(), _mTableRef(0) {};

		QString toStringTranslated() const;
		
		virtual ~OracleIdentifier() {};		
	private:
		Token const* _mTableRef;
	protected:
	};
	
	class OracleSQLStatement: public Statement
	{
	public:
		OracleSQLStatement(const QString &statement, const QString &name);
		~OracleSQLStatement()
		{};
		virtual void tree2Dot(std::ostream &o) const;
		//template<class T> T& recursiveWalk(Token const* node, T &stream) const;
	private:
		void parse ();
		/* Recursive walk through ANTLR3_BASE_TREE and create AST tree*/
		void treeWalk(pOracleSQLParser psr, QPointer<Token> root,  ANTLR3_BASE_TREE *tree, ANTLR3_UINT32 &lastindex);
		/* Walk through Token tree and look for table names, table aliases, ... */
		void scanTree();

		void disambiguate();
		
		void addTranslation(QString const& alias, Token const *tableOrSubquery, Token const *context);
		Token const* translate(QString const& alias, Token const *context);
		
		pANTLR3_VECTOR lexerTokenVector;
	};

	OracleSQLStatement::OracleSQLStatement(const QString &statement, const QString &name) : Statement(statement, name)
	{
		_mStatementType = S_SELECT;
		parse();
		scanTree();
		disambiguate();
	};

	void OracleSQLStatement::parse ()
	{
		pANTLR3_INPUT_STREAM input;
		pOracleSQLLexer lxr;
		pANTLR3_COMMON_TOKEN_STREAM tstream;
		pOracleSQLParser psr;
		OracleSQLParser_start_rule_return langAST;
		//pANTLR3_COMMON_TREE_NODE_STREAM	nodes;

		QByteArray QBAinput(_mStatement.toUtf8());
		QByteArray QBAname(_mname.toUtf8());

		input = antlr3StringStreamNew( (pANTLR3_UINT8) QBAinput.data(), ANTLR3_ENC_8BIT, (ANTLR3_UINT64) QBAinput.size(), (pANTLR3_UINT8)QBAname.data());
		//input = antlr3NewAsciiStringInPlaceStream( (uint8_t*) QBAinput.data(), (ANTLR3_UINT64) QBAinput.size(), NULL);
			
		input->setUcaseLA(input, ANTLR3_TRUE); // ignore case

		if (input == NULL)
		{
			// TODO throw here
			throw ParseException();
			exit(ANTLR3_ERR_NOMEM);
		}

		// Our input stream is now open and all set to go, so we can create a new instance of our
		// lexer and set the lexer input to our input stream:
		//  (file | memory | ?) --> inputstream -> lexer --> tokenstream --> parser ( --> treeparser )?
		//
		lxr	    = OracleSQLLexerNew(input);	    // CLexerNew is generated by ANTLR
		lxr->pLexer->input->charPositionInLine = 0; // fix off-by-one error for getCharPositionInLine for the 1st row
			
		if ( lxr == NULL )
		{
			// TODO throw here
			throw ParseException();
			exit(ANTLR3_ERR_NOMEM);
		}

		tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
			
		if (tstream == NULL)
		{
			// TODO throw here
			_mState = P_ERROR;			
			throw ParseException();
			exit(ANTLR3_ERR_NOMEM);
		}
		this->lexerTokenVector = tstream->getTokens(tstream);
		_mState = P_LEXER;
		
		// Finally, now that we have our lexer constructed, we can create the parser
		//
		psr	    = OracleSQLParserNew(tstream);  // CParserNew is generated by ANTLR3
			
		if (psr == NULL)
		{
			// TODO throw here
			throw ParseException();
			exit(ANTLR3_ERR_NOMEM);
		}
		
		langAST = psr->start_rule(psr);
			
		if (psr->pParser->rec->state->errorCount > 0)
		{
			fprintf(stderr, "The parser returned %d errors, tree walking aborted.\n", psr->pParser->rec->state->errorCount);
			throw ParseException();
		}

		//pANTLR3_COMMON_TOKEN root_token = langAST.tree->getToken(langAST.tree);
		_mAST = new Token( NULL
				   ,Position(0,0)
				   , ""
				   , Token::X_ROOT
			);
		_mAST->setTokenATypeName("ROOT");
		ANTLR3_UINT32 lastIndex = 0;
		
		treeWalk(psr, _mAST, langAST.tree, lastIndex);
		
		psr ->free (psr); psr		= NULL;
		tstream ->free (tstream); tstream	= NULL; this->lexerTokenVector = NULL;
		lxr ->free (lxr); lxr		= NULL;
		input ->close (input); input	= NULL;
	};

	/* recursively copy an AST tree into */
	void OracleSQLStatement::treeWalk(pOracleSQLParser psr, QPointer<Token> root,  ANTLR3_BASE_TREE *tree, ANTLR3_UINT32 &lastindex)
	{
		pANTLR3_BASE_TREE pRootNode = (ANTLR3_BASE_TREE*)tree;
	
		// if  (pRootNode->isNilNode(pRootNode) == ANTLR3_TRUE) {
		// 	printf("// nil-node\n");
		// 	//return;
		// 	//TODO throw here
		// }

		ANTLR3_UINT32 uChildCount = pRootNode->getChildCount(pRootNode);
		for  (ANTLR3_UINT32 i=0; i<uChildCount; ++i)
		{
			pANTLR3_BASE_TREE pChildNode = (pANTLR3_BASE_TREE) pRootNode->getChild(pRootNode, i);
			//ANTLR3_UINT32 uChildLexemeType = pChildNode->getType(pChildNode);
			pANTLR3_COMMON_TOKEN pChildLexeme = pChildNode->getToken(pChildNode);

			// if child is not a leaf node - recurse
			if ( pChildNode->getChildCount(pChildNode) != 0)
			{
				// pANTLR3_COMMON_TOKEN parser_token = child->getToken(child);
				// printf("Non-Leaf node \'%s\'(%d)\n",
				//        (const char*)parser_token->getText(parser_token)->chars,
				//        parser_token->user1 //tree->u
				//        );

				// skip useless T_COND_OR_SEQ T_COND_AND_SEQ having only one son
				ANTLR3_UINT32 TokenType = pChildNode->getType(pChildNode);
				if( (TokenType == T_COND_OR_SEQ || TokenType == T_COND_AND_SEQ) && pChildNode->getChildCount(pChildNode) == 1)
				{
					pChildNode = (pANTLR3_BASE_TREE) pChildNode->getChild(pChildNode, 0);
					//uChildLexemeType = pChildNode->getType(pChildNode);
					pChildLexeme = pChildNode->getToken(pChildNode);

					treeWalk(psr, root, pChildNode, lastindex);
					continue;
				}

				Token *childToken = new OracleSQLToken ( root
								      , Position(pChildLexeme->getLine(pChildLexeme), pChildLexeme->getCharPositionInLine(pChildLexeme))
								      , (const char*)pChildLexeme->getText(pChildLexeme)->chars
								      , pChildLexeme->getType(pChildLexeme)
									  , (const char*) psr->pParser->rec->state->tokenNames[ pChildNode->getType(pChildNode) ]
								      , (ANTLR3_UINT64)(pChildNode->u)
					);
				//childToken->setTokenTypeName( (const char*) psr->pParser->rec->state->tokenNames[ pChildNode->getType(pChildNode) ]);

				root->appendChild(childToken); 
				treeWalk(psr, childToken, pChildNode, lastindex);
			} else { // if child is a leaf node
				/* this is a leaf node */
				ANTLR3_MARKER uChildLexemeStart = pChildLexeme->start;
				ANTLR3_UINT32 lexemeTotal = lexerTokenVector->count;
				pANTLR3_COMMON_TOKEN pLocalLexeme;

				/* loop over lexer's tokens until leaf if found */
				while(lastindex < lexemeTotal)
				{
					pLocalLexeme = (pANTLR3_COMMON_TOKEN) lexerTokenVector->get(lexerTokenVector, lastindex);					
					ANTLR3_MARKER uLocalLexemeStart = pLocalLexeme->start;
					if(uLocalLexemeStart == uChildLexemeStart)
						break;
					lastindex++;
				}
				
				// TODO check for EOF in lexer stream */
				printf("Leaf node \'%s\'(%d)\n",
				       (const char*)pChildNode->getText(pChildNode)->chars,
				       lastindex
				       );

				Token *childToken = new OracleSQLToken ( root
									 , Position(pChildLexeme->getLine(pChildLexeme), pChildLexeme->getCharPositionInLine(pChildLexeme))
									 , (const char*)pChildLexeme->getText(pChildLexeme)->chars
									 // Leaf node can be either a reserved (key)word or identifier. Also some keywords can be identifiers.
									 // if the attribute user1 is set then the token is considered to be an identifier
									 // user2 represents either alias declaration or usage
									 , pChildLexeme->user1 ? pChildLexeme->user1 : pChildLexeme->getType(pChildLexeme)
									 , (const char*) psr->pParser->rec->state->tokenNames[ pChildNode->getType(pChildNode) ]
									 , pChildLexeme->user2 ? pChildLexeme->user2 : T_UNKNOWN
					);
				root->appendChild(childToken);
				//childToken->setTokenTypeName( (const char*) psr->pParser->rec->state->tokenNames[ pChildNode->getType(pChildNode) ]);
				
				// Process spaces and comments after parser_token
				while(++lastindex < lexemeTotal)
				{
					pANTLR3_COMMON_TOKEN pSpacerLexeme = (pANTLR3_COMMON_TOKEN) lexerTokenVector->get(lexerTokenVector, lastindex);
					ANTLR3_UINT32 SpacerLexemeChannel = pSpacerLexeme->getChannel(pSpacerLexeme);

					if(SpacerLexemeChannel != HIDDEN )
						break;

					Token *spacerToken =  new Token( root
									 , Position(pSpacerLexeme->getLine(pSpacerLexeme), pSpacerLexeme->getCharPositionInLine(pSpacerLexeme))
									 , (const char*)pSpacerLexeme->getText(pSpacerLexeme)->chars
									 , Token::X_COMMENT
						);
					childToken->appendSpacer(spacerToken);
				}				
			} // else for child is a leaf node
		} // for each child
	};

	/* walk through statement tree and detect identifiers, tables, table aliases */
	void OracleSQLStatement::scanTree()
	{
		/* find all identifiers */
		QVector<Token*> identifierParts;
		bool dotFound = false;
		SQLParser::Statement::token_const_iterator node;
		for(node=begin(); node!=end(); ++node)
		{
			Token* pNode = const_cast<Token*>(&*node);
			if( node->getTokenType() == Token::S_IDENTIFIER)
			{
				QList<QPointer<Token> > const& children = node->getChildren();
				foreach( const QPointer<Token> i, children)
				{
					if( i->toString().compare(QString(".")) == 0)
						continue;
					identifierParts.push_back(i.data());
				}
				_mIdentifiers.push_back(OracleIdentifier(identifierParts));
				identifierParts.clear();
			}
		}
		
		for(SQLParser::Statement::token_const_iterator i=begin(); i!=end(); ++i)
		{
			Token const &node = *i;
			switch( node.getTokenType())
			{
			case Token::L_TABLENAME:
				_mTablesSet.insert( node.toString().toUpper());
				break;
			case Token::L_SCHEMANAME:
				//_mAliases.insert( node.toString().toUpper());
				break;
			case Token::L_TABLEALIAS:
			{
				if( node.getTokenUsageType() != Token::Declaration )
					break;
				
				//TODO loop over rigth brothers unil you find either a reserved word or a table name
				QList<QPointer<Token> > const& brothers = node.parent()->getChildren();
				QPointer<Token> t = const_cast<Token*>(&(*i));
				QString table_name;

				std::cout << "Alias found:" << node.toString().toAscii().constData() << std::endl;
							
				for( int j = brothers.indexOf(t) ; j >= 0; --j)
				{
					QPointer<Token> const brother = brothers.at(j);
					if( brother->getTokenType() == Token::S_SUBQUERY_NESTED)
					{
						std::cout << node.toString().toAscii().constData() << "=>"
							  << (*brother).toStringRecursive().toStdString() << std::endl;
						_mDeclarations.insertMulti(node.toString(), brother.data());
						for(SQLParser::Statement::token_const_iterator_to_root k(&*i); &*k; ++k)
						{							
							std::cout << k->toString().toStdString() << "->";
						}
						std::cout << std::endl;						
						break;						
					}
					if( brother->getTokenType() == Token::S_TABLE_REF)
					{
						std::cout << node.toString().toAscii().constData() << "=>"
							  << (*brother).toStringRecursive().toAscii().constData() << std::endl;
						_mDeclarations.insertMulti(node.toString(), brother.data());
						for(SQLParser::Statement::token_const_iterator_to_root k(&*i); &*k; ++k)
						{							
							std::cout << k->toString().toStdString() << "->";
						}
						std::cout << std::endl;
						addTranslation(node.toString(), brother.data(), &node);
						break;
					}
					if( brother->toString().compare(QString(",")) == 0)
						break;
				}
				
				_mAliasesSet.insert( node.toString().toUpper());
				break;
			}
			case Token::L_SUBQUERY_ALIAS:
			{
				std::cout << "Subquery alias found:" << node.toString().toAscii().constData() << std::endl;
				QList<QPointer<Token> > const& brothers = node.parent()->getChildren();
				bool nodeFound = false;
				foreach( const QPointer<Token> brother, brothers)
				{
					if( brother == &node)
					{
						nodeFound = true;
						continue;
					}
					if( nodeFound == false)
						continue;
					if( brother->getTokenType() == Token::S_SUBQUERY_FACTORED)
					{
						std::cout << node.toString().toAscii().constData() << "=>"
							  << (*brother).toStringRecursive().toAscii().constData() << std::endl;
						_mDeclarations.insertMulti(node.toString(), brother.data());
						break;
					}
				}
				if( node.getTokenUsageType() == Token::Declaration )
					_mAliasesSet.insert( node.toString().toUpper());
				break;
			}
			} // switch(node.getTokenType())
		}
	};

	void OracleSQLStatement::disambiguate()
	{
		/* loop over all the identifiers and try to find table alias translation in the AST tree */
		foreach( Identifier const &i, _mIdentifiers)
		{
			if ( i.length() == 1)
				continue;

			OracleSQLToken const* context;
			if (( context = dynamic_cast<OracleSQLToken*>(i._mFields.at(0))) == NULL)
				continue;
				
			QString alias = i._mFields.at(0)->toString();
			OracleSQLToken const* table = ( OracleSQLToken*)translate(alias, context);

			if( table)
				std::cout << "al: " << i.toString().toStdString() << "=>" << table->toStringRecursive().toStdString() << std::endl;
		}
		/* TODO - table name can also point onto SUBQUERY_FACTORED */		
	};
	
	void OracleSQLStatement::addTranslation(QString const& alias, Token const *tableOrSubquery, Token const *context)
	{
		for( SQLParser::Statement::token_const_iterator_to_root k(context); k->parent(); ++k)
		{
			std::cout << "tr: " << k->toString().toStdString() << std::endl;
			if( k->getTokenType() == Token::S_SUBQUERY_NESTED ||
			    k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
			    k->getTokenType() == Token::X_ROOT)
			{
				if( OracleSQLToken const *l = dynamic_cast<OracleSQLToken const*>((Token const*)k))
				{
					OracleSQLToken &m = *(const_cast<OracleSQLToken*>(l));
					m._mTranslation.insert(alias, const_cast<Token*>(tableOrSubquery));
				}
				break;
			}
		}
	};

	Token const* OracleSQLStatement::translate(QString const& alias, Token const *context)
	{
		for( SQLParser::Statement::token_const_iterator_to_root k(context); k->parent(); ++k)
		{
			if( k->getTokenType() == Token::S_SUBQUERY_NESTED ||
			    k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
			    k->getTokenType() == Token::X_ROOT)
			{
				if( OracleSQLToken const *l = dynamic_cast<OracleSQLToken const*>((Token const*)k))
				{
					OracleSQLToken &m = *(const_cast<OracleSQLToken*>(l));
					if(m._mTranslation.contains(alias))
						return m._mTranslation.value(alias);
				}
			}
		}
		return NULL;
	};
	
	/*virtual*/ void OracleSQLStatement::tree2Dot(std::ostream &o) const
	{
		Token const* _root = Statement::root();
		QString s;
		//recursiveWalk<>(_root, s);
	};
	
}; // namespace SQLParser

Util::RegisterInFactory<SQLParser::OracleSQLStatement, StatementFactTwoParmSing> regOracleSQLStatement("OracleSQL");
