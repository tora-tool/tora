
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#pragma once

//#include "core/tosqlparse.h"

#include "core/tocache.h"

#include <list>
#include <map>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtCore/QString>

class QWidget;
class toConnection;

// Liberally ported from DDL::Oracle 1.06
// Copyright (c) 2000, 2001 Richard Sutherland - United States of America

/**
 * This class can be used to reverse engineered database objects.
 */

class toExtract :public QObject
{
    Q_OBJECT;
    Q_ENUMS(ObjectType);
    public:
        enum ObjectType
        {
            NO_TYPE = toCache::NO_TYPE,  // used internally
            TABLE = toCache::TABLE,
            VIEW  = toCache::VIEW,
            SYNONYM = toCache::SYNONYM,
            PROCEDURE = toCache::PROCEDURE,
            FUNCTION = toCache::FUNCTION,
            PACKAGE = toCache::PACKAGE,
            PACKAGE_BODY = toCache::PACKAGE_BODY,
            INDEX = toCache::INDEX,
            SEQUENCE = toCache::SEQUENCE,
            TRIGGER = toCache::TRIGGER,
            //// These toCache enum values are not used by the extractor
            // DATABASE,         // used by MySQL
            // ANY,              // used for querying purposes only
            // TORA_SCHEMA_LIST, // Curious object type - used internally by TORA, if present browser knows that that schema was read from DB
            // TORA_USER_LIST,   // Curious object type - used internally by TORA purpose unknown so far.
            USER = toCache::USER,
            ROLE = toCache::ROLE,
            PROFILE = toCache::PROFILE,
            DATABASE_LINK = toCache::DATABASE_LINK,
            TYPE = toCache::TYPE,
            DIRECTORY = toCache::DIRECTORY,
            MATERIALIZED_VIEW = toCache::MATERIALIZED_VIEW,
            // Following enum values are not shared with toCache
            TABLE_FAMILY = toCache::OTHER + 1,// Table including indexes and constraints
            TABLE_CONTENTS,
            TABLE_REFERENCES,
            CONSTRAINT
        };


        /** Describes an available datatype for the database.
         */
        class datatype
        {
            private:
                QString Name;
                int MaxLength;
                int MaxPrecision;
            public:
                /** Create a new datatype description.
                 * @param name Name of datatype.
                 * @param maxLength Maximum length of parameter if length is needed.
                 * @param maxPrecision Maximum precision of parameter if precision is supported.
                 */
                datatype(const QString &name, int maxLength = -1, int maxPrecision = -1)
                    : Name(name), MaxLength(maxLength), MaxPrecision(maxPrecision)
                { }
                /** Returns true if type has length specification.
                 */
                bool hasLength()
                {
                    return MaxLength != -1;
                }
                /** Returns true if type has precision specification.
                 */
                bool hasPrecision()
                {
                    return MaxPrecision != -1;
                }
                /** Get the maximum length value.
                 */
                int maxLength()
                {
                    return MaxLength;
                }
                /** Get the maximum precision of the datatype.
                 */
                int maxPrecision()
                {
                    return MaxPrecision;
                }
                /** Get the name of the datatype.
                 */
                const QString &name()
                {
                    return Name;
                }

                /** Implemented so that you can easily sort a list of them. Just compares the names.
                 */
                bool operator < (const datatype &dat) const
                {
                    return Name < dat.Name;
                }
                /** Implemented so that you can easily sort a list of them. Just compares the names.
                 */
                bool operator == (const datatype &dat) const
                {
                    return Name == dat.Name;
                }
        };

        /** This is an abstract class to implement part of an extractor for a database. Observe
         * that an extractor must be stateless and threadsafe except for constructors and
         * destructors. Use the toExtract::context function for saving context.
         */
        class extractor
        {
            protected:
                toExtract &ext;
                /** Register an operation to be handled by this extractor.
                 * @param db Database this extractor works on.
                 * @param oper What kind of operation to implement. Can be one of CREATE,
                 *             DESCRIBE, MIGRATE or DROP.
                 * @param type The type of object that this is implemented for. Database specific.
                 */
                void registerExtract(const QString &db,
                                     const QString &oper,
                                     ObjectType type);
            public:
                /** Create an extractor. Normally called from a statical instantiator. Should register
                 * objects it can handle @ref registerExtract. Apart from the objects it handles one
                 * extractor per database can also register an empty operation and type parameter to
                 * @ref registerExtract which will be called to initialize an extractor once per
                 * constructed toExtract object.
                 * operation.
                 */
                extractor(toExtract &parent);
                /** Destructor.
                 */
                virtual ~extractor();

                /** Initiate an extractor for a given connection. Can be used to set up states necessary
                 * for further processing. Should probably set blocksize to be used for resizing (@ref
                 * toExtract::setBlocksize).
                 * @param ext Extractor to generate script.
                 */
                virtual void initialize() = 0;

                /** Called to generate a script to recreate a database object.
                 * @param ext Extractor to generate script.
                 * @param stream Stream to write script to.
                 * @param type Type of object to recreate.
                 * @param schema Specify the schema of the output script or description. If empty
                 *               don't specify any object. If the string "1" use same object as input.
                 *               Otherwise use the specified schema.
                 * @param owner Owner of database object.
                 * @param name Name of database object.
                 * @return A string containing a script to recreate an object.
                 */
                virtual void create(
                                    QTextStream &stream,
                                    ObjectType type,
                                    const QString &schema,
                                    const QString &owner,
                                    const QString &name) = 0;
                /** Called to describe a database object.
                 * @param ext Extractor to generate script.
                 * @param lst List of descriptions for the object. Should be appended.
                 * @param type Type of object to recreate.
                 * @param schema Specify the schema of the output script or description. If empty
                 *               don't specify any object. If the string "1" use same object as input.
                 *               Otherwise use the specified schema.
                 * @param owner Owner of database object.
                 * @param name Name of database object.
                 */
                virtual void describe(
                                      std::list<QString> &lst,
                                      ObjectType type,
                                      const QString &schema,
                                      const QString &owner,
                                      const QString &name) = 0;

                /** Get the available datatypes for the database.
                 */
                virtual std::list<datatype> datatypes() const;
            protected:
                toConnection& connection() { return ext.connection(); }
        };

        toConnection &Connection;
    private:
        QWidget *Parent;

        // Attributes
        QString Schema;
        bool Code;
        bool Comments;
        bool Constraints;
        bool Contents;
        bool Grants;
        bool Heading;
        bool Indexes;
        bool Parallel;
        bool Partition;
        bool Prompt;
        bool Storage;
        bool Initialized;
        bool Replace; // if object creation extracts should support RE-creation of existing objects

        int CommitDistance;

        // Database info
        int BlockSize;
        std::list<QString> Initial;
        std::list<QString> Next;
        std::list<QString> Limit;

        // Context, can be used by the extractor to save context
        std::map<QString, QVariant> Context;

        std::unique_ptr<extractor> ext;

        // General internal functions

        /** Parse an object string to get owner and name of the object.
         * @param object Object string on the format {owner}.{name}.
         * @param owner Reference to string which will get the object owner.
         * @param name Reference to string which will get the object name.
         */
        void parseObject(const QString &object, QString &owner, QString &name);

        void rethrow(const QString &what, const QString &object, const QString &exc);
        QString generateHeading(const QString &action, std::list<QString> &list);
        static std::map<QString, std::list<QString> > migrateGroup(std::list<QString> &grpLst);
    public:
        /** Create a new extractor.
         * @param conn Connection to extract from.
         * @param parent Parent widget of progress indicator.
         */
        toExtract(toConnection& conn, QWidget *parent);

        /** Create script to recreate list of objects.
         * @param object List of object. This has the format {type}:{schema}.{object}.
         *               The type is database dependent but can as an example be of
         *               CONSTRAINT, DATABASE LINK, EXCHANGE INDEX,
         *               EXCHANGE TABLE, FUNCTION, INDEX, MATERIALIZED VIEW,
         *               MATERIALIZED VIEW LOG, PACKAGE, PACKAGE BODY, PROCEDURE,
         *               PROFILE, ROLE, ROLE GRANTS, ROLLBACK SEGMENT, SEQUENCE,
         *               SNAPSHOT, SNAPSHOT LOG, SYNONYM, TABLE, TABLE FAMILY,
         *               TABLE CONTENTS, TABLE REFERENCES, TABLESPACE, TRIGGER,
         *               TRIGGER, TYPE, USER, USER GRANTS for Oracle databases.
         * @return A string containing a script to recreate the specified objects.
         */
        QString create(std::list<QString> &object)
        {
            QString ret;
            QTextStream s(&ret, QIODevice::WriteOnly);
            create(s, object);
            return ret;
        }
        /** Create script to recreate list of objects.
         * @param stream Stream to write result to.
         * @param object List of object. This has the format {type}:{schema}.{object}.
         *               The type is database dependent but can as an example be of
         *               CONSTRAINT, DATABASE LINK, EXCHANGE INDEX,
         *               EXCHANGE TABLE, FUNCTION, INDEX, MATERIALIZED VIEW,
         *               MATERIALIZED VIEW LOG, PACKAGE, PACKAGE BODY, PROCEDURE,
         *               PROFILE, ROLE, ROLE GRANTS, ROLLBACK SEGMENT, SEQUENCE,
         *               SNAPSHOT, SNAPSHOT LOG, SYNONYM, TABLE, TABLE FAMILY,
         *               TABLE CONTENTS, TABLE REFERENCES, TABLESPACE, TRIGGER,
         *               TRIGGER, TYPE, USER, USER GRANTS for Oracle databases.
         */
        void create(QTextStream &stream, std::list<QString> &object);

        /** Create a description of objects.
         * @param object List of object. This has the format {type}:{schema}.{object}.
         *               The type is database dependent but can as an example be of
         *               CONSTRAINT, DATABASE LINK, EXCHANGE INDEX,
         *               EXCHANGE TABLE, FUNCTION, INDEX, MATERIALIZED VIEW,
         *               MATERIALIZED VIEW LOG, PACKAGE, PACKAGE BODY, PROCEDURE,
         *               PROFILE, ROLE, ROLE GRANTS, ROLLBACK SEGMENT, SEQUENCE,
         *               SNAPSHOT, SNAPSHOT LOG, SYNONYM, TABLE, TABLE FAMILY,
         *               TABLE CONTENTS, TABLE REFERENCES, TABLESPACE, TRIGGER,
         *               TRIGGER, TYPE, USER, USER GRANTS for Oracle databases.
         * @return A list of strings describing the objects. Each string should be
         *         considered like a list of strings separated by the character '\001'.
         *         The later in each string the smaller item the change and it is hierachical.
         */
        std::list<QString> describe(std::list<QString> &object);

        /** Set a context for this extractor.
         * @param name Name of this context
         * @param val Value of this context
         */
        void setState(const QString &name, const QVariant &val);
        /** Get the value of a context for the current extractor.
         * @param name Name of the context to extract.
         * @return The value of the context.
         */
        QVariant state(const QString &name);

        /** Set the schema of the extraction.
         * @param schema Specify the schema of the output script or description. If empty
         *               don't specify any object. If the string "1" use same object as input.
         *               Otherwise use the specified schema.
         */
        void setSchema(const QString &schema)
        {
            Schema = schema;
        }
        /** Set inclusion of prompts.
         * @param prompt If prompt generation should be generated.
         */
        void setPrompt(bool prompt)
        {
            Prompt = prompt;
        }
        /** Include heading in scripts.
         * @param heading Include heading in scripts.
         */
        void setHeading(bool heading)
        {
            Heading = heading;
        }
        /** Include constraints in extraction.
         * @param constraints Include constraints.
         */
        void setConstraints(bool constraints)
        {
            Constraints = constraints;
        }
        /** Include indexes in extraction.
         * @param indexes Include indexes.
         */
        void setIndexes(bool indexes)
        {
            Indexes = indexes;
        }
        /** Include grants in extraction.
         * @param val Include grants.
         */
        void setGrants(bool val)
        {
            Grants = val;
        }
        /** Include storage specification in extraction.
         * @param val Include storage specification.
         */
        void setStorage(bool val)
        {
            Storage = val;
        }
        /** Include parallel specification in extraction.
         * @param val Include parallel specification.
         */
        void setParallel(bool val)
        {
            Parallel = val;
        }
        /** Include partition specification in extraction.
         * @param val Include partition specification.
         */
        void setPartition(bool val)
        {
            Partition = val;
        }
        /** Include contents of tables in scripts.
         * @param val Include contents of tables.
         * @param commitdistance The commit distance of the contents 0 means whole tables.
         */
        void setContents(bool val, int commitdistance)
        {
            Contents = val;
            CommitDistance = commitdistance;
        }
        /** Include comments in extraction.
         * @param val Include indexes.
         */
        void setComments(bool val)
        {
            Comments = val;
        }
        /** Include code in extraction.
         * @param val Include code.
         */
        void setCode(bool val)
        {
            Code = val;
        }
        /** Support RE-creation of existing objects in "create" extracts.
         * @param val Include code.
         */
        void setReplace(bool val)
        {
            Replace = val;
        }
        /** Set blocksize of database.
         * @param val New value of blocksize.
         */
        void setBlockSize(int val)
        {
            BlockSize = val;
        }

        /** Get schema specification.
         * @return Schema specification.
         * @see setSchema
         */
        const QString &getSchema(void)
        {
            return Schema;
        }
        /** Check if prompt are generated.
         * @return If prompts are generated.
         */
        bool getPrompt(void)
        {
            return Prompt;
        }
        /** Check if headings are generated.
         * @return If headings are generated.
         */
        bool getHeading(void)
        {
            return Heading;
        }
        /** Check if constraints are generated.
         * @return If constraints are generated.
         */
        bool getConstraints(void)
        {
            return Constraints;
        }
        /** Check if indexes are generated.
         * @return If indexes are generated.
         */
        bool getIndexes(void)
        {
            return Indexes;
        }
        /** Check if grants are generated.
         * @return If grants are generated.
         */
        bool getGrants(void)
        {
            return Grants;
        }
        /** Check if storage specification are generated.
         * @return If storage specification are generated.
         */
        bool getStorage(void)
        {
            return Storage;
        }
        /** Check if parallell specification are generated.
         * @return If parallell specification are generated.
         */
        bool getParallel(void)
        {
            return Parallel;
        }
        /** Check if partition specification are generated.
         * @return If partition specification  are generated.
         */
        bool getPartition(void)
        {
            return Partition;
        }
        /** Check if contents of tables are generated.
         * @return If content is generated.
         */
        bool getContents(void)
        {
            return Contents;
        }
        /** Check if support RE-creation of existing objects in "create" extracts is enabled.
         * @return If RE-creation is enabled.
         */
        bool getReplace(void)
        {
            return Replace;
        }
        /** Get the distance of the commits when content is generated.
         * @return Commit distance.
         */
        int getCommitDistance(void)
        {
            return CommitDistance;
        }
        /** Check if comments are generated.
         * @return If comments are generated.
         */
        bool getComments(void)
        {
            return Comments;
        }
        /** Check if code is generated.
         * @return If code is generated.
         */
        bool getCode(void)
        {
            return Code;
        }
        /** Get blocksize.
         */
        int getBlockSize(void)
        {
            return BlockSize;
        }
        /** Get the connection this extractor is working on.
         */
        toConnection & connection()
        {
            return Connection;
        }

        /** Get the schema name specified by the extractor setup. Will include the following '.'
         * if needed.
         * @param owner Owner of object to get schema for.
         * @param desc Used from describe and not to generate script.
         * @return The translated schema.
         */
        QString intSchema(const QString &owner, bool desc);

        /** Create a source and destination object list to two other lists
         * containing dropped and created objects or attributes.
         * @param source Source list input (Will not be modified).
         * @param destination Destination list input (Will not be modified).
         * @param drop Drop list output (Will be overwritten).
         * @param create Create list output (Will be overwritten).
         */
        static void srcDst2DropCreate(std::list<QString> &source, std::list<QString> &destination,
                                      std::list<QString> &drop, std::list<QString> &creat);

        /** Add a list to description.
         * @param ret The return list to add a line to.
         * @param ctx The current description context.
         * @param arg1 First extra argument to add.
         */
        static void addDescription(std::list<QString> &ret, const std::list<QString> &ctx,
                                   const QString &arg1 = QString::null, const QString &arg2 = QString::null,
                                   const QString &arg3 = QString::null, const QString &arg4 = QString::null,
                                   const QString &arg5 = QString::null, const QString &arg6 = QString::null,
                                   const QString &arg7 = QString::null, const QString &arg8 = QString::null,
                                   const QString &arg9 = QString::null);
        /** Split a description line into its components.
         * @param str The description line to split.
         * @return The list of components.
         */
        static std::list<QString> splitDescribe(const QString &str);

        /** Get the part of a description indicated by the level
         * @param str Description string
         * @param level The level to get (0 is the first level)
         */
        static QString partDescribe(const QString &str, int level);

        /** Get the context of a given level.
         * @param str Description string
         * @param level The level to get the context for (1 is the first to not return empty string.
         * @return The context, if not enough parts are available null is returned.
         */
        static QString contextDescribe(const QString &str, int level);

        /** Used to get column information from a describe list
         */
        struct columnInfo
        {
            /** Name of column
             */
            QString Name;
            /** Definition of column
             */
            QString Definition;
            /** Extra data for the column (Except for order which has own field.
             */
            std::map<QString, QString> Data;
            /** Order of the column. Used for sorting.
             */
            int Order;

            /** Can column be null?
             */
            bool bNotNull;

            /** Default value for column
             */
            QString DefaultValue;

            /** Column comment
             */
            QString Comment;

            /** Create column information from name.
             */
            columnInfo(const QString &name)
                : Name(name)
                , bNotNull(false)
            {
                Order = 0;
            }
            /** Implement sort order based only on Order field.
             */
            bool operator <(const columnInfo &inf) const
            {
                return Order < inf.Order;
            }
            /** Implement sort order based only on Order field.
             */
            bool operator ==(const columnInfo &inf) const
            {
                return Order == inf.Order;
            }
        };

        /** Parse a column description and return a sorted list of column data.
         * @param begin The iterator indicating the beginning to start searching from (Inclusive).
         * @param end The iterator indicating the end to end searching at (Not inclusive).
         * @param level Number of levels of context to discard before looking for "COLUMN" definition.
         * @return The list of column definitions. Sorted by column order.
         */
        static std::list<columnInfo> parseColumnDescription(std::list<QString>::const_iterator begin,
                std::list<QString>::const_iterator end,
                int level = 3);

        /** Parse a column description and return a sorted list of column data.
         * @param description A description list. Will go through entire list.
         * @param level Number of levels of context to discard before looking for "COLUMN" definition.
         * @return The list of column definitions. Sorted by column order.
         */
        static std::list<columnInfo> parseColumnDescription(const std::list<QString> &description, int level = 3)
        {
            return parseColumnDescription(description.begin(), description.end(), level);
        }

        static ObjectType objectTypeFromString(const QString& type);

        friend class extractor;
};

typedef TORA_EXPORT Util::GenericFactory<toExtract::extractor, LOKI_TYPELIST_1(toExtract &)> ExtractorFactory;
class TORA_EXPORT ExtractorFactorySing: public ::Loki::SingletonHolder<ExtractorFactory> {};
