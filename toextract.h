#ifndef __TO_EXTRACT
#define __TO_EXTRACT

#include "toconnection.h"

// Liberally ported from DDL::Oracle 1.06
// Copyright (c) 2000, 2001 Richard Sutherland - United States of America

class toExtract {
  toConnection &Connection;

  // Attributes
  QString Schema;
  QString Resize;
  bool Prompt;
  bool Heading;
  bool SQL;

  // Flags
  bool IsASnapIndex;
  bool IsASnapTable;

  // Database info
  int BlockSize;
  list<QString> Initial;
  list<QString> Next;
  list<QString> Limit;

  // Compile functions
  QString compile(const QString &schema,const QString &owner,const QString &name,
		  const QString &type);
  QString compilePackage(const QString &schema,const QString &owner,const QString &name,
			 const QString &type);

  // Create functions
  QString createConstraint(const QString &schema,const QString &owner,const QString &name);
  QString createDBLink(const QString &schema,const QString &owner,const QString &name);
  QString createExchangeIndex(const QString &schema,const QString &owner,const QString &name);
  QString createExchangeTable(const QString &schema,const QString &owner,const QString &name);
  QString createFunction(const QString &schema,const QString &owner,const QString &name);
  QString createIndex(const QString &schema,const QString &owner,const QString &name);
  QString createMaterializedView(const QString &schema,const QString &owner,const QString &name);
  QString createMaterializedViewLog(const QString &schema,const QString &owner,const QString &name);
  QString createPackage(const QString &schema,const QString &owner,const QString &name);
  QString createPackageBody(const QString &schema,const QString &owner,const QString &name);
  QString createProcedure(const QString &schema,const QString &owner,const QString &name);
  QString createProfile(const QString &schema,const QString &owner,const QString &name);
  QString createRole(const QString &schema,const QString &owner,const QString &name);
  QString createRollbackSegment(const QString &schema,const QString &owner,const QString &name);
  QString createSequence(const QString &schema,const QString &owner,const QString &name);
  QString createSnapshot(const QString &schema,const QString &owner,const QString &name);
  QString createSnapshotLog(const QString &schema,const QString &owner,const QString &name);
  QString createSynonym(const QString &schema,const QString &owner,const QString &name);
  QString createTable(const QString &schema,const QString &owner,const QString &name);
  QString createTableFamily(const QString &schema,const QString &owner,const QString &name);
  QString createTablespace(const QString &schema,const QString &owner,const QString &name);
  QString createTrigger(const QString &schema,const QString &owner,const QString &name);
  QString createType(const QString &schema,const QString &owner,const QString &name);
  QString createUser(const QString &schema,const QString &owner,const QString &name);
  QString createView(const QString &schema,const QString &owner,const QString &name);

  // Drop functions
  QString dropConstraint(const QString &schema,const QString &name,const QString &type,
			 const QString &owner);
  QString dropDatabaseLink(const QString &schema,const QString &name,const QString &type,
			   const QString &owner);
  QString dropDimension(const QString &schema,const QString &name,const QString &type,
			const QString &owner);
  QString dropDirectory(const QString &schema,const QString &name,const QString &type,
			const QString &owner);
  QString dropFunction(const QString &schema,const QString &name,const QString &type,
		       const QString &owner);
  QString dropIndex(const QString &schema,const QString &name,const QString &type,
		    const QString &owner);
  QString dropLibrary(const QString &schema,const QString &name,const QString &type,
		      const QString &owner);
  QString dropMaterializedView(const QString &schema,const QString &name,const QString &type,
			       const QString &owner);
  QString dropMaterializedViewLog(const QString &schema,const QString &name,const QString &type,
				  const QString &owner);
  QString dropPackage(const QString &schema,const QString &name,const QString &type,
		      const QString &owner);
  QString dropProcedure(const QString &schema,const QString &name,const QString &type,
			const QString &owner);
  QString dropProfile(const QString &schema,const QString &name,const QString &type,
		      const QString &owner);
  QString dropRole(const QString &schema,const QString &name,const QString &type,
		   const QString &owner);
  QString dropRollbackSegment(const QString &schema,const QString &name,const QString &type,
			      const QString &owner);
  QString dropSequence(const QString &schema,const QString &name,const QString &type,
		       const QString &owner);
  QString dropSnapshot(const QString &schema,const QString &name,const QString &type,
		       const QString &owner);
  QString dropSnapshotLog(const QString &schema,const QString &name,const QString &type,
			  const QString &owner);
  QString dropSynonym(const QString &schema,const QString &name,const QString &type,
		      const QString &owner);
  QString dropTable(const QString &schema,const QString &name,const QString &type,
		    const QString &owner);
  QString dropTablespace(const QString &schema,const QString &name,const QString &type,
			 const QString &owner);
  QString dropTrigger(const QString &schema,const QString &name,const QString &type,
		      const QString &owner);
  QString dropType(const QString &schema,const QString &name,const QString &type,
		   const QString &owner);
  QString dropUser(const QString &schema,const QString &name,const QString &type,
		   const QString &owner);
  QString dropView(const QString &schema,const QString &name,const QString &type,
		   const QString &owner);

  // Resize functions
  QString resizeIndex(const QString &schema,const QString &owner,const QString &name);
  QString resizeTable(const QString &schema,const QString &owner,const QString &name);

  // Misc extract functions
  QString constraintColumns(const QString &owner,const QString &name);
  QString createComments(const QString &schema,const QString &owner,const QString &name);
  QString indexColumns(const QString &schema,const QString &owner,const QString &name);
  QString createTableText(list<QString> &result,const QString &schema,
			  const QString &owner,const QString &name);
  QString tableColumns(const QString &owner,const QString &name);
  QString displaySource(const QString &schema,
			const QString &owner,const QString &name,
			const QString &type);
  QString createPartitionedIndex(const QString &schema,
				 const QString &owner,const QString &name,
				 const QString &soFar);
  QString rangePartitions(const QString &owner,const QString &name,
			  const QString &subPartitionType,const QString &locality);
  QString keyColumns(const QString &owner,const QString &name,
		     const QString &type,const QString &table);
  QString partitionKeyColumns(const QString &owner,const QString &name,const QString &type);
  QString subPartitionKeyColumns(const QString &owner,const QString &name,const QString &type);
  QString createMView(const QString &schema,const QString &owner,
		      const QString &name,const QString &type);
  QString createMViewLog(const QString &schema,const QString &owner,
			 const QString &name,const QString &type);
  QString createMViewTable(const QString &schema,const QString &owner,const QString &name);
  QString createMViewIndex(const QString &schema,const QString &owner,const QString &name);
  QString createPartitionedIOT(const QString &schema,const QString &owner,const QString &name);
  QString createIOT(const QString &schema,const QString &owner,const QString &name);
  QString createPartitionedTable(const QString &schema,const QString &owner,const QString &name);
  QString grantedPrivs(const QString &name);

  // Misc functions
  QString generateHeading(const QString &action,
			  const QString &type,
			  list<QString> &list);
  void parseObject(const QString &object,QString &owner,QString &name);
  void objectExists(const QString &owner,const QString &name,const QString &type);
  QString setSchema(const QString &owner);
  QString prepareDB(const QString &data);
  QString segmentAttributes(list<QString> &result);
  void initialNext(const QString &blocks,QString &initial,QString &next);
  void setSizes(void);
public:
  toExtract(toConnection &conn);

  QString compile(const QString &type,list<QString> &object);
  QString create(const QString &type,list<QString> &object);
  QString drop(const QString &type,list<QString> &object);
  QString resize(const QString &type,list<QString> &object);

#if 0
  void setSchema(const QString &schema)
  { Schema=schema; }
#endif
  void setResize(const QString &resize)
  { Resize=resize; setSizes(); }
  void setPrompt(bool prompt)
  { Prompt=prompt; }
  void setHeading(bool heading)
  { Heading=heading; }
  void setSQL(bool sql)
  { SQL=sql; }

  const QString &getSchema(void)
  { return Schema; }
  bool getResize(void)
  { return Resize; }
  bool getPrompt(void)
  { return Prompt; }
  bool getHeading(void)
  { return Heading; }
  bool getSQL(void)
  { return SQL; }
};

#endif
