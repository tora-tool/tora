#ifndef _ANTLR3_TRAITS_HPP
#define _ANTLR3_TRAITS_HPP

#include	"antlr3defs.hpp"


ANTLR_BEGIN_NAMESPACE()
	
//Users implementing overrides should inherit from this
template<class ImplTraits>
class CustomTraitsBase
{
public:
	typedef Empty AllocPolicyType;
	typedef Empty StringType;
	typedef Empty StringStreamType;
	typedef Empty StreamDataType;
	typedef Empty Endianness;

	//collections
	typedef Empty BitsetType;
	typedef Empty BitsetListType;

	typedef Empty InputStreamType;

	template<class StreamType>
	class IntStreamType : public Empty
	{
	public:
		typedef Empty BaseType;
	};

	typedef Empty LexStateType;

	typedef Empty CommonTokenType;
	typedef Empty TokenIntStreamType;

	typedef Empty TokenStreamType;
	typedef Empty TreeNodeStreamType;


	typedef Empty DebugEventListenerType;
	template<class StreamType>
	class RecognizerSharedStateType : public Empty
	{
	public:
		typedef Empty BaseType;
	};

	template<class StreamType>
	class RecognizerType : public Empty
	{
	public:
		typedef Empty BaseType;
	};
	
	typedef Empty TreeType;
	typedef Empty TreeAdaptorType;
	
	template<class StreamType>
	class ExceptionBaseType : public Empty
	{
	public:
		typedef Empty BaseType;
	};

	//this should be overridden with generated lexer
	typedef Empty BaseLexerType;
	
	typedef Empty TokenSourceType;
	typedef Empty BaseParserType;//this should be overridden with generated lexer
	typedef Empty BaseTreeParserType;
	
	template<class StreamType>
	class RewriteStreamType : public Empty
	{
	public:
		typedef Empty BaseType;
	};

	typedef Empty  RuleReturnValueType;
	
	//If we want to change the way tokens are stored
	static const bool TOKENS_ACCESSED_FROM_OWNING_RULE = false;
	static const int  TOKEN_FILL_BUFFER_INCREMENT = 100; //used only if the above val is true

	static void displayRecognitionError( const std::string& str ) {  printf("%s", str.c_str() ); }
};

template<class A, class B>
class TraitsSelector
{
public:
	typedef A selected;
};

template<class B>
class TraitsSelector<Empty, B>
{
public:
	typedef B selected;
};

template<class A, class B, class C>
class TraitsOneArgSelector
{
public:
	typedef A selected;
};

template<class A, class B>
class TraitsOneArgSelector<A,B,Empty>
{
public:
	typedef B selected;
};

template<bool v, class A, class B>
class BoolSelector
{
public:
	typedef A selected;
};

template<class A, class B>
class BoolSelector<false, A, B>
{
public:
	typedef B selected;
};

template< template<class ImplTraits> class UserTraits >
class TraitsBase
{
public:
	typedef TraitsBase  TraitsType;
	
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::AllocPolicyType, DefaultAllocPolicy >::selected  AllocPolicyType;

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::StringType, 
									std::string >::selected StringType;
	
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::StringStreamType, 
		                             std::stringstream >::selected StringStreamType;

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::StreamDataType, 
		                             ANTLR_UINT8 >::selected StreamDataType;
	
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::Endianness, 
									 RESOLVE_ENDIAN_AT_RUNTIME >::selected Endianness;

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::BitsetType, 
		                             Bitset<TraitsType> >::selected BitsetType;
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::BitsetListType, 
		                             BitsetList<TraitsType> >::selected BitsetListType;

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::InputStreamType, 
		                              InputStream<TraitsType> >::selected InputStreamType;

	template<class SuperType>
	class IntStreamType 
		: public TraitsOneArgSelector< 
					typename UserTraits<TraitsType>::template IntStreamType<SuperType>, 
		            IntStream<TraitsType, SuperType>,
					typename UserTraits<TraitsType>::template IntStreamType<SuperType>::BaseType
								>::selected  
	{ };
	
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::LexStateType, 
		                             LexState<TraitsType> >::selected LexStateType;

	static const bool TOKENS_ACCESSED_FROM_OWNING_RULE = UserTraits<TraitsType>::TOKENS_ACCESSED_FROM_OWNING_RULE;
	static const int  TOKEN_FILL_BUFFER_INCREMENT = UserTraits<TraitsType>::TOKEN_FILL_BUFFER_INCREMENT; //used only if the above val is true

	static void displayRecognitionError( const StringType& str ) { UserTraits<TraitsType>::displayRecognitionError(str);  }
};

template<
	     class LxrType, 
         class PsrType, 
		 template<class ImplTraits> class UserTraits = CustomTraitsBase, 
		 class TreePsrType = antlr3::Empty
         >
class Traits  : public TraitsBase<UserTraits>
{
public:
	typedef Traits TraitsType;
	typedef TraitsBase<UserTraits> BaseTraitsType;	

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::CommonTokenType, 
									 CommonToken<TraitsType> >::selected CommonTokenType;
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::TokenIntStreamType, 
		                             TokenIntStream<TraitsType> >::selected TokenIntStreamType;
	
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::TokenStreamType, 
									CommonTokenStream<TraitsType> >::selected TokenStreamType;
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::TreeNodeStreamType, 
		                            CommonTreeNodeStream<TraitsType> >::selected TreeNodeStreamType;
	
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::DebugEventListenerType, 
		                            DebugEventListener<TraitsType> >::selected DebugEventListenerType;
		
	template<class StreamType>
	class  RecognizerSharedStateType 
		          : public TraitsOneArgSelector< 
							typename UserTraits<TraitsType>::template RecognizerSharedStateType<StreamType>, 
							RecognizerSharedState<TraitsType, StreamType>,
							typename UserTraits<TraitsType>::template RecognizerSharedStateType<StreamType>::BaseType
										>::selected 
	{};

	template<class StreamType>
	class  RecognizerType 
				   : public TraitsOneArgSelector< 
				             typename UserTraits<TraitsType>::template RecognizerType<StreamType>, 
							 BaseRecognizer<TraitsType, StreamType>,
							 typename UserTraits<TraitsType>::template RecognizerType<StreamType>::BaseType
									>::selected 
	{
	public:
		typedef typename TraitsOneArgSelector< 
							typename UserTraits<TraitsType>::template RecognizerType<StreamType>, 
							BaseRecognizer<TraitsType, StreamType>,
							typename UserTraits<TraitsType>::template RecognizerType<StreamType>::BaseType
										>::selected  BaseType;
		typedef typename BaseType::RecognizerSharedStateType RecognizerSharedStateType;

	public:
		RecognizerType(ANTLR_UINT32 sizeHint, RecognizerSharedStateType* state)
			: BaseType( sizeHint, state )
		{
		}
	};

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::TreeType, 
										CommonTree<TraitsType> >::selected TreeType;
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::TreeAdaptorType, 
										CommonTreeAdaptor<TraitsType> >::selected TreeAdaptorType;
		
	template<class StreamType>
	class ExceptionBaseType : public TraitsOneArgSelector< 
								typename UserTraits<TraitsType>::template ExceptionBaseType<StreamType>, 
								ANTLR_ExceptionBase<TraitsType, StreamType>, 
								typename UserTraits<TraitsType>::template ExceptionBaseType<StreamType>::BaseType
													>::selected 
	{
	public:
		typedef typename TraitsOneArgSelector< 
			                    typename UserTraits<TraitsType>::template ExceptionBaseType<StreamType>, 
								ANTLR_ExceptionBase<TraitsType, StreamType>,
								typename UserTraits<TraitsType>::template ExceptionBaseType<StreamType>::BaseType
							>::selected BaseType;
	
	protected:
		ExceptionBaseType( const typename BaseTraitsType::StringType& message )
			:BaseType(message)
		{
		}
	};

	//this should be overridden with generated lexer
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::BaseLexerType, 
											Lexer<TraitsType> >::selected BaseLexerType;
	typedef LxrType LexerType;

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::TokenSourceType, 
											TokenSource<TraitsType> >::selected TokenSourceType;
	typedef typename TraitsSelector< typename UserTraits<TraitsType>::BaseParserType, 
											Parser<TraitsType> >::selected BaseParserType;
	
	typedef PsrType ParserType;

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::BaseTreeParserType, 
											 TreeParser<TraitsType> >::selected BaseTreeParserType;
	typedef TreePsrType TreeParserType;

	template<class SuperType>
	class RewriteStreamType : public TraitsOneArgSelector< 
								typename UserTraits<TraitsType>::template RewriteStreamType<SuperType>, 
								RewriteRuleElementStream<TraitsType, SuperType>,
								typename UserTraits<TraitsType>::template RewriteStreamType<SuperType>::BaseType
													>::selected 
	{
	public:
		typedef typename TraitsOneArgSelector< 
				typename UserTraits<TraitsType>::template RewriteStreamType<SuperType>, 
				RewriteRuleElementStream<TraitsType, SuperType>, 
				typename UserTraits<TraitsType>::template RewriteStreamType<SuperType>::BaseType
						>::selected BaseType;

		typedef typename SuperType::StreamType StreamType;
		typedef typename BaseType::RecognizerType Recognizer_Type;
		typedef typename BaseType::TokenType TokenType;
		typedef typename BaseType::ElementsType ElementsType;			

	public:
		RewriteStreamType(TreeAdaptorType* adaptor = NULL, Recognizer_Type* rec=NULL, ANTLR_UINT8* description = NULL)
			:BaseType(adaptor, rec, description)
		{
		}
		RewriteStreamType(TreeAdaptorType* adaptor, Recognizer_Type* rec, ANTLR_UINT8* description, TokenType* oneElement)
			:BaseType(adaptor, rec, description, oneElement)
		{
		}
		RewriteStreamType(TreeAdaptorType* adaptor, Recognizer_Type* rec, ANTLR_UINT8* description, const ElementsType& elements)
			:BaseType(adaptor, rec, description, elements)
		{
		}
	};

	typedef typename TraitsSelector< typename UserTraits<TraitsType>::RuleReturnValueType, 
										typename BoolSelector< TraitsType::TOKENS_ACCESSED_FROM_OWNING_RULE, 
														RuleReturnValue_1<TraitsType>, RuleReturnValue<TraitsType> >::selected
								   >::selected RuleReturnValueType;
};


ANTLR_END_NAMESPACE()

#endif //_ANTLR3_TRAITS_HPP
