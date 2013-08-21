/**
 * Oracle(c) PL/SQL 11g Lexer/Parser  
 *
 * Copyright (c) 2009-2011 Alexandre Porcelli <alexandre.porcelli@gmail.com>
 * Copyright (c) 2012-2013 Ivan Brezina <ibre5041@ibrezina.net>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This code was forked from PL/SQL parser and adopted
 * for QScintilla's custom lexer.
 *
 */

lexer grammar PLSQLGuiLexer;

options {
        language=Cpp;
}

tokens {
    DOUBLE_PERIOD;
    SQLPLUS_COMMAND;
    SQLPLUS_SOLIDUS;
}

@lexer::includes 
{
#include "UserGuiTraits.hpp"
#include <vector>
}

@lexer::namespace 
{ Antlr3Impl }

@header {
/*
 * Oracle(c) PL/SQL 11g Parser  
 *
 * Copyright (c) 2009-2011 Alexandre Porcelli <alexandre.porcelli@gmail.com>
 * Copyright (c) 2012-2013 Ivan Brezina <ibre5041@ibrezina.net> 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This code was forked from PL/SQL parser and adoptopted 
 * for QScintilla's custom lexer.
 */
}

@lexer::context {
  void advanceInput();
}

@lexer::members {
  //@lexer::members
  void PLSQLGuiLexer::advanceInput()
  {
    RecognizerSharedStateType *state = get_state();
    state->set_tokenStartCharIndex(getCharIndex());    
    state->set_tokenStartCharPositionInLine(getCharPositionInLine());
    state->set_tokenStartLine(getLine());
  }
}

SELECT_COMMAND_INTRODUCER:
        'SELECT'                        |
        'WITH';

DML_COMMAND_INTRODUCER:
        'INSERT'                        |
        'UPDATE'                        |
        'MERGE'                         |
        'COMMIT'                        | // COMMIT COMMENT 'Transfer From 7715 to 7720' WRITE IMMEDIATE NOWAIT;
        'ROLLBACK'                      | // ROLLBACK TO my_savepoint;
        'SAVEPOINT'                     |
        'LOCK'                          |
        'DELETE';
    
PLSQL_COMMAND_INTRODUCER:
        (
        c='CALL'                        | // TODO hande call in the TRIGGER declaration
        d='DECLARE'   { $d->setBlockContext(BlkCtx::DECLARE);   $d->set_type(PLSQL_COMMAND_INTRODUCER); get_tokSource()->enqueueToken($d); } |
        b='BEGIN'     { $b->setBlockContext(BlkCtx::BEGIN);     $b->set_type(PLSQL_COMMAND_INTRODUCER); get_tokSource()->enqueueToken($b); } |
        p='PROCEDURE' { $p->setBlockContext(BlkCtx::PROCEDURE); $p->set_type(PLSQL_COMMAND_INTRODUCER); get_tokSource()->enqueueToken($p); } |
        t='TYPE'      { $t->setBlockContext(BlkCtx::PROCEDURE); $t->set_type(PLSQL_COMMAND_INTRODUCER); get_tokSource()->enqueueToken($t); } |            
        f='FUNCTION'  { $f->setBlockContext(BlkCtx::PROCEDURE); $f->set_type(PLSQL_COMMAND_INTRODUCER); get_tokSource()->enqueueToken($f); } |
        t='TRIGGER'   { $t->setBlockContext(BlkCtx::PROCEDURE); $t->set_type(PLSQL_COMMAND_INTRODUCER); get_tokSource()->enqueueToken($t); } |
        c='CREATE'    { $c->setBlockContext(BlkCtx::CREATE);    $c->set_type(PLSQL_COMMAND_INTRODUCER); get_tokSource()->enqueueToken($c); } |
        p='PACKAGE'   { $p->setBlockContext(BlkCtx::PACKAGE);   $p->set_type(PLSQL_COMMAND_INTRODUCER); get_tokSource()->enqueueToken($p); }
        )
        { skip(); }//{ $channel = HIDDEN; }
    ;

OTHER_COMMAND_INTRODUCER:
        'ALTER'                         |
        'ANALYZE'                       |
        'ASSOCIATE'                     |
        'AUDIT'                         |
        'COMMENT'                       |
        'DISASSOCIATE'                  |        
        'DROP'                          |
        'EXPLAIN'                       |        
        'FLASHBACK'                     |
        'GRANT'                         |
        'NOAUDIT'                       |
        'RENAME'                        |
        'REVOKE'                        |
        //'SET'                           |  // either "SET ROLE/CONSTRAINT/TRANSACTION ..." or "SET LINE ..." or "UPDATE A SET A.C1=.."
        'TRUNCATE';        

// NOTE: this one must be listed before SQLPLUS CONNECT
CONNECT_BY
	:	'CONNECT_BY_ROOT'                  /* SQL_NON_RESERVED_CONNECT_BY_ROOT:*/
        { $type = PLSQL_RESERVED; }
    ;

// Ambiguous word can be either PLSQL function, SQL reserved word or SQLPLUS command
SET
	@init
	{
		ANTLR_UINT32 linePos = getCharPositionInLine(); // TODO check linePos == 0
		ANTLR_UINT32 line = getLine();
	}
	:   // First check is set is part sqlplus command (should start on newline)
        ('SET' SPACE REGULAR_ID SPACE
            (
             	('.'|';'|'_'|'&'|'#'|'>'|'\\'|'$'|'%'|'@'|'?') (NEWLINE|EOF)
            |	UNSIGNED_INTEGER
            |	CHAR_STRING
            )
        ) => ('SET' SPACE SWALLOW_TO_NEWLINE_FRAGMENT)
        	{ $type = SQLPLUS_COMMAND_INTRODUCER; }
        // Second check if is followed by '(' => part of SET operator (buildin function)
    	|	('SET' SPACE_FRAGMENT LEFT_PAREN) => 'SET'
        	{ $type = BUILDIN_FUNCTIONS; }
        // Otherwise PL/SQL reserved word
		|	'SET'
        	{ $type = PLSQL_RESERVED; }
    ;

// Ambiguous word can be either PLSQL statement or SQLPLUS command
EXIT
	@init
	{
		ANTLR_UINT32 linePos = getCharPositionInLine(); // TODO check linePos == 0
		ANTLR_UINT32 line = getLine();
	}
	: ('EXIT' SPACE 'WHEN') => e='EXIT'             { $type = PLSQL_RESERVED; }
	| ('EXIT' SPACE_FRAGMENT SEMICOLON) => e='EXIT' { $type = PLSQL_RESERVED; }	
    | 'EXIT'
        {
            if( linePos == 0 )
                $type = SQLPLUS_COMMAND_INTRODUCER;
			else
                $type = PLSQL_RESERVED;
        }
	;

// Ambiguous word can be either PLSQL statement or SQLPLUS command
EXECUTE
	@init
	{
		ANTLR_UINT32 linePos = getCharPositionInLine(); // TODO check linePos == 0
		ANTLR_UINT32 line = getLine();
	}
	: ('EXECUTE' (SPACE|NEWLINE)+ 'IMMEDIATE') => e='EXECUTE' { $type = PLSQL_RESERVED; }
    | 'EXECUTE'
       {
            if( linePos == 0 )
                $type = SQLPLUS_COMMAND_INTRODUCER;
			else
                $type = PLSQL_RESERVED;
       }
	;

// Ambiguous word can be either SQL keyword or SQLPLUS command
CONNECT
	@init
	{
		ANTLR_UINT32 linePos = getCharPositionInLine(); // TODO check linePos == 0
		ANTLR_UINT32 line = getLine();
	}
	: ('CONNECT' (SPACE|NEWLINE)+ 'BY') => e='CONNECT' { $type = PLSQL_RESERVED; }
    | 'CONNECT'
       {
            if( linePos == 0 )
                $type = SQLPLUS_COMMAND_INTRODUCER;
			else
                $type = PLSQL_RESERVED;
       }
	;

// All these should start a NEWLINE, tricky to implement
fragment
SQLPLUS_COMMAND_INTRODUCER
	// @init
	// {
	//     ANTLR_UINT32 linePos = getCharPositionInLine();
	//     ANTLR_UINT32 line = getLine();
	// }
    :
	(
	//	'@' ('@')?
	//|	'/'           // See SOLIDUS rule
        'ACCEPT'
    |   'APPEND'
    |   'ARCHIVE'
    |   'ATTRIBUTE'
    |   'BREAK'
    |   'BTITLE'
    |   'CHANGE'
    |   'CLEAR'
    |   'COLUMN'
    |   'COMPUTE'
    |   'CONN' ('E'('C')?)?                 // the whole word "CONNECT" is handled elsewhere
    |   'COPY'
    |   'DEF'  ('I'('N'('E')?)?)?
    |   'DEL'
    |   'DESC' ('R'('I'('B'('E')?)?)?)?
    |   'DISCONNECT'
    |   'EDIT'
    |   'EXEC' ('U'('T')?)?                       // the whole word "EXECUTE" is handled elsewhere
    //|   'EXIT'                                  // See EXIT rule
    |   'GET'
    |   'HELP'
    |   'HOST'
    |   'INPUT'
    |   'LIST'
    |   'PASSWORD'
    |   'PAUSE'
    |   'PRINT'
    |   'PROMPT'
    |   'QUIT'
    |   'RECOVER'
    |   'REMARK'
    |   'REPFOOTER'
    |   'REPHEADER'
    |   'RUN'
    |   'SAVE'
    //|   'SET'
    |   'SHOW'
    |   'SHUTDOWN'
    |   'SPOOL'
	|   'STARTUP' //|   'START' ('UP')?	
    |   'STORE'
    |   'TIMING'
    |   'TTITLE'
    |   'UNDEFINE'
    |   'VAR' ('I'('A'('B'('L'('E')?)?)?)?)?
    |   'WHENEVER'
    |   'XQUERY'
	)
	(SPACE SWALLOW_TO_NEWLINE_FRAGMENT)?
    ;

// All these PLSQL constructs can also contain 'END';
R_IF:   r='IF'   { $r->setBlockContext(BlkCtx::LOOP); $r->set_type(R_IF);   get_tokSource()->enqueueToken($r); skip(); /*$channel = HIDDEN;*/ };
R_THEN: r='THEN';
R_LOOP: r='LOOP' { $r->setBlockContext(BlkCtx::LOOP); $r->set_type(R_LOOP); get_tokSource()->enqueueToken($r); skip(); /*$channel = HIDDEN;*/ };
R_CASE: r='CASE' { $r->setBlockContext(BlkCtx::LOOP); $r->set_type(R_CASE); get_tokSource()->enqueueToken($r); skip(); /*$channel = HIDDEN;*/ };
R_END:  r='END'	 { $r->setBlockContext(BlkCtx::END);  $r->set_type(R_END);  get_tokSource()->enqueueToken($r); skip(); /*$channel = HIDDEN;*/ };

R_AS:   a='AS' { $a->setBlockContext(BlkCtx::DECLARE); $a->set_type(PLSQL_RESERVED); get_tokSource()->enqueueToken($a); skip(); /*$channel = HIDDEN;*/ };
R_IS:   i='IS' { $i->setBlockContext(BlkCtx::DECLARE); $i->set_type(PLSQL_RESERVED); get_tokSource()->enqueueToken($i); skip(); /*$channel = HIDDEN;*/ };

PLSQL_RESERVED:
        'A'                                                                     | /* A_KEY: */
        'ADD'                                                                   | /* ADD_KEY: */
        'ADVISE'                                                                | /* ADVISE_KEY: */
        'AFTER'                                                                 | /* AFTER_KEY: */
        'AGENT'                                                                 | /* AGENT_KEY: */
        'AGGREGATE'                                                             | /* AGGREGATE_KEY: */
        'ALL'                                                                   | /* SQL92_RESERVED_ALL:*/ 
        //'ALTER'                                                               | /* SQL92_RESERVED_ALTER:*/ 
        //'ANALYZE'                                                             | /* ANALYZE_KEY: */
        'AND'                                                                   | /* SQL92_RESERVED_AND:*/ 
        'ANY'                                                                   | /* SQL92_RESERVED_ANY:*/ 
        'ARRAY'                                                                 | /* ARRAY_KEY: */
        //'AS'                                                                  | /* SQL92_RESERVED_AS:*/ 
        'ASC'                                                                   | /* SQL92_RESERVED_ASC:*/ 
        //'ASSOCIATE'                                                           | /* ASSOCIATE_KEY: */
        'AT'                                                                    | /* AT_KEY: */
        'ATTRIBUTE'                                                             | /* ATTRIBUTE_KEY: */
        //'AUDIT'                                                               | /* AUDIT_KEY: */
        'AUTHID'                                                                | /* AUTHID_KEY: */
        'AUTO'                                                                  | /* AUTO_KEY: */
        'AUTOMATIC'                                                             | /* AUTOMATIC_KEY: */
        'AUTONOMOUS_TRANSACTION'                                                | /* AUTONOMOUS_TRANSACTION_KEY: */
        'BATCH'                                                                 | /* BATCH_KEY: */
        'BEFORE'                                                                | /* BEFORE_KEY: */
        //'BEGIN'                                                               | /* SQL92_RESERVED_BEGIN:*/ 
        'BETWEEN'                                                               | /* SQL92_RESERVED_BETWEEN:*/ 
        'BFILE'                                                                 | /* BFILE_KEY: */
        'BINARY_DOUBLE'                                                         | /* BINARY_DOUBLE_KEY: */
        'BINARY_DOUBLE_INFINITY'                                                | /* BINARY_DOUBLE_INFINITY_KEY: */
        'BINARY_DOUBLE_MAX_NORMAL'                                              | /* BINARY_DOUBLE_MAX_NORMAL_KEY: */
        'BINARY_DOUBLE_MAX_SUBNORMAL'                                           | /* BINARY_DOUBLE_MAX_SUBNORMAL_KEY: */
        'BINARY_DOUBLE_MIN_NORMAL'                                              | /* BINARY_DOUBLE_MIN_NORMAL_KEY: */
        'BINARY_DOUBLE_MIN_SUBNORMAL'                                           | /* BINARY_DOUBLE_MIN_SUBNORMAL_KEY: */
        'BINARY_DOUBLE_NAN'                                                     | /* BINARY_DOUBLE_NAN_KEY: */
        'BINARY_FLOAT'                                                          | /* BINARY_FLOAT_KEY: */
        'BINARY_FLOAT_INFINITY'                                                 | /* BINARY_FLOAT_INFINITY_KEY: */
        'BINARY_FLOAT_MAX_NORMAL'                                               | /* BINARY_FLOAT_MAX_NORMAL_KEY: */
        'BINARY_FLOAT_MAX_SUBNORMAL'                                            | /* BINARY_FLOAT_MAX_SUBNORMAL_KEY: */
        'BINARY_FLOAT_MIN_NORMAL'                                               | /* BINARY_FLOAT_MIN_NORMAL_KEY: */
        'BINARY_FLOAT_MIN_SUBNORMAL'                                            | /* BINARY_FLOAT_MIN_SUBNORMAL_KEY: */
        'BINARY_FLOAT_NAN'                                                      | /* BINARY_FLOAT_NAN_KEY: */
        'BINARY_INTEGER'                                                        | /* BINARY_INTEGER_KEY: */
        'BIT'                                                                   | /* BIT_KEY: */
        'BIT_LENGTH'                                                            | /* BIT_LENGTH_KEY: */
        'BLOB'                                                                  | /* BLOB_KEY: */
        'BLOCK'                                                                 | /* BLOCK_KEY: */
        'BODY'                                                                  | /* BODY_KEY: */
        'BOOLEAN'                                                               | /* BOOLEAN_KEY: */
        'BOTH'                                                                  | /* BOTH_KEY: */
        'BREADTH'                                                               | /* BREADTH_KEY: */
        'BULK'                                                                  | /* BULK_KEY: */
        'BULK_ROWCOUNT'                                                         | /* PERCENT_BULK_ROWCOUNT_KEY: */
        'BY'                                                                    | /* SQL92_RESERVED_BY:*/ 
        'BYTE'                                                                  | /* BYTE_KEY: */
        'C'                                                                     | /* C_KEY: */
        'CACHE'                                                                 | /* CACHE_KEY: */
        //'CALL'                                                                | /* CALL_KEY: */
        'CANONICAL'                                                             | /* CANONICAL_KEY: */
        'CASCADE'                                                               | /* CASCADE_KEY: */
        //'CASE'                                                                | /* SQL92_RESERVED_CASE:*/ 
        'CHAR'                                                                  | /* CHAR_KEY: */
        'CHARACTER'                                                             | /* CHARACTER_KEY: */
        'CHARACTER_LENGTH'                                                      | /* CHARACTER_LENGTH_KEY: */
        'CHAR_CS'                                                               | /* CHAR_CS_KEY: */
        'CHARSET'                                                               | /* PERCENT_CHARSET_KEY: */
        'CHECK'                                                                 | /* SQL92_RESERVED_CHECK:*/ 
        'CLOB'                                                                  | /* CLOB_KEY: */
        'CLOSE'                                                                 | /* CLOSE_KEY: */
        'CLUSTER'                                                               | /* CLUSTER_KEY: */
        'CLUSTERS'                                                              | /* PLSQL_RESERVED_CLUSTERS:*/ 
        'COLAUTH'                                                               | /* PLSQL_RESERVED_COLAUTH:*/ 
        'COLLATE'                                                               | /* COLLATE_KEY: */
        'COLUMNS'                                                               | /* COLUMNS_KEY: */
        //'COMMENT'                                                             | /* COMMENT_KEY: */
        //'COMMIT'                                                              | /* COMMIT_KEY: */
        'COMMITTED'                                                             | /* COMMITTED_KEY: */
        'COMPATIBILITY'                                                         | /* COMPATIBILITY_KEY: */
        'COMPILE'                                                               | /* COMPILE_KEY: */
        'COMPOUND'                                                              | /* COMPOUND_KEY: */
        'COMPRESS'                                                              | /* PLSQL_RESERVED_COMPRESS:*/ 
        //'CONNECT'                                                             | /* SQL92_RESERVED_CONNECT:*/ 
        //'CONNECT_BY_ROOT'                                                     | /* SQL_NON_RESERVED_CONNECT_BY_ROOT:*/ 
        'CONSTANT'                                                              | /* CONSTANT_KEY: */
        'CONSTRAINT'                                                            | /* CONSTRAINT_KEY: */
        'CONSTRAINTS'                                                           | /* CONSTRAINTS_KEY: */
        'CONSTRUCTOR'                                                           | /* CONSTRUCTOR_KEY: */
        'CONTENT'                                                               | /* CONTENT_KEY: */
        'CONTEXT'                                                               | /* CONTEXT_KEY: */
        'CONTINUE'                                                              | /* CONTINUE_KEY: */
        'CORRESPONDING'                                                         | /* CORRESPONDING_KEY: */
        'CORRUPT_XID'                                                           | /* CORRUPT_XID_KEY: */
        'CORRUPT_XID_ALL'                                                       | /* CORRUPT_XID_ALL_KEY: */
        'COST'                                                                  | /* COST_KEY: */
        'CRASH'                                                                 | /* PLSQL_RESERVED_CRASH:*/ 
        //'CREATE'                                                              | /* SQL92_RESERVED_CREATE:*/ 
        'CROSS'                                                                 | /* CROSS_KEY: */
        'CUBE'                                                                  | /* CUBE_KEY: */
        'CURRENT'                                                               | /* SQL92_RESERVED_CURRENT:*/ 
        'CURRENT_USER'                                                          | /* CURRENT_USER_KEY: */
        'CURSOR'                                                                | /* SQL92_RESERVED_CURSOR:*/ 
        'CUSTOMDATUM'                                                           | /* CUSTOMDATUM_KEY: */
        'CYCLE'                                                                 | /* CYCLE_KEY: */
        'DATA'                                                                  | /* DATA_KEY: */
        'DATABASE'                                                              | /* DATABASE_KEY: */
        'DATE'                                                                  | /* SQL92_RESERVED_DATE:*/ 
        'DAY'                                                                   | /* DAY_KEY: */
        'DB_ROLE_CHANGE'                                                        | /* DB_ROLE_CHANGE_KEY: */
        'DDL'                                                                   | /* DDL_KEY: */
        'DEBUG'                                                                 | /* DEBUG_KEY: */
        'DEC'                                                                   | /* DEC_KEY: */
        'DECIMAL'                                                               | /* DECIMAL_KEY: */
        //'DECLARE'                                                             | /* SQL92_RESERVED_DECLARE:*/ 
        'DECREMENT'                                                             | /* DECREMENT_KEY: */
        'DEFAULT'                                                               | /* SQL92_RESERVED_DEFAULT:*/ 
        'DEFAULTS'                                                              | /* DEFAULTS_KEY: */
        'DEFERRED'                                                              | /* DEFERRED_KEY: */
        'DEFINER'                                                               | /* DEFINER_KEY: */
        //'DELETE'                                                              | /* SQL92_RESERVED_DELETE:*/ 
        //'DESC'                                                                | /* SQL92_RESERVED_DESC:*/ 
        'DETERMINISTIC'                                                         | /* DETERMINISTIC_KEY: */
        'DIMENSION'                                                             | /* DIMENSION_KEY: */
        'DISABLE'                                                               | /* DISABLE_KEY: */
        //'DISASSOCIATE'                                                        | /* DISASSOCIATE_KEY: */
        'DISTINCT'                                                              | /* SQL92_RESERVED_DISTINCT:*/ 
        'DML'                                                                   | /* DML_KEY: */
        'DOCUMENT'                                                              | /* DOCUMENT_KEY: */
        'DOUBLE'                                                                | /* DOUBLE_KEY: */
        //'DROP'                                                                | /* SQL92_RESERVED_DROP:*/ 
        'DSINTERVAL_UNCONSTRAINED'                                              | /* DSINTERVAL_UNCONSTRAINED_KEY: */
        'EACH'                                                                  | /* EACH_KEY: */
        'ELEMENT'                                                               | /* ELEMENT_KEY: */
        'ELSE'                                                                  | /* SQL92_RESERVED_ELSE:*/ 
        'ELSIF'                                                                 | /* PLSQL_NON_RESERVED_ELSIF:*/ 
        'EMPTY'                                                                 | /* EMPTY_KEY: */
        'ENABLE'                                                                | /* ENABLE_KEY: */
        'ENCODING'                                                              | /* ENCODING_KEY: */
        //'END'                                                                 | /* SQL92_RESERVED_END:*/ 
        'ENTITYESCAPING'                                                        | /* ENTITYESCAPING_KEY: */
        'ERRORS'                                                                | /* ERRORS_KEY: */
        'ESCAPE'                                                                | /* ESCAPE_KEY: */
        'EVALNAME'                                                              | /* EVALNAME_KEY: */
        'EXCEPT'                                                                | /* EXCEPT_KEY: */
        'EXCEPTION'                                                             | /* SQL92_RESERVED_EXCEPTION:*/ 
        'EXCEPTIONS'                                                            | /* EXCEPTIONS_KEY: */
        'EXCEPTION_INIT'                                                        | /* EXCEPTION_INIT_KEY: */
        'EXCLUDE'                                                               | /* EXCLUDE_KEY: */
        'EXCLUSIVE'                                                             | /* PLSQL_RESERVED_EXCLUSIVE:*/ 
        'EXECUTE'                                                               | /* EXECUTE_KEY: */
        'EXISTS'                                                                | /* SQL92_RESERVED_EXISTS:*/ 
        //'EXIT'                                                                | /* EXIT_KEY: */
        //'EXPLAIN'                                                             | /* EXPLAIN_KEY: */
        'EXTEND'                                                                | /* EXTEND_KEY: */
        'EXTERNAL'                                                              | /* EXTERNAL_KEY: */
        'FAILURE'                                                               | /* FAILURE_KEY: */
        'FALSE'                                                                 | /* SQL92_RESERVED_FALSE:*/ 
        'FETCH'                                                                 | /* SQL92_RESERVED_FETCH:*/ 
        'FINAL'                                                                 | /* FINAL_KEY: */
        //'FLASHBACK'                                                           | /* FLASHBACK_KEY: */
        'FLOAT'                                                                 | /* FLOAT_KEY: */
        'FOLLOWING'                                                             | /* FOLLOWING_KEY: */
        'FOUND'                                                                 | /* PERCENT_FOUND_KEY: */
        'FOLLOWS'                                                               | /* FOLLOWS_KEY: */
        'FOR'                                                                   | /* SQL92_RESERVED_FOR:*/ 
        'FORALL'                                                                | /* FORALL_KEY: */
        'FORCE'                                                                 | /* FORCE_KEY: */
        'FROM'                                                                  | /* SQL92_RESERVED_FROM:*/ 
        'FULL'                                                                  | /* FULL_KEY: */
        //'FUNCTION'                                                            | /* FUNCTION_KEY: */
        'GLOBAL'                                                                | /* GLOBAL_KEY: */
        'GOTO'                                                                  | /* SQL92_RESERVED_GOTO:*/ 
        //'GRANT'                                                               | /* SQL92_RESERVED_GRANT:*/ 
        'GROUP'                                                                 | /* SQL92_RESERVED_GROUP:*/ 
        'GUARD'                                                                 | /* GUARD_KEY: */
        'HASH'                                                                  | /* HASH_KEY: */
        'HAVING'                                                                | /* SQL92_RESERVED_HAVING:*/ 
        'HIDE'                                                                  | /* HIDE_KEY: */
        'HOUR'                                                                  | /* HOUR_KEY: */
        'IDENTIFIED'                                                            | /* PLSQL_RESERVED_IDENTIFIED:*/ 
        //'IF'                                                                  | /* PLSQL_RESERVED_IF:*/ 
        'IGNORE'                                                                | /* IGNORE_KEY: */
        'IMMEDIATE'                                                             | /* IMMEDIATE_KEY: */
        'IN'                                                                    | /* SQL92_RESERVED_IN:*/ 
        'INCLUDE'                                                               | /* INCLUDE_KEY: */
        'INCLUDING'                                                             | /* INCLUDING_KEY: */
        'INCREMENT'                                                             | /* INCREMENT_KEY: */
        'INDENT'                                                                | /* INDENT_KEY: */
        'INDEX'                                                                 | /* PLSQL_RESERVED_INDEX:*/ 
        'INDEXED'                                                               | /* INDEXED_KEY: */
        'INDEXES'                                                               | /* PLSQL_RESERVED_INDEXES:*/ 
        'INDICATOR'                                                             | /* INDICATOR_KEY: */
        'INDICES'                                                               | /* INDICES_KEY: */
        'INFINITE'                                                              | /* INFINITE_KEY: */
        'INLINE'                                                                | /* INLINE_KEY: */
        'INNER'                                                                 | /* INNER_KEY: */
        'INOUT'                                                                 | /* INOUT_KEY: */
        //'INSERT'                                                              | /* SQL92_RESERVED_INSERT:*/ 
        'INSTANTIABLE'                                                          | /* INSTANTIABLE_KEY: */
        'INSTEAD'                                                               | /* INSTEAD_KEY: */
        'INT'                                                                   | /* INT_KEY: */
        'INTEGER'                                                               | /* INTEGER_KEY: */
        'INTERSECT'                                                             | /* SQL92_RESERVED_INTERSECT:*/ 
        'INTERVAL'                                                              | /* INTERVAL_KEY: */
        'INTO'                                                                  | /* SQL92_RESERVED_INTO:*/ 
        'INVALIDATE'                                                            | /* INVALIDATE_KEY: */
        //'IS'                                                                  | /* SQL92_RESERVED_IS:*/ 
        'ISOPEN'                                                                | /* PERCENT_ISOPEN_KEY: */
        'ISOLATION'                                                             | /* ISOLATION_KEY: */
        'ITERATE'                                                               | /* ITERATE_KEY: */
        'JAVA'                                                                  | /* JAVA_KEY: */
        'JOIN'                                                                  | /* JOIN_KEY: */
        'KEEP'                                                                  | /* KEEP_KEY: */
        'LANGUAGE'                                                              | /* LANGUAGE_KEY: */
        'LEADING'                                                               | /* LEADING_KEY: */
        'LEFT'                                                                  | /* LEFT_KEY: */
        'LEVEL'                                                                 | /* LEVEL_KEY: */
        'LIBRARY'                                                               | /* LIBRARY_KEY: */
        'LIKE'                                                                  | /* SQL92_RESERVED_LIKE:*/ 
        'LIKE2'                                                                 | /* LIKE2_KEY: */
        'LIKE4'                                                                 | /* LIKE4_KEY: */
        'LIKEC'                                                                 | /* LIKEC_KEY: */
        'LIMIT'                                                                 | /* LIMIT_KEY: */
        'LINK'                                                                  | /* LINK_KEY: */
        'LOCAL'                                                                 | /* LOCAL_KEY: */
        //'LOCK'                                                                | /* PLSQL_RESERVED_LOCK:*/ 
        'LOCKED'                                                                | /* LOCKED_KEY: */
        'LOGOFF'                                                                | /* LOGOFF_KEY: */
        'LOGON'                                                                 | /* LOGON_KEY: */
        'LONG'                                                                  | /* LONG_KEY: */
        //'LOOP'                                                                | /* LOOP_KEY: */
        'MAIN'                                                                  | /* MAIN_KEY: */
        'MAP'                                                                   | /* MAP_KEY: */
        'MATCH'                                                                 | /* MATCH_KEY: */
        'MATCHED'                                                               | /* MATCHED_KEY: */
        'MAXVALUE'                                                              | /* MAXVALUE_KEY: */
        'MEASURES'                                                              | /* MEASURES_KEY: */
        'MEMBER'                                                                | /* MEMBER_KEY: */
        //'MERGE'                                                               | /* MERGE_KEY: */
        'MINUS'                                                                 | /* PLSQL_RESERVED_MINUS:*/ 
        'MINUTE'                                                                | /* MINUTE_KEY: */
        'MINVALUE'                                                              | /* MINVALUE_KEY: */
        'MLSLABEL'                                                              | /* MLSLABEL_KEY: */
        'MODE'                                                                  | /* PLSQL_RESERVED_MODE:*/ 
        'MODEL'                                                                 | /* SQL_NON_RESERVED_MODEL:*/ 
        'MODIFY'                                                                | /* MODIFY_KEY: */
        'MODULE'                                                                | /* MODULE_KEY: */
        'MONTH'                                                                 | /* MONTH_KEY: */
        'MULTISET'                                                              | /* MULTISET_KEY: */
        'NAME'                                                                  | /* NAME_KEY: */
        'NAN'                                                                   | /* NAN_KEY: */
        'NATIONAL'                                                              | /* NATIONAL_KEY: */
        'NATURAL'                                                               | /* NATURAL_KEY: */
        'NATURALN'                                                              | /* NATURALN_KEY: */
        'NAV'                                                                   | /* NAV_KEY: */
        'NCHAR'                                                                 | /* NCHAR_KEY: */
        'NCHAR_CS'                                                              | /* NCHAR_CS_KEY: */
        'NCLOB'                                                                 | /* NCLOB_KEY: */
        'NESTED'                                                                | /* NESTED_KEY: */
        'NEW'                                                                   | /* NEW_KEY: */
        'NEXTVAL'                                                               | /* NEXTVAL_KEY: */
        'NO'                                                                    | /* NO_KEY: */
        //'NOAUDIT'                                                             | /* NOAUDIT_KEY: */
        'NOCACHE'                                                               | /* NOCACHE_KEY: */
        'NOCOMPRESS'                                                            | /* PLSQL_RESERVED_NOCOMPRESS:*/ 
        'NOCOPY'                                                                | /* NOCOPY_KEY: */
        'NOCYCLE'                                                               | /* NOCYCLE_KEY: */
        'NOENTITYESCAPING'                                                      | /* NOENTITYESCAPING_KEY: */
        'NOMAXVALUE'                                                            | /* NOMAXVALUE_KEY: */
        'NOMINVALUE'                                                            | /* NOMINVALUE_KEY: */
        'NONE'                                                                  | /* NONE_KEY: */
        'NOORDER'                                                               | /* NOORDER_KEY: */
        'NOSCHEMACHECK'                                                         | /* NOSCHEMACHECK_KEY: */
        'NOT'                                                                   | /* SQL92_RESERVED_NOT:*/ 
        'NOTFOUND'                                                              | /* PERCENT_NOTFOUND_KEY: */ 
        'NOTHING'                                                               | /* NOTHING_KEY: */
        'NOWAIT'                                                                | /* PLSQL_RESERVED_NOWAIT:*/ 
        'NULL'                                                                  | /* SQL92_RESERVED_NULL:*/ 
        'NULLS'                                                                 | /* NULLS_KEY: */
        'NUMBER'                                                                | /* NUMBER_KEY: */
        'NUMERIC'                                                               | /* NUMERIC_KEY: */
        'NVARCHAR2'                                                             | /* NVARCHAR2_KEY: */
        'OBJECT'                                                                | /* OBJECT_KEY: */
        'OCTET_LENGTH'                                                          | /* OCTET_LENGTH_KEY: */
        'OF'                                                                    | /* SQL92_RESERVED_OF:*/ 
        'OFF'                                                                   | /* OFF_KEY: */
        'OID'                                                                   | /* OID_KEY: */
        'OLD'                                                                   | /* OLD_KEY: */
        'ON'                                                                    | /* SQL92_RESERVED_ON:*/ 
        'ONLY'                                                                  | /* ONLY_KEY: */
        'OPEN'                                                                  | /* OPEN_KEY: */
        'OPTION'                                                                | /* SQL92_RESERVED_OPTION:*/ 
        'OR'                                                                    | /* SQL92_RESERVED_OR:*/ 
        'ORADATA'                                                               | /* ORADATA_KEY: */
        'ORDER'                                                                 | /* SQL92_RESERVED_ORDER:*/ 
        'ORDINALITY'                                                            | /* ORDINALITY_KEY: */
        'OSERROR'                                                               | /* OSERROR_KEY: */
        'OUT'                                                                   | /* OUT_KEY: */
        'OUTER'                                                                 | /* OUTER_KEY: */
        'OVER'                                                                  | /* OVER_KEY: */
        'OVERLAPS'                                                              | /* SQL92_RESERVED_OVERLAPS:*/ 
        'OVERRIDING'                                                            | /* OVERRIDING_KEY: */
        //'PACKAGE'                                                             | /* PACKAGE_KEY: */
        'PARALLEL'                                                              | /* PARALLEL_KEY: */
        'PARALLEL_ENABLE'                                                       | /* PARALLEL_ENABLE_KEY: */
        'PARAMETERS'                                                            | /* PARAMETERS_KEY: */
        'PARENT'                                                                | /* PARENT_KEY: */
        'PARTIAL'                                                               | /* PARTIAL_KEY: */
        'PARTITION'                                                             | /* PARTITION_KEY: */
        'PASSING'                                                               | /* PASSING_KEY: */
        'PIPELINED'                                                             | /* PIPELINED_KEY: */
        'PIVOT'                                                                 | /* SQL_NON_RESERVED_PIVOT:*/ 
        'PLAN'                                                                  | /* PLAN_KEY: */
        'PLS_INTEGER'                                                           | /* PLS_INTEGER_KEY: */
        'POSITION'                                                              | /* POSITION_KEY: */
        'POSITIVE'                                                              | /* POSITIVE_KEY: */
        'POSITIVEN'                                                             | /* POSITIVEN_KEY: */
        'PRAGMA'                                                                | /* PRAGMA_KEY: */
        'PRECEDING'                                                             | /* PRECEDING_KEY: */
        'PRECISION'                                                             | /* PRECISION_KEY: */
        'PRESENT'                                                               | /* PRESENT_KEY: */
        'PRIOR'                                                                 | /* SQL92_RESERVED_PRIOR:*/ 
        //'PROCEDURE'                                                           | /* SQL92_RESERVED_PROCEDURE:*/ 
        'PUBLIC'                                                                | /* SQL92_RESERVED_PUBLIC:*/ 
        'QUERY'                                                                 | /* QUERY_KEY: */
        'RAISE'                                                                 | /* RAISE_KEY: */
        'RANGE'                                                                 | /* RANGE_KEY: */
        'RAW'                                                                   | /* RAW_KEY: */
        'READ'                                                                  | /* READ_KEY: */
        'REAL'                                                                  | /* REAL_KEY: */
        'RECORD'                                                                | /* RECORD_KEY: */
        'REFERENCE'                                                             | /* REFERENCE_KEY: */
        'REFERENCING'                                                           | /* REFERENCING_KEY: */
        'REJECT'                                                                | /* REJECT_KEY: */
        'RELIES_ON'                                                             | /* RELIES_ON_KEY: */
        //'RENAME'                                                              | /* RENAME_KEY: */
        'REPLACE'                                                               | /* SQL92_NON_RESERVED_REPLACE:*/        
        'RESOURCE'                                                              | /* PLSQL_RESERVED_RESOURCE:*/ 
        'RESPECT'                                                               | /* RESPECT_KEY: */
        'RESTRICT_REFERENCES'                                                   | /* RESTRICT_REFERENCES_KEY: */
        'RESULT'                                                                | /* RESULT_KEY: */
        'RESULT_CACHE'                                                          | /* RESULT_CACHE_KEY: */
        'RESUMABLE'                                                             | /* RESUMABLE_KEY: */
        'RETURN'                                                                | /* RETURN_KEY: */
        'RETURNING'                                                             | /* RETURNING_KEY: */
        'REUSE'                                                                 | /* REUSE_KEY: */        
        'REVERSE'                                                               | /* REVERSE_KEY: */
        'REVOKE'                                                                | /* SQL92_RESERVED_REVOKE:*/
        'RIGHT'                                                                 | /* RIGHT_KEY: */
        'ROLE'                                                                  | /* ROLE_KEY: */
        //'ROLLBACK'                                                            | /* ROLLBACK_KEY: */
        'ROLLUP'                                                                | /* ROLLUP_KEY: */
        'ROW'                                                                   | /* ROW_KEY: */
        'ROWCOUNT'                                                              | /* PERCENT_ROWCOUNT_KEY: */
        'ROWID'                                                                 | /* ROWID_KEY: */
        'ROWS'                                                                  | /* ROWS_KEY: */
        'ROWTYPE'                                                               | /* PERCENT_ROWTYPE_KEY: */
        'RULES'                                                                 | /* RULES_KEY: */
        'SAMPLE'                                                                | /* SAMPLE_KEY: */
        'SAVE'                                                                  | /* SAVE_KEY: */
        //'SAVEPOINT'                                                           | /* SAVEPOINT_KEY: */
        'SCHEMA'                                                                | /* SCHEMA_KEY: */
        'SCHEMACHECK'                                                           | /* SCHEMACHECK_KEY: */
        'SCN'                                                                   | /* SCN_KEY: */
        'SEARCH'                                                                | /* SEARCH_KEY: */
        'SECOND'                                                                | /* SECOND_KEY: */
        'SEED'                                                                  | /* SEED_KEY: */
        'SEGMENT'                                                               | /* SEGMENT_KEY: */
        //'SELECT'                                                              | /* SQL92_RESERVED_SELECT:*/ 
        'SELF'                                                                  | /* SELF_KEY: */
        'SEQUENCE'                                                              | /* SEQUENCE_KEY: */
        'SEQUENTIAL'                                                            | /* SEQUENTIAL_KEY: */
        'SERIALIZABLE'                                                          | /* SERIALIZABLE_KEY: */
        'SERIALLY_REUSABLE'                                                     | /* SERIALLY_REUSABLE_KEY: */
        'SERVERERROR'                                                           | /* SERVERERROR_KEY: */
        'SERVEROUTPUT'                                                          | /* SERVEROUTPUT_KEY: */
        'SESSION'                                                               | /* SESSION_KEY: */
        'SESSION_USER'                                                          | /* SESSION_USER_KEY: */
        //'SET'                                                                 | /* SET_KEY: */
        'SETS'                                                                  | /* SETS_KEY: */
        'SETTINGS'                                                              | /* SETTINGS_KEY: */
        'SHARE'                                                                 | /* PLSQL_RESERVED_SHARE:*/ 
        'SHOW'                                                                  | /* SHOW_KEY: */
        'SHUTDOWN'                                                              | /* SHUTDOWN_KEY: */
        'SIBLINGS'                                                              | /* SIBLINGS_KEY: */
        'SIGNTYPE'                                                              | /* SIGNTYPE_KEY: */
        'SIMPLE_INTEGER'                                                        | /* SIMPLE_INTEGER_KEY: */
        'SINGLE'                                                                | /* SINGLE_KEY: */
        'SIZE'                                                                  | /* SQL92_RESERVED_SIZE:*/ 
        'SKIP'                                                                  | /* SKIP_KEY: */
        'SMALLINT'                                                              | /* SMALLINT_KEY: */
        'SNAPSHOT'                                                              | /* SNAPSHOT_KEY: */
        'SOME'                                                                  | /* SOME_KEY: */
        'SPECIFICATION'                                                         | /* SPECIFICATION_KEY: */
        'SQLDATA'                                                               | /* SQLDATA_KEY: */
        'SQLERROR'                                                              | /* SQLERROR_KEY: */
        'STANDALONE'                                                            | /* STANDALONE_KEY: */
        'START'                                                                 | /* PLSQL_RESERVED_START:*/ 
        //'STARTUP'                                                             | /* STARTUP_KEY: */
        'STATEMENT'                                                             | /* STATEMENT_KEY: */
        'STATEMENT_ID'                                                          | /* STATEMENT_ID_KEY: */
        'STATIC'                                                                | /* STATIC_KEY: */
        'STATISTICS'                                                            | /* STATISTICS_KEY: */
        'STRING'                                                                | /* STRING_KEY: */
        'SUBMULTISET'                                                           | /* SUBMULTISET_KEY: */
        'SUBPARTITION'                                                          | /* SUBPARTITION_KEY: */
        'SUBSTITUTABLE'                                                         | /* SUBSTITUTABLE_KEY: */
        'SUBTYPE'                                                               | /* SUBTYPE_KEY: */
        'SUCCESS'                                                               | /* SUCCESS_KEY: */
        'SUSPEND'                                                               | /* SUSPEND_KEY: */
        'SYSTEM'                                                                | /* SYSTEM_KEY: */
        'SYSTEM_USER'                                                           | /* SYSTEM_USER_KEY: */
        'TABAUTH'                                                               | /* PLSQL_RESERVED_TABAUTH:*/ 
        'TABLE'                                                                 | /* SQL92_RESERVED_TABLE:*/ 
        'THE'                                                                   | /* SQL92_RESERVED_THE:*/ 
        //'THEN'                                                                | /* SQL92_RESERVED_THEN:*/ 
        'TIME'                                                                  | /* TIME_KEY: */
        'TIMEOUT'                                                               | /* TIMEOUT_KEY: */
        'TIMESTAMP'                                                             | /* TIMESTAMP_KEY: */
        'TIMESTAMP_LTZ_UNCONSTRAINED'                                           | /* TIMESTAMP_LTZ_UNCONSTRAINED_KEY: */
        'TIMESTAMP_TZ_UNCONSTRAINED'                                            | /* TIMESTAMP_TZ_UNCONSTRAINED_KEY: */
        'TIMESTAMP_UNCONSTRAINED'                                               | /* TIMESTAMP_UNCONSTRAINED_KEY: */
        'TIMEZONE_ABBR'                                                         | /* TIMEZONE_ABBR_KEY: */
        'TIMEZONE_HOUR'                                                         | /* TIMEZONE_HOUR_KEY: */
        'TIMEZONE_MINUTE'                                                       | /* TIMEZONE_MINUTE_KEY: */
        'TIMEZONE_REGION'                                                       | /* TIMEZONE_REGION_KEY: */
        'TO'                                                                    | /* SQL92_RESERVED_TO:*/ 
        'TRAILING'                                                              | /* TRAILING_KEY: */
        'TRANSACTION'                                                           | /* TRANSACTION_KEY: */
        //'TRIGGER'                                                             | /* TRIGGER_KEY: */
        'TRUE'                                                                  | /* SQL92_RESERVED_TRUE:*/ 
        //'TRUNCATE'                                                            | /* TRUNCATE_KEY: */
        //'TYPE'                                                                  | /* TYPE_KEY: */
        'UNBOUNDED'                                                             | /* UNBOUNDED_KEY: */
        'UNDER'                                                                 | /* UNDER_KEY: */
        'UNION'                                                                 | /* SQL92_RESERVED_UNION:*/ 
        'UNIQUE'                                                                | /* SQL92_RESERVED_UNIQUE:*/ 
        'UNKNOWN'                                                               | /* UNKNOWN_KEY: */
        'UNLIMITED'                                                             | /* UNLIMITED_KEY: */
        'UNPIVOT'                                                               | /* SQL_NON_RESERVED_UNPIVOT:*/
        'UNTIL'                                                                 | /* UNTIL_KEY: */
        //'UPDATE'                                                              | /* SQL92_RESERVED_UPDATE:*/ 
        'UPDATED'                                                               | /* UPDATED_KEY: */
        'UPSERT'                                                                | /* UPSERT_KEY: */
        'UROWID'                                                                | /* UROWID_KEY: */
        'USE'                                                                   | /* USE_KEY: */
        'USING'                                                                 | /* PLSQL_NON_RESERVED_USING:*/ 
        'VALIDATE'                                                              | /* VALIDATE_KEY: */
        'VALUES'                                                                | /* SQL92_RESERVED_VALUES:*/ 
        'VARCHAR'                                                               | /* VARCHAR_KEY: */
        'VARCHAR2'                                                              | /* VARCHAR2_KEY: */
        //'VARIABLE'                                                            | /* VARIABLE_KEY: */
        'VARRAY'                                                                | /* VARRAY_KEY: */
        'VARYING'                                                               | /* VARYING_KEY: */
        'VERSION'                                                               | /* VERSION_KEY: */
        'VERSIONS'                                                              | /* VERSIONS_KEY: */
        'VIEW'                                                                  | /* SQL92_RESERVED_VIEW:*/ 
        'VIEWS'                                                                 | /* PLSQL_RESERVED_VIEWS:*/ 
        'WAIT'                                                                  | /* WAIT_KEY: */
        'WARNING'                                                               | /* WARNING_KEY: */
        'WELLFORMED'                                                            | /* WELLFORMED_KEY: */
        'WHEN'                                                                  | /* SQL92_RESERVED_WHEN:*/ 
        //'WHENEVER'                                                            | /* WHENEVER_KEY: */
        'WHERE'                                                                 | /* SQL92_RESERVED_WHERE:*/ 
        'WHILE'                                                                 | /* WHILE_KEY: */
        //'WITH'                                                                | /* SQL92_RESERVED_WITH:*/ 
        'WITHIN'                                                                | /* WITHIN_KEY: */
        'WORK'                                                                  | /* WORK_KEY: */
        'WRITE'                                                                 | /* WRITE_KEY: */
        'XML'                                                                   | /* XML_KEY: */
        'XMLATTRIBUTES'                                                         | /* XMLATTRIBUTES_KEY: */
        'XMLNAMESPACES'                                                         | /* XMLNAMESPACES_KEY: */
        'YEAR'                                                                  | /* YEAR_KEY: */
        'YES'                                                                   | /* YES_KEY: */
        'YMINTERVAL_UNCONSTRAINED'                                              | /* YMINTERVAL_UNCONSTRAINED_KEY: */
        'ZONE'                                                                  ; /* ZONE_KEY: */
       
BUILDIN_FUNCTIONS:
        'ABS'                         | /* Numeric */
        'ACOS'                        | /* Numeric */
        'ADD_MONTHS'                  | /* Datetime */
        'APPENDCHILDXML'              | /* XML */
        'ASCII'                       | /* Character */
        'ASCIISTR'                    | /* Conversion */
        'ASIN'                        | /* Numeric */
        'ATAN'                        | /* Numeric */
        'ATAN2'                       | /* Numeric */
        'AVG'                         | /* Aggregate + Analytic */
        'BFILENAME'                   | /* LOB */
        'BIN_TO_NUM'                  | /* Conversion */
        'BITAND'                      | /* Numeric */
        'CARDINALITY'                 | /* Collection */
        'CAST'                        | /* Conversion */
        'CEIL'                        | /* Numeric */
        'CHARTOROWID'                 | /* Conversion */
        'CHR'                         | /* Character */
        'CLUSTER_ID'                  | /* Data Mining */
        'CLUSTER_PROBABILITY'         | /* Data Mining */
        'CLUSTER_SET'                 | /* Data Mining */
        'COALESCE'                    | /* NULL-Related */
        'COLLECT'                     | /* Aggregate + Collection */
        'COMPOSE'                     | /* Conversion */
        'CONCAT'                      | /* Character */
        'CONVERT'                     | /* Conversion */
        'CORR'                        | /* Aggregate + Analytic */
        'CORR_K'                      | /* Aggregate */
        'CORR_S'                      | /* Aggregate */
        'COS'                         | /* Numeric */
        'COSH'                        | /* Numeric */
        'COUNT'                       | /* Aggregate + Analytic */
        'COVAR_POP'                   | /* Aggregate + Analytic */
        'COVAR_SAMP'                  | /* Aggregate + Analytic */
        'CUBE_TABLE'                  | /* OLAP */
        'CUME_DIST'                   | /* Aggregate + Analytic */
        'CURRENT_DATE'                | /* Datetime */
        'CURRENT_TIMESTAMP'           | /* Datetime */
        'CV'                          | /* Model */
        'DATAOBJ_TO_PARTITION'        | /* Data Cartrige */
        'DBTIMEZONE'                  | /* Datetime */
        'DECODE'                      | /* Encode/Decode */
        'DECOMPOSE'                   | /* Conversion */
        'DELETEXML'                   | /* XML */
        'DENSE_RANK'                  | /* Aggregate + Analytic */
        'DEPTH'                       | /* XML */
        'DEREF'                       | /* Object Reference */
        'DUMP'                        | /* Encode/Decode */
        'EMPTY_BLOB'                  | /* LOB */
        'EMPTY_CLOB'                  | /* LOB */
        'EXISTSNODE'                  | /* XML */
        'EXP'                         | /* Numeric */
        'EXTRACT'                     | /* Datetime + XML */
        'EXTRACTVALUE'                | /* XML */
        'FEATURE_ID'                  | /* Data Mining */
        'FEATURE_SET'                 | /* Data Mining */
        'FEATURE_VALUE'               | /* Data Mining */
        'FIRST'                       | /* Aggregate + Analytic */
        'FIRST_VALUE'                 | /* Analytic */
        'FLOOR'                       | /* Numeric */
        'FROM_TZ'                     | /* Datetime */
        'GREATEST'                    | /* General Comparsion */
        'GROUPING'                    | /* Aggregate */
        'GROUPING_ID'                 | /* Aggregate */
        'GROUP_ID'                    | /* Aggregate */
        'HEXTORAW'                    | /* Conversion */
        'INITCAP'                     | /* Character */
        'INSERTCHILDXML'              | /* XML */
        'INSERTCHILDXMLAFTER'         | /* XML */
        'INSERTCHILDXMLBEFORE'        | /* XML */
        'INSERTXMLAFTER'              | /* XML */
        'INSERTXMLBEFORE'             | /* XML */
        'INSTR'                       | /* Character */
        'ITERATION_NUMBER'            | /* Model */
        'LAG'                         | /* Analytic */
        'LAST'                        | /* Aggregate + Analytic */
        'LAST_DAY'                    | /* Datetime */
        'LAST_VALUE'                  | /* Analytic */
        'LEAD'                        | /* Analytic */
        'LEAST'                       | /* General Comparsion */
        'LENGTH'                      | /* Character */
        'LISTAGG'                     | /* Aggregate + Analytic */
        'LN'                          | /* Numeric */
        'LNNVL'                       | /* NULL-Related */
        'LOCALTIMESTAMP'              | /* Datetime */
        'LOG'                         | /* Numeric */
        'LOWER'                       | /* Character */
        'LPAD'                        | /* Character */
        'LTRIM'                       | /* Character */
        'MAKE_REF'                    | /* Object Reference */
        'MAX'                         | /* Aggregate + Analytic */
        'MEDIAN'                      | /* Aggregate */
        'MIN'                         | /* Aggregate + Analytic */
        'MOD'                         | /* Numeric */
        'MONTHS_BETWEEN'              | /* Datetime */
        'NANVL'                       | /* NULL-Related + Numeric */
        'NCHR'                        | /* Character */
        'NEW_TIME'                    | /* Datetime */
        'NEXT_DAY'                    | /* Datetime */
        'NLSSORT'                     | /* Character */
        'NLS_CHARSET_DECL_LEN'        | /* NLS Character */
        'NLS_CHARSET_ID'              | /* NLS Character */
        'NLS_CHARSET_NAME'            | /* NLS Character */
        'NLS_INITCAP'                 | /* Character */
        'NLS_LOWER'                   | /* Character */
        'NLS_UPPER'                   | /* Character */
        'NTH_VALUE'                   | /* Analytic */
        'NTILE'                       | /* Analytic */
        'NULLIF'                      | /* NULL-Related */
        'NUMTODSINTERVAL'             | /* Conversion + Datetime */
        'NUMTOYMINTERVAL'             | /* Conversion + Datetime */
        'NVL'                         | /* NULL-Related */
        'NVL2'                        | /* NULL-Related */
        'ORA_DST_AFFECTED'            | /* Datetime */
        'ORA_DST_CONVERT'             | /* Datetime */
        'ORA_DST_ERROR'               | /* Datetime */
        'ORA_HASH'                    | /* Encode/Decode */
        'PATH'                        | /* XML */
        'PERCENTILE_CONT'             | /* Aggregate + Analytic */
        'PERCENTILE_DISC'             | /* Aggregate + Analytic */
        'PERCENT_RANK'                | /* Aggregate + Analytic */
        'POWER'                       | /* Numeric */
        'POWERMULTISET'               | /* Collection */
        'POWERMULTISET_BY_CARDINALITY'| /* Collection */
        'PREDICTION'                  | /* Data Mining */
        'PREDICTION_BOUNDS'           | /* Data Mining */
        'PREDICTION_COST'             | /* Data Mining */
        'PREDICTION_DETAILS'          | /* Data Mining */
        'PREDICTION_PROBABILITY'      | /* Data Mining */
        'PREDICTION_SET'              | /* Data Mining */
        'PRESENTNNV'                  | /* Model */
        'PRESENTV'                    | /* Model */
        'PREVIOUS'                    | /* Model */
        'RANK'                        | /* Aggregate + Analytic */
        'RATIO_TO_REPORT'             | /* Analytic */
        'RAWTOHEX'                    | /* Conversion */
        'RAWTONHEX'                   | /* Conversion */
        'REF'                         | /* Object Reference */
        'REFTOHEX'                    | /* Object Reference */
        'REGEXP_COUNT'                | /* Character */
        'REGEXP_INSTR'                | /* Character */
        'REGEXP_REPLACE'              | /* Character */
        'REGEXP_SUBSTR'               | /* Character */
        'REGR_AVGX'                   | /* Aggregate + Analytic */
        'REGR_AVGY'                   | /* Aggregate + Analytic */
        'REGR_COUNT'                  | /* Aggregate + Analytic */
        'REGR_INTERCEPT'              | /* Aggregate + Analytic */
        'REGR_R2'                     | /* Aggregate + Analytic */
        'REGR_SLOPE'                  | /* Aggregate + Analytic */
        'REGR_SXX'                    | /* Aggregate + Analytic */
        'REGR_SXY'                    | /* Aggregate + Analytic */
        'REGR_SYY'                    | /* Aggregate + Analytic */
        'REMAINDER'                   | /* Numeric */
        //  'REPLACE'                     | /* Character */ Also a KEYWORD 'CREATE OR REPLACE ...'
        'ROUND'                       | /* Datetime + Numeric */
        'ROWIDTOCHAR'                 | /* Conversion */
        'ROWIDTONCHAR'                | /* Conversion */
        'ROW_NUMBER'                  | /* Analytic */
        'RPAD'                        | /* Character */
        'RTRIM'                       | /* Character */
        'SCN_TO_TIMESTAMP'            | /* Conversion */
        'SESSIONTIMEZONE'             | /* Datetime */
        // 'SET'                         | /* Collection */ // Also a KEYWORD 'SET ROLE ...'
        'SIGN'                        | /* Numeric */
        'SIN'                         | /* Numeric */
        'SINH'                        | /* Numeric */
        'SOUNDEX'                     | /* Character */
        'SQRT'                        | /* Numeric */
        'STATS_BINOMIAL_TEST'         | /* Aggregate */
        'STATS_CROSSTAB'              | /* Aggregate */
        'STATS_F_TEST'                | /* Aggregate */
        'STATS_KS_TEST'               | /* Aggregate */
        'STATS_MODE'                  | /* Aggregate */
        'STATS_MW_TEST'               | /* Aggregate */
        'STATS_ONE_WAY_ANOVA'         | /* Aggregate */
        'STATS_T_TEST_INDEP'          | /* Aggregate */
        'STATS_T_TEST_INDEPU'         | /* Aggregate */
        'STATS_T_TEST_ONE'            | /* Aggregate */
        'STATS_T_TEST_PAIRED'         | /* Aggregate */
        'STATS_WSR_TEST'              | /* Aggregate */
        'STDDEV'                      | /* Aggregate + Analytic */
        'STDDEV_POP'                  | /* Aggregate + Analytic */
        'STDDEV_SAMP'                 | /* Aggregate + Analytic */
        'SUBSTR'                      | /* Character */
        'SUM'                         | /* Aggregate + Analytic */
        'SYSDATE'                     | /* Datetime */
        'SYSTIMESTAMP'                | /* Datetime */
        'SYS_CONNECT_BY_PATH'         | /* Hierarchical */
        'SYS_CONTEXT'                 | /* Environment */
        'SYS_DBURIGEN'                | /* XML */
        'SYS_EXTRACT_UTC'             | /* Datetime */
        'SYS_GUID'                    | /* Environment */
        'SYS_TYPEID'                  | /* Environment */
        'SYS_XMLAGG'                  | /* Aggregate + XML */
        'SYS_XMLGEN'                  | /* XML */
        'TAN'                         | /* Numeric */
        'TANH'                        | /* Numeric */
        'TIMESTAMP_TO_SCN'            | /* Conversion */
        'TO_BINARY_DOUBLE'            | /* Conversion */
        'TO_BINARY_FLOAT'             | /* Conversion */
        'TO_BLOB'                     | /* Conversion */
        'TO_CHAR'                     | /* Conversion */
        'TO_CLOB'                     | /* Conversion */
        'TO_DATE'                     | /* Conversion */
        'TO_DSINTERVAL'               | /* Conversion + Datetime */
        'TO_LOB'                      | /* Conversion */
        'TO_MULTI_BYTE'               | /* Conversion */
        'TO_NCHAR'                    | /* Conversion */
        'TO_NCLOB'                    | /* Conversion */
        'TO_NUMBER'                   | /* Conversion */
        'TO_SINGLE_BYTE'              | /* Conversion */
        'TO_TIMESTAMP'                | /* Conversion + Datetime */
        'TO_TIMESTAMP_TZ'             | /* Conversion + Datetime */
        'TO_YMINTERVAL'               | /* Conversion + Datetime */
        'TRANSLATE'                   | /* Character + Conversion */
        'TREAT'                       | /* Character */
        'TRIM'                        | /* Character */
        'TRUNC'                       | /* Datetime + Numeric */
        'TZ_OFFSET'                   | /* Datetime */
        'UID'                         | /* Environment */
        'UNISTR'                      | /* Conversion */
        'UPDATEXML'                   | /* XML */
        'UPPER'                       | /* Character */
        'USER'                        | /* Environment */
        'USERENV'                     | /* Environment */
        'VALUE'                       | /* Object Reference */
        'VARIANCE'                    | /* Aggregate + Analytic */
        'VAR_POP'                     | /* Aggregate + Analytic */
        'VAR_SAMP'                    | /* Aggregate + Analytic */
        'VSIZE'                       | /* Encode/Decode */
        'WIDTH_BUCKET'                | /* Numeric */
        'XMLAGG'                      | /* Aggregate + XML */
        'XMLCAST'                     | /* XML */
        'XMLCDATA'                    | /* XML */
        'XMLCOLATTVAL'                | /* XML */
        'XMLCOMMENT'                  | /* XML */
        'XMLCONCAT'                   | /* XML */
        'XMLDIFF'                     | /* XML */
        'XMLELEMENT'                  | /* XML */
        'XMLEXISTS'                   | /* XML */
        'XMLFOREST'                   | /* XML */
        'XMLISVALID'                  | /* XML */
        'XMLPARSE'                    | /* XML */
        'XMLPATCH'                    | /* XML */
        'XMLPI'                       | /* XML */
        'XMLQUERY'                    | /* XML */
        'XMLROOT'                     | /* XML */
        'XMLSEQUENCE'                 | /* XML */
        'XMLSERIALIZE'                | /* XML */
        'XMLTABLE'                    | /* XML */
        'XMLTRANSFORM'                ; /* XML */
        
FOR_NOTATION
    :
	(   
		a=UNSIGNED_INTEGER { $a->set_type(UNSIGNED_INTEGER); get_tokSource()->enqueueToken($a); advanceInput(); }
        b='..'             { $b->set_type(DOUBLE_PERIOD); get_tokSource()->enqueueToken($b); advanceInput(); }
        (
			c=UNSIGNED_INTEGER { $c->set_type(UNSIGNED_INTEGER); get_tokSource()->enqueueToken($c); advanceInput(); }
        )?
	)
	// All three tokens were already emited (the token of type FOR_NOTATION is put into the hidden channel)
	// { skip(); } // this does not work in C++ target. Order of emitted tokens is garbled
	{ skip(); }//{ $channel=HIDDEN; }
    ;

PERIOD
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
		UNSIGNED_INTEGER ( '.' UNSIGNED_INTEGER? )? ( ('E' | 'e') ('+' | '-')? UNSIGNED_INTEGER )?
    |	
		'.' UNSIGNED_INTEGER ( ('E' | 'e') ('+' | '-')? UNSIGNED_INTEGER )?
    )
    ( 'D' | 'd' | 'f' | 'F')?
    ;

NATIONAL_CHAR_STRING_LIT
    :    ('N' | 'n') '\'' (options{greedy=true;}: ~('\'' | '\r' | '\n' ) | '\'' '\'' | NEWLINE)* '\''
    ;

CHAR_STRING
    :    '\'' (options{greedy=true;}: ~('\'' | '\r' | '\n') | '\'' '\'' | NEWLINE)* '\''
    ;

// Perl-style quoted string, see Oracle SQL reference, chapter String Literals
CHAR_STRING_PERL    : ('N'|'n')? ('q'|'Q') ( QS_ANGLE | QS_BRACE | QS_BRACK | QS_PAREN | QS_OTHER) {$type = CHAR_STRING;};
fragment QUOTE      : '\'' ;
fragment QS_ANGLE   : QUOTE '<' ( options {greedy=false;} : . )* '>' QUOTE ;
fragment QS_BRACE   : QUOTE '{' ( options {greedy=false;} : . )* '}' QUOTE ;
fragment QS_BRACK   : QUOTE '[' ( options {greedy=false;} : . )* ']' QUOTE ;
fragment QS_PAREN   : QUOTE '(' ( options {greedy=false;} : . )* ')' QUOTE ;

fragment QS_OTHER_CH: ~('<'|'{'|'['|'('|' '|'\t'|'\n'|'\r');
fragment QS_OTHER
//    For C target we have to preserve case sensitivity.
// @declarations {
// ANTLR3_UINT32 (*oldLA)(struct ANTLR3_INT_STREAM_struct *, ANTLR3_INT32);
// }
// @init {
// 	oldLA = INPUT->istream->_LA;
//    INPUT->setUcaseLA(INPUT, ANTLR3_FALSE);
// }
		:
		QUOTE delimiter=QS_OTHER_CH
/* JAVA Syntax */
//    ( { input.LT(1) != $delimiter.text.charAt(0) || ( input.LT(1) == $delimiter.text.charAt(0) && input.LT(2) != '\'') }? => . )*
//    ( { input.LT(1) == $delimiter.text.charAt(0) && input.LT(2) == '\'' }? => . ) QUOTE
/* C Syntax */
//		( { LA(1) != $delimiter->getText()->chars[0] || LA(2) != '\'' }? => . )*
//		( { LA(1) == $delimiter->getText()->chars[0] && LA(2) == '\'' }? => . ) QUOTE
//		{ INPUT->istream->_LA = oldLA; }
/* C++ Syntax */
		( { LA(1) != $delimiter->getText().at(0) || LA(2) != '\'' }? => . )*
		( { LA(1) == $delimiter->getText().at(0) && LA(2) == '\'' }? => . ) QUOTE
		;


LLABEL:	        '<<' ;

RLABEL:	        '>>' ;

PERCENT:        '%';

LEFT_PAREN:     '(';

RIGHT_PAREN:    ')';

ASTERISK:       '*';

PLUS_SIGN:      '+';

MINUS_SIGN:     '-';    

COMMA:          ',';

// Match slash '/' and then also try to match some spaces and newline
// If the slash is the only token on the line then it's a SQLPLUS_COMMAND
SOLIDUS
	@init
	{
		ANTLR_UINT32 linePos = getCharPositionInLine();
		ANTLR_UINT32 line = getLine();
		ANTLR_MARKER start;
        bool success;
	}
	: '/'
	{
    this->inc_backtracking();
    start	= this->mark();
    this->mSPACE_FRAGMENT();
    this->mNEWLINE();
    success	= !( this->get_failedflag() );
    this->rewind(start);
    this->dec_backtracking();
    this->set_failedflag(false);
    // TODO theoretically even this is SQLPLUS COMMAND "   /    " - check spaces before slash
    if(success && linePos == 0)
        $type = SQLPLUS_SOLIDUS;
	}
    ;        

AT_SIGN
	@init
	{
		ANTLR_UINT32 linePos = getCharPositionInLine();
		ANTLR_UINT32 line = getLine();
		ANTLR_MARKER start;
        bool success;
	}
	: '@' ('@')?
	{
    this->inc_backtracking();
    start	= this->mark();
    this->mSWALLOW_TO_NEWLINE_FRAGMENT();
    this->mNEWLINE();	    // can never throw exception
    success	= !( this->get_failedflag() );
    this->rewind(start);
    this->dec_backtracking();
    this->set_failedflag(false);
    if(success && linePos == 0)
        $type = SQLPLUS_COMMAND;
	}
    ;

ASSIGN_OP:      ':=';

ARROW:          '=>';

COLON:          ':';

SEMICOLON:      ';';

LESS_THAN_OR_EQUALS_OP:'<=';

LESS_THAN_OP:   '<';

GREATER_THAN_OR_EQUALS_OP:'>=';

NOT_EQUAL_OP
    :    '!='
    |    '<>'
    |    '^='
    |    '~='
    ;

CARRET_OPERATOR_PART:'^';

TILDE_OPERATOR_PART:'~';

EXCLAMATION_OPERATOR_PART:'!';

GREATER_THAN_OP:'>';

CONCATENATION_OP:'||';

VERTICAL_BAR:   '|';

EQUALS_OP:      '=';

// Used for model queries only
LEFT_BRACKET:   '[';

RIGHT_BRACKET:  ']';

fragment
SIMPLE_LETTER
    :    'a'..'z'
    |    'A'..'Z'
    ;

fragment
UNSIGNED_INTEGER
    :    ('0'..'9')+ 
    ;

COMMENT_SL
    :    '--' ( ~('\r' | '\n') )* //(NEWLINE|EOF)
	;
COMMENT_ML
    :    '/*' (options{greedy=false;} : .)* '*/'
    ;

//{ Rule #360 <NEWLINE>
fragment
NEWLINE
    :    '\r' (options{greedy=true;}: '\n')?
    |    '\n'
    ;
//}

fragment
SWALLOW_TO_NEWLINE_FRAGMENT
	:	( ~('\r' | '\n') )*
	;

// Line end. Match NEWLINE and then try to match SQLPLUS_COMMAND_INTRODUCER
// If it is found emit also SPACE and SQLPLUS_COMMAND
LINEEND
	:
(
	n=NEWLINE 
	{
		$n->set_type(NEWLINE);
		get_tokSource()->enqueueToken($n);
		advanceInput();
	}
	(
		s=SPACE
		{
			$s->set_type(SPACE);
			get_tokSource()->enqueueToken($s); 
			advanceInput();
		}
		| // or nothing
	)
	(
		(SQLPLUS_COMMAND_INTRODUCER (NEWLINE|EOF)) => sqlplus=SQLPLUS_COMMAND_INTRODUCER
		{
			$sqlplus->set_type(SQLPLUS_COMMAND);
			get_tokSource()->enqueueToken($sqlplus);
			advanceInput();
		}
		| // or nothing
	)
)
{ skip(); }//{ $channel=HIDDEN; }
	;
	
//{ Rule #522 <SPACE>
//fragment
SPACE
    :    (' ' | '\t')+
    ;
//}

// See SOLIDUS rule
fragment
SPACE_FRAGMENT
    :    (' ' | '\t')*
    ;

fragment
QUESTION_MARK
    :    '?'
    ;
// See OCI reference for more information about this
BINDVAR
    :    COLON SIMPLE_LETTER  ( SIMPLE_LETTER | '0' .. '9' | '_' )*
    |    COLON DELIMITED_ID  // not used in SQL but spotted in v$sqltext when using cursor_sharing
    |    COLON UNSIGNED_INTEGER
    |    QUESTION_MARK // not in SQL, not in Oracle, not in OCI, use this for JDBC
    ;

REGULAR_ID
        :    (SIMPLE_LETTER) (SIMPLE_LETTER | '$' | '_' | '#' | '0'..'9')*
        ;

DELIMITED_ID
    :    '"' (~('"' | '\r' | '\n') | '"' '"')+ '"' 
    ;

// DELIMITED_ID_PART
//     :    '"' (~('"' | '\r' | '\n') | '"' '"')+ (NEWLINE|EOF)
//     ;

// these two rules are use to tocustomlext to highlight partially edited line
// COMMENT_ML_PART
//     :    '/*' (options{greedy=false;} : ~('*/') )* (NEWLINE)
//     ;

// COMMENT_ML_END
//     :    '*/'
//     ;

// Last resort rule matches any character. This lexer should never fail.
TOKEN_FAILURE : . ;
