#ifndef __TOLEXERORACLEAPIS__
#define __TOLEXERORACLEAPIS__

#include <Qsci/qsciabstractapis.h>

class toLexerOracle;

//! \brief The QsciAbstractAPIs class represents the interface to the textual
//! API information used in call tips and for auto-completion.  A sub-class
//! will provide the actual implementation of the interface.
//!
//! API information is specific to a particular language lexer but can be
//! shared by multiple instances of the lexer.
class toLexerOracleAPIs : public QsciAbstractAPIs
{
        Q_OBJECT

    public:
        //! Constructs a QsciAbstractAPIs instance attached to lexer \a lexer.  \a
        //! lexer becomes the instance's parent object although the instance can
        //! also be subsequently attached to other lexers.
        toLexerOracleAPIs(QsciLexer *lexer);

        //! Destroy the QsciAbstractAPIs instance.
        virtual ~toLexerOracleAPIs();

        //! Return the lexer that the instance is attached to.
        toLexerOracle *lexer() const;

        //! Update the list \a list with API entries derived from \a context.  \a
        //! context is the list of words in the text preceding the cursor position.
        //! The characters that make up a word and the characters that separate
        //! words are defined by the lexer.  The last word is a partial word and
        //! may be empty if the user has just entered a word separator.
        virtual void updateAutoCompletionList(const QStringList &context,
                                              QStringList &list);

        QStringList callTips(const QStringList &context, int commas,
                             QsciScintilla::CallTipsStyle style,
                             QList<int> &shifts);
    private:

        toLexerOracleAPIs(const toLexerOracleAPIs &);
        toLexerOracleAPIs &operator=(const toLexerOracleAPIs &);
};

#endif
