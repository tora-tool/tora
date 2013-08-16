grammar SQLMini;

options {
	language=Cpp;
	backtrack=true;
	memoize=true;
	output=AST;
}

tokens 
{
    T_SUBQUERY='t_subquery';
    T_FOR_UPDATE_CLAUSE='t_for_update';

    T_TABLE_NAME='t_table_name';
    T_SCHEMA_NAME='t_schema_name';
    T_COLUMN_NAME='t_column_name';
}

@lexer::includes 
{
#include "UserMiniTraits.hpp"
}

@lexer::namespace 
{ Antlr3Mini }

@lexer::members {

  void advanceInput()
  {
    RecognizerSharedStateType *state = get_state();
    state->set_tokenStartCharIndex(getCharIndex());    
    state->set_tokenStartCharPositionInLine(getCharPositionInLine());
    state->set_tokenStartLine(getLine());
  }
}
        
@parser::includes {
#include "UserMiniTraits.hpp"
#include "SQLMiniLexer.hpp"
}

@parser::namespace 
{ Antlr3Mini }
        
@members {
            void perserMember();
}
        
start_rule
	: (
            subquery
        )            
        (SEMI|EOF)
        ;      

subquery
	:
        (	
//LPAREN select_statement RPAREN
//        ( group_by_clause )?
//        ( (k_model (k_main|k_partition|k_dimension) ) => model_clause )?
//        ( union_clause )*
        ( fu1=for_update_clause )?        
//        ( order_by_clause )?
        ( fu2=for_update_clause )?            
        ) -> ^(T_SUBQUERY $fu1 $fu2)
	|	(
		LPAREN subquery RPAREN
//        ( group_by_clause )?
//        ( (k_model (k_main|k_partition|k_dimension) ) => model_clause )?
//        ( union_clause )*
        ( fu1=for_update_clause )?        
//        ( order_by_clause )?
        ( fu2=for_update_clause )?                
        ) -> ^(T_SUBQUERY subquery $fu1 $fu2)
	;
identifier[int identifierClass, int usageType]
	:	i=ID                  //{ $i->user1 = identifierClass; $i->user2 = usageType; }
	;
table_name
	:	identifier[T_TABLE_NAME,0]
	;
schema_name
	:	identifier[T_SCHEMA_NAME,0]
	;
column_name
	:	identifier[T_COLUMN_NAME,0]
	;

/* ================================================================================
    FOR UPDATE CLAUSE
   ================================================================================ */
for_update_clause
	: r_for r_update ( r_of for_update_clause_part_first for_update_clause_part_next* )? (r_nowait | k_wait NUMBER | k_skip k_locked)?
	-> ^(T_FOR_UPDATE_CLAUSE r_for r_update r_of? for_update_clause_part_first? for_update_clause_part_next* r_nowait? k_wait? NUMBER? k_skip? k_locked?)
	;
for_update_clause_part_first
	: (schema_name DOT)? (table_name DOT)? column_name
	;
for_update_clause_part_next
	: COMMA (schema_name DOT)? (table_name DOT)? column_name
	;
	
r_for : r='FOR' ; //{ $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
r_nowait : r='NOWAIT' ; //{ $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
r_of : r='OF' ; //{ $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
r_update : r='UPDATE' ; //{ $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_locked : 'LOCKED'; //{ !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LOCKED")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_skip : 'SKIP' ; //{ !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SKIP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_wait : 'WAIT' ; //{ !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "WAIT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;

ID /*options { testLiterals=true; }*/
    :	'A' .. 'Z' ( 'A' .. 'Z' | '0' .. '9' | '_' | '$' | '#' )*
    ;
SEMI
	:	';'
	;
COMMA
	:	','
	;
LPAREN
	:	'('
	;
RPAREN
	:	')'
	;       
DOT
	:	POINT
	;
fragment
POINT
	:	'.'
	;
NUMBER
	: (	( NUM POINT NUM ) => NUM POINT NUM
		|	POINT NUM
		|	NUM
		)
    ;
fragment
NUM
	: '0' .. '9' ( '0' .. '9' )*
	;
WS	:	(' '|'\r'|'\t'|'\n') {$channel=HIDDEN;}
	;
SL_COMMENT
	:	'--' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
	;
ML_COMMENT
	:	'/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
	;
