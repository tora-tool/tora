/**
 * Oracle(c) PL/SQL 11g Parser  
 *
 * Copyright (c) 2009-2011 Alexandre Porcelli <alexandre.porcelli@gmail.com>
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
 */

lexer grammar OracleDMLLexer;

options {
        language=Cpp;
//        filter=true;
}

tokens { // moved to the import vocabulary
    UNSIGNED_INTEGER; // Imaginary token based on subtoken typecasting - see the rule <EXACT_NUM_LIT>
    APPROXIMATE_NUM_LIT; // Imaginary token based on subtoken typecasting - see the rule <EXACT_NUM_LIT>
    MINUS_SIGN; // Imaginary token based on subtoken typecasting - see the rule <SEPARATOR>
    DOUBLE_PERIOD;
    UNDERSCORE; // Imaginary token based on subtoken typecasting - see the rule <INTRODUCER>

    //TORA TOKENS
    T_UNKNOWN;
    T_RESERVED;
    // Identifier subtypes AST leafs
    T_TABLE_ALIAS;
    T_COLUMN_ALIAS;
    T_COLUMN_NAME;
    T_TABLE_NAME;
    T_SCHEMA_NAME;
    T_FUNCTION_NAME;
    T_PACKAGE_NAME;
    T_DBLINK_NAME;
    T_BINDVAR_NAME;

    //Alias type(declaration, usage)
    T_DECL;
    T_USE ;
    
    //Tree nodes
    T_IDENTIFIER;
    T_TABLE_REF;
    T_WITH;
    T_SELECT;
    T_SUBQUERY;
    T_TABLE_CAST;
    T_COLUMN_LIST;
    T_SELECT_COLUMN;
    T_FROM;
    //T_SELECTED_TABLE;
    T_WHERE;
    T_HIERARCHICAL;
    T_GROUP_BY;
    T_MODEL;
    T_UNION;
    T_ORDER_BY_CLAUSE;
    T_FOR_UPDATE_CLAUSE;
    T_JOINING_CLAUSE;

    T_OPERATOR_UNARY;
    T_OPERATOR_BINARY;
    T_OPERATOR_PART;
    T_COND_OR;
    T_COND_OR_SEQ;
    T_COND_AND;
    T_COND_AND_SEQ;
    T_COND_NOT;
    T_COND_EXISTS;
    T_COND_IS;
    T_COND_COMPARISON;
    T_COND_GROUP_COMPARISON;
    T_COND_IN;
    T_COND_IS_A_SET;
    T_COND_IS_ANY;
    T_COND_IS_EMPTY;
    T_COND_IS_OF_TYPE;
    T_COND_IS_PRESENT;
    T_COND_LIKE;
    T_COND_MEMEBER;
    T_COND_BETWEEN;
    T_COND_REGEXP_LIKE;
    T_COND_SUBMULTISET;
    T_COND_EQUALS_PATH;
    T_COND_UNDER_PATH;
    T_COND_PAREN;

    EXPLAIN_STATEMENT;
    SELECT_STATEMENT;
    FACTORING;
    SUBQUERY;
    SELECT_LIST;
    SELECT_ITEM;
    DOT_ASTERISK;
    TABLE_REF;
    TABLE_REF_ELEMENT;
    JOIN_DEF;
    LOGIC_EXPR;
    SELECTED_TABLEVIEW;
    TABLE_EXPRESSION;
    COLLECTION_MODE;
    SELECT_MODE;
    DIRECT_MODE;
    PIVOT_ELEMENT;
    ANY_MODE;
    ELEMENTS_MODE;
    PIVOT_IN_ELEMENT;
    UNPIVOT_IN_ELEMENT;
    HIERARCHICAL;
    GROUP_BY_CLAUSE;
    GROUP_BY_ELEMENT;
    GROUPIN_SET;
    MAIN_MODEL;
    MODEL_COLUMN;
    MODEL_COLUMNS;
    MODEL_RULES;
    MODEL_RULE;
    ASSIGN;
    ORDER_BY_ELEMENTS;
    ORDER_BY_ELEMENT;
    SINGLE_TABLE_MODE;
    MULTI_TABLE_MODE;
    TABLE_ELEMENT;
    CONDITIONAL_INSERT;
    COLUMNS;
    MERGE_UPDATE;
    MERGE_INSERT;
    LOCK_TABLE_ELEMENT;
    STATIC_RETURNING;
    EXPR_LIST;
    IS_NOT_NULL;
    IS_NULL;
    IS_NOT_NAN;
    IS_NAN;
    IS_NOT_PRESENT;
    IS_PRESENT;
    IS_NOT_INFINITE;
    IS_INFINITE;
    IS_NOT_A_SET;
    IS_A_SET;
    IS_NOT_EMPTY;
    IS_EMPTY;
    IS_NOT_OF_TYPE;
    IS_OF_TYPE;
    NOT_IN;
    NOT_BETWEEN;
    NOT_LIKE;
    UNARY_OPERATOR;
    STANDARD_FUNCTION;
    MODEL_EXPRESSION;
    FOR_SINGLE_COLUMN;
    FOR_MULTI_COLUMN;
    SIMPLE_CASE;
    SEARCHED_CASE;
    VECTOR_EXPR;
    FUNCTION_ENABLING_OVER;
    FUNCTION_ENABLING_USING;
    FUNCTION_ENABLING_WITHIN_OR_OVER;
    PREDICTION_FUNCTION;
    ELEMENT;
    XML_COLUMN;
    XML_ALIAS;
    PIVOT_ALIAS;
    DATETIME_OP;
    XML_ELEMENT;
    EXPR;
    ROUTINE_CALL;
    ARGUMENTS;
    ARGUMENT;

// OracleDMLKeys tokens    
    DEBUG_VK;
    REUSE_VK;
    REPLACE_VK;
    DETERMINISTIC_VK;
    RESULT_CACHE_VK;
    PIPELINED_VK;
    AGGREGATE_VK;
    RELIES_ON_VK;
    AUTHID_VK;
    DEFINER_VK;
    CURRENT_USER_VK;
    CLUSTER_VK;
    PACKAGE_VK;
    BODY_VK;
    PARALLEL_ENABLE_VK;
    SPECIFICATION_VK;
    RANGE_VK;
    HASH_VK;
    EXTERNAL_VK;
    CALL_VK;
    DDL_VK;
    ENABLE_VK;
    DATABASE_VK;
    DISABLE_VK;
    BEFORE_VK;
    REFERENCING_VK;
    LOGON_VK;
    AFTER_VK;
    SCHEMA_VK;
    TRUNCATE_VK;
    STARTUP_VK;
    STATISTICS_VK;
    NOAUDIT_VK;
    SUSPEND_VK;
    AUDIT_VK;
    DISASSOCIATE_VK;
    SHUTDOWN_VK;
    COMPOUND_VK;
    SERVERERROR_VK;
    PARENT_VK;
    FOLLOWS_VK;
    NESTED_VK;
    OLD_VK;
    DB_ROLE_CHANGE_VK;
    LOGOFF_VK;
    ANALYZE_VK;
    INSTEAD_VK;
    ASSOCIATE_VK;
    NEW_VK;
    RENAME_VK;
    COMMENT_VK;
    FORCE_VK;
    VALIDATE_VK;
    TYPE_VK;
    COMPILE_VK;
    EXCEPTIONS_VK;
    LIMIT_VK;
    INSTANTIABLE_VK;
    FINAL_VK;
    ADD_VK;
    MODIFY_VK;
    INCLUDING_VK;
    CASCADE_VK;
    INVALIDATE_VK;
    CONVERT_VK;
    LANGUAGE_VK;
    JAVA_VK;
    OVERRIDING_VK;
    C_VK;
    LIBRARY_VK;
    CONTEXT_VK;
    OUT_VK;
    INOUT_VK;
    PARAMETERS_VK;
    AGENT_VK;
    NOCOPY_VK;
    PRAGMA_VK;
    CUSTOMDATUM_VK;
    ORADATA_VK;
    CONSTRUCTOR_VK;
    SQLDATA_VK;
    MEMBER_VK;
    SELF_VK;
    OBJECT_VK;
    STATIC_VK;
    UNDER_VK;
    MAP_VK;
    CONSTANT_VK;
    EXCEPTION_INIT_VK;
    PERCENT_NOTFOUND_VK;
    PERCENT_FOUND_VK;
    PERCENT_ISOPEN_VK;
    PERCENT_ROWCOUNT_VK;
    PERCENT_ROWTYPE_VK;
    PERCENT_TYPE_VK;
    SERIALLY_REUSABLE_VK;
    AUTONOMOUS_TRANSACTION_VK;
    INLINE_VK;
    RESTRICT_REFERENCES_VK;
    EXIT_VK;
    RETURN_VK;
    RAISE_VK;
    LOOP_VK;
    FORALL_VK;
    CONTINUE_VK;
    REVERSE_VK;
    OFF_VK;
    EXECUTE_VK;
    IMMEDIATE_VK;
    COMMIT_VK;
    WORK_VK;
    BULK_VK;
    COMMITTED_VK;
    ISOLATION_VK;
    SERIALIZABLE_VK;
    WRITE_VK;
    WAIT_VK;
    CORRUPT_XID_ALL_VK;
    CORRUPT_XID_VK;
    BATCH_VK;
    DEFERRED_VK;
    ROLLBACK_VK;
    OPEN_VK;
    SAVEPOINT_VK;
    CLOSE_VK;
    READ_VK;
    ONLY_VK;
    REF_VK;
    PLS_INTEGER_VK;
    SUBPARTITION_VK;
    PARTITION_VK;
    TIMESTAMP_TZ_UNCONSTRAINED_VK;
    UROWID_VK;
    POSITIVEN_VK;
    TIMEZONE_ABBR_VK;
    BINARY_DOUBLE_VK;
    BFILE_VK;
    TIMEZONE_REGION_VK;
    TIMESTAMP_LTZ_UNCONSTRAINED_VK;
    NATURALN_VK;
    SIMPLE_INTEGER_VK;
    BYTE_VK;
    BINARY_FLOAT_VK;
    NCLOB_VK;
    CLOB_VK;
    DSINTERVAL_UNCONSTRAINED_VK;
    YMINTERVAL_UNCONSTRAINED_VK;
    ROWID_VK;
    TIMESTAMP_UNCONSTRAINED_VK;
    SIGNTYPE_VK;
    BLOB_VK;
    NVARCHAR2_VK;
    STRING_VK;
    MAXVALUE_VK;
    MINVALUE_VK;
    DBTIMEZONE_VK;
    SESSIONTIMEZONE_VK;
    RAW_VK;
    NUMBER_VK;
    VARCHAR2_VK;
    BOOLEAN_VK;
    POSITIVE_VK;
    MLSLABEL_VK;
    BINARY_INTEGER_VK;
    LONG_VK;
    CHARACTER_VK;
    CHAR_VK;
    VARCHAR_VK;
    NCHAR_VK;
    BIT_VK;
    FLOAT_VK;
    REAL_VK;
    DOUBLE_VK;
    PRECISION_VK;
    TIME_VK;
    TIMESTAMP_VK;
    NUMERIC_VK;
    DECIMAL_VK;
    DEC_VK;
    INTEGER_VK;
    INT_VK;
    SMALLINT_VK;
    NATURAL_VK;
    SECOND_VK;
    TIMEZONE_HOUR_VK;
    TIMEZONE_MINUTE_VK;
    LOCAL_VK;
    YEAR_VK;
    MONTH_VK;
    DAY_VK;
    HOUR_VK;
    MINUTE_VK;
    MERGE_VK;
    REJECT_VK;
    LOG_VK;
    UNLIMITED_VK;
    FIRST_VK;
    NOCYCLE_VK;
    BLOCK_VK;
    XML_VK;
    PIVOT_VK;
    SEQUENTIAL_VK;
    SINGLE_VK;
    SKIP_VK;
    UPDATED_VK;
    EXCLUDE_VK;
    REFERENCE_VK;
    UNTIL_VK;
    SEED_VK;
    SIBLINGS_VK;
    CUBE_VK;
    NULLS_VK;
    DIMENSION_VK;
    SCN_VK;
    UNPIVOT_VK;
    KEEP_VK;
    MEASURES_VK;
    SAMPLE_VK;
    UPSERT_VK;
    VERSIONS_VK;
    RULES_VK;
    ITERATE_VK;
    ROLLUP_VK;
    NAV_VK;
    AUTOMATIC_VK;
    LAST_VK;
    GROUPING_VK;
    INCLUDE_VK;
    IGNORE_VK;
    RESPECT_VK;
    SUBMULTISET_VK;
    LIKEC_VK;
    LIKE2_VK;
    LIKE4_VK;
    ROW_VK;
    SET_VK;
    SOME_VK;
    FULL_VK;
    CROSS_VK;
    LEFT_VK;
    RIGHT_VK;
    INNER_VK;
    VALUE_VK;
    INCREMENT_VK;
    DECREMENT_VK;
    AT_VK;
    DENSE_RANK_VK;
    NAME_VK;
    COLLECT_VK;
    ROWS_VK;
    NCHAR_CS_VK;
    DECOMPOSE_VK;
    FOLLOWING_VK;
    FIRST_VALUE_VK;
    PRECEDING_VK;
    WITHIN_VK;
    CANONICAL_VK;
    COMPATIBILITY_VK;
    OVER_VK;
    LAST_VALUE_VK;
    CURRENT_VK;
    UNBOUNDED_VK;
    COST_VK;
    CHAR_CS_VK;
    AUTO_VK;
    TREAT_VK;
    CONTENT_VK;
    XMLPARSE_VK;
    XMLELEMENT_VK;
    ENTITYESCAPING_VK;
    STANDALONE_VK;
    WELLFORMED_VK;
    XMLEXISTS_VK;
    VERSION_VK;
    XMLCAST_VK;
    YES_VK;
    NO_VK;
    EVALNAME_VK;
    XMLPI_VK;
    XMLCOLATTVAL_VK;
    DOCUMENT_VK;
    XMLFOREST_VK;
    PASSING_VK;
    INDENT_VK;
    HIDE_VK;
    XMLAGG_VK;
    XMLNAMESPACES_VK;
    NOSCHEMACHECK_VK;
    NOENTITYESCAPING_VK;
    XMLQUERY_VK;
    XMLTABLE_VK;
    XMLROOT_VK;
    SCHEMACHECK_VK;
    XMLATTRIBUTES_VK;
    ENCODING_VK;
    SHOW_VK;
    XMLSERIALIZE_VK;
    ORDINALITY_VK;
    DEFAULTS_VK;
    CHR_VK;
    COUNT_VK;
    CAST_VK;
    TRANSLATE_VK;
    TRIM_VK;
    LEADING_VK;
    TRAILING_VK;
    BOTH_VK;
    EXTRACT_VK;
    SEQUENCE_VK;
    NOORDER_VK;
    CYCLE_VK;
    CACHE_VK;
    NOCACHE_VK;
    NOMAXVALUE_VK;
    NOMINVALUE_VK;
    SEARCH_VK;
    DEPTH_VK;
    BREADTH_VK;    

// OracleDMLCommons
    ALIAS;
    EXPR;
    ARGUMENTS;
    ARGUMENT;
    PARAMETER_NAME;
    ATTRIBUTE_NAME;
    SAVEPOINT_NAME;
    ROLLBACK_SEGMENT_NAME;
    TABLE_VAR_NAME;
    SCHEMA_NAME;
    ROUTINE_NAME;
    PACKAGE_NAME;
    IMPLEMENTATION_TYPE_NAME;
    REFERENCE_MODEL_NAME;
    MAIN_MODEL_NAME;
    QUERY_NAME;
    CONSTRAINT_NAME;
    LABEL_NAME;
    TYPE_NAME;
    SEQUENCE_NAME;
    EXCEPTION_NAME;
    FUNCTION_NAME;
    PROCEDURE_NAME;
    TRIGGER_NAME;
    INDEX_NAME;
    CURSOR_NAME;
    RECORD_NAME;
    COLLECTION_NAME;
    LINK_NAME;
    COLUMN_NAME;
    TABLEVIEW_NAME;
    CHAR_SET_NAME;
    ID;
    VARIABLE_NAME;
    HOSTED_VARIABLE_NAME;
    CUSTOM_TYPE;
    NATIVE_DATATYPE;
    INTERVAL_DATATYPE;
    PRECISION;
    CASCATED_ELEMENT;
    HOSTED_VARIABLE_ROUTINE_CALL;
    HOSTED_VARIABLE;
    ROUTINE_CALL;
    ANY_ELEMENT;
    COST_CLASS_NAME;
    XML_COLUMN_NAME;    
}

@lexer::includes
{
	#include "OracleDMLTraits.hpp"
}
@lexer::namespace{ Antlr3BackendImpl }

@header {
/*
 * Oracle(c) PL/SQL 11g Parser  
 *
 * Copyright (c) 2009-2011 Alexandre Porcelli <alexandre.porcelli@gmail.com>
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
 */
}

@members {
//     // buffer (queue) to hold the emit()'d tokens
//     // private LinkedList<Token> tokenBuffer = new LinkedList<Token>();

//     // public void emit(Token t) {
//     //     tokenBuffer.add(t);
//     // }

//     // private void advanceInput(){
//     //     state.tokenStartCharIndex = input.index();
//     //     state.tokenStartCharPositionInLine = input.getCharPositionInLine();
//     //     state.tokenStartLine = input.getLine();
//     // }


    /**
     * Return a token from this source; i.e., match a token on the char stream.
     */
    // public Token nextToken() {
    //     while (true) {
    //         if (tokenBuffer.size() == 0) {
    //             state.token = null;
    //             state.channel = Token.DEFAULT_CHANNEL;
    //             state.tokenStartCharIndex = input.index();
    //             state.tokenStartCharPositionInLine = input.getCharPositionInLine();
    //             state.tokenStartLine = input.getLine();
    //             state.text = null;
    //             if (input.LA(1) == CharStream.EOF) {
    //                 return Token.EOF_TOKEN;
    //             }
    //             try {
    //                 int m = input.mark();
    //                 state.backtracking = 1;
    //                 state.failed = false;
    //                 mTokens();
    //                 state.backtracking = 0;

    //                 if (state.failed) {
    //                     input.rewind(m);
    //                     input.consume();
    //                 } else {
    //                     emit();
    //                 }
    //             } catch (RecognitionException re) {
    //                 // shouldn't happen in backtracking mode, but...
    //                 reportError(re);
    //                 recover(re);
    //             }
    //         } else {
    //             Token result = tokenBuffer.poll();
    //             if (result == Token.SKIP_TOKEN || result.getType() == Token.INVALID_TOKEN_TYPE || result == null)
    //             {
    //                 // discard
    //                 // SKIP & INVALID
    //                 // tokens
    //                 continue;
    //             }
    //             return result;
    //         }
    //     }
    // }
}

FOR_NOTATION
    :    UNSIGNED_INTEGER
        ///{state.type = UNSIGNED_INTEGER; emit(); advanceInput();}
        '..'
        ///{state.type = DOUBLE_PERIOD; emit(); advanceInput();}
        UNSIGNED_INTEGER
        ///{state.type = UNSIGNED_INTEGER; emit(); advanceInput(); $channel=HIDDEN;}
    ;

//{ Rule #358 <NATIONAL_CHAR_STRING_LIT> - subtoken typecast in <REGULAR_ID>, it also incorporates <character_representation>
//  Lowercase 'n' is a usual addition to the standard
NATIONAL_CHAR_STRING_LIT
    :    ('N' | 'n') '\'' (options{greedy=true;}: ~('\'' | '\r' | '\n' ) | '\'' '\'' | NEWLINE)* '\''
    ;
//}

//{ Rule #040 <BIT_STRING_LIT> - subtoken typecast in <REGULAR_ID>
//  Lowercase 'b' is a usual addition to the standard
BIT_STRING_LIT
    :    ('B' | 'b') ('\'' ('0' | '1')* '\'' SEPARATOR? )+
    ;
//}


//{ Rule #284 <HEX_STRING_LIT> - subtoken typecast in <REGULAR_ID>
//  Lowercase 'x' is a usual addition to the standard
HEX_STRING_LIT
    :    ('X' | 'x') ('\'' ('a'..'f' | 'A'..'F' | '0'..'9')* '\'' SEPARATOR? )+ 
    ;
//}

PERIOD
    :    '.' 
    {    if ((char) LA(1) == '.') {
            consume();
            $type = DOUBLE_PERIOD;
        }
    }        
    ;

//{ Rule #238 <EXACT_NUM_LIT> 
//  This rule is a bit tricky - it resolves the ambiguity with <PERIOD> 
//  It als44o incorporates <mantisa> and <exponent> for the <APPROXIMATE_NUM_LIT>
//  Rule #501 <signed_integer> was incorporated directly in the token <APPROXIMATE_NUM_LIT>
//  See also the rule #617 <unsigned_num_lit>
EXACT_NUM_LIT
    : (
            UNSIGNED_INTEGER
            ( '.' UNSIGNED_INTEGER?
            |    {$type = UNSIGNED_INTEGER;}
            ) ( ('E' | 'e') ('+' | '-')? UNSIGNED_INTEGER {$type = APPROXIMATE_NUM_LIT;} )?
    |    '.' UNSIGNED_INTEGER ( ('E' | 'e') ('+' | '-')? UNSIGNED_INTEGER {$type = APPROXIMATE_NUM_LIT;} )?
    )
    ( 'D' | 'd' | 'f' | 'F')?
    ;
//}

//{ Rule #--- <CHAR_STRING> is a base for Rule #065 <char_string_lit> , it incorporates <character_representation>
//  and a superfluous subtoken typecasting of the "QUOTE"
CHAR_STRING
    :    '\'' (options{greedy=true;}: ~('\'' | '\r' | '\n') | '\'' '\'' | NEWLINE)* '\''
    ;
//}

// Perl-style quoted string, see Oracle SQL reference, chapter String Literals
CHAR_STRING_PERL    : ('q'|'Q') ( QS_ANGLE | QS_BRACE | QS_BRACK | QS_PAREN | QS_OTHER) {$type = CHAR_STRING;};
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


//{ Rule #163 <DELIMITED_ID>
DELIMITED_ID
    :    '"' (~('"' | '\r' | '\n') | '"' '"')+ '"' 
    ;
//}

//{ Rule #546 <SQL_SPECIAL_CHAR> was split into single rules
PERCENT
    :    '%'
    ;

AMPERSAND
    :    '&'
    ;

LEFT_PAREN
    :    '('
    ;

RIGHT_PAREN
    :    ')'
    ;

DOUBLE_ASTERISK
    :    '**'
    ;

ASTERISK
    :    '*'
    ;

PLUS_SIGN
    :    '+'
    ;

COMMA
    :    ','
    ;

SOLIDUS
    :    '/'
    ; 

AT_SIGN
    :    '@'
    ;

ASSIGN_OP
    :    ':='
    ;

// See OCI reference for more information about this
BINDVAR
    :    COLON SIMPLE_LETTER  ( SIMPLE_LETTER | '0' .. '9' | '_' )*
    |    COLON DELIMITED_ID  // not used in SQL but spotted in v$sqltext when using cursor_sharing
    |    COLON UNSIGNED_INTEGER
    |    QUESTION_MARK // not in SQL, not in Oracle, not in OCI, use this for JDBC
    ;

COLON
    :    ':'
    ;

SEMICOLON
    :    ';'
    ;

LESS_THAN_OR_EQUALS_OP
    :    '<='
    ;

LESS_THAN_OP
    :    '<'
    ;

GREATER_THAN_OR_EQUALS_OP
    :    '>='
    ;

NOT_EQUAL_OP
    :    '!='
    |    '<>'
    |    '^='
    |    '~='
    ;
CARRET_OPERATOR_PART
    :    '^'
    ;

TILDE_OPERATOR_PART
    :    '~'
    ;

EXCLAMATION_OPERATOR_PART
    :    '!'
    ;

GREATER_THAN_OP
    :    '>'
    ;

fragment
QUESTION_MARK
    :    '?'
    ;

// protected UNDERSCORE : '_' SEPARATOR ; // subtoken typecast within <INTRODUCER>
CONCATENATION_OP
    :    '||'
    ;

VERTICAL_BAR
    :    '|'
    ;

EQUALS_OP
    :    '='
    ;

//{ Rule #532 <SQL_EMBDD_LANGUAGE_CHAR> was split into single rules:
LEFT_BRACKET
    :    '['
    ;

RIGHT_BRACKET
    :    ']'
    ;

//}

//{ Rule #319 <INTRODUCER>
INTRODUCER
    :    '_' (SEPARATOR {$type = UNDERSCORE;})?
    ;

//{ Rule #479 <SEPARATOR>
//  It was originally a protected rule set to be filtered out but the <COMMENT> and <MINUS_SIGN> clashed. 
SEPARATOR
    :    '-' {$type = MINUS_SIGN;}
    |    COMMENT { $channel=HIDDEN; }
    |    (SPACE | NEWLINE)+ { $channel=HIDDEN; }
    ;
//}

//{ Rule #504 <SIMPLE_LETTER> - simple_latin _letter was generalised into SIMPLE_LETTER
//  Unicode is yet to be implemented - see NSF0
fragment
SIMPLE_LETTER
    :    'a'..'z'
    |    'A'..'Z'
    ;
//}

//  Rule #176 <DIGIT> was incorporated by <UNSIGNED_INTEGER> 
//{ Rule #615 <UNSIGNED_INTEGER> - subtoken typecast in <EXACT_NUM_LIT> 
fragment
UNSIGNED_INTEGER
    :    ('0'..'9')+ 
    ;
//}

//{ Rule #097 <COMMENT>
fragment
COMMENT
    :    '--' ( ~('\r' | '\n') )* (NEWLINE|EOF)
    |    '/*' (options{greedy=false;} : .)* '*/'
    ;
//}

// SQL*Plus prompt
// TODO should be grammar rule, but tricky to implement
PROMPT
	: 'PROMPT' SPACE ( ~('\r' | '\n') )* (NEWLINE|EOF)
	;
//}



//{ Rule #360 <NEWLINE>
fragment
NEWLINE
    :    '\r' (options{greedy=true;}: '\n')?
    |    '\n'
    ;
//}

//{ Rule #522 <SPACE>
fragment
SPACE    :    ' '
    |    '\t'
    ;
//}

fragment APPROXIMATE_NUM_LIT: ;
fragment MINUS_SIGN: ;    
fragment UNDERSCORE: ;
fragment DOUBLE_PERIOD: ;

//{ Rule #442 <REGULAR_ID> additionally encapsulates a few STRING_LITs.
//  Within testLiterals all reserved and non-reserved words are being resolved

SQL92_RESERVED_ALL
    :    'ALL'
    ;

SQL92_RESERVED_ALTER
    :    'ALTER'
    ;

SQL92_RESERVED_AND
    :    'AND'
    ;

SQL92_RESERVED_ANY
    :    'ANY'
    ;

SQL92_RESERVED_AS
    :    'AS'
    ;

SQL92_RESERVED_ASC
    :    'ASC'
    ;

//SQL92_RESERVED_AT
//    :    'AT'
//    ;

SQL92_RESERVED_BEGIN
    :    'BEGIN'
    ;

SQL92_RESERVED_BETWEEN
    :    'BETWEEN'
    ;

SQL92_RESERVED_BY
    :    'BY'
    ;

SQL92_RESERVED_CASE
    :    'CASE'
    ;

SQL92_RESERVED_CHECK
    :    'CHECK'
    ;

PLSQL_RESERVED_CLUSTERS
    :    'CLUSTERS'
    ;

PLSQL_RESERVED_COLAUTH
    :    'COLAUTH'
    ;

PLSQL_RESERVED_COMPRESS
    :    'COMPRESS'
    ;

SQL92_RESERVED_CONNECT
    :    'CONNECT'
    ;

//PLSQL_NON_RESERVED_COLUMNS
//    :    'COLUMNS'
//    ;

PLSQL_NON_RESERVED_CONNECT_BY_ROOT
    :    'CONNECT_BY_ROOT'
    ;

PLSQL_RESERVED_CRASH
    :    'CRASH'
    ;

SQL92_RESERVED_CREATE
    :    'CREATE'
    ;

SQL92_RESERVED_CURRENT
    :    'CURRENT'
    ;

SQL92_RESERVED_CURSOR
    :    'CURSOR'
    ;

SQL92_RESERVED_DATE
    :    'DATE'
    ;

SQL92_RESERVED_DECLARE
    :    'DECLARE'
    ;

SQL92_RESERVED_DEFAULT
    :    'DEFAULT'
    ;

SQL92_RESERVED_DELETE
    :    'DELETE'
    ;

SQL92_RESERVED_DESC
    :    'DESC'
    ;

SQL92_RESERVED_DISTINCT
    :    'DISTINCT'
    ;

SQL92_RESERVED_DROP
    :    'DROP'
    ;

SQL92_RESERVED_ELSE
    :    'ELSE'
    ;

SQL92_RESERVED_END
    :    'END'
    ;

SQL92_RESERVED_EXCEPTION
//@declarations {
//    ANTLR3_MARKER excStart; // ANTLR should declare it for us, but it does not
//}
    :    'EXCEPTION'
    // "exception" is a keyword only withing the contex of the PL/SQL language
    // while it can be an identifier(column name, table name) in SQL
    // "exception" is a keyword if and only it is followed by "when"
    //{
    //        $type = SQL92_RESERVED_EXCEPTION;
    //        excStart = MARK();
    // exc->set_type(exc, SQL92_RESERVED_EXCEPTION);
    // // emit(exc);
    // // advanceInput();

    // $type = Token.INVALID_TOKEN_TYPE;
    // int markModel = input.mark();

    // // Now loop over next Tokens in the input and eventually set Token's type to REGULAR_ID

    // // Subclassed version will return NULL unless EOF is reached.
    // // nextToken either returns NULL => then the next token is put into the queue tokenBuffer
    // // or it returns Token.EOF, then nothing is put into the queue
    // Token t1 = super.nextToken();
    // {    // This "if" handles the situation when the "model" is the last text in the input.
    //     if( t1 != null && t1.getType() == Token.EOF)
    //     {
    //          $e.set_type(REGULAR_ID);
    //     } else {
    //          t1 = tokenBuffer.pollLast(); // "withdraw" the next token from the queue
    //          while(true)
    //          {
    //              if(t1.getType() == EOF)   // is it EOF?
    //              {
    //                  $e.set_type(REGULAR_ID);
    //                  break;
    //              }

    //              if(t1.getChannel() == HIDDEN) // is it a white space? then advance to the next token
    //              {
    //                  t1 = super.nextToken(); if( t1 == null) { t1 = tokenBuffer.pollLast(); };
    //                  continue;
    //              }

    //              if( t1.getType() != SQL92_RESERVED_WHEN && t1.getType() != SEMICOLON) // is something other than "when"
    //              {
    //                  $e.set_type(REGULAR_ID);
    //                  break;
    //              }

    //              break; // we are in the model_clase do not rewrite anything
    //           } // while true
    //      } // else if( t1 != null && t1.getType() == Token.EOF)
    // }
    // input.rewind(markModel);
    //        REWIND(excStart);
    //}
    ;

PLSQL_RESERVED_EXCLUSIVE
    :    'EXCLUSIVE'
    ;

SQL92_RESERVED_EXISTS
    :    'EXISTS'
    ;

SQL92_RESERVED_FALSE
    :    'FALSE'
    ;

SQL92_RESERVED_FETCH
    :    'FETCH'
    ;

SQL92_RESERVED_FOR
    :    'FOR'
    ;

SQL92_RESERVED_FROM
    :    'FROM'
    ;

SQL92_RESERVED_GOTO
    :    'GOTO'
    ;

SQL92_RESERVED_GRANT
    :    'GRANT'
    ;

SQL92_RESERVED_GROUP
    :    'GROUP'
    ;

SQL92_RESERVED_HAVING
    :    'HAVING'
    ;

PLSQL_RESERVED_IDENTIFIED
    :    'IDENTIFIED'
    ;

PLSQL_RESERVED_IF
    :    'IF'
    ;

SQL92_RESERVED_IN
    :    'IN'
    ;

PLSQL_RESERVED_INDEX
    :    'INDEX'
    ;

PLSQL_RESERVED_INDEXES
    :    'INDEXES'
    ;

SQL92_RESERVED_INSERT
    :    'INSERT'
    ;

SQL92_RESERVED_INTERSECT
    :    'INTERSECT'
    ;

SQL92_RESERVED_INTO
    :    'INTO'
    ;

SQL92_RESERVED_IS
    :    'IS'
    ;

SQL92_RESERVED_LIKE
    :    'LIKE'
    ;

PLSQL_RESERVED_LOCK
    :    'LOCK'
    ;

PLSQL_RESERVED_MINUS
    :    'MINUS'
    ;

PLSQL_RESERVED_MODE
    :    'MODE'
    ;

PLSQL_RESERVED_NOCOMPRESS
    :    'NOCOMPRESS'
    ;

SQL92_RESERVED_NOT
    :    'NOT'
    ;

PLSQL_RESERVED_NOWAIT
    :    'NOWAIT'
    ;

SQL92_RESERVED_NULL
    :    'NULL'
    ;

SQL92_RESERVED_OF
    :    'OF'
    ;

SQL92_RESERVED_ON
    :    'ON'
    ;

SQL92_RESERVED_OPTION
    :    'OPTION'
    ;

SQL92_RESERVED_OR
    :    'OR'
    ;

SQL92_RESERVED_ORDER
    :    'ORDER'
    ;

SQL92_RESERVED_OVERLAPS
    :    'OVERLAPS'
    ;

SQL92_RESERVED_PRIOR
    :    'PRIOR'
    ;

SQL92_RESERVED_PROCEDURE
    :    'PROCEDURE'
    ;

SQL92_RESERVED_PUBLIC
    :    'PUBLIC'
    ;

PLSQL_RESERVED_RESOURCE
    :    'RESOURCE'
    ;

SQL92_RESERVED_REVOKE
    :    'REVOKE'
    ;

SQL92_RESERVED_SELECT
    :    'SELECT'
    ;

PLSQL_RESERVED_SHARE
    :    'SHARE'
    ;

SQL92_RESERVED_SIZE
    :    'SIZE'
    ;

// SQL92_RESERVED_SQL
//     :    'SQL'
//     ;

PLSQL_RESERVED_START
    :    'START'
    ;

PLSQL_RESERVED_TABAUTH
    :    'TABAUTH'
    ;

SQL92_RESERVED_TABLE
    :    'TABLE'
    ;

SQL92_RESERVED_THE
    :    'THE'
    ;

SQL92_RESERVED_THEN
    :    'THEN'
    ;

SQL92_RESERVED_TO
    :    'TO'
    ;

SQL92_RESERVED_TRUE
    :    'TRUE'
    ;

SQL92_RESERVED_UNION
    :    'UNION'
    ;

SQL92_RESERVED_UNIQUE
    :    'UNIQUE'
    ;

SQL92_RESERVED_UPDATE
    :    'UPDATE'
    ;

SQL92_RESERVED_VALUES
    :    'VALUES'
    ;

SQL92_RESERVED_VIEW
    :    'VIEW'
    ;

PLSQL_RESERVED_VIEWS
    :    'VIEWS'
    ;

SQL92_RESERVED_WHEN
    :    'WHEN'
    ;

SQL92_RESERVED_WHERE
    :    'WHERE'
    ;

SQL92_RESERVED_WITH
    :    'WITH'
    ;
PLSQL_NON_RESERVED_CAST
    :    'CAST'
    ;

PLSQL_NON_RESERVED_MULTISET
       :       'MULTISET'
       ;

PLSQL_NON_RESERVED_USING
    :    'USING'
    ;

PLSQL_NON_RESERVED_MODEL
    :    'MODEL'
    {
         // "MODEL" is a keyword if and only if it is followed by ("main"|"partition"|"dimension")
         // otherwise it is a identifier(REGULAR_ID).
         // This wodoo implements something like context sensitive lexer.
         // Here we've matched the word "model". Then the Token is created and en-queued in tokenBuffer
         // We still remember the reference(m) onto this Token
         $type = PLSQL_NON_RESERVED_MODEL;
         // emit(m);
         // advanceInput();

         // $type = Token.INVALID_TOKEN_TYPE;
         // int markModel = input.mark();

         // // Now loop over next Tokens in the input and eventually set Token's type to REGULAR_ID

         // // Subclassed version will return NULL unless EOF is reached.
         // // nextToken either returns NULL => then the next token is put into the queue tokenBuffer
         // // or it returns Token.EOF, then nothing is put into the queue
         // Token t1 = super.nextToken();
         // {    // This "if" handles the situation when the "model" is the last text in the input.
         //      if( t1 != null && t1.getType() == Token.EOF)
         //      {
         //          $m.set_type(REGULAR_ID);
         //      } else {
         //          t1 = tokenBuffer.pollLast(); // "withdraw" the next token from the queue
         //          while(true)
         //          {
         //             if(t1.getType() == EOF)   // is it EOF?
         //             {
         //                 $m.set_type(REGULAR_ID);
         //                 break;
         //             }

         //             if(t1.getChannel() == HIDDEN) // is it a white space? then advance to the next token
         //             {
         //                 t1 = super.nextToken(); if( t1 == null) { t1 = tokenBuffer.pollLast(); };
         //                 continue;
         //             }

         //             if( t1.getType() != REGULAR_ID || // is something other than ("main"|"partition"|"dimension")
         //                ( !t1.getText().equalsIgnoreCase("main") &&
         //                  !t1.getText().equalsIgnoreCase("partition") &&
         //                  !t1.getText().equalsIgnoreCase("dimension")
         //               ))
         //             {
         //                 $m.set_type(REGULAR_ID);
         //                 break;
         //             }

         //             break; // we are in the model_clase do not rewrite anything
         //          } // while true
         //      } // else if( t1 != null && t1.getType() == Token.EOF)
         // }
         // input.rewind(markModel);
    }
    ;

PLSQL_NON_RESERVED_ELSIF
    :    'ELSIF'
    ;

PLSQL_NON_RESERVED_PIVOT
    :    'PIVOT'
    ;

PLSQL_NON_RESERVED_UNPIVOT
    :    'UNPIVOT'
    ;

REGULAR_ID
    :    (SIMPLE_LETTER) (SIMPLE_LETTER | '$' | '_' | '#' | '0'..'9')*
    ;

ZV
    :    '@!' {$channel=HIDDEN;}
    ;

// disambiguate these
