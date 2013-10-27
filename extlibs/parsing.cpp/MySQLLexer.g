lexer grammar MySQLLexer;

options 
{
	language=Cpp;
}

tokens {
    BIND_VAR_WITH_PARAMS;
}

@lexer::includes 
{
#include "UserTraits.hpp"
#include <vector>
}

@lexer::namespace{ Antlr3BackendImpl }

// -----------------------------------------------------------------------------        
// numbers
fragment UNSIGNED_INTEGER: ('0'..'9')+ ;
fragment REAL_NUMBER:
	(  UNSIGNED_INTEGER DOT UNSIGNED_INTEGER | UNSIGNED_INTEGER DOT | DOT UNSIGNED_INTEGER )
	(  ('E'|'e') ( PLUS | MINUS )? UNSIGNED_INTEGER  )? 
;

fragment HEX_DIGIT_FRAGMENT: ( 'a'..'f' | 'A'..'F' | '0'..'9' ) ;
fragment HEX_DIGIT:
	(  '0x'     (HEX_DIGIT_FRAGMENT)+  )
	|
	(  'X' '\'' (HEX_DIGIT_FRAGMENT)+ '\''  ) 
;

fragment BIT_NUM:
	(  '0b'    ('0'|'1')+  )
	|
	( 'B\'' ('0'|'1')+ '\''  ) 
;

NUMBER_LIT: UNSIGNED_INTEGER | REAL_NUMBER | HEX_DIGIT | BIT_NUM;

// -----------------------------------------------------------------------------
// basic token definition 

DIVIDE                          : ('DIV') | '/' ;
MOD_SYM                         : ('MOD') | '%' ;
OR_SYM                          : ('OR')  | '||';
AND_SYM                         : ('AND') | '&&';

ARROW                           : '=>' ;
EQ_SYM                          : '=' | '<=>' ;
NOT_EQ                          : '<>' | '!=' | '~='| '^=';
LET                             : '<=' ;
GET                             : '>=' ;
SET_VAR                         : ':=' ;
SHIFT_LEFT                      : '<<' ;
SHIFT_RIGHT                     : '>>' ;
ALL_FIELDS                      : '.*' ;

SEMI                            : ';' ;
COLON                           : ':' ;
DOT                             : '.' ;
COMMA                           : ',' ;
ASTERISK                        : '*' ;
RPAREN                          : ')' ;
LPAREN                          : '(' ;
RBRACK                          : ']' ;
LBRACK                          : '[' ;
PLUS                            : '+' ;
MINUS                           : '-' ;
NEGATION                        : '~' ;
VERTBAR                         : '|' ;
BITAND                          : '&' ;
POWER_OP                        : '^' ;
GTH                             : '>' ;
LTH                             : '<' ;

// -----------------------------------------------------------------------------
// WHITE SPACE
fragment
SPACE_LIT
    :    ' '
    |    '\t'
    ;

fragment
NEWLINE
    :    '\r' (options{greedy=true;}: '\n')?
    |    '\n'
    ;

WHITE
	:	( SPACE_LIT | NEWLINE)+
	;

// -----------------------------------------------------------------------------
// http://dev.mysql.com/doc/refman/5.6/en/comments.html
COMMENT_SL
	: ('--'|'#') ( ~('\n'|'\r') )* (NEWLINE|EOF)
	;		
COMMENT_ML
	: '/*' ( options {greedy=false;} : . )* '*/'
	;

// -----------------------------------------------------------------------------
// basic const data definition ---------------------------------------------------------------
fragment ID
	:	( 'A'..'Z' | 'a'..'z' | '_' | '$') ( 'A'..'Z' | 'a'..'z' | '_' | '$' | '0'..'9' )*
	;

fragment BACKTICKED_ID
	:	(  '`' ( ('`' '`') | ~('`') )* '`'  )
	;

REGULAR_ID
	:	ID | BACKTICKED_ID
	;

fragment TEXT_STRING:
	( 'N' | ('_UTF8') )?
	(
		(  '\'' ( ('\\' '\\') | ('\'' '\'') | ('\\' '\'') | ~('\'') )* '\''  )
		|
		(  '\"' ( ('\\' '\\') | ('\"' '\"') | ('\\' '\"') | ~('\"') )* '\"'  ) 
	)
;

// http://dev.mysql.com/doc/refman/5.6/en/user-variables.html
USER_VAR:
	'@' (BACKTICKED_ID | USER_VAR_SUBFIX2 | USER_VAR_SUBFIX3 | USER_VAR_SUBFIX4)
;
//fragment USER_VAR_SUBFIX1:	(  '`' (~'`' )+ '`'  ) ;
fragment USER_VAR_SUBFIX2:	( '\'' (~'\'')+ '\'' ) ;
fragment USER_VAR_SUBFIX3:	( '\"' (~'\"')+ '\"' ) ;
fragment USER_VAR_SUBFIX4:	( 'A'..'Z' | 'a'..'z' | '_' | '$' | '0'..'9' | DOT )+ ;

STRING_LITERAL: TEXT_STRING /*| USER_VAR_SUBFIX2 | USER_VAR_SUBFIX3*/ | USER_VAR_SUBFIX4;

// -----------------------------------------------------------------------------
// Bind variables
BIND_VAR:
        '?'
        (
            LTH ID (LBRACK UNSIGNED_INTEGER RPAREN)? GTH { $type = BIND_VAR_WITH_PARAMS; }
        )?
;

// Last resort rule matches any character. This lexer should never fail.
// TOKEN_FAILURE : . ;
