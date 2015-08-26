/**
 * Oracle(c) SQL*Plus 11g Parser
 *
 * Copyright (c) 2009-2011 Alexandre Porcelli <alexandre.porcelli@gmail.com>
 *                         Tomi Pakarinen <tomi.pakarinen@iki.fi>
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
parser grammar SQLPLUSParser;

options {
    output=AST;
}

sql_plus_command 
    :    (SOLIDUS!|whenever_command|exit_command|prompt_command|set_command) SEMICOLON?
    ;

whenever_command
    :    whenever_key^ (sqlerror_key|oserror_key) (exit_key (success_key|failure_key|warning_key) (commit_key|rollback_key) | continue_key (commit_key|rollback_key|none_key))
    ;

set_command
    :    set_key^ REGULAR_ID (CHAR_STRING|on_key|off_key|EXACT_NUM_LIT|REGULAR_ID)
    ;

exit_command
    :    exit_key 
    ;

prompt_command
    :    PROMPT
    ;

