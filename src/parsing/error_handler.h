#ifndef __ERROR_HANDLER_ANT__
#define __ERROR_HANDLER_ANT__

#include "antlr3baserecognizer.h"

#ifdef __cplusplus
extern "C" {
#endif

    void myDisplayRecognitionLexerError (pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames);
    void myDisplayRecognitionParserError (pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames);
    void * myGetMissingSymbol (pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_INT_STREAM istream, pANTLR3_EXCEPTION e,
                               ANTLR3_UINT32 expectedTokenType, pANTLR3_BITSET_LIST follow);
    void * myRecoverFromMismatchedToken  (pANTLR3_BASE_RECOGNIZER recognizer, ANTLR3_UINT32 ttype, pANTLR3_BITSET_LIST follow);

#ifdef __cplusplus
}
#endif

#endif // __ERROR_HANDLER_ANT__
