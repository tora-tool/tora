#include "utils.hpp"
#include "UserTraits.hpp"
#include "OracleDMLLexer.hpp"
#include "OracleDML.hpp"
#include "OracleDML_OracleDMLKeys.hpp"
#include "OracleDML_OracleDMLCommons.hpp"

#include <sys/types.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>

using namespace Antlr3BackendImpl;
using namespace std;

static OracleDMLLexer* lxr;

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
	delete lxr; lxr = NULL;
	return 0;
}

bool parseFile(const char* fName, int fd)
{
	using InputStream = Antlr3BackendImpl::OracleSQLParserTraits::InputStreamType;
	using TokenStream = Antlr3BackendImpl::OracleSQLParserTraits::TokenStreamType;
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
	// Finally, now that we have our lexer constructed, we can create the parser
	auto psr = make_unique<OracleDML>(tstream.get());
	auto ast = psr->seq_of_statements();
	if (ast.tree.get() == NULL)
	{
		string failed("failed/");
		std::ofstream out(failed + fName);
		out << data;
		out.close();
		return false;
	}

	std::cout << ast.tree->toStringTree() << std::endl;
	std::cout << "Root:" << ast.tree.get() << std::endl;
	return true;
}
