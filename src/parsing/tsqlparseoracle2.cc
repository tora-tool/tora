
#include "OracleDMLTraits.hpp"
#include "OracleDMLLexer.hpp"
#include "OracleDML.hpp"
#include "OracleDML_OracleDMLKeys.hpp"
#include "OracleDML_OracleDMLCommons.hpp"

#include "parsing/tsqlparse.h"
//#include "core/tologger.h"

#include <QPair>

using namespace std;

namespace SQLParser
{
class OracleDMLToken: public Token
{
	using Tokens = Antlr3BackendImpl::OracleDMLLexerTokens;
	using AntlrToken = Antlr3BackendImpl::OracleSQLParserTraits::CommonTokenType;
public:
	OracleDMLToken (Token *parent, const Position &pos, const QString &str, unsigned tokentype, const char* tokentypestring, unsigned usagetype = Tokens::T_UNKNOWN);
	OracleDMLToken (Token *parent, AntlrToken &token);
};

OracleDMLToken::OracleDMLToken (Token *parent, const Position &pos, const QString &str, unsigned tokentype, const char* tokentypestring, unsigned usagetype)
    : Token(parent, pos, str)
{
	using Tokens = Antlr3BackendImpl::OracleDMLLexerTokens;
    _mTokenATypeName = tokentypestring;

    UsageType &_mUsageTypeRef = const_cast<Token::UsageType&>(_mUsageType);
    switch(usagetype)
    {
    case 0:
	case Tokens::T_UNKNOWN:
        _mUsageTypeRef = Unknown;
        break;
    case Tokens::T_USE:
        _mUsageTypeRef = Usage;
        break;
	case Tokens::T_DECL:
        _mUsageTypeRef = Declaration;
        break;
    }

    TokenType &_mTokenTypeRef = const_cast<Token::TokenType&>(_mTokenType);
    switch(tokentype)
    {
	case Tokens::T_UNKNOWN:
        _mTokenTypeRef = X_UNASSIGNED;
        break;
	case Tokens::T_RESERVED:
        _mTokenTypeRef = L_RESERVED;
        break;
	case Tokens::T_TABLE_NAME:
        _mTokenTypeRef = L_TABLENAME;
        break;
	case Tokens::T_TABLE_ALIAS:
        // Resolve grammar ambiguity: SELECT * FROM A INNER JOIN B; (=> INNER is not a table alias)
        // The same for NATURAL JOIN, CROSS JOIN, LEFT/RIGHT OUTER JOIN
        if( usagetype == Tokens::T_DECL &&  (!str.compare("INNER", Qt::CaseInsensitive) ||
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
	case Tokens::T_SCHEMA_NAME:
        _mTokenTypeRef = L_SCHEMANAME;
        break;
    case Tokens::T_TABLE_REF:
        _mTokenTypeRef = S_TABLE_REF;
        break;
    case Tokens::T_FUNCTION_NAME:
        _mTokenTypeRef = L_FUNCTIONNAME;
        break;
    case Tokens::T_PACKAGE_NAME:
        _mTokenTypeRef = L_PACKAGENAME;
        break;
    case Tokens::T_BINDVAR_NAME:
        _mTokenTypeRef = L_BINDVARNAME;
        break;
    case Tokens::T_COLUMN_ALIAS:
    case Tokens::T_COLUMN_NAME:
        _mTokenTypeRef = L_IDENTIFIER;
        break;
    case Tokens::T_WITH:
        _mTokenTypeRef = S_WITH;
        break;
    case Tokens::T_UNION:
        _mTokenTypeRef = S_UNION;
        break;
    case Tokens::T_SELECT:
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
    case Tokens::T_COLUMN_LIST:
        _mTokenTypeRef = S_COLUMN_LIST;
        break;
    case Tokens::T_TABLE_CAST:
        _mTokenTypeRef = S_SUBQUERY_NESTED;
        break;
    case Tokens::T_FROM:
        _mTokenTypeRef = S_FROM;
        break;
    case Tokens::T_JOINING_CLAUSE:
        _mTokenTypeRef = L_JOINING_CLAUSE;
        break;
    case Tokens::T_WHERE:
        _mTokenTypeRef = S_WHERE;
        break;
    case Tokens::T_IDENTIFIER:
        _mTokenTypeRef = S_IDENTIFIER;
        break;
    case Tokens::T_OPERATOR_BINARY:
        _mTokenTypeRef = S_OPERATOR_BINARY;
        break;
    } // switch(tokentype)

};

OracleDMLToken::OracleDMLToken (Token *parent, AntlrToken &token)
	: Token(parent, Position(token.get_line(), token.get_charPositionInLine()), token.getText().c_str())
{
	
};
	
class OracleDMLStatement: public Statement
{
	using TokenStream = Antlr3BackendImpl::OracleSQLParserTraits::TokenStreamType;
	using Traits = Antlr3BackendImpl::OracleSQLParserTraits;
public:
    OracleDMLStatement(const QString &statement, const QString &name);
    virtual ~OracleDMLStatement() {};

    //template<class T> T& recursiveWalk(Token const* node, T &stream) const;
private:
    void parse();
    /* Recursive walk through ANTLR3_BASE_TREE and create AST tree*/
    void treeWalk(unique_ptr<Antlr3BackendImpl::OracleDML> &psr, QPointer<Token> root, Traits::TreeTypePtr& tree, unsigned &lastindex);

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

	TokenStream::TokensType* lexerTokenVector;
};

OracleDMLStatement::OracleDMLStatement(const QString &statement, const QString &name) : Statement(statement, name)
{
    _mStatementType = S_SELECT;
    parse();
    disambiguate();
};

void OracleDMLStatement::parse()
{
	using InputStream = Antlr3BackendImpl::OracleSQLParserTraits::InputStreamType;
	using TokenStream = Antlr3BackendImpl::OracleSQLParserTraits::TokenStreamType;
	//using Token = Antlr3BackendImpl::OracleSQLParserTraits::CommonTokenType;
	using Tokens = Antlr3BackendImpl::OracleDMLTokens;
	using namespace std;

	_mState = P_ERROR;
	QByteArray QBAinput(_mStatement.toUtf8());
	QByteArray QBAname(_mname.toUtf8());

	_mState = P_INIT;
	auto input = make_unique<InputStream>((const ANTLR_UINT8 *)QBAinput.data(), antlr3::ENC_8BIT, QBAinput.length(), (ANTLR_UINT8*)QBAname.data());
	if (input == NULL)
		throw ParseException();
	input->setUcaseLA(true); // ignore case

	auto lxr = make_unique<Antlr3BackendImpl::OracleDMLLexer>(input.get());
	if ( lxr == NULL )
		throw ParseException();

	auto tstream = make_unique<TokenStream>(ANTLR_SIZE_HINT, lxr->get_tokSource());
	if (tstream == NULL)
	{
		throw ParseException();
	}

	lexerTokenVector = tstream->getTokens();
	_mState = P_LEXER;

	// Finally, now that we have our lexer constructed, we can create the parser
	auto psr = make_unique<Antlr3BackendImpl::OracleDML>(tstream.get());
	if (psr == NULL)
		throw ParseException();

	auto langAST = psr->seq_of_statements();
	if (psr->getNumberOfSyntaxErrors())
	{
		// TODO throw reasonable exception HERE
		throw ParseException();
	}
	_mState = P_PARSER;
	
	_mAST = new TokenSubquery( NULL
				   , Position(0, 0)
				   , ""
				   , Token::X_ROOT
		);
	_mAST->setTokenATypeName("ROOT");
	unsigned lastIndex = 0;

	treeWalk(psr, _mAST, langAST.tree, lastIndex);
	lexerTokenVector->clear();
};

/* recursively copy an AST tree into */
void OracleDMLStatement::treeWalk(unique_ptr<Antlr3BackendImpl::OracleDML> &psr, QPointer<Token> root, Traits::TreeTypePtr &tree, unsigned &lastindex)
{
	using Tokens = Antlr3BackendImpl::OracleDMLLexerTokens;
	auto &children = tree->get_children();
	for (auto i = children.begin(); i != children.end(); ++i)
	{
		Traits::TreeTypePtr &childNode(*i);
		auto childToken = childNode->get_token();

		// if child is not a leaf node - recurse
		if (!childNode->get_children().empty())
		{
			// skip useless T_COND_OR_SEQ T_COND_AND_SEQ having only one son
			ANTLR_UINT32 TokenType = childNode->getType();
			if( (TokenType == Tokens::T_COND_OR_SEQ || TokenType == Tokens::T_COND_AND_SEQ) && childNode->getChildCount() == 1)
			{
				auto &grandChildNode = childNode->getChild(0);
				auto grandChildToken = grandChildNode->get_token();

				treeWalk(psr, root, grandChildNode, lastindex);
				continue;
			}

			Token *childTokenNew = new OracleDMLToken ( root
								    , Position(childToken->get_line(), childToken->get_charPositionInLine())
								    , childToken->getText().c_str()
								    , childToken->getType()
								    , childNode->getType() == Tokens::EOF_TOKEN ? "EOF" : (const char *)Antlr3BackendImpl::OracleDML::getTokenNames()[childNode->getType()]
								    , childNode->UserData.usageType
				);
			//childToken->setTokenTypeName( (const char*) psr->pParser->rec->state->tokenNames[ pChildNode->getType(pChildNode) ]);

			root->appendChild(childTokenNew);
			treeWalk(psr, childTokenNew, childNode, lastindex);
		}
		else     // if child is a leaf node
		{
			/* this is a leaf node */
			ANTLR_MARKER uChildLexemeStart = childToken->get_tokenIndex();
			auto lexemeTotal = lexerTokenVector->size();

			/* loop over lexer's tokens until leaf if found */
			while(lastindex < lexemeTotal)
			{
				auto &localToken = lexerTokenVector->at(lastindex);
				ANTLR_MARKER uLocalLexemeStart = localToken.get_tokenIndex();
				if(uLocalLexemeStart == uChildLexemeStart)
					break;
				lastindex++;
			}

			// TODO check for EOF in lexer stream */
			//printf("Leaf node \'%s\'(%d)\n",
			//       (const char*)pChildNode->getText(pChildNode)->chars,
			//       lastindex
			//       );

			Token *childTokenNew = 
				new OracleDMLToken(
					root
					, Position(childToken->get_line(), childToken->get_charPositionInLine())
					, childToken->getText().c_str()
					// Leaf node can be either a reserved (key)word or an identifier. Also some keywords can be identifiers.
					// if the attribute toraTokenType is set then the token is considered to be an identifier
					// usageType represents either alias declaration or usage
					, childToken->UserData.toraTokenType ? childToken->UserData.toraTokenType : childToken->getType()
					, childNode->getType() == Tokens::EOF_TOKEN ? "EOF" : (const char*)Antlr3BackendImpl::OracleDML::getTokenNames()[childNode->getType()]
					, childToken->UserData.usageType ? childToken->UserData.usageType : Tokens::T_UNKNOWN
					);
			root->appendChild(childTokenNew);
			//childToken->setTokenTypeName( (const char*) psr->pParser->rec->state->tokenNames[ pChildNode->getType(pChildNode) ]);

			// Process spaces and comments after parser_token
			while(++lastindex < lexemeTotal)
			{
				auto &spacerToken = lexerTokenVector->at(lastindex);
				unsigned SpacerLexemeChannel = spacerToken.get_channel();

				if(SpacerLexemeChannel != antlr3::HIDDEN )
					break;

				Token *spacerTokenNew =  new Token( root
								    , Position(spacerToken.get_line(), spacerToken.get_charPositionInLine())
								    , spacerToken.getText().c_str()
								    , Token::X_COMMENT
					);
				childTokenNew->appendSpacer(spacerTokenNew);
			}
		} // else for child is a leaf node
	} // for each child
};

void OracleDMLStatement::disambiguate()
{
    // First of all replace some instances of Token with Token's sub-classes
    for(SQLParser::Statement::token_const_iterator i = begin(); i != end(); ++i)
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

    for(SQLParser::Statement::token_const_iterator i = begin(); i != end(); ++i)
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
                    //  std::cout << k->toString().toStdString() << "->";
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
                    //  std::cout << k->toString().toStdString() << "->";
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
            //  _mAliasesSet.insert( node.toString().toUpper());
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
void OracleDMLStatement::scanTree(ObjectCache* o, QString const& cs)
{
    for(SQLParser::Statement::token_const_iterator i = begin(); i != end(); ++i)
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
                    //TLOG(0, toNoDecorator, __HERE__) << " Dont Resolve identifier: " << i->toStringRecursive(false).toStdString() << "\t under: " <<  stopToken.toStdString() << std::endl;
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
            //TLOG(0, toNoDecorator, __HERE__) << "Resolve identifier: " << id.toStringRecursive(false).toStdString() << "\t under: " <<  stopToken.toStdString() << std::endl;

            QString schemaName, tableName, columnName;

            switch(id.childCount()) // odd children should be dots.
            {
            case 0:
            case 2:
            case 4:
                //TLOG(0, toNoDecorator, __HERE__) << " invalid length:" << id.childCount() << std::endl;
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
                //TLOG(0, toNoDecorator, __HERE__) << " identifier found " << cs << '.' << tableName << '.' << columnName << ':' << columnExists << std::endl;
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
                //TLOG(0, toNoDecorator, __HERE__) << " identifier found: " << schemaName << '.' << tableName << '.' << columnName << ':' << columnExists << std::endl;
            }
            break;
            default:
                //TLOG(0, toNoDecorator, __HERE__) << " unsupported length:" << id.childCount() << std::endl;
                break;
            } // switch(id.childCount()) // odd children should be dots.
        }
    }
};

/* Information stored using this function can be queried using translateAlias
*/
void OracleDMLStatement::addTranslation(QString const& alias, Token const *tableOrSubquery, Token const *context)
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
void OracleDMLStatement::addTableRef(Token const *tableRef, Token const *context)
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

Util::RegisterInFactory<SQLParser::OracleDMLStatement, StatementFactTwoParmSing> regOracleDMLStatement("OracleDML");
