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
#include <memory>
#include <boost/algorithm/string/predicate.hpp>

namespace User {
	class PLSQLLexer;
	class PLSQLParser;
};

#if defined(USEBOOST)
#  include "UserTraitsBoost.hpp"
#elif defined(USETBB)
#  include "UserTraitsTBB.hpp"  
#else // STL
namespace User {
	typedef antlr3::Traits<PLSQLLexer, PLSQLParser> PLSQLTraits;
};
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

namespace User
{
	typedef PLSQLTraits PLSQLLexerTraits;
	typedef PLSQLTraits PLSQLParserTraits;
	typedef PLSQLTraits PLSQLParser_PLSQLKeysTraits;
	typedef PLSQLTraits PLSQLParser_PLSQLCommonsTraits;
	typedef PLSQLTraits PLSQLParser_PLSQL_DMLParserTraits;
	typedef PLSQLTraits PLSQLParser_SQLPLUSParserTraits;	
	typedef PLSQLTraits PLSQLParser_PLSQL_DMLParser_PLSQLKeysTraits;
	typedef PLSQLTraits PLSQLParser_PLSQL_DMLParser_PLSQLCommonsTraits;
	
	template<class CommonTokenType>
	inline bool isTableAlias(CommonTokenType *LT1, CommonTokenType *LT2) {
		static const std::string wPARTITION("PARTITION");
		static const std::string wBY("BY");
		static const std::string wCROSS("CROSS");
		static const std::string wNATURAL("NATURAL");
		static const std::string wINNER("INNER");
		static const std::string wJOIN("JOIN");
		static const std::string wFULL("FULL");
		static const std::string wLEFT("LEFT");
		static const std::string wRIGHT("RIGHT");
		static const std::string wOUTER("OUTER");

		PLSQLTraits::StringType lt1 = LT1->getText();
		PLSQLTraits::StringType lt2 = "";
		//std::transform(lt1.begin(), lt1.end(), lt1.begin(), ::toupper);
        
		if ( LT2 && LT2->getText() != ""){
			lt2 = LT2->getText();
			std::transform(lt2.begin(), lt2.end(), lt2.begin(), ::toupper); 
		}
		
		if ( (boost::iequals(lt1, wPARTITION) && boost::iequals(lt2, wBY))
		     || boost::iequals(lt1, wCROSS)
		     || boost::iequals(lt1, wNATURAL)
		     || boost::iequals(lt1, wINNER)
		     || boost::iequals(lt1, wJOIN)
		     || ( ( boost::iequals(lt1, wFULL) || boost::iequals(lt1, wLEFT) || boost::iequals(lt1, wRIGHT) ) && ( boost::iequals(lt2, wOUTER) || boost::iequals(lt2, wJOIN) ) )
		     )
		{
			return false;
		}
		return true;
	}

	template<class StringType>
	inline bool isStandardPredictionFunction(StringType const& originalFunctionName) {
		static const std::string wPREDICTION("PREDICTION");
		static const std::string wPREDICTION_BOUNDS("PREDICTION_BOUNDS");
		static const std::string wPREDICTION_COST("PREDICTION_COST");
		static const std::string wPREDICTION_DETAILS("PREDICTION_DETAILS");
		static const std::string wPREDICTION_PROBABILITY("PREDICTION_PROBABILITY");
		static const std::string wPREDICTION_SET("PREDICTION_SET");		
		// StringType  functionName = originalFunctionName;
		// std::transform(functionName.begin(), functionName.end(), functionName.begin(), ::toupper);
		
		if ( boost::iequals(originalFunctionName, wPREDICTION)
		     || boost::iequals(originalFunctionName, wPREDICTION_BOUNDS)
		     || boost::iequals(originalFunctionName, wPREDICTION_COST)
		     || boost::iequals(originalFunctionName, wPREDICTION_DETAILS)
		     || boost::iequals(originalFunctionName, wPREDICTION_PROBABILITY)
		     || boost::iequals(originalFunctionName, wPREDICTION_SET)
			)
		{
			return true;
		}
		return false;
	}

	template<class StringType>     
	inline bool enablesWithinOrOverClause(StringType const& originalFunctionName) {
		static const std::string wCUME_DIST("CUME_DIST");
		static const std::string wDENSE_RANK("DENSE_RANK");
		static const std::string wLISTAGG("LISTAGG");
		static const std::string wPERCENT_RANK("PERCENT_RANK");
		static const std::string wPERCENTILE_CONT("PERCENTILE_CONT");
		static const std::string wPERCENTILE_DISC("PERCENTILE_DISC");
		static const std::string wRANK("RANK");		
		// StringType functionName = originalFunctionName;
		// std::transform(functionName.begin(), functionName.end(), functionName.begin(), ::toupper);
		
		if ( boost::iequals(originalFunctionName, wCUME_DIST)
		     || boost::iequals(originalFunctionName, wDENSE_RANK)
		     || boost::iequals(originalFunctionName, wLISTAGG)
		     || boost::iequals(originalFunctionName, wPERCENT_RANK)
		     || boost::iequals(originalFunctionName, wPERCENTILE_CONT)
		     || boost::iequals(originalFunctionName, wPERCENTILE_DISC)
		     || boost::iequals(originalFunctionName, wRANK)
			)
		{
			return true;
		}
		return false;
	}

	template<class StringType>          
	inline bool enablesUsingClause(StringType const& originalFunctionName) {
		static const std::string wCLUSTER("CLUSTER_");
		static const std::string wFEATURE("FEATURE_");		
		// StringType functionName = originalFunctionName;
		// std::transform(functionName.begin(), functionName.end(), functionName.begin(), ::toupper);
		
		if ( boost::algorithm::istarts_with(originalFunctionName, wCLUSTER)
		     || boost::algorithm::istarts_with(originalFunctionName, wFEATURE)
			)
		{
			return true;
		}
		return false;
	}

	//template<class StringType>     
	inline bool enablesOverClause(PLSQLTraits::StringType const& originalFunctionName) {
		static const std::string wREGR("REGR_");
		static const std::string wSTDDEV("STDDEV");
		static const std::string wVAR("VAR_");
		static const std::string wCOVAR("COVAR_");		
		static const std::string wAVG("AVG");
		static const std::string wCORR("CORR");
		static const std::string wLAG("LAG");
		static const std::string wLEAD("LEAD");
		static const std::string wMAX ("MAX");
		static const std::string wMEDIAN("MEDIAN");
		static const std::string wMIN("MIN");
		static const std::string wNTILE("NTILE");
		static const std::string wRATIO_TO_REPORT("RATIO_TO_REPORT");
		static const std::string wROW_NUMBER("ROW_NUMBER");
		static const std::string wSUM("SUM");
		static const std::string wVARIANCE("VARIANCE");       
		// StringType functionName = originalFunctionName;
		// std::transform(functionName.begin(), functionName.end(), functionName.begin(), ::toupper);		
		
		if ( boost::iequals(originalFunctionName, wAVG)
		     || boost::iequals(originalFunctionName, wCORR)
		     || boost::iequals(originalFunctionName, wLAG)
		     || boost::iequals(originalFunctionName, wLEAD)
		     || boost::iequals(originalFunctionName, wMAX)
		     || boost::iequals(originalFunctionName, wMEDIAN)
		     || boost::iequals(originalFunctionName, wMIN)
		     || boost::iequals(originalFunctionName, wNTILE)
		     || boost::iequals(originalFunctionName, wRATIO_TO_REPORT)
		     || boost::iequals(originalFunctionName, wROW_NUMBER)
		     || boost::iequals(originalFunctionName, wSUM)
		     || boost::iequals(originalFunctionName, wVARIANCE)
		     || boost::algorithm::istarts_with(originalFunctionName, wREGR)
		     || boost::algorithm::istarts_with(originalFunctionName, wSTDDEV)
		     || boost::algorithm::istarts_with(originalFunctionName, wVAR)
		     || boost::algorithm::istarts_with(originalFunctionName, wCOVAR)
			)
		{
			return true;
		}
		return false;
	}

	//template<class StringType>
	inline bool equalsIgnoreCase(PLSQLTraits::StringType const& s1, const char* s2)        
	{
		// return !strcasecmp(s1.c_str(), s2);
		// StringType s1U(s1);
		// std::transform(s1U.begin(), s1U.end(), s1U.begin(), ::toupper);
		// return s1U == s2;
		PLSQLTraits::StringType ST2(s2);
		return boost::iequals(s1, ST2);
	}
}

#endif
