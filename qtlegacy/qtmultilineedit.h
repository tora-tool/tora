/**********************************************************************
** $Id$
**
** Definition of QtMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file contains a class moved out of the Qt GUI Toolkit API. It
** may be used, distributed and modified without limitation.
**
**********************************************************************/

#ifndef QTMULTILINEEDIT_H
#define QTMULTILINEEDIT_H

#ifndef QT_H
#include "qttableview.h"
#include "qstring.h"
#include "qptrlist.h"
#endif // QT_H

#ifndef QT_NO_QTMULTILINEEDIT

struct QtMultiLineData;
class QtMultiLineEditCommand;
class QValidator;

class QtMultiLineEdit : public QtTableView
{
    Q_OBJECT
    Q_ENUMS( EchoMode WordWrap WrapPolicy )
    Q_PROPERTY( int numLines READ numLines )
    Q_PROPERTY( bool atBeginning READ atBeginning )
    Q_PROPERTY( bool atEnd READ atEnd )
    Q_PROPERTY( int maxLineWidth READ maxLineWidth )
    Q_PROPERTY( Alignment alignment READ alignment WRITE setAlignment )
    Q_PROPERTY( bool edited READ edited WRITE setEdited DESIGNABLE false )
    Q_PROPERTY( EchoMode echoMode READ echoMode WRITE setEchoMode )
    Q_PROPERTY( int maxLength READ maxLength WRITE setMaxLength )
    Q_PROPERTY( int maxLines READ maxLines WRITE setMaxLines )
    Q_PROPERTY( int hMargin READ hMargin WRITE setHMargin )
    Q_PROPERTY( WordWrap wordWrap READ wordWrap WRITE setWordWrap )
    Q_PROPERTY( int wrapColumnOrWidth READ wrapColumnOrWidth WRITE setWrapColumnOrWidth )
    Q_PROPERTY( WrapPolicy wrapPolicy READ wrapPolicy WRITE setWrapPolicy )
    Q_PROPERTY( bool autoUpdate READ autoUpdate WRITE setAutoUpdate DESIGNABLE false )
    Q_PROPERTY( bool undoEnabled READ isUndoEnabled WRITE setUndoEnabled )
    Q_PROPERTY( int undoDepth READ undoDepth WRITE setUndoDepth )
    Q_PROPERTY( bool readOnly READ isReadOnly WRITE setReadOnly )
    Q_PROPERTY( bool overWriteMode READ isOverwriteMode WRITE setOverwriteMode )
    Q_PROPERTY( QString text READ text WRITE setText )
    Q_PROPERTY( int length READ length )

public:
    QtMultiLineEdit( QWidget *parent=0, const char *name=0 );
   ~QtMultiLineEdit();

    QString textLine( int line ) const;
    int numLines() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    QSizePolicy sizePolicy() const;

    virtual void setFont( const QFont &font );

    virtual void insertLine( const QString &s, int line = -1 );
    virtual void insertAt( const QString &s, int line, int col, bool mark = FALSE );
    virtual void removeLine( int line );

    void cursorPosition( int *line, int *col ) const;
    virtual void setCursorPosition( int line, int col, bool mark = FALSE );
    void getCursorPosition( int *line, int *col ) const;
    bool atBeginning() const;
    bool atEnd() const;

    virtual void setFixedVisibleLines( int lines );

    int maxLineWidth() const;

    void setAlignment( int flags );
    int alignment() const;

    virtual void setValidator( const QValidator * );
    const QValidator * validator() const;

    void setEdited( bool );
    bool edited() const;

    void cursorWordForward( bool mark );
    void cursorWordBackward( bool mark );

    enum EchoMode { Normal, NoEcho, Password };
    virtual void setEchoMode( EchoMode );
    EchoMode echoMode() const;

    void setMaxLength(int);
    int maxLength() const;
    virtual void setMaxLineLength(int);
    int maxLineLength() const;
    virtual void setMaxLines(int);
    int maxLines() const;
    virtual void setHMargin(int);
    int hMargin() const;

    virtual void setSelection( int row_from, int col_from, int row_to, int col_t );

    enum WordWrap {
	NoWrap,
	WidgetWidth,
	FixedPixelWidth,
	FixedColumnWidth
    };
    void setWordWrap( WordWrap mode );
    WordWrap wordWrap() const;
    void setWrapColumnOrWidth( int );
    int wrapColumnOrWidth() const;

    enum WrapPolicy {
	AtWhiteSpace,
	Anywhere
    };
    void setWrapPolicy( WrapPolicy policy );
    WrapPolicy wrapPolicy() const;

    bool autoUpdate()	const;
    virtual void setAutoUpdate( bool );

    void setUndoEnabled( bool );
    bool isUndoEnabled() const;
    void setUndoDepth( int );
    int undoDepth() const;

    bool isReadOnly() const;
    bool isOverwriteMode() const;

    QString text() const;

    int length() const;

    static void setDefaultTabStop( int ex );
    static int defaultTabStop();
public slots:
    virtual void       setText( const QString &);
    virtual void       setReadOnly( bool );
    virtual void       setOverwriteMode( bool );

    void       clear();
    void       append( const QString &);
    void       deselect();
    void       selectAll();
#ifndef QT_NO_CLIPBOARD
    void       paste();
    void       pasteSubType(const QCString& subtype);
    void       copyText() const;
    void       copy() const;
    void       cut();
#endif
    void       insert( const QString& );
    void       undo();
    void       redo();

signals:
    void	textChanged();
    void	returnPressed();
    void	undoAvailable( bool );
    void	redoAvailable( bool );
    void	copyAvailable( bool );

protected:
    void	paintCell( QPainter *, int row, int col );
    bool	event( QEvent * );

    void	mousePressEvent( QMouseEvent * );
    void	mouseMoveEvent( QMouseEvent * );
    void	mouseReleaseEvent( QMouseEvent * );
    void	mouseDoubleClickEvent( QMouseEvent * );
    void 	wheelEvent( QWheelEvent * );
    void	keyPressEvent( QKeyEvent * );
    void	focusInEvent( QFocusEvent * );
    void	focusOutEvent( QFocusEvent * );
    void	timerEvent( QTimerEvent * );
    void	leaveEvent( QEvent * );
    void	resizeEvent( QResizeEvent * );

    bool	focusNextPrevChild( bool );

#ifndef QT_NO_DRAGANDDROP
    void	dragMoveEvent( QDragMoveEvent* );
    void	dragEnterEvent( QDragEnterEvent * );
    void	dropEvent( QDropEvent* );
    void	dragLeaveEvent( QDragLeaveEvent* );
#endif

    bool	hasMarkedText() const;
    QString	markedText() const;
    int		textWidth( int );
    int		textWidth( const QString &);

    QPoint	cursorPoint() const;

protected:
    virtual void insert( const QString&, bool mark );
    virtual void newLine();
    virtual void killLine();
    virtual void pageUp( bool mark=FALSE );
    virtual void pageDown( bool mark=FALSE );
    virtual void cursorLeft( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorRight( bool mark=FALSE, bool wrap = TRUE );
    virtual void cursorUp( bool mark=FALSE );
    virtual void cursorDown( bool mark=FALSE );
    virtual void backspace();
    virtual void del();
    virtual void home( bool mark=FALSE );
    virtual void end( bool mark=FALSE );

    bool getMarkedRegion( int *line1, int *col1,
			  int *line2, int *col2 ) const;
    int lineLength( int row ) const;
    QString *getString( int row ) const;
    bool isEndOfParagraph( int row ) const;
    QString stringShown( int row ) const;

protected:
    bool	cursorOn;
    void	insertChar( QChar );

private slots:
    void	clipboardChanged();
    void	blinkTimerTimeout();
    void	scrollTimerTimeout();
    void	dndTimeout();

private:
#ifndef QT_NO_MIME
    QCString pickSpecial(QMimeSource* ms, bool always_ask, const QPoint&);
#endif
#ifndef QT_NO_MIMECLIPBOARD
    void       pasteSpecial(const QPoint&);
#endif
    struct QtMultiLineEditRow {
	QtMultiLineEditRow( QString string, int width, bool nl = TRUE )
	    :s(string), w(width), newline( nl )
	{
	};
	QString s;
	int w;
	bool newline;
    };
    QPtrList<QtMultiLineEditRow> *contents;
    QtMultiLineData *d;

    bool	readOnly;
    bool	dummy;
    bool	markIsOn;
    bool	dragScrolling ;
    bool	dragMarking;
    bool	textDirty;
    bool	wordMark;
    bool	overWrite;

    int		cursorX;
    int		cursorY;
    int		markAnchorX;
    int		markAnchorY;
    int		markDragX;
    int		markDragY;
    int		curXPos;	// cell coord of cursor
    int		blinkTimer; // #### not used anymore - remove in 3.0
    int		scrollTimer; // #### not used anymore - remove in 3.0

    int		mapFromView( int xPos, int row );
    int		mapToView( int xIndex, int row );

    void	pixelPosToCursorPos(QPoint p, int* x, int* y) const;
    void	setCursorPixelPosition(QPoint p, bool clear_mark=TRUE);

    void	setWidth( int );
    void	updateCellWidth();
    bool 	partiallyInvisible( int row );
    void	makeVisible();
    void	setBottomCell( int row );

    void 	newMark( int posx, int posy, bool copy=TRUE );
    void 	markWord( int posx, int posy );
    void	extendSelectionWord( int &newX, int&newY);
    int 	charClass( QChar );
    void	turnMark( bool on );
    bool	inMark( int posx, int posy ) const;
    bool	beforeMark( int posx, int posy ) const;
    bool	afterMark( int posx, int posy ) const;
    int		setNumRowsAndTruncate();

#ifndef QT_NO_DRAGANDDROP
    void	doDrag();
#endif
    void	startAutoScroll();
    void	stopAutoScroll();

    void	cursorLeft( bool mark, bool clear_mark, bool wrap );
    void	cursorRight( bool mark, bool clear_mark, bool wrap );
    void	cursorUp( bool mark, bool clear_mark );
    void	cursorDown( bool mark, bool clear_mark );

    void	wrapLine( int line, int removed = 0);
    void	rebreakParagraph( int line, int removed = 0 );
    void	rebreakAll();
    void	insertAtAux( const QString &s, int line, int col, bool mark = FALSE );
    void	killLineAux();
    void	delAux();
    int	positionToOffsetInternal( int row, int col ) const;
    void	offsetToPositionInternal( int position, int *row, int *col ) const;
    void	deleteNextChar( int offset, int row, int col );

    void addUndoCmd( QtMultiLineEditCommand* );
    void addRedoCmd( QtMultiLineEditCommand* );
    void processCmd( QtMultiLineEditCommand*, bool );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QtMultiLineEdit( const QtMultiLineEdit & );
    QtMultiLineEdit &operator=( const QtMultiLineEdit & );
#endif
};

inline bool QtMultiLineEdit::isReadOnly() const { return readOnly; }

inline bool QtMultiLineEdit::isOverwriteMode() const { return overWrite; }

inline void QtMultiLineEdit::setOverwriteMode( bool on )
{
    overWrite = on;
 }

inline int QtMultiLineEdit::lineLength( int row ) const
{
    return contents->at( row )->s.length();
}

inline bool QtMultiLineEdit::atEnd() const
{
    return cursorY == (int)contents->count() - 1
	&& cursorX == lineLength( cursorY ) ;
}

inline bool QtMultiLineEdit::atBeginning() const
{
    return cursorY == 0 && cursorX == 0;
}

inline QString *QtMultiLineEdit::getString( int row ) const
{
    return &(contents->at( row )->s);
}

inline int QtMultiLineEdit::numLines() const
{
    return contents->count();
}

#endif // QT_NO_QTMULTILINEEDIT

#endif // QTMULTILINEDIT_H
