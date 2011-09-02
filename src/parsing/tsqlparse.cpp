
#include "macros.h"
#include "tsqlparser_export.h"
#include "tsqlparse.h"

namespace SQLParser
{
	TSQLPARSER_EXPORT const char* SQLParser::Token::TokenType2Text[] = {
		"UNASSIGNED"
 		,"ROOT"
		,"FAILURE"
		
		,"COMMENT"
		,"RESERVED"
		,"TABLENAME"
		,"SCHEMANAME"
		,"FUNCTIONNAME"
		,"PROCEDURENAME"		
		,"PACKAGENAME"
		,"BINDVARNAME"
		,"CURSORNAME"
		,"EXCEPTIONNAME"
		,"TABLEALIAS"
		,"IDENTIFIER"
		,"JOIN"
		,"ON"
		,"JOINING_CLAUSE"		
 		,"USING"
		,"VARIABLE"
		,"DATATYPE"
		,"PARAMETER"
		,"SUBQUERY_FACTORED"
		,"SUBQUERY_NESTED"
		,"SUBQUERY_ALIAS"
		,"UNION"
		,"WITH"
		,"COLUMN_LIST"
		,"FROM"
		,"WHERE"
		,"COND_AND"
		,"COND_OR"
		,"IDENTIFIER"
		,"TABLE_REF"
	};

	TSQLPARSER_EXPORT  void Statement::token_const_iterator::increment()
	{
		if(m_token->isLeaf())
		{
			// while token has no right brother POP
			while(m_token->row() == m_token->parent()->childCount()-1)
			{
				m_stack.pop_back();
				m_token = m_stack.back();
			}
			// replace stack's bottom with node's right brother
			m_token = m_token->parent()->child(m_token->row()+1);
			m_stack.pop_back();
			m_stack.push_back(m_token);
		} else {
			m_token = m_token->child(0);
			m_stack.push_back(m_token);
		}
	};

	TSQLPARSER_EXPORT  bool Statement::token_const_iterator::equal(token_const_iterator const& other) const
	{
		return this->m_token == other.m_token;
	};

	TSQLPARSER_EXPORT  Statement::token_const_iterator Statement::begin() const
	{
		return token_const_iterator(_mAST);
	}

	TSQLPARSER_EXPORT Statement::token_const_iterator Statement::end() const
	{
		if(_mEnd)
			return token_const_iterator(_mEnd);
		_mEnd = _mAST;
		while(!_mEnd->isLeaf())
		{
			_mEnd = _mEnd->child( _mEnd->childCount()-1 );
		}
		return token_const_iterator(_mEnd);
	};


	TSQLPARSER_EXPORT void Statement::token_const_iterator_to_root::increment()
	{
		if(m_token)
			m_token = m_token->parent();
	};

	TSQLPARSER_EXPORT bool Statement::token_const_iterator_to_root::equal(Statement::token_const_iterator_to_root const& other) const
	{
		return this->m_token == other.m_token;
	};

};
