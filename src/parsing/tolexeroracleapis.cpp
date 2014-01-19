#include "parsing/tolexeroracleapis.h"

toLexerOracleAPIs::toLexerOracleAPIs(QsciLexer *lexer) : QsciAbstractAPIs(lexer)
{
};

toLexerOracleAPIs::~toLexerOracleAPIs()
{
};

void toLexerOracleAPIs::updateAutoCompletionList(const QStringList &context, QStringList &list)
{
	list << "AAA" << "BBB";
}
