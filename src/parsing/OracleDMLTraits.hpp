#ifndef	_ORACLE_DML_TRAITS_H
#define	_ORACLE_DML_TRAITS_H

#include <antlr3.hpp>
#include <algorithm>
#include <memory>
#include <queue>
#include <boost/algorithm/string/predicate.hpp>

#include "TokenAttr.hpp"

using namespace SQLLexer;

namespace Antlr3BackendImpl {
	class OracleDMLLexer;
	class OracleDML;
	
	class EmptyParser {};

	//code for overriding
	template<class ImplTraits>
	class UserTraits : public antlr3::CustomTraitsBase<ImplTraits>
	{
	public:
		//for using the token stream which deleted the tokens, once it is reduced to a rule
		//but it leaves the start and stop tokens. So they can be accessed as usual
		//static const bool TOKENS_ACCESSED_FROM_OWNING_RULE = true;

		static void displayRecognitionError(const std::string& str)
		{
			antlr3::CustomTraitsBase<ImplTraits>::displayRecognitionError(str);
		}
	
		template<class StreamType>
		class RecognizerType : public antlr3::BaseRecognizer<ImplTraits, StreamType>
		{
		public:
			typedef antlr3::BaseRecognizer<ImplTraits, StreamType> BaseType;
			typedef typename antlr3::BaseRecognizer<ImplTraits, StreamType>::RecognizerSharedStateType RecognizerSharedStateType;
			RecognizerType(ANTLR_UINT32 sizeHint, RecognizerSharedStateType* state)
				: BaseType(sizeHint, state)
			{}

			ANTLR_INLINE void reportError()
			{
				BaseType::reportError();
			}

			// This recover method is probably only called from Parser
			// ANTLR_INLINE void recover()
			// {
			// 	BaseType::recover();
			// }
		};

		class BaseLexerType : public antlr3::Lexer<ImplTraits>
		{
		public:
			typedef antlr3::Lexer<ImplTraits> super;
			typedef typename antlr3::Lexer<ImplTraits>::RecognizerSharedStateType RecognizerSharedStateType;
			typedef typename antlr3::Lexer<ImplTraits>::InputStreamType InputStreamType;

			BaseLexerType(ANTLR_UINT32 sizeHint, RecognizerSharedStateType* state)
				: super(sizeHint, state)
			{}
			BaseLexerType(ANTLR_UINT32 sizeHint, InputStreamType* input, RecognizerSharedStateType* state)
				: super(sizeHint, input, state)
			{}
		
			ANTLR_INLINE void recover()
			{
				super::recover();
			}
		};

		struct ToraTokenUserDataType
		{
			ToraTokenUserDataType() : identifierClass(0), usageType(0) {};
			int identifierClass, usageType, toraTokenType;
		};

		class ToraToken : public antlr3::CommonToken<ImplTraits>
		{
			typedef antlr3::CommonToken<ImplTraits> super;
			typedef typename antlr3::CommonToken<ImplTraits>::TOKEN_TYPE TOKEN_TYPE;
			typedef typename super::StringType StringType;
		public:
			ToraToken() : m_block_context(BlkCtx::NONE), super() {};
			ToraToken( ANTLR_UINT32 type) : super(type), m_block_context(BlkCtx::NONE)  {};
			ToraToken( TOKEN_TYPE type) : super(type), m_block_context(BlkCtx::NONE)  {};
			ToraToken( const ToraToken& ctoken ) : super(ctoken), m_block_context(ctoken.m_block_context) {};

			ToraToken& operator=( const ToraToken& other ) { super::operator=(other); m_block_context = other.m_block_context; return *this; };

			StringType toString() const
			{
				StringType m_txt;				
				m_txt = super::getText();
				//m_txt += "[" + std::to_string(super::UserData.identifierClass) + "]";
				return m_txt;
			}
			
			void setBlockContext(BlkCtx::BlockContextEnum bc) { m_block_context = bc; }
			BlkCtx::BlockContextEnum getBlockContext() const { return m_block_context; }

		private:
			BlkCtx::BlockContextEnum m_block_context;
		};

		typedef ToraToken CommonTokenType;
		typedef ToraTokenUserDataType TokenUserDataType;
		typedef ToraTokenUserDataType TreeUserDataType;

		//Similarly, if you want to override the nextToken function. write a class that
		//derives from antlr3::TokenSource and override the nextToken function. But name the class
		//as TokenSourceType
		class TokenSourceType : public antlr3::TokenSource<ImplTraits>
		{
			typedef typename antlr3::TokenSource<ImplTraits> super;
			typedef typename antlr3::TokenSource<ImplTraits>::TokenType TokenType;
		public:
			TokenSourceType()
				: super()
				, jumpToken(NULL)
			{}

			ANTLR_INLINE TokenType*  nextToken()
			{
				TokenType* retval = NULL;
				if (!tokenBuffer.empty()) // previous call returned some token(s) in a buffer
				{
					retval = tokenBuffer.front();
					tokenBuffer.pop();
					usedTokens.push(retval);
					return retval;
				}

				if (jumpToken) // previous call returned more than one token
				{
					retval = jumpToken;
					jumpToken = NULL;
					return retval;
				}

				retval = super::nextToken();
				if (!tokenBuffer.empty())  // call to mTokens returned more than one token
				{
					jumpToken = retval;    // try to "flush" buffer first;
					retval = tokenBuffer.front();
					tokenBuffer.pop();
					usedTokens.push(retval);
				}
				return retval;
			}

			ANTLR_INLINE void recover()
			{
				super::recover();
			}

			//ANTLR_INLINE void enqueueToken(TokenType *t)
			//{
			//	tokenBuffer.push(t);
			//}

			ANTLR_INLINE void enqueueToken(std::unique_ptr<TokenType> &t)
			{
				tokenBuffer.push(t.release());
			}

			~TokenSourceType()
			{
				while(!usedTokens.empty())
				{
					TokenType* token = usedTokens.front();
					delete token;
					usedTokens.pop();
				}
			}
		private:
			// buffer (queue) to hold the emit()'d tokens
			std::queue<TokenType*> tokenBuffer;
			std::queue<TokenType*> usedTokens; // "virtually" emitted tokens are kept and freed here
			TokenType* jumpToken;
		};	
	};
  
	typedef antlr3::Traits<OracleDMLLexer, OracleDML, UserTraits> OracleSQLParserTraits;
	typedef OracleSQLParserTraits OracleDMLLexerTraits;
	typedef OracleSQLParserTraits OracleDMLTraits;
	typedef OracleSQLParserTraits OracleDML_OracleDMLCommonsTraits;
	typedef OracleSQLParserTraits OracleDML_OracleDMLKeysTraits;
  
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

		auto lt1 = LT1->getText();
		std::string lt2;
        
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

	template<class StringType>     
	inline bool enablesOverClause(StringType const& originalFunctionName) {
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

	template<class StringType>
	inline bool equalsIgnoreCase(StringType const& s1, const char* s2)        
	{
		// return !strcasecmp(s1.c_str(), s2);
		// StringType s1U(s1);
		// std::transform(s1U.begin(), s1U.end(), s1U.begin(), ::toupper);
		// return s1U == s2;
		StringType ST2(s2);
		return boost::iequals(s1, ST2);
	}

	template<class StringType>  
	inline StringType toUpper(StringType const& s)
	{
		StringType sU(s);
		std::transform(sU.begin(), sU.end(), sU.begin(), ::toupper);
		return sU;
	}

}

#endif
