#include "utils.hpp"
#include "OracleDMLTraits.hpp"
#include "OracleDMLLexer.hpp"
#include "OracleDML.hpp"
#include "OracleDML_OracleDMLKeys.hpp"
#include "OracleDML_OracleDMLCommons.hpp"

#include <sys/types.h>
#if defined __linux
#include <libgen.h>
#endif

#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <iomanip>
 
using namespace Antlr3BackendImpl;
using namespace std;

void treeWalk(OracleSQLParserTraits::TreeTypePtr const&, unsigned depth = 0);

static OracleDMLLexer* lxr;
static set<string> tables, aliases;
int 
main	(int argc, char *argv[])
{
	if (argc < 2 || argv[1] == NULL)
	{
		Utils::processDir("t5.input"); // Note in VS2005 debug, working directory must be configured
	}
	else
	{
		int i;

		for (i = 1; i < argc; i++)
		{
			Utils::processDir(argv[i]);
		}
	}

	printf("finished parsing OK\n");
	std::cout << "sizeof(TreeType)" << sizeof(Antlr3BackendImpl::OracleSQLParserTraits::TreeType) << std::endl;
	std::cout << "sizeof(TokenType):" << sizeof(Antlr3BackendImpl::OracleSQLParserTraits::CommonTokenType) << std::endl;
	delete lxr; lxr = NULL;
	return 0;
}

bool parseFile(const char* fName, int fd)
{
	using InputStream = Antlr3BackendImpl::OracleSQLParserTraits::InputStreamType;
	using TokenStream = Antlr3BackendImpl::OracleSQLParserTraits::TokenStreamType;
	using Token       = Antlr3BackendImpl::OracleSQLParserTraits::CommonTokenType;
	using Tokens      = Antlr3BackendImpl::OracleDMLTokens;
	
	string data = Utils::slurp(fd);
	auto input = make_unique<InputStream>((const ANTLR_UINT8 *)data.c_str(),
					      antlr3::ENC_8BIT,
					      data.length(),
					      (ANTLR_UINT8*)fName);
	input->setUcaseLA(true);
	if (lxr == NULL)
		lxr = new OracleDMLLexer(input.get());
	else
		lxr->setCharStream(input.get());

	auto tstream = make_unique<TokenStream>(ANTLR_SIZE_HINT, lxr->get_tokSource());

	Token const *token = tstream->LT(1);
	unsigned type = token->getType();
	// if ( type != SQL92_RESERVED_WITH &&
	//      type != SQL92_RESERVED_SELECT &&
	//      type != SQL92_RESERVED_INSERT &&
	//      type != SQL92_RESERVED_UPDATE &&
	//      type != SQL92_RESERVED_DELETE
	// 	)
	if ( type == Tokens::SQL92_RESERVED_BEGIN ||
	     type == Tokens::SQL92_RESERVED_DECLARE ||
	     toUpper(token->getText()) == "CALL" ||
	     type == Tokens::SQL92_RESERVED_CREATE ||
	     type == Tokens::SQL92_RESERVED_ALTER ||
	     type == Tokens::SQL92_RESERVED_DROP ||
	     type == Tokens::SQL92_RESERVED_GRANT ||
	     toUpper(token->getText()) == "TRUNCATE" ||
	     toUpper(token->getText()) == "COMMIT" ||
	     toUpper(token->getText()) == "ROLLBACK" ||
	     toUpper(token->getText()) == "SAVEPOINT" ||
	     toUpper(token->getText()) == "ANALYZE" ||
	     toUpper(token->getText()) == "SET"
		)
	{
		static string failed("plsql/");
		char *filename = basename((char*)fName);
		std::ofstream out(failed + filename);
		out << data;
		out.close();
		return false;		
	}
	
	// Finally, now that we have our lexer constructed, we can create the parser
	auto psr = make_unique<OracleDML>(tstream.get());
	auto ast = psr->seq_of_statements();
	if (psr->getNumberOfSyntaxErrors())
	{
		static string failed("failed/");
		char *filename = basename((char*)fName);
		std::ofstream out(failed + filename);
		out << data;
		out.close();
		return false;
	}

	//std::cout << ast.tree->toStringTree() << std::endl;
	//std::cout << "Root:" << ast.tree.get() << std::endl;
	tables.clear(); aliases.clear();		
	treeWalk(ast.tree);
	return true;
}

void treeWalk(OracleSQLParserTraits::TreeTypePtr const& root, unsigned depth)
{
	using Tokens = Antlr3BackendImpl::OracleDMLTokens;
	OracleSQLParserTraits::StringType tokenString, leafIndicator(" "), tokenType;
	auto &children = root->get_children();
	auto token = root->get_token();
	int toraTokenType = 0, usageType = 0;
	
	if (root->getType() < (numeric_limits<ANTLR_UINT32>::max)())
		tokenType = (const char*)OracleDML::getTokenNames()[root->getType()];
	
	if (children.empty())
	{
		leafIndicator = "*";
	}

	/* Need a new string with nothing at all in it. (current ANTLR3 does not generate nil nodes)	   
	 */
	if (root->isNilNode() == true)
	{
		leafIndicator = "#";
	}

	if (token)
	{
		toraTokenType = token->UserData.toraTokenType;
		usageType = token->UserData.usageType;
	}
		  
	cout << setw(4 * depth + 3) << leafIndicator << tokenType << '[' << root->getType() << ',' << toraTokenType << ',' << usageType << ']';
	if ( tokenType != root->toString())
		cout << '/' << root->toString();
	cout << endl;
	
	if (!children.empty())
	{
		treeWalk(children.front(), depth+1);
		for (auto i = std::next(children.begin()); i != children.end(); ++i)
		{
			treeWalk(*i, depth+1);
		}
	}

	if (token && token->UserData.toraTokenType == Tokens::T_SCHEMA_NAME)
		cout << "Schema name:\t" << token->getText() << endl;	
	if (token && token->UserData.toraTokenType == Tokens::T_TABLE_NAME)
		cout << "Table name:\t" << token->getText() << endl;
	if (token && token->UserData.toraTokenType == Tokens::T_COLUMN_NAME)
		cout << "Column name:\t" << token->getText() << endl;
	if (token && token->UserData.toraTokenType == Tokens::T_TABLE_ALIAS /*&& token->UserData.usageType == Tokens::T_DECL*/)
		cout << "Table alias:\t" << token->getText() << endl;
}
