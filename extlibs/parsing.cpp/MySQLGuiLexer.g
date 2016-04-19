lexer grammar MySQLGuiLexer;

options 
{
	language=Cpp;
}

tokens {
    BIND_VAR_WITH_PARAMS;
}

@lexer::includes 
{
#include "UserGuiTraits.hpp"
#include <vector>
}

@lexer::namespace{ Antlr3GuiImpl }

MYSQL_RESERVED:
	  'ACCESSIBLE' 			// fragment ACCESSIBLE_SYM:
	| 'ACTION' 			// fragment ACTION:
	| 'ADD' 			// fragment ADD_SYM:
	| 'AFTER' 			// fragment AFTER_SYM:
	| 'AGAINST' 			// fragment AGAINST:
	| 'AGGREGATE' 			// fragment AGGREGATE_SYM:
	| 'ALGORITHM' 			// fragment ALGORITHM_SYM:
	| 'ALL' 			// fragment ALL:
	| 'ALTER' 			// fragment ALTER:
	| 'ANALYZE' 			// fragment ANALYZE_SYM:
	| 'ANY' 			// fragment ANY:
	| 'ARMSCII8' 			// fragment ARMSCII8:
	| 'AS' 				// fragment AS_SYM:
	| 'ASC' 			// fragment ASC:
	| 'ASENSITIVE' 			// fragment ASENSITIVE_SYM:
	| 'AT' 				// fragment AT_SYM:
	| 'AUTHORS' 			// fragment AUTHORS_SYM:
	| 'AUTO_INCREMENT' 		// fragment AUTO_INCREMENT:
	| 'AUTOEXTEND_SIZE' 		// fragment AUTOEXTEND_SIZE_SYM:
	| 'AVG_ROW_LENGTH' 		// fragment AVG_ROW_LENGTH:
	| 'BACKUP' 			// fragment BACKUP_SYM:
	| 'BEFORE' 			// fragment BEFORE_SYM:
	| 'BEGIN' 			// fragment BEGIN_SYM:
	| 'BETWEEN' 			// fragment BETWEEN:
	| 'BIG5' 			// fragment BIG5:
	| 'BIGINT' 			// fragment BIGINT:
	| 'BINARY' 			// fragment BINARY:
	| 'BINLOG' 			// fragment BINLOG_SYM:
	| 'BIT' 			// fragment BIT_SYM:
	| 'BLOB' 			// fragment BLOB_SYM:
	| 'BLOCK' 			// fragment BLOCK_SYM:
	| 'BOOL' 			// fragment BOOL_SYM:
	| 'BOOLEAN' 			// fragment BOOLEAN_SYM:
	| 'BOTH' 			// fragment BOTH:
	| 'BTREE' 			// fragment BTREE_SYM:
	| 'BY' 				// fragment BY_SYM:
	| 'BYTE' 			// fragment BYTE_SYM:
	| 'CACHE' 			// fragment CACHE_SYM:
	| 'CALL' 			// fragment CALL_SYM:
	| 'CASCADE' 			// fragment CASCADE:
	| 'CASCADED' 			// fragment CASCADED:
	| 'CASE' 			// fragment CASE_SYM:
	| 'CAST' 			// fragment CAST_SYM:
	| 'CHAIN' 			// fragment CHAIN_SYM:
	| 'CHANGE' 			// fragment CHANGE:
	| 'CHANGED' 			// fragment CHANGED:
	| 'CHARACTER' 			// fragment CHARACTER_SYM:
	| 'CHECK' 			// fragment CHECK_SYM:
	| 'CHECKSUM' 			// fragment CHECKSUM_SYM:
	| 'CIPHER' 			// fragment CIPHER_SYM:
	| 'CLIENT' 			// fragment CLIENT_SYM:
	| 'CLOSE' 			// fragment CLOSE_SYM:
	| 'COALESCE' 			// fragment COALESCE:
	| 'CODE' 			// fragment CODE_SYM:
	| 'COLLATE' 			// fragment COLLATE_SYM:
	| 'COLUMN_FORMAT' 		// fragment COLUMN_FORMAT:
	| 'COLUMN' 			// fragment COLUMN_SYM:
	| 'COLUMNS' 			// fragment COLUMNS_SYM:
	| 'COMMENT' 			// fragment COMMENT_SYM:
	| 'COMMIT' 			// fragment COMMIT_SYM:
	| 'COMMITTED' 			// fragment COMMITTED_SYM:
	| 'COMPACT' 			// fragment COMPACT_SYM:
	| 'COMPLETION' 			// fragment COMPLETION_SYM:
	| 'COMPRESS' 			// fragment COMPRESS:
	| 'COMPRESSED' 			// fragment COMPRESSED_SYM:
	| 'CONCURRENT' 			// fragment CONCURRENT:
	| 'CONDITION' 			// fragment CONDITION_SYM:
	| 'CONNECTION' 			// fragment CONNECTION_SYM:
	| 'CONSISTENT' 			// fragment CONSISTENT_SYM:
	| 'CONSTRAINT' 			// fragment CONSTRAINT:
	| 'CONTAINS' 			// fragment CONTAINS_SYM:
	| 'CONTEXT' 			// fragment CONTEXT_SYM:
	| 'CONTINUE' 			// fragment CONTINUE_SYM:
	| 'CONTRIBUTORS' 		// fragment CONTRIBUTORS_SYM:
	| 'CONVERT' 			// fragment CONVERT_SYM:
	| 'COPY' 			// fragment COPY_SYM:
	| 'CP1250' 			// fragment CP1250:
	| 'CP1251' 			// fragment CP1251:
	| 'CP1256' 			// fragment CP1256:
	| 'CP1257' 			// fragment CP1257:
	| 'CP850' 			// fragment CP850:
	| 'CP852' 			// fragment CP852:
	| 'CP866' 			// fragment CP866:
	| 'CP932' 			// fragment CP932:
	| 'CPU' 			// fragment CPU_SYM:
	| 'CREATE' 			// fragment CREATE:
	| 'CROSECOND' 			// fragment CROSECOND:
	| 'CROSS' 			// fragment CROSS:
	| 'CUBE' 			// fragment CUBE_SYM:
	| 'CURRENT_TIMESTAMP' 		// fragment CURRENT_TIMESTAMP:
	| 'CURSOR' 			// fragment CURSOR_SYM:
	| 'DATAFILE' 			// fragment DATAFILE_SYM:
	| 'DATA' 			// fragment DATA_SYM:
	| 'DATETIME' 			// fragment DATETIME:
	| 'DAY' 			// fragment DAY_SYM:
	| 'DAY_HOUR' 			// fragment DAY_HOUR:
	| 'DAY_MICROSECOND' 		// fragment DAY_MICROSECOND:
	| 'DAY_MINUTE' 			// fragment DAY_MINUTE:
	| 'DAY_SECOND' 			// fragment DAY_SECOND:
	| 'DEALLOCATE' 			// fragment DEALLOCATE_SYM:
	| 'DEC8' 			// fragment DEC8:
	| 'DECIMAL' 			// fragment DECIMAL_SYM:
	| 'DECLARE' 			// fragment DECLARE_SYM:
	| 'DEFINER' 			// fragment DEFINER:
	| 'DELAY_KEY_WRITE' 		// fragment DELAY_KEY_WRITE_SYM:
	| 'DELAYED' 			// fragment DELAYED_SYM:
	| 'DELETE' 			// fragment DELETE_SYM:
	| 'DES_KEY_FILE' 		// fragment DES_KEY_FILE:
	| 'DESC' 			// fragment DESC:
	| 'DETERMINISTIC' 		// fragment DETERMINISTIC_SYM:
	| 'DIRECTORY' 			// fragment DIRECTORY_SYM:
	| 'DISABLE' 			// fragment DISABLE_SYM:
	| 'DISCARD' 			// fragment DISCARD:
	| 'DISK' 			// fragment DISK_SYM:
	| 'DISTINCT' 			// fragment DISTINCT:
	| 'DISTINCTROW' 		// fragment DISTINCTROW:
	| 'DO' 				// fragment DO_SYM:
	| 'DOUBLE' 			// fragment DOUBLE_SYM:
	| 'DROP' 			// fragment DROP:
	| 'DUAL' 			// fragment DUAL_SYM:
	| 'DUMPFILE' 			// fragment DUMPFILE:
	| 'DUPLICATE' 			// fragment DUPLICATE_SYM:
	| 'DYNAMIC' 			// fragment DYNAMIC_SYM:
	| 'EACH' 			// fragment EACH_SYM:
	| 'ELSE' 			// fragment ELSE_SYM:
	| 'ELSIF' 			// fragment ELSIF_SYM:
	| 'ENABLE' 			// fragment ENABLE_SYM:
	| 'ENCLOSED' 			// fragment ENCLOSED:
	| 'END' 			// fragment END_SYM:
	| 'ENDS' 			// fragment ENDS_SYM:
	| 'ENGINE' 			// fragment ENGINE_SYM:
	| 'ENGINES' 			// fragment ENGINES_SYM:
	| 'ENUM' 			// fragment ENUM:
	| 'ERRORS' 			// fragment ERRORS:
	| 'ESCAPE' 			// fragment ESCAPE_SYM:
	| 'ESCAPED' 			// fragment ESCAPED:
	| 'EUCJPMS' 			// fragment EUCJPMS:
	| 'EUCKR' 			// fragment EUCKR:
	| 'EVENT' 			// fragment EVENT_SYM:
	| 'EVENTS' 			// fragment EVENTS_SYM:
	| 'EVERY' 			// fragment EVERY_SYM:
	| 'EXCHANGE_SYM' 		// fragment EXCHANGE_SYM:
	| 'EXECUTE' 			// fragment EXECUTE_SYM:
	| 'EXCLUSIVE' 			// fragment EXCLUSIVE_SYM:
	| 'EXISTS' 			// fragment EXISTS:
	| 'EXIT' 			// fragment EXIT_SYM:
	| 'EXPANSION' 			// fragment EXPANSION_SYM:
	| 'EXTENDED' 			// fragment EXTENDED_SYM:
	| 'EXTENT_SIZE' 		// fragment EXTENT_SIZE_SYM:
	| 'FALSE' 			// fragment FALSE: 
	| 'FAST' 			// fragment FAST_SYM:
	| 'FAULTS' 			// fragment FAULTS_SYM:
	| 'FIELDS' 			// fragment FIELDS_SYM:
	| 'FETCH' 			// fragment FETCH_SYM:
	| 'FILE' 			// fragment FILE_SYM:
	| 'FIRST' 			// fragment FIRST_SYM:
	| 'FIXED' 			// fragment FIXED_SYM:
	| 'FLOAT' 			// fragment FLOAT_SYM:
	| 'FLUSH' 			// fragment FLUSH_SYM:
	| 'FOR' 			// fragment FOR_SYM:
	| 'FORCE' 			// fragment FORCE_SYM:
	| 'FOREIGN' 			// fragment FOREIGN:
	| 'FOUND' 			// fragment FOUND_SYM:
	| 'FROM' 			// fragment FROM:
	| 'FULL' 			// fragment FULL:
	| 'FULLTEXT' 			// fragment FULLTEXT_SYM:
	| 'FUNCTION' 			// fragment FUNCTION_SYM:
	| 'GB2312' 			// fragment GB2312:
	| 'GBK' 			// fragment GBK:
	| 'GEOMETRY' 			// fragment GEOMETRY_SYM:
	| 'GEOMETRYCOLLECTION' 		// fragment GEOMETRYCOLLECTION:
	| 'GEOSTD8' 			// fragment GEOSTD8:
	| 'GLOBAL' 			// fragment GLOBAL_SYM:
	| 'GRANT' 			// fragment GRANT:
	| 'GRANTS' 			// fragment GRANTS:
	| 'GREEK' 			// fragment GREEK:
	| 'GROUP' 			// fragment GROUP_SYM:
	| 'HANDLER' 			// fragment HANDLER_SYM:
	| 'HASH' 			// fragment HASH_SYM:
	| 'HAVING' 			// fragment HAVING:
	| 'HEBREW' 			// fragment HEBREW:
	| 'HELP' 			// fragment HELP_SYM:
	| 'HIGH_PRIORITY' 		// fragment HIGH_PRIORITY:
	| 'HOST' 			// fragment HOST_SYM:
	| 'HOSTS' 			// fragment HOSTS_SYM:
	| 'HOUR_MICROSECOND' 		// fragment HOUR_MICROSECOND:
	| 'HOUR_MINUTE' 		// fragment HOUR_MINUTE:
	| 'HOUR_SECOND' 		// fragment HOUR_SECOND:
	| 'HP8' 			// fragment HP8:
	| 'IDENTIFIED' 			// fragment IDENTIFIED_SYM:
	| 'IGNORE' 			// fragment IGNORE_SYM:
	| 'IMPORT' 			// fragment IMPORT:
	| 'IN' 				// fragment IN_SYM:
	| 'INDEX' 			// fragment INDEX_SYM:
	| 'INDEXES' 			// fragment INDEXES:
	| 'INFILE' 			// fragment INFILE:
	| 'INITIAL_SIZE' 		// fragment INITIAL_SIZE_SYM:
	| 'INNER' 			// fragment INNER_SYM:
	| 'INOUT' 			// fragment INOUT_SYM:
	| 'INPLACE' 			// fragment INPLACE_SYM:
	| 'INSENSITIVE' 		// fragment INSENSITIVE_SYM:
	| 'INSERT_METHOD' 		// fragment INSERT_METHOD:
	| 'INSTALL' 			// fragment INSTALL_SYM:
	| 'INT' 			// fragment INT_SYM:
	| 'INTEGER' 			// fragment INTEGER_SYM:
	| 'INTERVAL' 			// fragment INTERVAL_SYM:
	| 'INTO' 			// fragment INTO:
	| 'INVOKER' 			// fragment INVOKER_SYM:
	| 'IO' 				// fragment IO_SYM:
	| 'IPC' 			// fragment IPC_SYM:
	| 'IS' 				// fragment IS_SYM:
	| 'ISOLATION' 			// fragment ISOLATION:
	| 'ISSUER' 			// fragment ISSUER_SYM:
	| 'ITERATE' 			// fragment ITERATE_SYM:
	| 'JOIN' 			// fragment JOIN_SYM:
	| 'KEY_BLOCK_SIZE' 		// fragment KEY_BLOCK_SIZE:
	| 'KEY' 			// fragment KEY_SYM:
	| 'KEYBCS2' 			// fragment KEYBCS2:
	| 'KEYS' 			// fragment KEYS:
	| 'KILL' 			// fragment KILL_SYM:
	| 'KOI8 R' 			// fragment KOI8R:
	| 'KOI8 U' 			// fragment KOI8U:
	| 'LANGUAGE' 			// fragment LANGUAGE:
	| 'LAST' 			// fragment LAST_SYM:
	| 'LATIN1_BIN' 			// fragment LATIN1_BIN:
	| 'LATIN1_GENERAL_CS' 		// fragment LATIN1_GENERAL_CS:
	| 'LATIN1' 			// fragment LATIN1:
	| 'LATIN2' 			// fragment LATIN2:
	| 'LATIN5' 			// fragment LATIN5:
	| 'LATIN7' 			// fragment LATIN7:
	| 'LEADING' 			// fragment LEADING:
	| 'LEAVE' 			// fragment LEAVE_SYM:
	| 'LEAVES' 			// fragment LEAVES:
	| 'LESS' 			// fragment LESS_SYM:
	| 'LEVEL' 			// fragment LEVEL_SYM:
	| 'LIKE' 			// fragment LIKE_SYM:
	| 'LIMIT' 			// fragment LIMIT:
	| 'LINEAR' 			// fragment LINEAR_SYM:
	| 'LINES' 			// fragment LINES:
	| 'LINESTRING' 			// fragment LINESTRING:
	| 'LIST' 			// fragment LIST_SYM:
	| 'LOAD' 			// fragment LOAD:
	| 'LOCAL' 			// fragment LOCAL_SYM:
	| 'LOCK' 			// fragment LOCK:
	| 'LOCKS' 			// fragment LOCKS_SYM:
	| 'LOGFILE' 			// fragment LOGFILE_SYM:
	| 'LOGS' 			// fragment LOGS_SYM:
	| 'LONG' 			// fragment LONG_SYM:
	| 'LONGBLOB' 			// fragment LONGBLOB:
	| 'LONGTEXT' 			// fragment LONGTEXT:
	| 'LOOP' 			// fragment LOOP_SYM:
	| 'LOW_PRIORITY' 		// fragment LOW_PRIORITY:
	| 'MACCE' 			// fragment MACCE:
	| 'MACROMAN' 			// fragment MACROMAN:
	| 'MASTER_CONNECT_RETRY' 	// fragment MASTER_CONNECT_RETRY:
	| 'MASTER_HOST' 		// fragment MASTER_HOST_SYM:
	| 'MASTER_LOG_FILE' 		// fragment MASTER_LOG_FILE_SYM:
	| 'MASTER_LOG_POS' 		// fragment MASTER_LOG_POS_SYM:
	| 'MASTER_PASSWORD' 		// fragment MASTER_PASSWORD_SYM:
	| 'MASTER_PORT' 		// fragment MASTER_PORT_SYM:
	| 'MASTER_SERVER_ID' 		// fragment MASTER_SERVER_ID_SYM:
	| 'MASTER_SSL_CA' 		// fragment MASTER_SSL_CA_SYM:
	| 'MASTER_SSL_CAPATH' 		// fragment MASTER_SSL_CAPATH_SYM:
	| 'MASTER_SSL_CERT' 		// fragment MASTER_SSL_CERT_SYM:
	| 'MASTER_SSL_CIPHER' 		// fragment MASTER_SSL_CIPHER_SYM:
	| 'MASTER_SSL_KEY' 		// fragment MASTER_SSL_KEY_SYM:
	| 'MASTER_SSL' 			// fragment MASTER_SSL_SYM:
	| 'MASTER_SSL_VERIFY_SERVER_CERT' // fragment MASTER_SSL_VERIFY_SERVER_CERT:
	| 'MASTER' 			// fragment MASTER_SYM:
	| 'MASTER_USER' 		// fragment MASTER_USER_SYM:
	| 'MATCH' 			// fragment MATCH:
	| 'MAX_CONNECTIONS_PER_HOUR' 	// fragment MAX_CONNECTIONS_PER_HOUR:
	| 'MAX_QUERIES_PER_HOUR' 	// fragment MAX_QUERIES_PER_HOUR:
	| 'MAX_ROWS' 			// fragment MAX_ROWS:
	| 'MAX_SIZE' 			// fragment MAX_SIZE_SYM:
	| 'MAX_UPDATES_PER_HOUR' 	// fragment MAX_UPDATES_PER_HOUR:
	| 'MAX_USER_CONNECTIONS' 	// fragment MAX_USER_CONNECTIONS_SYM:
	| 'MAXVALUE' 			// fragment MAXVALUE_SYM:
	| 'MEDIUM' 			// fragment MEDIUM_SYM:
	| 'MEDIUMBLOB' 			// fragment MEDIUMBLOB:
	| 'MEDIUMINT' 			// fragment MEDIUMINT:
	| 'MEDIUMTEXT' 			// fragment MEDIUMTEXT:
	| 'MEMORY' 			// fragment MEMORY_SYM:
	| 'MERGE' 			// fragment MERGE_SYM:
	| 'MIGRATE' 			// fragment MIGRATE_SYM:
	| 'MIN_ROWS' 			// fragment MIN_ROWS:
	| 'MINUTE_MICROSECOND' 		// fragment MINUTE_MICROSECOND:
	| 'MINUTE_SECOND' 		// fragment MINUTE_SECOND:
	| 'MODE' 			// fragment MODE_SYM:
	| 'MODIFIES' 			// fragment MODIFIES_SYM:
	| 'MODIFY' 			// fragment MODIFY_SYM:
	| 'MULTILINESTRING' 		// fragment MULTILINESTRING:
	| 'MULTIPOINT' 			// fragment MULTIPOINT:
	| 'MULTIPOLYGON' 		// fragment MULTIPOLYGON:
	| 'MUTEX' 			// fragment MUTEX_SYM:
	| 'NAME' 			// fragment NAME_SYM:
	| 'NAMES' 			// fragment NAMES_SYM:
	| 'NATIONAL' 			// fragment NATIONAL_SYM:
	| 'NATURAL' 			// fragment NATURAL:
	| 'NCHAR' 			// fragment NCHAR_SYM:
	| 'NEW' 			// fragment NEW_SYM:
	| 'NEXT' 			// fragment NEXT_SYM:
	| 'NO' 				// fragment NO_SYM:
	| 'NO_WAIT' 			// fragment NO_WAIT_SYM:
	| 'NO_WRITE_TO_BINLOG' 		// fragment NO_WRITE_TO_BINLOG:
	| 'NODEGROUP' 			// fragment NODEGROUP_SYM:
	| 'NONE' 			// fragment NONE_SYM:
	| ('NOT') | ('!') 		// fragment NOT_SYM:
	| 'NULL' 			// fragment NULL: 
	| 'NUMERIC' 			// fragment NUMERIC_SYM:
	| 'NVARCHAR' 			// fragment NVARCHAR_SYM:
	| 'OFFSET' 			// fragment OFFSET_SYM:
	| 'OJ' 				// fragment OJ_SYM:
	| 'ON' 				// fragment ON:
	| 'ONE_SHOT' 			// fragment ONE_SHOT_SYM:
	| 'ONE' 			// fragment ONE_SYM:
	| 'OPEN' 			// fragment OPEN_SYM:
	| 'OPTIMIZE' 			// fragment OPTIMIZE:
	| 'OPTION' 			// fragment OPTION:
	| 'OPTIONALLY' 			// fragment OPTIONALLY:
	| 'OPTIONS' 			// fragment OPTIONS_SYM:
	| 'ORDER' 			// fragment ORDER_SYM:
	| 'OUT' 			// fragment OUT_SYM:
	| 'OUTER' 			// fragment OUTER:
	| 'OUTFILE' 			// fragment OUTFILE:
	| 'OWNER' 			// fragment OWNER_SYM:
	| 'PACK_KEYS' 			// fragment PACK_KEYS_SYM:
	| 'PAGE_CHECKSUM' 		// fragment PAGE_CHECKSUM_SYM:
	| 'PAGE' 			// fragment PAGE_SYM:
	| 'PARSER' 			// fragment PARSER_SYM:
	| 'PARTIAL' 			// fragment PARTIAL:
	| 'PARTITION' 			// fragment PARTITION_SYM:
	| 'PARTITIONING' 		// fragment PARTITIONING_SYM:
	| 'PARTITIONS' 			// fragment PARTITIONS_SYM:
	| 'PHASE' 			// fragment PHASE_SYM:
	| 'PLUGIN' 			// fragment PLUGIN_SYM:
	| 'PLUGINS' 			// fragment PLUGINS_SYM:
	| 'POINT' 			// fragment POINT_SYM:
	| 'POLYGON' 			// fragment POLYGON:
	| 'PORT' 			// fragment PORT_SYM:
	| 'PRECISION' 			// fragment PRECISION:
	| 'PREPARE' 			// fragment PREPARE_SYM:
	| 'PRESERVE' 			// fragment PRESERVE_SYM:
	| 'PREV' 			// fragment PREV_SYM:
	| 'PRIMARY' 			// fragment PRIMARY_SYM:
	| 'PRIVILEGES' 			// fragment PRIVILEGES:
	| 'PROCEDURE' 			// fragment PROCEDURE:
	| 'PROCESS' 			// fragment PROCESS:
	| 'PROCESSLIST' 		// fragment PROCESSLIST_SYM:
	| 'PROFILE' 			// fragment PROFILE_SYM:
	| 'PROFILES' 			// fragment PROFILES_SYM:
	| 'PURGE' 			// fragment PURGE:
	| 'QUERY' 			// fragment QUERY_SYM:
	| 'QUICK' 			// fragment QUICK:
	| 'RANGE' 			// fragment RANGE_SYM:
	| 'READ_ONLY' 			// fragment READ_ONLY_SYM:
	| 'READ' 			// fragment READ_SYM:
	| 'READ_WRITE' 			// fragment READ_WRITE_SYM:
	| 'READS' 			// fragment READS_SYM:
	| 'REAL' 			// fragment REAL:
	| 'REBUILD' 			// fragment REBUILD_SYM:
	| 'RECOVER' 			// fragment RECOVER_SYM:
	| 'REDO_BUFFER_SIZE' 		// fragment REDO_BUFFER_SIZE_SYM:
	| 'REDOFILE' 			// fragment REDOFILE_SYM:
	| 'REDUNDANT' 			// fragment REDUNDANT_SYM:
	| 'REFERENCES' 			// fragment REFERENCES:
	| ('REGEXP') | ('RLIKE') 	// fragment REGEXP:
	| 'RELAY_LOG_FILE' 		// fragment RELAY_LOG_FILE_SYM:
	| 'RELAY_LOG_POS' 		// fragment RELAY_LOG_POS_SYM:
	| 'RELEASE' 			// fragment RELEASE_SYM:
	| 'RELOAD' 			// fragment RELOAD:
	| 'REMOVE' 			// fragment REMOVE_SYM:
	| 'RENAME' 			// fragment RENAME:
	| 'REORGANIZE' 			// fragment REORGANIZE_SYM:
	| 'REPAIR' 			// fragment REPAIR:
	| 'REPEATABLE' 			// fragment REPEATABLE_SYM:
	| 'REPLICATION' 		// fragment REPLICATION:
	| 'REQUIRE' 			// fragment REQUIRE_SYM:
	| 'RESET' 			// fragment RESET_SYM:
	| 'USER_RESOURCES' 		// fragment RESOURCES:
	| 'RESTORE' 			// fragment RESTORE_SYM:
	| 'RESTRICT' 			// fragment RESTRICT:
	| 'RESUME' 			// fragment RESUME_SYM:
	| 'RETURN' 			// fragment RETURN_SYM:
	| 'RETURNS' 			// fragment RETURNS_SYM:
	| 'REVOKE' 			// fragment REVOKE:
	| 'ROLLBACK' 			// fragment ROLLBACK:
	| 'ROLLUP' 			// fragment ROLLUP_SYM:
	| 'ROUTINE' 			// fragment ROUTINE_SYM:
	| 'ROW_FORMAT' 			// fragment ROW_FORMAT_SYM:
	| 'ROW' 			// fragment ROW_SYM:
	| 'ROWS' 			// fragment ROWS_SYM:
	| 'RTREE' 			// fragment RTREE_SYM:
	| 'SAVEPOINT' 			// fragment SAVEPOINT:
	| 'SCHEDULE' 			// fragment SCHEDULE_SYM:
	| 'SECOND_MICROSECOND' 		// fragment SECOND_MICROSECOND:
	| 'SECURITY' 			// fragment SECURITY_SYM:
	| 'SELECT' 			// fragment SELECT:
	| 'SENSITIVE' 			// fragment SENSITIVE_SYM:
	| 'SEPARATOR' 			// fragment SEPARATOR_SYM:
	| 'SERIAL' 			// fragment SERIAL_SYM:
	| 'SERIALIZABLE' 		// fragment SERIALIZABLE_SYM:
	| 'SERVER' 			// fragment SERVER_SYM:
	| 'SESSION' 			// fragment SESSION_SYM:
	| 'SET' 			// fragment SET_SYM:
	| 'SHARED' 			// fragment SHARED_SYM:
	| 'SHARE' 			// fragment SHARE_SYM:
	| 'SHOW' 			// fragment SHOW:
	| 'SHUTDOWN' 			// fragment SHUTDOWN:
	| 'SIGNED' 			// fragment SIGNED_SYM:
	| 'SIMPLE' 			// fragment SIMPLE_SYM:
	| 'SJIS' 			// fragment SJIS:
	| 'SLAVE' 			// fragment SLAVE:
	| 'SMALLINT' 			// fragment SMALLINT:
	| 'SNAPSHOT' 			// fragment SNAPSHOT_SYM:
	| 'SOCKET' 			// fragment SOCKET_SYM:
	| 'SONAME' 			// fragment SONAME_SYM:
	| 'SOUNDS' 			// fragment SOUNDS_SYM:
	| 'SOURCE' 			// fragment SOURCE_SYM:
	| 'SPATIAL' 			// fragment SPATIAL_SYM:
	| 'SPECIFIC' 			// fragment SPECIFIC_SYM:
	| 'SQL_BIG_RESULT' 		// fragment SQL_BIG_RESULT:
	| 'SQL_BUFFER_RESULT' 		// fragment SQL_BUFFER_RESULT:
	| 'SQL_CACHE' 			// fragment SQL_CACHE_SYM:
	| 'SQL_CALC_FOUND_ROWS' 	// fragment SQL_CALC_FOUND_ROWS:
	| 'SQL_NO_CACHE' 		// fragment SQL_NO_CACHE_SYM:
	| 'SQL_SMALL_RESULT' 		// fragment SQL_SMALL_RESULT:
	| 'SQL' 			// fragment SQL_SYM:
	| 'SQL_THREAD' 			// fragment SQL_THREAD:
	| 'SQLEXCEPTION' 		// fragment SQLEXCEPTION_SYM:
	| 'SQLSTATE' 			// fragment SQLSTATE_SYM:
	| 'SQLWARNING' 			// fragment SQLWARNING_SYM:
	| 'SSL' 			// fragment SSL_SYM:
	| 'START' 			// fragment START_SYM:
	| 'STARTING' 			// fragment STARTING:
	| 'STATS_AUTO_RECALC' 		// fragment STATS_AUTO_RECALC:
	| 'STATS_PERSISTENT' 		// fragment STATS_PERSISTENT:
	| 'STARTS' 			// fragment STARTS_SYM:
	| 'STATUS' 			// fragment STATUS_SYM:
	| 'STOP' 			// fragment STOP_SYM:
	| 'STORAGE' 			// fragment STORAGE_SYM:
	| 'STRAIGHT_JOIN' 		// fragment STRAIGHT_JOIN:
	| 'STRING' 			// fragment STRING_SYM:
	| 'SUBJECT' 			// fragment SUBJECT_SYM:
	| 'SUBPARTITION' 		// fragment SUBPARTITION_SYM:
	| 'SUBPARTITIONS' 		// fragment SUBPARTITIONS_SYM:
	| 'SUPER' 			// fragment SUPER_SYM:
	| 'SUSPEND' 			// fragment SUSPEND_SYM:
	| 'SWAPS' 			// fragment SWAPS_SYM:
	| 'SWE7' 			// fragment SWE7:
	| 'SWITCHES' 			// fragment SWITCHES_SYM:
	| 'TABLE' 			// fragment TABLE:
	| 'TABLE_CHECKSUM' 		// fragment TABLE_CHECKSUM_SYM:
	| 'TABLES' 			// fragment TABLES:
	| 'TABLESPACE' 			// fragment TABLESPACE:
	| 'TEMPORARY' 			// fragment TEMPORARY:
	| 'TEMPTABLE' 			// fragment TEMPTABLE_SYM:
	| 'TERMINATED' 			// fragment TERMINATED:
	| 'TEXT' 			// fragment TEXT_SYM:
	| 'THAN' 			// fragment THAN_SYM:
	| 'THEN' 			// fragment THEN_SYM:
	| 'TINYBLOB' 			// fragment TINYBLOB:
	| 'TINYINT' 			// fragment TINYINT:
	| 'TINYTEXT' 			// fragment TINYTEXT:
	| 'TIS620' 			// fragment TIS620:
	| 'TO' 				// fragment TO_SYM:
	| 'TRAILING' 			// fragment TRAILING:
	| 'TRANSACTION' 		// fragment TRANSACTION:
	| 'TRANSACTIONAL' 		// fragment TRANSACTIONAL_SYM:
	| 'TRIGGER' 			// fragment TRIGGER_SYM:
	| 'TRIGGERS' 			// fragment TRIGGERS_SYM:
    | 'TRUE' 			// fragment TRUE:
	| 'TYPE' 			// fragment TYPE_SYM:
	| 'TYPES' 			// fragment TYPES_SYM:
	| 'UCS2' 			// fragment UCS2:
	| 'UJIS' 			// fragment UJIS:
	| 'UNCOMMITTED' 		// fragment UNCOMMITTED_SYM:
	| 'UNCOMPRESS' 			// fragment UNCOMPRESS:
	| 'UNCOMPRESSED_LENGTH' 	// fragment UNCOMPRESSED_LENGTH:
	| 'UNDEFINED' 			// fragment UNDEFINED_SYM:
	| 'UNDO_BUFFER_SIZE' 		// fragment UNDO_BUFFER_SIZE_SYM:
	| 'UNDO' 			// fragment UNDO_SYM:
	| 'UNDOFILE' 			// fragment UNDOFILE_SYM:
	| 'UNICODE' 			// fragment UNICODE_SYM:
	| 'UNINSTALL' 			// fragment UNINSTALL_SYM:
	| 'UNION' 			// fragment UNION_SYM:
	| 'UNIQUE' 			// fragment UNIQUE_SYM:
	| 'UNKNOWN' 			// fragment UNKNOWN_SYM:
	| 'UNLOCK' 			// fragment UNLOCK_SYM:
	| 'UNSIGNED' 			// fragment UNSIGNED_SYM:
	| 'UNTIL' 			// fragment UNTIL_SYM:
	| 'UPDATE' 			// fragment UPDATE:
	| 'UPGRADE' 			// fragment UPGRADE_SYM:
	| 'USAGE' 			// fragment USAGE:
	| 'USE_FRM' 			// fragment USE_FRM:
	| 'USE' 			// fragment USE_SYM:
	| 'USING	' 		// fragment USING_SYM:
	| 'UTF16 LE' 			// fragment UTF16LE:
	| 'UTF16' 			// fragment UTF16:
	| 'UTF32' 			// fragment UTF32:
	| 'UTF8' 			// fragment UTF8:
	| 'VALUE' 			// fragment VALUE_SYM:
	| 'VARBINARY' 			// fragment VARBINARY:
	| 'VARCHAR' 			// fragment VARCHAR:
	| 'VARIABLES' 			// fragment VARIABLES:
	| 'VARYING' 			// fragment VARYING:
	| 'VIEW' 			// fragment VIEW_SYM:
	| 'WAIT' 			// fragment WAIT_SYM:
	| 'WARNINGS' 			// fragment WARNINGS:
	| 'WHEN	' 			// fragment WHEN_SYM:
	| 'WHERE' 			// fragment WHERE:
	| 'WHILE' 			// fragment WHILE_SYM:
	| 'WITH' 			// fragment WITH:
	| 'WORK' 			// fragment WORK_SYM:
	| 'WRAPPER' 			// fragment WRAPPER_SYM:
	| 'WRITE' 			// fragment WRITE_SYM:
	| 'X509' 			// fragment X509_SYM:
	| 'XA' 				// fragment XA_SYM:
	| 'XML' 			// fragment XML_SYM:
	| 'XOR' 			// fragment XOR:
	| 'YEAR_MONTH' 			// fragment YEAR_MONTH:
	| 'ZEROFILL' 			// fragment ZEROFILL:
;

// basic type definition -----------------------------------------------------------------------
// RELATIONAL_OP: 
// 	EQ_SYM | LTH | GTH | NOT_EQ | LET | GET  ;

// CHARSET_NAME:
// 	  ARMSCII8
// 	| ASCII_SYM
// 	| BIG5
// 	| BINARY
// 	| CP1250
// 	| CP1251
// 	| CP1256
// 	| CP1257
// 	| CP850
// 	| CP852
// 	| CP866
// 	| CP932
// 	| DEC8
// 	| EUCJPMS
// 	| EUCKR
// 	| GB2312
// 	| GBK
// 	| GEOSTD8
// 	| GREEK
// 	| HEBREW
// 	| HP8
// 	| KEYBCS2
// 	| KOI8R
// 	| KOI8U
// 	| LATIN1
// 	| LATIN2
// 	| LATIN5
// 	| LATIN7
// 	| MACCE
// 	| MACROMAN
// 	| SJIS
// 	| SWE7
// 	| TIS620
// 	| UCS2
// 	| UJIS
// 	| UTF8;

// INTERVAL_UNIT:
// 	  SECOND
// 	| MINUTE
// 	| HOUR
// 	| DAY_SYM
// 	| WEEK
// 	| MONTH
// 	| QUARTER
// 	| YEAR
// 	| SECOND_MICROSECOND
// 	| MINUTE_MICROSECOND
// 	| MINUTE_SECOND
// 	| HOUR_MICROSECOND
// 	| HOUR_SECOND
// 	| HOUR_MINUTE
// 	| DAY_MICROSECOND
// 	| DAY_SECOND
// 	| DAY_MINUTE
// 	| DAY_HOUR
// 	| YEAR_MONTH
// ;

// function defintion ------  http://dev.mysql.com/doc/refman/5.6/en/func-op-summary-ref.html  ----------
// buildin_functions
fragment ABS                             : 'ABS';
fragment ACOS                            : 'ACOS';
fragment ADDDATE                         : 'ADDDATE';
fragment ADDTIME                         : 'ADDTIME';
fragment AES_DECRYPT                     : 'AES_DECRYPT';
fragment AES_ENCRYPT                     : 'AES_ENCRYPT';
fragment ASCII_SYM                       : 'ASCII';
fragment ASIN                            : 'ASIN';
fragment ATAN                            : 'ATAN';
fragment ATAN2                           : 'ATAN2';
fragment AVG                             : 'AVG';
fragment BENCHMARK                       : 'BENCHMARK';
fragment BIN                             : 'BIN';
fragment BIT_AND                         : 'BIT_AND';
fragment BIT_LENGTH                      : 'BIT_LENGTH';
fragment BIT_OR                          : 'BIT_OR';
fragment BIT_XOR                         : 'BIT_XOR';
fragment CEIL                            : 'CEIL';
fragment CEILING                         : 'CEILING';
fragment CHAR                            : 'CHAR';
fragment CHARSET                         : 'CHARSET';
fragment CHAR_LENGTH                     : ('CHAR_LENGTH') | ('CHARACTER_LENGTH');
fragment COERCIBILITY                    : 'COERCIBILITY';
fragment COLLATION                       : 'COLLATION';
fragment CONCAT                          : 'CONCAT';
fragment CONCAT_WS                       : 'CONCAT_WS';
fragment CONNECTION_ID                   : 'CONNECTION_ID';
fragment CONV                            : 'CONV';
fragment CONVERT_TZ                      : 'CONVERT_TZ';
fragment COS                             : 'COS';
fragment COT                             : 'COT';
fragment COUNT                           : 'COUNT';
fragment CRC32                           : 'CRC32';
fragment CURDATE                         : ('CURDATE') | ('CURRENT_DATE');
fragment CURRENT_USER                    : 'CURRENT_USER';
fragment CURTIME                         : ('CURTIME') | ('CURRENT_TIME');
fragment DATABASE                        : 'DATABASE';
fragment DATEDIFF                        : 'DATEDIFF';
fragment DATE_ADD                        : 'DATE_ADD';
fragment DATE_FORMAT                     : 'DATE_FORMAT';
fragment DATE_SUB                        : ('DATE_SUB') | ('SUBDATE');
fragment DATE_SYM                        : 'DATE';
fragment DAYNAME                         : 'DAYNAME';
fragment DAYOFMONTH                      : ('DAYOFMONTH') | ('DAY');
fragment DAYOFWEEK                       : 'DAYOFWEEK';
fragment DAYOFYEAR                       : 'DAYOFYEAR';
fragment DECODE                          : 'DECODE';
fragment DEFAULT                         : 'DEFAULT';
fragment DEGREES                         : 'DEGREES';
fragment DES_DECRYPT                     : 'DES_DECRYPT';
fragment DES_ENCRYPT                     : 'DES_ENCRYPT';
fragment ELT                             : 'ELT';
fragment ENCODE                          : 'ENCODE';
fragment ENCRYPT                         : 'ENCRYPT';
fragment EXP                             : 'EXP';
fragment EXPORT_SET                      : 'EXPORT_SET';
fragment EXTRACT                         : 'EXTRACT';
fragment FIELD                           : 'FIELD';
fragment FIND_IN_SET                     : 'FIND_IN_SET';
fragment FLOOR                           : 'FLOOR';
fragment FORMAT                          : 'FORMAT';
fragment FOUND_ROWS                      : 'FOUND_ROWS';
fragment FROM_BASE64                     : 'FROM_BASE64';
fragment FROM_DAYS                       : 'FROM_DAYS';
fragment FROM_UNIXTIME                   : 'FROM_UNIXTIME';
fragment GET_FORMAT                      : 'GET_FORMAT';
fragment GET_LOCK                        : 'GET_LOCK';
fragment GROUP_CONCAT                    : 'GROUP_CONCAT';
fragment HEX                             : 'HEX';
fragment HOUR                            : 'HOUR';
fragment IF                              : 'IF';
fragment IFNULL                          : 'IFNULL';
fragment INET_ATON                       : 'INET_ATON';
fragment INET_NTOA                       : 'INET_NTOA';
fragment INSERT                          : 'INSERT';
fragment INSTR                           : 'INSTR';
fragment IS_FREE_LOCK                    : 'IS_FREE_LOCK';
fragment IS_USED_LOCK                    : 'IS_USED_LOCK';
fragment LAST_DAY                        : 'LAST_DAY';
fragment LAST_INSERT_ID                  : 'LAST_INSERT_ID';
fragment LEFT                            : 'LEFT';
fragment LENGTH                          : ('LENGTH') | ('OCTET_LENGTH');
fragment LN                              : 'LN';
fragment LOAD_FILE                       : 'LOAD_FILE';
fragment LOCATE                          : ('LOCATE') | ('POSITION');
fragment LOG                             : 'LOG';
fragment LOG10                           : 'LOG10';
fragment LOG2                            : 'LOG2';
fragment LOWER                           : ('LOWER') | ('LCASE');
fragment LPAD                            : 'LPAD';
fragment LTRIM                           : 'LTRIM';
fragment MAKEDATE                        : 'MAKEDATE';
fragment MAKETIME                        : 'MAKETIME';
fragment MAKE_SET                        : 'MAKE_SET';
fragment MASTER_POS_WAIT                 : 'MASTER_POS_WAIT';
fragment MAX_SYM                         : 'MAX';
fragment MD5                             : 'MD5';
fragment MICROSECOND                     : 'MICROSECOND';
fragment MID                             : 'MID';
fragment MINUTE                          : 'MINUTE';
fragment MIN_SYM                         : 'MIN';
fragment MOD                             : 'MOD';
fragment MONTH                           : 'MONTH';
fragment MONTHNAME                       : 'MONTHNAME';
fragment NAME_CONST                      : 'NAME_CONST';
fragment NOW                             : ('NOW') | ('LOCALTIME') | ('LOCALTIMESTAMP') | ('CURRENT_TIMESTAMP');
fragment OCT                             : 'OCT';
fragment OLD_PASSWORD                    : 'OLD_PASSWORD';
fragment ORD                             : 'ORD';
fragment PASSWORD                        : 'PASSWORD';
fragment PERIOD_ADD                      : 'PERIOD_ADD';
fragment PERIOD_DIFF                     : 'PERIOD_DIFF';
fragment PI                              : 'PI';
fragment POW                             : 'POW';
fragment POWER                           : 'POWER';
fragment QUARTER                         : 'QUARTER';
fragment QUOTE                           : 'QUOTE';
fragment RADIANS                         : 'RADIANS';
fragment RAND                            : 'RAND';
fragment RELEASE_LOCK                    : 'RELEASE_LOCK';
fragment REPEAT                          : 'REPEAT';
fragment REPLACE                         : 'REPLACE';
fragment REVERSE                         : 'REVERSE';
fragment RIGHT                           : 'RIGHT';
fragment ROUND                           : 'ROUND';
fragment RPAD                            : 'RPAD';
fragment RTRIM                           : 'RTRIM';
fragment SCHEMA                          : 'SCHEMA';
fragment SECOND                          : 'SECOND';
fragment SEC_TO_TIME                     : 'SEC_TO_TIME';
fragment SESSION_USER                    : 'SESSION_USER';
fragment SIGN                            : 'SIGN';
fragment SIN                             : 'SIN';
fragment SLEEP                           : 'SLEEP';
fragment SOUNDEX                         : 'SOUNDEX';
fragment SPACE                           : 'SPACE';
fragment SQRT                            : 'SQRT';
fragment STD                             : 'STD';
fragment STDDEV                          : 'STDDEV';
fragment STDDEV_POP                      : 'STDDEV_POP';
fragment STDDEV_SAMP                     : 'STDDEV_SAMP';
fragment STRCMP                          : 'STRCMP';
fragment STR_TO_DATE                     : 'STR_TO_DATE';
fragment SUBSTRING                       : ('SUBSTRING') | ('SUBSTR');
fragment SUBSTRING_INDEX                 : 'SUBSTRING_INDEX';
fragment SUBTIME                         : 'SUBTIME';
fragment SUM                             : 'SUM';
fragment SYSDATE                         : 'SYSDATE';
fragment SYSTEM_USER                     : 'SYSTEM_USER';
fragment TAN                             : 'TAN';
fragment TIMEDIFF                        : 'TIMEDIFF';
fragment TIMESTAMP                       : 'TIMESTAMP';
fragment TIMESTAMPADD                    : 'TIMESTAMPADD';
fragment TIMESTAMPDIFF                   : 'TIMESTAMPDIFF';
fragment TIME_FORMAT                     : 'TIME_FORMAT';
fragment TIME_SYM                        : 'TIME';
fragment TIME_TO_SEC                     : 'TIME_TO_SEC';
fragment TO_BASE64                       : 'TO_BASE64';
fragment TO_DAYS                         : 'TO_DAYS';
fragment TO_SECONDS                      : 'TO_SECONDS';
fragment TRIM                            : 'TRIM';
fragment TRUNCATE                        : 'TRUNCATE';
fragment UNHEX                           : 'UNHEX';
fragment UNIX_TIMESTAMP                  : 'UNIX_TIMESTAMP';
fragment UPPER                           : ('UPPER') | ('UCASE');
fragment USER                            : 'USER';
fragment UTC_DATE                        : 'UTC_DATE';
fragment UTC_TIME                        : 'UTC_TIME';
fragment UTC_TIMESTAMP                   : 'UTC_TIMESTAMP';
fragment UUID                            : 'UUID';
fragment VALUES                          : 'VALUES';
fragment VARIANCE                        : 'VARIANCE';
fragment VAR_POP                         : 'VAR_POP';
fragment VAR_SAMP                        : 'VAR_SAMP';
fragment VERSION_SYM                     : 'VERSION';
fragment WEEK                            : 'WEEK';
fragment WEEKDAY                         : 'WEEKDAY';
fragment WEEKOFYEAR                      : 'WEEKOFYEAR';
fragment WEIGHT_STRING                   : 'WEIGHT_STRING';
fragment YEAR                            : 'YEAR';
fragment YEARWEEK                        : 'YEARWEEK';

fragment NUMBER_FUNCTIONS:
	  ABS
	| ACOS
	| ASIN
	| ATAN2
	| ATAN
	| CEIL
	| CEILING
	| CONV
	| COS
	| COT
	| CRC32
	| DEGREES
	| EXP
	| FLOOR
	| LN
	| LOG10
	| LOG2
	| LOG
	| MOD
	| PI
	| POW
	| POWER
	| RADIANS
	| RAND
	| ROUND
	| SIGN
	| SIN
	| SQRT
	| TAN
	| TRUNCATE
;	

fragment CHAR_FUNCTIONS:
	  ASCII_SYM
	| BIN
	| BIT_LENGTH
	| CHAR_LENGTH
	| CHAR
	| CONCAT_WS
	| CONCAT
	| ELT
	| EXPORT_SET
	| FIELD
	| FIND_IN_SET
	| FORMAT
	| FROM_BASE64
	| HEX
	| INSERT
	| INSTR
	| LEFT
	| LENGTH
	| LOCATE
	| LOWER
	| LPAD
	| LTRIM
	| MID
	| OCT
	| ORD
	| QUOTE
	| REPEAT
	| REPLACE
	| REVERSE
	| RIGHT
	| RPAD
	| RTRIM
	| SOUNDEX
	| SPACE
	| STRCMP
	| SUBSTRING_INDEX
	| SUBSTRING
	| TO_BASE64
	| TRIM
	| UNHEX
	| UPPER
	| WEIGHT_STRING
;

fragment TIME_FUNCTIONS:
	  ADDDATE
	| ADDTIME
	| CONVERT_TZ
	| CURDATE
	| CURTIME
	| DATE_ADD
	| DATE_FORMAT
	| DATE_SUB
	| DATE_SYM
	| DATEDIFF
	| DAYNAME
	| DAYOFMONTH
	| DAYOFWEEK
	| DAYOFYEAR
	| EXTRACT
	| FROM_DAYS
	| FROM_UNIXTIME
	| GET_FORMAT
	| HOUR
	| LAST_DAY 
	| MAKEDATE
	| MAKETIME 
	| MICROSECOND
	| MINUTE
	| MONTH
	| MONTHNAME
	| NOW
	| PERIOD_ADD
	| PERIOD_DIFF
	| QUARTER
	| SEC_TO_TIME
	| SECOND
	| STR_TO_DATE
	| SUBTIME
	| SYSDATE
	| TIME_FORMAT
	| TIME_TO_SEC
	| TIME_SYM
	| TIMEDIFF
	| TIMESTAMP
	| TIMESTAMPADD
	| TIMESTAMPDIFF
	| TO_DAYS
	| TO_SECONDS
	| UNIX_TIMESTAMP
	| UTC_DATE
	| UTC_TIME
	| UTC_TIMESTAMP
	| WEEK
	| WEEKDAY
	| WEEKOFYEAR
	| YEAR
	| YEARWEEK
;

fragment OTHER_FUNCTIONS:
	  AES_DECRYPT
	| AES_ENCRYPT
	| BENCHMARK
	| CHARSET
	| COERCIBILITY
	| COLLATION
	| CONNECTION_ID
	| CURRENT_USER
	| DATABASE
	| DECODE
	| DEFAULT
	| DES_DECRYPT
	| DES_ENCRYPT
	| ENCODE
	| ENCRYPT
	| FOUND_ROWS
	| GET_LOCK
	| IF
	| IFNULL
	| INET_ATON
	| INET_NTOA
	| IS_FREE_LOCK
	| IS_USED_LOCK
	| LAST_INSERT_ID
	| LOAD_FILE
	| MAKE_SET
	| MASTER_POS_WAIT
	| MD5
	| NAME_CONST
	| OLD_PASSWORD
	| PASSWORD
	| RELEASE_LOCK
	| SCHEMA
	| SESSION_USER
	| SLEEP
	| SYSTEM_USER
	| USER
	| UUID
	| VALUES
	| VERSION_SYM
;

fragment GROUP_FUNCTIONS:
	AVG 
	| BIT_AND 
	| BIT_OR 
	| BIT_XOR
	| COUNT 
	| GROUP_CONCAT
	| MAX_SYM 
	| MIN_SYM 
	| STD 
	| STDDEV 
	| STDDEV_POP 
	| STDDEV_SAMP
	| SUM
	| VARIANCE
	| VAR_POP 
	| VAR_SAMP 
;

BUILDIN_FUNCTIONS:
	 NUMBER_FUNCTIONS 
	| CHAR_FUNCTIONS 
	| TIME_FUNCTIONS
	| OTHER_FUNCTIONS
;

// numbers

fragment UNSIGNED_INTEGER: ('0'..'9')+ ;
fragment REAL_NUMBER:
	(  UNSIGNED_INTEGER DOT UNSIGNED_INTEGER | UNSIGNED_INTEGER DOT | DOT UNSIGNED_INTEGER )
	(  ('E'|'e') ( PLUS | MINUS )? UNSIGNED_INTEGER  )? 
;

fragment HEX_DIGIT_FRAGMENT: ( 'a'..'f' | 'A'..'F' | '0'..'9' ) ;
fragment HEX_DIGIT:
	(  '0x'     (HEX_DIGIT_FRAGMENT)+  )
	|
	(  'X' '\'' (HEX_DIGIT_FRAGMENT)+ '\''  ) 
;

fragment BIT_NUM:
	(  '0b'    ('0'|'1')+  )
	|
	( 'B\'' ('0'|'1')+ '\''  ) 
;

NUMBER_LIT: UNSIGNED_INTEGER | REAL_NUMBER | HEX_DIGIT | BIT_NUM;

// basic token definition ------------------------------------------------------------

DIVIDE                          : ('DIV') | '/' ;
MOD_SYM                         : ('MOD') | '%' ;
OR_SYM                          : ('OR')  | '||';
AND_SYM                         : ('AND') | '&&';

ARROW                           : '=>' ;
EQ_SYM                          : '=' | '<=>' ;
NOT_EQ                          : '<>' | '!=' | '~='| '^=';
LET                             : '<=' ;
GET                             : '>=' ;
SET_VAR                         : ':=' ;
SHIFT_LEFT                      : '<<' ;
SHIFT_RIGHT                     : '>>' ;
ALL_FIELDS                      : '.*' ;

SEMI                            : ';' ;
//COLON                           : ':' ;
DOT                             : '.' ;
COMMA                           : ',' ;
ASTERISK                        : '*' ;
RPAREN                          : ')' ;
LPAREN                          : '(' ;
RBRACK                          : ']' ;
LBRACK                          : '[' ;
PLUS                            : '+' ;
MINUS                           : '-' ;
NEGATION                        : '~' ;
VERTBAR                         : '|' ;
BITAND                          : '&' ;
POWER_OP                        : '^' ;
GTH                             : '>' ;
LTH                             : '<' ;

fragment
SPACE_LIT
    :    ' '
    |    '\t'
    ;

fragment
NEWLINE
    :    '\r' (options{greedy=true;}: '\n')?
    |    '\n'
    ;

WHITE
	:	( SPACE_LIT | NEWLINE)+
	;

// http://dev.mysql.com/doc/refman/5.6/en/comments.html
COMMENT_SL
	: ('--'|'#') ( ~('\n'|'\r') )* (NEWLINE|EOF)
	;		
COMMENT_ML
	: '/*' ( options {greedy=false;} : . )* '*/'
	;


fragment ID
	:	( 'A'..'Z' | 'a'..'z' | '_' | '$') ( 'A'..'Z' | 'a'..'z' | '_' | '$' | '0'..'9' )*
	;

fragment BACKTICKED_ID
	:	(  '`' ( ('`' '`') | ~('`') )* '`'  )
	;

REGULAR_ID
	:	ID | BACKTICKED_ID
	;

fragment TEXT_STRING:
	( 'N' | ('_UTF8') )?
	(
		(  '\'' ( ('\\' '\\') | ('\'' '\'') | ('\\' '\'') | ~('\'') )* '\''  )
		|
		(  '\"' ( ('\\' '\\') | ('\"' '\"') | ('\\' '\"') | ~('\"') )* '\"'  ) 
	)
;

// http://dev.mysql.com/doc/refman/5.6/en/user-variables.html
USER_VAR:
	'@' (BACKTICKED_ID | USER_VAR_SUBFIX2 | USER_VAR_SUBFIX3 | USER_VAR_SUBFIX4)
;
//fragment USER_VAR_SUBFIX1:	(  '`' (~'`' )+ '`'  ) ;
fragment USER_VAR_SUBFIX2:	( '\'' (~'\'')+ '\'' ) ;
fragment USER_VAR_SUBFIX3:	( '\"' (~'\"')+ '\"' ) ;
fragment USER_VAR_SUBFIX4:	( 'A'..'Z' | 'a'..'z' | '_' | '$' | '0'..'9' | DOT )+ ;

// basic const data definition ---------------------------------------------------------------
STRING_LITERAL: TEXT_STRING /*| USER_VAR_SUBFIX2 | USER_VAR_SUBFIX3*/ | USER_VAR_SUBFIX4;

// GUI RULES
COMMENT_ML_PART
    :    '/*' (options{greedy=false;} : ~('*/') )* (NEWLINE)
    ;

COMMENT_ML_END
    :    '*/'
    ;

// -----------------------------------------------------------------------------
// Bind variables
BIND_VAR
	: '?'
;

BIND_VAR_WITH_NAME:
        ':' ID
        (
            LTH ID (LBRACK UNSIGNED_INTEGER RBRACK)? GTH { $type = BIND_VAR_WITH_PARAMS; }
        )?
;

// Last resort rule matches any character. This lexer should never fail.
TOKEN_FAILURE : . ;
