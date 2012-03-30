
// ANTLR runtime includes
#include "antlr3commontoken.h"
#include "antlr3string.h"
#include "antlr3input.h"
#include "antlr3lexer.h"

#include "OracleSQLLexer.h"
#include "OracleSQLParser.h"

#include "tsqlparse.h"
#include "tologger.h"

#include <QPair>

namespace SQLParser
{	
	class OracleSQLToken: public Token
	{
	public:
		OracleSQLToken (Token *parent, const Position &pos, const QString &str, unsigned tokentype, const char* tokentypestring, unsigned usagetype = T_UNKNOWN);				
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
		case T_COLUMN_LIST:
			_mTokenTypeRef = S_COLUMN_LIST;
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
		case T_OPERATOR_BINARY:
			_mTokenTypeRef = S_OPERATOR_BINARY;
			break;
		} // switch(tokentype)

	};
	
	class OracleSQLStatement: public Statement
	{
	public:
		OracleSQLStatement(const QString &statement, const QString &name);
		virtual ~OracleSQLStatement() {};

		//template<class T> T& recursiveWalk(Token const* node, T &stream) const;
	private:
		void parse ();
		/* Recursive walk through ANTLR3_BASE_TREE and create AST tree*/
		void treeWalk(pOracleSQLParser psr, QPointer<Token> root,  ANTLR3_BASE_TREE *tree, ANTLR3_UINT32 &lastindex);
		
		/* Walk through Token tree and look for table names, table aliases, ... and try to resolve them
		   Note: this function also replaces some instances of Token* with Token's subclass instances
		   */
		void disambiguate();
		
	public:
		/* Walk through Token tree and look for table names, table aliases, ... and try to resolve them using cached database catalogue*/
		virtual void scanTree(ObjectCache *, QString const&);
		
	private:		
		void addTranslation(QString const& alias, Token const *tableOrSubquery, Token const *context);
		void addTableRef(Token const *tableOrSubquery, Token const *context);
		
		pANTLR3_VECTOR lexerTokenVector;
	};

	OracleSQLStatement::OracleSQLStatement(const QString &statement, const QString &name) : Statement(statement, name)
	{
		_mStatementType = S_SELECT;
		parse();
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
			// TODO throw reasonable exception HERE
			// fprintf(stderr, "The parser returned %d errors, tree walking aborted.\n", psr->pParser->rec->state->errorCount);
			throw ParseException();
		}

		//pANTLR3_COMMON_TOKEN root_token = langAST.tree->getToken(langAST.tree);
		_mAST = new TokenSubquery( NULL
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
									 ,
									 (
										 pChildNode->getType(pChildNode) == ANTLR3_TOKEN_EOF
										 ?
										 (const char *)"EOF"
										 :										 
										 (const char*) psr->pParser->rec->state->tokenNames[ pChildNode->getType(pChildNode) ]
									 )
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
				//printf("Leaf node \'%s\'(%d)\n",
				//       (const char*)pChildNode->getText(pChildNode)->chars,
				//       lastindex
				//       );

				Token *childToken = new OracleSQLToken ( root
									 , Position(pChildLexeme->getLine(pChildLexeme), pChildLexeme->getCharPositionInLine(pChildLexeme))
									 , (const char*)pChildLexeme->getText(pChildLexeme)->chars
									 // Leaf node can be either a reserved (key)word or identifier. Also some keywords can be identifiers.
									 // if the attribute user1 is set then the token is considered to be an identifier
									 // user2 represents either alias declaration or usage
									 , pChildLexeme->user1 ? pChildLexeme->user1 : pChildLexeme->getType(pChildLexeme)
									 , 
									 (
										 pChildNode->getType(pChildNode) == ANTLR3_TOKEN_EOF
										 ?
										 (const char *)"EOF"
										 :										 
										 (const char*) psr->pParser->rec->state->tokenNames[ pChildNode->getType(pChildNode) ]
									 )									
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

	void OracleSQLStatement::disambiguate()
	{
		// First of all replace some instances of Token with Token's sub-classes
		for(SQLParser::Statement::token_const_iterator i=begin(); i!=end(); ++i)
		{
			Token const &node = *i;
			switch( node.getTokenType())
			{
			case Token::S_TABLE_REF:
			{
				i--; // At this moment iterator's stack points onto node beeing replaced.
				Token *parent = node.parent();
				Token *me = const_cast<Token*>(&node);
				TokenTable *newToken = new TokenTable(node);
				parent->replaceChild(me->row(), newToken);
				i++;
				break;
			}
			case Token::S_SUBQUERY_FACTORED:
			case Token::S_SUBQUERY_NESTED:
			case Token::X_ROOT: // TODO fix me. T_SELECT under T_UNION is also marked as X_ROOT => there is more than one X_ROOT
			{
				if(node.parent() == NULL)
					break;

				i--; // At this moment iterator's stack points onto node beeing replaced.
				Token *parent = node.parent();
				Token *me = const_cast<Token*>(&node);
				TokenSubquery *newToken = new TokenSubquery(node);
				parent->replaceChild(me->row(), newToken);
				i++;
				break;
			}
			case Token::S_IDENTIFIER:
			{
				i--; // At this moment iterator's stack points onto node beeing replaced.
				Token *parent = node.parent();
				Token *me = const_cast<Token*>(&node);
				TokenIdentifier *newToken = new TokenIdentifier(node);
				parent->replaceChild(me->row(), newToken);
				i++;
				break;
			}
			} // switch
		} // for each AST node

		for(SQLParser::Statement::token_const_iterator i=begin(); i!=end(); ++i)
		{
			Token const &node = *i;
			switch( node.getTokenType())
			{
			/*
				TABLE ALIAS was found:
				- loop over left brothers until you find TABLE_REF or SUBQUERY_NESTED
				- update brothers alias attribute
				- find parent of type SUBQUERY(or X_ROOT) and insert translation ALIAS => BROTHER
			*/
			case Token::L_TABLEALIAS:
			{
				if( node.getTokenUsageType() != Token::Declaration )
					break;
				
				//loop over left brothers until you find either a reserved word or a table name
				QList<QPointer<Token> > const& brothers = node.parent()->getChildren();
				////std::cout << "Alias found:" << node.toString().toAscii().constData() << std::endl;							
				for( int j = node.row() - 1 ; j >= 0; --j)
				{
					Token *brother = brothers.at(j);
					if( brother->getTokenType() == Token::S_SUBQUERY_NESTED)
					{
						////std::cout << node.toString().toStdString() << "=>" << brother->toStringRecursive().toStdString() << std::endl;

						//_mDeclarations.insertMulti(node.toString(), brother.data());
						//for(SQLParser::Statement::token_const_iterator_to_root k(&*i); &*k; ++k)
						//{							
						//	std::cout << k->toString().toStdString() << "->";
						//}						
						//std::cout << std::endl;
						
						static_cast<TokenSubquery*>(brother)->setNodeAlias(&node);
						addTranslation(node.toString(), brother, node.parent());
						break;						
					}
					if( brother->getTokenType() == Token::S_TABLE_REF)
					{
						//std::cout << node.toString().toAscii().constData() << "=>" << (*brother).toStringRecursive().toAscii().constData() << std::endl;

						//_mDeclarations.insertMulti(node.toString(), brother.data());
						//for(SQLParser::Statement::token_const_iterator_to_root k(&*i); &*k; ++k)
						//{							
						//	std::cout << k->toString().toStdString() << "->";
						//}
						//std::cout << std::endl;
						
						static_cast<TokenSubquery*>(brother)->setNodeAlias(&node);
						addTranslation(node.toString(), brother, node.parent());
						break;
					}

					/* We have not found anything usefull:
						SELECT * FROM TABLE_A, TABLE(CAST(VariableX) AS TABLE) X; => no translation for table alias X
						TODO - do break on "JOIN" (joining clause) too.
						SELECT * FROM TABLE_A A NATURAL JOIN TABLE(CAST(VariableX) AS TABLE) X; => no translation for table alias X
					*/
					if( brother->toString().compare(QString(",")) == 0)
						break;
					if( brother->getTokenType() == Token::L_TABLEALIAS)
						break;
					if( brother->getTokenType() == Token::L_RESERVED)
						break;
				}
				
				break;
			}
			/*
				SUBQUERY ALIAS was found:
				- loop over right brothers until you find SUBQUERY_FACTORED
				- update brothers alias attribute
				- find parent of type SUBQUERY(or X_ROOT) and insert translation ALIAS => BROTHER
			*/
			case Token::L_SUBQUERY_ALIAS:
			{				
				//loop over rigth brothers until you find S_SUBQUERY_FACTORED
				QList<QPointer<Token> > const& brothers = node.parent()->getChildren();				
				for( int j = node.row() + 1 ; j < node.parent()->childCount() ; ++j)
				{
					Token *brother = brothers.at(j);
					
					if( brother->getTokenType() == Token::S_SUBQUERY_FACTORED)
					{
						//std::cout << node.toString().toAscii().constData() << "=>" << brother->toStringRecursive().toStdString() << std::endl;
						
						//_mDeclarations.insertMulti(node.toString(), brother.data());
						
						static_cast<TokenSubquery*>(brother)->setNodeAlias(&node);
						addTranslation(node.toString(), brother, node.parent());
						
						//std::cout << "Subquery alias found:" << node.toString().toStdString() << "->" << brother->toString().toStdString() << std::endl;
						break;
					}
				}
				//if( node.getTokenUsageType() == Token::Declaration )
				//	_mAliasesSet.insert( node.toString().toUpper());
				break;
			}
			/* A table reference was found. Traverse to ROOT, try to find a SUBQUERies ROOT
			   and update its table list
			*/
			case Token::S_TABLE_REF:
			{
				addTableRef(&node, node.parent());
				_mTablesList.append(&*i);
				break;
			}
			case Token::S_SUBQUERY_FACTORED:
			case Token::S_SUBQUERY_NESTED:
			case Token::X_ROOT: // TODO fix me. T_SELECT under T_UNION is also marked as X_ROOT => there is more than one X_ROOT node in AST tree.
			{
				if(node.parent() == NULL)
					break;				
				addTableRef(&node, node.parent());
				break;
			}
			} // switch(node.getTokenType())
		} // for each AST node
	};	
	
	/* walk through statement tree and detect identifiers, tables, table aliases */
	void OracleSQLStatement::scanTree(ObjectCache* o, QString const& cs)
	{
		for(SQLParser::Statement::token_const_iterator i=begin(); i!=end(); ++i)
		{
			if( i->getTokenType() == Token::S_IDENTIFIER )
			{
				QString stopToken;
				bool insideColumnList;

				// Do not resolve identifiers under S_COLUMN_LIST
				for(token_iterator_to_root k(i->parent()); k->parent(); ++k)
				{
					stopToken = k->toString();
					if( k->getTokenType() == Token::S_COLUMN_LIST)
					{
						TLOG(0,toNoDecorator,__HERE__) << " Dont Resolve identifier: " << i->toStringRecursive(false).toStdString() << "\t under: " <<  stopToken.toStdString() << std::endl;
						insideColumnList = true;
						break;
					}
					if( k->getTokenType() == Token::X_ROOT ||
						k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
						k->getTokenType() == Token::S_SUBQUERY_NESTED ||
						k->getTokenType() == Token::S_WHERE ||
						k->getTokenType() == Token::L_JOINING_CLAUSE)
					{
						insideColumnList = false;
						break;
					}
				};
				if( insideColumnList)
					continue;

				TokenIdentifier const& id = static_cast<TokenIdentifier const&>(*i);
				TLOG(0,toNoDecorator,__HERE__) << "Resolve identifier: " << id.toStringRecursive(false).toStdString() << "\t under: " <<  stopToken.toStdString() << std::endl;

				QString schemaName, tableName, columnName;

				switch(id.childCount()) // odd children should be dots.
				{
				case 0:
				case 2:
				case 4:
					TLOG(0,toNoDecorator,__HERE__) << " invalid length:" << id.childCount() << std::endl;
					break;
				case 1:
					// We have column name only
					break;
				case 3:
					{
						// T1.C1
						tableName = id.child(0)->toStringRecursive(false).toUpper();
						// TODO check child(1) == '.'
						columnName = id.child(2)->toStringRecursive(false).toUpper();

						Token const *translatedAlias = translateAlias(tableName, &*i);
						if( translatedAlias)
						{
							if(translatedAlias->childCount() == 3)
							{
								schemaName = translatedAlias->child(0)->toStringRecursive(false).toUpper();
								tableName = translatedAlias->child(2)->toStringRecursive(false).toUpper();
								goto label5;
							}
							if(translatedAlias->childCount() == 1)
							{
								tableName = translatedAlias->child(0)->toStringRecursive(false).toUpper();
							}
						}
						bool columnExists = o->columnExists(cs, tableName, columnName);
						TLOG(0,toNoDecorator,__HERE__) << " identifier found " << cs << '.' << tableName << '.' << columnName << ':' << columnExists << std::endl;
					}
					break;
					label5:
				case 5:
					{
						if( schemaName.isEmpty() && tableName.isEmpty())
						{
							schemaName = id.child(0)->toStringRecursive(false).toUpper();
							tableName = id.child(2)->toStringRecursive(false).toUpper();
							columnName = id.child(4)->toStringRecursive(false).toUpper();
						}
						bool columnExists = o->columnExists(schemaName, tableName, columnName);
						TLOG(0,toNoDecorator,__HERE__) << " identifier found: " << schemaName << '.' << tableName << '.' << columnName << ':' << columnExists << std::endl;
					}
					break;
				default:
					TLOG(0,toNoDecorator,__HERE__) << " unsupported length:" << id.childCount() << std::endl;
					break;
				} // switch(id.childCount()) // odd children should be dots.
			}
		}
	};
	
	/* Information stored using this function can be queried using translateAlias
	*/
	void OracleSQLStatement::addTranslation(QString const& alias, Token const *tableOrSubquery, Token const *context)
	{
		for( SQLParser::Statement::token_iterator_to_root k(const_cast<Token*>(context)); k->parent(); ++k)
		{
			//std::cout << "tr: " << k->toString().toStdString() << std::endl;
			if( k->getTokenType() == Token::S_SUBQUERY_NESTED ||
				k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
				k->getTokenType() == Token::X_ROOT)
			{
				TokenSubquery &s = static_cast<TokenSubquery&>(*k);
				s.aliasTranslation().insert(alias.toUpper(), const_cast<Token*>(tableOrSubquery));
				//std::cout << "Alias translation added:" << alias.toStdString() << "=>" << tableOrSubquery->toStringRecursive(false).toStdString() << " context: " << k->toString().toStdString() << std::endl;
				break;
			}
		}
	};

	/* Information stored using this function can be queried using getTableRef
	*/
	void OracleSQLStatement::addTableRef(Token const *tableRef, Token const *context)
	{
		for( SQLParser::Statement::token_iterator_to_root k(const_cast<Token*>(context)); k->parent(); ++k)
		{
			//std::cout << "tr: " << k->toString().toStdString() << std::endl;
			if( k->getTokenType() == Token::S_SUBQUERY_NESTED ||
				k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
				k->getTokenType() == Token::X_ROOT)
			{
				TokenSubquery &s = static_cast<TokenSubquery&>(*k);
				s.nodeTables().insertMulti(tableRef->toStringRecursive(false).toUpper(), const_cast<Token*>(tableRef));
				//std::cout << "Table ref added:" << tableOrSubquery->toStringRecursive(false).toStdString() << " context: " << k->toString().toStdString() << std::endl;
				break;
			}
		}
	};

}; // namespace SQLParser

Util::RegisterInFactory<SQLParser::OracleSQLStatement, StatementFactTwoParmSing> regOracleSQLStatement("OracleSQL");
