create table data_types_1 (
  typecode_1		VARCHAR2(11) not null,
  typecode_1a		VARCHAR2(11 BYTE),
  typecode_1b		VARCHAR2(11 CHAR),
  typecode_1n		NVARCHAR2(11),
  typecode_2		NUMBER,
  typecode_2a		NUMBER(11),
  typecode_2b		NUMBER(11,22),
  typecode_8		LONG,
  typecode_12		DATE,
  typecode_23		RAW(11),
  typecode_104_69	ROWID,
  typecode_96		CHAR,
  typecode_96a		CHAR(11),
  typecode_96b		CHAR(11 BYTE),
  typecode_96c		CHAR(11 CHAR),
  typecode_96n		NCHAR,
  typecode_96na		NCHAR(11),
  typecode_100		BINARY_FLOAT,
  typecode_101		BINARY_DOUBLE,
--  typecode_108		UDT,
--  typecode_111		REF,
  typecode_112		CLOB,
  typecode_112n		NCLOB,
  typecode_113		BLOB,
/* unsupported by TOra */
--  typecode_114		BFILE,
  typecode_187_180	TIMESTAMP,
  typecode_187a_180	TIMESTAMP(9),
  typecode_188_181	TIMESTAMP WITH TIME ZONE,
  typecode_188a_181	TIMESTAMP(9) WITH TIME ZONE,
  typecode_189_182	INTERVAL YEAR TO MONTH,
  typecode_189a_182	INTERVAL YEAR(9) TO MONTH,
  typecode_190_183	INTERVAL DAY TO SECOND,
  typecode_190a_183	INTERVAL DAY(9) TO SECOND,
  typecode_208		UROWID,
  typecode_208a		UROWID(123),
  typecode_232_231	TIMESTAMP WITH LOCAL TIME ZONE,
  typecode_232a_231	TIMESTAMP(9) WITH LOCAL TIME ZONE
);

create or replace type nt_col as table of varchar2(64);
/

create table data_types_2 (
/* unsupported by TOra */
   typecode_anydata	anydata,
/* unsupported by TOra */
   typecode_nt		nt_col,
  typecode_24		LONG RAW)
nested table typecode_nt store as nt_col_tab;

