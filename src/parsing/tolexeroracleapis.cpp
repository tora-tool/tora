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

// Return the call tip for a function.
QStringList toLexerOracleAPIs::callTips(const QStringList &context, int commas,
        QsciScintilla::CallTipsStyle style,
        QList<int> &shifts)
{
    //QStringList wseps = lexer()->autoCompletionWordSeparators();
    QStringList cts;
    return cts;
}
