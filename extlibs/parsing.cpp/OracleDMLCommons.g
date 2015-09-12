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

parser grammar OracleDMLCommons;

options {
    output=AST;
}

tokens {
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

@includes
{
        #include "UserTraits.hpp"
}

@namespace{ Antlr3BackendImpl }

// $<Common SQL PL/SQL Clauses/Parts

partition_extension_clause
    :    ( subpartition_key^ | partition_key^ ) 
        for_key!? expression_list
    ;

column_alias
options
{
backtrack=true;
}
    :    as_key? (id[T_UNKNOWN,T_USE]|alias_quoted_string[T_UNKNOWN,T_USE])
    ->    ^(ALIAS id? alias_quoted_string?)
    |    as_key
    ;

table_alias
    :    ( id[1,1] | alias_quoted_string[1,1] )
    ->   ^(ALIAS id? alias_quoted_string?)
    ;

alias_quoted_string[int identifierClass, int usageType]
    :    quoted_string
        -> ID[$quoted_string.start]
    ;

where_clause
    :    where_key^ (current_of_clause|condition_wrapper)
    ;

current_of_clause
    :    current_key^ of_key! cursor_name
    ;

into_clause
    :    into_key^ variable_name (COMMA! variable_name)* 
    |    bulk_key^ collect_key! into_key! variable_name (COMMA! variable_name)* 
    ;

// $>

// $<Common PL/SQL Named Elements

xml_column_name
    :    id[T_UNKNOWN,T_USE] -> ^(XML_COLUMN_NAME id)
    |    quoted_string -> ^(XML_COLUMN_NAME ID[$quoted_string.start])
    ;

cost_class_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(COST_CLASS_NAME id)
    ;

attribute_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(ATTRIBUTE_NAME id)
    ;

savepoint_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(SAVEPOINT_NAME id)
    ;

rollback_segment_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(ROLLBACK_SEGMENT_NAME id)
    ;


table_var_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(TABLE_VAR_NAME id)
    ;

schema_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(SCHEMA_NAME id)
    ;

routine_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])* (AT_SIGN link_name)?
        -> ^(ROUTINE_NAME id id_expression* link_name?)
    ;

package_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(PACKAGE_NAME id)
    ;

implementation_type_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])?
        -> ^(IMPLEMENTATION_TYPE_NAME id id_expression?)
    ;

parameter_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(PARAMETER_NAME id)
    ;

reference_model_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(REFERENCE_MODEL_NAME id)
    ;

main_model_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(MAIN_MODEL_NAME id)
    ;

aggregate_function_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])*
        -> ^(ROUTINE_NAME id id_expression*)
    ;

query_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(QUERY_NAME id)
    ;

constraint_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])* (AT_SIGN link_name)?
        -> ^(CONSTRAINT_NAME id id_expression* link_name?)
    ;

label_name
    :    id_expression[T_UNKNOWN,T_USE]
        -> ^(LABEL_NAME id_expression)
    ;

type_name
    :    id_expression[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])*
        -> ^(TYPE_NAME id_expression+)
    ;

sequence_name
    :    id_expression[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])*
        -> ^(SEQUENCE_NAME id_expression+)
    ;

exception_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])* 
        ->^(EXCEPTION_NAME id id_expression*)
    ;

function_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])?
        -> ^(FUNCTION_NAME id id_expression*)
    ;

procedure_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])?
        -> ^(PROCEDURE_NAME id id_expression*)
    ;

trigger_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])?
        ->^(TRIGGER_NAME id id_expression*)
    ;

variable_name
    :    (INTRODUCER char_set_name)?
            id_expression[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])?
        -> ^(VARIABLE_NAME char_set_name? id_expression*)
    |    bind_variable
        -> ^(HOSTED_VARIABLE_NAME bind_variable)
    ;

index_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(INDEX_NAME id)
    ;

cursor_name
    :    (id[T_UNKNOWN,T_USE] | bind_variable)
        -> ^(CURSOR_NAME id? bind_variable?)
    ;

record_name
    :    (id[T_UNKNOWN,T_USE] | bind_variable)
        ->^(RECORD_NAME id)
    ;

collection_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])?
        ->^(COLLECTION_NAME id id_expression?)
    ;

link_name
    :    id[T_UNKNOWN,T_USE]
        -> ^(LINK_NAME id)
    ;

column_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])*
        -> ^(COLUMN_NAME id id_expression*)
    ;

tableview_name
    :    id[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])? 
    (    AT_SIGN link_name
    |    {!(LA(2) == SQL92_RESERVED_BY)}?=> partition_extension_clause
    )?
        -> ^(TABLEVIEW_NAME id id_expression? link_name? partition_extension_clause?)
    ;

char_set_name
    :    id_expression[T_UNKNOWN,T_USE] ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])*
        ->^(CHAR_SET_NAME id_expression+)
    ;

// $>

// $<Common PL/SQL Specs

// NOTE: In reality this applies to aggregate functions only
keep_clause
    :   keep_key^
        LEFT_PAREN!
            dense_rank_key (first_key|last_key)
             order_by_clause
        RIGHT_PAREN! over_clause?
    ;

function_argument
    :    LEFT_PAREN 
            argument? (COMMA argument )* 
        RIGHT_PAREN
        keep_clause?
        -> ^(ARGUMENTS argument* keep_clause?)
    ;

function_argument_analytic
    :    LEFT_PAREN
            (argument respect_or_ignore_nulls?)?
            (COMMA argument respect_or_ignore_nulls? )*
         RIGHT_PAREN
         keep_clause?
         -> ^(ARGUMENTS argument* keep_clause?)
    ;

function_argument_modeling
    :    LEFT_PAREN
            column_name (COMMA (numeric|null_key) (COMMA (numeric|null_key) )? )?
            using_key
                ( (tableview_name PERIOD ASTERISK)=> tableview_name PERIOD ASTERISK
                | ASTERISK
                | expression column_alias? (COMMA expression column_alias?)*
                )
         RIGHT_PAREN
         keep_clause?
         -> ^(ARGUMENTS column_name keep_clause?)
    ;

respect_or_ignore_nulls
    :    (respect_key | ignore_key) nulls_key
    ;

argument
@init    {    int mode = 0;    }
    :    ((id[T_UNKNOWN,T_USE] EQUALS_OP GREATER_THAN_OP)=> id[T_UNKNOWN,T_USE] EQUALS_OP GREATER_THAN_OP {mode = 1;})? expression_wrapper
        ->{mode == 1}? ^(ARGUMENT expression_wrapper ^(PARAMETER_NAME[$EQUALS_OP] id))
        -> ^(ARGUMENT expression_wrapper)
    ;

type_spec
    :     datatype
    |    ref_key? type_name (percent_rowtype_key|percent_type_key)? -> ^(CUSTOM_TYPE type_name ref_key? percent_rowtype_key? percent_type_key?)
    ;

datatype
    :    native_datatype_element
        precision_part?
        (with_key local_key? time_key zone_key)?
        -> ^(NATIVE_DATATYPE native_datatype_element precision_part? time_key? local_key?)
    |    interval_key (year_key|day_key)
                (LEFT_PAREN expression_wrapper RIGHT_PAREN)? 
            to_key (month_key|second_key) 
                (LEFT_PAREN expression_wrapper RIGHT_PAREN)?
        -> ^(INTERVAL_DATATYPE[$interval_key.start] year_key? day_key? month_key? second_key? expression_wrapper*)
    ;

precision_part
    :    LEFT_PAREN numeric (COMMA numeric)? (char_key | byte_key)? RIGHT_PAREN
        -> ^(PRECISION numeric+ char_key? byte_key?)
    ;

native_datatype_element
    :    binary_integer_key
    |    pls_integer_key
    |    natural_key
    |    binary_float_key
    |    binary_double_key
    |    naturaln_key
    |    positive_key
    |    positiven_key
    |    signtype_key
    |    simple_integer_key
    |    nvarchar2_key
    |    dec_key
    |    integer_key
    |    int_key
    |    numeric_key
    |    smallint_key
    |    number_key
    |    decimal_key 
    |    double_key precision_key?
    |    float_key
    |    real_key
    |    nchar_key
    |    long_key raw_key?
    |    char_key  
    |    character_key 
    |    varchar2_key
    |    varchar_key
    |    string_key
    |    raw_key
    |    boolean_key
    |    date_key
    |    rowid_key
    |    urowid_key
    |    year_key
    |    month_key
    |    day_key
    |    hour_key
    |    minute_key
    |    second_key
    |    timezone_hour_key
    |    timezone_minute_key
    |    timezone_region_key
    |    timezone_abbr_key
    |    timestamp_key
    |    timestamp_unconstrained_key
    |    timestamp_tz_unconstrained_key
    |    timestamp_ltz_unconstrained_key
    |    yminterval_unconstrained_key
    |    dsinterval_unconstrained_key
    |    bfile_key
    |    blob_key
    |    clob_key
    |    nclob_key
    |    mlslabel_key
    ;

bind_variable
    :    ( b1=BINDVAR | COLON u1=UNSIGNED_INTEGER)
         ( indicator_key? (b2=BINDVAR | COLON u2=UNSIGNED_INTEGER))?
         ((PERIOD general_element_part)=> PERIOD general_element_part)*
         ->^(HOSTED_VARIABLE_NAME $b1? $u1? indicator_key? $b2? $u2? general_element_part*)
    ;

general_element
@init    { int isCascated = true; }
    :    general_element_part ((PERIOD general_element_part)=> PERIOD general_element_part {isCascated = true;})*
        ->{isCascated}? ^(CASCATED_ELEMENT general_element_part+)
        -> general_element_part
    ;

general_element_part
@init    { int isRoutineCall = false; }
    :    (INTRODUCER char_set_name)? id_expression[T_UNKNOWN,T_USE]
            ((PERIOD id_expression[T_UNKNOWN,T_USE])=> PERIOD id_expression[T_UNKNOWN,T_USE])* (function_argument {isRoutineCall = true;})?
        ->{isRoutineCall}? ^(ROUTINE_CALL ^(ROUTINE_NAME char_set_name? id_expression+) function_argument)
        -> ^(ANY_ELEMENT char_set_name? id_expression+)
    ;

table_element
    :    (INTRODUCER char_set_name)? id_expression[T_UNKNOWN,T_USE] (PERIOD id_expression[T_UNKNOWN,T_USE])*
         -> ^(ANY_ELEMENT char_set_name? id_expression+)
    ;

// $>

// $<Lexer Mappings

constant
    :    timestamp_key (quoted_string | bind_variable) (at_key time_key zone_key quoted_string)?
    |    interval_key (quoted_string | bind_variable | general_element_part)
         ( day_key | hour_key | minute_key | second_key)
         ( LEFT_PAREN (UNSIGNED_INTEGER | bind_variable) (COMMA (UNSIGNED_INTEGER | bind_variable) )? RIGHT_PAREN)?
         ( to_key
             ( day_key | hour_key | minute_key | second_key (LEFT_PAREN (UNSIGNED_INTEGER | bind_variable) RIGHT_PAREN)? )
         )?
    |    numeric
    |    date_key quoted_string
    |    quoted_string
    |    null_key
    |    true_key
    |    false_key
    |    dbtimezone_key 
    |    sessiontimezone_key
    |    minvalue_key
    |    maxvalue_key
    |    default_key
    ;

numeric
    :    UNSIGNED_INTEGER
    |    EXACT_NUM_LIT
    |    APPROXIMATE_NUM_LIT;

quoted_string
    :    CHAR_STRING
    |    CHAR_STRING_PERL
    |    NATIONAL_CHAR_STRING_LIT
    ;

id[int identifierClass, int usageType]
    :    (INTRODUCER char_set_name)?
        r=id_expression[identifierClass, usageType] { int i = r->get_type(); }
        //-> char_set_name? id_expression
    ;

id_expression[int identifierClass, int usageType]
//    :    REGULAR_ID ->    ID[$REGULAR_ID] {}
    :    r=REGULAR_ID { const_cast<CommonTokenType*>($r)->set_type(ID); const_cast<CommonTokenType*>($r)->UserData.identifierClass = identifierClass; }
    |    DELIMITED_ID ->    ID[$DELIMITED_ID] 
    ;

not_equal_op
    :    NOT_EQUAL_OP
    |    LESS_THAN_OP GREATER_THAN_OP
    |    EXCLAMATION_OPERATOR_PART EQUALS_OP
    |    CARRET_OPERATOR_PART EQUALS_OP
    ;

greater_than_or_equals_op
    :    GREATER_THAN_OR_EQUALS_OP
    |    GREATER_THAN_OP EQUALS_OP
    ;

less_than_or_equals_op
    :    LESS_THAN_OR_EQUALS_OP
    |    LESS_THAN_OP EQUALS_OP
    ;

concatenation_op
    :    CONCATENATION_OP
    |    VERTICAL_BAR VERTICAL_BAR
    ;

multiset_op
    :    multiset_key
         ( except_key | intersect_key | union_key )
         ( all_key | distinct_key )?
    ;

outer_join_sign
    :    LEFT_PAREN PLUS_SIGN RIGHT_PAREN
    ;

// $>
