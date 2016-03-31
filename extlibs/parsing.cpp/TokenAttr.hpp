#ifndef	_TOKEN_ATTR_H
#define	_TOKEN_ATTR_H

namespace SQLLexer
{
	namespace BlkCtx
	{

		// All these PLSQL constructs can also contain 'END';
		typedef enum
		{
			NONE      = 0, // the Token does not start any nested block
			CREATE    = 1,
			PACKAGE   = 2, // or TYPE or TRIGGER
			PROCEDURE = 3, // or FUNCTION
			DECLARE   = 4, // or IS or AS
			BEGIN     = 5,
			LOOP      = 6, // or CASE or IF
			END       = 7,
			PREPROC   = 8, // $if $$id $then ... $end
			MAX       = 9  // not used by tokens, just's marks enum max. value
		} BlockContextEnum;
	}
}
#endif
