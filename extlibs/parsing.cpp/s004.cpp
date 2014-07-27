#include "utils.hpp"
#include "s004Traits.hpp"
#include "PLSQLLexer.hpp"
#include "PLSQLParser.hpp"
#include "PLSQLParser_PLSQLCommons.hpp"
#include "PLSQLParser_PLSQL_DMLParser.hpp"
#include "PLSQLParser_PLSQLKeys.hpp"
#include "PLSQLParser_SQLPLUSParser.hpp"

#include <sys/types.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <stdio.h>
#include <time.h>

using namespace Antlr3Test;
using namespace std;

static    PLSQLLexer*		    lxr;

// Main entry point for this example
//
int 
main	(int argc, char *argv[])
{
	if (argc < 2 || argv[1] == NULL)
	{
		Utils::processDir("./s004.input"); // Note in VS2005 debug, working directory must be configured
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
	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );
	std::cout<<ctime(&rawtime)<<": parsing started" << std::endl;

	///byIvan input	= new PLSQLTraits::InputStreamType(fName, ANTLR_ENC_8BIT);
#if defined __linux
	string data = Utils::slurp(fd);
#else
	string data = Utils::slurp(fName);
#endif
	auto input = new PLSQLTraits::InputStreamType((const ANTLR_UINT8 *)data.c_str(),
						      ANTLR_ENC_8BIT,
						      data.length(), //strlen(data.c_str()),
						      (ANTLR_UINT8*)fName);
	input->setUcaseLA(true);
    
	// Our input stream is now open and all set to go, so we can create a new instance of our
	// lexer and set the lexer input to our input stream:
	//  (file | memory | ?) --> inputstream -> lexer --> tokenstream --> parser ( --> treeparser )?
	//
	if (lxr == NULL)
	{
		lxr	    = new PLSQLLexer(input);
	}
	else
	{
		lxr->setCharStream(input);
	}

	auto tstream = new PLSQLTraits::TokenStreamType(ANTLR_SIZE_HINT, lxr->get_tokSource());
	auto psr = new PLSQLParser(tstream);

	//putc('L', stdout); fflush(stdout);
	//{
	//	ANTLR3_INT32 T;
    
	//	T = 0;
	//	while	(T != ANTLR3_TOKEN_EOF)
	//	{
	//		T = tstream->tstream->istream->_LA(tstream->tstream->istream, 1);
	//		tstream->tstream->istream->consume(tstream->tstream->istream);
	//		printf("%d %s\n", T,  (psr->pParser->rec->state->tokenNames)[T]);
	//	}
	//}

	//tstream->_LT(1);	// Don't do this mormally, just causes lexer to run for timings here
	//putc('P', stdout); fflush(stdout);
	{
	  auto ast = psr->seq_of_statements();
	  std::cout << ast.getTree()->toStringTree();
	}
	//putc('F', stdout); fflush(stdout);
	//putc('*', stdout); fflush(stdout);

	//ANTLR_BITWORD FOLLOW_truncate_key_in_synpred47_PLSQLParser6931_bits[]	= 
	//	{ 
	//		ANTLR_UINT64_LIT(0xFFFFFFFFFFFFFFF0), 
	//		ANTLR_UINT64_LIT(0xFFFFFFFFFFFFDFFF), 
	//		ANTLR_UINT64_LIT(0xFFFFFFFFFFFFFFFF), 
	//		ANTLR_UINT64_LIT(0xFFFFFFFFFFFFFFFF), 
	//		ANTLR_UINT64_LIT(0x0000000000001FFF) 
	//};
	//PLSQLParserImplTraits::BitsetListType FOLLOW_truncate_key_in_synpred47_PLSQLParser6931( FOLLOW_truncate_key_in_synpred47_PLSQLParser6931_bits, 5 );

	//PLSQLParserImplTraits::BitsetType *d = FOLLOW_truncate_key_in_synpred47_PLSQLParser6931.bitsetLoad();
	//PLSQLParserImplTraits::BitsetType *c = d->clone();

	//ANTLR_BITWORD d0 = d->get_blist().get_bits()[0];
	//ANTLR_BITWORD c0 = c->get_blist().get_bits()[0];

	//ANTLR_BITWORD d1 = d->get_blist().get_bits()[1];
	//ANTLR_BITWORD c1 = c->get_blist().get_bits()[1];

	//ANTLR_BITWORD d2 = d->get_blist().get_bits()[2];
	//ANTLR_BITWORD c2 = c->get_blist().get_bits()[2];

	//ANTLR_BITWORD d3 = d->get_blist().get_bits()[3];
	//ANTLR_BITWORD c3 = c->get_blist().get_bits()[3];

	//ANTLR_BITWORD d4 = d->get_blist().get_bits()[4];
	//ANTLR_BITWORD c4 = c->get_blist().get_bits()[4];

	//ANTLR_BITWORD d5 = d->get_blist().get_bits()[5];
	//ANTLR_BITWORD c5 = c->get_blist().get_bits()[5];

	time ( &rawtime );
	std::cout<<ctime(&rawtime)<<": parsing finished" << std::endl;

	delete psr; 
	delete tstream; 
	delete lxr; lxr = NULL;
	delete input; 
}
