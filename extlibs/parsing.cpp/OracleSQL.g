/**
 * Oracle(c) PL/SQL 11g Lexer/Parser  
 *
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
 *
 */

lexer grammar PLSQLLexer;

options {
        language=Cpp;
//        filter=true;
}

@lexer::includes 
{
#include "UserGuiTraits.hpp"
#include <vector>
}

@lexer::namespace{ Antlr3BackendImpl }

/* ================================================================================
   Oracle reserved words
   cannot by used for name database objects such as columns, tables, or indexes.
   ================================================================================ */
RESERVED:
		r='ACCESS'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ADD'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ALL'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
	|	r='ALTER'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='AND'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ANY'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
	|	r='ARRAYLEN'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='AS'                        // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ASC'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='AUDIT'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='BETWEEN'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
	|	r='BY'                        // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='CASE'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ; //PL/SQL
	|	r='CHAR'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='CHECK'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='CLUSTER'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='COLUMN'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='COMMENT'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='COMPRESS'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='CONNECT'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='CREATE'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='CURRENT'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='DATE'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='DECIMAL'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='DEFAULT'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='DELETE'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='DESC'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='DISTINCT'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='DROP'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ELSE'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='EXCLUSIVE'                 // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='EXISTS'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_UNARY;  }  ;
	|	r='FALSE'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ; //PL/SQL
	|	r='FILE'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='FLOAT'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='FOR'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='FROM'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='GRANT'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='GROUP'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='HAVING'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='IDENTIFIED'                // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='IMMEDIATE'                 // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='IN'                        // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
	|	r='INCREMENT'                 // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='INDEX'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='INITIAL'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='INSERT'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='INTEGER'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='INTERSECT'                 // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='INTO'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='IS'                        // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_UNARY;  }  ;
	|	r='LEVEL'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='LIKE'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
	|	r='LOCK'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='LONG'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='MAXEXTENTS'                // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='MINUS'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='MODE'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='MODIFY'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='NOAUDIT'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='NOCOMPRESS'                // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='NOT'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
	|	r='NOTFOUND'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='NOWAIT'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='NULL'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
	|	r='NUMBER'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='OF'                        // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
	|	r='OFFLINE'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ON'                        // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ONLINE'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='OPTION'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='OR'                        // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ORDER'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='PCTFREE'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='PRIOR'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
	|	r='PRIVILEGES'                // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='PUBLIC'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='RAW'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='RENAME'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='RESOURCE'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='REVOKE'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ROW'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ROWID'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ROWLABEL'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ROWNUM'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='ROWS'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SELECT'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SESSION'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SET'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
	|	r='SHARE'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SIZE'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SMALLINT'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SQLBUF'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='START'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SUCCESSFUL'                // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SYNONYM'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='SYSDATE'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='TABLE'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='THEN'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='TO'                        // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='TRIGGER'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='TRUE'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ; // PL/SQL
	|	r='UID'                       // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='UNION'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='UNIQUE'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='UPDATE'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='USER'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='VALIDATE'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='VALUES'                    // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='VARCHAR'                   // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='VARCHAR2'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='VIEW'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='WHENEVER'                  // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='WHERE'                     // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	|	r='WITH'                      // { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
	; 
/* ================================================================================
   Oracle keywords
   can by used for name database objects such as columns, tables, or indexes.
   ================================================================================ */


//k_a : r='A' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
//k_a : { !(LT(1)->getText(LT(1))->compare8(LT(1)->getText(LT(1)), "AS")) }? ID;
k_a	: { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "A")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;

k_at                    : r='AT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_at                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "AT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_automatic             : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "AUTOMATIC")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_avg                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "AVG")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_bfile                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BFILE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_binary_double         : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BINARY_DOUBLE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_binary_float          : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BINARY_FLOAT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_binary_integer        : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BINARY_INTEGER")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_blob                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BLOB")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_block                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BLOCK")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_boolean               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BOOLEAN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_both                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BOTH")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_breadth               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BREADTH")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_byte                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "BYTE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_cast                  : r='CAST' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_cast                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CAST")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_character             : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CHARACTER")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_clob                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CLOB")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_cluster_set           : r='CLUSTER_SET' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_cluster_set         : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CLUSTER_SET")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_column_value        : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "COLUMN_VALUE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_connect_by_iscycle  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CONNECT_BY_ISCYCLE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_connect_by_isleaf   : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CONNECT_BY_ISLEAF")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_connect_by_root       : r='CONNECT_BY_ROOT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_connect_by_root     : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CONNECT_BY_ROOT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_constraint            : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CONSTRAINT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_corr                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CORR")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_count                 : r='COUNT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_count               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "COUNT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ; 
//k_covar_pop           : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "COVAR_POP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_covar_samp          : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "COVAR_SAMP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_cross                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CROSS")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_cube                  : r='CUBE' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_cube                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CUBE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_cume_dist           : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CUME_DIST")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_cursor                : r='CURSOR' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_cursor              : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CURSOR")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_cycle                 : r='CYCLE' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_cycle               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "CYCLE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_day                   : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DAY")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_dbtimezone            : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DBTIMEZONE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_dec                   : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DEC")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_decrement             : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DECREMENT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_dense_rank            : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DENSE_RANK")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_depth                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DEPTH")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_dimension             : r='DIMENSION' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_dimension           : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DIMENSION")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_double                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "DOUBLE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_empty                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "EMPTY")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
k_end                   : r='END' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
//k_end                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "END")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
k_equals_path           : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "EQUALS_PATH")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_UNARY;  }  ;
k_errors                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ERRORS")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_escape                : r='ESCAPE' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_escape              : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ESCAPE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_exclude               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "EXCLUDE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_extract               : r='EXTRACT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_extract             : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "EXCLUDE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_first                 : r='FIRST' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_first : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "FIRST")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_first_value : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "FIRST_VALUE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_following : r='FOLLOWING' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_following : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "FOLLOWING")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_full : r='FULL' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_full : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "FULL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_grouping : r='GROUPING' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_grouping : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "GROUPING")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_hour : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "HOUR")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_ignore : r='IGNORE' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_ignore : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "IGNORE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_include : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "INCLUDE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_infinite : r='INFINITE' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
//k_infinite : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "INFINITE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
k_inner : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "INNER")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_int : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "INT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_interval : r='INTERVAL' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_interval : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "INTERVAL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_iterate : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ITERATE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_join : r='JOIN' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_join : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "JOIN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_keep : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "KEEP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_lag : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LAG")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_last : r='LAST' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_last : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LAST")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_last_value : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LAST_VALUE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_lead : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LEAD")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_leading : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LEADING")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_left : r='LEFT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_left : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LEFT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_like2 : r='LIKE2' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
//k_like2 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LIKE2")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
k_like4 : r='LIKE4' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
//k_like4 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LIKE4")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
k_likec : r='LIKEC' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
//k_likec : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LIKEC")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
k_limit                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LIMIT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_listagg               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LISTAGG")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_lnnvl                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LNNVL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_local                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LOCAL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_locked                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LOCKED")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_log                   : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "LOG"   )) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_main                  : r='MAIN' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_main                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MAIN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_matched               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MATCHED")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_max                 : r='MAX' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_max                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MAX")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_maxvalue              : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MAXVALUE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_measures              : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MEASURES")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_member                : r='MEMBER' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
//k_member              : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MEMBER")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
k_merge                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MERGE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_min                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MIN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_minute                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MINUTE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_mivalue               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MIVALUE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_mlslabel              : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MLSLABEL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_model : {
    !(strcasecmp((const char*)LT(2)->getText(LT(2))->chars, "MAIN")) ||
    !(strcasecmp((const char*)LT(2)->getText(LT(2))->chars, "PARTITION")) ||
    !(strcasecmp((const char*)LT(2)->getText(LT(2))->chars, "DIMENSION")) 
   }? r='MODEL' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
// i_model : {
//     (strcasecmp((const char*)LT(2)->getText(LT(2))->chars, "MAIN")) &&
//     (strcasecmp((const char*)LT(2)->getText(LT(2))->chars, "PARTITION")) &&
//     (strcasecmp((const char*)LT(2)->getText(LT(2))->chars, "DIMENSION")) 
//    }? r='MODEL' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_model : r='MODEL' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_model : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MODEL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_month : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MONTH")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_multiset : r='MULTISET' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_multiset : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "MULTISET")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_nan : r='NAN' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
//k_nan : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NAN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
k_national : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NATIONAL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_natural : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NATURAL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_nav : r='NAV' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_nav : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NAV")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_nchar : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NCHAR")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_nclob : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NCLOB")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_nocycle : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NOCYCLE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_ntile : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NTILE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_nulls : r='NULLS' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_nulls : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NULLS")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_numeric : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NUMERIC")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_nvarchar : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NVARCHAR")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_nvarchar2 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "NVARCHAR2")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_object_id : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "OBJECT_ID")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_object_value : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "OBJECT_VALUE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_only : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ONLY")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_ora_rowscn : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ORA_ROWSCN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_outer : r='OUTER' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_outer : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "OUTER")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_over : r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_over : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "OVER")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_partition : r='PARTITION' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_partition : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PARTITION")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_percent_rank : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PERCENT_RANK")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_percentile_cont : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PERCENTILE_CONT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_percentile_disc : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PERCENTILE_DISC")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_pivot : r='PIVOT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_pivot : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PIVOT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_pls_integer : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PLS_INTEGER")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_positive : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "POSITIVE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_preceding : r='PRECEDING' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_preceding : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PRECEDING")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_precision : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PRECISION")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_present : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "PRESENT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
k_range : r='RANGE' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_range : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "RANGE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_rank : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "RANK")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_ratio_to_report : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "RATIO_TO_REPORT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_read : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "READ")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_real : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REAL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_reference : r='REFERENCE' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_reference : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REFERENCE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_regexp_like : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGEXP_LIKE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_UNARY;  }  ;
//k_regr_avgx : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_AVGX")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_regr_avgy : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_AVGY")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_regr_count : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_COUNT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_regr_intercept : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_INTERCEPT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_regr_r2 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_R2")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_regr_slope : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_SLOPE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_regr_sxx : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_SXX")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_regr_sxy : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_SXY")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_regr_syy : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REGR_SYY")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_reject : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "REJECT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_respect : r='RESPECT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_respect : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "RESPECT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_return : r='RETURN' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_return : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "RETURN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_returning : r='RETURNING' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_returning : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "RETURNING")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_right : r='RIGHT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_right : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "RIGHT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_rollup : r='ROLLUP' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_rollup : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ROLLUP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_row_number : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ROW_NUMBER")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_rules : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "RULES")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_sample : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SAMPLE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_scn : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SCN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_search : r='SEARCH' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_search : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SEARCH")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_second : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SECOND")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_seed : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SEED")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_sequential : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SEQUENTIAL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_sessiontimezone : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SESSIONTIMEZONE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_sets : r='SETS' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_sets : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SETS")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_siblings : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SIBLINGS")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_single : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SINGLE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_skip : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SKIP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_some : r='SOME' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
//k_some : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SOME")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
k_sql : r='SQL' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_sql : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SQL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_stddev : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "STDDEV")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_stddev_pop : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "STDDEV_POP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_stddev_samp : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "STDDEV_SAMP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_submultiset : r='SUBMULTISET' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
//k_submultiset : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SUBMULTISET")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_BINARY;  }  ;
k_subpartition : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SUBPARTITION")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_sum : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "SUM")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_the : r='THE' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_the : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "THE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_time : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TIME")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_timestamp : r='TIMESTAMP' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_timestamp : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TIMESTAMP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_timezone_hour : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TIMEZONE_HOUR")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_timezone_minute : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TIMEZONE_MINUTE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_timezone_region : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TIMEZONE_REGION")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_timezone_abbr : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TIMEZONE_ABBR")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_to                    : r='TO' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_to                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TO")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_trailing              : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TRAILING")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_trim                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TRIM")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_type                : r='TYPE' { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
k_type                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "TYPE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_PART;  }  ;
k_unbounded             : r='UNBOUNDED' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_unbounded           : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "UNBOUNDED")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_under_path            : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "UNDER_PATH")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_OPERATOR_UNARY;  }  ;
k_unlimited             : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "UNLIMITED")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_unpivot               : r='UNPIVOT' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_unpivot             : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "UNPIVOT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_until                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "UNTIL")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_updated               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "UPDATED")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_upsert                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "UPSERT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_urowid                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "UROWID")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_using                 : r='USING' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_using               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "USING")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_value                 : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "VALUE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_var_pop             : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "VAR_POP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_var_samp            : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "VAR_SAMP")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_variance            : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "VARIANCE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_varying               : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "VARYING")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_versions              : r='VERSIONS' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_versions            : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "VERSIONS")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
// k_versions_endscn    : r='VERSIONS_ENDSCN' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
// k_versions_endtime   : r='VERSIONS_ENDTIME' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
// k_versions_operation : r='VERSIONS_OPERATION' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
// k_versions_starscn   : r='VERSIONS_STARSCN' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
// k_versions_starttime : r='VERSIONS_STARTTIME' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
// k_versions_xid       : r='VERSIONS_XID' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_wait                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "WAIT")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_when                  : r='WHEN' { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_when                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "WHEN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_within                : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "WITHIN")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_xml                   : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "XML")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
//k_xmldata             : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "XMLDATA")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_year                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "YEAR")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;
k_zone                  : { !(strcasecmp((const char*)LT(1)->getText(LT(1))->chars, "ZONE")) }? r=ID { $r->set_type($r, T_RESERVED); $r->user1 = T_RESERVED;  }  ;

keyword[int identifierClass, int usageType]
:	k=(
		//'A'	// note: this one is not listed in the docs but is a part of "IS A SET" condition clause
		'AT'
//         | 'AUTOMATIC'
//         | 'AVG'
//         | 'BFILE'
//         | 'BINARY_DOUBLE'
//         | 'BINARY_FLOAT'
//         | 'BINARY_INTEGER'
//         | 'BLOB'
//         | 'BLOCK'
//         | 'BOOLEAN'
//         | 'BREADTH'
//         | 'BYTE'
//         | 'CAST'
// NOTE!!! CASE is reserved word in Oracle but still can be used as identifier
         | 'CASE'
//         | 'CHARACTER'
//         | 'CLOB'
//         | 'CLUSTER_SET'
//         | 'COLUMN_VALUE'
//         | 'CONNECT_BY_ISCYCLE'
//         | 'CONNECT_BY_ISLEAF'
//         | 'CONNECT_BY_ROOT'
//         | 'CONSTRAINT'
//         | 'CORR'
         | 'COUNT'
//         | 'COVAR_POP'
//         | 'COVAR_SAMP'
//         | 'CROSS'
         | 'CUBE'
//         | 'CUME_DIST'
//             //| 'CURSOR'
         | 'CYCLE'
//         | 'DAY'
//         | 'DBTIMEZONE'
//         | 'DEC'
//         | 'DECREMENT'
//             //| 'DENSE_RANK'
//         | 'DEPTH'
//         | 'DIMENSION'
//         | 'DOUBLE'
//         | 'EMPTY'
//         | 'END'
//         | 'EQUALS_PATH'
//         | 'ERRORS'
         | 'ESCAPE'
//         | 'EXCLUDE'
           | 'EXTRACT'
        | 'FIRST'
//         | 'FIRST_VALUE'
//         | 'FOLLOWING'
//             //| 'FULL'
         | 'GROUPING'
//         | 'IGNORE'
//         | 'INCLUDE'
         | 'INFINITE'
//         | 'INNER'
//         | 'INT'
         | 'INTERVAL'
//         | 'ITERATE'
//             //| 'JOIN'
//         | 'KEEP'
//         | 'LAG'
         | 'LAST'
//         | 'LAST_VALUE'
//         | 'LEAD'
//             //| 'LEFT'
         | 'LIKE2'
         | 'LIKE4'
         | 'LIKEC'
//         | 'LIMIT'
//         | 'LISTAGG'
//         | 'LOCAL'
//         | 'LOCKED'
//         | 'LOG'
//         | 'MAIN'
//         | 'MATCHED'
//         | 'MAX'
//         | 'MAXVALUE'
//         | 'MEASURES'
         | 'MEMBER'
//         | 'MERGE'
//         | 'MIN'
//         | 'MIVALUE'
//         | 'MLSLABEL'
//           //| 'MODEL'
//        | i_model	
//         | 'MONTH'
//             //| 'MULTISET'
         | 'NAN'
//         | 'NATIONAL'
//         | 'NATURAL'
//         | 'NAV'
//         | 'NCHAR'
//         | 'NCLOB'
//         | 'NOCYCLE'
//         | 'NTILE'
         | 'NULLS'
//         | 'NUMERIC'
//         | 'NVARCHAR'
//         | 'NVARCHAR2'
//         | 'OBJECT_ID'
//         | 'OBJECT_VALUE'
//         | 'ONLY'
//         | 'ORA_ROWSCN'
             | 'OUTER'
//         | 'OVER'
//             //| 'PARTITION'
//             //| 'PERCENT_RANK'
//             //| 'PERCENTILE_CONT'
//             //| 'PERCENTILE_DISC'
         | 'PIVOT'
//         | 'PLS_INTEGER'
//         | 'POSITIVE'
//         | 'PRECEDING'
//         | 'PRECISION'
//         | 'PRESENT'
//         | 'RANGE'
//         | 'RANK'
//         | 'RATIO_TO_REPORT'
//         | 'READ'
//         | 'REAL'
         | 'REFERENCE'
//         | 'REGEXP_LIKE'
//         | 'REGR_AVGX'
//         | 'REGR_AVGY'
//         | 'REGR_COUNT'
//         | 'REGR_INTERCEPT'
//         | 'REGR_R2'
//         | 'REGR_SLOPE'
//         | 'REGR_SXX'
//         | 'REGR_SXY'
//         | 'REGR_SYY'
//         | 'REJECT'
//         | 'RESPECT'
         | 'RETURN'
//         | 'RETURNING'
//             //| 'RIGHT'
         | 'ROLLUP'
//         | 'ROW_NUMBER'
//         | 'RULES'
//         | 'SAMPLE'
//         | 'SCN'
         | 'SEARCH'
//         | 'SECOND'
//         | 'SEED'
//         | 'SEQUENTIAL'
//         | 'SESSIONTIMEZONE'
//         | 'SETS'
//         | 'SIBLINGS'
//         | 'SINGLE'
//         | 'SKIP'
//         | 'SOME'
         | 'SQL'
//         | 'STDDEV'
//         | 'STDDEV_POP'
//         | 'STDDEV_SAMP'
//         | 'SUBMULTISET'
//         | 'SUBPARTITION'
//         | 'SUM'
//         | 'THE'
//         | 'TIME'
         | 'TIMESTAMP'
//         | 'TO'		            
//         | 'TYPE'
//         | 'UNBOUNDED'
//         | 'UNDER_PATH'
//         | 'UNLIMITED'
         | 'UNPIVOT'
//         | 'UNTIL'
//         | 'UPDATED'
//         | 'UPSERT'
//         | 'UROWID'
//             //| 'USING'
//         | 'VALUE'
//         | 'VAR_POP'
//         | 'VAR_SAMP'
//         | 'VARIANCE'
//         | 'VARYING'
//         | 'VERSIONS'
//             // | 'VERSIONS_ENDSCN'
//             // | 'VERSIONS_ENDTIME'
//             // | 'VERSIONS_OPERATION'
//             // | 'VERSIONS_STARSCN'
//             // | 'VERSIONS_STARTTIME'
//             // | 'VERSIONS_XID'
//         | 'WAIT'
		| 'WHEN'
//         | 'WITHIN'        
//         | 'XML'
//         | 'XMLDATA'
//         | 'YEAR'
//         | 'ZONE'
		) { $k->user1 = identifierClass; $k->user2 = usageType; }
	;

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
		@declarations {
    		ANTLR3_UINT32 (*oldLA)(struct ANTLR3_INT_STREAM_struct *, ANTLR3_INT32);
		}
		@init {
			oldLA = INPUT->istream->_LA;
            INPUT->setUcaseLA(INPUT, ANTLR3_FALSE);
		}
		:	
		QUOTE delimiter=QS_OTHER_CH
/* JAVA Syntax */        
// 		( { input.LT(1) != $delimiter.text.charAt(0) || ( input.LT(1) == $delimiter.text.charAt(0) && input.LT(2) != '\'') }? => . )*
// 		( { input.LT(1) == $delimiter.text.charAt(0) && input.LT(2) == '\'' }? => . ) QUOTE
/* C Syntax */ 
		( { LA(1) != $delimiter->getText(delimiter)->chars[0] || LA(2) != '\'' }? => . )*
		( { LA(1) == $delimiter->getText(delimiter)->chars[0] && LA(2) == '\'' }? => . ) QUOTE
 		{ INPUT->istream->_LA = oldLA; }
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
DOUBLEDOT
	:	POINT POINT
	;
DOT
	:	POINT
	;
fragment
POINT
	:	'.'
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
PERCENTAGE
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
NUMBER
	:
		(	( NUM POINT NUM ) => NUM POINT NUM
		|	POINT NUM
		|	NUM
		)
		( 'E' ( PLUS | MINUS )? NUM )?
		( 'D' | 'F')?		
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
	:	'--' ~('\n'|'\r')* '\r'? ('\n'|EOF)  {$channel=HIDDEN;}
	;
ML_COMMENT
	:	'/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
	;

TYPE_ATTR:                              '%TYPE' ;
ROWTYPE_ATTR:                           '%ROWTYPE' ;
NOTFOUND_ATTR:                          '%NOTFOUND' ;
FOUND_ATTR:                             '%FOUND';
ISOPEN_ATTR:                            '%ISOPEN' ;
ROWCOUNT_ATTR:                          '%ROWCOUNT' ;
BULK_ROWCOUNT_ATTR:                     '%BULK_ROWCOUNT';
CHARSET_ATTR:                           '%CHARSET';

ZV	:	'@!' {$channel=HIDDEN;}
	;
