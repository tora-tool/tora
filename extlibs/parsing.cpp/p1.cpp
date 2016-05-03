#include "utils.hpp"
#include "UserGuiTraits.hpp"
#include "PostgreSQLGuiLexer.hpp"

#include <sys/types.h>

#include <iostream>
#include <sstream>
#include <fstream>

using namespace Antlr3GuiImpl;
using namespace std;

static    PostgreSQLGuiLexer*		    lxr;

int main (int argc, char *argv[])
{
    // for this example, the input will always default to ./input if there is no explicit
    // argument, otherwise we are expecting potentially a whole list of 'em.
    //
	if (argc < 2 || argv[1] == NULL)
	{
		Utils::processDir("./input"); // Note in VS2005 debug, working directory must be configured
	}
	else
	{
		int i;

		for (i = 1; i < argc; i++)
		{
			Utils::processDir(argv[i]);
		}
	}

	printf("finished parsing OK\n");	// Finnish parking is pretty good - I think it is all the snow

	return 0;
}

void parseFile(const char* fName, int fd)
{
	PostgreSQLGuiLexerTraits::InputStreamType*    input;
	PostgreSQLGuiLexerTraits::TokenStreamType*	tstream;

#if defined __linux
	string data = Utils::slurp(fd);
#else
	string data = Utils::slurp(fName);
#endif
	input	= new PostgreSQLGuiLexerTraits::InputStreamType((const ANTLR_UINT8 *)data.c_str(),
						   ANTLR_ENC_8BIT,
						   data.length(), //strlen(data.c_str()),
						   (ANTLR_UINT8*)fName);

	input->setUcaseLA(true);
    
	if (lxr == NULL)
	{
		lxr = new PostgreSQLGuiLexer(input); 
	}
	else
	{
		lxr->setCharStream(input);
	}

	tstream = new PostgreSQLGuiLexerTraits::TokenStreamType(ANTLR_SIZE_HINT, lxr->get_tokSource());

	putc('L', stdout); fflush(stdout);
	{
		ANTLR_INT32 T = 0;
		while	(T != PostgreSQLGuiLexer::EOF_TOKEN)
		{
			T = tstream->_LA(1);
			printf("%d %s\n", T,  tstream->_LT(1)->getText().c_str());
			tstream->consume();
		}
	}

	tstream->_LT(1);

	delete tstream; 
	delete lxr; lxr = NULL;
	delete input; 
}
