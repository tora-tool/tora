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
parser grammar PLSQLParser;

options {
    tokenVocab=PLSQLLexer;
    language=Cpp;
    //memoize=true;
    //output=AST;
}

import PLSQLKeys, PLSQLCommons, PLSQL_DMLParser, SQLPLUSParser;

@parser::includes
{
	#include "UserTraits.hpp"
	#include "PLSQLLexer.hpp"
}
@parser::namespace { Antlr3BackendImpl }

@header {
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
}

swallow_to_semi
    :    ~( SEMICOLON )+
    ;

compilation_unit
    :    unit_statement* EOF
    ;

sql_script
    :   (unit_statement|sql_plus_command)* EOF
    ;

unit_statement
options{
backtrack=true;
}    :    alter_function
    |    alter_package
    |    alter_procedure
    |    alter_sequence
    |    alter_trigger
    |    alter_type

    |    create_function_body
    |    create_procedure_body
    |    create_package

//    |    create_index //TODO
//    |    create_table //TODO
//    |    create_view //TODO
//    |    create_directory //TODO
//    |    create_materialized_view //TODO

    |    create_sequence
    |    create_trigger
    |    create_type

    |    drop_function
    |    drop_package
    |    drop_procedure
    |    drop_sequence
    |    drop_trigger
    |    drop_type
    |    data_manipulation_language_statements
    ;

// $<DDL -> SQL Statements for Stored PL/SQL Units

// $<Function DDLs

drop_function
    :    drop_key function_key function_name
        SEMICOLON
    ;

alter_function
    :    alter_key function_key function_name
        compile_key debug_key? compiler_parameters_clause*
        (reuse_key settings_key)?
        SEMICOLON
    ;

create_function_body
    :    (create_key ( or_key replace_key )?)? function_key function_name 
              ( LEFT_PAREN parameter (COMMA parameter)* RIGHT_PAREN)?
        return_key type_spec
                (invoker_rights_clause|parallel_enable_clause|result_cache_clause|deterministic_key)*
        (
            (    pipelined_key? ( is_key | as_key )
                      (    declare_key? declare_spec* body 
                      |    call_spec)
            )
        |    (pipelined_key|aggregate_key) using_key implementation_type_name
        )
              SEMICOLON
    ;

// $<Creation Function - Specific Clauses

parallel_enable_clause
    :    parallel_enable_key partition_by_clause?
    ;

partition_by_clause
    :    LEFT_PAREN
            partition_key expression by_key 
            (    any_key 
            |    (hash_key|range_key) LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
            )
            streaming_clause?
        RIGHT_PAREN
    ;

result_cache_clause
    :    result_cache_key relies_on_part?
    ;

relies_on_part
    :    relies_on_key LEFT_PAREN tableview_name (COMMA tableview_name)* RIGHT_PAREN
    ;

streaming_clause
    :    (order_key|cluster_key) expression by_key 
        LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
    ;
// $>
// $>

// $<Package DDLs

drop_package
    :    drop_key package_key body_key? package_name
        SEMICOLON
    ;

alter_package
    :    alter_key package_key package_name
        compile_key debug_key? (package_key|body_key|specification_key)?
        compiler_parameters_clause*
        (reuse_key settings_key)?
        SEMICOLON
    ;

create_package
    :    create_key ( or_key replace_key )? package_key
        ( package_spec | package_body )?
        SEMICOLON
    ;

// $<Create Package - Specific Clauses

package_body
    :    body_key package_name (is_key | as_key)
        package_obj_body*
        (begin_key seq_of_statements|end_key package_name?)
    ;

package_spec
    :    package_name invoker_rights_clause? (is_key | as_key)
        package_obj_spec*
        end_key package_name?
    ;

package_obj_spec
options{
backtrack=true;
}    :    variable_declaration
    |     subtype_declaration
    |     cursor_declaration
    |     exception_declaration
    |     record_declaration
    |     table_declaration
    |     procedure_spec
    |     function_spec
    ;

procedure_spec
    :     procedure_key procedure_name 
        ( LEFT_PAREN parameter ( COMMA parameter )* RIGHT_PAREN )? SEMICOLON 
    ;

function_spec
    :    function_key function_name 
        (LEFT_PAREN parameter ( COMMA parameter)* RIGHT_PAREN )?
        return_key type_spec SEMICOLON 
    ;

package_obj_body
options{
backtrack=true;
}    :     variable_declaration 
    |     subtype_declaration 
    |     cursor_declaration 
    |     exception_declaration 
    |     record_declaration
    |     table_declaration
    |     create_procedure_body
    |     create_function_body 
    ;

// $>

// $>

// $<Procedure DDLs

drop_procedure
    :    drop_key procedure_key procedure_name
        SEMICOLON
    ;

alter_procedure
    :    alter_key procedure_key procedure_name
        compile_key debug_key? compiler_parameters_clause*
        (reuse_key settings_key)?
        SEMICOLON
    ;

create_procedure_body
    :    (create_key ( or_key replace_key )?)? procedure_key procedure_name
              ( LEFT_PAREN parameter ( COMMA parameter )* RIGHT_PAREN )? 
              invoker_rights_clause?
        ( is_key | as_key )
              (    declare_key? declare_spec* body 
              |    call_spec 
              |    external_key
              )
              SEMICOLON
      ;

// $>

// $<Trigger DDLs

drop_trigger
    :    drop_key trigger_key trigger_name
        SEMICOLON
    ;

alter_trigger
    :    alter_key trigger_key tn1=trigger_name
    (    (enable_key|disable_key)
    |    rename_key to_key tn2=trigger_name
    |    compile_key debug_key? compiler_parameters_clause* (reuse_key settings_key)?
    )    SEMICOLON
    ;

create_trigger
    :    create_key ( or_key replace_key )? trigger_key trigger_name
    (    simple_dml_trigger
    |    compound_dml_trigger
    |    non_dml_trigger
    )
    trigger_follows_clause?
    (enable_key|disable_key)?
    trigger_when_clause? 
    trigger_body SEMICOLON
    ;

trigger_follows_clause
    :    follows_key trigger_name (COMMA trigger_name)*
    ;

trigger_when_clause
    :    when_key LEFT_PAREN condition RIGHT_PAREN
    ;

// $<Create Trigger- Specific Clauses
simple_dml_trigger
    :    (before_key|after_key|instead_key of_key) dml_event_clause referencing_clause? for_each_row?
    ;

for_each_row
    :    for_key each_key row_key
    ;

compound_dml_trigger
    :    for_key dml_event_clause referencing_clause?
    ;

non_dml_trigger
    :    (before_key|after_key) non_dml_event (or_key non_dml_event)*
        on_key (database_key | (schema_name PERIOD)? schema_key )
    ;

trigger_body
    :    (compound_key trigger_key)=> compound_trigger_block
    |    (call_key id)=> call_key routine_clause 
    |    trigger_block
    ;

routine_clause
    :    routine_name function_argument?
    ;

compound_trigger_block
    :    compound_key trigger_key declare_spec* timing_point_section+ end_key trigger_name
    ;

timing_point_section
options{
k=3;
}    :    bk=before_key statement_key is_key trigger_block before_key statement_key SEMICOLON
    |    bk=before_key each_key row_key is_key trigger_block before_key each_key row_key SEMICOLON
    |    ak=after_key statement_key is_key trigger_block after_key statement_key SEMICOLON
    |    ak=after_key each_key row_key is_key trigger_block after_key each_key row_key SEMICOLON
    ;

non_dml_event
    :    alter_key
    |    analyze_key
    |    associate_key statistics_key
    |    audit_key
    |    comment_key
    |    create_key
    |    disassociate_key statistics_key
    |    drop_key
    |    grant_key
    |    noaudit_key
    |    rename_key
    |    revoke_key
    |    truncate_key
    |    ddl_key
    |    startup_key
    |    shutdown_key
    |    db_role_change_key
    |    logon_key
    |    logoff_key
    |    servererror_key
    |    suspend_key
    |    database_key
    |    schema_key
    |    follows_key
    ;

dml_event_clause
    :    dml_event_element (or_key dml_event_element)*
        on_key 
        dml_event_nested_clause? tableview_name
    ;

dml_event_element
    :    (delete_key|insert_key|update_key) (of_key column_name (COMMA column_name)*)?
    ;

dml_event_nested_clause
    :    nested_key table_key tableview_name of_key
    ;

referencing_clause
    :    referencing_key referencing_element+
    ;

referencing_element
    :    ( new_key | old_key | parent_key ) column_alias
    ;

// $>
// $>

// $<Type DDLs

drop_type
    :    drop_key type_key body_key? type_name (force_key|validate_key)?
        SEMICOLON
    ;

alter_type
    :    alter_key type_key type_name
    (    compile_type_clause
    |    replace_type_clause
    |    {equalsIgnoreCase(LT(2)->getText(), "ATTRIBUTE")}? alter_attribute_definition
    |    alter_method_spec
    |    alter_collection_clauses
    |    modifier_clause
    )
        dependent_handling_clause?
        SEMICOLON
    ;

// $<Alter Type - Specific Clauses
compile_type_clause
    :    compile_key debug_key? (specification_key|body_key)? compiler_parameters_clause* (reuse_key settings_key)?
    ;

replace_type_clause
    :    replace_key invoker_rights_clause? as_key object_key
        LEFT_PAREN object_member_spec (COMMA object_member_spec)* RIGHT_PAREN
    ;

alter_method_spec
    :     alter_method_element (COMMA alter_method_element)*
    ;

alter_method_element
    :    (add_key|drop_key) (map_order_function_spec|subprogram_spec)
    ;

alter_attribute_definition
    :    (add_key|modify_key|drop_key) attribute_key
        (    attribute_definition
        |    LEFT_PAREN attribute_definition (COMMA attribute_definition)* RIGHT_PAREN
        )
    ;

attribute_definition
    :    attribute_name type_spec?
    ;

alter_collection_clauses
    :    modify_key
    (    limit_key expression
    |    element_key type_key type_spec
    )
    ;

dependent_handling_clause
    :    invalidate_key
    |    cascade_key
        (    convert_key to_key substitutable_key
        |    not_key? including_key table_key data_key
        )?
        dependent_exceptions_part?
    ;

dependent_exceptions_part
    :    force_key? exceptions_key into_key tableview_name
    ;

// $>

create_type
    :    create_key (or_key replace_key)? type_key
        ( type_definition | type_body)
        SEMICOLON
    ;

// $<Create Type - Specific Clauses
type_definition
    :    type_name 
        (oid_key CHAR_STRING)?
        object_type_def?
    ;

object_type_def
    :    invoker_rights_clause?
    (    object_as_part
    |    object_under_part
    )
        sqlj_object_type?
        (LEFT_PAREN object_member_spec (COMMA object_member_spec)* RIGHT_PAREN)?
        modifier_clause*
    ;

object_as_part
    :    (is_key|as_key) (object_key|varray_type_def|nested_table_type_def)
    ;

object_under_part
    :    under_key type_spec
    ;

nested_table_type_def
    :    table_key of_key type_spec
        (not_key null_key)?
    ;

sqlj_object_type
    :    external_key name_key expression language_key java_key using_key (sqldata_key|customdatum_key|oradata_key)
    ;

type_body
    :    body_key type_name
        (is_key|as_key) type_body_elements (COMMA type_body_elements)*
        end_key
    ;

type_body_elements
    :    map_order_func_declaration
    |    subprog_decl_in_type
    ;

map_order_func_declaration
    :    (map_key|order_key) member_key func_decl_in_type
    ;

subprog_decl_in_type
    :    (member_key|static_key)
    (    proc_decl_in_type
    |    func_decl_in_type
    |    constructor_declaration)
    ;

proc_decl_in_type
    :    procedure_key procedure_name
        LEFT_PAREN type_elements_parameter (COMMA type_elements_parameter)* RIGHT_PAREN
        (is_key|as_key) 
            (call_spec|declare_key? declare_spec* body SEMICOLON)
    ;

func_decl_in_type
    :    function_key function_name 
        (LEFT_PAREN type_elements_parameter (COMMA type_elements_parameter)* RIGHT_PAREN)? 
        return_key type_spec
        (is_key|as_key)
            (call_spec |declare_key? declare_spec* body SEMICOLON)
    ;

constructor_declaration
    :    final_key? instantiable_key? constructor_key function_key type_spec
        (LEFT_PAREN (self_key in_key out_key type_spec COMMA) type_elements_parameter (COMMA type_elements_parameter)*  RIGHT_PAREN)?
        return_key self_key as_key result_key 
        (is_key|as_key) 
            (call_spec |declare_key? declare_spec* body SEMICOLON)
    ;

// $>

// $<Common Type Clauses

modifier_clause
    :    not_key? (instantiable_key|final_key|overriding_key)
    ;

object_member_spec
options{
backtrack=true;
}    :    id type_spec sqlj_object_type_attr?
    |    element_spec
    ;

sqlj_object_type_attr
    :    external_key name_key expression
    ;

element_spec
    :    modifier_clause?
        element_spec_options+
        (COMMA pragma_clause)?
    ;

element_spec_options
    :    subprogram_spec
    |    constructor_spec
    |    map_order_function_spec
    ;

subprogram_spec
    :    (member_key|static_key)
        (type_procedure_spec|type_function_spec)
    ;

type_procedure_spec
    :    procedure_key procedure_name
        LEFT_PAREN type_elements_parameter (COMMA type_elements_parameter)* RIGHT_PAREN
        ((is_key|as_key) call_spec )?
    ;

type_function_spec
    :    function_key function_name 
        (LEFT_PAREN type_elements_parameter (COMMA type_elements_parameter)* RIGHT_PAREN)? 
        return_key ( type_spec | self_key as_key result_key)
        ((is_key|as_key) call_spec | external_key variable_key? name_key expression)?
    ;

constructor_spec
    :    final_key? instantiable_key? constructor_key function_key type_spec
        (LEFT_PAREN (self_key in_key out_key type_spec COMMA) type_elements_parameter (COMMA type_elements_parameter)*  RIGHT_PAREN)?
        return_key self_key as_key result_key ((is_key|as_key) call_spec)?
    ;

map_order_function_spec
    :    (map_key|order_key) member_key type_function_spec
    ;

pragma_clause
    :    pragma_key restrict_references_key LEFT_PAREN pragma_elements (COMMA pragma_elements)* RIGHT_PAREN
    ;

pragma_elements
    :    id
    |    default_key
    ;

type_elements_parameter
    :    parameter_name type_spec
    ;

// $>
// $>

// $<Sequence DDLs

drop_sequence
    :   drop_key sequence_key sequence_name
        SEMICOLON
    ;

alter_sequence
    :    alter_key sequence_key sequence_name sequence_spec+
         SEMICOLON
    ;

create_sequence
    :    create_key sequence_key sequence_name
    (    sequence_start_clause
    |    sequence_spec
    )*   SEMICOLON
    ;

// $<Common Sequence

sequence_spec
    :    increment_key by_key UNSIGNED_INTEGER
    |    maxvalue_key UNSIGNED_INTEGER
    |    nomaxvalue_key
    |    minvalue_key UNSIGNED_INTEGER
    |    nominvalue_key
    |    cycle_key
    |    nocycle_key
    |    cache_key UNSIGNED_INTEGER
    |    nocache_key
    |    order_key
    |    noorder_key
    ;

sequence_start_clause
    :    start_key with_key UNSIGNED_INTEGER
    ;

// $>
// $>

// $<Common DDL Clauses

invoker_rights_clause
    :    authid_key (current_user_key|definer_key)
    ;

compiler_parameters_clause
    :    id EQUALS_OP expression
    ;

call_spec
    :    language_key ( java_spec | c_spec )
    ;

// $<Call Spec - Specific Clauses

java_spec
    :    java_key name_key CHAR_STRING
    ;

c_spec
    :    c_key (name_key CHAR_STRING)? 
        library_key id
        c_agent_in_clause? (with_key context_key)? c_parameters_clause?
    ;

c_agent_in_clause
    :    agent_key in_key LEFT_PAREN expression (COMMA expression)* RIGHT_PAREN
    ;

c_parameters_clause
    :    parameters_key LEFT_PAREN (expression (COMMA expression)* | DOUBLE_PERIOD PERIOD) RIGHT_PAREN
    ;

// $>

parameter
    :    parameter_name ( in_key | out_key | inout_key | nocopy_key)* type_spec? default_value_part?
    ;

default_value_part
    :    (ASSIGN_OP|default_key) expression
    ;

// $>

// $>

// $<PL/SQL Elements Declarations

declare_spec
options{
backtrack=true;
}    :    variable_declaration
    |     subtype_declaration
    |     cursor_declaration
    |     exception_declaration
    |     pragma_declaration
    |     record_declaration
    |     table_declaration
    |     create_procedure_body
    |     create_function_body
    ;

//incorporates constant_declaration
variable_declaration
    :    variable_name constant_key?
        type_spec (not_key null_key)? 
        default_value_part? SEMICOLON
    ;    

subtype_declaration
      :    subtype_key type_name is_key type_spec
          (range_key expression DOUBLE_PERIOD expression)?
          (not_key null_key)? SEMICOLON
      ;

//cursor_declaration incorportates curscursor_body and cursor_spec
cursor_declaration
    :    cursor_key cursor_name
              (LEFT_PAREN parameter_spec (COMMA parameter_spec)* RIGHT_PAREN )?
              (return_key type_spec)? (is_key select_statement)? SEMICOLON
    ;

parameter_spec
    :    parameter_name (in_key? type_spec)? 
        default_value_part?
    ;

exception_declaration 
    :    exception_name exception_key SEMICOLON
    ;             

pragma_declaration
    :    pragma_key
        (    serially_reusable_key 
        |    autonomous_transaction_key
        |    exception_init_key LEFT_PAREN exception_name COMMA numeric RIGHT_PAREN 
        |    inline_key LEFT_PAREN id1=id COMMA expression RIGHT_PAREN
        |    restrict_references_key LEFT_PAREN (id|default_key) (COMMA id)+ RIGHT_PAREN
        )
        SEMICOLON
    ;

record_declaration
    :    record_type_dec
    |    record_var_dec
    ;

// $<Record Declaration - Specific Clauses

//incorporates ref_cursor_type_definition
record_type_dec
    :    type_key type_name is_key 
    (    record_key LEFT_PAREN field_spec ( COMMA field_spec )* RIGHT_PAREN
    |    ref_key cursor_key (return_key type_spec)?
    )    SEMICOLON
    ;

field_spec
    :    column_name type_spec?
        (not_key null_key)?
        default_value_part?
    ;

record_var_dec
    :    record_name type_name (percent_rowtype_key|percent_type_key) SEMICOLON
    ;

// $>

table_declaration
    :    (table_type_dec | table_var_dec) SEMICOLON
    ;

table_type_dec
    :    type_key type_name is_key 
    (    table_key
            of_key type_spec 
            table_indexed_by_part?
        (not_key null_key)?
    |    varray_type_def
    )
    ;

table_indexed_by_part
    :    (idx1=indexed_key|idx2=index_key) by_key type_spec
    ;

varray_type_def
    :    (varray_key| varying_key array_key)
        LEFT_PAREN expression RIGHT_PAREN 
        of_key type_spec
        (not_key null_key)?
    ;

table_var_dec
    :    table_var_name type_spec
    ;

// $>

// $<PL/SQL Statements

seq_of_statements returns[void* p, void*q]
    :     (statement (SEMICOLON|EOF)|label_declaration)+
    ;  

label_declaration
    :    ltp1=LESS_THAN_OP LESS_THAN_OP label_name GREATER_THAN_OP GREATER_THAN_OP
    ;

statement
options{
backtrack=true;
}
    :    create_key swallow_to_semi (SEMICOLON|EOF)
    |    alter_key swallow_to_semi  (SEMICOLON|EOF)
    |    grant_key swallow_to_semi  (SEMICOLON|EOF)
    |    truncate_key swallow_to_semi  (SEMICOLON|EOF)
    |    (begin_key) => body
    |    (declare_key) => block
    |    assignment_statement
    |    continue_statement
    |    exit_statement
    |    goto_statement
    |    if_statement
    |    loop_statement
    |    forall_statement
    |    null_statement
    |    raise_statement
    |    return_statement
    |    case_statement[true]
    |    sql_statement
    |    function_call
    ;

assignment_statement
    :     (general_element|bind_variable) ASSIGN_OP expression
    ;

continue_statement
    :    continue_key label_name? (when_key condition)?
    ;

exit_statement
    :    exit_key label_name? (when_key condition)?
    ;

goto_statement
    :    goto_key label_name
    ;

if_statement
    :    if_key condition then_key seq_of_statements
        elsif_part* else_part?
        end_key if_key
    ;

elsif_part
    :    elsif_key condition then_key seq_of_statements
    ;

else_part
    :    else_key seq_of_statements
    ;

loop_statement
    :    label_name?
        (while_key condition | for_key cursor_loop_param)?
        loop_key
        seq_of_statements
        end_key loop_key label_name?
    ;

// $<Loop - Specific Clause

cursor_loop_param
    :    (index_name in_key reverse_key? lower_bound DOUBLE_PERIOD)=> 
            index_name in_key reverse_key? lower_bound DOUBLE_PERIOD upper_bound
    |     record_name in_key ( cursor_name expression_list? | LEFT_PAREN select_statement RIGHT_PAREN)
    ;
// $>

forall_statement
    :    forall_key index_name in_key bounds_clause sql_statement (save_key exceptions_key)?
    ;

bounds_clause
    :    lower_bound DOUBLE_PERIOD upper_bound
    |    indices_key of_key collection_name between_bound?
    |    values_key of_key index_name
    ;

between_bound
    :    between_key lower_bound and_key upper_bound
    ;

lower_bound
    :    concatenation
    ;

upper_bound
    :    concatenation
    ;

null_statement
    :    null_key
    ;

raise_statement
    :    raise_key exception_name?
    ;

return_statement
    :    return_key cn1=condition?
    ;

function_call
    :    call_key? routine_name function_argument?
    ;

body
    :    begin_key
        seq_of_statements
        exception_clause?
        end_key label_name?
    ;

// $<Body - Specific Clause

exception_clause
    :    exception_key exception_handler+
    ;

exception_handler
    :    when_key exception_name (or_key exception_name)* then_key
        seq_of_statements
    ;

// $>

trigger_block
    :    (declare_key? declare_spec+)?
        body
    ;

block
    :    declare_key? declare_spec+
        body
    ;

// $>

// $<SQL PL/SQL Statements

sql_statement
options{
backtrack=true;
}    :    execute_immediate
    |    data_manipulation_language_statements
    |    cursor_manipulation_statements
    |    transaction_control_statements
    ;

execute_immediate
    :    execute_key immediate_key expression
    (    into_clause using_clause?
    |    using_clause dynamic_returning_clause?
    |    dynamic_returning_clause
    )?
    ;

// $<Execute Immediate - Specific Clause
dynamic_returning_clause
    :    (returning_key|return_key) into_clause
    ;
// $>


// $<DML SQL PL/SQL Statements

data_manipulation_language_statements
    :    merge_statement
    |    lock_table_statement
    |    select_statement
    |    update_statement
    |    delete_statement
    |    insert_statement
    |    explain_statement
    ;

// $>

// $<Cursor Manipulation SQL PL/SQL Statements

cursor_manipulation_statements
    :    close_statement
    |    open_statement
    |    fetch_statement
    |    open_for_statement
    ;

close_statement
    :     close_key cursor_name
    ;

open_statement
    :    open_key cursor_name expression_list?
    ;

fetch_statement
    :    fetch_key cursor_name 
    (    it1=into_key variable_name (COMMA variable_name )*
    |    bulk_key collect_key into_key variable_name (COMMA variable_name )*
    )
    ;

open_for_statement
    :    open_key variable_name for_key
    (    (select_key|with_key)=> select_statement
    |    expression
    )
        using_clause?
    ;

// $>

// $<Transaction Control SQL PL/SQL Statements

transaction_control_statements
    :    set_transaction_command
    |    set_constraint_command
    |    commit_statement
    |    rollback_statement
    |    savepoint_statement
    ;

set_transaction_command
    :    set_key transaction_key 
    (    read_key (only_key|write_key)
    |    isolation_key level_key (serializable_key|read_key committed_key)
    |    use_key rollback_key segment_key rollback_segment_name
    )?
        (name_key quoted_string)?
    ;

set_constraint_command
    :    set_key ( constraint_key | constraints_key )
        ( all_key | constraint_name (COMMA constraint_name)* )
        (immediate_key|deferred_key)
    ;

commit_statement
    :     commit_key work_key?
    (    comment_key expression
    |    force_key (corrupt_xid_key expression| corrupt_xid_all_key | expression (COMMA expression)?)
    )?
        write_clause?
    ;

write_clause
    :    write_key (wait_key|nowait_key)? (immediate_key|batch_key)?
    ;

rollback_statement
    :     rollback_key work_key?
    (     to_key savepoint_key? savepoint_name
    |    force_key quoted_string
    )?
    ;

savepoint_statement
    :    savepoint_key savepoint_name 
    ;

// $>
