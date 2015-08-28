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
parser grammar OracleDML;

options {
    output=AST;
    language=Cpp;    
    tokenVocab=PLSQLLexer;
}

import OracleDMLKeys, PLSQLCommons;
tokens {
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
}

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
         -> ^(EXPLAIN_STATEMENT select_statement? update_statement? delete_statement? insert_statement? merge_statement?)
    ;

select_statement
    :    subquery_factoring_clause?
        subquery
        (for_update_clause|(order_key siblings_key? by_key)=> order_by_clause)*
        -> ^(SELECT_STATEMENT subquery_factoring_clause? subquery for_update_clause* order_by_clause*)  
    ;

// $<Select - Specific Clauses
subquery_factoring_clause
    :    with_key^ factoring_element (COMMA! factoring_element)*
    ;

factoring_element
    :    query_name (LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN)? as_key LEFT_PAREN subquery order_by_clause? RIGHT_PAREN
         search_clause?
         cycle_clause?
        -> ^(FACTORING query_name subquery search_clause? cycle_clause?)
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
        -> ^(SUBQUERY subquery_basic_elements subquery_operation_part*)
    ;

subquery_operation_part
    :    (union_key^ all_key?|intersect_key^|minus_key^) subquery_basic_elements
    ;

subquery_basic_elements
    :    query_block
    |    LEFT_PAREN! subquery RIGHT_PAREN!
    ;

query_block
@init    {    int mode = 0;    }
    :    select_key
        ((distinct_key|unique_key|all_key)=> (distinct_key|unique_key|all_key))?
        (ASTERISK {mode = 1;}| selected_element (COMMA selected_element)*)
        into_clause?
        from_clause 
        where_clause? 
        hierarchical_query_clause? 
        group_by_clause?
        model_clause?
        -> {mode == 1}? ^(select_key distinct_key? unique_key? all_key? ASTERISK
                into_clause? from_clause where_clause? hierarchical_query_clause? group_by_clause? model_clause?)
        -> ^(select_key distinct_key? unique_key? all_key? ^(SELECT_LIST selected_element+)
                into_clause? from_clause where_clause? hierarchical_query_clause? group_by_clause? model_clause?)
    ;

selected_element
    :    select_list_elements column_alias?
        -> ^(SELECT_ITEM select_list_elements column_alias?)
    ;

from_clause
    :    from_key^ table_ref_list
    ;

select_list_elements
    :    (tableview_name PERIOD ASTERISK)=> tableview_name PERIOD ASTERISK -> ^(EXPR ^(DOT_ASTERISK[$ASTERISK] tableview_name))  
    |    expression -> ^(EXPR expression)
    ;

table_ref_list
    :    table_ref (COMMA! table_ref)*
    ;

// NOTE to PIVOT clause
// according the SQL reference this should not be possible
// according to he reality it is. Here we probably apply pivot/unpivot onto whole join clause
// eventhough it is not enclosed in parenthesis. See pivot examples 09,10,11
table_ref
    :    table_ref_aux join_clause* (pivot_clause|unpivot_clause)?
        -> ^(TABLE_REF table_ref_aux join_clause* pivot_clause? unpivot_clause?)
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
        -> ^(TABLE_REF_ELEMENT table_alias? dml_table_expression_clause? table_ref? subquery_operation_part* only_key? pivot_clause? unpivot_clause? flashback_query_clause*)
    ;

join_clause
    :    query_partition_clause?
        (cross_key|natural_key)? (inner_key|outer_join_type)? join_key
        table_ref_aux
        query_partition_clause?
    (    join_on_part
    |    join_using_part
    )*
        -> ^(JOIN_DEF[$join_key.start] cross_key? natural_key? inner_key? outer_join_type? table_ref_aux query_partition_clause* join_on_part* join_using_part*)
    ;

join_on_part
    :    on_key condition
        -> ^(on_key ^(LOGIC_EXPR condition)) 
    ;

join_using_part
    :    using_key^ LEFT_PAREN! column_name (COMMA! column_name)* RIGHT_PAREN!
    ;

outer_join_type
    :    
    (    full_key
    |    left_key
    |    right_key
    )
        outer_key!?
    ;

query_partition_clause
    :    partition_key by_key
    (    (LEFT_PAREN (select_key|with_key)) => LEFT_PAREN subquery RIGHT_PAREN
    |    (LEFT_PAREN)=> expression_list
    |    expression (COMMA expression)*
    )
        -> ^(partition_key expression_list? (EXPR expression)*)
    ;

flashback_query_clause
    :    versions_key between_key (scn_key|timestamp_key) expression -> ^(versions_key scn_key? timestamp_key? ^(EXPR expression)) 
    |    as_key of_key (scn_key|timestamp_key|snapshot_key) expression -> ^(as_key scn_key? timestamp_key? snapshot_key? ^(EXPR expression))
    ;

pivot_clause
    :    pivot_key xml_key?
        LEFT_PAREN
            pivot_element (COMMA pivot_element)*
            pivot_for_clause
            pivot_in_clause  
        RIGHT_PAREN
        -> ^(pivot_key xml_key? pivot_element+ pivot_for_clause pivot_in_clause)
    ;

pivot_element
    :    aggregate_function_name LEFT_PAREN expression RIGHT_PAREN column_alias?
        -> ^(PIVOT_ELEMENT column_alias? ^(EXPR ^(ROUTINE_CALL aggregate_function_name ^(ARGUMENTS ^(ARGUMENT ^(EXPR expression))))))
    ;

pivot_for_clause
    :    for_key 
    (    column_name
    |    LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
    )
        -> ^(for_key column_name+)
    ;

pivot_in_clause
@init    {    int mode = 0;    }
    :    in_key
        LEFT_PAREN
            (    (select_key)=> subquery {mode = 1;}
            |    (any_key)=> any_key (COMMA any_key)* {mode = 2;}
            |    pivot_in_clause_element (COMMA pivot_in_clause_element)*
            )
        RIGHT_PAREN
        ->{mode == 1}? ^(in_key subquery)
        ->{mode == 2}? ^(in_key ^(ANY_MODE any_key+))
        -> ^(in_key ^(ELEMENTS_MODE pivot_in_clause_element+))
    ;

pivot_in_clause_element
    :    pivot_in_clause_elements column_alias?
        -> ^(PIVOT_IN_ELEMENT column_alias? pivot_in_clause_elements)
    ;

pivot_in_clause_elements
    :    expression -> ^(EXPR expression)
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
        -> ^(unpivot_key include_key? exclude_key? nulls_key? column_name+ pivot_for_clause unpivot_in_clause)
    ;

unpivot_in_clause
    :    in_key
        LEFT_PAREN
            unpivot_in_elements (COMMA unpivot_in_elements)*
        RIGHT_PAREN
        -> ^(in_key ^(UNPIVOT_IN_ELEMENT unpivot_in_elements)+)
    ;

unpivot_in_elements
    :   (    column_name
        |    LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
        )
        (    as_key
        (    constant
        |    (LEFT_PAREN)=> LEFT_PAREN constant (COMMA constant)* RIGHT_PAREN
        )
        )?
        -> column_name+ ^(PIVOT_ALIAS constant+)?
    ;

hierarchical_query_clause
    :    connect_key by_key nocycle_key? condition start_part?
        -> ^(HIERARCHICAL start_part? ^(connect_key nocycle_key? ^(LOGIC_EXPR condition)))
    |    start_part connect_key by_key nocycle_key? condition
        -> ^(HIERARCHICAL start_part ^(connect_key nocycle_key? ^(LOGIC_EXPR condition)))
    ;

start_part
    :    start_key with_key condition
        -> ^(start_key ^(LOGIC_EXPR condition))
    ;

group_by_clause
    :   (group_key) => group_key by_key
			(LEFT_PAREN RIGHT_PAREN
			| group_by_elements ((COMMA group_by_elements)=> COMMA group_by_elements)*
			)
		having_clause?
		-> ^(GROUP_BY_CLAUSE[$group_key.start] ^(group_key ^(GROUP_BY_ELEMENT group_by_elements)*) having_clause?)
    |   (having_key) => having_clause
		(group_key by_key
			(LEFT_PAREN RIGHT_PAREN
			| group_by_elements ((COMMA group_by_elements)=> COMMA group_by_elements)*
			)
		)?
		-> ^(GROUP_BY_CLAUSE[$having_clause.start] having_clause ^(group_key ^(GROUP_BY_ELEMENT group_by_elements)*)?)
    ;

group_by_elements
    :    grouping_sets_clause
    |    rollup_cube_clause 
    |    expression -> ^(EXPR expression)
    ;

rollup_cube_clause
    :    (rollup_key^|cube_key^) LEFT_PAREN! grouping_sets_elements (COMMA! grouping_sets_elements)* RIGHT_PAREN! 
    ;

grouping_sets_clause
    :    grouping_key sets_key 
        LEFT_PAREN grouping_sets_elements (COMMA grouping_sets_elements)* RIGHT_PAREN
        -> ^(grouping_key ^(GROUPIN_SET grouping_sets_elements)+)
    ;

grouping_sets_elements
    :    (rollup_key|cube_key)=> rollup_cube_clause
    |    (LEFT_PAREN)=> expression_list
    |    expression -> ^(EXPR expression)
    ;

having_clause
    :    having_key condition
        -> ^(having_key ^(LOGIC_EXPR condition))
    ;

model_clause
    :    model_key cell_reference_options* return_rows_clause? reference_model* main_model
        -> ^(model_key main_model cell_reference_options* return_rows_clause? reference_model*)
    ;

cell_reference_options
    :    (ignore_key^|keep_key^) nav_key
    |    unique_key^ (dimension_key|single_key reference_key!) 
    ;

return_rows_clause
    :    return_key^ (updated_key|all_key) rows_key!
    ;

reference_model
    :    reference_key^ reference_model_name on_key! 
            LEFT_PAREN! subquery RIGHT_PAREN! model_column_clauses 
            cell_reference_options*
    ;

main_model
    :    (main_key main_model_name)? model_column_clauses cell_reference_options* model_rules_clause
        -> ^(MAIN_MODEL main_model_name? model_column_clauses model_rules_clause cell_reference_options*)
    ;

model_column_clauses
    :    model_column_partition_part?
        dimension_key by_key model_column_list measures_key model_column_list
        -> ^(MODEL_COLUMN ^(dimension_key model_column_list) ^(measures_key model_column_list) model_column_partition_part?)
    ;

model_column_partition_part
    :    partition_key^ by_key! model_column_list
    ;

model_column_list
    :    LEFT_PAREN model_column (COMMA model_column)*  RIGHT_PAREN
        -> ^(MODEL_COLUMNS model_column+)
    ;

model_column
    :    expression column_alias?
        -> ^(MODEL_COLUMN column_alias? ^(EXPR expression))
    ;

model_rules_clause
    :    model_rules_part? LEFT_PAREN (model_rules_element (COMMA model_rules_element)*)? RIGHT_PAREN
        -> ^(MODEL_RULES model_rules_element* model_rules_part?)
    ;

model_rules_part
    :    rules_key (update_key|upsert_key all_key?)? ((automatic_key|sequential_key) order_key)? model_iterate_clause?
        -> ^(rules_key update_key? upsert_key? all_key? automatic_key? sequential_key? model_iterate_clause?)
    ;

model_rules_element
    :    (update_key|upsert_key ((all_key)=> all_key)?)?
        cell_assignment
                order_by_clause?
            EQUALS_OP expression
        -> ^(MODEL_RULE ^(ASSIGN[$EQUALS_OP] cell_assignment ^(EXPR expression)) update_key? upsert_key? all_key? order_by_clause?)
    ;

cell_assignment
    :    model_expression
    ;

model_iterate_clause
    :    iterate_key LEFT_PAREN expression RIGHT_PAREN until_part?
        ->^(iterate_key ^(EXPR expression) until_part?)
    ;

until_part
    :    until_key LEFT_PAREN condition RIGHT_PAREN
        -> ^(until_key ^(LOGIC_EXPR condition))
    ;

order_by_clause
    :    order_key siblings_key? by_key order_by_elements (COMMA order_by_elements)*
        -> ^(order_key siblings_key? ^(ORDER_BY_ELEMENTS order_by_elements+))
    ;

order_by_elements
    :    expression (asc_key|desc_key)? (nulls_key (first_key|last_key))?
        -> ^(ORDER_BY_ELEMENT ^(EXPR expression) asc_key? desc_key? nulls_key? first_key? last_key?)
    ;

for_update_clause
    :    for_key update_key for_update_of_part? for_update_options?
        -> ^(for_key for_update_of_part? for_update_options?)
    ;

for_update_of_part
    :    of_key^ column_name (COMMA! column_name)*
    ;

for_update_options
    :    skip_key locked_key!
    |    nowait_key
    |    wait_key expression -> ^(wait_key ^(EXPR expression))
    ;

// $>

update_statement
    :    update_key^ general_table_ref
        update_set_clause
        where_clause? static_returning_clause? error_logging_clause?
    ;

// $<Update - Specific Clauses
update_set_clause
@init    {    int mode = 1;    }
    :    set_key
    (    column_based_update_set_clause (COMMA column_based_update_set_clause)*
    |    value_key LEFT_PAREN id RIGHT_PAREN EQUALS_OP expression {mode = 0;}
    )
        ->{mode == 1}? ^(set_key column_based_update_set_clause+)
        -> ^(set_key ^(value_key id ^(EXPR expression)))
    ;

column_based_update_set_clause
    :    column_name EQUALS_OP expression -> ^(ASSIGN[$EQUALS_OP] column_name ^(EXPR expression))
    |    LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN EQUALS_OP subquery -> ^(ASSIGN[$EQUALS_OP] column_name+ subquery)
    ;

// $>

delete_statement
    :    delete_key^ from_key!?
        general_table_ref
        where_clause? static_returning_clause? error_logging_clause?
    ;

insert_statement
    :    insert_key^
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
        -> ^(SINGLE_TABLE_MODE insert_into_clause values_clause? static_returning_clause? select_statement? error_logging_clause?)
    ;

multi_table_insert
    :    
    (    all_key multi_table_element+
    |    conditional_insert_clause
    )
        select_statement
        -> ^(MULTI_TABLE_MODE select_statement multi_table_element* conditional_insert_clause?)
    ;

multi_table_element
    :    insert_into_clause values_clause? error_logging_clause?
        -> ^(TABLE_ELEMENT insert_into_clause values_clause? error_logging_clause?)
    ;

conditional_insert_clause
    :    (all_key|first_key)?
        conditional_insert_when_part+ conditional_insert_else_part?
        -> ^(CONDITIONAL_INSERT all_key? first_key? conditional_insert_when_part+ conditional_insert_else_part?) 
    ;

conditional_insert_when_part
    :    when_key condition then_key multi_table_element+
        -> ^(when_key ^(LOGIC_EXPR condition) multi_table_element+)
    ;

conditional_insert_else_part
    :    else_key^ multi_table_element+
    ;

insert_into_clause
    :    into_key general_table_ref 
        (LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN)?
        -> ^(into_key general_table_ref ^(COLUMNS column_name*))
    ;

values_clause
    :    values_key^ (expression_list | record_name)
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
        -> ^(merge_key table_alias? tableview_name ^(using_key selected_tableview ^(LOGIC_EXPR condition))
                 merge_update_clause? merge_insert_clause? error_logging_clause?)
    ;

// $<Merge - Specific Clauses

merge_update_clause
    :    when_key matched_key then_key update_key set_key 
        merge_element (COMMA merge_element)*
        where_clause? merge_update_delete_part?
        ->^(MERGE_UPDATE merge_element+ where_clause? merge_update_delete_part?)
    ;

merge_element
    :    column_name EQUALS_OP expression
        -> ^(ASSIGN[$EQUALS_OP] column_name ^(EXPR expression))
    ;

merge_update_delete_part
    :    delete_key^ where_clause
    ;

merge_insert_clause
    :    when_key not_key matched_key then_key insert_key 
        (LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN)?
        values_key expression_list where_clause?
        -> ^(MERGE_INSERT ^(COLUMNS column_name*) expression_list where_clause?) 
    ;

selected_tableview
    :    ( tableview_name | LEFT_PAREN select_statement RIGHT_PAREN) table_alias?
        -> ^(SELECTED_TABLEVIEW table_alias? tableview_name? select_statement?)
    ;

// $>

lock_table_statement
    :    lock_key table_key 
        lock_table_element (COMMA lock_table_element)* 
        in_key lock_mode mode_key wait_nowait_part?
        -> ^(lock_key lock_table_element+ lock_mode wait_nowait_part?)
    ;

wait_nowait_part
    :    wait_key expression -> ^(wait_key ^(EXPR expression))
    |    nowait_key
    ;

// $<Lock - Specific Clauses

lock_table_element
    :    tableview_name partition_extension_clause?
        -> ^(LOCK_TABLE_ELEMENT tableview_name partition_extension_clause?)
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
        -> ^(TABLE_REF table_alias? dml_table_expression_clause only_key?)
    ;

static_returning_clause
    :    (returning_key|return_key) expression (COMMA expression)* 
        into_clause
        -> ^(STATIC_RETURNING ^(EXPR expression)+ into_clause)
    ;

error_logging_clause
    :    log_key errors_key 
        error_logging_into_part?
        ((LEFT_PAREN)=> expression_wrapper)?
        error_logging_reject_part?
        -> ^(log_key error_logging_into_part? expression_wrapper? error_logging_reject_part?)
    ;

error_logging_into_part
    :    into_key^ tableview_name
    ;

error_logging_reject_part
    :    reject_key^ limit_key! ((unlimited_key)=>unlimited_key|expression_wrapper)
    ;

dml_table_expression_clause
    :    table_collection_expression -> ^(TABLE_EXPRESSION ^(COLLECTION_MODE table_collection_expression))
    |    LEFT_PAREN select_statement subquery_restriction_clause? RIGHT_PAREN -> ^(TABLE_EXPRESSION ^(SELECT_MODE select_statement subquery_restriction_clause?))
    |    tableview_name sample_clause? -> ^(TABLE_EXPRESSION ^(DIRECT_MODE tableview_name sample_clause?))
    ;

table_collection_expression
    :    ( table_key | the_key)
         ( (LEFT_PAREN (select_key | with_key)) => LEFT_PAREN subquery RIGHT_PAREN
         | LEFT_PAREN expression RIGHT_PAREN //(LEFT_PAREN PLUS_SIGN RIGHT_PAREN)?
         )
        -> ^(EXPR subquery? expression?) //PLUS_SIGN?
    ;

subquery_restriction_clause
    :    with_key
    (    read_key only_key
    |    check_key option_key (constraint_key constraint_name)?
    )
        -> ^(with_key read_key? check_key? constraint_name?)
    ;

sample_clause
    :    sample_key block_key? 
        LEFT_PAREN e1=expression (COMMA e2=expression)? RIGHT_PAREN
        seed_part?
        -> ^(sample_key block_key? ^(EXPR $e1) ^(EXPR $e2)? seed_part?)
    ;

seed_part
    :    seed_key LEFT_PAREN expression RIGHT_PAREN
        -> ^(seed_key ^(EXPR expression))
    ;

// $>

// $<Expression & Condition
cursor_expression
    :    cursor_key LEFT_PAREN subquery RIGHT_PAREN
    ;

expression_list
    :    LEFT_PAREN expression? (COMMA expression)* RIGHT_PAREN
        -> ^(EXPR_LIST ^(EXPR expression)*)
    ;

condition
    :     expression
    ;

condition_wrapper
    :    expression
        -> ^(LOGIC_EXPR expression)
    ;

expression
    :    (cursor_key LEFT_PAREN (select_key|with_key)) => cursor_expression
    |    logical_and_expression ( or_key^ logical_and_expression )*
    ;

expression_wrapper
    :    expression
        -> ^(EXPR expression)
    ;

logical_and_expression
    :    negated_expression ( and_key^ negated_expression )*
    ;

negated_expression
    :    not_key^ negated_expression
    |    equality_expression
    ;

equality_expression
@init    {    int isNegated = false;    }
    :    (multiset_comparsion -> multiset_comparsion)
    (    is_key (not_key {isNegated = true;})?
        (    null_key
                -> {isNegated}? ^(IS_NOT_NULL $equality_expression)
                -> ^(IS_NULL $equality_expression)
        |    nan_key
                -> {isNegated}? ^(IS_NOT_NAN $equality_expression)
                -> ^(IS_NAN $equality_expression)
        |    present_key
                -> {isNegated}? ^(IS_NOT_PRESENT $equality_expression)
                -> ^(IS_PRESENT $equality_expression)
        |    infinite_key
                -> {isNegated}? ^(IS_NOT_INFINITE $equality_expression)
                -> ^(IS_INFINITE $equality_expression)
        |    a_key set_key
                -> {isNegated}? ^(IS_NOT_A_SET $equality_expression)
                -> ^(IS_A_SET $equality_expression)
        |    empty_key
                -> {isNegated}? ^(IS_NOT_EMPTY $equality_expression)
                -> ^(IS_EMPTY $equality_expression)
        |    of_key type_key? LEFT_PAREN only_key? type_spec (COMMA type_spec)* RIGHT_PAREN
                -> {isNegated}? ^(IS_NOT_OF_TYPE $equality_expression type_spec+)
                -> ^(IS_OF_TYPE $equality_expression type_spec+)
        )
    )?
    ;


multiset_comparsion
    :    (relational_expression -> relational_expression)
    (    multiset_type of_key? concatenation
        -> ^(multiset_type $multiset_comparsion ^(EXPR concatenation)))?
    ;

multiset_type
    :    member_key
    |    submultiset_key
    ;

relational_expression
    :    compound_expression
    ( ( EQUALS_OP^ | not_equal_op^ | LESS_THAN_OP^ | GREATER_THAN_OP^ | less_than_or_equals_op^ | greater_than_or_equals_op^ ) compound_expression)*
    ;

compound_expression
@init    {    int isNegated = false;    }
    :    (concatenation -> concatenation)
    (    (not_key {isNegated = true;} )?
        (    in_key in_elements
                -> {isNegated}? ^(NOT_IN $compound_expression in_elements)
                -> ^(in_key $compound_expression in_elements)
        |    between_key between_elements
                -> {isNegated}? ^(NOT_BETWEEN $compound_expression between_elements)
                -> ^(between_key $compound_expression between_elements)
        |    like_type concatenation like_escape_part?
                -> {isNegated}? ^(NOT_LIKE $compound_expression ^(EXPR concatenation) like_escape_part?)
                -> ^(like_type $compound_expression  ^(EXPR concatenation) like_escape_part?)
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
        -> ^(EXPR concatenation)
    ;

in_elements
@init    {    int mode = 0;    }
    :    (LEFT_PAREN+ (select_key|with_key)) =>  LEFT_PAREN subquery RIGHT_PAREN
         -> subquery
    |    LEFT_PAREN concatenation_wrapper (COMMA concatenation_wrapper)* RIGHT_PAREN
         -> ^(EXPR_LIST concatenation_wrapper+)
    |    constant
         -> ^(EXPR_LIST constant)
    |    bind_variable
         -> ^(EXPR_LIST bind_variable)
    |    general_element
         -> ^(EXPR_LIST general_element)
    ;

between_elements
    :    cn1=concatenation and_key cn2=concatenation
        -> ^(EXPR $cn1) ^(EXPR $cn2) 
    ;

concatenation
    :    additive_expression (concatenation_op^ additive_expression)*
    ;

concatenation_wrapper
    :    concatenation
        -> ^(EXPR concatenation)
    ;

additive_expression
    :    multiply_expression ( ( PLUS_SIGN^ | MINUS_SIGN^ ) multiply_expression)*
    ;

multiply_expression
    :    datetime_expression ( ( ASTERISK^ | SOLIDUS^ ) datetime_expression)*
    ;

datetime_expression
    :    (model_expression -> model_expression)
    (    at_key (local_key|time_key zone_key concatenation_wrapper)
            -> ^(DATETIME_OP $datetime_expression ^(at_key local_key? time_key? concatenation_wrapper))
    |    (interval_expression)=> interval_expression
            -> ^(DATETIME_OP $datetime_expression interval_expression)
    )?
    ;

interval_expression
    :    day_key    (LEFT_PAREN cn1=concatenation_wrapper RIGHT_PAREN)? to_key second_key (LEFT_PAREN cn2=concatenation_wrapper RIGHT_PAREN)?
        -> ^(day_key second_key $cn1? $cn2?)
    |    year_key (LEFT_PAREN concatenation_wrapper RIGHT_PAREN)? to_key month_key
        -> ^(year_key month_key concatenation_wrapper)
    ;

model_expression
    :    (multiset_expression -> multiset_expression)
        (LEFT_BRACKET model_expression_element RIGHT_BRACKET
            -> ^(MODEL_EXPRESSION[$LEFT_BRACKET] $model_expression model_expression_element))?
    ;

model_expression_element
    :    ((any_key)=> any_key|condition_wrapper) (COMMA! ((any_key)=> any_key|condition_wrapper))*
    |    single_column_for_loop (COMMA! single_column_for_loop)*
    |    multi_column_for_loop
    ;

single_column_for_loop
@init    {    int mode = 0;    }
    :    for_key column_name 
    (    in_key expression_list {mode = 1;}
    |    for_like_part? from_key ex1=expression
            to_key ex2=expression for_increment_decrement_type ex3=expression     
    )
        ->{mode == 1}? ^(FOR_SINGLE_COLUMN[$for_key.start] column_name ^(in_key expression_list))
        -> ^(FOR_SINGLE_COLUMN[$for_key.start] column_name ^(from_key ^(EXPR $ex1)) ^(to_key ^(EXPR $ex2)) ^(for_increment_decrement_type ^(EXPR $ex3)) for_like_part?) 
    ;

for_like_part
    :    like_key expression
        -> ^(like_key ^(EXPR expression))
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
        -> ^(FOR_MULTI_COLUMN[$for_key.start] column_name+ ^(in_key subquery? expression_list*))
    ;

multiset_expression
    :    unary_expression
         ( multiset_op^ unary_expression )*
    ;

unary_expression
options
{
backtrack=true;
}
    :    MINUS_SIGN unary_expression -> ^(UNARY_OPERATOR[$MINUS_SIGN] unary_expression)
    |    PLUS_SIGN unary_expression -> ^(UNARY_OPERATOR[$PLUS_SIGN] unary_expression)
    |    prior_key^ unary_expression
    |    connect_by_root_key^ unary_expression
    |    {LT(1)->getText() == "NEW" && LT(2)->getText() != "."}?=> new_key^ unary_expression
    |    distinct_key^ unary_expression
    |    all_key^ unary_expression
    |    {(LA(1) == SQL92_RESERVED_CASE || LA(2) == SQL92_RESERVED_CASE)}? case_statement[false]
    |    quantified_expression
    |    standard_function -> ^(STANDARD_FUNCTION standard_function)
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
        -> ^(SIMPLE_CASE[$ck1.start] label_name* ^(EXPR atom) simple_case_when_part+ case_else_part?)  
    ;

simple_case_when_part
    :    when_key^ ew1=expression_wrapper then_key! ({$case_statement::isStatement}? seq_of_statements | ew2=expression_wrapper)
    ;

searched_case_statement
    :    label_name? ck1=case_key
        searched_case_when_part+
        case_else_part?
        end_key case_key? label_name?
        -> ^(SEARCHED_CASE[$ck1.start] label_name* searched_case_when_part+ case_else_part?) 
    ;

searched_case_when_part
    :    when_key^ condition_wrapper then_key! ({$case_statement::isStatement}? seq_of_statements | expression_wrapper)
    ;

case_else_part
    :    else_key^ ({$case_statement::isStatement}? seq_of_statements | expression_wrapper)
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
    |    LEFT_PAREN!
         (
              ( select_key | with_key)=> subquery RIGHT_PAREN! subquery_operation_part*
              | expression_or_vector RIGHT_PAREN!
         )
    ;

expression_or_vector
@init    {    int mode = 0;    }
    :    expression (vector_expr {mode = 1;})?
        -> {mode == 1}? ^(VECTOR_EXPR ^(EXPR expression) vector_expr)
        -> expression
    ;

vector_expr
    :    COMMA expression (COMMA expression)*
        -> ^(EXPR expression)+
    ;

quantified_expression
    :    ( some_key^ | exists_key^ | all_key^ | any_key^ )
         ( (LEFT_PAREN (select_key|with_key)) => LEFT_PAREN! subquery RIGHT_PAREN!
           | LEFT_PAREN! expression_wrapper RIGHT_PAREN!
         )
    ;

standard_function
    :    stantard_function_enabling_over^ function_argument_analytic over_clause?
    |    stantard_function_enabling_using^ function_argument_modeling using_clause?
    |    count_key^
            LEFT_PAREN!
                ( ASTERISK | (distinct_key|unique_key|all_key)? concatenation_wrapper )
            RIGHT_PAREN! over_clause?
    |    (cast_key^|xmlcast_key^) 
            LEFT_PAREN!
            ( (multiset_key LEFT_PAREN+ (select_key|with_key)) => (multiset_key! LEFT_PAREN! subquery order_by_clause? RIGHT_PAREN!)
              (as_key! type_spec)?
            | concatenation_wrapper as_key! type_spec
            )
            RIGHT_PAREN!
    |    chr_key^
            LEFT_PAREN! 
                concatenation_wrapper using_key! nchar_cs_key 
            RIGHT_PAREN!
    |    collect_key^
            LEFT_PAREN! 
                (distinct_key|unique_key)? concatenation_wrapper collect_order_by_part?
            RIGHT_PAREN!
    |    stantard_function_enabling_within_or_over^ 
            function_argument within_or_over_part+
    |    decompose_key^
            LEFT_PAREN! 
                concatenation_wrapper (canonical_key|compatibility_key)? 
            RIGHT_PAREN!
    |    extract_key^
            LEFT_PAREN!
                REGULAR_ID from_key! concatenation_wrapper 
            RIGHT_PAREN!
    |    (first_value_key^|last_value_key^) function_argument_analytic
             respect_or_ignore_nulls? over_clause
    |    stantard_function_pedictions^
            LEFT_PAREN!
                expression_wrapper (COMMA expression_wrapper)* cost_matrix_clause? using_clause? 
            RIGHT_PAREN!
    |    translate_key^
            LEFT_PAREN! 
                expression_wrapper (using_key! (char_cs_key|nchar_cs_key))? 
                    (COMMA expression_wrapper)* 
            RIGHT_PAREN!
    |    treat_key^
            LEFT_PAREN!
                expression_wrapper as_key! ref_key? type_spec 
            RIGHT_PAREN!
    |    trim_key^
            LEFT_PAREN!
                ((leading_key|trailing_key|both_key)? quoted_string? from_key)?
                concatenation_wrapper
            RIGHT_PAREN!
    |    xmlagg_key^
            LEFT_PAREN! 
                expression_wrapper order_by_clause? 
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    |    (xmlcolattval_key^|xmlforest_key^) 
            LEFT_PAREN!
                xml_multiuse_expression_element (COMMA! xml_multiuse_expression_element)*
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    |    xmlelement_key^
            LEFT_PAREN!
                (entityescaping_key|noentityescaping_key)?
                (name_key|evalname_key)? expression_wrapper
                ({LT(2)->getText() == "XMLATTRIBUTES"}? COMMA! xml_attributes_clause)?
                (COMMA! expression_wrapper column_alias?)*
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    |    xmlexists_key^
            LEFT_PAREN!
                expression_wrapper
                xml_passing_clause?
            RIGHT_PAREN!
    |    xmlparse_key^
            LEFT_PAREN! 
                (document_key|content_key) concatenation_wrapper wellformed_key?
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    |    xmlpi_key^
            LEFT_PAREN! 
                (    name_key id
                |    evalname_key concatenation_wrapper
                )
                (COMMA! concatenation_wrapper)?
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    |    xmlquery_key^
            LEFT_PAREN! 
                concatenation_wrapper xml_passing_clause?
                returning_key! content_key! (null_key on_key! empty_key!)?
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    |    xmlroot_key^
            LEFT_PAREN!
                concatenation_wrapper
                    xmlroot_param_version_part
                    (COMMA! xmlroot_param_standalone_part)?
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    |    xmlserialize_key^
            LEFT_PAREN!
                (document_key|content_key)
                concatenation_wrapper (as_key! type_spec)?
                xmlserialize_param_enconding_part?
                xmlserialize_param_version_part?
                xmlserialize_param_ident_part?
                ((hide_key|show_key) defaults_key)?
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    |    xmltable_key^
            LEFT_PAREN!
                xml_namespaces_clause?
                concatenation_wrapper
                xml_passing_clause?
                (columns_key! xml_table_column (COMMA! xml_table_column))?
            RIGHT_PAREN!
            (PERIOD general_element_part)?
    ;

stantard_function_enabling_over
    :    {enablesOverClause(LT(1)->getText())}?=> REGULAR_ID -> FUNCTION_ENABLING_OVER[$REGULAR_ID]
    ;

stantard_function_enabling_using
    :    {enablesUsingClause(LT(1)->getText())}?=> REGULAR_ID -> FUNCTION_ENABLING_USING[$REGULAR_ID]
    ;

stantard_function_enabling_within_or_over
    :    {enablesWithinOrOverClause(LT(1)->getText())}?=> REGULAR_ID -> FUNCTION_ENABLING_WITHIN_OR_OVER[$REGULAR_ID]
    ;

stantard_function_pedictions
    :    {isStandardPredictionFunction(LT(1)->getText())}?=> REGULAR_ID -> PREDICTION_FUNCTION[$REGULAR_ID]
    ;

over_clause
    :    over_key^
        LEFT_PAREN!
            query_partition_clause?
            (order_by_clause windowing_clause?)?
        RIGHT_PAREN!
    ;

windowing_clause
@init    {    int mode = 0;    }
    :    windowing_type
    (    between_key windowing_elements and_key windowing_elements {mode = 1;}
    |    windowing_elements    )
    ->{mode == 1}? ^(windowing_type ^(between_key windowing_elements+))
    -> ^(windowing_type windowing_elements+)   
    ;

windowing_type
    :    rows_key
    |    range_key
    ;

windowing_elements
    :    unbounded_key^ preceding_key
    |    current_key^ row_key
    |    concatenation_wrapper (preceding_key^|following_key^)
    ;

using_clause
    :    using_key^
    (    ASTERISK
    |    using_element (COMMA! using_element)*
    )
    ;

using_element
    :    (in_key out_key?|out_key)? select_list_elements column_alias?
        -> ^(ELEMENT in_key? out_key? select_list_elements column_alias?)
    ;

collect_order_by_part
    :    order_key^ by_key! concatenation_wrapper
    ;

within_or_over_part
    :    within_key^ group_key! LEFT_PAREN! order_by_clause RIGHT_PAREN!
    |    over_clause
    ;

cost_matrix_clause
    :    cost_key^
    (    model_key auto_key?
    |    LEFT_PAREN! cost_class_name (COMMA! cost_class_name)* RIGHT_PAREN! values_key! 
            expression_list
    )
    ;

xml_passing_clause
    :    passing_key^ (by_key! value_key)?
            expression_wrapper column_alias? (COMMA! expression_wrapper column_alias?)
    ;

xml_attributes_clause
    :    xmlattributes_key^
        LEFT_PAREN!
            (entityescaping_key|noentityescaping_key)?
            (schemacheck_key|noschemacheck_key)?
            xml_multiuse_expression_element (COMMA! xml_multiuse_expression_element)*
        RIGHT_PAREN!
    ;

xml_namespaces_clause
    :    xmlnamespaces_key^
        LEFT_PAREN!
            (concatenation_wrapper column_alias)?
                (COMMA! concatenation_wrapper column_alias)*
            ((default_key)=> xml_general_default_part)?
        RIGHT_PAREN!
    ;

xml_table_column
    :    xml_column_name
    (    for_key ordinality_key
    |    type_spec (path_key concatenation_wrapper)? ((default_key)=> xml_general_default_part)?
    )
        -> ^(XML_COLUMN xml_column_name ordinality_key? type_spec? concatenation_wrapper? xml_general_default_part?)
    ;

xml_general_default_part
    :    default_key^ concatenation_wrapper
    ;

xml_multiuse_expression_element
@init    {    int mode = 0;    }
    :    expression (as_key (id_expression {mode = 1;}|evalname_key concatenation {mode = 2;}))?
        -> {mode == 1}? ^(XML_ELEMENT ^(EXPR expression) ^(XML_ALIAS[$as_key.start] id_expression))
        -> {mode == 2}? ^(XML_ELEMENT ^(EXPR expression) ^(XML_ALIAS[$as_key.start] ^(evalname_key ^(EXPR concatenation))))
        -> ^(XML_ELEMENT ^(EXPR expression))
    ;

xmlroot_param_version_part
    :    version_key^ (no_key value_key|expression_wrapper)
    ;

xmlroot_param_standalone_part
    :    standalone_key^ (yes_key|no_key value_key?)
    ;

xmlserialize_param_enconding_part
    :    encoding_key^ concatenation_wrapper
    ;

xmlserialize_param_version_part
    :    version_key^ concatenation_wrapper
    ;

xmlserialize_param_ident_part
    :    no_key indent_key
    |    indent_key^ (size_key! EQUALS_OP! concatenation_wrapper)?
    ;

// $>
