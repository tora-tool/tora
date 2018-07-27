/** \file
 *  This C++ header file was generated by $ANTLR version 3.5.3-SNAPSHOT
 *
 *     -  From the grammar source file : OracleDMLLexer.g
 *     -                            On : 2018-07-27 18:52:38
 *     -                 for the lexer : OracleDMLLexerLexer
 *
 * Editing it, at least manually, is not wise.
 *
 * C++ language generator and runtime by Gokulakannan Somasundaram ( heavy lifting from C Run-time by Jim Idle )
 *
 *
 * The lexer OracleDMLLexer has the callable functions (rules) shown below,
 * which will invoke the code for the associated rule in the source grammar
 * assuming that the input stream is pointing to a token/text stream that could begin
 * this rule.
 *
 * For instance if you call the first (topmost) rule in a parser grammar, you will
 * get the results of a full parse, but calling a rule half way through the grammar will
 * allow you to pass part of a full token stream to the parser, such as for syntax checking
 * in editors and so on.
 *
 */
// [The "BSD license"]
// Copyright (c) 2005-2009 Gokulakannan Somasundaram. 
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef	_OracleDMLLexer_H
#define _OracleDMLLexer_H
/* =============================================================================
 * Standard antlr3 C++ runtime definitions
 */
#include <antlr3.hpp>

/* End of standard antlr 3 runtime definitions
 * =============================================================================
 */


	#include "OracleDMLTraits.hpp"




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


namespace  Antlr3BackendImpl  {

typedef OracleDMLLexerTraits OracleDMLLexerImplTraits;


class OracleDMLLexerTokens
{
public:
	/** Symbolic definitions of all the tokens that the lexer will work with.
	 *
	 * Antlr will define EOF, but we can't use that as it it is too common in
	 * in C header files and that would be confusing. There is no way to filter this out at the moment
	 * so we just undef it here for now. That isn't the value we get back from C recognizers
	 * anyway. We are looking for ANTLR_TOKEN_EOF.
	 */
	enum Tokens : ANTLR_UINT32
	{
		EOF_TOKEN = OracleDMLLexerImplTraits::CommonTokenType::TOKEN_EOF
		, ADD_VK = 4 
		, AFTER_VK = 5 
		, AGENT_VK = 6 
		, AGGREGATE_VK = 7 
		, ALIAS = 8 
		, AMPERSAND = 9 
		, ANALYZE_VK = 10 
		, ANY_ELEMENT = 11 
		, ANY_MODE = 12 
		, APPROXIMATE_NUM_LIT = 13 
		, ARGUMENT = 14 
		, ARGUMENTS = 15 
		, ASSIGN = 16 
		, ASSIGN_OP = 17 
		, ASSOCIATE_VK = 18 
		, ASTERISK = 19 
		, ATTRIBUTE_NAME = 20 
		, AT_SIGN = 21 
		, AT_VK = 22 
		, AUDIT_VK = 23 
		, AUTHID_VK = 24 
		, AUTOMATIC_VK = 25 
		, AUTONOMOUS_TRANSACTION_VK = 26 
		, AUTO_VK = 27 
		, BATCH_VK = 28 
		, BEFORE_VK = 29 
		, BFILE_VK = 30 
		, BINARY_DOUBLE_VK = 31 
		, BINARY_FLOAT_VK = 32 
		, BINARY_INTEGER_VK = 33 
		, BINDVAR = 34 
		, BIT_STRING_LIT = 35 
		, BIT_VK = 36 
		, BLOB_VK = 37 
		, BLOCK_VK = 38 
		, BODY_VK = 39 
		, BOOLEAN_VK = 40 
		, BOTH_VK = 41 
		, BREADTH_VK = 42 
		, BULK_VK = 43 
		, BYTE_VK = 44 
		, CACHE_VK = 45 
		, CALL_VK = 46 
		, CANONICAL_VK = 47 
		, CARRET_OPERATOR_PART = 48 
		, CASCADE_VK = 49 
		, CASCATED_ELEMENT = 50 
		, CAST_VK = 51 
		, CHARACTER_VK = 52 
		, CHAR_CS_VK = 53 
		, CHAR_SET_NAME = 54 
		, CHAR_STRING = 55 
		, CHAR_STRING_PERL = 56 
		, CHAR_VK = 57 
		, CHR_VK = 58 
		, CLOB_VK = 59 
		, CLOSE_VK = 60 
		, CLUSTER_VK = 61 
		, COLLECTION_MODE = 62 
		, COLLECTION_NAME = 63 
		, COLLECT_VK = 64 
		, COLON = 65 
		, COLUMNS = 66 
		, COLUMN_NAME = 67 
		, COMMA = 68 
		, COMMENT = 69 
		, COMMENT_VK = 70 
		, COMMITTED_VK = 71 
		, COMMIT_VK = 72 
		, COMPATIBILITY_VK = 73 
		, COMPILE_VK = 74 
		, COMPOUND_VK = 75 
		, CONCATENATION_OP = 76 
		, CONDITIONAL_INSERT = 77 
		, CONSTANT_VK = 78 
		, CONSTRAINT_NAME = 79 
		, CONSTRUCTOR_VK = 80 
		, CONTENT_VK = 81 
		, CONTEXT_VK = 82 
		, CONTINUE_VK = 83 
		, CONVERT_VK = 84 
		, CORRUPT_XID_ALL_VK = 85 
		, CORRUPT_XID_VK = 86 
		, COST_CLASS_NAME = 87 
		, COST_VK = 88 
		, COUNT_VK = 89 
		, CROSS_VK = 90 
		, CUBE_VK = 91 
		, CURRENT_USER_VK = 92 
		, CURRENT_VK = 93 
		, CURSOR_NAME = 94 
		, CUSTOMDATUM_VK = 95 
		, CUSTOM_TYPE = 96 
		, CYCLE_VK = 97 
		, C_VK = 98 
		, DATABASE_VK = 99 
		, DATETIME_OP = 100 
		, DAY_VK = 101 
		, DBTIMEZONE_VK = 102 
		, DB_ROLE_CHANGE_VK = 103 
		, DDL_VK = 104 
		, DEBUG_VK = 105 
		, DECIMAL_VK = 106 
		, DECOMPOSE_VK = 107 
		, DECREMENT_VK = 108 
		, DEC_VK = 109 
		, DEFAULTS_VK = 110 
		, DEFERRED_VK = 111 
		, DEFINER_VK = 112 
		, DELIMITED_ID = 113 
		, DENSE_RANK_VK = 114 
		, DEPTH_VK = 115 
		, DETERMINISTIC_VK = 116 
		, DIMENSION_VK = 117 
		, DIRECT_MODE = 118 
		, DISABLE_VK = 119 
		, DISASSOCIATE_VK = 120 
		, DOCUMENT_VK = 121 
		, DOT_ASTERISK = 122 
		, DOUBLE_ASTERISK = 123 
		, DOUBLE_PERIOD = 124 
		, DOUBLE_VK = 125 
		, DSINTERVAL_UNCONSTRAINED_VK = 126 
		, ELEMENT = 127 
		, ELEMENTS_MODE = 128 
		, ENABLE_VK = 129 
		, ENCODING_VK = 130 
		, ENTITYESCAPING_VK = 131 
		, EQUALS_OP = 132 
		, EVALNAME_VK = 133 
		, EXACT_NUM_LIT = 134 
		, EXCEPTIONS_VK = 135 
		, EXCEPTION_INIT_VK = 136 
		, EXCEPTION_NAME = 137 
		, EXCLAMATION_OPERATOR_PART = 138 
		, EXCLUDE_VK = 139 
		, EXECUTE_VK = 140 
		, EXIT_VK = 141 
		, EXPLAIN_STATEMENT = 142 
		, EXPR = 143 
		, EXPR_LIST = 144 
		, EXTERNAL_VK = 145 
		, EXTRACT_VK = 146 
		, FACTORING = 147 
		, FINAL_VK = 148 
		, FIRST_VALUE_VK = 149 
		, FIRST_VK = 150 
		, FLOAT_VK = 151 
		, FOLLOWING_VK = 152 
		, FOLLOWS_VK = 153 
		, FORALL_VK = 154 
		, FORCE_VK = 155 
		, FOR_MULTI_COLUMN = 156 
		, FOR_NOTATION = 157 
		, FOR_SINGLE_COLUMN = 158 
		, FULL_VK = 159 
		, FUNCTION_ENABLING_OVER = 160 
		, FUNCTION_ENABLING_USING = 161 
		, FUNCTION_ENABLING_WITHIN_OR_OVER = 162 
		, FUNCTION_NAME = 163 
		, GREATER_THAN_OP = 164 
		, GREATER_THAN_OR_EQUALS_OP = 165 
		, GROUPING_VK = 166 
		, GROUPIN_SET = 167 
		, GROUP_BY_CLAUSE = 168 
		, GROUP_BY_ELEMENT = 169 
		, HASH_VK = 170 
		, HEX_STRING_LIT = 171 
		, HIDE_VK = 172 
		, HIERARCHICAL = 173 
		, HOSTED_VARIABLE = 174 
		, HOSTED_VARIABLE_NAME = 175 
		, HOSTED_VARIABLE_ROUTINE_CALL = 176 
		, HOUR_VK = 177 
		, ID = 178 
		, IGNORE_VK = 179 
		, IMMEDIATE_VK = 180 
		, IMPLEMENTATION_TYPE_NAME = 181 
		, INCLUDE_VK = 182 
		, INCLUDING_VK = 183 
		, INCREMENT_VK = 184 
		, INDENT_VK = 185 
		, INDEX_NAME = 186 
		, INLINE_VK = 187 
		, INNER_VK = 188 
		, INOUT_VK = 189 
		, INSTANTIABLE_VK = 190 
		, INSTEAD_VK = 191 
		, INTEGER_VK = 192 
		, INTERVAL_DATATYPE = 193 
		, INTRODUCER = 194 
		, INT_VK = 195 
		, INVALIDATE_VK = 196 
		, ISOLATION_VK = 197 
		, IS_A_SET = 198 
		, IS_EMPTY = 199 
		, IS_INFINITE = 200 
		, IS_NAN = 201 
		, IS_NOT_A_SET = 202 
		, IS_NOT_EMPTY = 203 
		, IS_NOT_INFINITE = 204 
		, IS_NOT_NAN = 205 
		, IS_NOT_NULL = 206 
		, IS_NOT_OF_TYPE = 207 
		, IS_NOT_PRESENT = 208 
		, IS_NULL = 209 
		, IS_OF_TYPE = 210 
		, IS_PRESENT = 211 
		, ITERATE_VK = 212 
		, JAVA_VK = 213 
		, JOIN_DEF = 214 
		, KEEP_VK = 215 
		, LABEL_NAME = 216 
		, LANGUAGE_VK = 217 
		, LAST_VALUE_VK = 218 
		, LAST_VK = 219 
		, LEADING_VK = 220 
		, LEFT_BRACKET = 221 
		, LEFT_PAREN = 222 
		, LEFT_VK = 223 
		, LESS_THAN_OP = 224 
		, LESS_THAN_OR_EQUALS_OP = 225 
		, LIBRARY_VK = 226 
		, LIKE2_VK = 227 
		, LIKE4_VK = 228 
		, LIKEC_VK = 229 
		, LIMIT_VK = 230 
		, LINK_NAME = 231 
		, LOCAL_VK = 232 
		, LOCK_TABLE_ELEMENT = 233 
		, LOGIC_EXPR = 234 
		, LOGOFF_VK = 235 
		, LOGON_VK = 236 
		, LOG_VK = 237 
		, LONG_VK = 238 
		, LOOP_VK = 239 
		, MAIN_MODEL = 240 
		, MAIN_MODEL_NAME = 241 
		, MAP_VK = 242 
		, MAXVALUE_VK = 243 
		, MEASURES_VK = 244 
		, MEMBER_VK = 245 
		, MERGE_INSERT = 246 
		, MERGE_UPDATE = 247 
		, MERGE_VK = 248 
		, MINUS_SIGN = 249 
		, MINUTE_VK = 250 
		, MINVALUE_VK = 251 
		, MLSLABEL_VK = 252 
		, MODEL_COLUMN = 253 
		, MODEL_COLUMNS = 254 
		, MODEL_EXPRESSION = 255 
		, MODEL_RULE = 256 
		, MODEL_RULES = 257 
		, MODIFY_VK = 258 
		, MONTH_VK = 259 
		, MULTI_TABLE_MODE = 260 
		, NAME_VK = 261 
		, NATIONAL_CHAR_STRING_LIT = 262 
		, NATIVE_DATATYPE = 263 
		, NATURALN_VK = 264 
		, NATURAL_VK = 265 
		, NAV_VK = 266 
		, NCHAR_CS_VK = 267 
		, NCHAR_VK = 268 
		, NCLOB_VK = 269 
		, NESTED_EXPR = 270 
		, NESTED_SUBQUERY = 271 
		, NESTED_VK = 272 
		, NEWLINE = 273 
		, NEW_VK = 274 
		, NOAUDIT_VK = 275 
		, NOCACHE_VK = 276 
		, NOCOPY_VK = 277 
		, NOCYCLE_VK = 278 
		, NOENTITYESCAPING_VK = 279 
		, NOMAXVALUE_VK = 280 
		, NOMINVALUE_VK = 281 
		, NOORDER_VK = 282 
		, NOSCHEMACHECK_VK = 283 
		, NOT_BETWEEN = 284 
		, NOT_EQUAL_OP = 285 
		, NOT_IN = 286 
		, NOT_LIKE = 287 
		, NO_VK = 288 
		, NULLS_VK = 289 
		, NUMBER_VK = 290 
		, NUMERIC_VK = 291 
		, NVARCHAR2_VK = 292 
		, OBJECT_VK = 293 
		, OFF_VK = 294 
		, OLD_VK = 295 
		, ONLY_VK = 296 
		, OPEN_VK = 297 
		, ORADATA_VK = 298 
		, ORDER_BY_ELEMENT = 299 
		, ORDER_BY_ELEMENTS = 300 
		, ORDINALITY_VK = 301 
		, OUT_VK = 302 
		, OVERRIDING_VK = 303 
		, OVER_VK = 304 
		, PACKAGE_NAME = 305 
		, PACKAGE_VK = 306 
		, PARALLEL_ENABLE_VK = 307 
		, PARAMETERS_VK = 308 
		, PARAMETER_NAME = 309 
		, PARENT_VK = 310 
		, PARTITION_VK = 311 
		, PASSING_VK = 312 
		, PERCENT = 313 
		, PERCENT_FOUND_VK = 314 
		, PERCENT_ISOPEN_VK = 315 
		, PERCENT_NOTFOUND_VK = 316 
		, PERCENT_ROWCOUNT_VK = 317 
		, PERCENT_ROWTYPE_VK = 318 
		, PERCENT_TYPE_VK = 319 
		, PERIOD = 320 
		, PIPELINED_VK = 321 
		, PIVOT_ALIAS = 322 
		, PIVOT_ELEMENT = 323 
		, PIVOT_IN_ELEMENT = 324 
		, PIVOT_VK = 325 
		, PLSQL_NON_RESERVED_CAST = 326 
		, PLSQL_NON_RESERVED_CONNECT_BY_ROOT = 327 
		, PLSQL_NON_RESERVED_ELSIF = 328 
		, PLSQL_NON_RESERVED_MODEL = 329 
		, PLSQL_NON_RESERVED_MULTISET = 330 
		, PLSQL_NON_RESERVED_PIVOT = 331 
		, PLSQL_NON_RESERVED_UNPIVOT = 332 
		, PLSQL_NON_RESERVED_USING = 333 
		, PLSQL_RESERVED_CLUSTERS = 334 
		, PLSQL_RESERVED_COLAUTH = 335 
		, PLSQL_RESERVED_COMPRESS = 336 
		, PLSQL_RESERVED_CRASH = 337 
		, PLSQL_RESERVED_EXCLUSIVE = 338 
		, PLSQL_RESERVED_IDENTIFIED = 339 
		, PLSQL_RESERVED_IF = 340 
		, PLSQL_RESERVED_INDEX = 341 
		, PLSQL_RESERVED_INDEXES = 342 
		, PLSQL_RESERVED_LOCK = 343 
		, PLSQL_RESERVED_MINUS = 344 
		, PLSQL_RESERVED_MODE = 345 
		, PLSQL_RESERVED_NOCOMPRESS = 346 
		, PLSQL_RESERVED_NOWAIT = 347 
		, PLSQL_RESERVED_RESOURCE = 348 
		, PLSQL_RESERVED_SHARE = 349 
		, PLSQL_RESERVED_START = 350 
		, PLSQL_RESERVED_TABAUTH = 351 
		, PLSQL_RESERVED_VIEWS = 352 
		, PLS_INTEGER_VK = 353 
		, PLUS_SIGN = 354 
		, POSITIVEN_VK = 355 
		, POSITIVE_VK = 356 
		, PRAGMA_VK = 357 
		, PRECEDING_VK = 358 
		, PRECISION = 359 
		, PRECISION_VK = 360 
		, PREDICTION_FUNCTION = 361 
		, PROCEDURE_NAME = 362 
		, PROMPT = 363 
		, QS_ANGLE = 364 
		, QS_BRACE = 365 
		, QS_BRACK = 366 
		, QS_OTHER = 367 
		, QS_OTHER_CH = 368 
		, QS_PAREN = 369 
		, QUERY_NAME = 370 
		, QUESTION_MARK = 371 
		, QUOTE = 372 
		, RAISE_VK = 373 
		, RANGE_VK = 374 
		, RAW_VK = 375 
		, READ_VK = 376 
		, REAL_VK = 377 
		, RECORD_NAME = 378 
		, REFERENCE_MODEL_NAME = 379 
		, REFERENCE_VK = 380 
		, REFERENCING_VK = 381 
		, REF_VK = 382 
		, REGULAR_ID = 383 
		, REJECT_VK = 384 
		, RELIES_ON_VK = 385 
		, RENAME_VK = 386 
		, REPLACE_VK = 387 
		, RESPECT_VK = 388 
		, RESTRICT_REFERENCES_VK = 389 
		, RESULT_CACHE_VK = 390 
		, RETURN_VK = 391 
		, REUSE_VK = 392 
		, REVERSE_VK = 393 
		, RIGHT_BRACKET = 394 
		, RIGHT_PAREN = 395 
		, RIGHT_VK = 396 
		, ROLLBACK_SEGMENT_NAME = 397 
		, ROLLBACK_VK = 398 
		, ROLLUP_VK = 399 
		, ROUTINE_CALL = 400 
		, ROUTINE_NAME = 401 
		, ROWID_VK = 402 
		, ROWS_VK = 403 
		, ROW_VK = 404 
		, RULES_VK = 405 
		, SAMPLE_VK = 406 
		, SAVEPOINT_NAME = 407 
		, SAVEPOINT_VK = 408 
		, SCHEMACHECK_VK = 409 
		, SCHEMA_NAME = 410 
		, SCHEMA_VK = 411 
		, SCN_VK = 412 
		, SEARCHED_CASE = 413 
		, SEARCH_VK = 414 
		, SECOND_VK = 415 
		, SEED_VK = 416 
		, SELECTED_TABLEVIEW = 417 
		, SELECT_ITEM = 418 
		, SELECT_LIST = 419 
		, SELECT_MODE = 420 
		, SELECT_STATEMENT = 421 
		, SELF_VK = 422 
		, SEMICOLON = 423 
		, SEPARATOR = 424 
		, SEQUENCE_NAME = 425 
		, SEQUENCE_VK = 426 
		, SEQUENTIAL_VK = 427 
		, SERIALIZABLE_VK = 428 
		, SERIALLY_REUSABLE_VK = 429 
		, SERVERERROR_VK = 430 
		, SESSIONTIMEZONE_VK = 431 
		, SET_VK = 432 
		, SHOW_VK = 433 
		, SHUTDOWN_VK = 434 
		, SIBLINGS_VK = 435 
		, SIGNTYPE_VK = 436 
		, SIMPLE_CASE = 437 
		, SIMPLE_INTEGER_VK = 438 
		, SIMPLE_LETTER = 439 
		, SINGLE_TABLE_MODE = 440 
		, SINGLE_VK = 441 
		, SKIP_VK = 442 
		, SMALLINT_VK = 443 
		, SOLIDUS = 444 
		, SOME_VK = 445 
		, SPACE = 446 
		, SPECIFICATION_VK = 447 
		, SQL92_RESERVED_ALL = 448 
		, SQL92_RESERVED_ALTER = 449 
		, SQL92_RESERVED_AND = 450 
		, SQL92_RESERVED_ANY = 451 
		, SQL92_RESERVED_AS = 452 
		, SQL92_RESERVED_ASC = 453 
		, SQL92_RESERVED_BEGIN = 454 
		, SQL92_RESERVED_BETWEEN = 455 
		, SQL92_RESERVED_BY = 456 
		, SQL92_RESERVED_CASE = 457 
		, SQL92_RESERVED_CHECK = 458 
		, SQL92_RESERVED_CONNECT = 459 
		, SQL92_RESERVED_CREATE = 460 
		, SQL92_RESERVED_CURRENT = 461 
		, SQL92_RESERVED_CURSOR = 462 
		, SQL92_RESERVED_DATE = 463 
		, SQL92_RESERVED_DECLARE = 464 
		, SQL92_RESERVED_DEFAULT = 465 
		, SQL92_RESERVED_DELETE = 466 
		, SQL92_RESERVED_DESC = 467 
		, SQL92_RESERVED_DISTINCT = 468 
		, SQL92_RESERVED_DROP = 469 
		, SQL92_RESERVED_ELSE = 470 
		, SQL92_RESERVED_END = 471 
		, SQL92_RESERVED_EXCEPTION = 472 
		, SQL92_RESERVED_EXISTS = 473 
		, SQL92_RESERVED_FALSE = 474 
		, SQL92_RESERVED_FETCH = 475 
		, SQL92_RESERVED_FOR = 476 
		, SQL92_RESERVED_FROM = 477 
		, SQL92_RESERVED_GOTO = 478 
		, SQL92_RESERVED_GRANT = 479 
		, SQL92_RESERVED_GROUP = 480 
		, SQL92_RESERVED_HAVING = 481 
		, SQL92_RESERVED_IN = 482 
		, SQL92_RESERVED_INSERT = 483 
		, SQL92_RESERVED_INTERSECT = 484 
		, SQL92_RESERVED_INTO = 485 
		, SQL92_RESERVED_IS = 486 
		, SQL92_RESERVED_LIKE = 487 
		, SQL92_RESERVED_NOT = 488 
		, SQL92_RESERVED_NULL = 489 
		, SQL92_RESERVED_OF = 490 
		, SQL92_RESERVED_ON = 491 
		, SQL92_RESERVED_OPTION = 492 
		, SQL92_RESERVED_OR = 493 
		, SQL92_RESERVED_ORDER = 494 
		, SQL92_RESERVED_OVERLAPS = 495 
		, SQL92_RESERVED_PRIOR = 496 
		, SQL92_RESERVED_PROCEDURE = 497 
		, SQL92_RESERVED_PUBLIC = 498 
		, SQL92_RESERVED_REVOKE = 499 
		, SQL92_RESERVED_SELECT = 500 
		, SQL92_RESERVED_SIZE = 501 
		, SQL92_RESERVED_TABLE = 502 
		, SQL92_RESERVED_THE = 503 
		, SQL92_RESERVED_THEN = 504 
		, SQL92_RESERVED_TO = 505 
		, SQL92_RESERVED_TRUE = 506 
		, SQL92_RESERVED_UNION = 507 
		, SQL92_RESERVED_UNIQUE = 508 
		, SQL92_RESERVED_UPDATE = 509 
		, SQL92_RESERVED_VALUES = 510 
		, SQL92_RESERVED_VIEW = 511 
		, SQL92_RESERVED_WHEN = 512 
		, SQL92_RESERVED_WHERE = 513 
		, SQL92_RESERVED_WITH = 514 
		, SQLDATA_VK = 515 
		, STANDALONE_VK = 516 
		, STANDARD_FUNCTION = 517 
		, STARTUP_VK = 518 
		, STATIC_RETURNING = 519 
		, STATIC_VK = 520 
		, STATISTICS_VK = 521 
		, STRING_VK = 522 
		, SUBMULTISET_VK = 523 
		, SUBPARTITION_VK = 524 
		, SUBQUERY = 525 
		, SUSPEND_VK = 526 
		, TABLEVIEW_NAME = 527 
		, TABLE_ELEMENT = 528 
		, TABLE_EXPRESSION = 529 
		, TABLE_REF = 530 
		, TABLE_REF_ELEMENT = 531 
		, TABLE_VAR_NAME = 532 
		, TILDE_OPERATOR_PART = 533 
		, TIMESTAMP_LTZ_UNCONSTRAINED_VK = 534 
		, TIMESTAMP_TZ_UNCONSTRAINED_VK = 535 
		, TIMESTAMP_UNCONSTRAINED_VK = 536 
		, TIMESTAMP_VK = 537 
		, TIMEZONE_ABBR_VK = 538 
		, TIMEZONE_HOUR_VK = 539 
		, TIMEZONE_MINUTE_VK = 540 
		, TIMEZONE_REGION_VK = 541 
		, TIME_VK = 542 
		, TRAILING_VK = 543 
		, TRANSLATE_VK = 544 
		, TREAT_VK = 545 
		, TRIGGER_NAME = 546 
		, TRIM_VK = 547 
		, TRUNCATE_VK = 548 
		, TYPE_NAME = 549 
		, TYPE_VK = 550 
		, T_BINDVAR_NAME = 551 
		, T_COLUMN_ALIAS = 552 
		, T_COLUMN_LIST = 553 
		, T_COLUMN_NAME = 554 
		, T_COND_AND = 555 
		, T_COND_AND_SEQ = 556 
		, T_COND_BETWEEN = 557 
		, T_COND_COMPARISON = 558 
		, T_COND_EQUALS_PATH = 559 
		, T_COND_EXISTS = 560 
		, T_COND_GROUP_COMPARISON = 561 
		, T_COND_IN = 562 
		, T_COND_IS = 563 
		, T_COND_IS_ANY = 564 
		, T_COND_IS_A_SET = 565 
		, T_COND_IS_EMPTY = 566 
		, T_COND_IS_OF_TYPE = 567 
		, T_COND_IS_PRESENT = 568 
		, T_COND_LIKE = 569 
		, T_COND_MEMEBER = 570 
		, T_COND_NOT = 571 
		, T_COND_OR = 572 
		, T_COND_OR_SEQ = 573 
		, T_COND_PAREN = 574 
		, T_COND_REGEXP_LIKE = 575 
		, T_COND_SUBMULTISET = 576 
		, T_COND_UNDER_PATH = 577 
		, T_DBLINK_NAME = 578 
		, T_DECL = 579 
		, T_FOR_UPDATE_CLAUSE = 580 
		, T_FROM = 581 
		, T_FUNCTION_NAME = 582 
		, T_GROUP_BY = 583 
		, T_HIERARCHICAL = 584 
		, T_IDENTIFIER = 585 
		, T_JOINING_CLAUSE = 586 
		, T_MODEL = 587 
		, T_OPERATOR_BINARY = 588 
		, T_OPERATOR_PART = 589 
		, T_OPERATOR_UNARY = 590 
		, T_ORDER_BY_CLAUSE = 591 
		, T_PACKAGE_NAME = 592 
		, T_RESERVED = 593 
		, T_SCHEMA_NAME = 594 
		, T_SELECT = 595 
		, T_SELECT_COLUMN = 596 
		, T_SUBQUERY = 597 
		, T_TABLE_ALIAS = 598 
		, T_TABLE_CAST = 599 
		, T_TABLE_NAME = 600 
		, T_TABLE_REF = 601 
		, T_UNION = 602 
		, T_UNKNOWN = 603 
		, T_USE = 604 
		, T_WHERE = 605 
		, T_WITH = 606 
		, UNARY_OPERATOR = 607 
		, UNBOUNDED_VK = 608 
		, UNDERSCORE = 609 
		, UNDER_VK = 610 
		, UNLIMITED_VK = 611 
		, UNPIVOT_IN_ELEMENT = 612 
		, UNPIVOT_VK = 613 
		, UNSIGNED_INTEGER = 614 
		, UNTIL_VK = 615 
		, UPDATED_VK = 616 
		, UPSERT_VK = 617 
		, UROWID_VK = 618 
		, VALIDATE_VK = 619 
		, VALUE_VK = 620 
		, VARCHAR2_VK = 621 
		, VARCHAR_VK = 622 
		, VARIABLE_NAME = 623 
		, VECTOR_EXPR = 624 
		, VERSIONS_VK = 625 
		, VERSION_VK = 626 
		, VERTICAL_BAR = 627 
		, WAIT_VK = 628 
		, WELLFORMED_VK = 629 
		, WITHIN_VK = 630 
		, WORK_VK = 631 
		, WRITE_VK = 632 
		, XMLAGG_VK = 633 
		, XMLATTRIBUTES_VK = 634 
		, XMLCAST_VK = 635 
		, XMLCOLATTVAL_VK = 636 
		, XMLELEMENT_VK = 637 
		, XMLEXISTS_VK = 638 
		, XMLFOREST_VK = 639 
		, XMLNAMESPACES_VK = 640 
		, XMLPARSE_VK = 641 
		, XMLPI_VK = 642 
		, XMLQUERY_VK = 643 
		, XMLROOT_VK = 644 
		, XMLSERIALIZE_VK = 645 
		, XMLTABLE_VK = 646 
		, XML_ALIAS = 647 
		, XML_COLUMN = 648 
		, XML_COLUMN_NAME = 649 
		, XML_ELEMENT = 650 
		, XML_VK = 651 
		, YEAR_VK = 652 
		, YES_VK = 653 
		, YMINTERVAL_UNCONSTRAINED_VK = 654 
		, ZV = 655 
	};
       static const ANTLR_UINT8* getTokenName(ANTLR_UINT32 index);

protected:
	static ANTLR_UINT8* TokenNames[];
	template<class T, size_t N>
	inline static size_t arraysize(const T(&)[N]) { return N; }

};

/** Context tracking structure for OracleDMLLexer
 */
class OracleDMLLexer : public OracleDMLLexerImplTraits::BaseLexerType
    , public OracleDMLLexerTokens
{
public:
	typedef OracleDMLLexerImplTraits ImplTraits;
	typedef OracleDMLLexer ComponentType;
	typedef ComponentType::StreamType StreamType;
	typedef OracleDMLLexerImplTraits::BaseLexerType BaseType;
	typedef ImplTraits::RecognizerSharedStateType<StreamType> RecognizerSharedStateType;
	typedef StreamType InputType;
	static const bool IsFiltered = false;


private:	
public:
    OracleDMLLexer(InputType* instream);
    OracleDMLLexer(InputType* instream, RecognizerSharedStateType* state);

    void init(InputType* instream  );
    static ANTLR_UINT8** getTokenNames();


    
    void  mFOR_NOTATION( );
    void  mNATIONAL_CHAR_STRING_LIT( );
    void  mBIT_STRING_LIT( );
    void  mHEX_STRING_LIT( );
    void  mPERIOD( );
    void  mEXACT_NUM_LIT( );
    void  mCHAR_STRING( );
    void  mCHAR_STRING_PERL( );
    void  mQUOTE( );
    void  mQS_ANGLE( );
    void  mQS_BRACE( );
    void  mQS_BRACK( );
    void  mQS_PAREN( );
    void  mQS_OTHER_CH( );
    void  mQS_OTHER( );
    void  mDELIMITED_ID( );
    void  mPERCENT( );
    void  mAMPERSAND( );
    void  mLEFT_PAREN( );
    void  mRIGHT_PAREN( );
    void  mDOUBLE_ASTERISK( );
    void  mASTERISK( );
    void  mPLUS_SIGN( );
    void  mCOMMA( );
    void  mSOLIDUS( );
    void  mAT_SIGN( );
    void  mASSIGN_OP( );
    void  mBINDVAR( );
    void  mCOLON( );
    void  mSEMICOLON( );
    void  mLESS_THAN_OR_EQUALS_OP( );
    void  mLESS_THAN_OP( );
    void  mGREATER_THAN_OR_EQUALS_OP( );
    void  mNOT_EQUAL_OP( );
    void  mCARRET_OPERATOR_PART( );
    void  mTILDE_OPERATOR_PART( );
    void  mEXCLAMATION_OPERATOR_PART( );
    void  mGREATER_THAN_OP( );
    void  mQUESTION_MARK( );
    void  mCONCATENATION_OP( );
    void  mVERTICAL_BAR( );
    void  mEQUALS_OP( );
    void  mLEFT_BRACKET( );
    void  mRIGHT_BRACKET( );
    void  mINTRODUCER( );
    void  mSEPARATOR( );
    void  mSIMPLE_LETTER( );
    void  mUNSIGNED_INTEGER( );
    void  mCOMMENT( );
    void  mPROMPT( );
    void  mNEWLINE( );
    void  mSPACE( );
    void  mAPPROXIMATE_NUM_LIT( );
    void  mMINUS_SIGN( );
    void  mUNDERSCORE( );
    void  mDOUBLE_PERIOD( );
    void  mSQL92_RESERVED_ALL( );
    void  mSQL92_RESERVED_ALTER( );
    void  mSQL92_RESERVED_AND( );
    void  mSQL92_RESERVED_ANY( );
    void  mSQL92_RESERVED_AS( );
    void  mSQL92_RESERVED_ASC( );
    void  mSQL92_RESERVED_BEGIN( );
    void  mSQL92_RESERVED_BETWEEN( );
    void  mSQL92_RESERVED_BY( );
    void  mSQL92_RESERVED_CASE( );
    void  mSQL92_RESERVED_CHECK( );
    void  mPLSQL_RESERVED_CLUSTERS( );
    void  mPLSQL_RESERVED_COLAUTH( );
    void  mPLSQL_RESERVED_COMPRESS( );
    void  mSQL92_RESERVED_CONNECT( );
    void  mPLSQL_NON_RESERVED_CONNECT_BY_ROOT( );
    void  mPLSQL_RESERVED_CRASH( );
    void  mSQL92_RESERVED_CREATE( );
    void  mSQL92_RESERVED_CURRENT( );
    void  mSQL92_RESERVED_CURSOR( );
    void  mSQL92_RESERVED_DATE( );
    void  mSQL92_RESERVED_DECLARE( );
    void  mSQL92_RESERVED_DEFAULT( );
    void  mSQL92_RESERVED_DELETE( );
    void  mSQL92_RESERVED_DESC( );
    void  mSQL92_RESERVED_DISTINCT( );
    void  mSQL92_RESERVED_DROP( );
    void  mSQL92_RESERVED_ELSE( );
    void  mSQL92_RESERVED_END( );
    void  mSQL92_RESERVED_EXCEPTION( );
    void  mPLSQL_RESERVED_EXCLUSIVE( );
    void  mSQL92_RESERVED_EXISTS( );
    void  mSQL92_RESERVED_FALSE( );
    void  mSQL92_RESERVED_FETCH( );
    void  mSQL92_RESERVED_FOR( );
    void  mSQL92_RESERVED_FROM( );
    void  mSQL92_RESERVED_GOTO( );
    void  mSQL92_RESERVED_GRANT( );
    void  mSQL92_RESERVED_GROUP( );
    void  mSQL92_RESERVED_HAVING( );
    void  mPLSQL_RESERVED_IDENTIFIED( );
    void  mPLSQL_RESERVED_IF( );
    void  mSQL92_RESERVED_IN( );
    void  mPLSQL_RESERVED_INDEX( );
    void  mPLSQL_RESERVED_INDEXES( );
    void  mSQL92_RESERVED_INSERT( );
    void  mSQL92_RESERVED_INTERSECT( );
    void  mSQL92_RESERVED_INTO( );
    void  mSQL92_RESERVED_IS( );
    void  mSQL92_RESERVED_LIKE( );
    void  mPLSQL_RESERVED_LOCK( );
    void  mPLSQL_RESERVED_MINUS( );
    void  mPLSQL_RESERVED_MODE( );
    void  mPLSQL_RESERVED_NOCOMPRESS( );
    void  mSQL92_RESERVED_NOT( );
    void  mPLSQL_RESERVED_NOWAIT( );
    void  mSQL92_RESERVED_NULL( );
    void  mSQL92_RESERVED_OF( );
    void  mSQL92_RESERVED_ON( );
    void  mSQL92_RESERVED_OPTION( );
    void  mSQL92_RESERVED_OR( );
    void  mSQL92_RESERVED_ORDER( );
    void  mSQL92_RESERVED_OVERLAPS( );
    void  mSQL92_RESERVED_PRIOR( );
    void  mSQL92_RESERVED_PROCEDURE( );
    void  mSQL92_RESERVED_PUBLIC( );
    void  mPLSQL_RESERVED_RESOURCE( );
    void  mSQL92_RESERVED_REVOKE( );
    void  mSQL92_RESERVED_SELECT( );
    void  mPLSQL_RESERVED_SHARE( );
    void  mSQL92_RESERVED_SIZE( );
    void  mPLSQL_RESERVED_START( );
    void  mPLSQL_RESERVED_TABAUTH( );
    void  mSQL92_RESERVED_TABLE( );
    void  mSQL92_RESERVED_THE( );
    void  mSQL92_RESERVED_THEN( );
    void  mSQL92_RESERVED_TO( );
    void  mSQL92_RESERVED_TRUE( );
    void  mSQL92_RESERVED_UNION( );
    void  mSQL92_RESERVED_UNIQUE( );
    void  mSQL92_RESERVED_UPDATE( );
    void  mSQL92_RESERVED_VALUES( );
    void  mSQL92_RESERVED_VIEW( );
    void  mPLSQL_RESERVED_VIEWS( );
    void  mSQL92_RESERVED_WHEN( );
    void  mSQL92_RESERVED_WHERE( );
    void  mSQL92_RESERVED_WITH( );
    void  mPLSQL_NON_RESERVED_CAST( );
    void  mPLSQL_NON_RESERVED_MULTISET( );
    void  mPLSQL_NON_RESERVED_USING( );
    void  mPLSQL_NON_RESERVED_MODEL( );
    void  mPLSQL_NON_RESERVED_ELSIF( );
    void  mPLSQL_NON_RESERVED_PIVOT( );
    void  mPLSQL_NON_RESERVED_UNPIVOT( );
    void  mREGULAR_ID( );
    void  mZV( );
    void  mTokens( );
    const char *    getGrammarFileName();
    void            reset();
    ~OracleDMLLexer();

};

// Function protoypes for the constructor functions that external translation units
// such as delegators and delegates may wish to call.
//

/* End of token definitions for OracleDMLLexer
 * =============================================================================
 */

}

#endif

/* END - Note:Keep extra line feed to satisfy UNIX systems */
