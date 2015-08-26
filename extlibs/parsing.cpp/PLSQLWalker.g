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
tree grammar PLSQLWalker;

options {
    tokenVocab=PLSQLParser;
    ASTLabelType=CommonTree;
}

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
package br.com.porcelli.parser.plsql;
}

compilation_unit
    :    ^(COMPILATION_UNIT unit_statement*)
    ;

sql_script
    :    ^(SQL_SCRIPT serveroutput_declaration? seq_of_statements)
    ;

serveroutput_declaration
    :    ^(SET_SERVEROUTPUT (SQL92_RESERVED_ON|OFF_VK))
    ;

unit_statement
    :    alter_function
    |    alter_package
    |    alter_procedure
    |    alter_sequence
    |    alter_trigger
    |    alter_type
    |    create_function_body
    |    create_procedure_body
    |    create_package
    |    create_sequence
    |    create_trigger
    |    create_type
    |    drop_function
    |    drop_package
    |    drop_procedure
    |    drop_sequence
    |    drop_trigger
    |    drop_type
    ;

// $<DDL -> SQL Statements for Stored PL/SQL Units

// $<Function DDLs

drop_function
    :    ^(DROP_FUNCTION function_name)
    ;

alter_function
    :    ^(ALTER_FUNCTION function_name DEBUG_VK? REUSE_VK? compiler_parameters_clause*)
    ;

create_function_body
    :    ^(CREATE_FUNCTION REPLACE_VK? function_name type_spec ^(PARAMETERS parameter*)
            invoker_rights_clause* parallel_enable_clause* result_cache_clause* DETERMINISTIC_VK*
            (    ^(USING_MODE PIPELINED_VK? AGGREGATE_VK? implementation_type_name)
            |    ^(CALL_MODE PIPELINED_VK? call_spec)
            |    ^(BODY_MODE PIPELINED_VK? declare_spec* body)
            )
        )
    ;

// $<Creation Function - Specific Clauses

parallel_enable_clause
    :    ^(PARALLEL_ENABLE_VK partition_by_clause?)
    ;

partition_by_clause
    :    ^(PARTITION_VK expression 
            (    SQL92_RESERVED_ANY
            |    ^(HASH_VK ^(COLUMNS column_name+))
            |    ^(RANGE_VK ^(COLUMNS column_name+))
            ) 
            streaming_clause?
        )
    ;

result_cache_clause
    :    ^(RESULT_CACHE_VK relies_on_part?)
    ;

relies_on_part
    :    ^(RELIES_ON_VK tableview_name+)
    ;

streaming_clause
    :    ^(STREAMING_CLAUSE (SQL92_RESERVED_ORDER|CLUSTER_VK) expression ^(COLUMNS column_name+)) 
    ;
// $>
// $>

// $<Package DDLs

drop_package
    :    ^(DROP_PACKAGE package_name BODY_VK?)
    ;

alter_package
    :    ^(ALTER_PACKAGE package_name DEBUG_VK? REUSE_VK? 
                (PACKAGE_VK|BODY_VK|SPECIFICATION_VK)? compiler_parameters_clause*)
    ;

create_package
    :    ^(CREATE_PACKAGE_SPEC REPLACE_VK? package_name+ invoker_rights_clause? package_obj_spec*) 
    |    ^(CREATE_PACKAGE_BODY REPLACE_VK? package_name+ package_obj_body* seq_of_statements?)
    ;

// $<Create Package - Specific Clauses

package_obj_spec
    :    variable_declaration
    |     subtype_declaration
    |     cursor_declaration
    |     exception_declaration
    |     record_declaration
    |     table_declaration
    |     procedure_spec
    |     function_spec
    ;

procedure_spec
    :     ^(PROCEDURE_SPEC procedure_name ^(PARAMETERS parameter*)
            (^(CALL_MODE call_spec))?
    ) 
    ;

function_spec
    :    ^(FUNCTION_SPEC function_name (type_spec|SELF_VK) ^(PARAMETERS parameter*)
            (    ^(CALL_MODE call_spec)
            |    ^(EXTERNAL_VK expression)
            )?
        )
    ;

package_obj_body
    :     variable_declaration 
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
    :    ^(DROP_PROCEDURE procedure_name)
    ;

alter_procedure
    :    ^(ALTER_PROCEDURE procedure_name DEBUG_VK? REUSE_VK? compiler_parameters_clause*)
    ;

create_procedure_body
    :    ^(CREATE_PROCEDURE REPLACE_VK? procedure_name ^(PARAMETERS parameter*) invoker_rights_clause?
            (    EXTERNAL_VK
            |    ^(CALL_MODE call_spec)
            |    ^(BODY_MODE declare_spec* body)
            )
        )
    ;

// $>

// $<Trigger DDLs

drop_trigger
    :    ^(DROP_TRIGGER trigger_name)
    ;

alter_trigger
    :    ^(ALTER_TRIGGER trigger_name 
            (    (ENABLE_VK|DISABLE_VK)
            |    ^(RENAME_VK trigger_name)
            |    DEBUG_VK? REUSE_VK? compiler_parameters_clause*
            )
        )
    ;

create_trigger
    :    ^(CREATE_TRIGGER REPLACE_VK? trigger_name  
            simple_dml_trigger? compound_dml_trigger? non_dml_trigger?
            trigger_follows_clause? (ENABLE_VK|DISABLE_VK)? trigger_when_clause? trigger_body)
    ;

trigger_follows_clause
    :    ^(FOLLOWS_VK trigger_name+)
    ;

trigger_when_clause
    :    ^(SQL92_RESERVED_WHEN expression)
    ;

// $<Create Trigger- Specific Clauses
simple_dml_trigger
    :    ^(SIMPLE_DML (BEFORE_VK|AFTER_VK|INSTEAD_VK) FOR_EACH_ROW? referencing_clause? dml_event_clause)
    ;

compound_dml_trigger
    :    ^(COMPOUND_DML referencing_clause? dml_event_clause)
    ;

non_dml_trigger
    :    ^(NON_DML (BEFORE_VK|AFTER_VK) non_dml_event+ (DATABASE_VK|schema_name? SCHEMA_VK))
    ;

trigger_body
    :    ^(COMPOUND_VK trigger_name declare_spec* timing_point_section+)
    |    ^(CALL_VK routine_name function_argument?) 
    |    ^(BODY_MODE block)
    ;

timing_point_section
    :    ^(BEFORE_STATEMENT block)
    |    ^(BEFORE_EACH_ROW block)
    |    ^(AFTER_STATEMENT block)
    |    ^(AFTER_EACH_ROW block)
    ;

non_dml_event
    :    SQL92_RESERVED_ALTER
    |    ANALYZE_VK
    |    ASSOCIATE_VK STATISTICS_VK
    |    AUDIT_VK
    |    COMMENT_VK
    |    SQL92_RESERVED_CREATE
    |    DISASSOCIATE_VK STATISTICS_VK
    |    SQL92_RESERVED_DROP
    |    SQL92_RESERVED_GRANT
    |    NOAUDIT_VK
    |    RENAME_VK
    |    SQL92_RESERVED_REVOKE
    |    TRUNCATE_VK
    |    DDL_VK
    |    STARTUP_VK
    |    SHUTDOWN_VK
    |    DB_ROLE_CHANGE_VK
    |    LOGON_VK
    |    LOGOFF_VK
    |    SERVERERROR_VK
    |    SUSPEND_VK
    |    DATABASE_VK
    |    SCHEMA_VK
    |    FOLLOWS_VK
    ;

dml_event_clause
    :    ^(DML_EVENT dml_event_element+ ^(SQL92_RESERVED_ON tableview_name dml_event_nested_clause?))  
    ;

dml_event_element
    :    ^(DML_EVENT_ELEMENT (SQL92_RESERVED_DELETE|SQL92_RESERVED_INSERT|SQL92_RESERVED_UPDATE) ^(COLUMNS column_name*))
    ;

dml_event_nested_clause
    :    ^(NESTED_VK tableview_name)
    ;

referencing_clause
    :    ^(REFERENCING_VK referencing_element+)
    ;

referencing_element
    :    ^((NEW_VK|OLD_VK|PARENT_VK) alias)
    ;

// $>
// $>

// $<Type DDLs

drop_type
    :    ^(DROP_TYPE type_name BODY_VK? FORCE_VK? VALIDATE_VK?)
    ;

alter_type
    :    ^(ALTER_TYPE type_name 
            (    ^(REPLACE_VK ^(OBJECT_MEMBERS element_spec+) invoker_rights_clause?)
            |    ^(ALTER_ATTRIBUTE (ADD_VK|MODIFY_VK|SQL92_RESERVED_DROP) ^(ATTRIBUTES attribute_definition+))
            |    ^(ALTER_METHOD alter_method_element+)
            |    alter_collection_clauses
            |    modifier_clause
            |    ^(COMPILE_VK (SPECIFICATION_VK|BODY_VK)? DEBUG_VK? REUSE_VK? compiler_parameters_clause*)
            ) 
            dependent_handling_clause?
        )
    ;

// $<Alter Type - Specific Clauses

alter_method_element
    :    ^(ALTER_METHOD_ELEMENT (ADD_VK|SQL92_RESERVED_DROP) map_order_function_spec? subprogram_spec?)
    ;

attribute_definition
    :    ^(ATTRIBUTE attribute_name type_spec?)
    ;

alter_collection_clauses
    :    ^(ALTER_COLLECTION 
            (    ^(TYPE_VK type_spec)
            |    ^(LIMIT_VK expression)
            )
        ) 
    ;

dependent_handling_clause
    :    ^(DEPENDENT_HANDLING 
            (    INVALIDATE_VK
            |    ^(CASCADE_VK 
                    (    CONVERT_VK
                    |    SQL92_RESERVED_NOT? INCLUDING_VK
                    )?
                )
            )
        )
    ;

dependent_exceptions_part
    :    ^(EXCEPTIONS_VK FORCE_VK? tableview_name)
    ;

// $>

create_type
    :    ^(CREATE_TYPE_BODY REPLACE_VK? type_name ^(TYPE_BODY_ELEMENTS type_body_elements+))
    |    ^(CREATE_TYPE_SPEC REPLACE_VK? type_name CHAR_STRING? object_type_def?)
    ;

object_type_def
    :    ^(OBJECT_TYPE_DEF (object_as_part|object_under_part) invoker_rights_clause?
             sqlj_object_type? modifier_clause* ^(OBJECT_MEMBERS element_spec*))  
    ;

object_as_part
    :    ^(OBJECT_AS (OBJECT_VK|varray_type_def|nested_table_type_def))
    ;

object_under_part
    :    ^(UNDER_VK type_spec)
    ;

nested_table_type_def
    :    ^(NESTED_TABLE_TYPE_DEF type_spec SQL92_RESERVED_NULL?) 
    ;

sqlj_object_type
    :    ^(JAVA_VK expression (SQLDATA_VK|CUSTOMDATUM_VK|ORADATA_VK))
    ;

type_body_elements
    :    map_order_func_declaration
    |    subprog_decl_in_type
    ;

map_order_func_declaration
    :    ^((MAP_VK|SQL92_RESERVED_ORDER) create_function_body)
    ;

subprog_decl_in_type
    :    ^((MEMBER_VK|STATIC_VK)
            (    create_procedure_body
            |    create_function_body
            |    constructor_declaration
            )
        )
    ;

constructor_declaration
    :    ^(CONSTRUCTOR_VK type_spec FINAL_VK? INSTANTIABLE_VK? ^(PARAMETERS type_elements_parameter*) 
            (    ^(CALL_MODE call_spec)
            |    ^(BODY_MODE declare_spec* body)
            )
        )
    ;

// $>

// $<Common Type Clauses

modifier_clause
    :    ^(MODIFIER SQL92_RESERVED_NOT? (INSTANTIABLE_VK|FINAL_VK|OVERRIDING_VK))
    ;

sqlj_object_type_attr
    :    ^(EXTERNAL_VK expression)
    ;

element_spec
    :    ^(ELEMENT_SPEC element_spec_options+ modifier_clause? pragma_clause?)
    ;

element_spec_options
    :    subprogram_spec
    |    constructor_spec
    |    map_order_function_spec
    |    ^(FIELD_SPEC id type_spec sqlj_object_type_attr?)
    ;

subprogram_spec
    :    ^((MEMBER_VK|STATIC_VK)
            (    procedure_spec
            |    function_spec
            )
        )
    ;

constructor_spec
    :    ^(CONSTRUCTOR_SPEC type_spec FINAL_VK? INSTANTIABLE_VK? ^(PARAMETERS type_elements_parameter*) constructor_call_mode?)
    ;

constructor_call_mode
    :    ^(CALL_MODE call_spec)
    ;

map_order_function_spec
    :    ^((MAP_VK|SQL92_RESERVED_ORDER) function_spec)
    ;

pragma_clause
    :    ^(PRAGMA_VK pragma_elements+)
    ;

pragma_elements
    :    id
    |    SQL92_RESERVED_DEFAULT
    ;

type_elements_parameter
    :    ^(PARAMETER parameter_name type_spec)
    ;

// $>
// $>


// $<Sequence DDLs

drop_sequence
    :   ^(DROP_SEQUENCE sequence_name)
    ;

alter_sequence
    :    ^(ALTER_SEQUENCE sequence_name sequence_spec+)
    ;

create_sequence
    :    ^(CREATE_SEQUENCE sequence_name sequence_spec*)
    ;

// $<Common Sequence

sequence_spec
    :    ^(START_VK UNSIGNED_INTEGER)
    |    ^(INCREMENT_VK UNSIGNED_INTEGER)
    |    ^(MAXVALUE_VK UNSIGNED_INTEGER)
    |    ^(MINVALUE_VK UNSIGNED_INTEGER)
    |    ^(CACHE_VK UNSIGNED_INTEGER)
    |    NOMAXVALUE_VK
    |    NOMINVALUE_VK
    |    CYCLE_VK
    |    NOCYCLE_VK
    |    NOCACHE_VK
    |    ORDER_VK
    |    NOORDER_VK
    ;

// $>
// $>


// $<Common DDL Clauses

invoker_rights_clause
    :    ^(AUTHID_VK (CURRENT_USER_VK|DEFINER_VK))
    ;

compiler_parameters_clause
    :    ^(COMPILER_PARAMETER ^(ASSIGN id expression))
    ;

call_spec
    :    ^(LANGUAGE_VK ( java_spec | c_spec ))
    ;

// $<Call Spec - Specific Clauses

java_spec
    :    ^(JAVA_VK CHAR_STRING)
    ;

c_spec
    :    ^(C_VK CHAR_STRING? CONTEXT_VK? ^(LIBRARY_VK id) c_agent_in_clause? c_parameters_clause?)
    ;

c_agent_in_clause
    :    ^(AGENT_VK expression+)
    ;

c_parameters_clause
    :    ^(PARAMETERS_VK (THREE_DOTS|expression+))
    ;

// $>

parameter
    :    ^(PARAMETER parameter_name (SQL92_RESERVED_IN|OUT_VK|INOUT_VK)* type_spec? default_value_part?)
    ;

default_value_part
    :    ^(DEFAULT_VALUE expression)
    ;

// $>

// $>

// $<PL/SQL Elements Declarations

declare_spec
    :    variable_declaration
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
    :    ^(VARIABLE_DECLARE variable_name type_spec CONSTANT_VK? SQL92_RESERVED_NULL? default_value_part?)
    ;    

subtype_declaration
      :    ^(SUBTYPE_DECLARE type_name type_spec SQL92_RESERVED_NULL? subtype_range?)
      ;

subtype_range
    :    ^(RANGE_VK expression+)
    ;

//cursor_declaration incorportates curscursor_body and cursor_spec
cursor_declaration
    :    ^(CURSOR_DECLARE cursor_name type_spec? select_statement? ^(PARAMETERS parameter_spec*)) 
    ;

parameter_spec
    :    ^(PARAMETER parameter_name type_spec? default_value_part?)
    ;

exception_declaration 
    :    ^(EXCEPTION_DECLARE exception_name)
    ;             

pragma_declaration
    :    ^(PRAGMA_DECLARE 
            (    SERIALLY_REUSABLE_VK
            |     AUTONOMOUS_TRANSACTION_VK
            |    ^(EXCEPTION_INIT_VK exception_name constant)
            |    ^(INLINE_VK id expression)
            |    ^(RESTRICT_REFERENCES_VK SQL92_RESERVED_DEFAULT? id*)
            )
        )
    ;

record_declaration
    :    record_type_dec
    |    record_var_dec
    ;

// $<Record Declaration - Specific Clauses

//incorporates ref_cursor_type_definition
record_type_dec
    :    ^(RECORD_TYPE_DECLARE type_name REF_VK? type_spec? ^(FIELDS field_spec*))
    ;

field_spec
    :    ^(FIELD_SPEC column_name type_spec? SQL92_RESERVED_NULL? default_value_part?)
    ;

record_var_dec
    :    ^(RECORD_VAR_DECLARE record_name type_name (PERCENT_ROWTYPE_VK|PERCENT_TYPE_VK))
    ;

// $>

table_declaration
    :    table_type_dec
    |    table_var_dec
    ;

table_type_dec
    :    ^(TABLE_TYPE_DECLARE type_name 
            (    varray_type_def
            |    SQL92_RESERVED_NULL? ^(SQL92_RESERVED_TABLE type_spec table_indexed_by_part?)
            )
        )
    ;

table_indexed_by_part
    :    ^(INDEXED_BY type_spec)
    ;

varray_type_def
    :    SQL92_RESERVED_NULL? ^(VARR_ARRAY_DEF expression type_spec)
    ;

table_var_dec
    :    ^(TABLE_VAR_DECLARE table_var_name type_spec)
    ;

// $>

// $<PL/SQL Statements

seq_of_statements
    :     ^(STATEMENTS statement+)
    ;

statement
    :    label_declaration
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
    |    case_statement
    |    sql_statement
    |    function_call
    |    body
    |    block
    ;

label_declaration
    :    ^(LABEL_DECLARE label_name)
    ;

assignment_statement
    :     ^(ASSIGN general_element expression)
    ;

continue_statement
    :    ^(CONTINUE_VK label_name? general_when?)
    ;

general_when
    :    ^(SQL92_RESERVED_WHEN expression)
    ;

exit_statement
    :    ^(EXIT_VK label_name? general_when?)
    ;

goto_statement
    :    ^(SQL92_RESERVED_GOTO label_name)
    ;

if_statement
    :    ^(PLSQL_RESERVED_IF expression seq_of_statements elsif_part* else_part?)
    ;

elsif_part
    :    ^(PLSQL_NON_RESERVED_ELSIF expression seq_of_statements)
    ;

else_part
    :    ^(SQL92_RESERVED_ELSE seq_of_statements)
    ;

loop_statement
    :    ^(WHILE_LOOP label_name* expression seq_of_statements)
    |    ^(FOR_LOOP label_name* cursor_loop_param seq_of_statements)
    |    ^(LOOP_VK label_name* seq_of_statements)
    ;

// $<Loop - Specific Clause

cursor_loop_param
    :    ^(INDEXED_FOR index_name REVERSE_VK? ^(SIMPLE_BOUND expression expression))
    |    ^(CURSOR_BASED_FOR record_name cursor_name expression_list?)
    |    ^(SELECT_BASED_FOR record_name select_statement)
    ;

// $>

forall_statement
    :    ^(FORALL_VK index_name bounds_clause sql_statement EXCEPTIONS_VK?)
    ;

bounds_clause
    :    ^(SIMPLE_BOUND expression expression)
    |    ^(INDICES_BOUND collection_name between_bound?)
    |    ^(VALUES_BOUND index_name) 
    ;

between_bound
    :    ^(SQL92_RESERVED_BETWEEN expression expression)
    ;

null_statement
    :    SQL92_RESERVED_NULL
    ;

raise_statement
    :    ^(RAISE_VK exception_name?)
    ;

return_statement
    :    ^(RETURN_VK expression?)
    ;

function_call
    :    ^(ROUTINE_CALL routine_name function_argument?)
    ;

body
    :    ^(BODY label_name? seq_of_statements exception_clause?) 
    ;

// $<Body - Specific Clause

exception_clause
    :    ^(SQL92_RESERVED_EXCEPTION exception_handler+)
    ;

exception_handler
    :    ^(SQL92_RESERVED_WHEN exception_name+ seq_of_statements)
    ;

// $>

block
    :    ^(BLOCK declare_spec* body)
    ;

// $>

// $<SQL PL/SQL Statements

sql_statement
    :    execute_immediate
    |    data_manipulation_language_statements
    |    cursor_manipulation_statements
    |    transaction_control_statements
    ;

execute_immediate
    :    ^(EXECUTE_VK expression (into_clause|using_clause|dynamic_returning_clause)?) 
    ;

// $<Execute Immediate - Specific Clause
dynamic_returning_clause
    :    ^(DYNAMIC_RETURN into_clause)
    ;
// $>


// $<DML SQL PL/SQL Statements

data_manipulation_language_statements
    :    merge_statement
    |    lock_table_statement
    |    select_statement
    |     update_statement
    |     delete_statement
    |    insert_statement
    ;

select_statement
    :    ^(SELECT_STATEMENT subquery_factoring_clause? subquery for_update_clause* order_by_clause*)  
    ;

// $<Select - Specific Clauses
subquery_factoring_clause
    :    ^(SQL92_RESERVED_WITH factoring_element+)
    ;

factoring_element
    :    ^(FACTORING query_name subquery)
    ;

subquery
    :    ^(SUBQUERY subquery_basic_elements subquery_operation_part*)
    ;

subquery_operation_part
    :    ^((SQL92_RESERVED_UNION|SQL92_RESERVED_INTERSECT|PLSQL_RESERVED_MINUS) SQL92_RESERVED_ALL? subquery_basic_elements)
    ;

subquery_basic_elements
    :    query_block
    |    subquery
    ;

query_block
    :    ^(SQL92_RESERVED_SELECT 
            from_clause 
            (SQL92_RESERVED_DISTINCT|SQL92_RESERVED_UNIQUE)? SQL92_RESERVED_ALL? 
            (    ASTERISK
            |    ^(SELECT_LIST selected_element+)
            )
            into_clause? where_clause? hierarchical_query_clause? 
            group_by_clause? having_clause? model_clause?
        )
    ;

selected_element
    :    ^(SELECT_ITEM expression alias?)
    ;

from_clause
    :    ^(SQL92_RESERVED_FROM table_ref+)
    ;

table_ref
    :    ^(TABLE_REF table_ref_aux join_clause*)
    ;

table_ref_aux
    :    ^(TABLE_REF_ELEMENT alias? dml_table_expression_clause ONLY_VK? pivot_clause? unpivot_clause? flashback_query_clause*)
    ;

join_clause
    :    ^(JOIN_DEF (CROSS_VK|NATURAL_VK)? (INNER_VK|FULL_VK|LEFT_VK|RIGHT_VK)? table_ref_aux query_partition_clause* (join_on_part|join_using_part)?) 
    ;

join_on_part
    :    ^(SQL92_RESERVED_ON expression) 
    ;

join_using_part
    :    ^(PLSQL_NON_RESERVED_USING column_name+)
    ;

query_partition_clause
    :    ^(PARTITION_VK (expression_list|expression+))
    ;

flashback_query_clause
    :    ^((VERSIONS_VK|SQL92_RESERVED_AS) (SCN_VK|TIMESTAMP_VK)? expression) 
    ;

pivot_clause
    :    ^(PIVOT_VK XML_VK? pivot_element+ pivot_for_clause pivot_in_clause)
    ;

pivot_element
    :    ^(PIVOT_ELEMENT alias? expression)
    ;

pivot_for_clause
    :    ^(SQL92_RESERVED_FOR column_name+)
    ;

pivot_in_clause
    :    ^(SQL92_RESERVED_IN 
        (    subquery
        |    ^(ANY_MODE SQL92_RESERVED_ANY+)
        |    ^(ELEMENTS_MODE pivot_in_clause_element+)
        )
        )
    ;

pivot_in_clause_element
    :    ^(PIVOT_IN_ELEMENT alias? (expression|expression_list))
    ;

unpivot_clause
    :    ^(UNPIVOT_VK ((INCLUDE_VK|EXCLUDE_VK) NULLS_VK?)? column_name+ pivot_for_clause unpivot_in_clause)
    ;

unpivot_in_clause
    :    ^(SQL92_RESERVED_IN unpivot_in_element+)
    ;

unpivot_in_element
    :    ^(UNPIVOT_IN_ELEMENT column_name+ ^(PIVOT_ALIAS (expression|expression_list)))
    ;

hierarchical_query_clause
    :    ^(HIERARCHICAL start_part? ^(SQL92_RESERVED_CONNECT NOCYCLE_VK? expression))
    ;

start_part
    :    ^(PLSQL_RESERVED_START expression)
    ;

group_by_clause
    :    ^(SQL92_RESERVED_GROUP group_by_element+)
    ;

group_by_element
    :    ^(GROUP_BY_ELEMENT group_by_elements)
    ;

group_by_elements
    :    ^(GROUPING_VK groupin_set+)
    |    grouping_element 
    ;

groupin_set
    :    ^(GROUPIN_SET grouping_element)
    ;

grouping_element
    :    ^((ROLLUP_VK|CUBE_VK) grouping_element+)
    |    expression_list
    |    expression 
    ;

having_clause
    :    ^(SQL92_RESERVED_HAVING expression)
    ;

model_clause
    :    ^(PLSQL_NON_RESERVED_MODEL main_model cell_reference_options* return_rows_clause? reference_model*)
    ;

cell_reference_options
    :    ^((IGNORE_VK|KEEP_VK) NAV_VK)
    |    ^(SQL92_RESERVED_UNIQUE (DIMENSION_VK|SINGLE_VK))
    ;

return_rows_clause
    :    ^(RETURN_VK (UPDATED_VK|SQL92_RESERVED_ALL))
    ;

reference_model
    :    ^(REFERENCE_VK reference_model_name subquery model_column_clauses cell_reference_options*)
    ;

main_model
    :    ^(MAIN_MODEL main_model_name? model_column_clauses model_rules_clause cell_reference_options*)
    ;

model_column_clauses
    :    ^(MODEL_COLUMN ^(DIMENSION_VK model_column_list) ^(MEASURES_VK model_column_list) model_column_partition_part?)
    ;

model_column_partition_part
    :    ^(PARTITION_VK model_column_list)
    ;

model_column_list
    :    ^(MODEL_COLUMNS model_column+)
    ;

model_column
    :    ^(MODEL_COLUMN alias? expression) 
    ;

model_rules_clause
    :    ^(MODEL_RULES model_rules_element+ model_rules_part?)
    ;

model_rules_part
    :    ^(RULES_VK 
            (SQL92_RESERVED_UPDATE|UPSERT_VK SQL92_RESERVED_ALL?)? 
            (AUTOMATIC_VK|SEQUENTIAL_VK)? 
            model_iterate_clause?
        )
    ;

model_rules_element
    :    ^(MODEL_RULE 
            ^(ASSIGN model_expression expression) 
            (SQL92_RESERVED_UPDATE|UPSERT_VK SQL92_RESERVED_ALL?)? 
            order_by_clause?
        )
    ;

model_iterate_clause
    :    ^(ITERATE_VK expression until_part?)
    ;

until_part
    :    ^(UNTIL_VK expression)
    ;

order_by_clause
    :    ^(SQL92_RESERVED_ORDER SIBLINGS_VK? ^(ORDER_BY_ELEMENTS order_by_elements+))
    ;

order_by_elements
    :    ^(ORDER_BY_ELEMENT expression (SQL92_RESERVED_ASC|SQL92_RESERVED_DESC)? (NULLS_VK (FIRST_VK|LAST_VK))?)
    ;

for_update_clause
    :    ^(SQL92_RESERVED_FOR for_update_of_part? for_update_options?)
    ;

for_update_of_part
    :    ^(SQL92_RESERVED_OF column_name+)
    ;

for_update_options
    :    SKIP_VK
    |    PLSQL_RESERVED_NOWAIT
    |    ^(WAIT_VK expression)
    ;

// $>

update_statement
    :    ^(SQL92_RESERVED_UPDATE general_table_ref
            update_set_clause
            where_clause? static_returning_clause? error_logging_clause?
        )
    ;

// $<Update - Specific Clauses
update_set_clause
    :    ^(SET_VK update_set_elements+)
    ;

update_set_elements
    :    ^(ASSIGN column_name expression)
    |    ^(ASSIGN column_name+ subquery)
    |    ^(VALUE_VK char_set_name? ID expression)
    ;

// $>

delete_statement
    :    ^(SQL92_RESERVED_DELETE general_table_ref
            where_clause? static_returning_clause? error_logging_clause?)
    ;

insert_statement
    :    ^(SQL92_RESERVED_INSERT
        (    single_table_insert
        |    multi_table_insert
        )
        )
    ;

// $<Insert - Specific Clauses

single_table_insert
    :    ^(SINGLE_TABLE_MODE insert_into_clause (values_clause static_returning_clause?| select_statement) error_logging_clause?)
    ;

multi_table_insert
    :    ^(MULTI_TABLE_MODE select_statement (conditional_insert_clause|multi_table_element+))
    ;

multi_table_element
    :    ^(TABLE_ELEMENT insert_into_clause values_clause? error_logging_clause?)
    ;

conditional_insert_clause
    :    ^(CONDITIONAL_INSERT (SQL92_RESERVED_ALL|FIRST_VK)? conditional_insert_when_part+ conditional_insert_else_part?) 
    ;

conditional_insert_when_part
    :    ^(SQL92_RESERVED_WHEN expression multi_table_element+)
    ;

conditional_insert_else_part
    :    ^(SQL92_RESERVED_ELSE multi_table_element+)
    ;

insert_into_clause
    :    ^(SQL92_RESERVED_INTO general_table_ref ^(COLUMNS column_name*))
    ;

values_clause
    :    ^(SQL92_RESERVED_VALUES expression_list)
    ;

// $>
merge_statement
    :    ^(MERGE_VK alias? tableview_name 
            ^(PLSQL_NON_RESERVED_USING selected_tableview expression)
             merge_update_clause? merge_insert_clause? error_logging_clause?)
    ;

// $<Merge - Specific Clauses

merge_update_clause
    :    ^(MERGE_UPDATE merge_element+ where_clause? merge_update_delete_part?)
    ;

merge_element
    :    ^(ASSIGN column_name expression)
    ;

merge_update_delete_part
    :    ^(SQL92_RESERVED_DELETE where_clause)
    ;

merge_insert_clause
    :    ^(MERGE_INSERT ^(COLUMNS column_name*) expression_list where_clause?) 
    ;

selected_tableview
    :    ^(SELECTED_TABLEVIEW alias? (tableview_name|subquery))
    ;

// $>

lock_table_statement
    :    ^(PLSQL_RESERVED_LOCK lock_table_element+ lock_mode wait_nowait_part?)
    ;

wait_nowait_part
    :    ^(WAIT_VK expression)
    |    PLSQL_RESERVED_NOWAIT
    ;

// $<Lock - Specific Clauses

lock_table_element
    :    ^(LOCK_TABLE_ELEMENT tableview_name partition_extension_clause?)
    ;

lock_mode
    :    ROW_VK PLSQL_RESERVED_SHARE
    |    ROW_VK PLSQL_RESERVED_EXCLUSIVE
    |    PLSQL_RESERVED_SHARE SQL92_RESERVED_UPDATE?
    |    PLSQL_RESERVED_SHARE ROW_VK PLSQL_RESERVED_EXCLUSIVE
    |    PLSQL_RESERVED_EXCLUSIVE
    ;
// $>

// $<Common DDL Clauses

general_table_ref
    :    ^(TABLE_REF alias? dml_table_expression_clause ONLY_VK?)
    ;

static_returning_clause
    :    ^(STATIC_RETURNING expression+ into_clause)
    ;

error_logging_clause
    :    ^(LOG_VK error_logging_into_part? expression? error_logging_reject_part?)
    ;

error_logging_into_part
    :    ^(SQL92_RESERVED_INTO tableview_name)
    ;

error_logging_reject_part
    :    ^(REJECT_VK (UNLIMITED_VK|expression))
    ;

dml_table_expression_clause
    :    ^(TABLE_EXPRESSION 
        (    ^(COLLECTION_MODE expression PLUS_SIGN?)
        |    ^(SELECT_MODE select_statement subquery_restriction_clause?)
        |    ^(DIRECT_MODE tableview_name sample_clause?)
        )
        )
    ;

subquery_restriction_clause
    :    ^(SQL92_RESERVED_WITH (READ_VK|SQL92_RESERVED_CHECK constraint_name?))
    ;

sample_clause
    :    ^(SAMPLE_VK BLOCK_VK? expression seed_part?) 
    ;

seed_part
    :    ^(SEED_VK expression)
    ;

// $>

// $>

// $<Cursor Manipulation SQL PL/SQL Statements

cursor_manipulation_statements
    :    close_statement
    |    open_statement
    |    fetch_statement
    |    open_for_statement
    ;

close_statement
    :     ^(CLOSE_VK variable_name) 
    ;

open_statement
    :    ^(OPEN_VK cursor_name expression_list?)
    ;

fetch_statement
    :    ^(SQL92_RESERVED_FETCH cursor_name 
            (    ^(SQL92_RESERVED_INTO variable_name+)
            |    ^(BULK_VK variable_name+)
            )
        )
    ;

open_for_statement
    :    ^(OPEN_VK variable_name (expression|select_statement) using_clause?)
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
    :    ^(SET_TRANSACTION CHAR_STRING?
            (    ^(READ_VK (ONLY_VK|WRITE_VK))
            |    ^(ISOLATION_VK (SERIALIZABLE_VK|COMMITTED_VK))
            |    ^(ROLLBACK_VK rollback_segment_name)
            )?
        )
    ;

set_constraint_command
    :    ^(SET_CONSTRAINT (SQL92_RESERVED_ALL|constraint_name+) (IMMEDIATE_VK|DEFERRED_VK))
    ;

commit_statement
    :     ^(COMMIT_VK WORK_VK? 
            (    ^(COMMENT_VK expression)
            |    ^(FORCE_VK (CORRUPT_XID_VK expression|CORRUPT_XID_ALL_VK|expression expression?))
            )?
            write_clause?
        )
    ;

write_clause
    :    ^(WRITE_VK (WAIT_VK|PLSQL_RESERVED_NOWAIT)? (IMMEDIATE_VK|BATCH_VK)?)
    ;

rollback_statement
    :     ^(ROLLBACK_VK WORK_VK? 
            (    ^(SQL92_RESERVED_TO savepoint_name)
            |    ^(FORCE_VK CHAR_STRING)
            )?
        )
    ;

savepoint_statement
    :    ^(SAVEPOINT_VK savepoint_name) 
    ;

// $>

// $<Expression & Condition

expression_list
    :    ^(EXPR_LIST expression*)
    ;

expression
    :    ^(LOGIC_EXPR expression_element)
    |    ^(EXPR expression_element)
    ;

expression_element
    :    ^(SQL92_RESERVED_OR expression_element expression_element)
    |    ^(SQL92_RESERVED_AND expression_element expression_element)
    |    ^(SQL92_RESERVED_NOT expression_element)
    |    ^((EQUALS_OP|NOT_EQUAL_OP|LESS_THAN_OP|GREATER_THAN_OP|LESS_THAN_OR_EQUALS_OP|GREATER_THAN_OR_EQUALS_OP) expression_element expression_element)

    |    ^(IS_NOT_NULL expression_element)
    |    ^(IS_NULL expression_element)
    |    ^(IS_NOT_NAN expression_element)
    |    ^(IS_NAN expression_element)
    |    ^(IS_NOT_PRESENT expression_element)
    |    ^(IS_PRESENT expression_element)
    |    ^(IS_NOT_INFINITE expression_element)
    |    ^(IS_INFINITE expression_element)
    |    ^(IS_NOT_A_SET expression_element)
    |    ^(IS_A_SET expression_element)
    |    ^(IS_NOT_EMPTY expression_element)
    |    ^(IS_EMPTY expression_element)
    |    ^(IS_NOT_OF_TYPE expression_element type_spec+)
    |    ^(IS_OF_TYPE expression_element type_spec+)

    |    ^((MEMBER_VK|SUBMULTISET_VK) expression_element expression_element)

    |    ^(NOT_IN expression_element in_elements)
    |    ^(SQL92_RESERVED_IN expression_element in_elements)
    |    ^(NOT_BETWEEN expression_element expression_element expression_element)
    |    ^(SQL92_RESERVED_BETWEEN expression_element expression_element expression_element)
    |    ^(NOT_LIKE expression_element expression_element expression_element?)
    |    ^((SQL92_RESERVED_LIKE|LIKEC_VK|LIKE2_VK|LIKE4_VK) expression_element expression_element expression_element?)

    |    ^(CONCATENATION_OP expression_element expression_element)
    |    ^(PLUS_SIGN expression_element expression_element)
    |    ^(MINUS_SIGN expression_element expression_element)
    |    ^(ASTERISK expression_element expression_element)
    |    ^(SOLIDUS expression_element expression_element)

    |    ^(UNARY_OPERATOR expression_element)
    |    ^(SQL92_RESERVED_PRIOR expression_element)
    |    ^(NEW_VK expression)
    |    ^(SQL92_RESERVED_DISTINCT expression_element)
    |    ^(STANDARD_FUNCTION standard_function)
    |    ^((SOME_VK|SQL92_RESERVED_EXISTS|SQL92_RESERVED_ALL|SQL92_RESERVED_ANY) expression_element)
    |    ^(VECTOR_EXPR expression_element+)

    |    ^(DATETIME_OP expression_element datetime_element)
    |    model_expression
    |    ^(KEEP_VK expression_element DENSE_RANK_VK (FIRST_VK|LAST_VK) order_by_clause over_clause?)

    |    ^(DOT_ASTERISK tableview_name)

    |    case_statement
    |    constant
    |    general_element
    |    subquery
    ;

in_elements
    :    subquery
    |    expression_list
    ;

datetime_element
    :    ^(AT_VK expression_element (LOCAL_VK|TIME_VK expression))
    |    ^(DAY_VK SECOND_VK expression*)
    |    ^(YEAR_VK MONTH_VK expression)
    ;

model_expression
    :    ^(MODEL_EXPRESSION expression_element model_expression_element+)
    ;

model_expression_element
    :    SQL92_RESERVED_ANY
    |    expression
    |    ^(FOR_SINGLE_COLUMN column_name for_single_column_element for_like_part?)
    |    ^(FOR_MULTI_COLUMN column_name+ ^(SQL92_RESERVED_IN (subquery|expression_list+)))
    ;

for_single_column_element
    :    ^(SQL92_RESERVED_IN expression_list)
    |    ^(SQL92_RESERVED_FROM expression) 
    |    ^(SQL92_RESERVED_TO expression) 
    |    ^((INCREMENT_VK|DECREMENT_VK) expression) 
    ;

for_like_part
    :    ^(SQL92_RESERVED_LIKE expression)
    ;

case_statement
    :    ^(SIMPLE_CASE label_name* expression case_when_part+ case_else_part?)  
    |    ^(SEARCHED_CASE label_name* case_when_part+ case_else_part?) 
    ;

// $<CASE - Specific Clauses

case_when_part
    :    ^(SQL92_RESERVED_WHEN expression (seq_of_statements|expression))
    ;

case_else_part
    :    ^(SQL92_RESERVED_ELSE (seq_of_statements|expression))
    ;
// $>

standard_function
    :    ^(FUNCTION_ENABLING_OVER function_argument over_clause?)
    |    ^(FUNCTION_ENABLING_USING function_argument using_clause?)
    |    ^(COUNT_VK ( ASTERISK | expression ) over_clause?)
    |    ^((CAST_VK|XMLCAST_VK) (subquery|expression) type_spec)
    |    ^(CHR_VK expression NCHAR_CS_VK)
    |    ^(COLLECT_VK (SQL92_RESERVED_DISTINCT|SQL92_RESERVED_UNIQUE)? column_name collect_order_by_part?)
    |    ^(FUNCTION_ENABLING_WITHIN_OR_OVER function_argument (within_clause|over_clause)+ )
    |    ^(DECOMPOSE_VK expression (CANONICAL_VK|COMPATIBILITY_VK)?) 
    |    ^(EXTRACT_VK REGULAR_ID expression)
    |    ^((FIRST_VALUE_VK|LAST_VALUE_VK) expression NULLS_VK? over_clause) 
    |    ^(PREDICTION_FUNCTION expression+ cost_matrix_clause? using_clause?)
    |    ^(TRANSLATE_VK expression (CHAR_CS_VK|NCHAR_CS_VK)? expression*)
    |    ^(TREAT_VK expression REF_VK? type_spec)
    |    ^(TRIM_VK (LEADING_VK|TRAILING_VK|BOTH_VK)? expression expression?) 

    |    ^(XMLAGG_VK expression order_by_clause?)
    |    ^((XMLCOLATTVAL_VK|XMLFOREST_VK) xml_multiuse_expression_element+)
    |    ^(XMLEXISTS_VK expression xml_passing_clause?)
    |    ^(XMLPARSE_VK (DOCUMENT_VK|CONTENT_VK) expression WELLFORMED_VK?)
    |    ^(XMLQUERY_VK expression xml_passing_clause? SQL92_RESERVED_NULL?)
    |    ^(XMLROOT_VK expression xml_param_version_part xmlroot_param_standalone_part?)
    |    ^(XMLTABLE_VK xml_namespaces_clause? expression xml_passing_clause? xml_table_column*)
    |    ^(XMLELEMENT_VK
            (ENTITYESCAPING_VK|NOENTITYESCAPING_VK)?
            (NAME_VK|EVALNAME_VK)? expression
            xml_attributes_clause? (expression alias?)*
        )
    |    ^(XMLPI_VK
                (    NAME_VK char_set_name? ID
                |    EVALNAME_VK expression
                )
                expression?
        )
    |    ^(XMLSERIALIZE_VK
                (DOCUMENT_VK|CONTENT_VK)
                expression type_spec?
                xmlserialize_param_enconding_part?
                xml_param_version_part?
                xmlserialize_param_ident_part?
                ((HIDE_VK|SHOW_VK) DEFAULTS_VK)?
        )
    ;

over_clause
    :    ^(OVER_VK query_partition_clause? (order_by_clause windowing_clause?)?)
    ;

windowing_clause
    :    ^((ROWS_VK|RANGE_VK)
            (    ^(SQL92_RESERVED_BETWEEN windowing_elements windowing_elements)
            |    windowing_elements+
            )
        )
    ;

windowing_elements
    :    ^(UNBOUNDED_VK PRECEDING_VK)
    |    ^(CURRENT_VK ROW_VK)
    |    ^((PRECEDING_VK|FOLLOWING_VK) expression)
    ;

using_clause
    :    ^(PLSQL_NON_RESERVED_USING using_element+)
    ;

using_element
    :    ^(ELEMENT SQL92_RESERVED_IN? OUT_VK? expression alias?)
    |    ASTERISK
    ;

collect_order_by_part
    :    ^(SQL92_RESERVED_ORDER expression)
    ;

within_clause
    :    ^(WITHIN_VK order_by_clause)
    ;

cost_matrix_clause
    :    ^(COST_VK
            (    PLSQL_NON_RESERVED_MODEL AUTO_VK?
            |    cost_class_name+ expression_list
            )
        )
    ;

xml_passing_clause
    :    ^(PASSING_VK VALUE_VK? expression alias? (expression alias?)?)
    ;

xml_attributes_clause
    :    ^(XMLATTRIBUTES_VK
            (ENTITYESCAPING_VK|NOENTITYESCAPING_VK)?
            (SCHEMACHECK_VK|NOSCHEMACHECK_VK)?
            xml_multiuse_expression_element+
        )
    ;

xml_namespaces_clause
    :    ^(XMLNAMESPACES_VK
            (expression alias?)* xml_general_default_part?
        )
    ;

xml_table_column
    :    ^(XML_COLUMN xml_column_name (ORDINALITY_VK|type_spec expression? xml_general_default_part?) )
    ;

xml_general_default_part
    :    ^(SQL92_RESERVED_DEFAULT expression)
    ;

xml_multiuse_expression_element
    :    ^(XML_ELEMENT expression xml_alias?)
    ;

xml_alias
    :    ^(XML_ALIAS ID)
    |    ^(XML_ALIAS ^(EVALNAME_VK expression))
    ;

xml_param_version_part
    :    ^(VERSION_VK (NO_VK VALUE_VK|expression))
    ;

xmlroot_param_standalone_part
    :    ^(STANDALONE_VK (YES_VK|NO_VK VALUE_VK?))
    ;

xmlserialize_param_enconding_part
    :    ^(ENCODING_VK expression)
    ;

xmlserialize_param_ident_part
    :    NO_VK INDENT_VK
    |    ^(INDENT_VK expression?)
    ;

// $>

// $<Common SQL PL/SQL Clauses/Parts

partition_extension_clause
    :    ^((SUBPARTITION_VK|PARTITION_VK) expression_list)
    ;

alias
    :    ^(ALIAS char_set_name? ID)
    ;

where_clause
    :    ^(SQL92_RESERVED_WHERE expression)
    ;

into_clause
    :    ^(SQL92_RESERVED_INTO variable_name+) 
    |    ^(BULK_VK variable_name+) 
    ;

// $>

// $<Common PL/SQL Named Elements

xml_column_name
    :    ^(XML_COLUMN_NAME char_set_name? ID)
    ;

cost_class_name
    :    ^(COST_CLASS_NAME char_set_name? ID)
    ;

attribute_name
    :    ^(ATTRIBUTE_NAME char_set_name? ID)
    ;

savepoint_name
    :    ^(SAVEPOINT_NAME char_set_name? ID)
    ;

rollback_segment_name
    :    ^(ROLLBACK_SEGMENT_NAME char_set_name? ID)
    ;


table_var_name
    :    ^(TABLE_VAR_NAME char_set_name? ID)
    ;

schema_name
    :    ^(SCHEMA_NAME char_set_name? ID)
    ;

routine_name
    :    ^(ROUTINE_NAME char_set_name? ID+ link_name?)
    ;

package_name
    :    ^(PACKAGE_NAME char_set_name? ID)
    ;

implementation_type_name
    :    ^(IMPLEMENTATION_TYPE_NAME char_set_name? ID+)
    ;

parameter_name
    :    ^(PARAMETER_NAME char_set_name? ID)
    ;

reference_model_name
    :    ^(REFERENCE_MODEL_NAME char_set_name? ID)
    ;

main_model_name
    :    ^(MAIN_MODEL_NAME char_set_name? ID)
    ;

query_name
    :    ^(QUERY_NAME char_set_name? ID)
    ;

constraint_name
    :    ^(CONSTRAINT_NAME char_set_name? ID+ link_name?)
    ;

label_name
    :    ^(LABEL_NAME ID)
    ;

type_name
    :    ^(TYPE_NAME ID+)
    ;

sequence_name
    :    ^(SEQUENCE_NAME ID+)
    ;

exception_name
    :    ^(EXCEPTION_NAME char_set_name? ID+)
    ;

function_name
    :    ^(FUNCTION_NAME char_set_name? ID+)
    ;

procedure_name
    :    ^(PROCEDURE_NAME char_set_name? ID+)
    ;

trigger_name
    :    ^(TRIGGER_NAME char_set_name? ID+)
    ;

variable_name
    :    ^(HOSTED_VARIABLE_NAME char_set_name? ID+)
    |    ^(VARIABLE_NAME char_set_name? ID+)
    ;

index_name
    :    ^(INDEX_NAME char_set_name? ID)
    ;

cursor_name
    :    ^(CURSOR_NAME char_set_name? ID)
    ;

record_name
    :    ^(RECORD_NAME char_set_name? ID)
    ;

collection_name
    :    ^(COLLECTION_NAME char_set_name? ID+)
    ;

link_name
    :    ^(LINK_NAME char_set_name? ID)
    ;

column_name
    :    ^(COLUMN_NAME char_set_name? ID+)
    ;

tableview_name
    :    ^(TABLEVIEW_NAME char_set_name? ID+ link_name? partition_extension_clause?)
    ;

char_set_name
    :    ^(CHAR_SET_NAME ID+)
    ;

// $>

// $<Common PL/SQL Specs

function_argument
    :    ^(ARGUMENTS argument*)
    ;

argument
    :    ^(ARGUMENT expression parameter_name?)
    ;

type_spec
    :     ^(CUSTOM_TYPE type_name REF_VK? (PERCENT_ROWTYPE_VK|PERCENT_TYPE_VK)?)
    |    ^(NATIVE_DATATYPE native_datatype_element type_precision? (TIME_VK LOCAL_VK?)?)
    |    ^(INTERVAL_DATATYPE (YEAR_VK|DAY_VK) (MONTH_VK|SECOND_VK) expression*)
    ;

type_precision
    :    ^(PRECISION constant constant? (CHAR_VK|BYTE_VK)? (TIME_VK LOCAL_VK?)?)
    ;

native_datatype_element
    :    BINARY_INTEGER_VK
    |    PLS_INTEGER_VK
    |    NATURAL_VK
    |    BINARY_FLOAT_VK
    |    BINARY_DOUBLE_VK
    |    NATURALN_VK
    |    POSITIVE_VK
    |    POSITIVEN_VK
    |    SIGNTYPE_VK
    |    SIMPLE_INTEGER_VK
    |    NVARCHAR2_VK
    |    DEC_VK
    |    INTEGER_VK
    |    INT_VK
    |    NUMERIC_VK
    |    SMALLINT_VK
    |    NUMBER_VK
    |    DECIMAL_VK 
    |    DOUBLE_VK PRECISION_VK?
    |    FLOAT_VK
    |    REAL_VK
    |    NCHAR_VK
    |    LONG_VK RAW_VK?
    |    CHAR_VK  
    |    CHARACTER_VK 
    |    VARCHAR2_VK
    |    VARCHAR_VK
    |    STRING_VK
    |    RAW_VK
    |    BOOLEAN_VK
    |    DATE_VK
    |    ROWID_VK
    |    UROWID_VK
    |    YEAR_VK
    |    MONTH_VK
    |    DAY_VK
    |    HOUR_VK
    |    MINUTE_VK
    |    SECOND_VK
    |    TIMEZONE_HOUR_VK
    |    TIMEZONE_MINUTE_VK
    |    TIMEZONE_REGION_VK
    |    TIMEZONE_ABBR_VK
    |    TIMESTAMP_VK
    |    TIMESTAMP_UNCONSTRAINED_VK
    |    TIMESTAMP_TZ_UNCONSTRAINED_VK
    |    TIMESTAMP_LTZ_UNCONSTRAINED_VK
    |    YMINTERVAL_UNCONSTRAINED_VK
    |    DSINTERVAL_UNCONSTRAINED_VK
    |    BFILE_VK
    |    BLOB_VK
    |    CLOB_VK
    |    NCLOB_VK
    |    MLSLABEL_VK
    ;

general_element
    :    ^(CASCATED_ELEMENT general_element+)
    |    ^(HOSTED_VARIABLE_ROUTINE_CALL routine_name function_argument)
    |    ^(HOSTED_VARIABLE char_set_name? ID+)
    |    ^(ROUTINE_CALL routine_name function_argument)
    |    ^(ANY_ELEMENT char_set_name? ID+)
    ;

// $>

// $<Lexer Mappings

constant
    :    UNSIGNED_INTEGER
    |    EXACT_NUM_LIT
    |    APPROXIMATE_NUM_LIT
    |    CHAR_STRING
    |    SQL92_RESERVED_NULL
    |    SQL92_RESERVED_TRUE
    |    SQL92_RESERVED_FALSE
    |    DBTIMEZONE_VK 
    |    SESSIONTIMEZONE_VK
    |    MINVALUE_VK
    |    MAXVALUE_VK
    |    SQL92_RESERVED_DEFAULT
    ;

// $>

id
    :    char_set_name? ID
    ;

