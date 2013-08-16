/** \file
 * This is the include file for the ANTLR3 sample project "dynamic-scope"
 * which shows how to use a C grammar and call it from C code.
 */
#ifndef	_T_TRAITS_H
#define	_T_TRAITS_H

// First thing we always do is include the ANTLR3 generated files, which
// will automatically include the antlr3 runtime header files.
// The compiler must use -I (or set the project settings in VS2005)
// to locate the antlr3 runtime files and -I. to find this file
//
#include <antlr3.hpp>
#include <algorithm>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/pool/pool_alloc.hpp>

namespace User {

class PLSQLLexer;
class PLSQLParser;

#ifdef USEBOOST
template<class ImplTraits>
class BoostTraits: public antlr3::CustomTraitsBase<ImplTraits>
{
public:
	typedef std::basic_string<char, std::char_traits<char>, boost::fast_pool_allocator<char> > StringType;
	typedef std::basic_stringstream<char, std::char_traits<char>, boost::fast_pool_allocator<char> > StringStreamType;
	static void displayRecognitionError( const StringType& str ) {  printf("%s", str.c_str() ); }
	///static void displayRecognitionError( const std::string& str ) {  printf("%s", str.c_str() ); }	
};
typedef antlr3::Traits<PLSQLLexer, PLSQLParser, BoostTraits> PLSQLTraits;
#else
typedef antlr3::Traits<PLSQLLexer, PLSQLParser, BoostTraits> PLSQLTraits;
#endif
	
/*
//code for overriding
template<class ImplTraits>
class UserTraits : public antlr3::CustomTraitsBase<ImplTraits>
{
public:
	//for using the token stream which deleted the tokens, once it is reduced to a rule
	//but it leaves the start and stop tokens. So they can be accessed as usual
	static const bool TOKENS_ACCESSED_FROM_OWNING_RULE = true;

	//Similarly, if you want to override the nextToken function. write a class that 
	//derives from antlr3::TokenSource and override the nextToken function. But name the class
	//as TokenSourceType
	class TokenSourceType : public antlr3::TokenSource<ImplTraits>
	{
		public:
			TokenType*  nextToken()
			{
			   ..your version...
			}
	};
};

typedef antlr3::Traits< JavaLexer, JavaParser, UserTraits > PLSQLTraits;
*/

typedef PLSQLTraits PLSQLLexerTraits;
typedef PLSQLTraits PLSQL_DMLParser_PLSQLCommonsTraits;
typedef PLSQLTraits PLSQL_DMLParser_PLSQLKeysTraits;
typedef PLSQLTraits PLSQL_DMLParserTraits;

typedef PLSQLTraits PLSQLParserTraits;
typedef PLSQLTraits PLSQLParser_PLSQLKeysTraits;
typedef PLSQLTraits PLSQLParser_PLSQLCommonsTraits;
typedef PLSQLTraits PLSQLParser_PLSQL_DMLParserTraits;
typedef PLSQLTraits PLSQLParser_SQLPLUSParserTraits;	
typedef PLSQLTraits PLSQLParser_PLSQL_DMLParser_PLSQLKeysTraits;
typedef PLSQLTraits PLSQLParser_PLSQL_DMLParser_PLSQLCommonsTraits;
	
	template<class CommonTokenType>
	inline bool isTableAlias(CommonTokenType *LT1, CommonTokenType *LT2) {
		PLSQLTraits::StringType lt1 = LT1->getText();
		PLSQLTraits::StringType lt2 = "";
		std::transform(lt1.begin(), lt1.end(), lt1.begin(), ::toupper);
        
		if ( LT2 && LT2->getText() != ""){
			lt2 = LT2->getText();
			std::transform(lt2.begin(), lt2.end(), lt2.begin(), ::toupper); 
		}
		
		if ( (lt1 == "PARTITION" && lt2 == "BY")
		     || lt1 == "CROSS"
		     || lt1 == "NATURAL"
		     || lt1 == "INNER"
		     || lt1 == "JOIN"
		     || ( (lt1 == "FULL" || lt1 == "LEFT" || lt1 == "RIGHT") && (lt2 == "OUTER" || lt2 == "JOIN" ) )
			)
		{
			return false;
		}
		return true;
	}

	template<class StringType>
	inline bool isStandardPredictionFunction(StringType const& originalFunctionName) {
		StringType  functionName = originalFunctionName;
		std::transform(functionName.begin(), functionName.end(), functionName.begin(), ::toupper);
		
		if ( functionName == "PREDICTION"
		     || functionName == "PREDICTION_BOUNDS"
		     || functionName == "PREDICTION_COST"
		     || functionName == "PREDICTION_DETAILS"
		     || functionName == "PREDICTION_PROBABILITY"
		     || functionName == "PREDICTION_SET")
		{
			return true;
		}
		return false;
	}

	template<class StringType>     
	inline bool enablesWithinOrOverClause(StringType const& originalFunctionName) {
		StringType functionName = originalFunctionName;
		std::transform(functionName.begin(), functionName.end(), functionName.begin(), ::toupper);
	     
		if ( functionName == "CUME_DIST"
		     || functionName == "DENSE_RANK"
		     || functionName == "LISTAGG"
		     || functionName == "PERCENT_RANK"
		     || functionName == "PERCENTILE_CONT"
		     || functionName == "PERCENTILE_DISC"
		     || functionName == "RANK")
		{
			return true;
		}
		return false;
	}

	template<class StringType>          
	inline bool enablesUsingClause(StringType const& originalFunctionName) {
		static const StringType cluster("CLUSTER_");
		static const StringType feature("FEATURE_");
		
		// StringType functionName = originalFunctionName;
		// std::transform(functionName.begin(), functionName.end(), functionName.begin(), ::toupper);
		
		if ( boost::algorithm::istarts_with(originalFunctionName, cluster) || boost::algorithm::istarts_with(originalFunctionName, feature))
		{
			return true;
		}
		return false;
	}

	template<class StringType>     
	inline bool enablesOverClause(StringType const& originalFunctionName) {
		static const StringType regr("REGR_");
		static const StringType stddev("STDDEV");
		static const StringType var("VAR_");
		static const StringType covar("COVAR_");
		
		StringType functionName = originalFunctionName;
		std::transform(functionName.begin(), functionName.end(), functionName.begin(), ::toupper);		
		
		if ( functionName == "AVG"
		     || functionName == "CORR"
		     || functionName == "LAG"
		     || functionName == "LEAD"
		     || functionName == "MAX"
		     || functionName == "MEDIAN"
		     || functionName == "MIN"
		     || functionName == "NTILE"
		     || functionName == "RATIO_TO_REPORT"
		     || functionName == "ROW_NUMBER"
		     || functionName == "SUM"
		     || functionName == "VARIANCE"
		     || boost::algorithm::istarts_with(originalFunctionName, regr)
		     || boost::algorithm::istarts_with(originalFunctionName, stddev)
		     || boost::algorithm::istarts_with(originalFunctionName, var)
		     || boost::algorithm::istarts_with(originalFunctionName, covar)
			)
		{
			return true;
		}
		return false;
	}

	template<class StringType>
	inline bool equalsIgnoreCase(StringType const& s1, const char* s2)        
	{
		// return !strcasecmp(s1.c_str(), s2);
		StringType s1U(s1);
		std::transform(s1U.begin(), s1U.end(), s1U.begin(), ::toupper);
		return s1U == s2;
		// return iequals(s1, s2);
	}
}

#endif
