/*
Oracle PL/SQL grammar built with ANTLR v3.2 and v3.1.3. I only vouch that it works for v3.2, though.

Author: Patrick Higgins
License: GNU Lesser General Public License, version 2.1 or (at your option) any later version.

I have used a swallow_to_semi trick to avoid parsing SQL statements and other statements that were not of value to me.
The idea was that a separate parser for SQL would be created (I believe this is what Oracle itself does).

Nearly all of the PL/SQL language from 11g is in this grammar, though. It works on all files in a fairly large code
base.

This has some limited support for parsing SQL*Plus files, which turns out to be pretty hard to work into ANTLR.

It works for my usage, but I think doing it properly would mean writing a Java class to parse the SQL*Plus language
(which is pretty simple and shouldn't need ANTLR) and another adapter for ANTLR which enables tracking back to the
original source line numbers. This PL/SQL parser might be invoked many times for each SQL*Plus file.
*/

grammar OraclePLSQL;

options {
    language=Cpp;
    backtrack=true;
	memoize=true;
    output=AST;
    //TokenLabelType=CommonTokenTypeCTT;
}

tokens {
    APPROXIMATE_NUM_LIT = 'approximate_num_lit';
    DOUBLE_PERIOD = 'double_period';

    T_UNKNOWN = 't_unknown';
    T_RESERVED = 't_reserved';
    // Identifier subtypes AST leafs 
    T_SCHEMA_NAME = 't_schema_name';
    T_TRIGGER_NAME = 't_trigger_name';
    T_PACKAGE_NAME = 't_package_name';
    T_FUNCTION_NAME = 't_function_name';
    T_PROCEDURE_NAME = 't_procedure_name';
    T_PARAMETER_NAME = 't_parameter_name';
    T_PACKAGE_NAME = 't_package_name';
    T_BINDVAR_NAME = 't_bindvar_name';
    T_VARIABLE_NAME = 't_variable_name';
    T_DATATYPE_NAME = 't_datatype_name';
    T_CURSOR_NAME = 't_cursor_name';
    T_TABLE_NAME = 't_table_name';
    T_COLUMN_NAME = 't_column_name';
    T_OBJECT_NAME = 't_object_name';
    T_LABEL_NAME = 't_label_name';
    T_EXCEPTION_NAME = 't_exception_name';
    T_DBLINK_NAME = 't_dblink_name';

    //Alias type(declaration, usage)
    T_DECL = 't_decl';
    T_USE  = 't_use';
    T_USEL = 't_usel'; // 'L' for lvalue, the variable is beeing assigned/modified here

    //declaration types
    T_TYPE_DECL      = 't_type_decl';
    T_SUBTYPE_DECL   = 't_subtype_decl';
    T_CURSOR_DECL    = 't_cursor_decl';
    T_VAR_DECL       = 't_variable_decl';
    T_CONST_DECL     = 't_const_decl';
    T_EXC_DECL       = 't_exc_decl';
    
    T_FUNCTION_DECL  = 't_function_decl';
    T_PROCEDURE_DECL = 't_procedure_decl';
    T_PRAGMA_DECL    = 't_pragma_decl';
        
    //block types
    T_DECL_BLOCK='t_decl_block';    
    T_BLOCK_BLOCK='t_block_block';
    T_PROCEDURE_BLOCK='t_procedure_block';
    T_FUNCTION_BLOCK='t_function_block';
    T_DECLARE_BLOCK='t_declare_block';
    T_BEGIN_BLOCK='t_begin_block';
    T_EXCEPTIONS_BLOCK='t_exceptions_block';

    //SQL statements
    T_COMMIT = 't_commit';
    T_DELETE = 't_delete';
    T_INSERT = 't_insert';
    T_LOCK_TABLE = 't_lock_table';
    T_MERGE = 't_merge';
    T_ROLLBACK = 't_rollback';
    T_SAVEPOINT = 't_savepoint';
    T_SELECT = 't_select';
    T_SET_TRANSACTION = 't_set_transaction';
    T_UPDATE = 't_update';

    //Other statements
    T_EXECUTE_IMMEDIATE = 't_execute_immediate';
    T_CASE  = 't_case' ;
    T_OPEN  = 't_open' ;
    T_CLOSE = 't_close' ;
    T_CONTINUE = 't_continue' ;
    T_PIPE = 't_pipe' ;
    T_GOTO = 't_goto' ;
    T_EXIT = 't_exit' ;
    T_RETURN  = 't_return' ;
    T_DECLARE = 't_declare' ;
    T_RAISE = 't_raise' ;
    T_FETCH = 't_fetch' ;
    T_LOOP  = 't_loop' ;
    T_WHILE = 't_while' ;
    T_NULL_STAT  = 't_null' ;
    T_FOR   = 't_for' ;
    T_FORALL = 't_forall' ;
    //block constens - statements
    T_IF='t_if';
    T_ELSIF='t_elsif';
    T_ELSE='t_else';
    T_ENDIF='t_endif';
    
    T_ASSIGN = 't_assign';
    T_CALL = 't_call';
        
    T_DECLARATION = 't_declaration';
    T_STATEMENT = 't_statement';
}

@lexer::includes 
{
#include "UserGuiTraits.hpp"
#include <vector>
}

@lexer::namespace 
{ Antlr3Impl }

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
#include "UserGuiTraits.hpp"
#include "OraclePLSQLLexer.hpp"
#include <vector>
}

@parser::namespace 
{ Antlr3Impl }
        
@members {
            void perserMember();
}

// @members {

// public String getErrorHeader(RecognitionException e) {
//     return getSourceName()+":"+e.line+":"+(e.charPositionInLine+1)+":";
// }

// public String getErrorMessage(RecognitionException e, String[] tokenNames) {
//     List stack = getRuleInvocationStack(e, this.getClass().getName());
//     String msg = null;
//     if (e instanceof NoViableAltException) {
//         NoViableAltException nvae = (NoViableAltException) e;
//         msg = " no viable alt; token="+e.token+
//               " (decision="+nvae.decisionNumber+
//               " state "+nvae.stateNumber+")"+
//               " decision=<<"+nvae.grammarDecisionDescription+">>";
//     }
//     else {
//         msg = super.getErrorMessage(e, tokenNames);
//     }
//     return stack+" "+msg;
// }
    
// public String getTokenErrorDisplay(Token t) {
//     return t.toString();
// }

// }

start_rule
    : sqlplus_file | plsql_block
    ;

sqlplus_file
    : ( create_object ( DIVIDE show_errors )? DIVIDE? )+ //TODO EOF
    ;
    
show_errors
    : kSHOW kERRORS SEMI?
    ;

create_object
    : (k_create? k_or? kREPLACE? k_trigger) => create_trigger
    | (k_create? k_or? kREPLACE? k_function) => create_function
    | (k_create? k_or? kREPLACE? k_procedure) => create_procedure
    | (k_create? k_or? kREPLACE? k_type) => create_type
    | (k_create? k_or? kREPLACE? k_package k_body) => create_package_body
    | create_package create_package_body? //TODO delete the second one
    ;    

procedure_heading :
        k_procedure procedure_name[T_DECL] parameter_declarations?
    ;

function_heading :
        k_function identifier[T_FUNCTION_NAME,T_DECL] parameter_declarations? k_return datatype
    ;

parameter_declarations :
        (   LPAREN  parameter_declaration ( COMMA  parameter_declaration )* RPAREN )
    ;

parameter_declaration :
        parameter_name[T_DECL] ( K_IN | ( ( K_OUT | K_IN K_OUT ) k_nocopy? ) )? datatype
        ( ( ASSIGN | k_default ) expression )?
    ;

declare_section :
    ( type_definition SEMI                         //-> ^('t_declaration' type_definition SEMI)
    | subtype_definition SEMI                      //-> ^('t_declaration' subtype_definition SEMI)
    | cursor_definition SEMI                       //-> ^('t_declaration' cursor_definition SEMI)
    | item_declaration SEMI                        //-> ^('t_declaration' item_declaration SEMI)
    | function_declaration_or_definition SEMI      //-> ^('t_declaration' function_declaration_or_definition SEMI)
    | procedure_declaration_or_definition SEMI     //-> ^('t_declaration' procedure_declaration_or_definition SEMI)
    | pragma                                       //-> ^('t_declaration' pragma)
    )+
    ;


type_definition_sub : k_type identifier[T_DATATYPE_NAME,T_DECL] ( k_is | k_as ) ( record_type_definition | collection_type_definition | ref_cursor_type_definition ) ;
type_definition : type_definition_sub
        //->^('t_type_decl' type_definition_sub)
    ;

subtype_definition_sub : k_subtype identifier[T_DATATYPE_NAME,T_DECL] ( k_is | k_as ) datatype ( k_not k_null )? ;
subtype_definition: subtype_definition_sub
        //->^('t_subtype_decl' subtype_definition_sub)
    ;

cursor_definition_sub : k_cursor identifier[T_CURSOR_NAME,T_DECL] parameter_declarations? (k_return datatype)? k_is select_statement ;
cursor_definition : cursor_definition_sub
        //->^('t_cursor_decl' cursor_definition_sub)
    ;

item_declaration
    : variable_declaration  //->^('t_variable_decl' variable_declaration)
    | constant_declaration  //->^('t_const_decl' constant_declaration)
    | exception_declaration //->^('t_exc_decl' exception_declaration)
    ;

variable_declaration : variable_name[T_DECL] datatype (  (  k_not k_null )? (  ASSIGN  | k_default ) expression  )? ;

constant_declaration : variable_name[T_DECL] k_constant datatype ( k_not k_null )? (   ASSIGN  | k_default  ) expression ;

exception_declaration : identifier[T_EXCEPTION_NAME,T_DECL] k_exception ;
    
record_type_definition :
	k_record LPAREN record_field_declaration ( COMMA record_field_declaration )* RPAREN
    ;

record_field_declaration :
	ID datatype ( ( k_not k_null )? ( ASSIGN | k_default ) expression )?
    ;

collection_type_definition
	:	varray_type_definition
	|	nested_table_type_definition
	;

varray_type_definition
	:	( k_varying k_array? | k_varray ) LPAREN numeric_literal RPAREN kOF datatype ( k_not k_null )?
	;

nested_table_type_definition
	:	k_table kOF datatype ( k_not k_null )? ( k_index k_by associative_index_type )?
	;

associative_index_type
	:	datatype
	;

ref_cursor_type_definition
	:	k_ref k_cursor ( k_return datatype )?
	;

datatype_name
	// :	k_binary_integer 
	// |	k_binary_float
	// |	k_binary_double
	// |	k_natural
	// |	k_positive
	// |	( r_number | k_numeric | r_decimal | k_dec ) ( LPAREN INTEGER ( COMMA INTEGER )? RPAREN )?
	:	r_long ( r_raw)? ( LPAREN INTEGER RPAREN )?
	|	r_raw ( LPAREN INTEGER RPAREN )?
	//|	k_boolean
	//|	r_date
	|	k_interval kDAY ( LPAREN INTEGER RPAREN )? r_to kSECOND ( LPAREN INTEGER RPAREN )?
	|	k_interval kYEAR ( LPAREN INTEGER RPAREN )? r_to kMONTH
	|	( /*k_time |*/ k_timestamp ) ( LPAREN INTEGER RPAREN )? ( r_with ( kLOCAL )? kTIME kZONE )?
	// |	r_integer
	// |	k_int
	// |	r_smallint
	// |	r_float ( LPAREN INTEGER RPAREN )?
	// |	k_real
	// |	k_double k_precision
	|	r_char      ( k_varying )? ( LPAREN INTEGER ( kBYTE | r_char )? RPAREN )? ( k_character r_set ( identifier[T_UNKNOWN,0] | identifier[T_UNKNOWN,0] charset_attr ) )?
	|	r_varchar                  ( LPAREN INTEGER ( kBYTE | r_char )? RPAREN )? ( k_character r_set ( identifier[T_UNKNOWN,0] | identifier[T_UNKNOWN,0] charset_attr ) )?
	|	r_varchar2                 ( LPAREN INTEGER ( kBYTE | r_char )? RPAREN )? ( k_character r_set ( identifier[T_UNKNOWN,0] | identifier[T_UNKNOWN,0] charset_attr ) )?
	|	k_character ( k_varying )? ( LPAREN INTEGER RPAREN )?
	|	k_nchar     ( k_varying )? ( LPAREN INTEGER RPAREN )?
	//|	k_nvarchar  ( LPAREN INTEGER RPAREN )?
	//|	k_nvarchar2 ( LPAREN INTEGER RPAREN )?
	|	k_national  ( k_character | r_char ) ( k_varying )? ( LPAREN INTEGER RPAREN )?
	//|	k_mlslabel
	//|	k_pls_integer
	//|	k_blob
	|	k_clob ( k_character r_set ( identifier[T_UNKNOWN,0] | identifier[T_UNKNOWN,0] charset_attr ) )?
	//|	k_nclob
	//|	k_bfile
	//|	r_rowid 
	//|	k_urowid ( LPAREN INTEGER RPAREN )?
    ;

datatype
    : ( k_ref )? (schema_name DOT) (object_attribute_name DOT)+ identifier[T_TABLE_NAME,T_USE] DOT column_name[T_USE] type_attr
	| ( k_ref )? (schema_name DOT)? identifier[T_TABLE_NAME,T_USE] DOT column_name[T_USE] type_attr
    | ( k_ref )? (schema_name DOT)? variable_name_ex[T_USE] type_attr
    | ( k_ref )? (schema_name DOT)? identifier[T_TABLE_NAME,T_USE] rowtype_attr        
    | ( k_ref )? datatype_name
    | ( k_ref )? (k_date | k_timestamp) ( LPAREN numeric_literal ( COMMA numeric_literal )? RPAREN )?
    | ( k_ref )? (schema_name DOT) (identifier[T_OBJECT_NAME,T_USE] DOT)+ identifier[T_DATATYPE_NAME,T_USE]
    | ( k_ref )? (schema_name DOT)? identifier[T_DATATYPE_NAME,T_USE] ( LPAREN numeric_literal ( COMMA numeric_literal )? RPAREN )?
    ;

function_declaration_or_definition : function_declaration_or_definition_sub
        //-> ^('t_function_decl' function_declaration_or_definition_sub)
    ;
function_declaration_or_definition_sub :
        function_heading
        ( k_deterministic | k_pipelined | k_parallel_enable (LPAREN ~(RPAREN)+ RPAREN)? | k_result_cache )*
        ( ( k_is | k_as ) declare_section? body exception_handler? k_end ID? )?
	;

// function_declaration :
//         function_heading
//         ( k_deterministic | k_pipelined | k_parallel_enable (LPAREN ~(RPAREN)+ RPAREN)? | k_result_cache )*
//     ;

// function_definition :
//         function_heading
//         ( k_deterministic | k_pipelined | k_parallel_enable (LPAREN ~(RPAREN)+ RPAREN)? | k_result_cache )*
//         ( k_is | k_as ) declare_section? body exception_handler? k_end ID?
// 	;

procedure_declaration_or_definition : procedure_declaration_or_definition_sub
        //-> ^('t_procedure_decl' procedure_declaration_or_definition_sub)
    ;
procedure_declaration_or_definition_sub :
        procedure_heading
        ( ( k_is | k_as ) declare_section? body exception_handler? k_end ID? )?
    ;

// procedure_declaration :
// 	procedure_heading
// 	;

// procedure_definition :
// 	procedure_heading
// 	( k_is | k_as ) declare_section? body exception_handler? k_end ID?
// 	;
	
statement :
    label*
    ( (k_execute k_immediate) => execute_immediate_statement
	| (k_if) => if_statement
	| (k_case) => case_statement
    | (k_open) => open_statement            
	| (k_close) => close_statement
    | (k_continue) => continue_statement
    | (k_pipe) => pipe_row_statement
    | (k_goto) => goto_statement 
    | (k_exit) => exit_statement
    | (k_return) => return_statement
    | (k_declare | k_begin) => plsql_block
    | (k_raise) => raise_statement
    | (k_fetch) => fetch_statement
    | (k_loop) => basic_loop_statement
    | (k_while) => while_loop_statement
    | (k_null) => null_statement
    | (k_for) => for_loop_statement
    | (k_forall) => forall_statement
    | (k_commit | k_delete | k_insert | k_lock | k_rollback | k_savepoint | k_select | r_with | k_merge | r_set | k_update ) => sql_statement            
//	| assign_statement
//  | procedure_call
    | assign_or_call_statement
    )
    SEMI
    ;
        
lvalue
    : call_first[T_USEL] ( DOT call_next[T_USEL] )*
    ;

assign_or_call_statement
    : lvalue DOT delete_call   //-> ^('t_call' lvalue DOT delete_call)
    | lvalue ASSIGN expression //-> ^('t_assign' lvalue ASSIGN expression)
    | lvalue                   //-> ^('t_call' lvalue)
    ;

call_first[int usageType]
    : (bindvar | variable_name[usageType] ) dblink_name? ( LPAREN ( parameter ( COMMA parameter )* )? RPAREN )?
    ;

call_next[int usageType]
    : (variable_name_ex[usageType]) dblink_name? ( LPAREN ( parameter ( COMMA parameter )* )? RPAREN )?
    ;

delete_call
    : k_delete ( LPAREN parameter? RPAREN )?
    ;

basic_loop_statement : basic_loop_statement_sub
        //->^('t_loop' basic_loop_statement_sub)
    ;
basic_loop_statement_sub : k_loop statement+ k_end k_loop label_name? ;

case_statement : case_statement_sub
        //->^('t_case' case_statement_sub)
    ;
case_statement_sub :
        k_case expression?
        ( k_when expression k_then ( statement+ | expression) )+
        ( k_else ( statement+ | expression) )?
        k_end k_case?
    ;

close_statement : close_statement_sub
        //->^('t_close' close_statement_sub)
    ;
close_statement_sub : k_close cursor_name ;

continue_statement: continue_statement_sub
        //->^('t_continue' continue_statement_sub)
;
continue_statement_sub : k_continue ( lbl=ID )? ( k_when expression )? ;

execute_immediate_statement : execute_immediate_statement_sub
        //->^('t_execute_immediate' execute_immediate_statement_sub)
    ;
execute_immediate_statement_sub :
        k_execute k_immediate expression (
        ( into_clause | bulk_collect_into_clause) using_clause?
        | using_clause dynamic_returning_clause?
        | dynamic_returning_clause
        )?
    ;

exit_statement : exit_statement_sub
        //->^('t_exit' exit_statement_sub)
    ;
exit_statement_sub : k_exit ( lbl=ID )? ( k_when expression )? ;

fetch_statement : fetch_statement_sub
        //->^('t_fetch' fetch_statement_sub)
    ;
fetch_statement_sub : k_fetch qual_id ( into_clause | bulk_collect_into_clause ( k_limit numeric_expression )? ) ;
    
into_clause :
        k_into lvalue ( COMMA lvalue )*
    ;
    
bulk_collect_into_clause :
        k_bulk k_collect k_into lvalue ( COMMA lvalue )*
    ;

using_clause :
        K_USING param_modifiers? expression ( COMMA param_modifiers? expression )*
    ;

param_modifiers
	: K_IN K_OUT? | K_OUT
	;

dynamic_returning_clause :
        ( k_returning | k_return ) ( into_clause | bulk_collect_into_clause )
    ;

for_loop_statement : for_loop_statement_sub
	//->^('t_for' for_loop_statement_sub)
;

for_loop_statement_sub : k_for ID K_IN ( ~('LOOP') )+ k_loop statement+ k_end k_loop label_name? ;

forall_statement : forall_statement_sub
        //->^('t_forall' forall_statement_sub)
    ;
forall_statement_sub :
        k_forall ID K_IN bounds_clause
        (
            ( kSAVE kEXCEPTIONS ) sql_statement
        |   sql_statement ( kSAVE kEXCEPTIONS )?
        |   execute_immediate_statement
        )
    ; 

bounds_clause 
    : numeric_expression DOUBLE_PERIOD numeric_expression
    | kINDICES kOF atom ( k_between numeric_expression k_and numeric_expression )?
    | kVALUES kOF atom
    ;

goto_statement : k_goto label_name
        //->^('t_goto' k_goto label_name)
    ;

if_statement :
        // k_if expression (k_then^ s1=statement+) 
        // ( k_elsif^ expression (k_then^ s2=statement+) )*
        // ( k_else^ s3=statement+ )?
        //k_end k_if //-> ^('t_endif' k_end k_if))
		if_statement_if
        if_statement_elsif*
        if_statement_else?
		if_statement_end
    ;
if_statement_if
	: k_if expression k_then statement+
		//->  ^('t_if' k_if expression k_then ^('t_block_block' statement+))
	;
if_statement_elsif
	: k_elsif expression k_then statement+
		//->  ^('t_elsif' k_elsif expression k_then ^('t_block_block' statement+))
	;
if_statement_else
	: k_else statement+
		//->  ^('t_else' k_else ^('t_block_block' statement+))
	;
if_statement_end
	: k_end k_if
		//->^('t_endif' k_end k_if)
	;

while_loop_statement : while_loop_statement_sub
		//->^('t_while' while_loop_statement_sub)
	;
while_loop_statement_sub : k_while expression k_loop statement+ k_end k_loop label_name? ;

pipe_row_statement : pipe_row_statement_sub
        //->^('t_pipe' pipe_row_statement_sub)
    ;
pipe_row_statement_sub : k_pipe k_row LPAREN expression RPAREN ;

null_statement : k_null
        //->^('t_null' k_null)
    ;

//TODO add more RAM and fix using section (use expression or using_clause)
open_statement : open_statement_sub
		//->^('t_open' open_statement_sub)
    ;
open_statement_sub : k_open cursor_name call_args? ( k_for (select_statement|expression) )? (K_USING lvalue ( COMMA lvalue )* )? ;

pragma : k_pragma swallow_to_semi SEMI
        //->^('t_pragma_decl' k_pragma swallow_to_semi SEMI)
    ;

raise_statement : raise_statement_sub
		//->^('t_raise' raise_statement_sub)
    ;
raise_statement_sub : k_raise ( (ID DOT)* exception_name )? ;

return_statement : return_statement_sub
        //->^('t_return' return_statement_sub)
    ;
return_statement_sub : k_return expression? ;

declare_block
	:	k_declare declare_section?
        //->^('t_decl_block' k_declare declare_section?)
    ;

body
	:	k_begin pragma* statement*
        //-> ^('t_block_block' k_begin pragma* statement*)
	;

exception_handler : exception_handler_sub
		//->^('t_exceptions_block' exception_handler_sub)
    ;
exception_handler_sub
	:	k_exception
        (
            k_when ( qual_id ( k_or qual_id )* | k_others )
            k_then statement+
        )+
	;

plsql_block
	:	declare_block? body exception_handler? k_end ID?
        //-> ^('t_block_block' declare_block? body exception_handler? k_end ID?)
    ;

label :
        LLABEL label_name RLABEL
    ;

// qual_id :
// 	COLON? ID ( DOT COLON? ID )*
//     ;
qual_id :
	COLON? identifier[T_UNKNOWN,T_USE] ( DOT identifier[T_UNKNOWN,T_USE] )*
    ;

schema_name
	: identifier[T_SCHEMA_NAME, T_USE]
	;

object_attribute_name
	: identifier[T_OBJECT_NAME, T_USE]
	;

cursor_name
	: (schema_name DOT)? identifier[T_CURSOR_NAME,T_USE]
	| bindvar
	| COLON identifier[T_CURSOR_NAME,T_USE]
	;

variable_name[int usageType]
	:       identifier[T_VARIABLE_NAME,usageType]
	| 'FOUND'
    | 'TIMESTAMP'
	| 'OUT'
	| 'ROWCOUNT'
	| 'INTERVAL'
	;

variable_name_ex[int usageType]
	:       identifier[T_VARIABLE_NAME,usageType]
	//| K_OUT // Grr. how could somebody make something like this?
	| 'FOUND'
    | 'TIMESTAMP'
//	| 'TYPE'
	| 'EXECUTE'
//	| 'OUT'
	| 'ROWCOUNT'
	| 'INTERVAL'        
//	| 'ENABLE' | 'DISABLE'
	| 'LANGUAGE'
    | 'DELETE'
    | 'RAISE'
	| 'OPEN'
    | 'CALL'        
	;

parameter_name[int usageType]
	:       identifier[T_PARAMETER_NAME,usageType]
	| 'FOUND'
    | 'TIMESTAMP'
//    | 'TYPE'
//	| 'OUT'
	| 'ROWCOUNT'
	| 'INTERVAL'
	;

column_name[int usageType]
	:       identifier[T_COLUMN_NAME,usageType]
	//| K_OUT // Grr. how could somebody make something like this?
	| 'FOUND'
    | 'TIMESTAMP'
//	| 'TYPE'
	| 'EXECUTE'
//	| 'OUT'
	| 'ROWCOUNT'
	| 'INTERVAL'        
//	| 'ENABLE' | 'DISABLE'
	| 'LANGUAGE'
    | 'DELETE'
    | 'RAISE'
	;

procedure_name[int usageType]
	:       identifier[T_PROCEDURE_NAME,usageType]
	//| K_OUT // Grr. how could somebody make something like this?
	| 'FOUND'
    | 'TIMESTAMP'
//	| 'TYPE'
	| 'EXECUTE'
//	| 'OUT'
	| 'ROWCOUNT'
	| 'INTERVAL'        
//	| 'ENABLE'
	| 'LANGUAGE'
    | 'DELETE'
    | 'RAISE'
	| 'OPEN'
    | 'CALL'
	;

exception_name
	: /*((schema_name DOT)?*/ (package_name[T_USE] DOT)? identifier[T_EXCEPTION_NAME,T_USE]
	;

package_name[int usageType]
	: identifier[T_PACKAGE_NAME,usageType]
	| 'ARRAY'
	;

identifier[int identifierClass, int usageType]
	: i=ID                    { /* $i->user1 = identifierClass; $i->user2 = usageType; */ }
	;
sql_statement
    : commit_statement //-> ^('t_commit' commit_statement)
    | delete_statement //-> ^('t_delete' delete_statement)
    | insert_statement //-> ^('t_insert' insert_statement)
    | lock_table_statement //-> ^('t_lock_table' lock_table_statement)
    | rollback_statement   //-> ^('t_rollback' rollback_statement)
    | savepoint_statement  //-> ^('t_savepoint' savepoint_statement)
    | select_statement     //-> ^('t_select' select_statement)
    | merge_statement      //-> ^('t_merge' merge_statement)
    | set_transaction_statement //-> ^('t_set_transaction' set_transaction_statement)
    | update_statement     //-> ^('t_update' update_statement)
    ;

commit_statement :
        k_commit swallow_to_semi?
    ;

delete_statement :
        k_delete swallow_to_semi
    ;

insert_statement :
        k_insert swallow_to_semi
    ;

lock_table_statement :
        k_lock k_table swallow_to_semi
    ;

rollback_statement :
        k_rollback swallow_to_semi?
    ;

savepoint_statement :
        k_savepoint ID
    ;

select_statement :
      LPAREN* k_select swallow_to_semi
    |  LPAREN* r_with swallow_to_semi
    ;

merge_statement :
        k_merge swallow_to_semi
    ;

set_transaction_statement :
        r_set k_transaction swallow_to_semi
    ;

update_statement :
        k_update swallow_to_semi
    ;

swallow_to_semi :
        ~( SEMI )+
    ;

label_name:	identifier[T_LABEL_NAME,T_USE];

dblink_name: AT_SIGN identifier[T_DBLINK_NAME,T_USE] (DOT identifier[T_DBLINK_NAME,T_USE])*;

expression
    : or_expr
    ;

or_expr
    : and_expr ( k_or and_expr )*
    ;

and_expr
    : not_expr ( k_and not_expr )*
    ;

not_expr
    : k_not? compare_expr
    ;

compare_expr
    : is_null_expr ( ( EQ | not_equal | LTH | leq | GTH | geq ) is_null_expr )?        
    ;

is_null_expr
    : like_expr ( k_is k_not? k_null)?
    ;

like_expr
    : between_expr ( k_not? k_like between_expr ( k_escape quoted_string )? )?        
    ;

between_expr
    : in_expr ( k_not? k_between in_expr k_and in_expr )?
    ;

in_expr
    : add_expr ( k_not? K_IN LPAREN add_expr ( COMMA add_expr )* RPAREN )?
    ;

numeric_expression
    : add_expr
    ;

add_expr
    : mul_expr ( ( MINUS | PLUS | DOUBLEVERTBAR ) mul_expr )*
    ;

mul_expr
    : unary_sign_expr ( ( ASTERISK | DIVIDE | kMOD ) unary_sign_expr )*
    ;

unary_sign_expr
    : ( MINUS | PLUS )? atom
    ;

// exponent_expr
//     : atom ( k_exponent atom )?
//     ;

atom
    : (k_case) => case_statement
	| (k_cast) => cast_expression
	| extract_datetime_expression
	| variable_or_function_call attribute?
    | k_sql attribute
    | string_literal
    | numeric_atom
    | boolean_atom
    | k_null
	| timestamp_expression
    | LPAREN expression RPAREN
    ;
    
variable_or_function_call
    : call_first[T_USE] ( DOT call_next[T_USE] )* ( DOT delete_call )?
    ;

timestamp_expression
	: (k_date | k_timestamp) quoted_string
	| k_systimestamp (kAT kTIME kZONE quoted_string)?
    ;

cast_expression
    : k_cast LPAREN (expression /* TODO | k_multiset subquery */) k_as (datatype | identifier[T_DATATYPE_NAME,T_USE] ) RPAREN
    ;

extract_datetime_expression
	:	//k_extract
		identifier[T_FUNCTION_NAME,0]
        LPAREN
        ( identifier[T_RESERVED,0]
        //     kYEAR
        // |	kMONTH
        // |	kDAY
        // |	kHOUR
        // |	kMINUTE
        // |	k_second
        // |	k_timezone_hour
        // |	k_timezone_minute
        // |	k_timezone_region
        // |	k_timezone_abbr
        )
        r_from
        expression
        // TODO this cast should be in atom
        ( identifier[T_RESERVED,0] r_to identifier[T_RESERVED,0])?
        RPAREN
    ;

attribute
    : type_attr
    | rowtype_attr
    | notfound_attr
    | found_attr
    | isopen_attr
    | rowcount_attr
    | bulk_rowcount_attr LPAREN expression RPAREN
    | charset_attr              //
    ;

call_args
    : LPAREN ( parameter ( COMMA parameter )* )? RPAREN
    ;

boolean_atom
    : boolean_literal
    | collection_exists
    | conditional_predicate
    ;

numeric_atom
    : numeric_literal
    ;

numeric_literal
    : NUMBER_LIT
    | INTEGER
    | APPROXIMATE_NUM_LIT
    ;

boolean_literal
    : k_true
    | k_false
    ;

string_literal
    : QUOTED_STRING
    ;

collection_exists
    : variable_name[T_USE] DOT k_exists LPAREN expression RPAREN
    ;

conditional_predicate
    : k_inserting
    | k_updating ( LPAREN QUOTED_STRING RPAREN )?
    | k_deleting
    ;

parameter
    : ( ID ARROW )? expression
    ;

// index
//     : expression
//     ;

create_package :
		(k_create ( k_or kREPLACE )?)?
		k_package ( schema_name DOT )? package_name[T_DECL]
        ( invoker_rights_clause )?
        ( k_is | k_as ) declare_section? k_end package_name[T_USE]? SEMI
    ;

create_package_body :
        (k_create ( k_or kREPLACE )?)?
        k_package k_body ( schema_name DOT )? package_name[T_DECL]
        ( k_is | k_as ) declare_section?
        ( body exception_handler? k_end ID? | k_end package_name[T_USE]? )
        SEMI
    ;

create_procedure :
        (k_create ( k_or kREPLACE )?)?
        k_procedure ( schema_name DOT )? procedure_name[T_DECL]
        ( LPAREN parameter_declaration ( COMMA parameter_declaration )* RPAREN )?
        invoker_rights_clause?
        ( k_is | k_as )
        ( declare_section? body exception_handler? k_end ID?
        | call_spec
        | k_external
        ) SEMI
    ;

create_function :
        (k_create ( k_or kREPLACE )?)?
        // function_definition
        k_function ( schema_name DOT )? f=identifier[T_FUNCTION_NAME,T_DECL]
        ( LPAREN parameter_declaration ( COMMA parameter_declaration )* RPAREN )?
        k_return datatype
        ( k_deterministic | k_pipelined | k_parallel_enable (LPAREN ~(RPAREN)+ RPAREN)? | k_result_cache | invoker_rights_clause )*
        ( ( k_is | k_as ) declare_section? body exception_handler? k_end ID?
        | call_spec
        | k_external
        | k_aggregate? using_clause
        ) (SEMI|EOF) // TODO remove '?' and EOF
    ;

create_trigger :
        (k_create ( k_or kREPLACE )?)? k_trigger ( schema_name DOT )? t=identifier[T_TRIGGER_NAME,T_DECL]
//         ( sim | com | non_dml )
//         ( k_enable | k_disable )?
//         ( k_when LPAREN expression /*TODO condition*/ RPAREN )?
//         ( plsql_block | k_call routine_clause | comp_block)
        ~('DECLARE'|'BEGIN'|'CALL')*
        (k_declare|k_begin) => plsql_block
        SEMI
    ;
        
create_type :
        (k_create ( k_or kREPLACE )?)?
        //type_definition
        k_type identifier[T_DATATYPE_NAME, T_DECL]
        //( k_is | k_as )
        swallow_to_semi
        (SEMI|EOF) // TODO remove '?' and EOF
    ;

invoker_rights_clause :
        k_authid ( k_current_user | k_definer )
    ;

call_spec
    : k_language swallow_to_semi
    ;

r_char       : r='CHAR'     { /* $r->set_type(T_RESERVED); */ } ;
r_from       : r='FROM'     { /* $r->set_type(T_RESERVED); */ } ;
r_long       : r='LONG'     { /* $r->set_type(T_RESERVED); */ } ;
r_raw        : r='RAW'      { /* $r->set_type(T_RESERVED); */ } ;
r_set        : r='SET'      { /* $r->set_type(T_RESERVED); */ } ;
r_to         : r='TO'       { /* $r->set_type(T_RESERVED); */ } ;
r_varchar    : r='VARCHAR'  { /* $r->set_type(T_RESERVED); */ } ;
r_varchar2   : r='VARCHAR2' { /* $r->set_type(T_RESERVED); */ } ;
r_with       : r='WITH'     { /* $r->set_type(T_RESERVED); */ } ;

// Java version
// kERRORS     : {input.LT(1).getText().length() >= 3 && "errors".startsWith(input.LT(1).getText().toLowerCase())}? ID;
// kAT         : {input.LT(1).getText().equalsIgnoreCase("at")}? ID;
// kEXCEPTIONS : {input.LT(1).getText().equalsIgnoreCase("exceptions")}? ID;
// kFOUND      : {input.LT(1).getText().equalsIgnoreCase("found")}? ID;
// kINDICES    : {input.LT(1).getText().equalsIgnoreCase("indices")}? ID;
// kMOD        : {input.LT(1).getText().equalsIgnoreCase("mod")}? ID;
// kNAME       : {input.LT(1).getText().equalsIgnoreCase("name")}? ID;
// kOF         : {input.LT(1).getText().equalsIgnoreCase("of")}? ID;
// kREPLACE    : {input.LT(1).getText().equalsIgnoreCase("replace")}? ID;
// kSAVE       : {input.LT(1).getText().equalsIgnoreCase("save")}? ID;
// kSHOW       : {input.LT(1).getText().equalsIgnoreCase("show")}? ID;
///// kTYPE       : {input.LT(1).getText().equalsIgnoreCase("type")}? ID;
// kVALUES     : {input.LT(1).getText().equalsIgnoreCase("values")}? ID;
//kBYTE          : {input.LT(1).getText().equalsIgnoreCase("byte"  )}? ID;
//kDAY           : {input.LT(1).getText().equalsIgnoreCase("day"   )}? ID;
//kMONTH         : {input.LT(1).getText().equalsIgnoreCase("month" )}? ID;
//kSECOND        : {input.LT(1).getText().equalsIgnoreCase("second")}? ID;
//kYEAR          : {input.LT(1).getText().equalsIgnoreCase("year"  )}? ID;
//kLOCAL         : {input.LT(1).getText().equalsIgnoreCase("local"  )}? ID;   
//kZONE          : {input.LT(1).getText().equalsIgnoreCase("zone"  )}? ID;
//kTIME          : {input.LT(1).getText().equalsIgnoreCase("time"  )}? ID; 

// C version
// kAT         : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "AT"     ))}? identifier[T_RESERVED,0];
// kERRORS     : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ERRORS"     ))}? identifier[T_RESERVED,0];
// kEXCEPTIONS : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "EXCEPTIONS" ))}? identifier[T_RESERVED,0];
// kFOUND      : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "FOUND"      ))}? identifier[T_RESERVED,0];
// kINDICES    : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "INDICES"    ))}? identifier[T_RESERVED,0];
// kMOD        : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MOD"        ))}? identifier[T_RESERVED,0];
// kNAME       : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NAME"       ))}? identifier[T_RESERVED,0];
// kOF         : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "OF"         ))}? identifier[T_RESERVED,0];
// kREPLACE    : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REPLACE"    ))}? identifier[T_RESERVED,0];
// kSAVE       : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SAVE"       ))}? identifier[T_RESERVED,0];
// kSHOW       : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SHOW"       ))}? identifier[T_RESERVED,0];
// ////kTYPE       : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TYPE"       ))}? identifier[T_RESERVED,0];
// kVALUES     : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "VALUES"     ))}? identifier[T_RESERVED,0];
// kBYTE       : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BYTE"       ))}? identifier[T_RESERVED,0];
// kDAY        : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DAY"        ))}? identifier[T_RESERVED,0];
// kMONTH      : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MONTH"      ))}? identifier[T_RESERVED,0];
// kSECOND     : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SECOND"     ))}? identifier[T_RESERVED,0];
// kYEAR       : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "YEAR"       ))}? identifier[T_RESERVED,0];
// kLOCAL      : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LOCAL"      ))}? identifier[T_RESERVED,0];
// kZONE       : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ZONE"       ))}? identifier[T_RESERVED,0];
// kTIME       : {!(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TIME"       ))}? identifier[T_RESERVED,0];

// C++ version
kAT         : { boost::iequals(LT(1)->getText(), "AT")          }? identifier[T_RESERVED,0];
kERRORS     : { boost::iequals(LT(1)->getText(), "ERRORS")      }? identifier[T_RESERVED,0];
kEXCEPTIONS : { boost::iequals(LT(1)->getText(), "EXCEPTIONS")  }? identifier[T_RESERVED,0];
kFOUND      : { boost::iequals(LT(1)->getText(), "FOUND")       }? identifier[T_RESERVED,0];
kINDICES    : { boost::iequals(LT(1)->getText(), "INDICES")     }? identifier[T_RESERVED,0];
kMOD        : { boost::iequals(LT(1)->getText(), "MOD")         }? identifier[T_RESERVED,0];
kNAME       : { boost::iequals(LT(1)->getText(), "NAME")        }? identifier[T_RESERVED,0];
kOF         : { boost::iequals(LT(1)->getText(), "OF")          }? identifier[T_RESERVED,0];
kREPLACE    : { boost::iequals(LT(1)->getText(), "REPLACE")     }? identifier[T_RESERVED,0];
kSAVE       : { boost::iequals(LT(1)->getText(), "SAVE")        }? identifier[T_RESERVED,0];
kSHOW       : { boost::iequals(LT(1)->getText(), "SHOW")        }? identifier[T_RESERVED,0];
////kTYPE   : { boost::iequals(LT(1)->getText(), "TYPE")        }? identifier[T_RESERVED,0];
kVALUES     : { boost::iequals(LT(1)->getText(), "VALUES")      }? identifier[T_RESERVED,0];
kBYTE       : { boost::iequals(LT(1)->getText(), "BYTE")        }? identifier[T_RESERVED,0];
kDAY        : { boost::iequals(LT(1)->getText(), "DAY")         }? identifier[T_RESERVED,0];
kMONTH      : { boost::iequals(LT(1)->getText(), "MONTH")       }? identifier[T_RESERVED,0];
kSECOND     : { boost::iequals(LT(1)->getText(), "SECOND")      }? identifier[T_RESERVED,0];
kYEAR       : { boost::iequals(LT(1)->getText(), "YEAR")        }? identifier[T_RESERVED,0];
kLOCAL      : { boost::iequals(LT(1)->getText(), "LOCAL")       }? identifier[T_RESERVED,0];
kZONE       : { boost::iequals(LT(1)->getText(), "ZONE")        }? identifier[T_RESERVED,0];
kTIME       : { boost::iequals(LT(1)->getText(), "TIME")        }? identifier[T_RESERVED,0];

k_aggregate	:    r='AGGREGATE'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_and :	         r='AND'                { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_array :        r='ARRAY'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_as :           r='AS'                 { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_authid :       r='AUTHID'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_between :      r='BETWEEN'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_body :         r='BODY'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_bulk :         r='BULK'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_bulk_rowcount : r='BULK_ROWCOUNT'   { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_by :           r='BY'                 { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_case :         r='CASE'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_character :    r='CHARACTER'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_create :       r='CREATE'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_collect :      r='COLLECT'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_commit :       r='COMMIT'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_current_user : r='CURRENT_USER'       { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_date :         r='DATE'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_default :      r='DEFAULT'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_definer :      r='DEFINER'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_delete :	     r='DELETE'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_else :         r='ELSE'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_elsif :        r='ELSIF'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_external :     r='EXTERNAL'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_false :        r='FALSE'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_fetch :        r='FETCH'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_for :          r='FOR'                { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_forall :       r='FORALL'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_goto :         r='GOTO'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_if :           r='IF'                 { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
K_IN :             'IN';
k_index :        r='INDEX'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_insert :       r='INSERT'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_into :         r='INTO'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_is  :          r='IS'                 { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_language :     r='LANGUAGE'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_like :         r='LIKE'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_limit :        r='LIMIT'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_lock :         r='LOCK'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_nchar :        r='NCHAR'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_not :          r='NOT'                { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_notfound :   r='NOTFOUND'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_null :         r='NULL'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_open :         r='OPEN'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_or :           r='OR'                 { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_package :      r='PACKAGE'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_raise :        r='RAISE'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_rollback :     r='ROLLBACK'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_savepoint :    r='SAVEPOINT'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_select :       r='SELECT'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_set :        r='SET'                { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_sql :          r='SQL'                { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_table :        r='TABLE'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_time :       r='TIME'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_timestamp :    r='TIMESTAMP'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_transaction :  r='TRANSACTION'        { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_trigger :      r='TRIGGER'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_true :         r='TRUE'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_then :         r='THEN'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_type :         r='TYPE'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ; //NOTE see : kTYPE
k_update :       r='UPDATE'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_while :        r='WHILE'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_inserting :    r='INSERTING'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_updating :     r='UPDATING'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_deleting :     r='DELETING'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_isopen :     r='ISOPEN'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_exists :       r='EXISTS'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_begin :        r='BEGIN'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_call :         r='CALL'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_cast :         r='CAST'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_clob :         r='CLOB'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_close :        r='CLOSE'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_constant :     r='CONSTANT'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_continue :     r='CONTINUE'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_cursor :       r='CURSOR'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_disable :    r='DISABLE'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_declare :      r='DECLARE'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_deterministic : r='DETERMINISTIC'     { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_enable :     r='ENABLE'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_end :          r='END'                { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_escape :       r='ESCAPE'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_exception :    r='EXCEPTION'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_execute :      r='EXECUTE'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_exit :         r='EXIT'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_extract :    r='EXTRACT'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_function :     r='FUNCTION'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_immediate :    r='IMMEDIATE'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_interval :     r='INTERVAL'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_loop	 :	     r='LOOP'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_merge :        r='MERGE'              { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_national :     r='NATIONAL'           { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_nocopy :       r='NOCOPY'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_others :       r='OTHERS'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
K_OUT :            'OUT';
k_parallel_enable : r='PARALLEL_ENABLE' { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_pipe :         r='PIPE'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_pipelined :    r='PIPELINED'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_pragma :       r='PRAGMA'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_procedure :    r='PROCEDURE'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_record :       r='RECORD'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_ref :          r='REF'                { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_result_cache : r='RESULT_CACHE'       { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_return :       r='RETURN'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_returning :    r='RETURNING'          { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_row :          r='ROW'                { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_rowtype :      r='ROWTYPE'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_systimestamp : r='SYSTIMESTAMP'       { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_subtype :      r='SUBTYPE'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
K_USING :          'USING';
k_varray :       r='VARRAY'             { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_varying :      r='VARYING'            { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
k_when :         r='WHEN'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;
//k_with :       r='WITH'               { /* $r->set_type(T_RESERVED); */ /* $r->user1 = T_RESERVED; */  } ;


quoted_string
	:	QUOTED_STRING | QSTRING
	;

QUOTED_STRING
	:	( 'n'|'N' )? '\'' ( '\'\'' | ~('\'') )* '\''
	;

/* Perl-style quoted string */
QSTRING             : ('q'|'Q') ( QS_ANGLE | QS_BRACE | QS_BRACK | QS_PAREN | QS_OTHER) ;
fragment QS_ANGLE   : QUOTE '<' ( options {greedy=false;} : . )* '>' QUOTE ;
fragment QS_BRACE   : QUOTE '{' ( options {greedy=false;} : . )* '}' QUOTE ;
fragment QS_BRACK   : QUOTE '[' ( options {greedy=false;} : . )* ']' QUOTE ;
fragment QS_PAREN   : QUOTE '(' ( options {greedy=false;} : . )* ')' QUOTE ;

fragment QS_OTHER_CH: ~('<'|'{'|'['|'('|' '|'\t'|'\n'|'\r');
fragment QS_OTHER
/* C Syntax */ 
// @declarations {
// 	ANTLR3_UINT32 (*oldLA)(struct ANTLR3_INT_STREAM_struct *, ANTLR3_INT32);
// }
// @init {
// 	oldLA = INPUT->istream->_LA;
//     INPUT->setUcaseLA(INPUT, ANTLR3_FALSE);
// }
		:	
		QUOTE delimiter=QS_OTHER_CH
/* JAVA Syntax */        
// 		( { input.LT(1) != $delimiter.text.charAt(0) || ( input.LT(1) == $delimiter.text.charAt(0) && input.LT(2) != '\'') }? => . )*
// 		( { input.LT(1) == $delimiter.text.charAt(0) && input.LT(2) == '\'' }? => . ) QUOTE
/* C Syntax */ 
//		( { LA(1) != $delimiter->getText(delimiter)->chars[0] || LA(2) != '\'' }? => . )*
//		( { LA(1) == $delimiter->getText(delimiter)->chars[0] && LA(2) == '\'' }? => . ) QUOTE
// 		{ INPUT->istream->_LA = oldLA; }
/* C++ Syntax */
		( { LA(1) != $delimiter->getText().at(0) || LA(2) != '\'' }? => . )*
		( { LA(1) == $delimiter->getText().at(0) && LA(2) == '\'' }? => . ) QUOTE
		;
bindvar
	:	b=BINDVAR  { /* $b->set_type(T_BINDVAR_NAME); */ /* $b->user1 = T_BINDVAR_NAME, $b->user2 = T_USE; */ }
    |	COLON i=ID { /* $i->set_type(T_BINDVAR_NAME); */ /* $i->user1 = T_BINDVAR_NAME, $i->user2 = T_USE; */ }
	;
BINDVAR /*options { testLiterals=true; }*/
    :	COLON 'A' .. 'Z' ( 'A' .. 'Z' | '0' .. '9' | '_' | '$' | '#' | COLON )*
    |	COLON DOUBLEQUOTED_STRING
    |	COLON NUM
    ;

ID /*options { testLiterals=true; }*/
    :	'A' .. 'Z' ( 'A' .. 'Z' | '0' .. '9' | '_' | '$' | '#' )*
    |	DOUBLEQUOTED_STRING
    ;
SEMI
	:	';'
	;
COLON
	:	':'
	;
COMMA 
	:	','
	;
ASTERISK
	:	'*'
	;
AT_SIGN
	:	'@'
	;
RPAREN
	:	')'
	;
LPAREN
	:	'('
	;
RBRACK
	:	']'
	;
LBRACK
	:	'['
	;
PLUS
	:	'+'
	;
MINUS
	:	'-'
	;
DIVIDE
	:	'/'
	;
EQ
	:	'='
	;
PERCENT // NOTE was PERCENTAGE
 	:	'%'
 	;
LLABEL
	:	'<<'
	;
RLABEL
	:	'>>'
	;
ASSIGN
	:	':='
	;
ARROW
	:	'=>'
	;
VERTBAR
	:	'|'
	;
doublevertbar
	:	DOUBLEVERTBAR
    |	VERTBAR VERTBAR
    ;
DOUBLEVERTBAR
	:	'||'
	;
not_equal
	:	NOT_EQ
    |	LTH GTH
    |	'!' EQ
    |	'^' EQ
    |	'~' EQ        
    ;
NOT_EQ
	:	'<>' | '!=' | '^=' | '~='
	;
LTH
	:	'<'
	;
leq
	:	LEQ
    |	LTH EQ
    ;
LEQ
	:	'<='
	;
GTH
	:	'>'
	;
geq
	: GEQ
    | GTH EQ
    ;
GEQ
	:	'>='
	;

FOR_NOTATION
    :   a=INTEGER 
        { 
            $a->set_type(INTEGER);
            get_tokSource()->enqueueToken($a);    	
            advanceInput();
        }
        b='..'             
        { 
        	$b->set_type(DOUBLE_PERIOD);
        	get_tokSource()->enqueueToken($b);
        	advanceInput();
        }
        c=INTEGER
        {
        	$c->set_type(INTEGER);
        	get_tokSource()->enqueueToken($c);
        	advanceInput();
        	
        	//$channel=HIDDEN; // All three tokens were already emited (the token of type FOR_NOTATION is put into the hidden channel)
            skip();
        }
    ;

DOT
    :    '.' 
    {    if ((char) LA(1) == '.') {
            consume();
            $type = DOUBLE_PERIOD;
        }
    }        
    ;

NUMBER_LIT
    :
	(
		INTEGER
		( '.' INTEGER?
		| { $type = INTEGER; }		
		)
		( ('E' | 'e') ('+' | '-')? INTEGER {$type = APPROXIMATE_NUM_LIT;} )?
    |	
		'.' INTEGER ( ('E' | 'e') ('+' | '-')? INTEGER {$type = APPROXIMATE_NUM_LIT;} )?
    )
    ( 'D' | 'd' | 'f' | 'F')?
    ;

fragment
INTEGER
    :    ('0'..'9')+ 
    ;

fragment
NUM
	: '0' .. '9' ( '0' .. '9' )*
	;
QUOTE
	:	'\''
	;
fragment
DOUBLEQUOTED_STRING
	:	'"' ( ~('"') )* '"'
	;
WS	:	(' '|'\r'|'\t'|'\n') {$channel=HIDDEN;}
	;
SL_COMMENT
	:	'--' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
	;
ML_COMMENT
	:	'/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
	;
type_attr
	:	PERCENT k_type
	;
rowtype_attr
	:	PERCENT k_rowtype
	;
notfound_attr
	:	PERCENT 'NOTFOUND'
	;
found_attr
	:	PERCENT 'FOUND'
	;
isopen_attr
	:	PERCENT 'ISOPEN'
	;
rowcount_attr
	:	PERCENT 'ROWCOUNT'
	;
bulk_rowcount_attr
	:	PERCENT 'BULK_ROWCOUNT'
	;
charset_attr
 	:	PERCENT 'CHARSET'
 	;
