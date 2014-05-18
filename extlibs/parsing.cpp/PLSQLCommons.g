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
parser grammar PLSQLCommons;

@includes
{
        #include "s004Traits.hpp"
}

@namespace{ Antlr3Test }

// $<Common SQL PL/SQL Clauses/Parts

partition_extension_clause
    :    ( subpartition_key | partition_key ) 
        for_key? expression_list
    ;

column_alias
options
{
backtrack=true;
}
    :    as_key? ( id | alias_quoted_string )
    |    as_key
    ;

table_alias
    :    ( id | alias_quoted_string )
    ;

alias_quoted_string
    :    quoted_string
    ;

where_clause
    :    where_key (current_of_clause|condition_wrapper)
    ;

current_of_clause
    :    current_key of_key cursor_name
    ;

into_clause
    :    into_key variable_name (COMMA variable_name)* 
    |    bulk_key collect_key into_key variable_name (COMMA variable_name)* 
    ;

// $>

// $<Common PL/SQL Named Elements

xml_column_name
    :    id
    |    quoted_string
    ;

cost_class_name
    :    id
    ;

attribute_name
    :    id
    ;

savepoint_name
    :    id
    ;

rollback_segment_name
    :    id
    ;


table_var_name
    :    id
    ;

schema_name
    :    id
    ;

routine_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)* (AT_SIGN link_name)?
    ;

package_name
    :    id
    ;

implementation_type_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)?
    ;

parameter_name
    :    id
    ;

reference_model_name
    :    id
    ;

main_model_name
    :    id
    ;

aggregate_function_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)*
    ;

query_name
    :    id
    ;

constraint_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)* (AT_SIGN link_name)?
    ;

label_name
    :    id_expression
    ;

type_name
    :    id_expression ((PERIOD id_expression)=> PERIOD id_expression)*
    ;

sequence_name
    :    id_expression ((PERIOD id_expression)=> PERIOD id_expression)*
    ;

exception_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)* 
    ;

function_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)?
    ;

procedure_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)?
    ;

trigger_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)?
    ;

variable_name
    :    (INTRODUCER char_set_name)?
            id_expression ((PERIOD id_expression)=> PERIOD id_expression)?
    |    bind_variable
    ;

index_name
    :    id
    ;

cursor_name
    :    id
    |    bind_variable
    ;

record_name
    :    id
    |    bind_variable
    ;

collection_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)?
    ;

link_name
    :    id
    ;

column_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)*
    ;

tableview_name
    :    id ((PERIOD id_expression)=> PERIOD id_expression)? 
    (    AT_SIGN link_name
    |    {!(LA(2) == SQL92_RESERVED_BY)}?=> partition_extension_clause
    )?
    ;

char_set_name
    :    id_expression ((PERIOD id_expression)=> PERIOD id_expression)*
    ;

// $>

// $<Common PL/SQL Specs

// NOTE: In reality this applies to aggregate functions only
keep_clause
    :   keep_key
        LEFT_PAREN
            dense_rank_key (first_key|last_key)
             order_by_clause
        RIGHT_PAREN over_clause?
    ;

function_argument
    :    LEFT_PAREN 
            argument? (COMMA argument )* 
        RIGHT_PAREN
        keep_clause?
    ;

function_argument_analytic
    :    LEFT_PAREN
            (argument respect_or_ignore_nulls?)?
            (COMMA argument respect_or_ignore_nulls? )*
         RIGHT_PAREN
         keep_clause?
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
    ;

respect_or_ignore_nulls
    :    (respect_key | ignore_key) nulls_key
    ;

argument
    :    ((id EQUALS_OP GREATER_THAN_OP)=> id EQUALS_OP GREATER_THAN_OP)? expression_wrapper
    ;

type_spec
    :     datatype
    |    ref_key? type_name (percent_rowtype_key|percent_type_key)?
    ;

datatype
    :    native_datatype_element
        precision_part?
        (with_key local_key? time_key zone_key)?
    |    interval_key (year_key|day_key)
                (LEFT_PAREN expression_wrapper RIGHT_PAREN)? 
            to_key (month_key|second_key) 
                (LEFT_PAREN expression_wrapper RIGHT_PAREN)?
    ;

precision_part
    :    LEFT_PAREN numeric (COMMA numeric)? (char_key | byte_key)? RIGHT_PAREN
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
    :    ( BINDVAR | COLON UNSIGNED_INTEGER)
         ( indicator_key? (BINDVAR | COLON UNSIGNED_INTEGER))?
         ((PERIOD general_element_part)=> PERIOD general_element_part)*
    ;

general_element
    :    general_element_part ((PERIOD general_element_part)=> PERIOD general_element_part)*
    ;

general_element_part
    :    (INTRODUCER char_set_name)? id_expression
            ((PERIOD id_expression)=> PERIOD id_expression)* function_argument?
    ;

table_element
    :    (INTRODUCER char_set_name)? id_expression (PERIOD id_expression)*
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

id
    :    (INTRODUCER char_set_name)?
        id_expression
    ;

id_expression
    :    REGULAR_ID
    |    DELIMITED_ID
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
