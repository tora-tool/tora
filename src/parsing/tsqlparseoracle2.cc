
#include "OracleDMLTraits.hpp"
#include "OracleDMLLexer.hpp"
#include "OracleDML.hpp"
#include "OracleDML_OracleDMLKeys.hpp"
#include "OracleDML_OracleDMLCommons.hpp"

#include "parsing/tsqlparse.h"
#include "core/tologger.h"

#include <QtCore/QPair>
#include <QtCore/QtAlgorithms>

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
	: Token(parent, Position(node.get_token()->get_line(), node.get_token()->get_charPositionInLine()), node.getText().c_str())
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
	case Tokens::SELECT_STATEMENT:
	case Tokens::T_SELECT:
		switch(parent->getTokenType())
		{
#if 1
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
#endif
		case X_ROOT:
			tokenTypeRef = X_ROOT;
			break;
		default:
			tokenTypeRef = X_FAILURE;
		}
		break;
        case Tokens::NESTED_SUBQUERY:
            tokenTypeRef = S_SUBQUERY_NESTED;
            break;
	case Tokens::EQUALS_OP:
	case Tokens::NOT_EQUAL_OP:
	case Tokens::GREATER_THAN_OP:
	case Tokens::GREATER_THAN_OR_EQUALS_OP:
	case Tokens::LESS_THAN_OP:
	case Tokens::LESS_THAN_OR_EQUALS_OP:
	//case Tokens::SQL92_RESERVED_LIKE:
	case Tokens::LIKEC_VK:
	case Tokens::LIKE2_VK:
	case Tokens::LIKE4_VK:
	case Tokens::NOT_LIKE:
	case Tokens::NOT_IN:
	    tokenTypeRef = S_OPERATOR_BINARY;
	    break;
	case Tokens::T_WHERE:
	case Tokens::SQL92_RESERVED_WHERE:
	    tokenTypeRef = S_WHERE;
	    break;
	case Tokens::SQL92_RESERVED_ALL:
	case Tokens::SQL92_RESERVED_ALTER:
	//case Tokens::SQL92_RESERVED_AND:
	case Tokens::SQL92_RESERVED_ANY:
	case Tokens::SQL92_RESERVED_AS:
	case Tokens::SQL92_RESERVED_ASC:
	case Tokens::SQL92_RESERVED_BEGIN:
	case Tokens::SQL92_RESERVED_BETWEEN:
	case Tokens::SQL92_RESERVED_BY:
	case Tokens::SQL92_RESERVED_CASE:
	case Tokens::SQL92_RESERVED_CHECK:
	case Tokens::SQL92_RESERVED_CONNECT:
	case Tokens::SQL92_RESERVED_CREATE:
	case Tokens::SQL92_RESERVED_CURRENT:
	case Tokens::SQL92_RESERVED_CURSOR:
	case Tokens::SQL92_RESERVED_DATE:
	case Tokens::SQL92_RESERVED_DECLARE:
	case Tokens::SQL92_RESERVED_DEFAULT:
	case Tokens::SQL92_RESERVED_DELETE:
	case Tokens::SQL92_RESERVED_DESC:
	case Tokens::SQL92_RESERVED_DISTINCT:
	case Tokens::SQL92_RESERVED_DROP:
	case Tokens::SQL92_RESERVED_ELSE:
	case Tokens::SQL92_RESERVED_END:
	case Tokens::SQL92_RESERVED_EXCEPTION:
	case Tokens::SQL92_RESERVED_EXISTS:
	case Tokens::SQL92_RESERVED_FALSE:
	case Tokens::SQL92_RESERVED_FETCH:
	case Tokens::SQL92_RESERVED_FOR:
	case Tokens::SQL92_RESERVED_FROM:
	case Tokens::SQL92_RESERVED_GOTO:
	case Tokens::SQL92_RESERVED_GRANT:
	case Tokens::SQL92_RESERVED_GROUP:
	case Tokens::SQL92_RESERVED_HAVING:
	case Tokens::SQL92_RESERVED_IN:
	case Tokens::SQL92_RESERVED_INSERT:
	//case Tokens::SQL92_RESERVED_INTERSECT:
	case Tokens::SQL92_RESERVED_INTO:
	case Tokens::SQL92_RESERVED_IS:
	case Tokens::SQL92_RESERVED_LIKE:
	case Tokens::SQL92_RESERVED_NOT:
	case Tokens::SQL92_RESERVED_NULL:
	case Tokens::SQL92_RESERVED_OF:
	case Tokens::SQL92_RESERVED_ON:
	case Tokens::SQL92_RESERVED_OPTION:
	//case Tokens::SQL92_RESERVED_OR:
	case Tokens::SQL92_RESERVED_ORDER:
	case Tokens::SQL92_RESERVED_OVERLAPS:
	case Tokens::SQL92_RESERVED_PRIOR:
	case Tokens::SQL92_RESERVED_PROCEDURE:
	case Tokens::SQL92_RESERVED_PUBLIC:
	case Tokens::SQL92_RESERVED_REVOKE:
	case Tokens::SQL92_RESERVED_SELECT:
	case Tokens::SQL92_RESERVED_SIZE:
	case Tokens::SQL92_RESERVED_TABLE:
	case Tokens::SQL92_RESERVED_THE:
	case Tokens::SQL92_RESERVED_THEN:
	case Tokens::SQL92_RESERVED_TO:
	case Tokens::SQL92_RESERVED_TRUE:
	//case Tokens::SQL92_RESERVED_UNION:
	case Tokens::SQL92_RESERVED_UNIQUE:
	case Tokens::SQL92_RESERVED_UPDATE:
	case Tokens::SQL92_RESERVED_VALUES:
	case Tokens::SQL92_RESERVED_VIEW:
	case Tokens::SQL92_RESERVED_WHEN:
	case Tokens::SQL92_RESERVED_WITH:
	case Tokens::T_FROM:
	case Tokens::T_RESERVED:
		tokenTypeRef = L_RESERVED;
		break;
	case Tokens::TABLEVIEW_NAME:
		tokenTypeRef = S_TABLE_REF;
		break;
	case Tokens::T_TABLE_NAME:
		tokenTypeRef = L_TABLENAME;
		break;
	case Tokens::T_TABLE_ALIAS:
	case Tokens::ALIAS:
	{
		// Resolve grammar ambiguity: SELECT * FROM A INNER JOIN B; (=> INNER is not a table alias)
		// The same for NATURAL JOIN, CROSS JOIN, LEFT/RIGHT OUTER JOIN
		QString str = QString::fromStdString(node.getText()).toUpper();
		//cout << "Tokens::T_TABLE_ALIAS:" << qPrintable(str) << endl;
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
		case S_SUBQUERY_FACTORED:
			tokenTypeRef = L_SUBQUERY_ALIAS;
			break;
		default:
			tokenTypeRef = L_TABLEALIAS;
			break;
		}
	}
	break;
#if 0
	case Tokens::T_SCHEMA_NAME:
		tokenTypeRef = L_SCHEMANAME;
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
#endif
	case Tokens::FACTORING:
		tokenTypeRef = S_SUBQUERY_FACTORED;
		break;			
	case Tokens::SUBQUERY:
		break;
	case Tokens::T_COLUMN_LIST:
		tokenTypeRef = S_COLUMN_LIST;
		break;
	case Tokens::T_IDENTIFIER:
	case Tokens::ANY_ELEMENT: // todo use  -> ^(ANY_ELEMENT[ToraType(T_ITENTIFIER)] char_set_name? id_expression+)
		tokenTypeRef = S_IDENTIFIER;
		break;
	case Tokens::T_JOINING_CLAUSE:
	case Tokens::JOIN_DEF:
		tokenTypeRef = L_JOINING_CLAUSE;
		break;
#if 0
	case Tokens::T_TABLE_CAST:
		tokenTypeRef = S_SUBQUERY_NESTED;
		break;
	case Tokens::T_FROM:
		tokenTypeRef = S_FROM;
		break;
	case Tokens::T_WHERE:
		tokenTypeRef = S_WHERE;
		break;
	case Tokens::T_OPERATOR_BINARY:
		tokenTypeRef = S_OPERATOR_BINARY;
		break;
#endif
	case Tokens::SQL92_RESERVED_UNION:
		tokenTypeRef = S_UNION;
		break;
	case Tokens::SQL92_RESERVED_AND:
	    tokenTypeRef = S_COND_AND;
	    break;
	case Tokens::SQL92_RESERVED_OR:
	    tokenTypeRef = S_COND_OR;
	    break;
	case Tokens::PLSQL_RESERVED_MINUS:
		tokenTypeRef = S_MINUS;
		break;
	case Tokens::SQL92_RESERVED_INTERSECT:
		tokenTypeRef = S_INTERSECT;
		break;
	case Tokens::EOF_TOKEN:
		tokenTypeRef = X_EOF;
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
    void treeWalkAST(unique_ptr<Antlr3BackendImpl::OracleDML> &psr, QPointer<Token> root, Traits::TreeTypePtr& tree);
    QList<Token*> treeWalkToken(QPointer<Token> root);

    /* Walk through Token tree and look for table names, table aliases, ... and try to resolve them
       Note: this function also replaces some instances of Token* with Token's subclass instances
       */
    void disambiguate();

public:
    /* Walk through Token tree and look for table names, table aliases, ... and try to resolve them using cached database catalog*/
    void scanTree(ObjectCache *, QString const&) override;

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

namespace todocxx14
{
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args) {
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}
 
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
	auto input = todocxx14::make_unique<InputStream>((const ANTLR_UINT8 *)QBAinput.data(), antlr3::ENC_8BIT, QBAinput.length(), (ANTLR_UINT8*)QBAname.data());
	if (input == NULL)
		throw ParseException();
	input->setUcaseLA(true); // ignore case

	auto lxr = todocxx14::make_unique<Antlr3BackendImpl::OracleDMLLexer>(input.get());
	if ( lxr == NULL )
		throw ParseException();

	auto tstream = todocxx14::make_unique<TokenStream>(ANTLR_SIZE_HINT, lxr->get_tokSource());
	if (tstream == NULL)
	{
		throw ParseException();
	}

	lexerTokenVector = tstream->getTokens();
	_mState = P_LEXER;

	// Finally, now that we have our lexer constructed, we can create the parser
	auto psr = todocxx14::make_unique<Antlr3BackendImpl::OracleDML>(tstream.get());
	if (psr == NULL)
		throw ParseException();

	auto langAST = psr->seq_of_statements();
	if (psr->getNumberOfSyntaxErrors())
	{
		// TODO throw reasonable exception HERE
		throw ParseException();
	}
	Antlr3BackendImpl::OracleSQLParserTraits::TreeAdaptorType adaptor;
	this->dot = QString::fromStdString(adaptor.makeDot(langAST.tree));

	_mState = P_PARSER;
	
	_mAST = new TokenSubquery( NULL
				   , Position(0, 0)
				   , ""
				   , Token::X_ROOT
		);
	_mAST->setTokenATypeName("ROOT");


	// Copy ANTLR AST into Token tree structure
	treeWalkAST(psr, _mAST, langAST.tree);

	// Get sorted list of Token tree structure leaves
	QList<Token*> _mLeaves = treeWalkToken(_mAST);
	qSort(_mLeaves.begin(), _mLeaves.end(),
	      [](Token* a, Token* b)
	      {
	            return a->getPosition() < b->getPosition();
	      });


	QList<Token*>::iterator i = _mLeaves.begin(), j = _mLeaves.begin(); j++;
	unsigned lastIndex = 0;
	Token *t1, *t2, *t3; // t1 left leaf, t2 right leaf, t3 chosen leaf

	/* loop over lexer's tokens */
	while (lastIndex < lexerTokenVector->size())
	{
		auto &spacerToken = lexerTokenVector->at(lastIndex++);
		if (spacerToken.consumed())
			continue;

		Position spacerPosition(spacerToken.get_line(), spacerToken.get_charPositionInLine());

		t1 = *i;
		t2 = j == _mLeaves.end() ? NULL : *j;
CHECK:
		if (t2 == NULL)
		{
			t3 = t1;
		}
		else if (spacerPosition < t1->getPosition())
		{
			t3 = t1;
		}
		else if (t1->getPosition() == spacerPosition)
		{
		    continue;
		}
		else if (t1->getPosition() < spacerPosition && spacerPosition < t2->getPosition())
		{
			t3 = t1->depth() > t2->depth() ? t1 : t2;
		}
		else if (spacerPosition == t2->getPosition())
		{
		    continue;
		}
		else // t2->getPosition < spacerPosition
		{
			do
			{
				i++; j++;
				t1 = *i;
				t2 = j == _mLeaves.end() ? NULL : *j;
			} while (t2 && t2->getPosition() < spacerPosition);
			if (t2 == NULL)
			    t3 = t1;
			else
			    goto CHECK;
		}

		Token *spacerTokenNew = new Token(t3
			, spacerPosition
			, spacerToken.getText().c_str()
			, Token::X_COMMENT
		);
		const_cast<Traits::CommonTokenType&>(spacerToken).setConsumed();

		t3->addSpacer(spacerTokenNew);

		t3 = NULL;
	}

	lexerTokenVector->clear();
};

/* recursively copy an AST tree into */
void OracleDMLStatement::treeWalkAST(unique_ptr<Antlr3BackendImpl::OracleDML> &psr, QPointer<Token> root, Traits::TreeTypePtr &tree)
{
	using LexerTokens = Antlr3BackendImpl::OracleDMLLexerTokens;
	auto &children = tree->get_children();
	auto ns = tree->toString();
	for (auto i = children.begin(); i != children.end(); ++i)
	{
		Traits::TreeTypePtr &childNode(*i);
		auto childToken = childNode->get_token();
		auto s = childToken->toString();
		auto c = childNode->getChildCount();

		//auto &spacerTokenX = lexerTokenVector->at(lastindex);

		// if child is not a leaf node - recurse
		if (!childNode->get_children().empty())
		{
			// skip useless T_COND_OR_SEQ T_COND_AND_SEQ having only one son
			LexerTokens::Tokens tokenType = (LexerTokens::Tokens)childNode->getType();
			if( //false &&
				(tokenType == LexerTokens::T_COND_OR_SEQ     ||
				 tokenType == LexerTokens::T_COND_AND_SEQ    ||
				 tokenType == LexerTokens::TABLE_REF         ||
				 tokenType == LexerTokens::TABLE_EXPRESSION  ||
				 tokenType == LexerTokens::ALIAS             ||
				 tokenType == LexerTokens::QUERY_NAME        || // used only by subquery factoring
				 tokenType == LexerTokens::CASCATED_ELEMENT  ||
				 tokenType == LexerTokens::SUBQUERY          ||
				 tokenType == LexerTokens::DIRECT_MODE
					)
				&& childNode->getChildCount() == 1)
			{
				auto &grandChildNode = childNode->getChild(0);
				auto grandChildToken = grandChildNode->get_token();

				treeWalkAST(psr, root, childNode);
				continue;
			}

			//if ( tokenType == LexerTokens::SQL92_RESERVED_AND && root->getTokenType() == Token::S_COND_AND)
			//{
			//    treeWalkAST(psr, root, childNode);
			//    continue;
			//}
			if ( tokenType == LexerTokens::SQL92_RESERVED_OR && root->getTokenType() == Token::S_COND_OR)
			{
			    treeWalkAST(psr, root, childNode);
			    continue;
			}

			Token *childTokenNew = new OracleDMLToken(root, *childNode);
			root->appendChild(childTokenNew);
			// This token "select", "from", "where", "=" is already consumed. Do not prepend it to other tokens
			if (childToken->isRealToken())
			{
				// The token might be cloned by the grammar. lexerTokenVector and AST might point onto different instances
				const_cast<Traits::CommonTokenType*>(childToken)->setConsumed();
				auto &localToken = lexerTokenVector->at(childToken->get_tokenIndex());
				const_cast<Traits::CommonTokenType&>(localToken).setConsumed();
			}
			treeWalkAST(psr, childTokenNew, childNode);
		}
		else     // if child is a leaf node
		{
			/* this is a leaf node */
			Token *childTokenNew = new OracleDMLToken(root, *childNode);
			root->appendChild(childTokenNew);
		} // else for child is a leaf node
	} // for each child
};

QList<Token*> OracleDMLStatement::treeWalkToken(QPointer<Token> root)
{
    QList<Token*> tokens;
    foreach(QPointer<Token> child, root->getChildren())
    {
        tokens.append(treeWalkToken(child));
    }

    if (root->getPosition().isValid())
        tokens.append(root);
    return tokens;
}

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
            //if( node.getTokenUsageType() != Token::Declaration )
            //    break;

            //loop over left brothers until you find either a reserved word or a table name
            QList<QPointer<Token> > const& brothers = node.parent()->getChildren();
            std::cout << "Alias found:" << node.toString().toLatin1().constData() << std::endl;
            for( int j = node.row() - 1 ; j >= 0; --j)
            {
                Token *brother = brothers.at(j);
                if( brother->getTokenType() == Token::S_SUBQUERY_NESTED)
                {
                    std::cout << node.toString().toStdString() << "=>" << brother->toStringRecursive().toStdString() << std::endl;

                    //_mDeclarations.insertMulti(node.toString(), brother.data());
                    for(SQLParser::Statement::token_const_iterator_to_root k(&*i); &*k; ++k)
                    {
                     std::cout << k->toString().toStdString() << "->";
                    }
                    std::cout << std::endl;

                    static_cast<TokenSubquery*>(brother)->setNodeAlias(&node);
                    addTranslation(node.toString(), brother, node.parent());
                    break;
                }
                if( brother->getTokenType() == Token::S_TABLE_REF)
                {
                    std::cout << node.toString().toLatin1().constData() << "=>" << (*brother).toStringRecursive().toLatin1().constData() << std::endl;

                    //_mDeclarations.insertMulti(node.toString(), brother.data());
                    for(SQLParser::Statement::token_const_iterator_to_root k(&*i); &*k; ++k)
                    {
                     std::cout << k->toString().toStdString() << "->";
                    }
                    std::cout << std::endl;

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
            //loop over right brothers until you find S_SUBQUERY_FACTORED
            QList<QPointer<Token> > const& brothers = node.parent()->getChildren();
            for( int j = node.row() + 1 ; j < node.parent()->childCount() ; ++j)
            {
                Token *brother = brothers.at(j);

                if( brother->getTokenType() == Token::S_SUBQUERY_FACTORED)
                {
                    std::cout << node.toString().toLatin1().constData() << "=>" << brother->toStringRecursive().toStdString() << std::endl;

                    //_mDeclarations.insertMulti(node.toString(), brother.data());

                    static_cast<TokenSubquery*>(brother)->setNodeAlias(&node);
                    addTranslation(node.toString(), brother, node.parent());

                    std::cout << "Subquery alias found:" << node.toString().toStdString() << "->" << brother->toString().toStdString() << std::endl;
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
        Token const& token = *i;
        if( i->getTokenType() == Token::S_IDENTIFIER )
        {
            QString stopToken;
            bool insideColumnList = false;

            // Do not resolve identifiers under S_COLUMN_LIST
            for(token_iterator_to_root k(i->parent()); k->parent(); ++k)
            {
                stopToken = k->toString();
                if( k->getTokenType() == Token::S_COLUMN_LIST)
                {
                    TLOG(0, toNoDecorator, __HERE__) << " Dont Resolve identifier: " << i->toStringRecursive(false).toStdString() << "\t under: " <<  stopToken.toStdString() << std::endl;
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
            TLOG(8, toNoDecorator, __HERE__) << "Resolve identifier: " << id.toStringRecursive(true).toStdString() << "\t under: " <<  stopToken.toStdString() << std::endl;

            QString schemaName, tableName, columnName;

            switch(id.childCount()) // odd children should be dots.
            {
            case 0:
            case 4:
                //TLOG(8, toNoDecorator, __HERE__) << " invalid length:" << id.childCount() << std::endl;
                break;
            case 2:
            {
                // T1.C1
                tableName = id.child(0)->toStringRecursive(false).toUpper();
                columnName = id.child(1)->toStringRecursive(false).toUpper();
                Token const *translatedAlias = translateAlias(tableName, &(*i));
                if( translatedAlias)
                {
                    TLOG(8, toNoDecorator, __HERE__) << "Translation: " << tableName.toStdString() << "\t=>\t: " <<  translatedAlias->toStringRecursive().toStdString() << std::endl;
                }
            }
            break;
            case 1:
                // We have column name only
                break;
#if 0
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
                TLOG(8, toNoDecorator, __HERE__) << " identifier found " << cs << '.' << tableName << '.' << columnName << ':' << columnExists << std::endl;
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
                TLOG(8, toNoDecorator, __HERE__) << " identifier found: " << schemaName << '.' << tableName << '.' << columnName << ':' << columnExists << std::endl;
            }
            break;
#endif
            default:
                //TLOG(8, toNoDecorator, __HERE__) << " unsupported length:" << id.childCount() << std::endl;
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
        std::cout << "tr: " << k->toString().toStdString() << std::endl;
        if( k->getTokenType() == Token::S_SUBQUERY_NESTED ||
                k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
                k->getTokenType() == Token::X_ROOT)
        {
            TokenSubquery &s = static_cast<TokenSubquery&>(*k);
            s.aliasTranslation().insert(alias.toUpper(), const_cast<Token*>(tableOrSubquery));
            std::cout << "Alias translation added:" << alias.toStdString() << "=>" << tableOrSubquery->toStringRecursive(false).toStdString() << " context: " << k->toString().toStdString() << std::endl;
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
        //std::cout << "tr: " << k->toString().toStdString() << '(' << k->getTokenType() << ')' << std::endl;
        if( k->getTokenType() == Token::S_SUBQUERY_NESTED ||
                k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
                k->getTokenType() == Token::X_ROOT)
        {
            TokenSubquery &s = static_cast<TokenSubquery&>(*k);
            s.nodeTables().insertMulti(tableRef->toStringRecursive(false).toUpper(), const_cast<Token*>(tableRef));
            //std::cout << "Table ref added:" << tableRef->toStringRecursive(true).toStdString() << " context: " << k->toString().toStdString() << std::endl;
            break;
        }
    }
};

}; // namespace SQLParser

Util::RegisterInFactory<SQLParser::OracleDMLStatement, StatementFactTwoParmSing> regOracleDMLStatement("OracleDML");
