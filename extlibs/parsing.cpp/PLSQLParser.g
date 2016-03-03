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
    language=Cpp;
    //memoize=true;    
    output=AST;
    tokenVocab=PLSQLLexer;
}

import PLSQLKeys, PLSQLCommons, PLSQL_DMLParser, SQLPLUSParser;

tokens {
    COMPILATION_UNIT;
    DROP_FUNCTION;
    ALTER_FUNCTION;
    CREATE_FUNCTION;
    USING_MODE;
    CALL_MODE;
    BODY_MODE;
    EXTERNAL_MODE;
    STREAMING_CLAUSE;
    EXPR;
    CREATE_PACKAGE_SPEC;
    CREATE_PACKAGE_BODY;
    COLUMNS;
    DROP_PACKAGE;
    ALTER_PACKAGE;
    PROCEDURE_SPEC;
    FUNCTION_SPEC;
    DROP_PROCEDURE;
    ALTER_PROCEDURE;
    CREATE_PROCEDURE;
    DROP_TRIGGER;
    ALTER_TRIGGER;
    CREATE_TRIGGER;
    LOGIC_EXPR;
    SIMPLE_DML;
    FOR_EACH_ROW;
    COMPOUND_DML;
    NON_DML;
    BEFORE_STATEMENT;
    BEFORE_EACH_ROW;
    AFTER_STATEMENT;
    AFTER_EACH_ROW;
    DML_EVENT;
    DML_EVENT_ELEMENT;
    DROP_TYPE;
    ALTER_TYPE;
    ALTER_METHOD;
    ALTER_METHOD_ELEMENT;
    ALTER_ATTRIBUTE;
    ATTRIBUTES;
    DEPENDENT_HANDLING;
    ALTER_COLLECTION;
    CREATE_TYPE_BODY;
    CREATE_TYPE_SPEC;
    ASSIGN;
    THREE_DOTS;
    COMPILER_PARAMETER;
    MODIFIER;
    DEFAULT_VALUE;
    OBJECT_MEMBERS;
    OBJECT_TYPE_DEF;
    OBJECT_AS;
    NESTED_TABLE_TYPE_DEF;
    TYPE_BODY_ELEMENTS;
    ELEMENT_SPEC;
    FIELD_SPEC;
    CONSTRUCTOR_SPEC;
    VARIABLE_DECLARE;
    SUBTYPE_DECLARE;
    CURSOR_DECLARE;
    PARAMETERS;
    PARAMETER;
    EXCEPTION_DECLARE;
    PRAGMA_DECLARE;
    RECORD_TYPE_DECLARE;
    FIELDS;
    RECORD_VAR_DECLARE;
    TABLE_TYPE_DECLARE;
    TABLE_TYPE_DECLARE;
    INDEXED_BY;
    VARR_ARRAY_DEF;
    TABLE_VAR_DECLARE;
    LABEL_DECLARE;
    FOR_LOOP;
    WHILE_LOOP;
    INDEXED_FOR;
    CURSOR_BASED_FOR;
    SELECT_BASED_FOR;
    SIMPLE_BOUND;
    INDICES_BOUND;
    VALUES_BOUND;
    ROUTINE_CALL;
    BODY;
    BLOCK;
    STATEMENTS;
    DYNAMIC_RETURN;
    SET_TRANSACTION;
    SET_CONSTRAINT;
    SELECTED_TABLEVIEW;
    SQL_SCRIPT;
    SET_SERVEROUTPUT;
    ATTRIBUTE;
    DROP_SEQUENCE;
    ALTER_SEQUENCE;
    CREATE_SEQUENCE;
}


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
        -> ^(COMPILATION_UNIT unit_statement*)
    ;

sql_script
    :   (unit_statement|sql_plus_command)* EOF
        -> ^(SQL_SCRIPT sql_plus_command* unit_statement*)
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
        -> ^(DROP_FUNCTION[$drop_key.start] function_name)
    ;

alter_function
    :    alter_key function_key function_name
        compile_key debug_key? compiler_parameters_clause*
        (reuse_key settings_key)?
        SEMICOLON
        -> ^(ALTER_FUNCTION[$alter_key.start] function_name debug_key? reuse_key? compiler_parameters_clause*)
    ;

create_function_body
@init    {    int mode = 0;    }
    :    (create_key ( or_key replace_key )?)? function_key function_name 
              ( LEFT_PAREN parameter (COMMA parameter)* RIGHT_PAREN)?
        return_key type_spec
                (invoker_rights_clause|parallel_enable_clause|result_cache_clause|deterministic_key)*
        (
            (    pipelined_key? ( is_key | as_key )
                      (    declare_key? declare_spec* body 
                      |    call_spec {mode = 2;})
            )
        |    (pipelined_key|aggregate_key) using_key implementation_type_name {mode = 1;}
        )
              SEMICOLON
        ->    {mode == 1}?
            ^(CREATE_FUNCTION[$function_key.start] replace_key? function_name type_spec ^(PARAMETERS parameter*)
                invoker_rights_clause* parallel_enable_clause* result_cache_clause* deterministic_key*
                ^(USING_MODE pipelined_key? aggregate_key? implementation_type_name))
        -> {mode == 2}?
            ^(CREATE_FUNCTION[$function_key.start] replace_key? function_name type_spec ^(PARAMETERS parameter*)
                invoker_rights_clause* parallel_enable_clause* result_cache_clause* deterministic_key*
                ^(CALL_MODE pipelined_key? call_spec))
        ->    ^(CREATE_FUNCTION[$function_key.start] replace_key? function_name type_spec ^(PARAMETERS parameter*)
                invoker_rights_clause* parallel_enable_clause* result_cache_clause* deterministic_key*
                ^(BODY_MODE pipelined_key? declare_spec* body))
    ;

// $<Creation Function - Specific Clauses

parallel_enable_clause
    :    parallel_enable_key^ partition_by_clause?
    ;

partition_by_clause
@init    {    int mode = 0;    }
    :    LEFT_PAREN
            partition_key expression by_key 
            (    any_key {mode = 1;}
            |    (hash_key {mode = 2;}|range_key) LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
            )
            streaming_clause?
        RIGHT_PAREN
        ->{mode == 1}? 
            ^(partition_key ^(EXPR expression) any_key streaming_clause?)
        ->{mode == 2}?
            ^(partition_key ^(EXPR expression) ^(hash_key ^(COLUMNS column_name+)) streaming_clause?)
        ->
            ^(partition_key ^(EXPR expression) ^(range_key ^(COLUMNS column_name+)) streaming_clause?)
    ;

result_cache_clause
    :    result_cache_key relies_on_part?
        -> ^(result_cache_key relies_on_part?)
    ;

relies_on_part
    :    relies_on_key^ LEFT_PAREN! tableview_name (COMMA! tableview_name)* RIGHT_PAREN!
    ;

streaming_clause
    :    (order_key|cluster_key) expression by_key 
        LEFT_PAREN column_name (COMMA column_name)* RIGHT_PAREN
        -> ^(STREAMING_CLAUSE order_key? cluster_key? ^(EXPR expression) ^(COLUMNS column_name+)) 
    ;
// $>
// $>

// $<Package DDLs

drop_package
    :    drop_key package_key body_key? package_name
        SEMICOLON
        -> ^(DROP_PACKAGE[$drop_key.start] package_name body_key?)
    ;

alter_package
    :    alter_key package_key package_name
        compile_key debug_key? (package_key|body_key|specification_key)?
        compiler_parameters_clause*
        (reuse_key settings_key)?
        SEMICOLON
        -> ^(ALTER_PACKAGE[$alter_key.start] package_name debug_key? reuse_key? 
                package_key? body_key? specification_key? compiler_parameters_clause*)
    ;

create_package
@init    {    int mode = 0;    }
    :    create_key ( or_key replace_key )? package_key
        ( package_spec {mode = 1;} | package_body )?
        SEMICOLON
    -> {mode == 1}?    ^(CREATE_PACKAGE_SPEC[$create_key.start] replace_key? package_spec) 
    -> ^(CREATE_PACKAGE_BODY[$create_key.start] replace_key? package_body)
    ;

// $<Create Package - Specific Clauses

package_body
    :    body_key package_name (is_key | as_key)
        package_obj_body*
        (begin_key seq_of_statements|end_key package_name?)
        -> package_name+ package_obj_body* seq_of_statements?
    ;

package_spec
    :    package_name invoker_rights_clause? (is_key | as_key)
        package_obj_spec*
        end_key package_name?
        -> package_name+ invoker_rights_clause? package_obj_spec*
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
        -> ^(PROCEDURE_SPEC[$procedure_key.start] procedure_name ^(PARAMETERS parameter*)) 
    ;

function_spec
    :    function_key function_name 
        (LEFT_PAREN parameter ( COMMA parameter)* RIGHT_PAREN )?
        return_key type_spec SEMICOLON 
        -> ^(FUNCTION_SPEC[$function_key.start] function_name type_spec ^(PARAMETERS parameter*))
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
        -> ^(DROP_PROCEDURE[$drop_key.start] procedure_name)
    ;

alter_procedure
    :    alter_key procedure_key procedure_name
        compile_key debug_key? compiler_parameters_clause*
        (reuse_key settings_key)?
        SEMICOLON
        -> ^(ALTER_PROCEDURE[$alter_key.start] procedure_name debug_key? reuse_key? compiler_parameters_clause*)
    ;

create_procedure_body
@init    {    int mode = 0;    }
    :    (create_key ( or_key replace_key )?)? procedure_key procedure_name
              ( LEFT_PAREN parameter ( COMMA parameter )* RIGHT_PAREN )? 
              invoker_rights_clause?
        ( is_key | as_key )
              (    declare_key? declare_spec* body 
              |    call_spec {mode = 2;}
              |    external_key {mode = 1;}
              )
              SEMICOLON
        ->    {mode == 1}?
            ^(CREATE_PROCEDURE[$procedure_key.start] replace_key? procedure_name ^(PARAMETERS parameter*)
                invoker_rights_clause? external_key)
        -> {mode == 2}?
            ^(CREATE_PROCEDURE[$procedure_key.start] replace_key? procedure_name ^(PARAMETERS parameter*)
                invoker_rights_clause?
                ^(CALL_MODE call_spec))
        ->    ^(CREATE_PROCEDURE[$procedure_key.start] replace_key? procedure_name ^(PARAMETERS parameter*)
                invoker_rights_clause?
                ^(BODY_MODE declare_spec* body))
      ;

// $>

// $<Trigger DDLs

drop_trigger
    :    drop_key trigger_key trigger_name
        SEMICOLON
        -> ^(DROP_TRIGGER[$drop_key.start] trigger_name)
    ;

alter_trigger
@init    {    int mode = 0;    }
    :    alter_key trigger_key tn1=trigger_name
    (    (enable_key|disable_key) {mode = 1;}
    |    rename_key to_key tn2=trigger_name {mode = 2;}
    |    compile_key debug_key? compiler_parameters_clause* (reuse_key settings_key)?
    )    SEMICOLON
    -> {mode == 1}? ^(ALTER_TRIGGER[$alter_key.start] $tn1 enable_key? disable_key?)
    -> {mode == 2}? ^(ALTER_TRIGGER[$alter_key.start] $tn1 ^(rename_key $tn2))
    -> ^(ALTER_TRIGGER[$alter_key.start] $tn1 debug_key? reuse_key? compiler_parameters_clause*)
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
    -> ^(CREATE_TRIGGER[$create_key.start] replace_key? trigger_name  
        simple_dml_trigger? compound_dml_trigger? non_dml_trigger?
        trigger_follows_clause? enable_key? disable_key? trigger_when_clause? trigger_body)
    ;

trigger_follows_clause
    :    follows_key trigger_name (COMMA trigger_name)*
        -> ^(follows_key trigger_name+)
    ;

trigger_when_clause
    :    when_key LEFT_PAREN condition RIGHT_PAREN
        -> ^(when_key ^(LOGIC_EXPR condition))
    ;

// $<Create Trigger- Specific Clauses
simple_dml_trigger
    :    (before_key|after_key|instead_key of_key) dml_event_clause referencing_clause? for_each_row?
        -> ^(SIMPLE_DML before_key? after_key? instead_key? for_each_row? referencing_clause? dml_event_clause)
    ;

for_each_row
    :    for_key each_key row_key -> FOR_EACH_ROW[$for_key.start]
    ;

compound_dml_trigger
    :    for_key dml_event_clause referencing_clause?
        -> ^(COMPOUND_DML[$for_key.start] referencing_clause? dml_event_clause)
    ;

non_dml_trigger
    :    (before_key|after_key) non_dml_event (or_key non_dml_event)*
        on_key (database_key | (schema_name PERIOD)? schema_key )
        -> ^(NON_DML before_key? after_key? non_dml_event+ database_key? schema_name? schema_key?)
    ;

trigger_body
    :    (compound_key trigger_key)=> compound_trigger_block
    |    (call_key id)=> call_key^ routine_clause 
    |    block -> ^(BODY_MODE block)
    ;

routine_clause
    :    routine_name function_argument?
    ;

compound_trigger_block
    :    compound_key trigger_key declare_spec* timing_point_section+ end_key trigger_name
        -> ^(compound_key trigger_name declare_spec* timing_point_section+)
    ;

timing_point_section
options{
k=3;
}    :    bk=before_key statement_key is_key block before_key statement_key SEMICOLON
        -> ^(BEFORE_STATEMENT[$bk.start] block)
    |    bk=before_key each_key row_key is_key block before_key each_key row_key SEMICOLON
        -> ^(BEFORE_EACH_ROW[$bk.start] block)
    |    ak=after_key statement_key is_key block after_key statement_key SEMICOLON
        -> ^(AFTER_STATEMENT[$ak.start] block)
    |    ak=after_key each_key row_key is_key block after_key each_key row_key SEMICOLON
        -> ^(AFTER_EACH_ROW[$ak.start] block)
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
        -> ^(DML_EVENT dml_event_element+ ^(on_key tableview_name dml_event_nested_clause?))  
    ;

dml_event_element
    :    (delete_key|insert_key|update_key) (of_key column_name (COMMA column_name)*)?
        -> ^(DML_EVENT_ELEMENT delete_key? insert_key? update_key? ^(COLUMNS column_name*))
    ;

dml_event_nested_clause
    :    nested_key table_key tableview_name of_key
        -> ^(nested_key tableview_name)
    ;

referencing_clause
    :    referencing_key^ referencing_element+
    ;

referencing_element
    :    ( new_key^ | old_key^ | parent_key^ ) column_alias
    ;

// $>
// $>

// $<Type DDLs

drop_type
    :    drop_key type_key body_key? type_name (force_key|validate_key)?
        SEMICOLON
        -> ^(DROP_TYPE[$drop_key.start] type_name body_key? force_key? validate_key?)
    ;

alter_type
@init    {    int mode = 0;    }
    :    alter_key type_key type_name
    (    compile_type_clause
    |    replace_type_clause {mode = 1;}
    |    {LT(2)->getText() == ("ATTRIBUTE")}? alter_attribute_definition {mode = 2;}
    |    alter_method_spec {mode = 3;}
    |    alter_collection_clauses {mode = 4;}
    |    modifier_clause {mode = 5;}
    )
        dependent_handling_clause?
        SEMICOLON
        -> {mode == 1}? ^(ALTER_TYPE[$alter_key.start] type_name replace_type_clause dependent_handling_clause?)
        -> {mode == 2}? ^(ALTER_TYPE[$alter_key.start] type_name alter_attribute_definition dependent_handling_clause?)
        -> {mode == 3}? ^(ALTER_TYPE[$alter_key.start] type_name alter_method_spec dependent_handling_clause?)
        -> {mode == 4}? ^(ALTER_TYPE[$alter_key.start] type_name alter_collection_clauses dependent_handling_clause?)
        -> {mode == 5}? ^(ALTER_TYPE[$alter_key.start] type_name modifier_clause dependent_handling_clause?)
        -> ^(ALTER_TYPE[$alter_key.start] type_name compile_type_clause dependent_handling_clause?)
    ;

// $<Alter Type - Specific Clauses
compile_type_clause
    :    compile_key debug_key? (specification_key|body_key)? compiler_parameters_clause* (reuse_key settings_key)?
        -> ^(compile_key specification_key? body_key? debug_key? reuse_key? compiler_parameters_clause*)
    ;

replace_type_clause
    :    replace_key invoker_rights_clause? as_key object_key
        LEFT_PAREN object_member_spec (COMMA object_member_spec)* RIGHT_PAREN
        -> ^(replace_key ^(OBJECT_MEMBERS object_member_spec+) invoker_rights_clause?)
    ;

alter_method_spec
    :     alter_method_element (COMMA alter_method_element)*
        -> ^(ALTER_METHOD alter_method_element+)
    ;

alter_method_element
    :    (add_key|drop_key) (map_order_function_spec|subprogram_spec)
        -> ^(ALTER_METHOD_ELEMENT add_key? drop_key? map_order_function_spec? subprogram_spec?)
    ;

alter_attribute_definition
    :    (add_key|modify_key|drop_key) attribute_key
        (    attribute_definition
        |    LEFT_PAREN attribute_definition (COMMA attribute_definition)* RIGHT_PAREN
        )
        -> ^(ALTER_ATTRIBUTE add_key? modify_key? drop_key? ^(ATTRIBUTES attribute_definition+))
    ;

attribute_definition
    :    attribute_name type_spec?
        -> ^(ATTRIBUTE attribute_name type_spec?)
    ;

alter_collection_clauses
@init    {    int mode = 0;    }
    :    modify_key
    (    limit_key expression
    |    element_key type_key type_spec {mode = 1;}
    )
        -> {mode == 1}? ^(ALTER_COLLECTION[$modify_key.start] ^(type_key type_spec))
        ->  ^(ALTER_COLLECTION[$modify_key.start] ^(limit_key ^(EXPR expression))) 
    ;

dependent_handling_clause
@init    {    int mode = 0;    }
    :    invalidate_key -> ^(DEPENDENT_HANDLING invalidate_key)
    |    cascade_key
        (    convert_key to_key substitutable_key {mode = 1;}
        |    not_key? including_key table_key data_key {mode = 2;}
        )?
        dependent_exceptions_part?
        -> {mode == 1}? ^(DEPENDENT_HANDLING ^(cascade_key convert_key) dependent_exceptions_part?)
        -> {mode == 2}? ^(DEPENDENT_HANDLING ^(cascade_key not_key? including_key) dependent_exceptions_part?)
        -> ^(DEPENDENT_HANDLING ^(cascade_key) dependent_exceptions_part?)
    ;

dependent_exceptions_part
    :    force_key? exceptions_key into_key tableview_name
        -> ^(exceptions_key force_key? tableview_name)
    ;

// $>

create_type
@init    {    int mode = 0;    }
    :    create_key (or_key replace_key)? type_key
        ( type_definition | type_body {mode = 1;})
        SEMICOLON
        ->{mode == 1}? ^(CREATE_TYPE_BODY[$create_key.start] replace_key? type_body)
        -> ^(CREATE_TYPE_SPEC[$create_key.start] replace_key? type_definition)
    ;

// $<Create Type - Specific Clauses
type_definition
    :    type_name 
        (oid_key CHAR_STRING)?
        object_type_def?
        -> type_name CHAR_STRING? object_type_def?
    ;

object_type_def
    :    invoker_rights_clause?
    (    object_as_part
    |    object_under_part
    )
        sqlj_object_type?
        (LEFT_PAREN object_member_spec (COMMA object_member_spec)* RIGHT_PAREN)?
        modifier_clause*
        -> ^(OBJECT_TYPE_DEF object_as_part? object_under_part? invoker_rights_clause?
             sqlj_object_type? modifier_clause* ^(OBJECT_MEMBERS object_member_spec*))  
    ;

object_as_part
    :    (is_key|as_key) (object_key|varray_type_def|nested_table_type_def)
        -> ^(OBJECT_AS object_key? varray_type_def? nested_table_type_def?)
    ;

object_under_part
    :    under_key^ type_spec
    ;

nested_table_type_def
    :    table_key of_key type_spec
        (not_key null_key)?
        -> ^(NESTED_TABLE_TYPE_DEF[$table_key.start] type_spec null_key?) 
    ;

sqlj_object_type
    :    external_key name_key expression language_key java_key using_key (sqldata_key|customdatum_key|oradata_key)
        -> ^(java_key ^(EXPR expression) sqldata_key? customdatum_key? oradata_key?)
    ;

type_body
    :    body_key type_name
        (is_key|as_key) type_body_elements (COMMA type_body_elements)*
        end_key
        -> type_name ^(TYPE_BODY_ELEMENTS type_body_elements+)
    ;

type_body_elements
    :    map_order_func_declaration
    |    subprog_decl_in_type
    ;

map_order_func_declaration
    :    (map_key^|order_key^) member_key! func_decl_in_type
    ;

subprog_decl_in_type
    :    (member_key^|static_key^)
    (    proc_decl_in_type
    |    func_decl_in_type
    |    constructor_declaration)
    ;

proc_decl_in_type
@init    {    int mode = 0;    }
    :    procedure_key procedure_name
        LEFT_PAREN type_elements_parameter (COMMA type_elements_parameter)* RIGHT_PAREN
        (is_key|as_key) 
            (call_spec {mode = 1;}|declare_key? declare_spec* body SEMICOLON)
        -> {mode == 1}? ^(CREATE_PROCEDURE procedure_name ^(PARAMETERS type_elements_parameter+) ^(CALL_MODE call_spec))
        -> ^(CREATE_PROCEDURE procedure_name ^(PARAMETERS type_elements_parameter+) ^(BODY_MODE declare_spec* body))
    ;

func_decl_in_type
@init    {    int mode = 0;    }
    :    function_key function_name 
        (LEFT_PAREN type_elements_parameter (COMMA type_elements_parameter)* RIGHT_PAREN)? 
        return_key type_spec
        (is_key|as_key)
            (call_spec {mode = 1;}|declare_key? declare_spec* body SEMICOLON)
        -> {mode == 1}? ^(CREATE_FUNCTION function_name type_spec ^(PARAMETERS type_elements_parameter*) ^(CALL_MODE call_spec))
        -> ^(CREATE_FUNCTION function_name type_spec ^(PARAMETERS type_elements_parameter*) ^(BODY_MODE declare_spec* body))
    ;

constructor_declaration
@init    {    int mode = 0;    }
    :    final_key? instantiable_key? constructor_key function_key type_spec
        (LEFT_PAREN (self_key in_key out_key type_spec COMMA) type_elements_parameter (COMMA type_elements_parameter)*  RIGHT_PAREN)?
        return_key self_key as_key result_key 
        (is_key|as_key) 
            (call_spec {mode = 1;}|declare_key? declare_spec* body SEMICOLON)
        -> {mode == 1}? ^(constructor_key type_spec final_key? instantiable_key? ^(PARAMETERS type_elements_parameter*) ^(CALL_MODE call_spec))
        -> ^(constructor_key type_spec final_key? instantiable_key? ^(PARAMETERS type_elements_parameter*) ^(BODY_MODE declare_spec* body))
    ;

// $>

// $<Common Type Clauses

modifier_clause
    :    not_key? (instantiable_key|final_key|overriding_key)
    ->    ^(MODIFIER not_key? instantiable_key? final_key? overriding_key?)
    ;

object_member_spec
options{
backtrack=true;
}    :    id type_spec sqlj_object_type_attr? -> ^(ELEMENT_SPEC ^(FIELD_SPEC id type_spec sqlj_object_type_attr?))
    |    element_spec
    ;

sqlj_object_type_attr
    :    external_key name_key expression
        -> ^(external_key ^(EXPR expression))
    ;

element_spec
    :    modifier_clause?
        element_spec_options+
        (COMMA pragma_clause)?
        -> ^(ELEMENT_SPEC element_spec_options+ modifier_clause? pragma_clause?)
    ;

element_spec_options
    :    subprogram_spec
    |    constructor_spec
    |    map_order_function_spec
    ;

subprogram_spec
    :    (member_key^|static_key^)
        (type_procedure_spec|type_function_spec)
    ;

type_procedure_spec
@init    {    int mode = 0;    }
    :    procedure_key procedure_name
        LEFT_PAREN type_elements_parameter (COMMA type_elements_parameter)* RIGHT_PAREN
        ((is_key|as_key) call_spec {mode = 1;})?
        ->{mode == 1}? ^(PROCEDURE_SPEC procedure_name ^(PARAMETERS type_elements_parameter+) ^(CALL_MODE call_spec))
        -> ^(PROCEDURE_SPEC procedure_name ^(PARAMETERS type_elements_parameter+))
    ;

type_function_spec
@init    {    int mode = 0;    }
    :    function_key function_name 
        (LEFT_PAREN type_elements_parameter (COMMA type_elements_parameter)* RIGHT_PAREN)? 
        return_key ( type_spec | self_key as_key result_key)
        ((is_key|as_key) call_spec {mode = 1;}| external_key variable_key? name_key expression {mode = 2;})?
        ->{mode == 1}? ^(FUNCTION_SPEC function_name type_spec? self_key? ^(PARAMETERS type_elements_parameter*) ^(CALL_MODE call_spec))
        ->{mode == 2}? ^(FUNCTION_SPEC function_name type_spec? self_key? ^(PARAMETERS type_elements_parameter*) ^(external_key ^(EXPR expression)))
        -> ^(FUNCTION_SPEC function_name type_spec? self_key? ^(PARAMETERS type_elements_parameter*))
    ;

constructor_spec
@init    {    int mode = 0;    }
    :    final_key? instantiable_key? constructor_key function_key type_spec
        (LEFT_PAREN (self_key in_key out_key type_spec COMMA) type_elements_parameter (COMMA type_elements_parameter)*  RIGHT_PAREN)?
        return_key self_key as_key result_key ((is_key|as_key) call_spec {mode = 1;})?
        ->{mode == 1}? ^(CONSTRUCTOR_SPEC[$constructor_key.start] type_spec final_key? instantiable_key? ^(PARAMETERS type_elements_parameter*) ^(CALL_MODE call_spec))
        -> ^(CONSTRUCTOR_SPEC[$constructor_key.start] type_spec final_key? instantiable_key? ^(PARAMETERS type_elements_parameter*))
    ;

map_order_function_spec
    :    (map_key^|order_key^) member_key! type_function_spec
    ;

pragma_clause
    :    pragma_key restrict_references_key LEFT_PAREN pragma_elements (COMMA pragma_elements)* RIGHT_PAREN
        -> ^(pragma_key pragma_elements+)
    ;

pragma_elements
    :    id
    |    default_key
    ;

type_elements_parameter
    :    parameter_name type_spec
        -> ^(PARAMETER parameter_name type_spec)
    ;

// $>
// $>

// $<Sequence DDLs

drop_sequence
    :   drop_key sequence_key sequence_name
        SEMICOLON
        -> ^(DROP_SEQUENCE[$drop_key.start] sequence_name)
    ;

alter_sequence
    :    alter_key sequence_key sequence_name sequence_spec+
         SEMICOLON
        -> ^(ALTER_SEQUENCE[$alter_key.start] sequence_name sequence_spec+)
    ;

create_sequence
    :    create_key sequence_key sequence_name
    (    sequence_start_clause
    |    sequence_spec
    )*   SEMICOLON
        -> ^(CREATE_SEQUENCE[$create_key.start] sequence_name sequence_start_clause* sequence_spec*)
    ;

// $<Common Sequence

sequence_spec
    :    increment_key^ by_key! UNSIGNED_INTEGER
    |    maxvalue_key^ UNSIGNED_INTEGER
    |    minvalue_key^ UNSIGNED_INTEGER
    |    cache_key^ UNSIGNED_INTEGER
    |    nomaxvalue_key
    |    nominvalue_key
    |    cycle_key
    |    nocycle_key
    |    nocache_key
    |    order_key
    |    noorder_key
    ;

sequence_start_clause
    :    start_key^ with_key! UNSIGNED_INTEGER
    ;

// $>
// $>


// $<Common DDL Clauses

invoker_rights_clause
    :    authid_key (current_user_key|definer_key)
        -> ^(authid_key current_user_key? definer_key?)
    ;

compiler_parameters_clause
    :    id EQUALS_OP expression
        -> ^(COMPILER_PARAMETER ^(ASSIGN[$EQUALS_OP] id ^(EXPR expression)))
    ;

call_spec
    :    language_key^ ( java_spec | c_spec )
    ;

// $<Call Spec - Specific Clauses

java_spec
    :    java_key^ name_key! CHAR_STRING
    ;

c_spec
    :    c_key (name_key CHAR_STRING)? 
        library_key id
        c_agent_in_clause? (with_key context_key)? c_parameters_clause?
        -> ^(c_key CHAR_STRING? context_key? ^(library_key id) c_agent_in_clause? c_parameters_clause?)
    ;

c_agent_in_clause
    :    agent_key in_key LEFT_PAREN expression (COMMA expression)* RIGHT_PAREN
        -> ^(agent_key ^(EXPR expression)+)
    ;

c_parameters_clause
@init    {    int mode = 0;    }
    :    parameters_key LEFT_PAREN (expression (COMMA expression)* | DOUBLE_PERIOD PERIOD {mode = 1;}) RIGHT_PAREN
    -> {mode == 1}?    ^(parameters_key THREE_DOTS[$DOUBLE_PERIOD, "..."])
    -> ^(parameters_key ^(EXPR expression)+)
    ;

// $>

parameter
    :    parameter_name ( in_key | out_key | inout_key | nocopy_key)* type_spec? default_value_part?
        -> ^(PARAMETER parameter_name in_key* out_key* inout_key* type_spec? default_value_part?)
    ;

default_value_part
    :    (ASSIGN_OP|default_key) expression
        -> ^(DEFAULT_VALUE ^(EXPR expression))
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
        -> ^(VARIABLE_DECLARE variable_name type_spec constant_key? null_key? default_value_part?)
    ;    

subtype_declaration
@init    {    int mode = 0;    }
      :    subtype_key type_name is_key type_spec
          (range_key expression DOUBLE_PERIOD expression {mode = 1;})?
          (not_key null_key)? SEMICOLON
          -> {mode == 1}? ^(SUBTYPE_DECLARE[$subtype_key.start] type_name type_spec null_key? ^(range_key ^(EXPR expression)+))
          -> ^(SUBTYPE_DECLARE[$subtype_key.start] type_name type_spec null_key?)
      ;

//cursor_declaration incorportates curscursor_body and cursor_spec
cursor_declaration
    :    cursor_key cursor_name
              (LEFT_PAREN parameter_spec (COMMA parameter_spec)* RIGHT_PAREN )?
              (return_key type_spec)? (is_key select_statement)? SEMICOLON
        -> ^(CURSOR_DECLARE[$cursor_key.start] cursor_name type_spec? select_statement? ^(PARAMETERS parameter_spec*)) 
    ;

parameter_spec
    :    parameter_name (in_key? type_spec)? 
        default_value_part?
        -> ^(PARAMETER parameter_name type_spec? default_value_part?)
    ;

exception_declaration 
    :    exception_name exception_key SEMICOLON
        -> ^(EXCEPTION_DECLARE[$exception_key.start] exception_name)
    ;             

pragma_declaration
@init    {    int mode = 0;    }
    :    pragma_key
        (    serially_reusable_key {mode = 1;}
        |    autonomous_transaction_key {mode = 2;}
        |    exception_init_key LEFT_PAREN exception_name COMMA numeric RIGHT_PAREN {mode = 3;} 
        |    inline_key LEFT_PAREN id1=id COMMA expression RIGHT_PAREN {mode = 4;}
        |    restrict_references_key LEFT_PAREN (id|default_key) (COMMA id)+ RIGHT_PAREN
        )
        SEMICOLON
        -> {mode == 1}? ^(PRAGMA_DECLARE[$pragma_key.start] serially_reusable_key) 
        -> {mode == 2}? ^(PRAGMA_DECLARE[$pragma_key.start] autonomous_transaction_key) 
        -> {mode == 3}? ^(PRAGMA_DECLARE[$pragma_key.start] ^(exception_init_key exception_name numeric) ) 
        -> {mode == 4}? ^(PRAGMA_DECLARE[$pragma_key.start] ^(inline_key $id1 ^(EXPR expression)))
        -> ^(PRAGMA_DECLARE[$pragma_key.start] ^(restrict_references_key default_key? id*)) 
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
    -> ^(RECORD_TYPE_DECLARE[$type_key.start] type_name ref_key? type_spec? ^(FIELDS field_spec*))
    ;

field_spec
    :    column_name type_spec?
        (not_key null_key)?
        default_value_part?
        -> ^(FIELD_SPEC column_name type_spec? null_key? default_value_part?)
    ;

record_var_dec
    :    record_name type_name (percent_rowtype_key|percent_type_key) SEMICOLON
        -> ^(RECORD_VAR_DECLARE record_name type_name percent_rowtype_key? percent_type_key?)
    ;

// $>

table_declaration
    :    (table_type_dec | table_var_dec) SEMICOLON!
    ;

table_type_dec
@init    {    int mode = 0;    }
    :    type_key type_name is_key 
    (    table_key
            of_key type_spec 
            table_indexed_by_part?
        (not_key null_key)?
    |    varray_type_def {mode = 1;}
    )
        ->{mode == 1}? ^(TABLE_TYPE_DECLARE[$type_key.start] type_name varray_type_def)
        -> ^(TABLE_TYPE_DECLARE[$type_key.start] type_name null_key? ^(table_key type_spec table_indexed_by_part?))
    ;

table_indexed_by_part
    :    (idx1=indexed_key|idx2=index_key) by_key type_spec
        ->{idx1.tree != NULL}? ^(INDEXED_BY[$idx1.start] type_spec)
        -> ^(INDEXED_BY[$idx2.start] type_spec)
    ;

varray_type_def
    :    (varray_key| varying_key array_key)
        LEFT_PAREN expression RIGHT_PAREN 
        of_key type_spec
        (not_key null_key)?
        -> null_key? ^(VARR_ARRAY_DEF ^(EXPR expression) type_spec)
    ;

table_var_dec
    :    table_var_name type_spec
    ->    ^(TABLE_VAR_DECLARE table_var_name type_spec)
    ;

// $>

// $<PL/SQL Statements

seq_of_statements
    :     (statement (SEMICOLON|EOF)|label_declaration)+
        -> ^(STATEMENTS label_declaration* statement*)
    ;  

label_declaration
    :    ltp1=LESS_THAN_OP LESS_THAN_OP label_name GREATER_THAN_OP GREATER_THAN_OP
    ->    ^(LABEL_DECLARE[$ltp1] label_name)
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
        -> ^(ASSIGN[$ASSIGN_OP] general_element? bind_variable? ^(EXPR expression))
    ;

continue_statement
@init    {    int mode = 0;    }
    :    continue_key label_name? (when_key condition {mode = 1;})?
        ->{mode == 1}? ^(continue_key label_name? ^(when_key ^(LOGIC_EXPR condition)))
        -> ^(continue_key label_name?)
    ;

exit_statement
@init    {    int mode = 0;    }
    :    exit_key label_name? (when_key condition {mode = 1;})?
        ->{mode == 1}? ^(exit_key label_name? ^(when_key ^(LOGIC_EXPR condition)))
        -> ^(exit_key label_name?)
    ;

goto_statement
    :    goto_key^ label_name
    ;

if_statement
    :    if_key condition then_key seq_of_statements
        elsif_part* else_part?
        end_key if_key
        -> ^(if_key ^(LOGIC_EXPR condition) seq_of_statements elsif_part* else_part?)
    ;

elsif_part
    :    elsif_key condition then_key seq_of_statements
        -> ^(elsif_key ^(LOGIC_EXPR condition) seq_of_statements)
    ;

else_part
    :    else_key^ seq_of_statements
    ;

loop_statement
@init    {    int mode = 0;    } 
    :    label_name?
        (while_key condition {mode = 1;} | for_key cursor_loop_param {mode = 2;})?
        loop_key
        seq_of_statements
        end_key loop_key label_name?
        -> {mode == 1}? ^(WHILE_LOOP[$while_key.start] label_name* ^(LOGIC_EXPR condition) seq_of_statements)
        -> {mode == 2}? ^(FOR_LOOP[$for_key.start] label_name* cursor_loop_param seq_of_statements)
        -> ^(loop_key label_name* seq_of_statements)
    ;

// $<Loop - Specific Clause

cursor_loop_param
@init    {    int mode = 0;    }
    :    (index_name in_key reverse_key? lower_bound DOUBLE_PERIOD)=> 
            index_name in_key reverse_key? lower_bound DOUBLE_PERIOD upper_bound
        -> ^(INDEXED_FOR index_name reverse_key? ^(SIMPLE_BOUND lower_bound upper_bound))
    |     record_name in_key ( cursor_name expression_list? {mode = 1;} | LEFT_PAREN select_statement RIGHT_PAREN)
        ->{mode == 1}? ^(CURSOR_BASED_FOR record_name cursor_name expression_list?)
        -> ^(SELECT_BASED_FOR record_name select_statement)
    ;
// $>

forall_statement
    :    forall_key index_name in_key bounds_clause sql_statement (save_key exceptions_key)?
        -> ^(forall_key index_name bounds_clause sql_statement exceptions_key?)
    ;

bounds_clause
    :    lower_bound DOUBLE_PERIOD upper_bound -> ^(SIMPLE_BOUND lower_bound upper_bound)
    |    indices_key of_key collection_name between_bound?
        -> ^(INDICES_BOUND[$indices_key.start] collection_name between_bound?)
    |    values_key of_key index_name -> ^(VALUES_BOUND[$values_key.start] index_name) 
    ;

between_bound
    :    between_key lower_bound and_key upper_bound
        -> ^(between_key lower_bound upper_bound)
    ;

lower_bound
    :    concatenation
        -> ^(EXPR concatenation)
    ;

upper_bound
    :    concatenation
        -> ^(EXPR concatenation)
    ;

null_statement
    :    null_key
    ;

raise_statement
    :    raise_key^ exception_name?
    ;

return_statement
    :    return_key cn1=condition?
    -> {cn1.tree != NULL}? ^(return_key ^(LOGIC_EXPR condition))
    -> ^(return_key)
    ;

function_call
    :    call_key? routine_name function_argument?
        -> ^(ROUTINE_CALL routine_name function_argument?)
    ;

body
    :    begin_key
        seq_of_statements
        exception_clause?
        end_key label_name?
        -> ^(BODY[$begin_key.start] label_name? seq_of_statements exception_clause?) 
    ;

// $<Body - Specific Clause

exception_clause
    :    exception_key^ exception_handler+
    ;

exception_handler
    :    when_key exception_name (or_key exception_name)* then_key
        seq_of_statements
        -> ^(when_key exception_name+ seq_of_statements)
    ;

// $>

block
    :    (declare_key declare_spec+)?
        body
        -> ^(BLOCK declare_spec* body)
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
        -> ^(execute_key ^(EXPR expression) into_clause? using_clause? dynamic_returning_clause?) 
    ;

// $<Execute Immediate - Specific Clause
dynamic_returning_clause
    :    (returning_key|return_key) into_clause
        -> ^(DYNAMIC_RETURN into_clause)
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

// $>

// $<Cursor Manipulation SQL PL/SQL Statements

cursor_manipulation_statements
    :    close_statement
    |    open_statement
    |    fetch_statement
    |    open_for_statement
    ;

close_statement
    :     close_key^ cursor_name
    ;

open_statement
    :    open_key^ cursor_name expression_list?
    ;

fetch_statement
@init    {    int mode = 0;    }
    :    fetch_key cursor_name 
    (    it1=into_key variable_name (COMMA variable_name )* {mode = 1;}
    |    bulk_key collect_key into_key variable_name (COMMA variable_name )*
    )
        ->{mode == 1}? ^(fetch_key cursor_name ^($it1 variable_name+))
        -> ^(fetch_key cursor_name ^(bulk_key variable_name+))
    ;

open_for_statement
@init    {    int mode = 0;    }
    :    open_key variable_name for_key
    (    (select_key|with_key)=> select_statement {mode = 1;}
    |    expression
    )
        using_clause?
        ->{mode == 1}? ^(open_key variable_name select_statement using_clause?)
        -> ^(open_key variable_name ^(EXPR expression) using_clause?)
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
@init    {    int mode = 0;    }
    :    set_key transaction_key 
    (    read_key (only_key|write_key) {mode = 1;} 
    |    isolation_key level_key (serializable_key|read_key committed_key) {mode = 2;}
    |    use_key rollback_key segment_key rollback_segment_name {mode = 3;}
    )?
        (name_key quoted_string)?
        ->{mode == 1}? ^(SET_TRANSACTION[$set_key.start] quoted_string? ^(read_key only_key? write_key?))
        ->{mode == 2}? ^(SET_TRANSACTION[$set_key.start] quoted_string? ^(isolation_key serializable_key? committed_key?))
        ->{mode == 2}? ^(SET_TRANSACTION[$set_key.start] quoted_string? ^(rollback_key rollback_segment_name))
        -> ^(SET_TRANSACTION[$set_key.start] quoted_string?)
    ;

set_constraint_command
    :    set_key ( constraint_key | constraints_key )
        ( all_key | constraint_name (COMMA constraint_name)* )
        (immediate_key|deferred_key)
        -> ^(SET_CONSTRAINT[$set_key.start] all_key? constraint_name* immediate_key? deferred_key?)
    ;

commit_statement
@init    {    int mode = 0;    }
    :     commit_key work_key?
    (    comment_key expression {mode = 1;}
    |    force_key (corrupt_xid_key expression| corrupt_xid_all_key | expression (COMMA expression)?) {mode = 2;} 
    )?
        write_clause?
        -> {mode == 1}? ^(commit_key work_key? ^(comment_key ^(EXPR expression)) write_clause?)
        -> {mode == 2}? ^(commit_key work_key? ^(force_key corrupt_xid_key? corrupt_xid_all_key? ^(EXPR expression)+) write_clause?)
        -> ^(commit_key work_key? write_clause?)
    ;

write_clause
    :    write_key^ (wait_key|nowait_key)? (immediate_key|batch_key)?
    ;

rollback_statement
@init    {    int mode = 0;    }
    :     rollback_key work_key?
    (     to_key savepoint_key? savepoint_name {mode = 1;}
    |    force_key quoted_string {mode = 2;}
    )?
        -> {mode == 1}? ^(rollback_key work_key? ^(to_key savepoint_name))
        -> {mode == 2}? ^(rollback_key work_key? ^(force_key quoted_string))
        -> ^(rollback_key work_key?)
    ;

savepoint_statement
    :    savepoint_key^ savepoint_name 
    ;

// $>
