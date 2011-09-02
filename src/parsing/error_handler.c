#include "error_handler.h"

void myDisplayRecognitionLexerError (pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames)
{};

void myDisplayRecognitionParserError (pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames)
{};

void * myRecoverFromMismatchedToken  (pANTLR3_BASE_RECOGNIZER recognizer, ANTLR3_UINT32 ttype, pANTLR3_BITSET_LIST follow);

void * myGetMissingSymbol (pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_INT_STREAM istream, pANTLR3_EXCEPTION e,
			   ANTLR3_UINT32 expectedTokenType, pANTLR3_BITSET_LIST follow);

void * myRecoverFromMismatchedToken  (pANTLR3_BASE_RECOGNIZER recognizer, ANTLR3_UINT32 ttype, pANTLR3_BITSET_LIST follow)
{
	pANTLR3_PARSER			  parser;
	pANTLR3_TREE_PARSER	      tparser;
	pANTLR3_INT_STREAM	      is;
	void					* matchedSymbol;

	switch	(recognizer->type)
	{
	case	ANTLR3_TYPE_PARSER:

		parser  = (pANTLR3_PARSER) (recognizer->super);
		tparser	= NULL;
		is	= parser->tstream->istream;

		break;

	case	ANTLR3_TYPE_TREE_PARSER:

		tparser = (pANTLR3_TREE_PARSER) (recognizer->super);
		parser	= NULL;
		is	= tparser->ctnstream->tnstream->istream;

		break;

	default:

		ANTLR3_FPRINTF(stderr, "Base recognizer function recoverFromMismatchedToken called by unknown parser type - provide override for this function\n");
		return NULL;

		break;
	}

	// Create an exception if we need one
	//
	if	(recognizer->state->exception == NULL)
	{
		antlr3RecognitionExceptionNew(recognizer);
	}

	// If the next token after the one we are looking at in the input stream
	// is what we are looking for then we remove the one we have discovered
	// from the stream by consuming it, then consume this next one along too as
	// if nothing had happened.
	//
	if	( recognizer->mismatchIsUnwantedToken(recognizer, is, ttype) == ANTLR3_TRUE)
	{
		recognizer->state->exception->type		= ANTLR3_UNWANTED_TOKEN_EXCEPTION;
		recognizer->state->exception->message	= ANTLR3_UNWANTED_TOKEN_EXCEPTION_NAME;

		// Call resync hook (for debuggers and so on)
		//
		if	(recognizer->debugger != NULL)
		{
			recognizer->debugger->beginResync(recognizer->debugger);
		}

		// "delete" the extra token
		//
		recognizer->beginResync(recognizer);
		is->consume(is);
		recognizer->endResync(recognizer);
		// End resync hook 
		//
		if	(recognizer->debugger != NULL)
		{
			recognizer->debugger->endResync(recognizer->debugger);
		}

		// Print out the error after we consume so that ANTLRWorks sees the
		// token in the exception.
		//
		recognizer->reportError(recognizer);

		// Return the token we are actually matching
		//
		matchedSymbol = recognizer->getCurrentInputSymbol(recognizer, is);

		// Consume the token that the rule actually expected to get as if everything
		// was hunky dory.
		//
		is->consume(is);

		recognizer->state->error  = ANTLR3_FALSE;	// Exception is not outstanding any more

		return	matchedSymbol;
	}

	// Single token deletion (Unwanted above) did not work
	// so we see if we can insert a token instead by calculating which
	// token would be missing
	//
	if	(recognizer->mismatchIsMissingToken(recognizer, is, follow))
	{
		// We can fake the missing token and proceed
		//
		matchedSymbol = myGetMissingSymbol(recognizer, is, recognizer->state->exception, ttype, follow);
		//matchedSymbol = recognizer->getMissingSymbol(recognizer, is, recognizer->state->exception, ttype, follow);
		recognizer->state->exception->type		= ANTLR3_MISSING_TOKEN_EXCEPTION;
		recognizer->state->exception->message	= ANTLR3_MISSING_TOKEN_EXCEPTION_NAME;
		recognizer->state->exception->token		= matchedSymbol;
		recognizer->state->exception->expecting	= ttype;

		// Print out the error after we insert so that ANTLRWorks sees the
		// token in the exception.
		//
		recognizer->reportError(recognizer);

		recognizer->state->error  = ANTLR3_FALSE;	// Exception is not outstanding any more

		return	matchedSymbol;
	}

	recognizer->state->error	    = ANTLR3_TRUE;
	return NULL;
};


void * myGetMissingSymbol (pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_INT_STREAM istream, pANTLR3_EXCEPTION e,
			   ANTLR3_UINT32 expectedTokenType, pANTLR3_BITSET_LIST follow)
{
	pANTLR3_TOKEN_STREAM			ts;
	pANTLR3_COMMON_TOKEN_STREAM		cts;
	pANTLR3_COMMON_TOKEN			token;
	pANTLR3_COMMON_TOKEN			current;
	pANTLR3_STRING					text;

	// Dereference the standard pointers
	//
	ts		= (pANTLR3_TOKEN_STREAM)istream->super;
	cts		= (pANTLR3_COMMON_TOKEN_STREAM)ts->super;
	
	// Work out what to use as the current symbol to make a line and offset etc
	// If we are at EOF, we use the token before EOF
	//
	current	= ts->_LT(ts, 1);
	if	(current->getType(current) == ANTLR3_TOKEN_EOF)
	{
		current = ts->_LT(ts, -1);
	}

	// Create a new empty token
	//
	if	(recognizer->state->tokFactory == NULL)
	{
		// We don't yet have a token factory for making tokens
		// we just need a fake one using the input stream of the current
		// token.
		//
		recognizer->state->tokFactory = antlr3TokenFactoryNew(current->input);
	}
	token	= recognizer->state->tokFactory->newToken(recognizer->state->tokFactory);

	// Set some of the token properties based on the current token
	//
	token->setLine					(token, current->getLine(current));
	token->setCharPositionInLine	(token, current->getCharPositionInLine(current));
	token->setChannel				(token, ANTLR3_TOKEN_DEFAULT_CHANNEL);
	token->setType					(token, expectedTokenType);
	token->user1                    = current->user1;
	token->user2                    = current->user2;
	token->user3                    = current->user3;
	token->custom                   = current->custom;
	token->lineStart                = current->lineStart;
    
	// Create the token text that shows it has been inserted
	//
	token->setText8(token, (pANTLR3_UINT8)"<missing ");
	text = token->getText(token);

	if	(text != NULL)
	{
		text->append8(text,
			      expectedTokenType == EOF ? (const char *)"EOF" :
			      (const char *)recognizer->state->tokenNames[expectedTokenType]
			);
		text->append8(text, (const char *)">");
	}
	
	// Finally return the pointer to our new token
	//
	return	token;
};
