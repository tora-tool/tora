#ifndef	_T_GUI_TRAITS_H
#define	_T_GUI_TRAITS_H

// First thing we always do is include the ANTLR3 generated files, which
// will automatically include the antlr3 runtime header files.
#include <antlr3.hpp>
#include <algorithm>
#include <queue>
#include <memory>
#include <boost/algorithm/string/predicate.hpp>

#include "TokenAttr.hpp"

namespace Antlr3GuiImpl
{
    class PLSQLGuiLexer;
    class MySQLGuiLexer;
    class OraclePLSQLLexer;
    class OraclePLSQLParser;
};

using namespace SQLLexer;

namespace Antlr3GuiImpl
{
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
                        RecognizerSharedStateType* state = this->get_rec()->get_state();
                        /** Note: This class in not aware of ANTLR generated enums - probably (so CommonTokenType::TOKEN_INVALID is not accessible)
                         *  see: OracleGuiLexer::LA.
                         *  So instead of calling: state->set_type(85 "TOKEN_INVALID"); We set token type to MIN_TOKEN_TYPE which is not used anywhere else
                         */
                        state->set_type(ImplTraits::CommonTokenType::MIN_TOKEN_TYPE);
                        ImplTraits::CommonTokenType* t = super::emit();
                        //super::recover();
                    }
            };

            class ToraToken : public antlr3::CommonToken<ImplTraits>
            {
                    typedef antlr3::CommonToken<ImplTraits> super;
                    typedef typename antlr3::CommonToken<ImplTraits>::TOKEN_TYPE TOKEN_TYPE;
                public:
                    ToraToken() : m_block_context(BlkCtx::NONE), super() {};
                    ToraToken( ANTLR_UINT32 type) : super(type), m_block_context(BlkCtx::NONE)  {};
                    ToraToken( TOKEN_TYPE type) : super(type), m_block_context(BlkCtx::NONE)  {};
                    ToraToken( const ToraToken& ctoken ) : super(ctoken), m_block_context(ctoken.m_block_context) {};

                    ToraToken& operator=( const ToraToken& other )
                    {
                        super::operator=(other);
                        m_block_context = other.m_block_context;
                        return *this;
                    };

                    void setBlockContext(BlkCtx::BlockContextEnum bc)
                    {
                        m_block_context = bc;
                    }
                    BlkCtx::BlockContextEnum getBlockContext() const
                    {
                        return m_block_context;
                    }

                private:
                    BlkCtx::BlockContextEnum m_block_context;
            };

            typedef ToraToken CommonTokenType;

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

                    ANTLR_INLINE void enqueueToken(TokenType *t)
                    {
                        tokenBuffer.push(t);
                    }

                    ANTLR_INLINE void enqueueToken(std::auto_ptr< TokenType> &t)
                    {
                        tokenBuffer.push(t.release());
                    }

                    ANTLR_INLINE void enqueueToken(std::unique_ptr< TokenType> &t)
                    {
                        tokenBuffer.push(t.release());
                    }

                    ~TokenSourceType()
                    {
                        while (!usedTokens.empty())
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

    class EmptyParser {};

    typedef antlr3::Traits<PLSQLGuiLexer, EmptyParser, UserTraits>               PLSQLGuiLexerTraits;
    typedef antlr3::Traits<MySQLGuiLexer, EmptyParser, UserTraits>               MySQLGuiLexerTraits;

};

#endif
