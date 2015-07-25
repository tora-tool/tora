
//#include "parsing/macros.h"
//// #include "tsqlparser_export.h"
#include "parsing/tsqlparse.h"

namespace SQLParser
{
    const char* SQLParser::Token::TokenType2Text[] =
    {
        "UNASSIGNED"
        , "ROOT"
        , "FAILURE"

        , "COMMENT"
        , "RESERVED"
        , "TABLENAME"
        , "SCHEMANAME"
        , "FUNCTIONNAME"
        , "PROCEDURENAME"
        , "PACKAGENAME"
        , "BINDVARNAME"
        , "CURSORNAME"
        , "EXCEPTIONNAME"
        , "TABLEALIAS"
        , "IDENTIFIER_PART"
        , "JOIN"
        , "ON"
        , "JOINING_CLAUSE"
        , "USING"
        , "VARIABLE"
        , "DATATYPE"
        , "PARAMETER"
        , "SUBQUERY_FACTORED"
        , "SUBQUERY_NESTED"
        , "SUBQUERY_ALIAS"
        , "UNION"
        , "WITH"
        , "COLUMN_LIST"
        , "FROM"
        , "WHERE"
        , "COND_AND"
        , "COND_OR"
        , "IDENTIFIER"
        , "TABLE_REF"
        , "OPERATOR_BINARY"
    };

    QMap<QPair<QString, QString>, ObjectCache::TableInfo>::const_iterator ObjectCache::getTable(QString const& currentSchema, QString const& schema, QString const& table) const
    {
        QMap<QPair<QString, QString>, ObjectCache::TableInfo>::const_iterator retval;
        QMap<QPair<QString, QString>, QPair<QString, QString> >::const_iterator synonymTranslationNew, synonymTranslationLast = _mSynonymMap.end();
        QSet<QPair<QString, QString> > loopStack;

        // 1st try - schema is empty
        if ( schema.isEmpty())
        {
            // 1st try (a) - schema is empty => "table" is visible in my currentSchema
            retval = _mTableMap.constFind(qMakePair(currentSchema, table));
            if ( retval != _mTableMap.end())
                return retval;

            // 1st try (b) - schema is empty => "table" is local synonym in my currentSchema
            synonymTranslationNew = _mSynonymMap.constFind(qMakePair(currentSchema, table));
            // 1st try (c) - schema is empty and table was not found in currentSchema => is it PUBLIC SYNONYM?
            if ( synonymTranslationNew == _mSynonymMap.end())
                synonymTranslationNew = _mSynonymMap.constFind(qMakePair(QString("PUBLIC"), table));

            if ( synonymTranslationNew != _mSynonymMap.end())
            {
                loopStack.insert(*synonymTranslationNew);

                do
                {
                    // keep previous "valid" translation
                    synonymTranslationLast = synonymTranslationNew;

                    // check whether the synonym points onto another synonym
                    synonymTranslationNew = _mSynonymMap.constFind(*synonymTranslationLast);

                    if ( synonymTranslationNew == _mSynonymMap.end())
                        // check whether synonym points onto PUBLIC synonym
                        synonymTranslationNew = _mSynonymMap.constFind(qMakePair(QString("PUBLIC"), synonymTranslationLast->second));

                    if ( synonymTranslationNew != _mSynonymMap.end()) // synonym translation was successfull
                    {
                        if ( loopStack.contains(*synonymTranslationNew))
                            return _mTableMap.end(); // Loop detected, confused -> do not return anything
                        else
                            loopStack.insert(*synonymTranslationNew);
                    }
                }
                while ( synonymTranslationNew != _mSynonymMap.end());
            }
        }
        else     // if( schema.isEmpty())
        {
            // 2nd try(a) - "SCHEMA.TABLE" is a table
            QMap<QPair<QString, QString>, ObjectCache::TableInfo>::const_iterator retval;
            retval = _mTableMap.constFind(qMakePair(schema, table));
            if ( retval != _mTableMap.end())
                return retval;

            // 2st try (b) - "schema.table" is a synonym
            QMap<QPair<QString, QString>, QPair<QString, QString> >::const_iterator synonymTranslationNew, synonymTranslationLast;
            QSet<QPair<QString, QString> > loopStack;
            synonymTranslationNew = _mSynonymMap.constFind(qMakePair(schema, table));

            if ( synonymTranslationNew != _mSynonymMap.end())
            {
                loopStack.insert(*synonymTranslationNew);

                do
                {
                    // keep previous "valid" translation
                    synonymTranslationLast = synonymTranslationNew;

                    // check whether synonym points onto another synonym
                    synonymTranslationNew = _mSynonymMap.constFind(*synonymTranslationLast);

                    if ( synonymTranslationNew != _mSynonymMap.end()) // synonym translation was successfull
                    {
                        if ( loopStack.contains(*synonymTranslationNew))
                            return _mTableMap.end(); // Loop detected, confised -> do not return anything
                        else
                            loopStack.insert(*synonymTranslationNew);
                    }
                }
                while ( synonymTranslationNew != _mSynonymMap.end());
            }
        } // if( schema.isEmpty()) else ..

        if ( synonymTranslationLast == _mSynonymMap.end())
            return _mTableMap.end();

        retval = _mTableMap.constFind(*synonymTranslationLast);
        if ( retval != _mTableMap.end())
            return retval;

        return _mTableMap.end();
    };

    Token const* Statement::translateAlias(QString const& alias, Token const *context)
    {
        for ( SQLParser::Statement::token_const_iterator_to_root k(context); k->parent(); ++k)
        {
            if ( k->getTokenType() == Token::S_SUBQUERY_NESTED ||
                    k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
                    k->getTokenType() == Token::X_ROOT)
            {
                TokenSubquery const &s = static_cast<TokenSubquery const&>(*k);
                if ( s.aliasTranslation().contains(alias.toUpper()))
                {
                    Token const* a1 = s.aliasTranslation().value(alias.toUpper());
                    if ( a1->getTokenType() == Token::S_TABLE_REF && a1->childCount() == 1 ) // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
                    {
                        Token const* a2 = translateAlias( a1->toStringRecursive(false), &*k);
                        if ( a2)
                            return a2;
                    }
                    return a1;
                }
            }
        }
        return NULL;
    };

    Token const* Statement::getTableRef(QString const& tableName, Token const *context)
    {
        for ( SQLParser::Statement::token_const_iterator_to_root k(context); k->parent(); ++k)
        {
            if ( k->getTokenType() == Token::S_SUBQUERY_NESTED ||
                    k->getTokenType() == Token::S_SUBQUERY_FACTORED ||
                    k->getTokenType() == Token::X_ROOT)
            {
                TokenSubquery const &s = static_cast<TokenSubquery const&>(*k);
                if ( s.nodeTables().contains(tableName.toUpper()))
                {
                    Token const* t1 = s.nodeTables().value(tableName.toUpper());
                    Token const* t2 = s.aliasTranslation().value(tableName.toUpper()); // In fact S_TABLE_REF can be alias for S_SUBQUERY_FACTORED
                    if ( t2)
                        return t2;
                    if ( t1)
                        return t1;
                }
            }
        }
        return NULL;
    };


    void Statement::token_const_iterator::increment()
    {
        if (m_token->isLeaf())
        {
            // while token has no right brother POP
            while (m_token->row() == m_token->parent()->childCount() - 1)
            {
                m_stack.pop_back();
                m_token = m_stack.back();
            }
            // replace stack's bottom with node's right brother
            m_token = m_token->parent()->child(m_token->row() + 1);
            m_stack.pop_back();
            m_stack.push_back(m_token);
        }
        else
        {
            m_token = m_token->child(m_lastIndex);
            m_stack.push_back(m_token);
        }
        m_lastIndex = 0;
    };

    void Statement::token_const_iterator::decrement()
    {
        m_lastIndex = m_token->row(); // Ugly hack - by remembering the row we do support one decrement in a row - not more
        m_stack.pop_back();
        m_token = m_stack.back();
    };

    bool Statement::token_const_iterator::equal(token_const_iterator const& other) const
    {
        return this->m_token == other.m_token;
    };

    Statement::token_const_iterator Statement::begin() const
    {
        return token_const_iterator(_mAST);
    }

    Statement::token_const_iterator Statement::end() const
    {
        if (_mEnd)
            return token_const_iterator(_mEnd);
        _mEnd = _mAST;
        while (!_mEnd->isLeaf())
        {
            _mEnd = _mEnd->child( _mEnd->childCount() - 1 );
        }
        return token_const_iterator(_mEnd);
    };


    void Statement::token_const_iterator_to_root::increment()
    {
        if (m_token)
            m_token = m_token->parent();
    };

    bool Statement::token_const_iterator_to_root::equal(Statement::token_const_iterator_to_root const& other) const
    {
        return this->m_token == other.m_token;
    };

    void Statement::token_iterator_to_root::increment()
    {
        if (m_token)
            m_token = m_token->parent();
    };

    bool Statement::token_iterator_to_root::equal(Statement::token_iterator_to_root const& other) const
    {
        return this->m_token == other.m_token;
    };

};
