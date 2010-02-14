// This defines the interface to the QsciLexerPython class.
//
// Copyright (c) 2010 Riverbank Computing Limited <info@riverbankcomputing.com>
// 
// This file is part of QScintilla.
// 
// This file may be used under the terms of the GNU General Public
// License versions 2.0 or 3.0 as published by the Free Software
// Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
// included in the packaging of this file.  Alternatively you may (at
// your option) use any later version of the GNU General Public
// License if such license has been publicly approved by Riverbank
// Computing Limited (or its successors, if any) and the KDE Free Qt
// Foundation. In addition, as a special exception, Riverbank gives you
// certain additional rights. These rights are described in the Riverbank
// GPL Exception version 1.1, which can be found in the file
// GPL_EXCEPTION.txt in this package.
// 
// Please review the following information to ensure GNU General
// Public Licensing requirements will be met:
// http://trolltech.com/products/qt/licenses/licensing/opensource/. If
// you are unsure which license is appropriate for your use, please
// review the following information:
// http://trolltech.com/products/qt/licenses/licensing/licensingoverview
// or contact the sales department at sales@riverbankcomputing.com.
// 
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


#ifndef QSCILEXERPYTHON_H
#define QSCILEXERPYTHON_H

#ifdef __APPLE__
extern "C++" {
#endif

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexer.h>
#include "Qsci/qsciscintillabase.h"


//! \brief The QsciLexerPython class encapsulates the Scintilla Python lexer.
class QSCINTILLA_EXPORT QsciLexerPython : public QsciLexer
{
    Q_OBJECT

public:
    //! This enum defines the meanings of the different styles used by the
    //! Python lexer.
    enum {
        //! The default.
        Default = 0,

        //! A comment.
        Comment = 1,

        //! A number.
        Number = 2,

        //! A double-quoted string.
        DoubleQuotedString = 3,

        //! A single-quoted string.
        SingleQuotedString = 4,

        //! A keyword.
        Keyword = 5,

        //! A triple single-quoted string.
        TripleSingleQuotedString = 6,

        //! A triple double-quoted string.
        TripleDoubleQuotedString = 7,

        //! The name of a class.
        ClassName = 8,

        //! The name of a function or method.
        FunctionMethodName = 9,

        //! An operator.
        Operator = 10,

        //! An identifier
        Identifier = 11,

        //! A comment block.
        CommentBlock = 12,

        //! The end of a line where a string is not closed.
        UnclosedString = 13,

        //! A highlighted identifier.  These are defined by keyword set
        //! 2.  Reimplement keywords() to define keyword set 2.
        HighlightedIdentifier = 14,

        //! A decorator.
        Decorator = 15
    };

    //! This enum defines the different conditions that can cause
    //! indentations to be displayed as being bad.
    enum IndentationWarning {
        //! Bad indentation is not displayed differently.
        NoWarning = 0,

        //! The indentation is inconsistent when compared to the
        //! previous line, ie. it is made up of a different combination
        //! of tabs and/or spaces.
        Inconsistent = 1,

        //! The indentation is made up of spaces followed by tabs.
        TabsAfterSpaces = 2,

        //! The indentation contains spaces.
        Spaces = 3,

        //! The indentation contains tabs.
        Tabs = 4
    };

    //! Construct a QsciLexerPython with parent \a parent.  \a parent is
    //! typically the QsciScintilla instance.
    QsciLexerPython(QObject *parent = 0);

    //! Destroys the QsciLexerPython instance.
    virtual ~QsciLexerPython();

    //! Returns the name of the language.
    const char *language() const;

    //! Returns the name of the lexer.  Some lexers support a number of
    //! languages.
    const char *lexer() const;

    //! \internal Returns the character sequences that can separate
    //! auto-completion words.
    QStringList autoCompletionWordSeparators() const;

    //! \internal Returns the number of lines prior to the current one when
    //! determining the scope of a block when auto-indenting.
    int blockLookback() const;

    //! \internal Returns a space separated list of words or characters in
    //! a particular style that define the start of a block for
    //! auto-indentation.  The styles is returned via \a style.
    const char *blockStart(int *style = 0) const;

    //! \internal Returns the style used for braces for brace matching.
    int braceStyle() const;

    //! Returns the foreground colour of the text for style number \a style.
    //!
    //! \sa defaultPaper()
    QColor defaultColor(int style) const;

    //! Returns the end-of-line fill for style number \a style.
    bool defaultEolFill(int style) const;

    //! Returns the font for style number \a style.
    QFont defaultFont(int style) const;

    //! Returns the background colour of the text for style number \a style.
    //!
    //! \sa defaultColor()
    QColor defaultPaper(int style) const;

    //! \internal Returns the view used for indentation guides.
    virtual int indentationGuideView() const;

    //! Returns the set of keywords for the keyword set \a set recognised
    //! by the lexer as a space separated string.
    const char *keywords(int set) const;

    //! Returns the descriptive name for style number \a style.  If the
    //! style is invalid for this language then an empty QString is returned.
    //! This is intended to be used in user preference dialogs.
    QString description(int style) const;

    //! Causes all properties to be refreshed by emitting the
    //! propertyChanged() signal as required.
    void refreshProperties();

    //! Returns true if indented comment blocks can be folded.
    //!
    //! \sa setFoldComments()
    bool foldComments() const;

    //! Returns true if triple quoted strings can be folded.
    //!
    //! \sa setFoldQuotes()
    bool foldQuotes() const;

    //! Returns the condition that will cause bad indentations to be
    //! displayed.
    //!
    //! \sa setIndentationWarning()
    QsciLexerPython::IndentationWarning indentationWarning() const;

    //! If \a allowed is true then Python v2 unicode string literals (e.g.
    //! u"utf8") are allowed.  The default is true.
    //!
    //! \sa v2UnicodeAllowed()
    void setV2UnicodeAllowed(bool allowed);

    //! Returns true if Python v2 unicode string literals (e.g. u"utf8") are
    //! allowed.
    //!
    //! \sa setV2UnicodeAllowed()
    bool v2UnicodeAllowed() const;

    //! If \a allowed is true then Python v3 binary and octal literals (e.g.
    //! 0b1011, 0o712) are allowed.  The default is true.
    //!
    //! \sa v3BinaryOctalAllowed()
    void setV3BinaryOctalAllowed(bool allowed);

    //! Returns true if Python v3 binary and octal literals (e.g. 0b1011,
    //! 0o712) are allowed.
    //!
    //! \sa setV3BinaryOctalAllowed()
    bool v3BinaryOctalAllowed() const;

    //! If \a allowed is true then Python v3 bytes string literals (e.g.
    //! b"bytes") are allowed.  The default is true.
    //!
    //! \sa v3BytesAllowed()
    void setV3BytesAllowed(bool allowed);

    //! Returns true if Python v3 bytes string literals (e.g. b"bytes") are
    //! allowed.
    //!
    //! \sa setV3BytesAllowed()
    bool v3BytesAllowed() const;

public slots:
    //! If \a fold is true then indented comment blocks can be folded.  The
    //! default is false.
    //!
    //! \sa foldComments()
    virtual void setFoldComments(bool fold);

    //! If \a fold is true then triple quoted strings can be folded.  The
    //! default is false.
    //!
    //! \sa foldQuotes()
    virtual void setFoldQuotes(bool fold);

    //! Sets the condition that will cause bad indentations to be
    //! displayed.
    //!
    //! \sa indentationWarning()
    virtual void setIndentationWarning(QsciLexerPython::IndentationWarning warn);

protected:
    //! The lexer's properties are read from the settings \a qs.  \a prefix
    //! (which has a trailing '/') should be used as a prefix to the key of
    //! each setting.  true is returned if there is no error.
    //!
    bool readProperties(QSettings &qs,const QString &prefix);

    //! The lexer's properties are written to the settings \a qs.
    //! \a prefix (which has a trailing '/') should be used as a prefix to
    //! the key of each setting.  true is returned if there is no error.
    //!
    bool writeProperties(QSettings &qs,const QString &prefix) const;

private:
    void setCommentProp();
    void setQuotesProp();
    void setTabWhingeProp();
    void setV2UnicodeProp();
    void setV3BinaryOctalProp();
    void setV3BytesProp();

    bool fold_comments;
    bool fold_quotes;
    IndentationWarning indent_warn;
    bool v2_unicode;
    bool v3_binary_octal;
    bool v3_bytes;

    friend class QsciLexerHTML;

    static const char *keywordClass;

    QsciLexerPython(const QsciLexerPython &);
    QsciLexerPython &operator=(const QsciLexerPython &);
};

#ifdef __APPLE__
}
#endif

#endif
