
#include "parsing/PLSQLGuiLexer.hpp"
//#include "parsing/PLSQLLexer.hpp"
//#include "parsing/PLSQLParser.hpp"
#include "parsing/tsqllexer.h"

#include <QVector>

namespace SQLLexer
{

using namespace Antlr3GuiImpl;

class OracleGuiLexer : public Lexer
{
public:
	OracleGuiLexer(const QString &statement, const QString &name);
	virtual ~OracleGuiLexer();
	virtual QString firstWord();
	virtual QString wordAt(const Position &);
	virtual token_const_iterator findStartToken(token_const_iterator const &);
	virtual token_const_iterator findEndToken(token_const_iterator const &);
	virtual void setStatement(const char *s, unsigned len = -1);
	virtual void setStatement(const QString &s);

	typedef PLSQLGuiLexerTraits::CommonTokenType CommonTokenType;
	typedef SQLLexer::BlkCtx::BlockContextEnum BlockContextEnum;
	typedef SQLLexer::BlkCtx::BlockContextEnum B;

protected:
	virtual int size() const;                // Number of tokens - including the last EOF token
	virtual const Token& LA(int pos) const;  // Get token at pos - starting from 1st

private:
	void init();
	void clean();
	QByteArray QBAinput;
	QByteArray QBAname;

	Antlr3GuiImpl::PLSQLGuiLexerTraits::InputStreamType*    input;
	Antlr3GuiImpl::PLSQLGuiLexer *lxr;
	Antlr3GuiImpl::PLSQLGuiLexerTraits::TokenStreamType* tstream;
	
	unsigned lastLine, lastColumn, lastIndex;

	mutable Token retvalLA;
	
	// This template simply combines two values and return unique number to be used in switch/case
	// See fuzzy PL/SQL parser in findEndToken
	template<BlockContextEnum outer, BlockContextEnum inner> struct M
	{
		enum { value = outer * 10 + inner };
		static unsigned combine(BlockContextEnum out, BlockContextEnum in) { return out * 10 + in; }
	};	       
};

OracleGuiLexer::OracleGuiLexer(const QString &statement, const QString &name)
	: Lexer(statement, name)
	, QBAinput(statement.toUtf8())
	, QBAname(name.toUtf8())
	, lastLine(1)
	, lastColumn(0)
	, lastIndex(0)
{
	init();
}

OracleGuiLexer::~OracleGuiLexer()
{
	clean();
}

void OracleGuiLexer::init()
{
	input = new PLSQLGuiLexerTraits::InputStreamType(
    		(const ANTLR_UINT8 *)QBAinput.data(),
    		ANTLR_ENC_UTF8,
    		QBAinput.size(), //strlen(data.c_str()),
    		(ANTLR_UINT8*)QBAname.data());

	input->setUcaseLA(true); // ignore case

	// pinput = new User::PLSQLTraits::InputStreamType(
    	// 	(const ANTLR_UINT8 *)QBAinput.data(),
    	// 	ANTLR_ENC_UTF8,
    	// 	QBAinput.size(), //strlen(data.c_str()),
    	// 	(ANTLR_UINT8*)QBAname.data());

	// pinput->setUcaseLA(true); // ignore case

	if (input == NULL /*|| pinput == NULL*/ )
	{
		// TODO throw here
		throw Exception();
		exit(ANTLR_ERR_NOMEM);
	}

	lxr     = new PLSQLGuiLexer(input);
	//plxr    = new User::PLSQLLexer(pinput);

	if ( lxr == NULL /*|| plxr == NULL*/)
	{
		// TODO throw here
		throw Exception();
		exit(ANTLR_ERR_NOMEM);
	}

	tstream = new PLSQLGuiLexerTraits::TokenStreamType(ANTLR_SIZE_HINT, lxr->get_tokSource());
	//ptstream = new User::PLSQLTraits::TokenStreamType(ANTLR_SIZE_HINT, plxr->get_tokSource());

	if (tstream == NULL /*|| ptstream == NULL*/)
	{
		// TODO throw here
		//_mState = P_ERROR;
		throw Exception();
		exit(ANTLR_ERR_NOMEM);
	}
	//_mState = P_LEXER;
}

void OracleGuiLexer::setStatement(const char *s, unsigned len)
{
	clean();
	QBAinput.clear();
	QBAinput.append(s, len);
	lastLine = 1;
	lastColumn = 0;
	lastIndex = 0;
	init();
}

void OracleGuiLexer::setStatement(const QString &statement)
{
	clean();
	QBAinput.clear();
	QBAinput.append(statement.toUtf8());
	lastLine = 1;
	lastColumn = 0;
	lastIndex = 0;
	init();
}

void OracleGuiLexer::clean()
{
	if( tstream)
		delete tstream;
	if( lxr)
		delete lxr;
	if( input)
		delete input;
	tstream = NULL;
	lxr = NULL;
	input = NULL;
}

int OracleGuiLexer::size() const
{
	if(tstream)
		return tstream->getTokens()->size() + 1;
	else
		return 0;
}

const Token& OracleGuiLexer::LA(int pos) const
{
	if ( pos <= 0 || pos > size())
		throw Exception();

	if( pos == size())
	{
		Token::TokenType type = Token::X_EOF;
		retvalLA = Token(Position(0, 0), 0, PLSQLGuiLexer::EOF_TOKEN, type);
		retvalLA.setText("EOF");
		retvalLA.setBlockContext(NONE);
		return retvalLA;
	}

	CommonTokenType const* token = tstream->get(pos-1);
	if(token)
	{
		// ANTLR3 starts with 1st while QScintilla starts with 0th
		int line = token->get_line() - 1;
		int column = token->get_charPositionInLine();
		unsigned length = token->get_stopIndex() - token->get_startIndex() + 1;
		int offset = token->get_startIndex();
		Token::TokenType type = Token::X_UNASSIGNED;

		switch(token->getType())
		{
		case PLSQLGuiLexer::EOF_TOKEN: 
			type = Token::X_EOF;
			break;
		case PLSQLGuiLexer::BUILDIN_FUNCTIONS:
			type = Token::L_BUILDIN;
			break;
		case PLSQLGuiLexer::CHAR_STRING:
		case PLSQLGuiLexer::CHAR_STRING_PERL:
		case PLSQLGuiLexer::DELIMITED_ID:
		case PLSQLGuiLexer::NATIONAL_CHAR_STRING_LIT:
			type = Token::L_STRING;
			break;
		case PLSQLGuiLexer::COMMENT_ML:
//		case PLSQLGuiLexer::COMMENT_ML_PART:
			type = Token::X_COMMENT_ML;
			break;
//		case PLSQLGuiLexer::COMMENT_ML_END:
//			type = Token::X_COMMENT_ML_END;
//			break;
		case PLSQLGuiLexer::COMMENT_SL:
			type = Token::X_COMMENT;
			break;
		case PLSQLGuiLexer::TOKEN_FAILURE:
			type = Token::X_FAILURE;
			break;
		case PLSQLGuiLexer::PLSQL_RESERVED:
		case PLSQLGuiLexer::R_IF:
		case PLSQLGuiLexer::R_THEN:
		case PLSQLGuiLexer::R_LOOP:
		case PLSQLGuiLexer::R_CASE:
		case PLSQLGuiLexer::R_END:
			type = Token::L_RESERVED;
			break;
		case PLSQLGuiLexer::SELECT_COMMAND_INTRODUCER:
			type = Token::L_SELECT_INTRODUCER;
			break;
		case PLSQLGuiLexer::DML_COMMAND_INTRODUCER:
			type = Token::L_DML_INTRODUCER;
			break;
		case PLSQLGuiLexer::PLSQL_COMMAND_INTRODUCER:
			type = Token::L_PL_INTRODUCER;
			break;
		case PLSQLGuiLexer::OTHER_COMMAND_INTRODUCER:
			type = Token::L_OTHER_INTRODUCER;
			break;
		case PLSQLGuiLexer::SQLPLUS_COMMAND_INTRODUCER:
		case PLSQLGuiLexer::SQLPLUS_COMMAND:
		case PLSQLGuiLexer::SQLPLUS_SOLIDUS:			
			type = Token::X_ONE_LINE;
			break;		  
		case PLSQLGuiLexer::NEWLINE:
			type = Token::X_EOL;
			break;				
		case PLSQLGuiLexer::SPACE:
			type = Token::X_WHITE; 
			break;
		case PLSQLGuiLexer::LEFT_PAREN:
			type = Token::L_LPAREN;
			break;
		case PLSQLGuiLexer::RIGHT_PAREN:
			type = Token::L_RPAREN;
			break;
		case PLSQLGuiLexer::BINDVAR:
			type = Token::L_BIND_VAR;
			break;
		default:
			type = Token::X_UNASSIGNED;
			break;
		}

		retvalLA = Token(Position(line, column), length, token->getType(), type);
		retvalLA.setText(QString::fromUtf8(token->getText().c_str()));
		retvalLA.setBlockContext(token->getBlockContext());
		return retvalLA;
	} else
		throw Exception();
}

QString OracleGuiLexer::firstWord()
{
	CommonTokenType const* token = tstream->_LT(1);
	if( token)
	{
		return QString((const char*)(token->getText().c_str()));
	}
	return QString();
}

QString OracleGuiLexer::wordAt(const Position &pos)
{
	unsigned line = pos.getLine();
	unsigned column = pos.getLinePos();

    line++; // ANTLR3 starts with 1st while QScintilla starts with 0th

    QString retval;
    //ANTLR_UINT32 i, startIndex;
    //ANTLR_UINT32 size = tstream->getTokens()->size();
    //User::MyTraits::CommonTokenType const* token = NULL;

    //if(size == 0)
    //    return retval;

    //if( lastLine > line || (lastLine == line && lastColumn > column))
    //{
    //	User::MyTraits::CommonTokenType const* tokenZero = (pANTLR3_COMMON_TOKEN)lexerTokenVector->get(lexerTokenVector, 0);
    //	retval = QString::fromUtf8((const char*)(tokenZero->getText()->chars));
    //	startIndex = 1;
    //} else {
    //	User::MyTraits::CommonTokenType const* tokenZero = (pANTLR3_COMMON_TOKEN)lexerTokenVector->get(lexerTokenVector, lastIndex);
    //	retval = QString::fromUtf8((const char*)(tokenZero->getText()->chars));
    //	startIndex = lastIndex;
    //}

    //for (i = startIndex; i <= size; i++)
    //{
    //    token = (pANTLR3_COMMON_TOKEN)lexerTokenVector->get(lexerTokenVector, i);
    //    if( token == NULL)
    //    	break;

    //    if ( token->getChannel(token) != HIDDEN)
    //    {
    //        lastIndex  = i;
    //        lastLine   = token->getLine(token);
    //        lastColumn = token->getCharPositionInLine(token);
    //    	retval = QString::fromUtf8((const char*)(token->getText(token)->chars));
    //    }

    //    if( token->getLine(token) > line
    //    		|| ( token->getLine(token) == line && token->getCharPositionInLine(token) > column ))
    //    	break;	
    //}

    return retval;
}

Lexer::token_const_iterator OracleGuiLexer::findStartToken( Lexer::token_const_iterator const &start)
{
	// These Token types can start new statement
	static const QSet<SQLLexer::Token::TokenType> INTRODUCERS = QSet<SQLLexer::Token::TokenType>()
		<< SQLLexer::Token::L_SELECT_INTRODUCER
		<< SQLLexer::Token::L_DML_INTRODUCER
		<< SQLLexer::Token::L_PL_INTRODUCER
		<< SQLLexer::Token::L_OTHER_INTRODUCER
		<< SQLLexer::Token::L_LPAREN
		<< SQLLexer::Token::X_ONE_LINE
		;
	token_const_iterator i(start);	
	while(i->getTokenType() != SQLLexer::Token::X_EOF)
	{
		i = i.consumeUntil(INTRODUCERS);
		if(i->getTokenType() != SQLLexer::Token::L_LPAREN)
			return i;                      // Return this Token - it is not LPAREN '('  

		bool exitLoop = false;
		// LPAREN starts a new SQL query if and only if it's followed by SELECT
		// create a duplicit iterator and iterate it until something interesting is found
		{
			token_const_iterator j(i);
			while(j->getTokenType() != SQLLexer::Token::X_EOF && !exitLoop) 
			{
				j++;
				switch(j->getTokenType())
				{
				case SQLLexer::Token::X_WHITE:
				case SQLLexer::Token::X_COMMENT:
				case SQLLexer::Token::X_COMMENT_ML:
				case SQLLexer::Token::X_COMMENT_ML_END:
				case SQLLexer::Token::X_EOL:
				case SQLLexer::Token::L_LPAREN: // SQL QUERY can start with multiple LPARENs '((( SELECT ...'
					continue;         // nothing interesting was found, try the next Token
				case SQLLexer::Token::L_SELECT_INTRODUCER:
					if(j->getText().compare("SELECT", Qt::CaseInsensitive) == 0 )
						return i; // SELECT was found => return position of LPAREN
					return j;     // otherwise return position of the SELECT/WITH Token
				case SQLLexer::Token::L_DML_INTRODUCER:
					return j;         // otherwise return position of the DML Token
				default:
					exitLoop = true;  // SELECT was not found => exit the loop;
				}
			}
			i = j; // Advance position of the outer loop iterator - continue looping
		}
	}
	return i; // NOTE: this will only return EOF
}

Lexer::token_const_iterator OracleGuiLexer::findEndToken( Lexer::token_const_iterator const &start)
{
	token_const_iterator i(start);
	switch( start->getTokenType())
	{
	case Token::L_PL_INTRODUCER:
	{
		// Every nested block pushes it's context onto stack
		// Stack's top value can be also adjused, but the value can only be increased (CREATE => PACKAGE => DECLARE => BEGIN => END)
		QVector<BlockContextEnum> stack; 
		bool exitLoop = false;
		
		BlockContextEnum tokenContext = start->getBlockContext();
		
		if( tokenContext == BlkCtx::NONE)
			throw Exception(); // Assertion error. 1st Token must have block context
		stack << tokenContext;     // Push the 1st value onto stack
		
		QString prevText, currText = i->getText();
		while(!exitLoop)
		{
			i++;

			tokenContext = i->getBlockContext(); // context for next Token
			BlockContextEnum &stackContext = stack.back(); // non-const reference the current stack's top
			
			switch(i->getTokenType())
			{
			// These Tokens must not be present in PL/SQL CODE
			case Token::L_OTHER_INTRODUCER: // ALTER, EXPLAIN
			case Token::X_ONE_LINE:         // SPOOL, EXIT, DEFINE, ...
			case Token::X_EOF:
			case Token::X_FAILURE:          // Char not understood be the Lexer
				exitLoop = true;
				continue;
			case Token::L_SELECT_INTRODUCER:   // SELECT, WITH
			case Token::L_DML_INTRODUCER:   // INSERT, UPDATE, DELETE, ...
				// TODO TRIGGER abc BEFORE INSERT/UPDATE/DELETE is handled wrongly
				// 
				// DML keyword can be only present within BEGIN or LOOP context
				if( stackContext == BlkCtx::PACKAGE || stackContext == BlkCtx::PROCEDURE)
				{
					exitLoop = true;
					continue;
				}
				prevText = currText;
				currText = i->getText();
				break;
			case Token::L_RESERVED:               // Reserved word
			case Token::L_IDENTIFIER:
			case Token::L_BUILDIN:                // Built-in function name
			case Token::L_STRING:                 // single/double quoted string
			case Token::L_LPAREN:                 // '('
			case Token::L_RPAREN:                 // ')'
			case Token::L_PL_INTRODUCER:          // DECLARE, BEGIN, CREATE, PACKAGE, PROCEDURE
			case Token::X_UNASSIGNED:
				// prevText contains previous Token's text (except for WHITE SPACE and comments)
				prevText = currText;
				currText = i->getText();
				break;
			case Token::X_EOL:
			case Token::X_WHITE:                  // "white" char space, tab, newline
			case Token::X_COMMENT:                // single line comment, preferably without newline part
			case Token::X_COMMENT_ML:             // Multi-line comment (the correct one)
			case Token::X_COMMENT_ML_END:         // Partially edited multi-line comment, usually ends with EOF
				break;
			}

			if( tokenContext == BlkCtx::NONE)
				continue;

			// "TYPE" is not reserved word - beside being an OBJECT(type) it can also be a column name or part of the "%TYPE" declaration
			// NOTE: TODO "CREATE" TYPE does not end with "END TYPE;" (but "CREATE TYPE BODY " does)
			if ( currText.toUpper() == "TYPE" && stackContext != CREATE)
				continue;                                                       // this is not type body declaration

			if ( currText.toUpper() == "IF" && prevText.toUpper() == "END")                   // "END IF;" IF does not start a new block
				continue;

			if ( currText.toUpper() == "LOOP" && prevText.toUpper() == "END")                  // "END LOOP;" LOOP does not start a new block
				continue;

			if ( currText.toUpper() == "CASE" && prevText.toUpper() == "END")                  // "END CASE;" CASE does not start a new block
				continue;

			// Combine two enumerated values. The current Stack's top
			// and the current Token's context. The resulting value will be used in switch/case.
			// BlkCtx has 7 possible values, theoretically we should cover up to 49 possible combinations.
			//
			// - The value on the stack's top can only be increased, as token iterator moves forward
			//   CREATE -> PACKAGE -> DECLARE -> BEGIN -> END
			//   LOOP -> END
			// - In other cases we push new value onto stack ( like BLOCK inside BLOCK )
			// - All other combinations are considered illegal - they stop parsing (like LOOP inside DECLARE )
			unsigned var = M<NONE,NONE>::combine(stackContext, tokenContext);
			switch(var)
			{
			case M< CREATE,    CREATE >::value :
				if (prevText != "(")           // CREATE inside CREATE (CREATE TABLE can constain (CREATE INDEX ...) statement)
					exitLoop = true;           // The nested one always follows L_PAREN "("
				continue;

			case M< PACKAGE,   CREATE >::value :
			case M< PROCEDURE, CREATE >::value :
			case M< DECLARE,   CREATE >::value :
			case M< BEGIN,     CREATE >::value :
			case M< LOOP,      CREATE >::value :
				exitLoop = true;				// CREATE inside anything => failure, exit loop
				continue;

			case M< CREATE,    PACKAGE >::value :
				stackContext = PACKAGE;                         // PACKAGE inside CREATE (adjust value on the stack's top)
				continue;
				
			case M<CREATE,     PROCEDURE >::value :
				stackContext = PROCEDURE;                       // PROCEDURE inside CREATE (adjust value on the stack's top)
				continue;				
			case M<PACKAGE,    PROCEDURE >::value :
				stack << PROCEDURE;                             // PROCEDURE inside PACKAGE (push PROCEDURE)
				continue;
			case M<PROCEDURE,  PROCEDURE >::value :
				continue;                                       // PROCEDURE inside PROCEDURE => (One Procedure forward decl follows another one. Do nothing)
			case M<DECLARE,    PROCEDURE >::value :
				stack << PROCEDURE;				// PROCEDURE inside DECLARE/IS/AS (push nested PROCEDURE)
				continue;

			case M<PACKAGE,    DECLARE >::value :
			case M<PROCEDURE,  DECLARE >::value :
				                                                // DECLARE/IS/AS inside PACKAGE/PROCEDURE (adjust stack's top)
				if( i->getText().compare("IS", Qt::CaseInsensitive) == 0 ||
				    i->getText().compare("AS", Qt::CaseInsensitive) == 0 )
					stackContext = DECLARE;
				else
					exitLoop = true;                        // "DECLARE" inside PACKAGE/PROCEDURE => failure, exit loop
				continue;				
			case M<DECLARE,    DECLARE >::value :				                                                
				if( i->getText().compare("IS", Qt::CaseInsensitive) == 0 ||
				    i->getText().compare("AS", Qt::CaseInsensitive) == 0)
					continue;                               // IS/AS   inside DECLARE => CURSOR X IS SELECT * FROM T AS OF TIMESTAMP Y;
				exitLoop = true;                                // DECLARE inside DECLARE => failure exit loop
				continue;
			case M<BEGIN,     DECLARE >::value :
			case M<LOOP,      DECLARE >::value :
				// IS is also part of "IS NULL", "IS A SET"     // DECLARE/IS/AS inside BEGIN/LOOP (push DECLARE)
				// AS is also part of pure SQL statemets "AS OF TIMESTAMP ..."
				// DECLARE itself starts a new block
				if( i->getText().compare("DECLARE", Qt::CaseInsensitive) == 0 )
					stack << DECLARE;
				continue;

			case M<PACKAGE,   BEGIN >::value :
			case M<DECLARE,   BEGIN >::value :				
				stackContext = BEGIN;                           // BEGIN inside PACKAGE (adjust stack's top) (PACKAGE init block)
				continue;                                       // BEGIN inside DECLARE (adjust stack's top)

			case M<BEGIN,     BEGIN >::value :
			case M<LOOP,      BEGIN >::value :
				stack << BEGIN;                                // BEGIN inside DECLARE/BEGIN/LOOP (push BEGIN)
				continue;
				
			case M<DECLARE,   LOOP >::value :
			case M<BEGIN,     LOOP >::value :
			case M<LOOP,      LOOP >::value :
				stack << LOOP;                                 // CASE inside DECLARE (push LOOP) (CASE can return variable's init value)
				continue;                                      // LOOP/CASE inside BEGIN/LOOP (push LOOP) (CASE can return variables init value)

			case M<BEGIN,     END >::value :
			{                                                      // END inside BEGIN (pop stack)
				BlockContextEnum ctx = stackContext; stack.pop_back();
				if( stack.isEmpty() )
					exitLoop = true;
				// TODO find SEMICOLON (or label_name/package_name SEMICOLON)
				// i = i.consumeUntil(PLSQLGuiLexer::SEMICOLON);
				continue;
			}
			case M<LOOP,      END >::value :
			{                                                      // END inside LOOP/CASE/IF (pop stack)
				BlockContextEnum ctx = stackContext; stack.pop_back();
				if( stack.isEmpty() )
					exitLoop = true;
				// TODO find LOOP/CASE/IF SEMICOLON
				// i = i.consumeUntil(PLSQLGuiLexer::SEMICOLON);
				continue;
			}

			default:
				exitLoop = true;                                // PACKAGE inside anything(but CREATE) => failure exit loop
				continue;

			}
		}
		break;
	}
	/**
	 * Pseudo parser for: INSERT, SELECT, WITH, UPDATE, DELETE, MERGE, COMMIT, ROLLBACK, EXPLAIN statement
	 * - some of those words(SELECT, UPDATE, DELETE, WITH, SET) can also be inside other statement
	 * - SELECT can only follow LEFT_PAREN, RIGHT_PAREN or UNION (ALL), MINUS, INTERSECT, otherwise it starts a new statement
	 * - in case of MERGE statement DELETE can follow anything, SELECT/UPDATE follow 'THEN'
	 * - MERGE can not be in the middle of any statement
	 * - UPDATE can be part of FOR UPDATE OF statement
	 * - SELECT statement can start with LPAREN ('(') i.e " (SELECT * FROM DUAL ) UNION ALL (SELECT * FROM DUAL) "
	 * - statement can end with: SOLIDUS('/'), SEMICOLON(';') or other introducer(SELECT, UPDATE, ...)
	 */
	case Token::L_SELECT_INTRODUCER:
	case Token::L_DML_INTRODUCER:
	case Token::L_LPAREN:
	{
		bool parenFound = start->getTokenType() == Token::L_LPAREN;
		bool mergeFound = start->getText().compare("MERGE", Qt::CaseInsensitive) == 0;
		bool explainFound = start->getText().compare("EXPLAIN", Qt::CaseInsensitive) == 0;
		bool forFound = false;		
		while(i->getOrigTokenType() != PLSQLGuiLexer::EOF_TOKEN)
		{
			i++;
			QString txt = i->getText();
			switch( i->getOrigTokenType())
			{
			case PLSQLGuiLexer::LEFT_PAREN:
			case PLSQLGuiLexer::RIGHT_PAREN:
				parenFound = true;
				break;
			case PLSQLGuiLexer::SELECT_COMMAND_INTRODUCER:
			case PLSQLGuiLexer::DML_COMMAND_INTRODUCER:
				if( forFound && i->getText().compare("UPDATE", Qt::CaseInsensitive) == 0) // FOR UPDATE continue parsing
					break;
				else if( !parenFound && !mergeFound)                                      // No PARENS no MERGE but 'SELECT' was found
					goto exitLoop;
				else if( i->getText().compare("MERGE", Qt::CaseInsensitive) == 0)         // New MERGE statement begins
					goto exitLoop;
				break;
			case PLSQLGuiLexer::PLSQL_RESERVED:
				if( i->getText().compare("FOR", Qt::CaseInsensitive) == 0)
					forFound = true;
				if( i->getText().compare("UNION", Qt::CaseInsensitive) == 0
				    ||	i->getText().compare("INTERSECT", Qt::CaseInsensitive) == 0
				    ||	i->getText().compare("MINUS", Qt::CaseInsensitive) == 0)
					parenFound = true;
				else if( parenFound && i->getText().compare("ALL", Qt::CaseInsensitive) == 0)
					parenFound = true;
				else
					parenFound = false;
				break;
			case PLSQLGuiLexer::NEWLINE: // white chars do not clear parenFound
			case PLSQLGuiLexer::SPACE:
			case PLSQLGuiLexer::COMMENT_ML:
			case PLSQLGuiLexer::COMMENT_SL:
				break;
			case PLSQLGuiLexer::SQLPLUS_COMMAND:
			case PLSQLGuiLexer::SQLPLUS_COMMAND_INTRODUCER:
			case PLSQLGuiLexer::SQLPLUS_SOLIDUS:
			case PLSQLGuiLexer::SEMICOLON:
				i--;
				goto exitLoop;
			case PLSQLGuiLexer::OTHER_COMMAND_INTRODUCER: // ALTER, GRANT, TRUNCATE, SET TRANCACTION, ...
				// EXPLAIN PLAN SET STATEMENT - do not end this stmt
				//if(explainFound && i->getText().compare("SET", Qt::CaseInsensitive) == 0)
				//	break;
			case PLSQLGuiLexer::PLSQL_COMMAND_INTRODUCER: // DECLARE, BEGIN, CREATE, SET, ...
				// SET starts SQLPLUS command(SET LINE), also starts OTHER sql command(SET ROLE) 
				// but also can be present in UPDATE statement
				if(i->getText().compare("SET", Qt::CaseInsensitive) == 0)
					break;
			case PLSQLGuiLexer::EOF_TOKEN:
			case CommonTokenType::TOKEN_INVALID:
				goto exitLoop;
			default:                     // any other word clear parenFound
				parenFound = false;
			}
		} // while()
		exitLoop:;
		break;
	}
	/**
	 * Pseudo parser for: ALTER, ANALYZE, TRUNCATE, FLASHBACK, ... statement
	 * - GRANT statement can contain SELECT, UPDATE, DELETE, INSERT
	 * - statement can end with: SOLIDUS('/'), SEMICOLON(';') or other introducer(SELECT, UPDATE, ...)
	 */
	case Token::L_OTHER_INTRODUCER:
	{
		bool grantFound = start->getText().compare("GRANT", Qt::CaseInsensitive) == 0;
		while(true)
		{
			i++;
			switch( i->getOrigTokenType())
			{
			case PLSQLGuiLexer::EOF_TOKEN:
			case PLSQLGuiLexer::SEMICOLON:
			case PLSQLGuiLexer::SQLPLUS_SOLIDUS:
				goto exitLoopOtherIntroducer;
			}
		}
		exitLoopOtherIntroducer:;
		break;
	}
	// Handle OneLiners (SQLPLUS commands)
	case Token::X_ONE_LINE:
	{
		i = i.consumeUntil(PLSQLGuiLexer::NEWLINE);
		break;
	}
	default:
		throw Exception();
	}

	if( i == start) // If the statement contains only one token advance forward. (Never return the same token)
		i++;
	return i;
}

};

Util::RegisterInFactory<SQLLexer::OracleGuiLexer, LexerFactTwoParmSing> regOracleSQLLexStatement2("OracleGuiLexer");
