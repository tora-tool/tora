
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
	using AntlrNode = Antlr3BackendImpl::OracleSQLParserTraits::TreeType;
public:
	OracleDMLToken (Token *parent, AntlrNode &token);
};

OracleDMLToken::OracleDMLToken (Token *parent, AntlrNode &node)
	: Token(parent, Position(node.get_line(), node.get_charPositionInLine()), node.getText().c_str())
{
	using Tokens = Antlr3BackendImpl::OracleDMLLexerTokens;
	_mTokenATypeName = node.getType() == Tokens::EOF_TOKEN ? "EOF" : (const char *)Antlr3BackendImpl::OracleDML::getTokenNames()[node.getType()];

	UsageType &usageTypeRef = const_cast<Token::UsageType&>(_mUsageType);
	unsigned usagetype = node.UserData.usageType;
	if (node.get_token() && node.get_token()->UserData.usageType)
		usagetype = node.get_token()->UserData.usageType;
	switch(usagetype)
	{
	case 0:
	case Tokens::T_UNKNOWN:
		usageTypeRef = Unknown;
		break;
	case Tokens::T_USE:
		usageTypeRef = Usage;
		break;
	case Tokens::T_DECL:
		usageTypeRef = Declaration;
		break;
	}

	TokenType &tokenTypeRef = const_cast<Token::TokenType&>(_mTokenType);
	// toraTokenType has precedence, if defined
	unsigned toratokentype = node.UserData.toraTokenType;
	if (node.get_token() && node.get_token()->UserData.toraTokenType)
		toratokentype = node.get_token()->UserData.toraTokenType;

	switch(toratokentype ? toratokentype : node.getType())
	{
	case Tokens::T_UNKNOWN:
		tokenTypeRef = X_UNASSIGNED;
		break;
	case Tokens::T_RESERVED:
		tokenTypeRef = L_RESERVED;
		break;
	case Tokens::T_TABLE_NAME:
		tokenTypeRef = L_TABLENAME;
		break;
	case Tokens::T_TABLE_ALIAS:
	{
		// Resolve grammar ambiguity: SELECT * FROM A INNER JOIN B; (=> INNER is not a table alias)
		// The same for NATURAL JOIN, CROSS JOIN, LEFT/RIGHT OUTER JOIN
		QString str = QString::fromStdString(node.getText()).toUpper();
		cout << "Tokens::T_TABLE_ALIAS:" << qPrintable(str) << endl;
		if( usageTypeRef == Tokens::T_DECL &&  (!str.compare("INNER", Qt::CaseInsensitive) ||
							!str.compare("CROSS", Qt::CaseInsensitive) ||
							!str.compare("NATURAL", Qt::CaseInsensitive) ||
							!str.compare("LEFT", Qt::CaseInsensitive) ||
							!str.compare("RIGHT", Qt::CaseInsensitive)
			    )
			)
		{
			tokenTypeRef = L_RESERVED;
			usageTypeRef = Unknown;
			_mTokenATypeName = "T_RESERVED";
			break;
		}
		switch(parent->getTokenType())
		{
		case S_WITH:
			tokenTypeRef = L_SUBQUERY_ALIAS;
			break;
		default:
			tokenTypeRef = L_TABLEALIAS;
			break;
		}
	}
	break;
	case Tokens::T_SCHEMA_NAME:
		tokenTypeRef = L_SCHEMANAME;
		break;
	case Tokens::T_TABLE_REF:
		tokenTypeRef = S_TABLE_REF;
		break;
	case Tokens::T_FUNCTION_NAME:
		tokenTypeRef = L_FUNCTIONNAME;
		break;
	case Tokens::T_PACKAGE_NAME:
		tokenTypeRef = L_PACKAGENAME;
		break;
	case Tokens::T_BINDVAR_NAME:
		tokenTypeRef = L_BINDVARNAME;
		break;
	case Tokens::T_COLUMN_ALIAS:
	case Tokens::T_COLUMN_NAME:
		tokenTypeRef = L_IDENTIFIER;
		break;
	case Tokens::T_WITH:
		tokenTypeRef = S_WITH;
		break;
	case Tokens::T_UNION:
		tokenTypeRef = S_UNION;
		break;
	case Tokens::FACTORING:
	case Tokens::SUBQUERY:
		break;
	case Tokens::SQL92_RESERVED_SELECT:
		break;
	case Tokens::T_SELECT:
		switch(parent->getTokenType())
		{
		case S_WITH:
			tokenTypeRef = S_SUBQUERY_FACTORED;
			break;
		case S_FROM:
		case S_COLUMN_LIST:
		case S_WHERE:
		case S_COND_AND:
		case S_COND_OR:
		case S_UNION:
			tokenTypeRef = S_SUBQUERY_NESTED;
			break;
		case X_ROOT:
			tokenTypeRef = X_ROOT;
			break;
		default:
			tokenTypeRef = X_FAILURE;
		}
		break;
	case Tokens::T_COLUMN_LIST:
		tokenTypeRef = S_COLUMN_LIST;
		break;
	case Tokens::T_TABLE_CAST:
		tokenTypeRef = S_SUBQUERY_NESTED;
		break;
	case Tokens::T_FROM:
		tokenTypeRef = S_FROM;
		break;
	case Tokens::T_JOINING_CLAUSE:
		tokenTypeRef = L_JOINING_CLAUSE;
		break;
	case Tokens::T_WHERE:
		tokenTypeRef = S_WHERE;
		break;
	case Tokens::T_IDENTIFIER:
		tokenTypeRef = S_IDENTIFIER;
		break;
	case Tokens::T_OPERATOR_BINARY:
		tokenTypeRef = S_OPERATOR_BINARY;
		break;
	} // switch(tokentype)
	
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
	//using Tokens = Antlr3BackendImpl::OracleDMLTokens;
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
			if(
				(TokenType == Tokens::T_COND_OR_SEQ     ||
				 TokenType == Tokens::T_COND_AND_SEQ    ||
				 TokenType == Tokens::TABLE_REF_ELEMENT ||
				 TokenType == Tokens::TABLE_EXPRESSION  ||
				 TokenType == Tokens::DIRECT_MODE
					)
				&& childNode->getChildCount() == 1)
			{
				auto &grandChildNode = childNode->getChild(0);
				auto grandChildToken = grandChildNode->get_token();

				treeWalk(psr, root, grandChildNode, lastindex);
				continue;
			}

			Token *childTokenNew = new OracleDMLToken ( root, *childNode);
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

			Token *childTokenNew = new OracleDMLToken(root, *childNode);
			root->appendChild(childTokenNew);

			// Process spaces and comments after parser_token
			while(++lastindex < lexemeTotal)
			{
				auto &spacerToken = lexerTokenVector->at(lastindex);
				unsigned SpacerLexemeChannel = spacerToken.get_channel();

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
	default:
		break;
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
	default:
		break;
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
