//Define a grammar to parse statements from postgresql files
lexer grammar PostgreSQLGuiLexer;

KEYWORD :
      ABORT
    | ABSOLUTE
    | ACCESS
    | ACTION
    | ADD
    | ADMIN
    | AFTER
    | AGGREGATE
    | ALL
    | ALSO
    | ALTER
    | ALWAYS
    | ANALYSE
    | ANALYZE
    | AND
    | ANY
    | ARRAY
    | AS
    | ASC
    | ASSERTION
    | ASSIGNMENT
    | ASYMMETRIC
    | AT
    | AUTHORIZATION
    | BACKWARD
    | BEFORE
    | BEGIN
    | BETWEEN
    | BIGINT
    | BINARY
    | BIT
    | BOOLEAN
    | BOTH
    | BY
    | CACHE
    | CALLED
    | CASCADE
    | CASCADED
    | CASE
    | CAST
    | CATALOG
    | CHAIN
    | CHAR
    | CHARACTER
    | CHARACTERISTICS
    | CHECK
    | CHECKPOINT
    | CLASS
    | CLOSE
    | CLUSTER
    | COALESCE
    | COLLATE
    | COLUMN
    | COMMENT
    | COMMENTS
    | COMMIT
    | COMMITTED
    | CONCURRENTLY
    | CONFIGURATION
    | CONNECTION
    | CONSTRAINT
    | CONSTRAINTS
    | CONTENT
    | CONTINUE
    | CONVERSION
    | COPY
    | COST
    | CREATE
    | CREATEDB
    | CREATEROLE
    | CREATEUSER
    | CROSS
    | CSV
    | CURRENT
    | CURRENT_CATALOG
    | CURRENT_DATE
    | CURRENT_ROLE
    | CURRENT_SCHEMA
    | CURRENT_TIME
    | CURRENT_TIMESTAMP
    | CURRENT_USER
    | CURSOR
    | CYCLE
    | DATA
    | DATABASE
    | DAY
    | DEALLOCATE
    | DEC
    | DECIMAL
    | DECLARE
    | DEFAULT
    | DEFAULTS
    | DEFERRABLE
    | DEFERRED
    | DEFINER
    | DELETE
    | DELIMITER
    | DELIMITERS
    | DESC
    | DICTIONARY
    | DISABLE
    | DISCARD
    | DISTINCT
    | DO
    | DOCUMENT
    | DOMAIN
    | DOUBLE
    | DROP
    | EACH
    | ELSE
    | ENABLE
    | ENCODING
    | ENCRYPTED
    | END
    | ENUM
    | ESCAPE
    | EXCEPT
    | EXCLUDE
    | EXCLUDING
    | EXCLUSIVE
    | EXECUTE
    | EXISTS
    | EXPLAIN
    | EXTERNAL
    | EXTRACT
    | FALSE
    | FAMILY
    | FETCH
    | FIRST
    | FLOAT
    | FOLLOWING
    | FOR
    | FORCE
    | FOREIGN
    | FORWARD
    | FREEZE
    | FROM
    | FULL
    | FUNCTION
    | FUNCTIONS
    | GLOBAL
    | GRANT
    | GRANTED
    | GREATEST
    | GROUP
    | HANDLER
    | HAVING
    | HEADER
    | HOLD
    | HOUR
    | IDENTITY
    | IF
    | ILIKE
    | IMMEDIATE
    | IMMUTABLE
    | IMPLICIT
    | IN
    | INCLUDING
    | INCREMENT
    | INDEX
    | INDEXES
    | INHERIT
    | INHERITS
    | INITIALLY
    | INLINE
    | INNER
    | INOUT
    | INPUT
    | INSENSITIVE
    | INSERT
    | INSTEAD
    | INT
    | INTEGER
    | INTERSECT
    | INTERVAL
    | INTO
    | INVOKER
    | IS
    | ISNULL
    | ISOLATION
    | JOIN
    | KEY
    | LANGUAGE
    | LARGE
    | LAST
    | LCCOLLATE
    | LCCTYPE
    | LEADING
    | LEAST
    | LEFT
    | LEVEL
    | LIKE
    | LIMIT
    | LISTEN
    | LOAD
    | LOCAL
    | LOCALTIME
    | LOCALTIMESTAMP
    | LOCATION
    | LOCK
    | LOGIN
    | MAPPING
    | MATCH
    | MAXVALUE
    | MINUTE
    | MINVALUE
    | MODE
    | MONTH
    | MOVE
    | NAME
    | NAMES
    | NATIONAL
    | NATURAL
    | NCHAR
    | NEXT
    | NO
    | NOCREATEDB
    | NOCREATEROLE
    | NOCREATEUSER
    | NOINHERIT
    | NOLOGIN
    | NONE
    | NOSUPERUSER
    | NOT
    | NOTHING
    | NOTIFY
    | NOTNULL
    | NOWAIT
    | NULL
    | NULLIF
    | NULLS
    | NUMERIC
    | OBJECT
    | OF
    | OFF
    | OFFSET
    | OIDS
    | ON
    | ONLY
    | OPERATOR
    | OPTION
    | OPTIONS
    | OR
    | ORDER
    | OUT
    | OUTER
    | OVER
    | OVERLAPS
    | OVERLAY
    | OWNED
    | OWNER
    | PARSER
    | PARTIAL
    | PARTITION
    | PASSWORD
    | PLACING
    | PLANS
    | POSITION
    | PRECEDING
    | PRECISION
    | PRESERVE
    | PREPARE
    | PREPARED
    | PRIMARY
    | PRIOR
    | PRIVILEGES
    | PROCEDURAL
    | PROCEDURE
    | QUOTE
    | RANGE
    | READ
    | REAL
    | REASSIGN
    | RECHECK
    | RECURSIVE
    | REFERENCES
    | REINDEX
    | RELATIVE
    | RELEASE
    | RENAME
    | REPEATABLE
    | REPLACE
    | REPLICA
    | RESET
    | RESTART
    | RESTRICT
    | RETURNING
    | RETURNS
    | REVOKE
    | RIGHT
    | ROLE
    | ROLLBACK
    | ROW
    | ROWS
    | RULE
    | SAVEPOINT
    | SCHEMA
    | SCROLL
    | SEARCH
    | SECOND
    | SECURITY
    | SELECT
    | SEQUENCE
    | SEQUENCES
    | SERIALIZABLE
    | SERVER
    | SESSION
    | SESSIONUSER
    | SET
    | SETOF
    | SHARE
    | SHOW
    | SIMILAR
    | SIMPLE
    | SMALLINT
    | SOME
    | STABLE
    | STANDALONE
    | START
    | STATEMENT
    | STATISTICS
    | STDIN
    | STDOUT
    | STORAGE
    | STRICT
    | STRIP
    | SUBSTRING
    | SUPERUSER
    | SYMMETRIC
    | SYSID
    | SYSTEM
    | TABLE
    | TABLES
    | TABLESPACE
    | TEMP
    | TEMPLATE
    | TEMPORARY
    | TEXT
    | THEN
    | TIME
    | TIMESTAMP
    | TO
    | TRAILING
    | TRANSACTION
    | TREAT
    | TRIGGER
    | TRIM
    | TRUE
    | TRUNCATE
    | TRUSTED
    | TYPE
    | UNBOUNDED
    | UNCOMMITTED
    | UNENCRYPTED
    | UNION
    | UNIQUE
    | UNKNOWN
    | UNLISTEN
    | UNTIL
    | UPDATE
    | USER
    | USING
    | VACUUM
    | VALID
    | VALIDATOR
    | VALUE
    | VALUES
    | VARCHAR
    | VARIADIC
    | VARYING
    | VERBOSE
    | VERSION
    | VIEW
    | VOLATILE
    | WHEN
    | WHERE
    | WHITESPACE
    | WINDOW
    | WITH
    | WITHOUT
    | WORK
    | WRAPPER
    | WRITE
    | XML
    | XMLATTRIBUTES
    | XMLCONCAT
    | XMLELEMENT
    | XMLFOREST
    | XMLPARSE
    | XMLPI
    | XMLROOT
    | XMLSERIALIZE
    | YEAR
    | YES
    | ZONE
    ;


//define keywords
fragment ABORT          :  'ABORT';
fragment ABSOLUTE       :  'ABSOLUTE';
fragment ACCESS           :  'ACCESS';
fragment ACTION           :  'ACTION';
fragment ADD            :  'ADD';
fragment ADMIN            :  'ADMIN';
fragment AFTER            :  'AFTER';

fragment AGGREGATE        :  'AGGREGATE';
fragment ALL              :  'ALL';
fragment ALSO             :  'ALSO';
fragment ALTER            :  'ALTER';
fragment ALWAYS           :  'ALWAYS';
fragment ANALYSE          :  'ANALYSE';
fragment ANALYZE          :  'ANALYZE';
fragment AND              :  'AND';
fragment ANY              :  'ANY';
fragment ARRAY            :  'ARRAY';
fragment AS               :  'AS';
fragment ASC              :  'ASC';

fragment ASSERTION        :  'ASSERTION';
fragment ASSIGNMENT       :  'ASSIGNMENT';
fragment ASYMMETRIC       :  'ASYMMETRIC';
fragment AT               :  'AT';
fragment AUTHORIZATION    :  'AUTHORIZATION';

fragment BACKWARD         :  'BACKWARD';
fragment BEFORE           :  'BEFORE';
fragment BEGIN          :  'BEGIN';
fragment BETWEEN          :  'BETWEEN';
fragment BIGINT           :  'BIGINT';
fragment BINARY           :  'BINARY';
fragment BIT              :  'BIT';

fragment BOOLEAN        :  'BOOLEAN';
fragment BOTH             :  'BOTH';
fragment BY               :  'BY';

fragment CACHE            :  'CACHE';
fragment CALLED           :  'CALLED';
fragment CASCADE          :  'CASCADE';
fragment CASCADED         :  'CASCADED';
fragment CASE             :  'CASE';
fragment CAST             :  'CAST';
fragment CATALOG        :  'CATALOG';
fragment CHAIN            :  'CHAIN';
fragment CHAR           :  'CHAR';

fragment CHARACTER        :  'CHARACTER';
fragment CHARACTERISTICS  :  'CHARACTERISTICS';
fragment CHECK            :  'CHECK';
fragment CHECKPOINT       :  'CHECKPOINT';
fragment CLASS            :  'CLASS';
fragment CLOSE            :  'CLOSE';

fragment CLUSTER          :  'CLUSTER';
fragment COALESCE         :  'COALESCE';
fragment COLLATE          :  'COLLATE';
fragment COLUMN           :  'COLUMN';
fragment COMMENT          :  'COMMENT';
fragment COMMENTS         :  'COMMENTS';
fragment COMMIT           :  'COMMIT';

fragment COMMITTED        :  'COMMITTED';
fragment CONCURRENTLY     :  'CONCURRENTLY';
fragment CONFIGURATION    :  'CONFIGURATION';
fragment CONNECTION       :  'CONNECTION';
fragment CONSTRAINT       :  'CONSTRAINT';
fragment CONSTRAINTS      :  'CONSTRAINTS';

fragment CONTENT        :  'CONTENT';
fragment CONTINUE       :  'CONTINUE';
fragment CONVERSION     :  'CONVERSION';
fragment COPY             :  'COPY';
fragment COST             :  'COST';
fragment CREATE           :  'CREATE';
fragment CREATEDB         :  'CREATEDB';

fragment CREATEROLE       :  'CREATEROLE';
fragment CREATEUSER       :  'CREATEUSER';
fragment CROSS            :  'CROSS';
fragment CSV              :  'CSV';
fragment CURRENT        :  'CURRENT';

fragment CURRENT_CATALOG  :  'CURRENT_CATALOG';
fragment CURRENT_DATE     :  'CURRENT_DATE';
fragment CURRENT_ROLE     :  'CURRENT_ROLE';
fragment CURRENT_SCHEMA   :  'CURRENT_SCHEMA';

fragment CURRENT_TIME     :  'CURRENT_TIME';
fragment CURRENT_TIMESTAMP:  'CURRENT_TIMESTAMP';
fragment CURRENT_USER     :  'CURRENT_USER';
fragment CURSOR           :  'CURSOR';
fragment CYCLE            :  'CYCLE';

fragment DATA           :  'DATA';
fragment DATABASE         :  'DATABASE';
fragment DAY            :  'DAY';
fragment DEALLOCATE       :  'DEALLOCATE';
fragment DEC              :  'DEC';
fragment DECIMAL        :  'DECIMAL';
fragment DECLARE          :  'DECLARE';
fragment DEFAULT          :  'DEFAULT';
fragment DEFAULTS         :  'DEFAULTS';

fragment DEFERRABLE       :  'DEFERRABLE';
fragment DEFERRED         :  'DEFERRED';
fragment DEFINER          :  'DEFINER';
fragment DELETE         :  'DELETE';
fragment DELIMITER        :  'DELIMITER';
fragment DELIMITERS       :  'DELIMITERS';
fragment DESC             :  'DESC';

fragment DICTIONARY       :  'DICTIONARY';
fragment DISABLE        :  'DISABLE';
fragment DISCARD          :  'DISCARD';
fragment DISTINCT         :  'DISTINCT';
fragment DO               :  'DO';
fragment DOCUMENT       :  'DOCUMENT';
fragment DOMAIN         :  'DOMAIN';
fragment DOUBLE         :  'DOUBLE';
fragment DROP             :  'DROP';

fragment EACH             :  'EACH';
fragment ELSE             :  'ELSE';
fragment ENABLE         :  'ENABLE';
fragment ENCODING         :  'ENCODING';
fragment ENCRYPTED        :  'ENCRYPTED';
fragment END            :  'END';
fragment ENUM           :  'ENUM';
fragment ESCAPE           :  'ESCAPE';
fragment EXCEPT           :  'EXCEPT';

fragment EXCLUDE          :  'EXCLUDE';
fragment EXCLUDING        :  'EXCLUDING';
fragment EXCLUSIVE        :  'EXCLUSIVE';
fragment EXECUTE          :  'EXECUTE';
fragment EXISTS           :  'EXISTS';
fragment EXPLAIN          :  'EXPLAIN';
fragment EXTERNAL         :  'EXTERNAL';
fragment EXTRACT          :  'EXTRACT';

fragment FALSE          :  'FALSE';
fragment FAMILY           :  'FAMILY';
fragment FETCH            :  'FETCH';
fragment FIRST          :  'FIRST';
fragment FLOAT          :  'FLOAT';
fragment FOLLOWING        :  'FOLLOWING';
fragment FOR              :  'FOR';
fragment FORCE            :  'FORCE';
fragment FOREIGN          :  'FOREIGN';
fragment FORWARD          :  'FORWARD';

fragment FREEZE           :  'FREEZE';
fragment FROM             :  'FROM';
fragment FULL             :  'FULL';
fragment FUNCTION         :  'FUNCTION';
fragment FUNCTIONS        :  'FUNCTIONS';

fragment GLOBAL           :  'GLOBAL';
fragment GRANT            :  'GRANT';
fragment GRANTED          :  'GRANTED';
fragment GREATEST         :  'GREATEST';
fragment GROUP          :  'GROUP';

fragment HANDLER          :  'HANDLER';
fragment HAVING           :  'HAVING';
fragment HEADER         :  'HEADER';
fragment HOLD             :  'HOLD';
fragment HOUR           :  'HOUR';

fragment IDENTITY       :  'IDENTITY';
fragment IF             :  'IF';
fragment ILIKE            :  'ILIKE';
fragment IMMEDIATE        :  'IMMEDIATE';
fragment IMMUTABLE        :  'IMMUTABLE';
fragment IMPLICIT       :  'IMPLICIT';
fragment IN             :  'IN';

fragment INCLUDING        :  'INCLUDING';
fragment INCREMENT        :  'INCREMENT';
fragment INDEX            :  'INDEX';
fragment INDEXES          :  'INDEXES';
fragment INHERIT          :  'INHERIT';
fragment INHERITS         :  'INHERITS';
fragment INITIALLY        :  'INITIALLY';
fragment INLINE         :  'INLINE';

fragment INNER          :  'INNER';
fragment INOUT            :  'INOUT';
fragment INPUT          :  'INPUT';
fragment INSENSITIVE      :  'INSENSITIVE';
fragment INSERT           :  'INSERT';
fragment INSTEAD          :  'INSTEAD';
fragment INT            :  'INT';
fragment INTEGER          :  'INTEGER';

fragment INTERSECT        :  'INTERSECT';
fragment INTERVAL         :  'INTERVAL';
fragment INTO             :  'INTO';
fragment INVOKER          :  'INVOKER';
fragment IS               :  'IS';
fragment ISNULL           :  'ISNULL';
fragment ISOLATION        :  'ISOLATION';

fragment JOIN             :  'JOIN';

fragment KEY              :  'KEY';

fragment LANGUAGE         :  'LANGUAGE';
fragment LARGE          :  'LARGE';
fragment LAST           :  'LAST';
fragment LC_COLLATE     :  'LC_COLLATE';
fragment LC_CTYPE       :  'LC_CTYPE';
fragment LEADING          :  'LEADING';

fragment LEAST            :  'LEAST';
fragment LEFT             :  'LEFT';
fragment LEVEL            :  'LEVEL';
fragment LIKE             :  'LIKE';
fragment LIMIT            :  'LIMIT';
fragment LISTEN           :  'LISTEN';
fragment LOAD             :  'LOAD';
fragment LOCAL            :  'LOCAL';
fragment LOCALTIME        :  'LOCALTIME';
fragment LOCALTIMESTAMP   :  'LOCALTIMESTAMP';

fragment LOCATION         :  'LOCATION';
fragment LOCK           :  'LOCK';
fragment LOGIN          :  'LOGIN';

fragment MAPPING          :  'MAPPING';
fragment MATCH            :  'MATCH';
fragment MAXVALUE         :  'MAXVALUE';
fragment MINUTE         :  'MINUTE';
fragment MINVALUE         :  'MINVALUE';
fragment MODE             :  'MODE';
fragment MONTH          :  'MONTH';
fragment MOVE             :  'MOVE';

fragment NAME           :  'NAME';
fragment NAMES            :  'NAMES';
fragment NATIONAL         :  'NATIONAL';
fragment NATURAL          :  'NATURAL';
fragment NCHAR            :  'NCHAR';
fragment NEXT             :  'NEXT';
fragment NO               :  'NO';
fragment NOCREATEDB       :  'NOCREATEDB';

fragment NOCREATEROLE     :  'NOCREATEROLE';
fragment NOCREATEUSER     :  'NOCREATEUSER';
fragment NOINHERIT        :  'NOINHERIT';
fragment NOLOGIN        :  'NOLOGIN';
fragment NONE             :  'NONE';
fragment NOSUPERUSER      :  'NOSUPERUSER';

fragment NOT              :  'NOT';
fragment NOTHING          :  'NOTHING';
fragment NOTIFY           :  'NOTIFY';
fragment NOTNULL          :  'NOTNULL';
fragment NOWAIT           :  'NOWAIT';
fragment NULL           :  'NULL';
fragment NULLIF           :  'NULLIF';
fragment NULLS          :  'NULLS';
fragment NUMERIC          :  'NUMERIC';

fragment OBJECT         :  'OBJECT';
fragment OF               :  'OF';
fragment OFF              :  'OFF';
fragment OFFSET           :  'OFFSET';
fragment OIDS             :  'OIDS';
fragment ON               :  'ON';
fragment ONLY             :  'ONLY';
fragment OPERATOR         :  'OPERATOR';
fragment OPTION           :  'OPTION';
fragment OPTIONS          :  'OPTIONS';
fragment OR               :  'OR';

fragment ORDER            :  'ORDER';
fragment OUT            :  'OUT';
fragment OUTER          :  'OUTER';
fragment OVER             :  'OVER';
fragment OVERLAPS         :  'OVERLAPS';
fragment OVERLAY          :  'OVERLAY';
fragment OWNED            :  'OWNED';
fragment OWNER            :  'OWNER';

fragment PARSER           :  'PARSER';
fragment PARTIAL          :  'PARTIAL';
fragment PARTITION        :  'PARTITION';
fragment PASSWORD         :  'PASSWORD';
fragment PLACING          :  'PLACING';
fragment PLANS            :  'PLANS';
fragment POSITION         :  'POSITION';

fragment PRECEDING        :  'PRECEDING';
fragment PRECISION        :  'PRECISION';
fragment PRESERVE         :  'PRESERVE';
fragment PREPARE          :  'PREPARE';
fragment PREPARED         :  'PREPARED';
fragment PRIMARY          :  'PRIMARY';

fragment PRIOR            :  'PRIOR';
fragment PRIVILEGES       :  'PRIVILEGES';
fragment PROCEDURAL       :  'PROCEDURAL';
fragment PROCEDURE        :  'PROCEDURE';

fragment QUOTE            :  'QUOTE';

fragment RANGE            :  'RANGE';
fragment READ             :  'READ';
fragment REAL             :  'REAL';
fragment REASSIGN         :  'REASSIGN';
fragment RECHECK          :  'RECHECK';
fragment RECURSIVE        :  'RECURSIVE';
fragment REFERENCES       :  'REFERENCES';
fragment REINDEX          :  'REINDEX';

fragment RELATIVE       :  'RELATIVE';
fragment RELEASE          :  'RELEASE';
fragment RENAME           :  'RENAME';
fragment REPEATABLE       :  'REPEATABLE';
fragment REPLACE          :  'REPLACE';
fragment REPLICA          :  'REPLICA';
fragment RESET            :  'RESET';
fragment RESTART          :  'RESTART';

fragment RESTRICT         :  'RESTRICT';
fragment RETURNING        :  'RETURNING';
fragment RETURNS          :  'RETURNS';
fragment REVOKE           :  'REVOKE';
fragment RIGHT            :  'RIGHT';
fragment ROLE             :  'ROLE';
fragment ROLLBACK         :  'ROLLBACK';
fragment ROW              :  'ROW';
fragment ROWS             :  'ROWS';
fragment RULE             :  'RULE';

fragment SAVEPOINT        :  'SAVEPOINT';
fragment SCHEMA           :  'SCHEMA';
fragment SCROLL           :  'SCROLL';
fragment SEARCH           :  'SEARCH';
fragment SECOND         :  'SECOND';
fragment SECURITY         :  'SECURITY';
fragment SELECT           :  'SELECT';
fragment SEQUENCE         :  'SEQUENCE';
fragment SEQUENCES        :  'SEQUENCES';

fragment SERIALIZABLE     :  'SERIALIZABLE';
fragment SERVER           :  'SERVER';
fragment SESSION          :  'SESSION';
fragment SESSION_USER     :  'SESSION_USER';
fragment SET              :  'SET';
fragment SETOF            :  'SETOF';
fragment SHARE            :  'SHARE';

fragment SHOW             :  'SHOW';
fragment SIMILAR          :  'SIMILAR';
fragment SIMPLE           :  'SIMPLE';
fragment SMALLINT         :  'SMALLINT';
fragment SOME             :  'SOME';
fragment STABLE           :  'STABLE';
fragment STANDALONE     :  'STANDALONE';
fragment START            :  'START';
fragment STATEMENT        :  'STATEMENT';

fragment STATISTICS       :  'STATISTICS';
fragment STDIN            :  'STDIN';
fragment STDOUT           :  'STDOUT';
fragment STORAGE          :  'STORAGE';
fragment STRICT         :  'STRICT';
fragment STRIP          :  'STRIP';
fragment SUBSTRING        :  'SUBSTRING';
fragment SUPERUSER      :  'SUPERUSER';

fragment SYMMETRIC        :  'SYMMETRIC';
fragment SYSID            :  'SYSID';
fragment SYSTEM         :  'SYSTEM';

fragment TABLE            :  'TABLE';
fragment TABLES           :  'TABLES';
fragment TABLESPACE       :  'TABLESPACE';
fragment TEMP             :  'TEMP';
fragment TEMPLATE         :  'TEMPLATE';
fragment TEMPORARY        :  'TEMPORARY';
fragment TEXT           :  'TEXT';
fragment THEN             :  'THEN';
fragment TIME             :  'TIME';
fragment TIMESTAMP        :  'TIMESTAMP';

fragment TO               :  'TO';
fragment TRAILING         :  'TRAILING';
fragment TRANSACTION      :  'TRANSACTION';
fragment TREAT            :  'TREAT';
fragment TRIGGER          :  'TRIGGER';
fragment TRIM             :  'TRIM';
fragment TRUE           :  'TRUE';

fragment TRUNCATE         :  'TRUNCATE';
fragment TRUSTED          :  'TRUSTED';
fragment TYPE           :  'TYPE';

fragment UNBOUNDED        :  'UNBOUNDED';
fragment UNCOMMITTED      :  'UNCOMMITTED';
fragment UNENCRYPTED      :  'UNENCRYPTED';
fragment UNION            :  'UNION';
fragment UNIQUE           :  'UNIQUE';
fragment UNKNOWN          :  'UNKNOWN';
fragment UNLISTEN         :  'UNLISTEN';
fragment UNTIL            :  'UNTIL';

fragment UPDATE           :  'UPDATE';
fragment USER             :  'USER';
fragment USING            :  'USING';

fragment VACUUM           :  'VACUUM';
fragment VALID            :  'VALID';
fragment VALIDATOR        :  'VALIDATOR';
fragment VALUE          :  'VALUE';
fragment VALUES           :  'VALUES';
fragment VARCHAR          :  'VARCHAR';
fragment VARIADIC         :  'VARIADIC';
fragment VARYING          :  'VARYING';

fragment VERBOSE          :  'VERBOSE';
fragment VERSION        :  'VERSION';
fragment VIEW             :  'VIEW';
fragment VOLATILE         :  'VOLATILE';

fragment WHEN             :  'WHEN';
fragment WHERE            :  'WHERE';
fragment WHITESPACE     :  'WHITESPACE';
fragment WINDOW           :  'WINDOW';
fragment WITH             :  'WITH';
fragment WITHOUT          :  'WITHOUT';
fragment WORK             :  'WORK';
fragment WRAPPER          :  'WRAPPER';
fragment WRITE            :  'WRITE';

fragment XML            :  'XML';
fragment XMLATTRIBUTES    :  'XMLATTRIBUTES';
fragment XMLCONCAT        :  'XMLCONCAT';
fragment XMLELEMENT       :  'XMLELEMENT';
fragment XMLFOREST        :  'XMLFOREST';
fragment XMLPARSE         :  'XMLPARSE';

fragment XMLPI            :  'XMLPI';
fragment XMLROOT          :  'XMLROOT';
fragment XMLSERIALIZE     :  'XMLSERIALIZE';

fragment YEAR           :  'YEAR';
fragment YES            :  'YES';

fragment ZONE             :  'ZONE';

// Other symbols
// basic token definition ------------------------------------------------------------

fragment DIVIDE	 : ( 'DIV') | '/' ;
fragment MOD_SYM : ( 'MOD') | '%' ;
fragment OR_SYM	 : ( 'OR')  | '||';
fragment AND_SYM : ( 'AND') | '&&';

ARROW	: '=>' ;
EQ_SYM	: '=' | '<=>' ;
NOT_EQ	: '<>' | '!=' | '~='| '^=';
LET	: '<=' ;
GET	: '>=' ;
SET_VAR	: ':=' ;
SHIFT_LEFT	: '<<' ;
SHIFT_RIGHT	: '>>' ;
ALL_FIELDS	: '.*' ;

SEMI	: ';' ;
COLON	: ':' ;
DOT	: '.' ;
COMMA	: ',' ;
ASTERISK: '*' ;
RPAREN	: ')' ;
LPAREN	: '(' ;
RBRACK	: ']' ;
LBRACK	: '[' ;
PLUS	: '+' ;
MINUS	: '-' ;
NEGATION: '~' ;
VERTBAR	: '|' ;
BITAND	: '&' ;
POWER_OP: '^' ;
GTH	: '>' ;
LTH	: '<' ;

ID : [A-Za-z0-9_]+;
	:	( 'A'..'Z' | 'a'..'z' | '_' | '$') ( 'A'..'Z' | 'a'..'z' | '_' | '$' | '0'..'9' )*

CHARACTERLITERAL
    :   '\'' (  ~('\''|'\\') )* '\''
    ;

OPERATOR
	: DIVIDE
    | MOD_SYM
    | OR_SYM
    | AND_SYM
    | ARROW
    | EQ_SYM
    | NOT_EQ
    | LET
    | GET
    | SET_VAR
    | SHIFT_LEFT
    | SHIFT_RIGHT
    | ALL_FIELDS
    | COLON
    | DOT
    | COMMA
    | ASTERISK
    | RPAREN
    | LPAREN
    | RBRACK
    | LBRACK
    | PLUS
    | MINUS
    | NEGATION
    | VERTBAR
    | BITAND
    | POWER_OP
    | GTH
    | LTH;

fragment
NEWLINE
    :    '\r' (options{greedy=true;}: '\n')?
    |    '\n'
    ;

LINE_COMMENT : '--' ~('\n'|'\r') )* '\r'? (NEWLINE|EOF);

WS
	: (' ' | '\t' | '\r' | '\n')+
	;
	

