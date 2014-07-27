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
parser grammar PLSQL_DMLParser;

// options {
//         tokenVocab=PLSQLLexer;
//         language=Cpp;
// }

//import PLSQLKeys, PLSQLCommons;

@parser::includes
{
	#include "UserTraits.hpp"
	#include "PLSQLLexer.hpp"
}
@parser::namespace { Antlr3BackendImpl }

//SHOULD BE OVERRIDEN!
compilation_unit
    :     seq_of_statements* EOF
    ;

//SHOULD BE OVERRIDEN!
seq_of_statements 
    :
    (    select_statement
    |    update_statement
    |    delete_statement
    |    insert_statement
    |    lock_table_statement
    |    merge_statement
    |    explain_statement
    )
    (    SEMICOLON | EOF )
    ;

explain_statement
    :    explain_key plan_key
         (set_key statement_id_key EQUALS_OP quoted_string)?
         (into_key tableview_name)?
         for_key
         ( select_statement
         | update_statement
         | delete_statement
         | insert_statement
         | merge_statement
         )
    ;

select_statement
    :    subquery_factoring_clause?
        subquery
        (for_update_clause|(order_key siblings_key? by_key)=> order_by_clause)*
    ;

// $<Select - Specific Clauses
subquery_factoring_clause
    :    with_key factoring_element (COMMA factoring_element)*
    ;

factoring_element
    :    query_name (LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN)? as_key LEFT_PAREN subquery order_by_clause? RIGHT_PAREN
         search_clause?
         cycle_clause?
    ;

search_clause
    :    search_key ( depth_key | breadth_key ) first_key by_key
             column_name asc_key ? desc_key ? (nulls_key first_key)? (nulls_key last_key)?
             (COMMA column_name asc_key ? desc_key ? (nulls_key first_key)? (nulls_key last_key)? )*
             set_key column_name
    ;

cycle_clause
    :    cycle_key column_name ( COMMA column_name)* set_key column_name to_key expression default_key expression
    ;

subquery
    :    subquery_basic_elements subquery_operation_part*
    ;

subquery_operation_part
    :    (union_key all_key?|intersect_key|minus_key) subquery_basic_elements
    ;

subquery_basic_elements
    :    query_block
    |    LEFT_PAREN subquery RIGHT_PAREN
    ;

query_block
    :    select_key
        ((distinct_key|unique_key|all_key)=> (distinct_key|unique_key|all_key))?
        (ASTERISK | selected_element (COMMA selected_element)*)
        into_clause?
        from_clause 
        where_clause? 
        hierarchical_query_clause? 
        group_by_clause?
        model_clause?
    ;

selected_element
    :    select_list_elements column_alias?
    ;

from_clause
    :    from_key table_ref_list
    ;

select_list_elements
    :    (tableview_name PERIOD ASTERISK)=> tableview_name PERIOD ASTERISK
    |    expression
    ;

table_ref_list
    :    table_ref (COMMA table_ref)*
    ;

// NOTE to PIVOT clause
// according the SQL reference this should not be possible
// according to he reality it is. Here we probably apply pivot/unpivot onto whole join clause
// eventhough it is not enclosed in parenthesis. See pivot examples 09,10,11
table_ref
    :    table_ref_aux join_clause* (pivot_clause|unpivot_clause)?
    ;

table_ref_aux
    :
    (    (LEFT_PAREN (select_key|with_key)) => dml_table_expression_clause (pivot_clause|unpivot_clause)?
    |    (LEFT_PAREN) => LEFT_PAREN table_ref subquery_operation_part* RIGHT_PAREN (pivot_clause|unpivot_clause)?
    |    (only_key LEFT_PAREN) => only_key LEFT_PAREN dml_table_expression_clause RIGHT_PAREN
    |    dml_table_expression_clause (pivot_clause|unpivot_clause)?
    )
        flashback_query_clause*
        ({isTableAlias(LT(1), LT(2))}? table_alias)?
    ;

join_clause
    :    query_partition_clause?
        (cross_key|natural_key)? (inner_key|outer_join_type)? join_key
        table_ref_aux
        query_partition_clause?
    (    join_on_part
    |    join_using_part
    )*
    ;

join_on_part
    :    on_key condition
    ;

join_using_part
    :    using_key LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
    ;

outer_join_type
    :    
    (    full_key
    |    left_key
    |    right_key
    )
        outer_key?
    ;

query_partition_clause
    :    partition_key by_key
    (    (LEFT_PAREN (select_key|with_key)) => LEFT_PAREN subquery RIGHT_PAREN
    |    (LEFT_PAREN)=> expression_list
    |    expression (COMMA expression)*
    )
    ;

flashback_query_clause
    :    versions_key between_key (scn_key|timestamp_key) expression
    |    as_key of_key (scn_key|timestamp_key|snapshot_key) expression
    ;

pivot_clause
    :    pivot_key xml_key?
        LEFT_PAREN
            pivot_element (COMMA pivot_element)*
            pivot_for_clause
            pivot_in_clause  
        RIGHT_PAREN
    ;

pivot_element
    :    aggregate_function_name LEFT_PAREN expression RIGHT_PAREN column_alias?
    ;

pivot_for_clause
    :    for_key 
    (    column_name
    |    LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
    )
    ;

pivot_in_clause
    :    in_key
        LEFT_PAREN
            (    (select_key)=> subquery 
            |    (any_key)=> any_key (COMMA any_key)*
            |    pivot_in_clause_element (COMMA pivot_in_clause_element)*
            )
        RIGHT_PAREN
    ;

pivot_in_clause_element
    :    pivot_in_clause_elements column_alias?
    ;

pivot_in_clause_elements
    :    expression
    |    (LEFT_PAREN)=> expression_list
    ;

unpivot_clause
    :    unpivot_key 
        ((include_key|exclude_key) nulls_key)?
        LEFT_PAREN
            (    column_name
            |    LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
            )
            pivot_for_clause
            unpivot_in_clause
        RIGHT_PAREN
    ;

unpivot_in_clause
    :    in_key
        LEFT_PAREN
            unpivot_in_elements (COMMA unpivot_in_elements)*
        RIGHT_PAREN
    ;

unpivot_in_elements
    :   (    column_name
        |    LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
        )
        (     as_key
         (    constant
         |    (LEFT_PAREN)=> LEFT_PAREN constant (COMMA constant)* RIGHT_PAREN
         )
        )?
    ;

hierarchical_query_clause
    :    connect_key by_key nocycle_key? condition start_part?
    |    start_part connect_key by_key nocycle_key? condition
    ;

start_part
    :    start_key with_key condition
    ;

group_by_clause
    :   (group_key) => group_key by_key group_by_elements
            ((COMMA group_by_elements)=> COMMA group_by_elements)*
            having_clause?
    |   (having_key) => having_clause
            (group_key by_key group_by_elements
            ((COMMA group_by_elements)=> COMMA group_by_elements)*)?
    ;

group_by_elements
    :    grouping_sets_clause
    |    rollup_cube_clause 
    |    expression
    ;

rollup_cube_clause
    :    (rollup_key|cube_key) LEFT_PAREN grouping_sets_elements (COMMA grouping_sets_elements)* RIGHT_PAREN 
    ;

grouping_sets_clause
    :    grouping_key sets_key 
        LEFT_PAREN grouping_sets_elements (COMMA grouping_sets_elements)* RIGHT_PAREN
    ;

grouping_sets_elements
    :    (rollup_key|cube_key)=> rollup_cube_clause
    |    (LEFT_PAREN)=> expression_list
    |    expression
    ;

having_clause
    :    having_key condition
    ;

model_clause
    :    model_key cell_reference_options* return_rows_clause? reference_model* main_model
    ;

cell_reference_options
    :    (ignore_key|keep_key) nav_key
    |    unique_key (dimension_key|single_key reference_key) 
    ;

return_rows_clause
    :    return_key (updated_key|all_key) rows_key
    ;

reference_model
    :    reference_key reference_model_name on_key 
            LEFT_PAREN subquery RIGHT_PAREN model_column_clauses 
            cell_reference_options*
    ;

main_model
    :    (main_key main_model_name)? model_column_clauses cell_reference_options* model_rules_clause
    ;

model_column_clauses
    :    model_column_partition_part?
        dimension_key by_key model_column_list measures_key model_column_list
    ;

model_column_partition_part
    :    partition_key by_key model_column_list
    ;

model_column_list
    :    LEFT_PAREN model_column (COMMA model_column)*  RIGHT_PAREN
    ;

model_column
    :    expression table_alias?
    ;

model_rules_clause
    :    model_rules_part? LEFT_PAREN model_rules_element (COMMA model_rules_element)* RIGHT_PAREN
    ;

model_rules_part
    :    rules_key (update_key|upsert_key all_key?)? ((automatic_key|sequential_key) order_key)? model_iterate_clause?
    ;

model_rules_element
    :    (update_key|upsert_key ((all_key)=> all_key)?)?
        cell_assignment
                order_by_clause?
            EQUALS_OP expression
    ;

cell_assignment
    :    model_expression
    ;

model_iterate_clause
    :    iterate_key LEFT_PAREN expression RIGHT_PAREN until_part?
    ;

until_part
    :    until_key LEFT_PAREN condition RIGHT_PAREN
    ;

order_by_clause
    :    order_key siblings_key? by_key order_by_elements (COMMA order_by_elements)*
    ;

order_by_elements
    :    expression (asc_key|desc_key)? (nulls_key (first_key|last_key))?
    ;

for_update_clause
    :    for_key update_key for_update_of_part? for_update_options?
    ;

for_update_of_part
    :    of_key column_name (COMMA column_name)*
    ;

for_update_options
    :    skip_key locked_key
    |    nowait_key
    |    wait_key expression
    ;

// $>

update_statement
    :    update_key general_table_ref
        update_set_clause
        where_clause? static_returning_clause? error_logging_clause?
    ;

// $<Update - Specific Clauses
update_set_clause
    :    set_key
    (    column_based_update_set_clause (COMMA column_based_update_set_clause)*
    |    value_key LEFT_PAREN id RIGHT_PAREN EQUALS_OP expression
    )
    ;

column_based_update_set_clause
    :    column_name EQUALS_OP expression
    |    LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN EQUALS_OP subquery
    ;

// $>

delete_statement
    :    delete_key from_key?
        general_table_ref
        where_clause? static_returning_clause? error_logging_clause?
    ;

insert_statement
    :    insert_key
    (    single_table_insert
    |    multi_table_insert
    )
    ;

// $<Insert - Specific Clauses

single_table_insert
    :    insert_into_clause
    (    values_clause static_returning_clause?
    |    select_statement
    )
        error_logging_clause?
    ;

multi_table_insert
    :    
    (    all_key multi_table_element+
    |    conditional_insert_clause
    )
        select_statement
    ;

multi_table_element
    :    insert_into_clause values_clause? error_logging_clause?
    ;

conditional_insert_clause
    :    (all_key|first_key)?
        conditional_insert_when_part+ conditional_insert_else_part?
    ;

conditional_insert_when_part
    :    when_key condition then_key multi_table_element+
    ;

conditional_insert_else_part
    :    else_key multi_table_element+
    ;

insert_into_clause
    :    into_key general_table_ref 
        (LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN)?
    ;

values_clause
    :    values_key expression_list
    ;

// $>
merge_statement
    :    merge_key into_key tableview_name table_alias?
        using_key selected_tableview on_key LEFT_PAREN condition RIGHT_PAREN
        (
            (when_key matched_key) => merge_update_clause merge_insert_clause?
        |
            (when_key not_key matched_key) => merge_insert_clause merge_update_clause?
        )?
        error_logging_clause?
    ;

// $<Merge - Specific Clauses

merge_update_clause
    :    when_key matched_key then_key update_key set_key 
        merge_element (COMMA merge_element)*
        where_clause? merge_update_delete_part?
    ;

merge_element
    :    column_name EQUALS_OP expression
    ;

merge_update_delete_part
    :    delete_key where_clause
    ;

merge_insert_clause
    :    when_key not_key matched_key then_key insert_key 
        (LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN)?
        values_key expression_list where_clause?
    ;

selected_tableview
    :    ( tableview_name | LEFT_PAREN select_statement RIGHT_PAREN ) table_alias?
    ;

// $>

lock_table_statement
    :    lock_key table_key 
        lock_table_element (COMMA lock_table_element)* 
        in_key lock_mode mode_key wait_nowait_part?
    ;

wait_nowait_part
    :    wait_key expression
    |    nowait_key
    ;

// $<Lock - Specific Clauses

lock_table_element
    :    tableview_name partition_extension_clause?
    ;

lock_mode
    :    row_key share_key
    |    row_key exclusive_key
    |    share_key update_key?
    |    share_key row_key exclusive_key
    |    exclusive_key
    ;
// $>

// $<Common DDL Clauses

general_table_ref
    :    (    dml_table_expression_clause
        |    only_key LEFT_PAREN dml_table_expression_clause RIGHT_PAREN
        )    table_alias?
    ;

static_returning_clause
    :    (returning_key|return_key) expression (COMMA expression)* 
        into_clause
    ;

error_logging_clause
    :    log_key errors_key 
        error_logging_into_part?
        ((LEFT_PAREN)=> expression_wrapper)?
        error_logging_reject_part?
    ;

error_logging_into_part
    :    into_key tableview_name
    ;

error_logging_reject_part
    :    reject_key limit_key ((unlimited_key)=>unlimited_key|expression_wrapper)
    ;

dml_table_expression_clause
    :    table_collection_expression
    |    LEFT_PAREN select_statement subquery_restriction_clause? RIGHT_PAREN
    |    tableview_name sample_clause?
    ;

table_collection_expression
    :    ( table_key | the_key)
         ( (LEFT_PAREN (select_key | with_key)) => LEFT_PAREN subquery RIGHT_PAREN
         | LEFT_PAREN expression RIGHT_PAREN //(LEFT_PAREN PLUS_SIGN RIGHT_PAREN)?
         )
    ;

subquery_restriction_clause
    :    with_key
    (    read_key only_key
    |    check_key option_key (constraint_key constraint_name)?
    )
    ;

sample_clause
    :    sample_key block_key? 
        LEFT_PAREN expression (COMMA expression)? RIGHT_PAREN
        seed_part?
    ;

seed_part
    :    seed_key LEFT_PAREN expression RIGHT_PAREN
    ;

// $>

// $<Expression & Condition
cursor_expression
    :    cursor_key LEFT_PAREN subquery RIGHT_PAREN
    ;

expression_list
    :    LEFT_PAREN expression? (COMMA expression)* RIGHT_PAREN
    ;

condition
    :     expression
    ;

condition_wrapper
    :    expression
    ;

expression
    :    (cursor_key LEFT_PAREN (select_key|with_key)) => cursor_expression
    |    logical_and_expression ( or_key logical_and_expression )*
    ;

expression_wrapper
    :    expression
    ;

logical_and_expression
    :    negated_expression ( and_key negated_expression )*
    ;

negated_expression
    :    not_key negated_expression
    |    equality_expression
    ;

equality_expression
    :    multiset_comparsion
    (    is_key not_key?
        (    null_key
        |    nan_key
        |    present_key
        |    infinite_key
        |    a_key set_key
        |    empty_key
        |    of_key type_key? LEFT_PAREN only_key? type_spec (COMMA type_spec)* RIGHT_PAREN
        )
    )?
    ;


multiset_comparsion
    :    relational_expression
    (    multiset_type of_key? concatenation)?
    ;

multiset_type
    :    member_key
    |    submultiset_key
    ;

relational_expression
    :    compound_expression
    ( ( EQUALS_OP | not_equal_op | LESS_THAN_OP | GREATER_THAN_OP | less_than_or_equals_op | greater_than_or_equals_op ) compound_expression)*
    ;

compound_expression
    :    concatenation
    (    not_key?
        (    in_key in_elements
        |    between_key between_elements
        |    like_type concatenation like_escape_part?
        )
    )?
    ;

like_type
    :    like_key
    |    likec_key
    |    like2_key
    |    like4_key
    ;

like_escape_part
    :    escape_key concatenation
    ;

in_elements
    :    (LEFT_PAREN+ (select_key|with_key)) => LEFT_PAREN subquery RIGHT_PAREN
    |    LEFT_PAREN concatenation_wrapper (COMMA concatenation_wrapper)* RIGHT_PAREN
    |    constant
    |    bind_variable
    |    general_element
    ;

between_elements
    :    concatenation and_key concatenation
    ;

concatenation
    :    additive_expression (concatenation_op additive_expression)*
    ;

concatenation_wrapper
    :    concatenation
    ;

additive_expression
    :    multiply_expression ( ( PLUS_SIGN | MINUS_SIGN ) multiply_expression)*
    ;

multiply_expression
    :    datetime_expression ( ( ASTERISK | SOLIDUS ) datetime_expression)*
    ;

datetime_expression
    :    model_expression
    (    at_key (local_key|time_key zone_key concatenation_wrapper)
    |    (interval_expression)=> interval_expression
    )?
    ;

interval_expression
    :    day_key    (LEFT_PAREN concatenation_wrapper RIGHT_PAREN)? to_key second_key (LEFT_PAREN concatenation_wrapper RIGHT_PAREN)?
    |    year_key (LEFT_PAREN concatenation_wrapper RIGHT_PAREN)? to_key month_key
    ;

model_expression
    :    multiset_expression
        (LEFT_BRACKET model_expression_element RIGHT_BRACKET)?
    ;

model_expression_element
    :    ((any_key)=> any_key|condition_wrapper) (COMMA ((any_key)=> any_key|condition_wrapper))*
    |    single_column_for_loop (COMMA single_column_for_loop)*
    |    multi_column_for_loop
    ;

single_column_for_loop
    :    for_key column_name 
    (    in_key expression_list
    |    for_like_part? from_key ex1=expression
            to_key ex2=expression for_increment_decrement_type ex3=expression     
    )
    ;

for_like_part
    :    like_key expression
    ;

for_increment_decrement_type
    :    increment_key
    |    decrement_key
    ;

multi_column_for_loop
    :    for_key LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN in_key
        LEFT_PAREN
            (    (select_key)=> subquery
            |    (LEFT_PAREN)=> LEFT_PAREN expression_list (COMMA expression_list)* RIGHT_PAREN
            )
        RIGHT_PAREN
    ;

multiset_expression
	:	unary_expression
		( multiset_op unary_expression )*
	;

unary_expression
options
{
backtrack=true;
}
    :    MINUS_SIGN unary_expression
    |    PLUS_SIGN unary_expression
    |    prior_key unary_expression
    |    connect_by_root_key unary_expression
    |    { // input.LT(1).getText().equalsIgnoreCase("new") && !input.LT(2).getText().equals(".")}?
            equalsIgnoreCase(LT(1)->getText(), "NEW") && !equalsIgnoreCase(LT(2)->getText(), "." )
         }? => new_key unary_expression
    |    distinct_key unary_expression
    |    all_key unary_expression
    |    { LA(1) == SQL92_RESERVED_CASE || LA(2) == SQL92_RESERVED_CASE }? case_statement[false]
    |    quantified_expression
    |    standard_function
    |    atom
    ;

case_statement [int isStatementParameter]
scope    {
    int isStatement;
}
@init    {$case_statement::isStatement = $isStatementParameter;}
    :    (label_name? case_key when_key)=> searched_case_statement
    |    simple_case_statement
    ;

// $<CASE - Specific Clauses

simple_case_statement
    :    label_name? ck1=case_key atom
        simple_case_when_part+ 
        case_else_part?
        end_key case_key? label_name?
    ;

simple_case_when_part
    :    when_key expression_wrapper then_key ({$case_statement::isStatement}? seq_of_statements | expression_wrapper)
    ;

searched_case_statement
    :    label_name? ck1=case_key
        searched_case_when_part+
        case_else_part?
        end_key case_key? label_name?
    ;

searched_case_when_part
    :    when_key condition_wrapper then_key ({$case_statement::isStatement}? seq_of_statements | expression_wrapper)
    ;

case_else_part
    :    else_key ({$case_statement::isStatement}? seq_of_statements | expression_wrapper)
    ;
// $>

atom
options
{
backtrack=true;
}
    :    (table_element outer_join_sign) => table_element outer_join_sign
    |    bind_variable
    |    constant
    |    general_element
    |    LEFT_PAREN
         (
              ( select_key | with_key)=> subquery RIGHT_PAREN subquery_operation_part*
              | expression_or_vector RIGHT_PAREN
         )
    ;

expression_or_vector
    :    expression (vector_expr)?
    ;

vector_expr
    :    COMMA expression (COMMA expression)*
    ;

quantified_expression
    :    ( some_key | exists_key | all_key | any_key )
         ( (LEFT_PAREN (select_key|with_key)) => LEFT_PAREN subquery RIGHT_PAREN
           | LEFT_PAREN expression_wrapper RIGHT_PAREN
         )
    ;

standard_function
    :    stantard_function_enabling_over function_argument_analytic over_clause?
    |    stantard_function_enabling_using function_argument_modeling using_clause?
    |    count_key
            LEFT_PAREN
                ( ASTERISK | (distinct_key|unique_key|all_key)? concatenation_wrapper )
            RIGHT_PAREN over_clause?
    |    (cast_key|xmlcast_key) 
            LEFT_PAREN
         ( (multiset_key LEFT_PAREN+ (select_key|with_key)) => multiset_key LEFT_PAREN subquery order_by_clause? RIGHT_PAREN
           (as_key type_spec)?
                | concatenation_wrapper as_key type_spec
                )
            RIGHT_PAREN
    |    chr_key
            LEFT_PAREN 
                concatenation_wrapper using_key nchar_cs_key 
            RIGHT_PAREN
    |    collect_key
            LEFT_PAREN 
                (distinct_key|unique_key)? concatenation_wrapper collect_order_by_part?
            RIGHT_PAREN
    |    stantard_function_enabling_within_or_over 
            function_argument within_or_over_part+
    |    decompose_key
            LEFT_PAREN 
                concatenation_wrapper (canonical_key|compatibility_key)? 
            RIGHT_PAREN
    |    extract_key
            LEFT_PAREN
                REGULAR_ID from_key concatenation_wrapper 
            RIGHT_PAREN
    |    (first_value_key|last_value_key) function_argument_analytic
             respect_or_ignore_nulls? over_clause
    |    stantard_function_pedictions
            LEFT_PAREN
                expression_wrapper (COMMA expression_wrapper)* cost_matrix_clause? using_clause? 
            RIGHT_PAREN
    |    translate_key
            LEFT_PAREN 
                expression_wrapper (using_key (char_cs_key|nchar_cs_key))? 
                    (COMMA expression_wrapper)* 
            RIGHT_PAREN
    |    treat_key
            LEFT_PAREN
                expression_wrapper as_key ref_key? type_spec 
            RIGHT_PAREN
    |    trim_key
            LEFT_PAREN 
                ((leading_key|trailing_key|both_key)? quoted_string? from_key)?
                concatenation_wrapper
            RIGHT_PAREN
    |    xmlagg_key
            LEFT_PAREN 
                expression_wrapper order_by_clause? 
            RIGHT_PAREN
            (PERIOD general_element_part)?
    |    (xmlcolattval_key|xmlforest_key) 
            LEFT_PAREN
                xml_multiuse_expression_element (COMMA xml_multiuse_expression_element)*
            RIGHT_PAREN
            (PERIOD general_element_part)?
    |    xmlelement_key
            LEFT_PAREN
                (entityescaping_key|noentityescaping_key)?
                (name_key|evalname_key)? expression_wrapper
                ({equalsIgnoreCase(LT(2)->getText(), "XMLATTRIBUTES")}? COMMA xml_attributes_clause)?
                (COMMA expression_wrapper column_alias?)*
            RIGHT_PAREN
            (PERIOD general_element_part)?
    |    xmlexists_key
            LEFT_PAREN
                expression_wrapper
                xml_passing_clause?
            RIGHT_PAREN
    |    xmlparse_key
            LEFT_PAREN 
                (document_key|content_key) concatenation_wrapper wellformed_key?
            RIGHT_PAREN
            (PERIOD general_element_part)?
    |    xmlpi_key
            LEFT_PAREN 
                (    name_key id
                |    evalname_key concatenation_wrapper
                )
                (COMMA concatenation_wrapper)?
            RIGHT_PAREN
            (PERIOD general_element_part)?
    |    xmlquery_key
            LEFT_PAREN 
                concatenation_wrapper xml_passing_clause?
                returning_key content_key (null_key on_key empty_key)?
            RIGHT_PAREN
            (PERIOD general_element_part)?
    |    xmlroot_key
            LEFT_PAREN
                concatenation_wrapper
                    xmlroot_param_version_part
                    (COMMA xmlroot_param_standalone_part)?
            RIGHT_PAREN
            (PERIOD general_element_part)?
    |    xmlserialize_key
            LEFT_PAREN
                (document_key|content_key)
                concatenation_wrapper (as_key type_spec)?
                xmlserialize_param_enconding_part?
                xmlserialize_param_version_part?
                xmlserialize_param_ident_part?
                ((hide_key|show_key) defaults_key)?
            RIGHT_PAREN
            (PERIOD general_element_part)?
    |    xmltable_key
            LEFT_PAREN
                xml_namespaces_clause?
                concatenation_wrapper
                xml_passing_clause?
                (columns_key xml_table_column (COMMA xml_table_column))?
            RIGHT_PAREN
            (PERIOD general_element_part)?
    ;

stantard_function_enabling_over
    :    {enablesOverClause(LT(1)->getText())}?=> REGULAR_ID
    ;

stantard_function_enabling_using
    :    {enablesUsingClause(LT(1)->getText())}?=> REGULAR_ID
    ;

stantard_function_enabling_within_or_over
    :    {enablesWithinOrOverClause(LT(1)->getText())}?=> REGULAR_ID
    ;

stantard_function_pedictions
    :    {isStandardPredictionFunction(LT(1)->getText())}?=> REGULAR_ID
    ;

over_clause
    :    over_key
        LEFT_PAREN
            query_partition_clause?
            (order_by_clause windowing_clause?)?
        RIGHT_PAREN
    ;

windowing_clause
    :    windowing_type
    (    between_key windowing_elements and_key windowing_elements
    |    windowing_elements    )
    ;

windowing_type
    :    rows_key
    |    range_key
    ;

windowing_elements
    :    unbounded_key preceding_key
    |    current_key row_key
    |    concatenation_wrapper (preceding_key|following_key)
    ;

using_clause
    :    using_key
    (    ASTERISK
    |    using_element (COMMA using_element)*
    )
    ;

using_element
    :    (in_key out_key?|out_key)? select_list_elements column_alias?
    ;

collect_order_by_part
    :    order_key by_key concatenation_wrapper
    ;

within_or_over_part
    :    within_key group_key LEFT_PAREN order_by_clause RIGHT_PAREN
    |    over_clause
    ;

cost_matrix_clause
    :    cost_key
    (    model_key auto_key?
    |    LEFT_PAREN cost_class_name (COMMA cost_class_name)* RIGHT_PAREN values_key 
            expression_list
    )
    ;

xml_passing_clause
    :    passing_key (by_key value_key)?
            expression_wrapper column_alias? (COMMA expression_wrapper column_alias?)
    ;

xml_attributes_clause
    :    xmlattributes_key
        LEFT_PAREN
            (entityescaping_key|noentityescaping_key)?
            (schemacheck_key|noschemacheck_key)?
            xml_multiuse_expression_element (COMMA xml_multiuse_expression_element)*
        RIGHT_PAREN
    ;

xml_namespaces_clause
    :    xmlnamespaces_key
        LEFT_PAREN
            (concatenation_wrapper column_alias)?
                (COMMA concatenation_wrapper column_alias)*
            ((default_key)=> xml_general_default_part)?
        RIGHT_PAREN
    ;

xml_table_column
    :    xml_column_name
    (    for_key ordinality_key
    |    type_spec (path_key concatenation_wrapper)? ((default_key)=> xml_general_default_part)?
    )
    ;

xml_general_default_part
    :    default_key concatenation_wrapper
    ;

xml_multiuse_expression_element
    :    expression (as_key (id_expression |evalname_key concatenation ))?
    ;

xmlroot_param_version_part
    :    version_key (no_key value_key|expression_wrapper)
    ;

xmlroot_param_standalone_part
    :    standalone_key (yes_key|no_key value_key?)
    ;

xmlserialize_param_enconding_part
    :    encoding_key concatenation_wrapper
    ;

xmlserialize_param_version_part
    :    version_key concatenation_wrapper
    ;

xmlserialize_param_ident_part
    :    no_key indent_key
    |    indent_key (size_key EQUALS_OP concatenation_wrapper)?
    ;

// $>
