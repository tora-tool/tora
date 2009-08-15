
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TO_EXTRACT
#define TO_EXTRACT

#include "config.h"

#include <list>
#include <map>

#include <qfile.h>
#include <qstring.h>
#include <QTextStream>
#include <qvariant.h>
//Added by qt3to4:
#include <QString>

class QWidget;
class toConnection;

#include "tosqlparse.h"

// Liberally ported from DDL::Oracle 1.06
// Copyright (c) 2000, 2001 Richard Sutherland - United States of America

/**
 * This class can be used to reverse engineered database objects.
 */

class toExtract
{
public:
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
        /** Register an operation to be handled by this extractor.
         * @param db Database this extractor works on.
         * @param oper What kind of operation to implement. Can be one of CREATE,
         *             DESCRIBE, MIGRATE or DROP.
         * @param type The type of object that this is implemented for. Database specific.
         */
        void registerExtract(const QString &db,
                             const QString &oper,
                             const QString &type);
        /** Unregister an operation to be handled by this extractor.
         * @param db Database this extractor works on.
         * @param oper What kind of operation to implement. Can be one of CREATE,
         *             DESCRIBE, MIGRATE or DROP.
         * @param type The type of object that this is implemented for. Database specific.
         */
        void unregisterExtract(const QString &db,
                               const QString &oper,
                               const QString &type);
    public:
        /** Create an extractor. Normally called from a statical instantiator. Should register
         * objects it can handle @ref registerExtract. Apart from the objects it handles one
         * extractor per database can also register an empty operation and type parameter to
         * @ref registerExtract which will be called to initialize an extractor once per
         * constructed toExtract object.
         * operation.
         */
        extractor();
        /** Destructor.
         */
        virtual ~extractor();

        /** Initiate an extractor for a given connection. Can be used to set up states necessary
         * for further processing. Should probably set blocksize to be used for resizing (@ref
         * toExtract::setBlocksize).
         * @param ext Extractor to generate script.
         */
        virtual void initialize(toExtract &ext) const;

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
        virtual void create(toExtract &ext,
                            QTextStream &stream,
                            const QString &type,
                            const QString &schema,
                            const QString &owner,
                            const QString &name) const;
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
        virtual void describe(toExtract &ext,
                              std::list<QString> &lst,
                              const QString &type,
                              const QString &schema,
                              const QString &owner,
                              const QString &name) const;
        /** Called to generate a script to migrate a database object from one description to
         * another description.
         * @param ext Extractor to generate script.
         * @param type Type of object to migrate.
         * @param src Source description list.
         * @param dst Destination description list.
         * @return A script to change the src database object to dst.
         */
        virtual void migrate(toExtract &ext,
                             QTextStream &stream,
                             const QString &type,
                             std::list<QString> &src,
                             std::list<QString> &dst) const;

        /** Called to generate a script to drop an object.
         * @param ext Extractor to generate script.
         * @param type Type of object to recreate.
         * @param schema Specify the schema of the output script or description. If empty
         *               don't specify any object. If the string "1" use same object as input.
         *               Otherwise use the specified schema.
         * @param owner Owner of database object.
         * @param name Name of database object.
         * @return A string containing a script to recreate an object.
         */
        virtual void drop(toExtract &ext,
                          QTextStream &stream,
                          const QString &type,
                          const QString &schema,
                          const QString &owner,
                          const QString &name) const;

        /** Get the available datatypes for the database.
         */
        virtual std::list<datatype> datatypes() const;
    };

private:
    toConnection &Connection;
    QWidget *Parent;

    // Attributes
    QString Schema;
    QString Resize;
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

    int CommitDistance;

    // Database info
    int BlockSize;
    std::list<QString> Initial;
    std::list<QString> Next;
    std::list<QString> Limit;

    // Context, can be used by the extractor to save context
    std::map<QString, QVariant> Context;

    // Stuff to handle extractors
    static std::map<QString, extractor *> *Extractors;

    static void allocExtract(void);
    static QString extractorName(const QString &db,
                                 const QString &oper,
                                 const QString &type);
    static extractor *findExtractor(toConnection &conn,
                                    const QString &oper,
                                    const QString &type);
    extractor *findExtractor(const QString &oper,
                             const QString &type)
    {
        return findExtractor(Connection, oper, type);
    }
    void initialize(void);

    // General internal functions

    /** Parse an object string to get owner and name of the object.
     * @param object Object string on the format {owner}.{name}.
     * @param owner Reference to string which will get the object owner.
     * @param name Reference to string which will get the object name.
     */
    void parseObject(const QString &object, QString &owner, QString &name);
    void setSizes(void);

    void rethrow(const QString &what, const QString &object, const QString &exc);
    QString generateHeading(const QString &action, std::list<QString> &list);
    static std::map<QString, std::list<QString> > migrateGroup(std::list<QString> &grpLst);
public:
    /** Create a new extractor.
     * @param conn Connection to extract from.
     * @param parent Parent widget of progress indicator.
     */
    toExtract(toConnection &conn, QWidget *parent);

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

    /** Create script to drop a list of objects.
     * @param object List of object. This has the format {type}:{schema}.{object}.
     *               The type is database dependent but can as an example be of
     *               CONSTRAINT, DATABASE LINK, EXCHANGE INDEX,
     *               EXCHANGE TABLE, FUNCTION, INDEX, MATERIALIZED VIEW,
     *               MATERIALIZED VIEW LOG, PACKAGE, PACKAGE BODY, PROCEDURE,
     *               PROFILE, ROLE, ROLE GRANTS, ROLLBACK SEGMENT, SEQUENCE,
     *               SNAPSHOT, SNAPSHOT LOG, SYNONYM, TABLE, TABLE FAMILY,
     *               TABLE CONTENTS, TABLE REFERENCES, TABLESPACE, TRIGGER,
     *               TRIGGER, TYPE, USER, USER GRANTS for Oracle databases.
     * @return A string containing a script to drop the specified objects.
     */
    QString drop(std::list<QString> &object)
    {
        QString ret;
        QTextStream s(&ret, QIODevice::WriteOnly);
        drop(s, object);
        return ret;
    }

    /** Create script to drop a list of objects.
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
     * @return A string containing a script to drop the specified objects.
     */
    void drop(QTextStream &stream, std::list<QString> &object);

    /** Called to generate a script to migrate a database object from one description to
     * another description.
     * @param stream Stream to write result to.
     * @param src Source description list, generated by describes for the same database.
     * @param dst Destination description list, generated by describes for the same database.
     * @return A script to change the src database object to dst.
     */
    QString migrate(std::list<QString> &drpLst, std::list<QString> &crtLst)
    {
        QString ret;
        QTextStream s(&ret, QIODevice::WriteOnly);
        migrate(s, drpLst, crtLst);
        return ret;
    }

    /** Called to generate a script to migrate a database object from one description to
     * another description.
     * @param stream Stream to write result to.
     * @param src Source description list, generated by describes for the same database.
     * @param dst Destination description list, generated by describes for the same database.
     * @return A script to change the src database object to dst.
     */
    void migrate(QTextStream &stream, std::list<QString> &drpLst, std::list<QString> &crtLst);

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
    /** Set sizes to use for resize of object on extraction.
     * @param A list of sizes separated by ':'. Should be an even multiple of three where
     *        the first value is the largest current size to use these values. The next
     *        value is the initial value to use, the last is the next increment value to
     *        use. As a special case the string "1" can be used to set up auto resize.
     */
    void setResize(const QString &resize)
    {
        Resize = resize;
        setSizes();
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
    /** Set blocksize of database.
     * @param val New value of blocksize.
     */
    void setBlockSize(int val)
    {
        BlockSize = val;
        setSizes();
    }

    /** Get schema specification.
     * @return Schema specification.
     * @see setSchema
     */
    const QString &getSchema(void)
    {
        return Schema;
    }
    /** Get resize specification.
     * @return Resize specification.
     * @see setResize
     */
    bool getResize(void)
    {
        return !Resize.isEmpty();
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
    toConnection &connection()
    {
        return Connection;
    }

    /** Fill in the initial and next value for an object currently holding a @ref number of
     * allocated blocks. Uses the resize or default sizes.
     * @param blocks Blocks currently allocated.
     * @param initial New initial value.
     * @param next New next value.
     */
    void initialNext(const QString &blocks, QString &initial, QString &next);

    /** Get the schema name specified by the extractor setup. Will include the following '.'
     * if needed.
     * @param owner Owner of object to get schema for.
     * @param desc Used from describe and not to generate script.
     * @return The translated schema.
     */
    QString intSchema(const QString &owner, bool desc);

    /** Get the available datatypes for a database.
     * @return A list of datatypes.
     */
    std::list<datatype> datatypes();

    /** Utility function, creates a statement from the start to the end specified by the
     * parameters. The connection used to specify parsing is the one of the extractor.
     * @param start Start of parameters.
     * @param end End of parameters.
     * @return The indented statement.
     */
    QString createFromParse(std::list<toSQLParse::statement>::iterator start,
                            std::list<toSQLParse::statement>::iterator end);

    /** Create a source and destination object list to two other lists
     * containing dropped and created objects or attributes.
     * @param source Source list input (Will not be modified).
     * @param destination Destination list input (Will not be modified).
     * @param drop Drop list output (Will be overwritten).
     * @param create Create list output (Will be overwritten).
     */
    static void srcDst2DropCreate(std::list<QString> &source, std::list<QString> &destination,
                                  std::list<QString> &drop, std::list<QString> &creat);

    /** Check if a database is supported at all by the extractor.
     * @param conn Connection to check for support.
     * @return True if the database is supported.
     */
    static bool canHandle(toConnection &conn);

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

        /** Create column information from name.
         */
        columnInfo(const QString &name)
                : Name(name)
        {
            Order = 0;
        }
        /** Implement sort order based only on Order field.
         */
        bool operator <(const columnInfo &inf)
        {
            return Order < inf.Order;
        }
        /** Implement sort order based only on Order field.
         */
        bool operator ==(const columnInfo &inf)
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

    friend class extractor;
};

#endif
