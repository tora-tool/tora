// This module implements the QsciLexerPerl class.
//
// Copyright (c) 2007
// 	Phil Thompson <phil@river-bank.demon.co.uk>
// 
// This file is part of QScintilla.
// 
// This copy of QScintilla is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option) any
// later version.
// 
// QScintilla is supplied in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
// 
// You should have received a copy of the GNU General Public License along with
// QScintilla; see the file LICENSE.  If not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include "Qsci/qscilexerperl.h"

#include <qcolor.h>
#include <qfont.h>
#include <qsettings.h>


// The ctor.
QsciLexerPerl::QsciLexerPerl(QObject *parent)
    : QsciLexer(parent),
      fold_comments(false), fold_compact(true)
{
}


// The dtor.
QsciLexerPerl::~QsciLexerPerl()
{
}


// Returns the language name.
const char *QsciLexerPerl::language() const
{
    return "Perl";
}


// Returns the lexer name.
const char *QsciLexerPerl::lexer() const
{
    return "perl";
}


// Return the style used for braces.
int QsciLexerPerl::braceStyle() const
{
    return Operator;
}


// Return the string of characters that comprise a word.
const char *QsciLexerPerl::wordCharacters() const
{
    return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_$@%&";
}


// Returns the foreground colour of the text for a style.
QColor QsciLexerPerl::defaultColor(int style) const
{
    switch (style)
    {
    case Default:
        return QColor(0x80,0x80,0x80);

    case Error:
    case Backticks:
    case QuotedStringQX:
        return QColor(0xff,0xff,0x00);

    case Comment:
        return QColor(0x00,0x7f,0x00);

    case POD:
    case PODVerbatim:
        return QColor(0x00,0x40,0x00);

    case Number:
        return QColor(0x00,0x7f,0x7f);

    case Keyword:
        return QColor(0x00,0x00,0x7f);

    case DoubleQuotedString:
    case SingleQuotedString:
    case SingleQuotedHereDocument:
    case DoubleQuotedHereDocument:
    case BacktickHereDocument:
    case QuotedStringQ:
    case QuotedStringQQ:
        return QColor(0x7f,0x00,0x7f);

    case Operator:
    case Identifier:
    case Scalar:
    case Array:
    case Hash:
    case SymbolTable:
    case Regex:
    case Substitution:
    case HereDocumentDelimiter:
    case QuotedStringQR:
    case QuotedStringQW:
        return QColor(0x00,0x00,0x00);

    case DataSection:
        return QColor(0x60,0x00,0x00);
    }

    return QsciLexer::defaultColor(style);
}


// Returns the end-of-line fill for a style.
bool QsciLexerPerl::defaultEolFill(int style) const
{
    switch (style)
    {
    case POD:
    case DataSection:
    case SingleQuotedHereDocument:
    case DoubleQuotedHereDocument:
    case BacktickHereDocument:
    case PODVerbatim:
        return true;
    }

    return QsciLexer::defaultEolFill(style);
}


// Returns the font of the text for a style.
QFont QsciLexerPerl::defaultFont(int style) const
{
    QFont f;

    switch (style)
    {
    case Comment:
#if defined(Q_OS_WIN)
        f = QFont("Comic Sans MS",9);
#else
        f = QFont("Bitstream Vera Serif",9);
#endif
        break;

    case POD:
#if defined(Q_OS_WIN)
        f = QFont("Times New Roman",11);
#else
        f = QFont("Bitstream Charter",10);
#endif
        break;

    case Keyword:
    case Operator:
    case DoubleQuotedHereDocument:
        f = QsciLexer::defaultFont(style);
        f.setBold(true);
        break;

    case DoubleQuotedString:
    case SingleQuotedString:
    case QuotedStringQQ:
    case PODVerbatim:
#if defined(Q_OS_WIN)
        f = QFont("Courier New",10);
#else
        f = QFont("Bitstream Vera Sans Mono",9);
#endif
        break;

    case BacktickHereDocument:
        f = QsciLexer::defaultFont(style);
        f.setItalic(true);
        break;

    default:
        f = QsciLexer::defaultFont(style);
    }

    return f;
}


// Returns the set of keywords.
const char *QsciLexerPerl::keywords(int set) const
{
    if (set == 1)
        return
            "NULL __FILE__ __LINE__ __PACKAGE__ __DATA__ __END__ "
            "AUTOLOAD BEGIN CORE DESTROY END EQ GE GT INIT LE LT "
            "NE CHECK abs accept alarm and atan2 bind binmode "
            "bless caller chdir chmod chomp chop chown chr chroot "
            "close closedir cmp connect continue cos crypt "
            "dbmclose dbmopen defined delete die do dump each "
            "else elsif endgrent endhostent endnetent endprotoent "
            "endpwent endservent eof eq eval exec exists exit exp "
            "fcntl fileno flock for foreach fork format formline "
            "ge getc getgrent getgrgid getgrnam gethostbyaddr "
            "gethostbyname gethostent getlogin getnetbyaddr "
            "getnetbyname getnetent getpeername getpgrp getppid "
            "getpriority getprotobyname getprotobynumber "
            "getprotoent getpwent getpwnam getpwuid getservbyname "
            "getservbyport getservent getsockname getsockopt glob "
            "gmtime goto grep gt hex if index int ioctl join keys "
            "kill last lc lcfirst le length link listen local "
            "localtime lock log lstat lt m map mkdir msgctl "
            "msgget msgrcv msgsnd my ne next no not oct open "
            "opendir or ord our pack package pipe pop pos print "
            "printf prototype push q qq qr quotemeta qu qw qx "
            "rand read readdir readline readlink readpipe recv "
            "redo ref rename require reset return reverse "
            "rewinddir rindex rmdir s scalar seek seekdir select "
            "semctl semget semop send setgrent sethostent "
            "setnetent setpgrp setpriority setprotoent setpwent "
            "setservent setsockopt shift shmctl shmget shmread "
            "shmwrite shutdown sin sleep socket socketpair sort "
            "splice split sprintf sqrt srand stat study sub "
            "substr symlink syscall sysopen sysread sysseek "
            "system syswrite tell telldir tie tied time times tr "
            "truncate uc ucfirst umask undef unless unlink unpack "
            "unshift untie until use utime values vec wait "
            "waitpid wantarray warn while write x xor y";

    return 0;
}


// Returns the user name of a style.
QString QsciLexerPerl::description(int style) const
{
    switch (style)
    {
    case Default:
        return tr("Default");

    case Error:
        return tr("Error");

    case Comment:
        return tr("Comment");

    case POD:
        return tr("POD");

    case Number:
        return tr("Number");

    case Keyword:
        return tr("Keyword");

    case DoubleQuotedString:
        return tr("Double-quoted string");

    case SingleQuotedString:
        return tr("Single-quoted string");

    case Operator:
        return tr("Operator");

    case Identifier:
        return tr("Identifier");

    case Scalar:
        return tr("Scalar");

    case Array:
        return tr("Array");

    case Hash:
        return tr("Hash");

    case SymbolTable:
        return tr("Symbol table");

    case Regex:
        return tr("Regular expression");

    case Substitution:
        return tr("Substitution");

    case Backticks:
        return tr("Backticks");

    case DataSection:
        return tr("Data section");

    case HereDocumentDelimiter:
        return tr("Here document delimiter");

    case SingleQuotedHereDocument:
        return tr("Single-quoted here document");

    case DoubleQuotedHereDocument:
        return tr("Double-quoted here document");

    case BacktickHereDocument:
        return tr("Backtick here document");

    case QuotedStringQ:
        return tr("Quoted string (q)");

    case QuotedStringQQ:
        return tr("Quoted string (qq)");

    case QuotedStringQX:
        return tr("Quoted string (qx)");

    case QuotedStringQR:
        return tr("Quoted string (qr)");

    case QuotedStringQW:
        return tr("Quoted string (qw)");

    case PODVerbatim:
        return tr("POD verbatim");
    }

    return QString();
}


// Returns the background colour of the text for a style.
QColor QsciLexerPerl::defaultPaper(int style) const
{
    switch (style)
    {
    case Error:
        return QColor(0xff,0x00,0x00);

    case POD:
        return QColor(0xe0,0xff,0xe0);

    case Scalar:
        return QColor(0xff,0xe0,0xe0);

    case Array:
        return QColor(0xff,0xff,0xe0);

    case Hash:
        return QColor(0xff,0xe0,0xff);

    case SymbolTable:
        return QColor(0xe0,0xe0,0xe0);

    case Regex:
        return QColor(0xa0,0xff,0xa0);

    case Substitution:
        return QColor(0xf0,0xe0,0x80);

    case Backticks:
        return QColor(0xa0,0x80,0x80);

    case DataSection:
        return QColor(0xff,0xf0,0xd8);

    case HereDocumentDelimiter:
    case SingleQuotedHereDocument:
    case DoubleQuotedHereDocument:
    case BacktickHereDocument:
        return QColor(0xdd,0xd0,0xdd);

    case PODVerbatim:
        return QColor(0xc0,0xff,0xc0);
    }

    return QsciLexer::defaultPaper(style);
}


// Refresh all properties.
void QsciLexerPerl::refreshProperties()
{
    setCommentProp();
    setCompactProp();
}


// Read properties from the settings.
bool QsciLexerPerl::readProperties(QSettings &qs,const QString &prefix)
{
    int rc = true;

    fold_comments = qs.value(prefix + "foldcomments", false).toBool();
    fold_compact = qs.value(prefix + "foldcompact", true).toBool();

    return rc;
}


// Write properties to the settings.
bool QsciLexerPerl::writeProperties(QSettings &qs,const QString &prefix) const
{
    int rc = true;

    qs.setValue(prefix + "foldcomments", fold_comments);
    qs.setValue(prefix + "foldcompact", fold_compact);

    return rc;
}


// Return true if comments can be folded.
bool QsciLexerPerl::foldComments() const
{
    return fold_comments;
}


// Set if comments can be folded.
void QsciLexerPerl::setFoldComments(bool fold)
{
    fold_comments = fold;

    setCommentProp();
}


// Set the "fold.comment" property.
void QsciLexerPerl::setCommentProp()
{
    emit propertyChanged("fold.comment",(fold_comments ? "1" : "0"));
}


// Return true if folds are compact.
bool QsciLexerPerl::foldCompact() const
{
    return fold_compact;
}


// Set if folds are compact
void QsciLexerPerl::setFoldCompact(bool fold)
{
    fold_compact = fold;

    setCompactProp();
}


// Set the "fold.compact" property.
void QsciLexerPerl::setCompactProp()
{
    emit propertyChanged("fold.compact",(fold_compact ? "1" : "0"));
}
