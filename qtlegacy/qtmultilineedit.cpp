/**********************************************************************
** $Id$
**
** Implementation of QtMultiLineEdit widget class
**
** Created : 961005
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file contains a class moved out of the Qt GUI Toolkit API. It
** may be used, distributed and modified without limitation.
**
**********************************************************************/

#include "qtmultilineedit.h"
#ifndef QT_NO_QTMULTILINEEDIT
#include "qpainter.h"
#include "qscrollbar.h"
#include "qclipboard.h"
#include "qpixmap.h"
#include "qregexp.h"
#include "qapplication.h"
#include "qdragobject.h"
#include "qpopupmenu.h"
#include "qtimer.h"
#include "qdict.h"
#include "qcursor.h"
#ifndef QT_NO_COMPAT
#include "qstyle.h"
#endif


class QtMultiLineEditCommand
{
public:
    enum Commands { Invalid, Begin, End, Insert, Delete };
    virtual ~QtMultiLineEditCommand() {};
    virtual Commands type() { return Invalid; };
    virtual int terminator() { return 0; }

    virtual bool merge( QtMultiLineEditCommand* ) { return FALSE;}
};

class QBeginCommand : public QtMultiLineEditCommand
{

public:
    QBeginCommand() {}
    int terminator() { return 1; }
    Commands type() { return Begin; };
};

class QEndCommand : public QtMultiLineEditCommand
{
public:
    QEndCommand() {}
    int terminator() { return -1; }
    Commands type() { return End; };
};

// QtMultiLineEditUndoRedo methods
class QDelTextCmd : public QtMultiLineEditCommand
{
public:
    int mOffset;
    QString mStr;

    // have to handle deletion of current selection
    QDelTextCmd(int offset, const QString &str )
	: mOffset( offset ),
	  mStr ( str )
    {
    }
    Commands type() { return Delete; };

    bool merge( QtMultiLineEditCommand* other)
    {
	if ( other->type() == type() ) {
	    QDelTextCmd* o = (QDelTextCmd*) other;
	    if ( mOffset + int(mStr.length()) == o->mOffset ) {
		o->mStr.prepend( mStr );
		o->mOffset = mOffset;
		return TRUE;
	    }
	}
	return FALSE;
    }


};

class QInsTextCmd : public QDelTextCmd
{

public:
    QInsTextCmd(int offset,const QString &str )
	: QDelTextCmd( offset, str )
    {
    }

    Commands type() { return Insert; };

    bool merge( QtMultiLineEditCommand* other)
    {
	if ( other->type() == type() ) {
	    QInsTextCmd* o = (QInsTextCmd*) other;
	    if ( mOffset == o->mOffset + int(o->mStr.length()) ) {
		o->mStr += mStr;
		return TRUE;
	    }
	}
	return FALSE;
    }
};


/*!
  \class QtMultiLineEdit qtmultilineedit.h

  \brief The QtMultiLineEdit widget is a simple editor for inputting text.

  \obsolete

  The QtMultiLineEdit widget provides multiple line text input and display.
  It is intended for moderate amounts of text. There are no arbitrary
  limitations, but if you try to handle megabytes of data, performance
  will suffer.

  Per default, the edit widget does not perform any word
  wrapping. This can be adjusted by calling setWordWrap(). Both
  dynamic wrapping according to the visible width or a fixed number of
  character or pixels is supported.

  The widget can be used to display text by calling setReadOnly(TRUE).

  The default key bindings are described in keyPressEvent(); they cannot
  be customized except by inheriting the class.

  <img src=qmlined-m.png> <img src=qmlined-w.png>
*/

/*!
    \property QtMultiLineEdit::numLines
    \brief the number of lines in the multi-line edit

  numLines() returns the number of lines in the editor. The count
  includes any empty lines at top and bottom, so for an empty editor
  this method will return 1.
*/
/*!
    \property QtMultiLineEdit::atBeginning
    \brief whether the cursor is at the beginning

    atBeginning() returns TRUE if the cursor is placed at the
    beginning of the text.
*/
/*!
    \property QtMultiLineEdit::atEnd
    \brief whether the cursor is at the end

  atEnd() returns TRUE if the cursor is placed at the end of the text.
*/
/*!
    \property QtMultiLineEdit::maxLineWidth
    \brief the maximum line width in pixels
  Returns the width in pixels of the longest text line in this editor.
*/
/*!
    \property QtMultiLineEdit::alignment
    \brief the alignment

    Possible values are \c AlignLeft, \c Align(H)Center and \c
    AlignRight.
  \sa Qt::AlignmentFlags
*/
/*!
    \property QtMultiLineEdit::edited
    \brief whether the text had been edited

edited() returns the edited flag of the line edit.  If this returns FALSE,
the contents has not been changed since the construction of the
QtMultiLineEdit (or the last call to setEdited( FALSE ), if any).  If
it returns TRUE, the contents have been edited, or setEdited( TRUE )
has been called.

setEdited() sets the edited flag of this line edit to \e e.  The
edited flag is never read by QtMultiLineEdit, but is changed to TRUE
whenever the user changes its contents.

This is useful e.g. for things that need to provide a default value,
but cannot find the default at once.  Just open the widget without the
best default and when the default is known, check the edited() return
value and set the line edit's contents if the user has not started
editing the line edit.  Another example is to detect whether the
contents need saving.

*/
/*!
    \property QtMultiLineEdit::echoMode
    \brief the echo mode
*/
/*!
    \property QtMultiLineEdit::maxLength
    \brief the maximum length of the text

  The currently set text length limit, or -1 if there is
  no limit (this is the default).

*/
/*!
    \property QtMultiLineEdit::maxLines
    \brief the maximum number of lines
  The currently set line limit, or -1 if there is
  no limit (the default).

  Note that excess lines are deleted from the \e bottom of the
  lines. If you want teletype behaviour with lines disappearing
  from the \e top as the limit is exceed, you probably just want
  to use removeLine(0) prior to adding an excess line.

*/
/*!
    \property QtMultiLineEdit::hMargin
    \brief the horizontal margin
  The horizontal margin current set.  The default is 3.
*/
/*!
    \property QtMultiLineEdit::wordWrap
    \brief the word wrap mode

  By default, wrapping keeps words intact. To allow breaking within
  words, set the wrap policy to \c Anywhere (see setWrapPolicy() ).

  The default wrap mode is \c NoWrap.

  \sa wordWrap(), setWrapColumnOrWidth(), setWrapPolicy()
*/
/*!
    \property QtMultiLineEdit::wrapColumnOrWidth
    \brief the wrap width in columns or pixels
  The wrap column or wrap width, depending on the word wrap
  mode.
  \sa setWordWrap(), setWrapColumnOrWidth()
*/
/*!
    \property QtMultiLineEdit::wrapPolicy
    \brief the wrap policy mode
   The default is \c AtWhiteSpace.

*/
/*!
    \property QtMultiLineEdit::autoUpdate
    \brief whether auto update is enabled

  autoUpdate() returns TRUE if the view updates itself automatically
  whenever it is changed in some way.

  If autoUpdate() is TRUE (this is the default) then the editor updates
  itself automatically whenever it has changed in some way (generally,
  when text has been inserted or deleted).

  If autoUpdate() is FALSE, the view does NOT repaint itself, or update
  its internal state variables itself when it is changed.  This can be
  useful to avoid flicker during large changes, and is singularly
  useless otherwise: Disable auto-update, do the changes, re-enable
  auto-update, and call repaint().

  \warning Do not leave the view in this state for a long time
  (i.e. between events ). If, for example, the user interacts with the
  view when auto-update is off, strange things can happen.

  Setting auto-update to TRUE does not repaint the view, you must call
  repaint() to do this (preferable repaint(FALSE) to avoid flicker).

  \sa autoUpdate() repaint()

*/
/*!
    \property QtMultiLineEdit::undoEnabled
    \brief whether undo is enabled
*/
/*!
    \property QtMultiLineEdit::undoDepth
    \brief the undo depth

  The maximum number of operations that can be stored on the undo stack.

  \sa setUndoDepth()
*/
/*!
    \property QtMultiLineEdit::readOnly
    \brief whether the multi-line edit is read-only
*/
/*!
    \property QtMultiLineEdit::overWriteMode
    \brief the overwrite mode
*/
/*!
    \property QtMultiLineEdit::text
    \brief the multi-line edit's text
*/
/*!
    \property QtMultiLineEdit::length
    \brief the length of the text
*/

static const char * const arrow_xpm[] = {
    "     8     8        2            1",
    ". c None",
    "# c #000000",
    "........",
    "..####..",
    "..#####.",
    ".....##.",
    ".#..###.",
    ".#####..",
    ".####...",
    ".#####.."
};

enum {
    IdUndo,
    IdRedo,
#ifndef QT_NO_CLIPBOARD
    IdCut,
    IdCopy,
    IdPaste,
    IdPasteSpecial,
#endif
    IdClear,
    IdSelectAll,
    IdCount
};

struct QtMultiLineData
{
    QtMultiLineData() :
	isHandlingEvent(FALSE),
	edited(FALSE),
	maxLineWidth(0),
	align(Qt::AlignLeft),
	maxlines(-1),
	maxlinelen(-1),
	maxlen(-1),
	wordwrap( QtMultiLineEdit::NoWrap ),
	wrapcol( -1 ),
	wrappolicy( QtMultiLineEdit::AtWhiteSpace ),
	// This doesn't use font bearings, as textWidthWithTabs does that.
	// This is just an aesthetics value.
	// It should probably be QMAX(0,3-fontMetrics().minLeftBearing()) though,
	// as bearings give some border anyway.
	lr_marg(3),
	marg_extra(0),
	echomode(QtMultiLineEdit::Normal),
	val(0),
	dnd_primed(FALSE),
	dnd_forcecursor(FALSE),
	undo( TRUE ),
	undodepth( 256 )
    {
	undoList.setAutoDelete( TRUE );
	redoList.setAutoDelete( TRUE );
	clearChartable();
    }
    bool isHandlingEvent;
    bool edited;
    int  maxLineWidth;
    int	 scrollTime;
    int	 scrollAccel;
    int  align;
    int  numlines;
    int  maxlines;
    int  maxlinelen;
    int  maxlen;
    QtMultiLineEdit::WordWrap wordwrap;
    int wrapcol;
    QtMultiLineEdit::WrapPolicy wrappolicy;
    int lr_marg;
    int marg_extra;
    QtMultiLineEdit::EchoMode echomode;
    const QValidator* val;

    bool dnd_primed; // If TRUE, user has pressed
    bool dnd_forcecursor; // If TRUE show cursor for DND feedback,
    // even if !hasFocus()
    QPtrList<QtMultiLineEditCommand> undoList;
    QPtrList<QtMultiLineEditCommand> redoList;
    bool undo;
    int undodepth;
    short chartable[256];
    void clearChartable()
    {
	int i = 256;
	while ( i )
	    chartable[--i] = 0;
    }
    QPixmap arrow;
    QPoint dnd_startpos;
    QTimer *blinkTimer, *scrollTimer;
#ifndef QT_NO_DRAGANDDROP
    QTimer *dnd_timer;
#endif
};


#define CLEAR_UNDO {d->undoList.clear(); emit undoAvailable( FALSE );\
    d->redoList.clear(); emit redoAvailable( FALSE );}

void QtMultiLineEdit::addUndoCmd(QtMultiLineEditCommand* c)
{
    if ( d->undoList.isEmpty() )
	emit undoAvailable(TRUE);
    else if ( c->merge( d->undoList.last() ) ) {
	delete c;
	return;
    }
    if ( int(d->undoList.count()) >= d->undodepth )
	d->undoList.removeFirst();
    d->undoList.append(c);

    if ( !d->redoList.isEmpty() ) {
	d->redoList.clear();
	emit redoAvailable( FALSE );
    }
}

void QtMultiLineEdit::addRedoCmd(QtMultiLineEditCommand* c)
{
    if ( d->redoList.isEmpty() )
	emit redoAvailable(TRUE);
    d->redoList.append(c);
}

static const int initialScrollTime = 50; // mark text scroll time
static const int initialScrollAccel = 5; // mark text scroll accel (0=fastest)
static const int scroll_margin = 16;     // auto-scroll edge in DND

#define WORD_WRAP ( d->wordwrap != QtMultiLineEdit::NoWrap )
#define DYNAMIC_WRAP ( d->wordwrap == QtMultiLineEdit::WidgetWidth )
#define FIXED_WIDTH_WRAP ( d->wordwrap == QtMultiLineEdit::FixedPixelWidth )
#define FIXED_COLUMN_WRAP ( d->wordwrap == QtMultiLineEdit::FixedColumnWidth )
#define BREAK_WITHIN_WORDS ( d->wrappolicy == QtMultiLineEdit::Anywhere )

static int defTabStop = 8;

static int tabStopDist( const QFontMetrics &fm )
{
    int dist;
    dist = fm.width( QChar('x' ));
    if( dist == 0 )
       dist = fm.maxWidth();
    return defTabStop*dist;
}


/*!
  Sets the distance between tab stops for all QtMultiLineEdit instances
  to \a ex, which is measured in multiples of the width of a lower case 'x'
  in the widget's font. The initial value is 8.

  \warning This function does not cause a redraw. It is best to call
  it before any QtMultiLineEdit widgets are shown.

  \sa defaultTabStop()
*/

void QtMultiLineEdit::setDefaultTabStop( int ex )
{
    defTabStop = ex;
}



/*!
  Returns the distance between tab stops.

  \sa setDefaultTabStop();
*/

int QtMultiLineEdit::defaultTabStop()
{
    return defTabStop;
}




static int textWidthWithTabs( const QFontMetrics &fm, const QString &s, uint start, uint nChars, int align )
{
    if ( s.isEmpty() )
	return 0;

    int dist = -fm.leftBearing( s[(int)start] );
    int i = start;
    int tabDist = -1; // lazy eval
    while ( (uint)i < s.length() && (uint)i < start+nChars ) {
	if ( s[i] == '\t' && align == Qt::AlignLeft ) {
	    if ( tabDist<0 )
		tabDist = tabStopDist(fm);
	    dist = ( (dist+tabDist+1)/tabDist ) * tabDist;
	    i++;
	} else {
	    int ii = i;
	    while ( (uint)i < s.length() && (uint)i < start + nChars && ( align != Qt::AlignLeft || s[i] != '\t' ) )
		i++;
	    dist += fm.width( s.mid(ii,i-ii) );
	}
    }
    return dist;
}

static int xPosToCursorPos( const QString &s, const QFontMetrics &fm,
			    int xPos, int width, int align )
{
    int i = 0;
    int	  dist;
    int tabDist;

    if ( s.isEmpty() )
	return 0;
    if ( xPos > width )
	xPos = width;
    if ( xPos <= 0 )
	return 0;

    dist    = -fm.leftBearing( s[0] );

    if ( align == Qt::AlignCenter || align == Qt::AlignHCenter )
	dist = ( width - textWidthWithTabs( fm, s, 0, s.length(), align ) ) / 2;
    else if ( align == Qt::AlignRight )
	dist = width - textWidthWithTabs( fm, s, 0, s.length(), align );

    int     distBeforeLastTab = dist;
    tabDist = tabStopDist(fm);
    while ( (uint)i < s.length() && dist < xPos ) {
	if ( s[i] == '\t' && align == Qt::AlignLeft ) {
	    distBeforeLastTab = dist;
	    dist = (dist/tabDist + 1) * tabDist;
	} else {
	    dist += fm.width( s[i] );
	}
	i++;
    }
    if ( dist > xPos ) {
	if ( dist > width ) {
	    i--;
	} else {
	    if ( s[i-1] == '\t' && align == Qt::AlignLeft ) { // dist equals a tab stop position
		if ( xPos - distBeforeLastTab < (dist - distBeforeLastTab)/2 )
		    i--;
	    } else {
		if ( fm.width(s[i-1])/2 < dist-xPos )
		    i--;
	    }
	}
    }
    return i;
}

/*!
  Constructs a new, empty, QtMultiLineEdit with parent \a parent and
  called \a name.
*/

QtMultiLineEdit::QtMultiLineEdit( QWidget *parent , const char *name )
    :QtTableView( parent, name, WStaticContents | WRepaintNoErase )
{
    d = new QtMultiLineData;
    QFontMetrics fm( font() );
    setCellHeight( fm.lineSpacing() );
    setNumCols( 1 );

    contents = new QPtrList<QtMultiLineEditRow>;
    contents->setAutoDelete( TRUE );

    cursorX = 0; cursorY = 0;
    curXPos = 0;

    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
		   Tbl_smoothVScrolling |
		   Tbl_clipCellPainting
		   );
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundMode( PaletteBase );
    setWFlags( WResizeNoErase );
    setKeyCompression( TRUE );
    setFocusPolicy( WheelFocus );
#ifndef QT_NO_CURSOR
    setCursor( ibeamCursor );
    verticalScrollBar()->setCursor( arrowCursor );
    horizontalScrollBar()->setCursor( arrowCursor );
#endif
    readOnly 	   = FALSE;
    cursorOn	   = FALSE;
    markIsOn	   = FALSE;
    dragScrolling  = FALSE;
    dragMarking    = FALSE;
    textDirty	   = FALSE;
    wordMark	   = FALSE;
    overWrite	   = FALSE;
    markAnchorX    = 0;
    markAnchorY    = 0;
    markDragX      = 0;
    markDragY      = 0;
    d->blinkTimer = new QTimer( this );
    connect( d->blinkTimer, SIGNAL( timeout() ),
	     this, SLOT( blinkTimerTimeout() ) );
    d->scrollTimer = new QTimer( this );
    connect( d->scrollTimer, SIGNAL( timeout() ),
	     this, SLOT( scrollTimerTimeout() ) );
#ifndef QT_NO_DRAGANDDROP
    d->dnd_timer = new QTimer( this );
    connect( d->dnd_timer, SIGNAL( timeout() ),
	     this, SLOT( dndTimeout() ) );
#endif
    d->scrollTime = 0;

    dummy = TRUE;

    int w  = textWidth( QString::fromLatin1("") );
    contents->append( new QtMultiLineEditRow(QString::fromLatin1(""), w) );
    (void)setNumRowsAndTruncate();
    setWidth( w );
    setAcceptDrops(TRUE);
    if ( d->maxlines >= 0 && d->maxlines <= 6 ) {
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    } else {
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    }
}

/*!
  \fn int QtMultiLineEdit::lineLength( int line ) const
  Returns the number of characters at line number \a line.
*/

/*! \fn QString *QtMultiLineEdit::getString( int line ) const

  Returns a pointer to the text at line \a line.
*/

/*! \fn void QtMultiLineEdit::textChanged()

  This signal is emitted when the text is changed by an event or by a
  slot. Note that the signal is not emitted when you call a non-slot
  function such as insertLine().

  \sa returnPressed()
*/

/*! \fn void QtMultiLineEdit::returnPressed()

  This signal is emitted when the user presses the return or enter
  key. It is not emitted if isReadOnly() is TRUE.

  \sa textChanged()
*/

/*!
  \fn void QtMultiLineEdit::undoAvailable (bool yes)

  This signal is emitted when the availability of undo changes.
  If \a yes is TRUE, then undo() will work until
  undoAvailable( FALSE ) is next emitted.
*/

/*!
  \fn void QtMultiLineEdit::redoAvailable (bool yes)

  This signal is emitted when the availability of redo changes.
  If \a yes is TRUE, then redo() will work until
  redoAvailable( FALSE ) is next emitted.
*/

/*!
  \fn void QtMultiLineEdit::copyAvailable (bool yes)

  This signal is emitted when the availability of cut/copy changes.
  If \a yes is TRUE, then cut() and copy() will work until
  copyAvailable( FALSE ) is next emitted.
*/


/*
  If \a on is FALSE, this multi line edit accepts text input.
  Scrolling and cursor movements are accepted in any case.

  \sa isReadOnly() QWidget::setEnabled()
*/

void QtMultiLineEdit::setReadOnly( bool on )
{
    if ( readOnly != on ) {
	readOnly = on;
#ifndef QT_NO_CURSOR
	setCursor( on ? arrowCursor : ibeamCursor );
#endif
    }
}

/*
*/
int QtMultiLineEdit::maxLineWidth() const
{
    return d->maxLineWidth;
}

/*!
  Destroys the QtMultiLineEdit
*/

QtMultiLineEdit::~QtMultiLineEdit()
{
    delete contents;
    delete d;
}

static QPixmap *buffer = 0;

static void cleanupMLBuffer()
{
    delete buffer;
    buffer = 0;
}

static QPixmap *getCacheBuffer( QSize sz )
{
    if ( !buffer ) {
	qAddPostRoutine( cleanupMLBuffer );
	buffer = new QPixmap;
    }

    if ( buffer->width() < sz.width() || buffer->height() < sz.height() )
	buffer->resize( sz );
    return buffer;
}

/*!
  Implements the basic drawing logic. Paints the line at row \a row
  using painter \a painter. The \a col parameter is ignored.
*/
void QtMultiLineEdit::paintCell( QPainter *painter, int row, int )
{
    const QColorGroup & g = colorGroup();
    QFontMetrics fm( painter->font() );
    QString s = stringShown(row);
    if ( s.isNull() ) {
	qWarning( "QtMultiLineEdit::paintCell: (%s) no text at line %d",
		  name( "unnamed" ), row );
	return;
    }
    QRect updateR = cellUpdateRect();
    QPixmap *buffer = getCacheBuffer( updateR.size() );
    ASSERT(buffer);
    buffer->fill ( g.base() );

    QPainter p( buffer );
    p.setFont( painter->font() );
    p.translate( -updateR.left(), -updateR.top() );

    p.setTabStops( tabStopDist(fm) );

    int yPos = 0;
    int markX1, markX2;				// in x-coordinate pixels
    markX1 = markX2 = 0;			// avoid gcc warning
    if ( markIsOn ) {
	int markBeginX, markBeginY;
	int markEndX, markEndY;
	getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX );
	if ( row >= markBeginY && row <= markEndY ) {
	    if ( row == markBeginY ) {
		markX1 = markBeginX;
		if ( row == markEndY ) 		// both marks on same row
		    markX2 = markEndX;
		else
		    markX2 = s.length();	// mark till end of line
	    } else {
		if ( row == markEndY ) {
		    markX1 = 0;
		    markX2 = markEndX;
		} else {
		    markX1 = 0;			// whole line is marked
		    markX2 = s.length();	// whole line is marked
		}
	    }
	}
    }
    p.setPen( g.text() );
    QtMultiLineEditRow* r = contents->at( row );
    int wcell = cellWidth() - 2*d->lr_marg;// - d->marg_extra;
    int wrow = r->w;
    int x = d->lr_marg - p.fontMetrics().leftBearing(s[0]);
    if ( d->align == Qt::AlignCenter || d->align == Qt::AlignHCenter )
	x += (wcell - wrow) / 2;
    else if ( d->align == Qt::AlignRight )
	x += wcell - wrow;
    p.drawText( x,  yPos, cellWidth()-d->lr_marg-x, cellHeight(),
		d->align == AlignLeft?ExpandTabs:0, s );
    if ( !r->newline && BREAK_WITHIN_WORDS )
	p.drawPixmap( x + wrow - d->lr_marg - d->marg_extra, yPos, d->arrow );
#if 0
    if ( r->newline )
	p.drawLine( d->lr_marg,  yPos+cellHeight()-2, cellWidth() - d->lr_marg, yPos+cellHeight()-2);
#endif
    if ( markX1 != markX2 ) {
	int sLength = s.length();
	int xpos1   =  mapToView( markX1, row );
	int xpos2   =  mapToView( markX2, row );
	int fillxpos1 = xpos1;
	int fillxpos2 = xpos2;
	if ( markX1 == 0 )
	    fillxpos1 -= 2;
	if ( markX2 == sLength )
	    fillxpos2 += 3;
	p.setClipping( TRUE );
	p.setClipRect( fillxpos1 - updateR.left(), 0,
		       fillxpos2 - fillxpos1, cellHeight(row) );
	p.fillRect( fillxpos1, 0, fillxpos2 - fillxpos1, cellHeight(row),
		    g.brush( QColorGroup::Highlight ) );
	p.setPen( g.highlightedText() );
	p.drawText( x,  yPos, cellWidth()-d->lr_marg-x, cellHeight(),
		    d->align == AlignLeft?ExpandTabs:0, s );
	p.setClipping( FALSE );
    }

    if ( row == cursorY && cursorOn && !readOnly ) {
	int cursorPos = QMIN( (int)s.length(), cursorX );
	int cXPos   = QMAX( mapToView( cursorPos, row ), 0 );
	int cYPos   = 0;
	if ( hasFocus() || d->dnd_forcecursor ) {
	    p.setPen( g.text() );
	    /* styled?
	       p.drawLine( cXPos - 2, cYPos,
	       cXPos + 2, cYPos );
	    */
	    p.drawLine( cXPos, cYPos,
			cXPos, cYPos + fm.height() - 2);
	    /* styled?
	       p.drawLine( cXPos - 2, cYPos + fm.height() - 2,
	       cXPos + 2, cYPos + fm.height() - 2);
	    */

#ifndef QT_NO_TRANSFORMATIONS
	    // TODO: set it other times, eg. when scrollbar moves view
	    QWMatrix wm = painter->worldMatrix();
	    setMicroFocusHint( int(wm.dx()+cXPos),
			       int (wm.dy()+cYPos),
			       1, fm.ascent() );
#else
	    setMicroFocusHint( cXPos,
			       cYPos,
			       1, fm.ascent() );
#endif
	}
    }
    p.end();
    painter->drawPixmap( updateR.left(), updateR.top(), *buffer,
			 0, 0, updateR.width(), updateR.height() );
}


/*!
    \overload
  Returns the width in pixels of the string \a s.
  NOTE: only appropriate for whole lines.
*/

int QtMultiLineEdit::textWidth( const QString &s )
{
    int w = 0;
    if ( !s.isNull() ) {
	w = textWidthWithTabs( QFontMetrics( font() ), s, 0, s.length(),
			       d->align );
    }
    return w + 2 * d->lr_marg + d->marg_extra;
}


/*!
  Returns the width in pixels of the text at line \a line.
*/

int QtMultiLineEdit::textWidth( int line )
{
    if ( d->echomode == Password) {
	QString s = stringShown(line);
	return textWidth( s );
    }
    QtMultiLineEditRow* r = contents->at(line);
    return r?r->w:0;
}

/*!
  Starts the cursor blinking.
*/

void QtMultiLineEdit::focusInEvent( QFocusEvent * )
{
    stopAutoScroll();
    if ( !d->blinkTimer->isActive() )
	d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
    cursorOn = TRUE;
    updateCell( cursorY, 0, FALSE );
}


/*!\reimp
*/
void QtMultiLineEdit::leaveEvent( QEvent * )
{
}


/*!\reimp
*/
void QtMultiLineEdit::focusOutEvent( QFocusEvent * )
{
    stopAutoScroll();
    d->blinkTimer->stop();
    if ( cursorOn ) {
	cursorOn = FALSE;
	updateCell( cursorY, 0, FALSE );
    }
}


/*!
  \reimp
  Present for binary compatibility only!
*/

void QtMultiLineEdit::timerEvent( QTimerEvent * )
{
    // ############ Remove in 3.0!!!!!!!!
}

#ifndef QT_NO_DRAGANDDROP
void QtMultiLineEdit::doDrag()
{
    if ( d->dnd_timer ) {
	d->dnd_timer->stop();
    }
    QDragObject *drag_text = new QTextDrag(markedText(), this);
    if ( readOnly ) {
	drag_text->dragCopy();
    } else {
	if ( drag_text->drag() && QDragObject::target() != this ) {
	    del();
	    if ( textDirty && !d->isHandlingEvent )
		emit textChanged();
	}
    }
    d->dnd_primed = FALSE;
}
#endif

/*!
  If there is marked text, sets \a line1, \a col1, \a line2 and \a col2
  to the start and end of the marked region and returns TRUE. Returns
  FALSE if there is no marked text.
 */
bool QtMultiLineEdit::getMarkedRegion( int *line1, int *col1,
				      int *line2, int *col2 ) const
{
    if ( !markIsOn || !line1 || !line2 || !col1 || !col2 )
	return FALSE;
    if ( markAnchorY < markDragY ||
	 markAnchorY == markDragY && markAnchorX < markDragX) {
	*line1 = markAnchorY;
	*col1 = markAnchorX;
	*line2 = markDragY;
	*col2 = markDragX;
	if ( *line2 > numLines() - 1 ) {
	    *line2 = numLines() - 1;
	    *col2 = lineLength( *line2 );
	}
    } else {
	*line1 = markDragY;
	*col1 = markDragX;
	*line2 = markAnchorY;
	*col2 = markAnchorX;
	if ( *line2 > numLines() - 1 ) {
	    *line2 = numLines() - 1;
	    *col2 = lineLength( *line2 );
	}
    }
    return markIsOn;
}


/*!
  Returns TRUE if there is marked text.
*/

bool QtMultiLineEdit::hasMarkedText() const
{
    return markIsOn;
}


/*!
  Returns a copy of the marked text.
*/

QString QtMultiLineEdit::markedText() const
{
    int markBeginX, markBeginY;
    int markEndX, markEndY;
    if ( !getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX ) )
	return QString();
    if ( markBeginY == markEndY ) { //just one line
	QString *s  = getString( markBeginY );
	return s->mid( markBeginX, markEndX - markBeginX );
    } else { //multiline
	QString *firstS, *lastS;
	firstS = getString( markBeginY );
	lastS  = getString( markEndY );
	int i;
	QString tmp;
	if ( firstS )
	    tmp += firstS->mid(markBeginX);
	if ( contents->at( markBeginY )->newline )
	    tmp += '\n';

	for( i = markBeginY + 1; i < markEndY ; i++ ) {
	    tmp += *getString(i);
	    if ( contents->at( i )->newline )
		tmp += '\n';
	}

	if ( lastS ) {
	    tmp += lastS->left(markEndX);
	} else {
	    tmp.truncate(tmp.length()-1);
	}

	return tmp;
    }
}



/*!
  Returns the text at line number \a line (possibly the empty string),
  or a \link QString::operator!() null string\endlink if \a line is invalid.
*/

QString QtMultiLineEdit::textLine( int line ) const
{
    QString *s = getString(line);
    if ( s ) {
	if ( s->isNull() )
	    return QString::fromLatin1("");
	else
	    return *s;
    } else
	return QString::null;
}


/*
  Returns a copy of the whole text. If the multi line edit contains no
  text, a
  \link QString::operator!() null string\endlink
  is returned.
*/

QString QtMultiLineEdit::text() const
{
    QString tmp;
    for( int i = 0 ; i < (int)contents->count() ; i++ ) {
	tmp += *getString(i);
	if ( i+1 < (int)contents->count() && contents->at(i)->newline )
	    tmp += '\n';
    }
    return tmp;
}


/*!
  Selects all text without moving the cursor.
*/

void QtMultiLineEdit::selectAll()
{
    markAnchorX    = 0;
    markAnchorY    = 0;
    markDragY = numLines() - 1;
    markDragX = lineLength( markDragY );
    turnMark( markDragX != markAnchorX || markDragY != markAnchorY );
    if ( autoUpdate() )
	update();
}



/*!
  Deselects all text (i.e. removes marking) and leaves the cursor at the
  current position.
*/

void QtMultiLineEdit::deselect()
{
    turnMark( FALSE );
}


/*
  Sets the text to \a s, removing old text, if any.
*/

void QtMultiLineEdit::setText( const QString &s )
{
    bool oldUndo = isUndoEnabled();
    setUndoEnabled( FALSE );
    bool oldAuto = autoUpdate();
    setAutoUpdate( FALSE );
    bool b = signalsBlocked();
    blockSignals( TRUE );
    clear();
    CLEAR_UNDO
    blockSignals( b );
    insertLine( s, -1 );
    emit textChanged();
    setAutoUpdate(oldAuto);
    if ( autoUpdate() )
	update();
    setUndoEnabled( oldUndo );
}


/*!
  Appends \a s to the text.
*/

void QtMultiLineEdit::append( const QString &s )
{
    bool oldUndo = isUndoEnabled();
    setUndoEnabled( FALSE );
    insertLine( s, -1 );
    setUndoEnabled( oldUndo );
    emit textChanged();
}

/*! \reimp
Passes wheel events to the vertical scrollbar.
*/
void QtMultiLineEdit::wheelEvent( QWheelEvent *e ){
    QApplication::sendEvent( verticalScrollBar(), e);
}


/*!
  The key press event handler converts a key press in event \a e to
  some line editor action.

  Here are the default key bindings when isReadOnly() is FALSE:
  \list
  \i <i> Left Arrow </i> Move the cursor one character leftwards
  \i <i> Right Arrow </i> Move the cursor one character rightwards
  \i <i> Up Arrow </i> Move the cursor one line upwards
  \i <i> Down Arrow </i> Move the cursor one line downwards
  \i <i> Page Up </i> Move the cursor one page upwards
  \i <i> Page Down </i> Move the cursor one page downwards
  \i <i> Backspace </i> Delete the character to the left of the cursor
  \i <i> Home </i> Move the cursor to the beginning of the line
  \i <i> End </i> Move the cursor to the end of the line
  \i <i> Delete </i> Delete the character to the right of the cursor
  \i <i> Shift - Left Arrow </i> Mark text one character leftwards
  \i <i> Shift - Right Arrow </i> Mark text one character rightwards
  \i <i> Control-A </i> Move the cursor to the beginning of the line
  \i <i> Control-B </i> Move the cursor one character leftwards
  \i <i> Control-C </i> Copy the marked text to the clipboard
  \i <i> Control-D </i> Delete the character to the right of the cursor
  \i <i> Control-E </i> Move the cursor to the end of the line
  \i <i> Control-F </i> Move the cursor one character rightwards
  \i <i> Control-H </i> Delete the character to the left of the cursor
  \i <i> Control-K </i> Delete to end of line
  \i <i> Control-N </i> Move the cursor one line downwards
  \i <i> Control-P </i> Move the cursor one line upwards
  \i <i> Control-V </i> Paste the clipboard text into line edit
  \i <i> Control-X </i> Cut the marked text, copy to clipboard
  \i <i> Control-Z </i> Undo the last operation
  \i <i> Control-Y </i> Redo the last operation
  \i <i> Control - Left Arrow </i> Move the cursor one word leftwards
  \i <i> Control - Right Arrow </i> Move the cursor one word rightwards
  \i <i> Control - Up Arrow </i> Move the cursor one word upwards
  \i <i> Control - Down Arrow </i> Move the cursor one word downwards
  \i <i> Control - Home Arrow </i> Move the cursor to the beginning of the text
  \i <i> Control - End Arrow </i> Move the cursor to the end of the text
  \endlist
  In addition, the following key bindings are used on Windows:
  \list
  \i <i> Shift - Delete </i> Cut the marked text, copy to clipboard
  \i <i> Shift - Insert </i> Paste the clipboard text into line edit
  \i <i> Control - Insert </i> Copy the marked text to the clipboard
  \endlist
  All other keys with valid ASCII codes insert themselves into the line.

  Here are the default key bindings when isReadOnly() is TRUE:
  \list
  \i <i> Left Arrow </i> Scrolls the table rightwards
  \i <i> Right Arrow </i> Scrolls the table rightwards
  \i <i> Up Arrow </i> Scrolls the table one line downwards
  \i <i> Down Arrow </i> Scrolls the table one line upwards
  \i <i> Page Up </i> Scrolls the table one page downwards
  \i <i> Page Down </i> Scrolls the table one page upwards
  \i <i> Control-C </i> Copy the marked text to the clipboard
  \endlist

*/

void QtMultiLineEdit::keyPressEvent( QKeyEvent *e )
{
    textDirty = FALSE;
    d->isHandlingEvent = TRUE;
    int unknown = 0;
    if ( readOnly ) {
	int pageSize = viewHeight() / cellHeight();

	switch ( e->key() ) {
	case Key_Left:
	    setXOffset( xOffset() - viewWidth()/10 );
	    break;
	case Key_Right:
	    setXOffset( xOffset() + viewWidth()/10 );
	    break;
	case Key_Up:
	    setTopCell( topCell() - 1 );
	    break;
	case Key_Down:
	    setTopCell( topCell() + 1 );
	    break;
	case Key_Home:
	    setCursorPosition(0,0, e->state() & ShiftButton );
	    break;
	case Key_End:
	    setCursorPosition( numLines()-1, lineLength( numLines()-1 ),
			       e->state() & ShiftButton );
	    break;
        case Key_Next:
	    setTopCell( topCell() + pageSize );
	    break;
	case Key_Prior:
	    setTopCell( QMAX( topCell() - pageSize, 0 ) );
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_C:
	    if ( echoMode() == Normal && (e->state()&ControlButton) )
		copy();
	    else
		unknown++;
	    break;
	case Key_F16: // Copy key on Sun keyboards
	    if ( echoMode() == Normal )
		copy();
	    else
		unknown++;
	    break;
#endif
	default:
	    unknown++;
	}
	if ( unknown )
	    e->ignore();
	d->isHandlingEvent = FALSE;
	return;
    }
    if ( e->text().length() &&
	 e->key() != Key_Return &&
	 e->key() != Key_Enter &&
	 e->key() != Key_Delete &&
	 e->key() != Key_Backspace &&
	 (!e->ascii() || e->ascii()>=32)
	 ) {
	insert( e->text() );
	//QApplication::sendPostedEvents( this, QEvent::Paint );
	if ( textDirty )
	    emit textChanged();
	d->isHandlingEvent = FALSE;
	return;
    }
    if ( e->state() & ControlButton ) {
	switch ( e->key() ) {
	case Key_A:
	    home( e->state() & ShiftButton );
	    break;
	case Key_B:
	    cursorLeft( e->state() & ShiftButton );
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_C:
	    if ( echoMode() == Normal )
		copy();
	    break;
#endif
	case Key_D:
	    del();
	    break;
	case Key_E:
	    end( e->state() & ShiftButton );
	    break;
	case Key_Left:
	    cursorWordBackward( e->state() & ShiftButton );
	    break;
	case Key_Right:
	    cursorWordForward( e->state() & ShiftButton );
	    break;
	case Key_Up:
	    cursorUp( e->state() & ShiftButton );
	    break;
	case Key_Down:
	    cursorDown( e->state() & ShiftButton );
	    break;
	case Key_Home:
	    setCursorPosition(0,0, e->state() & ShiftButton );
	    break;
	case Key_End:
	    setCursorPosition( numLines()-1, lineLength( numLines()-1 ),
			       e->state() & ShiftButton );
	    break;
	case Key_F:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_H:
	    backspace();
	    break;
	case Key_K:
	    killLine();
	    break;
	case Key_N:
	    cursorDown( e->state() & ShiftButton );
	    break;
	case Key_P:
	    cursorUp( e->state() & ShiftButton );
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_V:
	    paste();
	    break;
	case Key_X:
	    cut();
	    break;
#endif
	case Key_Z:
	    undo();
	    break;
	case Key_Y:
	    redo();
	    break;
#if defined (Q_WS_WIN)
	case Key_Insert:
	    copy();
#endif
	default:
	    unknown++;
	}
    } else {
	switch ( e->key() ) {
	case Key_Left:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_Right:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_Up:
	    cursorUp( e->state() & ShiftButton );
	    break;
	case Key_Down:
	    cursorDown( e->state() & ShiftButton );
	    break;
	case Key_Backspace:
	    backspace();
	    break;
	case Key_Home:
	    home( e->state() & ShiftButton );
	    break;
	case Key_End:
	    end( e->state() & ShiftButton );
	    break;
	case Key_Delete:
#if defined (Q_WS_WIN)
	    if ( e->state() & ShiftButton ) {
		cut();
		break;
	    }
#endif
	    del();
	    break;
	case Key_Next:
	    pageDown( e->state() & ShiftButton );
	    break;
	case Key_Prior:
	    pageUp( e->state() & ShiftButton );
	    break;
	case Key_Enter:
	case Key_Return:
	    newLine();
	    emit returnPressed();
	    break;
	case Key_Tab:
	    insert( e->text() );
	    break;
#if defined (Q_WS_WIN)
	case Key_Insert:
	    if ( e->state() & ShiftButton )
		paste();
	    else
		unknown++;
	    break;
#endif
	case Key_F14: // Undo key on Sun keyboards
	    undo();
	    break;
#ifndef QT_NO_CLIPBOARD
	case Key_F16: // Copy key on Sun keyboards
	    if ( echoMode() == Normal )
		copy();
	    break;
	case Key_F18: // Paste key on Sun keyboards
	    paste();
	    break;
	case Key_F20: // Paste key on Sun keyboards
	    cut();
	    break;
#endif
	default:
	    unknown++;
	}
    }
    if ( textDirty )
	emit textChanged();

    if ( unknown )				// unknown key
	e->ignore();

    d->isHandlingEvent = FALSE;
}


/*!
  Moves the cursor one page down.  If \a mark is TRUE, the text
  is marked.
*/

void QtMultiLineEdit::pageDown( bool mark )
{
    bool oldAuto = autoUpdate();
    if ( mark )
	setAutoUpdate( FALSE );

    if ( partiallyInvisible( cursorY ) )
	cursorY = topCell();
    int delta = cursorY - topCell();
    int pageSize = viewHeight() / cellHeight();
    int newTopCell = QMIN( topCell() + pageSize, numLines() - 1 - pageSize );

    if ( pageSize >= numLines() ) { // quick fix to handle small texts
	newTopCell = topCell();
    }
    if ( !curXPos )
	curXPos = mapToView( cursorX, cursorY );
    int oldY = cursorY;

    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    if ( newTopCell != topCell() ) {
	cursorY = newTopCell + delta;
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	setTopCell( newTopCell );
    } else if ( cursorY != (int)contents->count() - 1) { // just move the cursor
	cursorY = QMIN( cursorY + pageSize, numLines() - 1);
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	makeVisible();
    }
    if ( oldAuto )
	if ( mark ) {
	    setAutoUpdate( TRUE );
	    update();
	} else {
	    updateCell( oldY, 0, FALSE );
	}
    if ( !mark )
	turnMark( FALSE );
}


/*!
  Moves the cursor one page up.  If \a mark is TRUE, the text
  is marked.
*/

void QtMultiLineEdit::pageUp( bool mark )
{
    bool oldAuto = autoUpdate();
    if ( mark )
	setAutoUpdate( FALSE );
    if ( partiallyInvisible( cursorY ) )
	cursorY = topCell();
    int delta = cursorY - topCell();
    int pageSize = viewHeight() / cellHeight();
    bool partial = delta == pageSize && viewHeight() != pageSize * cellHeight();
    int newTopCell = QMAX( topCell() - pageSize, 0 );
    if ( pageSize > numLines() ) { // quick fix to handle small texts
	newTopCell = 0;
	delta = 0;
    }
    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    if ( !curXPos )
	curXPos = mapToView( cursorX, cursorY );
    int oldY = cursorY;
    if ( newTopCell != topCell() ) {
	cursorY = QMIN( newTopCell + delta, numLines() - 1 );
	if ( partial )
	    cursorY--;
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMAX( cursorY - pageSize, 0 );
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
    }
    if ( oldAuto )
	if ( mark ) {
	    setAutoUpdate( TRUE );
	    update();
	} else {
	    updateCell( oldY, 0, FALSE );
	}
    if ( !mark )
	turnMark( FALSE );
}

// THE CORE INSERTION FUNCTION
void QtMultiLineEdit::insertAtAux( const QString &txt, int line, int col, bool mark )
{
    dummy = FALSE;
    d->blinkTimer->stop();
    cursorOn = TRUE;
    int oldw = contentsRect().width();

    line = QMAX( QMIN( line, numLines() - 1), 0 );
    col = QMAX( QMIN( col,  lineLength( line )), 0 );

    QString itxt = txt;
    QtMultiLineEditRow  *row = contents->at( line );
    if ( d->maxlen >= 0 && length() + int(txt.length()) > d->maxlen )
	itxt.truncate( d->maxlen - length() );

    row->s.insert( uint(col), itxt );

    if ( mark ) {
	markAnchorX = col;
	markAnchorY = line;
    }
    if ( cursorX == col && cursorY == line ) {
	cursorX += itxt.length();
    }
    QFontMetrics fm( font() );
    if ( !WORD_WRAP || ( col == 0 && itxt.contains('\n') == int(itxt.length())) )
	wrapLine( line, 0 );
    else if ( WORD_WRAP && itxt.find('\n')<0 && itxt.find('\t')<0
	      && (
		  ( DYNAMIC_WRAP && fm.width( itxt ) + row->w < contentsRect().width() -  2*d->lr_marg - d->marg_extra )
		  ||
		  ( FIXED_WIDTH_WRAP && ( d->wrapcol < 0 || fm.width( itxt ) + row->w < d->wrapcol ) )
		  ||
		  ( FIXED_COLUMN_WRAP && ( d->wrapcol < 0 || int(row->s.length()) < d->wrapcol ) )
		  )
	      && ( itxt.find(' ') < 0 || row->s.find(' ') >= 0 && row->s.find(' ') < col ) ){
	row->w = textWidth( row->s );
	setWidth( QMAX( maxLineWidth(), row->w) );
	updateCell( line, 0, FALSE );
    }
    else {
	if ( line > 0 && !contents->at( line-1)->newline )
	    rebreakParagraph( line-1 );
	else
	    rebreakParagraph( line );
    }
    if ( mark )
	newMark( cursorX, cursorY, FALSE );

    setNumRowsAndTruncate();

    textDirty = TRUE;
    d->edited = TRUE;
    if ( autoUpdate() ) {
	makeVisible();
	d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
	if ( DYNAMIC_WRAP && oldw != contentsRect().width() ) {
	    setAutoUpdate( FALSE );
	    rebreakAll();
	    setAutoUpdate( TRUE );
	    update();
	}
    }
}


/*!
  Inserts \a txt at line number \a line. If \a line is less than zero,
  or larger than the number of rows, the new text is put at the end.
  If \a txt contains newline characters, several lines are inserted.

  The cursor position is not changed.
*/

void QtMultiLineEdit::insertLine( const QString &txt, int line )
{
    QString s = txt;
    int oldXPos = cursorX;
    int oldYPos = cursorY;
    if ( line < 0 || line >= int( contents->count() ) ) {
	if ( !dummy )
	    contents->append( new QtMultiLineEditRow(QString::fromLatin1(""), 0) );
	insertAt( s, numLines()-1, 0 );
    } else {
	s.append('\n');
	insertAt( s, line, 0 );
    }
    cursorX = oldXPos;
    cursorY = oldYPos;
}

/*!
  Deletes the line at line number \a line. If \a
  line is less than zero, or larger than the number of lines,
  no line is deleted.
*/

void QtMultiLineEdit::removeLine( int line )
{
    CLEAR_UNDO
    if ( line >= numLines()  )
	return;
    if ( cursorY >= line && cursorY > 0 )
	cursorY--;
    bool updt = autoUpdate() && rowIsVisible( line );
    QtMultiLineEditRow* r = contents->at( line );
    ASSERT( r );
    bool recalc = r->w == maxLineWidth();
    contents->remove( line );
    if ( contents->count() == 0 ) {
	int w  = textWidth( QString::fromLatin1("") );
	contents->append( new QtMultiLineEditRow(QString::fromLatin1(""), w) );
	setWidth( w );
	dummy = TRUE;
    }
    if ( setNumRowsAndTruncate() )
	recalc = updt = FALSE;
    if ( recalc )
	updateCellWidth();
    makeVisible();
    if (updt)
	update();
    textDirty = TRUE;
    d->edited = TRUE;
}

/*!
  Inserts \a s at the current cursor position.
*/
void QtMultiLineEdit::insert( const QString& s )
{
    insert( s, FALSE );
}

/*!
  Inserts \a c at the current cursor position.
  (this function is provided for backward compatibility -
  it simply calls insert()).
*/
void QtMultiLineEdit::insertChar( QChar c )
{
    insert(c);
}

/*!
    \overload
  Inserts string \a str at the current cursor position. If \a mark is
  TRUE the string is marked.
*/

void QtMultiLineEdit::insert( const QString& str, bool mark )
{
    dummy = FALSE;
    bool wasMarkedText = hasMarkedText();
    if ( wasMarkedText )
	addUndoCmd( new QBeginCommand );
    if ( wasMarkedText )
	del();					// ## Will flicker
    QString *s = getString( cursorY );
    if ( cursorX > (int)s->length() )
	cursorX = s->length();
    else if ( overWrite && !wasMarkedText && cursorX < (int)s->length() )
	del();                                 // ## Will flicker
    insertAt(str, cursorY, cursorX, mark );
    makeVisible();

    if ( wasMarkedText )
	addUndoCmd( new QEndCommand() );
}

/*!
  Makes a line break at the current cursor position.
*/

void QtMultiLineEdit::newLine()
{
    insert("\n");
}

/*!
  Deletes text from the current cursor position to the end of the line.
*/

void QtMultiLineEdit::killLineAux()
{
    deselect(); // -sanders Don't let del() delete marked region
    QtMultiLineEditRow* r = contents->at( cursorY );
    if ( cursorX == (int)r->s.length() ) {
	//      if (r->newline) // -sanders Only del newlines!
	del();
	return;
    } else {
	bool recalc = r->w == maxLineWidth();
	r->s.remove( cursorX, r->s.length() );
	r->w = textWidth( r->s );
	updateCell( cursorY, 0, FALSE );
	if ( recalc )
	    updateCellWidth();
	rebreakParagraph( cursorY ); // -sanders
	textDirty = TRUE;
	d->edited = TRUE;
    }
    curXPos  = 0;
    makeVisible();
    turnMark( FALSE );
}


/*!
  Moves the cursor one character to the left. If \a mark is TRUE, the text
  is marked. If \a wrap is TRUE, the cursor moves to the end of the
  previous line  if it is placed at the beginning of the current line.

  \sa cursorRight() cursorUp() cursorDown()
*/

void QtMultiLineEdit::cursorLeft( bool mark, bool wrap )
{
    cursorLeft(mark,!mark,wrap);
}
void QtMultiLineEdit::cursorLeft( bool mark, bool clear_mark, bool wrap )
{
    if ( cursorX != 0 || cursorY != 0 && wrap ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	d->blinkTimer->stop();
	int ll = lineLength( cursorY );
	if ( cursorX > ll )
	    cursorX = ll;
	cursorOn = TRUE;
	cursorX--;
	if ( cursorX < 0 ) {
	    int oldY = cursorY;
	    if ( cursorY > 0 ) {
		cursorY--;
		cursorX = lineLength( cursorY );
		if ( cursorX > 1 && !isEndOfParagraph( cursorY ) )
		    cursorX--;
	    } else {
		cursorY = 0; //### ?
		cursorX = 0;
	    }
	    updateCell( oldY, 0, FALSE );
	}
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
	updateCell( cursorY, 0, FALSE );
    }
    curXPos  = 0;
    makeVisible();
    if ( clear_mark )
	turnMark( FALSE );
}

/*!
  Moves the cursor one character to the right.  If \a mark is TRUE, the text
  is marked. If \a wrap is TRUE, the cursor moves to the beginning of the next
  line if it is placed at the end of the current line.
  \sa cursorLeft() cursorUp() cursorDown()
*/

void QtMultiLineEdit::cursorRight( bool mark, bool wrap )
{
    cursorRight(mark,!mark,wrap);
}
void QtMultiLineEdit::cursorRight( bool mark, bool clear_mark, bool wrap )
{
    int strl = lineLength( cursorY );
    if ( strl > 1 && !isEndOfParagraph( cursorY ) )
	 strl--;
    if ( cursorX < strl || cursorY < (int)contents->count() - 1 && wrap ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	d->blinkTimer->stop();
	cursorOn = TRUE;
	cursorX++;
	if ( cursorX > strl ) {
	    int oldY = cursorY;
	    if ( cursorY < (int) contents->count() - 1 ) {
		cursorY++;
		cursorX = 0;
	    } else {
		cursorX = lineLength( cursorY );
	    }
	    updateCell( oldY, 0, FALSE );
	}
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( cursorY, 0, FALSE );
	d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
    }
    curXPos  = 0;
    makeVisible();
    if ( clear_mark )
	turnMark( FALSE );
}

/*!
  Moves the cursor up one line.  If \a mark is TRUE, the text
  is marked.
  \sa cursorDown() cursorLeft() cursorRight()
*/

void QtMultiLineEdit::cursorUp( bool mark )
{
    cursorUp(mark,!mark);
}
void QtMultiLineEdit::cursorUp( bool mark, bool clear_mark )
{
    if ( cursorY != 0 ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	if ( !curXPos )
	    curXPos = mapToView( cursorX, cursorY );
	int oldY = cursorY;
	d->blinkTimer->stop();
	cursorOn = TRUE;
	cursorY--;
	if ( cursorY < 0 ) {
	    cursorY = 0;
	}
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( oldY, 0, FALSE );
	updateCell( cursorY, 0, FALSE );
	d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
    }
    makeVisible();
    if ( clear_mark )
	turnMark( FALSE );
}

/*!
  Moves the cursor one line down.  If \a mark is TRUE, the text
  is marked.
  \sa cursorUp() cursorLeft() cursorRight()
*/

void QtMultiLineEdit::cursorDown( bool mark )
{
    cursorDown(mark,!mark);
}
void QtMultiLineEdit::cursorDown( bool mark, bool clear_mark )
{
    int lastLin = contents->count() - 1;
    if ( cursorY != lastLin ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	if ( !curXPos )
	    curXPos = mapToView( cursorX, cursorY );
	int oldY = cursorY;
	d->blinkTimer->stop();
	cursorOn = TRUE;
	cursorY++;
	if ( cursorY > lastLin ) {
	    cursorY = lastLin;
	}
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( oldY, 0, FALSE );
	updateCell( cursorY, 0, FALSE );
	d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
    }
    makeVisible();
    if ( clear_mark )
	turnMark( FALSE );
}

/*!
  Turns off marked text
*/
void QtMultiLineEdit::turnMark( bool on )
{
    if ( on != markIsOn ) {
	markIsOn = on;
	if ( echoMode() == Normal )
	    emit copyAvailable( on );
	update();
    }
}




/*!
  Deletes the character on the left side of the text cursor and moves
  the cursor one position to the left. If a text has been marked by
  the user (e.g. by clicking and dragging) the cursor is put at the
  beginning of the marked text and the marked text is removed.
  \sa del()
*/

void QtMultiLineEdit::backspace()
{
    if ( hasMarkedText() ) {
	del();
    } else {
	if ( !atBeginning() ) {
	    cursorLeft( FALSE );
	    del();
	}
    }
    makeVisible();
}

void QtMultiLineEdit::delAux()
{
    int markBeginX, markBeginY;
    int markEndX, markEndY;
    QRect oldContents = contentsRect();
    if ( getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX ) ) {
	turnMark( FALSE );
	textDirty = TRUE;
	d->edited = TRUE;
	if ( markBeginY == markEndY ) { //just one line
	    QtMultiLineEditRow *r = contents->at( markBeginY );
	    ASSERT(r);
	    bool recalc = r->w == maxLineWidth();
	    r->s.remove( markBeginX, markEndX - markBeginX );
	    r->w = textWidth( r->s );
	    cursorX  = markBeginX;
	    cursorY  = markBeginY;

	    if (autoUpdate() )
		updateCell( cursorY, 0, FALSE );
	    if ( recalc )
		updateCellWidth();
	} else { //multiline
	    bool oldAuto = autoUpdate();
	    setAutoUpdate( FALSE );
	    ASSERT( markBeginY >= 0);
	    ASSERT( markEndY < (int)contents->count() );

	    QtMultiLineEditRow *firstR, *lastR;
	    firstR = contents->at( markBeginY );
	    lastR  = contents->at( markEndY );
	    ASSERT( firstR != lastR );
	    firstR->s.remove( markBeginX, firstR->s.length() - markBeginX  );
	    lastR->s.remove( 0, markEndX  );
	    firstR->s.append( lastR->s );  // lastS will be removed in loop below
	    firstR->newline = lastR->newline; // Don't forget this -sanders
	    firstR->w = textWidth( firstR->s );

	    for( int i = markBeginY + 1 ; i <= markEndY ; i++ )
		contents->remove( markBeginY + 1 );

	    if ( contents->isEmpty() )
		insertLine( QString::fromLatin1(""), -1 );

	    cursorX  = markBeginX;
	    cursorY  = markBeginY;
	    curXPos  = 0;

	    setNumRowsAndTruncate();
	    updateCellWidth();
	    setAutoUpdate( oldAuto );
	    if ( autoUpdate() )
		update();
	}
	markAnchorY = markDragY = cursorY;
	markAnchorX = markDragX = cursorX;
    } else {
	if ( !atEnd() ) {
	    textDirty = TRUE;
	    d->edited = TRUE;
	    QtMultiLineEditRow *r = contents->at( cursorY );
	    if ( cursorX == (int) r->s.length() ) { // remove newline
		QtMultiLineEditRow* other = contents->at( cursorY + 1 );
		if ( ! r->newline && cursorX )
		    r->s.truncate( r->s.length()-1 );

		bool needBreak = !r->s.isEmpty();
		r->s += other->s;
		r->newline =  other->newline;
		contents->remove( cursorY + 1 );
		if ( needBreak )
		    rebreakParagraph( cursorY, 1 );
		else
		    wrapLine( cursorY, 1 );
	    } else {
		bool recalc = r->w == maxLineWidth();
		r->s.remove( cursorX, 1 );
		rebreakParagraph( cursorY );
		if ( recalc )
		    updateCellWidth();
	    }
	}
    }
    if ( DYNAMIC_WRAP && oldContents != contentsRect() ) {
	if ( oldContents.width() != contentsRect().width() ) {
	    bool oldAuto = autoUpdate();
	    setAutoUpdate( FALSE );
	    rebreakAll();
	    setAutoUpdate( oldAuto );
	}
	if ( autoUpdate() )
	    update();
    }
    curXPos  = 0;
    makeVisible();
}

/*!
  Moves the text cursor to the left end of the line. If \a mark is
  TRUE, text is marked towards the first position. If it is FALSE and
  the cursor is moved, all marked text is unmarked.

  \sa end()
*/

void QtMultiLineEdit::home( bool mark )
{
    if ( cursorX != 0 ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	d->blinkTimer->stop();
	cursorX = 0;
	cursorOn = TRUE;
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( cursorY, 0, FALSE );
	d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
    }
    curXPos  = 0;
    if ( !mark )
	turnMark( FALSE );
    makeVisible();
}

/*!
  Moves the text cursor to the right end of the line. If \a mark is TRUE
  text is marked towards the last position.  If it is FALSE and the
  cursor is moved, all marked text is unmarked.

  \sa home()
*/

void QtMultiLineEdit::end( bool mark )
{
    int tlen = lineLength( cursorY );
    if ( cursorX != tlen ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	d->blinkTimer->stop();
	cursorX = tlen;
	cursorOn  = TRUE;
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
	updateCell( cursorY, 0, FALSE );
    }
    curXPos  = 0;
    makeVisible();
    if ( !mark )
	turnMark( FALSE );
}

/*!\reimp
*/
void QtMultiLineEdit::mousePressEvent( QMouseEvent *e )
{
    stopAutoScroll();
    d->dnd_startpos = e->pos();

    if ( e->button() == RightButton ) {
	QPopupMenu *popup = new QPopupMenu( this );
	int id[ (int)IdCount ];
	id[ IdUndo ] = popup->insertItem( tr( "Undo" ) );
	id[ IdRedo ] = popup->insertItem( tr( "Redo" ) );
	popup->insertSeparator();
#ifndef QT_NO_CLIPBOARD
	id[ IdCut ] = popup->insertItem( tr( "Cut" ) );
	id[ IdCopy ] = popup->insertItem( tr( "Copy" ) );
	id[ IdPaste ] = popup->insertItem( tr( "Paste" ) );
#ifndef QT_NO_MIMECLIPBOARD
	id[ IdPasteSpecial ] = popup->insertItem( tr( "Paste special..." ) );
#endif
#endif
	id[ IdClear ] = popup->insertItem( tr( "Clear" ) );
	popup->insertSeparator();
	id[ IdSelectAll ] = popup->insertItem( tr( "Select All" ) );
	popup->setItemEnabled( id[ IdUndo ],
				  !this->d->undoList.isEmpty() );
	popup->setItemEnabled( id[ IdRedo ],
				  !this->d->redoList.isEmpty() );
#ifndef QT_NO_CLIPBOARD
	popup->setItemEnabled( id[ IdCut ],
			          !isReadOnly() && hasMarkedText() );
	popup->setItemEnabled( id[ IdCopy ], hasMarkedText() );
	popup->setItemEnabled( id[ IdPaste ],
	    !isReadOnly() && (bool)QApplication::clipboard()->text().length() );
#ifndef QT_NO_MIMECLIPBOARD
	// Any non-plain types?
	QMimeSource* ms = QApplication::clipboard()->data();
	bool ps = FALSE;
	if ( ms )  {
	    if ( !isReadOnly() ) {
		const char* fmt;
		for (int i=0; !ps && (fmt=ms->format(i)); i++) {
		    ps = qstrnicmp(fmt,"text/",5)==0
		      && qstrnicmp(fmt+5,"plain",5)!=0;
		}
	    }
	}
	popup->setItemEnabled( id[ IdPasteSpecial ], ps );
#endif
#endif
	popup->setItemEnabled( id[ IdClear ],
				  !isReadOnly() && (bool)text().length() );
	int allSelected = markIsOn && markAnchorX == 0 && markAnchorY == 0 &&
			  markDragY == numLines() - 1 && markDragX == lineLength( markDragY );
	popup->setItemEnabled( id[ IdSelectAll ],
				  (bool)text().length() && !allSelected );

	int r = popup->exec( e->globalPos() );
	delete popup;

	if ( r == id[ IdUndo ] )
	    undo();
	else if ( r == id[ IdRedo ] )
	    redo();
#ifndef QT_NO_CLIPBOARD
	else if ( r == id[ IdCut ] )
	    cut();
	else if ( r == id[ IdCopy ] )
	    copy();
	else if ( r == id[ IdPaste ] )
	    paste();
# ifndef QT_NO_MIMECLIPBOARD
	else if ( r == id[ IdPasteSpecial ] )
	    pasteSpecial(QCursor::pos());
# endif
#endif
	else if ( r == id[ IdClear ] )
	    clear();
	else if ( r == id[ IdSelectAll ] )
	    selectAll();
	return;
    }

    if ( e->button() != MidButton && e->button() != LeftButton)
	return;

    int newX, newY;
    pixelPosToCursorPos( e->pos(), &newX, &newY );

    if ( e->state() & ShiftButton ) {
	wordMark = FALSE;
	dragMarking    = TRUE;
	setCursorPosition( newY, newX, TRUE);
	return;
    }

#ifndef QT_NO_DRAGANDDROP
    if (
	inMark(newX, newY)		// Click on highlighted text
	&& echoMode() == Normal		// No DnD of passwords, etc.
	&& e->pos().y() < totalHeight() // Click past the end is not dragging
	)
    {
	// The user might be trying to drag
	d->dnd_primed = TRUE;
	d->dnd_timer->start( QApplication::startDragTime(), FALSE );
    } else
#endif
    {
	wordMark = FALSE;
	dragMarking    = TRUE;
	setCursorPixelPosition(e->pos());
    }
}

void QtMultiLineEdit::pixelPosToCursorPos(QPoint p, int* x, int* y) const
{
    *y = findRow( p.y() );
    if ( *y < 0 ) {
	if ( p.y() < lineWidth() ) {
	    *y = topCell();
	} else {
	    *y = lastRowVisible();
	    p.setX(cellWidth());
	}
    }
    *y = QMIN( (int)contents->count() - 1, *y );
    QFontMetrics fm( font() );
    *x = xPosToCursorPos( stringShown( *y ), fm,
			  p.x() - d->lr_marg + xOffset(),
			  cellWidth() - 2 * d->lr_marg - d->marg_extra,
			  d->align );
    QtMultiLineEditRow* r = contents->at( *y );
    if (r && !r->newline && ((unsigned int)*x == r->s.length()) && (*x > 0))
	--*x;
}

void QtMultiLineEdit::setCursorPixelPosition(QPoint p, bool clear_mark)
{
    int newY;
    pixelPosToCursorPos( p, &cursorX, &newY );
    curXPos        = 0;
    if ( clear_mark ) {
	markAnchorX    = cursorX;
	markAnchorY    = newY;
	bool markWasOn = markIsOn;
	turnMark( FALSE );
	if ( markWasOn ) {
	    cursorY = newY;
	    update();
	    d->isHandlingEvent = FALSE;
	    return;
	}
    }
    if ( cursorY != newY ) {
	int oldY = cursorY;
	cursorY = newY;
	updateCell( oldY, 0, FALSE );
    }
    updateCell( cursorY, 0, FALSE );		// ###
}

void QtMultiLineEdit::startAutoScroll()
{
    if ( !dragScrolling ) {
	d->scrollTime = initialScrollTime;
	d->scrollAccel = initialScrollAccel;
	d->scrollTimer->start( d->scrollTime, FALSE );
	dragScrolling = TRUE;
    }
}

void QtMultiLineEdit::stopAutoScroll()
{
    if ( dragScrolling ) {
	d->scrollTimer->stop();
	dragScrolling = FALSE;
    }
}

/*!\reimp
*/
void QtMultiLineEdit::mouseMoveEvent( QMouseEvent *e )
{
#ifndef QT_NO_DRAGANDDROP
    d->dnd_timer->stop();
    if ( d->dnd_primed &&
	 ( d->dnd_startpos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
	doDrag();
	return;
    }
#endif
    if ( !dragMarking )
	return;
    if ( rect().contains( e->pos() ) ) {
	stopAutoScroll();
    } else if ( !dragScrolling ) {
	startAutoScroll();
    }

    int newX, newY;
    pixelPosToCursorPos(e->pos(), &newX, &newY);

    if ( wordMark ) {
	extendSelectionWord( newX, newY);
    }

    if ( markDragX == newX && markDragY == newY )
	return;
    int oldY = markDragY;
    newMark( newX, newY, FALSE );
    for ( int i = QMIN(oldY,newY); i <= QMAX(oldY,newY); i++ )
	updateCell( i, 0, FALSE );
}

void QtMultiLineEdit::extendSelectionWord( int &newX, int&newY)
{
    QString s = stringShown( newY );
    int lim = s.length();
    if ( newX >= 0 && newX < lim ) {
	int i = newX;
	int startclass = charClass(s.at(i));
	if ( markAnchorY < markDragY ||
	     ( markAnchorY == markDragY && markAnchorX < markDragX ) ) {
	    // going right
	    while ( i < lim && charClass(s.at(i)) == startclass )
		i++;
	} else {
	    // going left
	    while ( i >= 0 && charClass(s.at(i)) == startclass )
		i--;
	    i++;
	}
	newX = i;
    }
}




/*!\reimp
*/
void QtMultiLineEdit::mouseReleaseEvent( QMouseEvent *e )
{
    stopAutoScroll();
#ifndef QT_NO_DRAGANDDROP
    if ( d->dnd_timer->isActive() ) {
	d->dnd_timer->stop();
	d->dnd_primed = FALSE;
	setCursorPixelPosition(e->pos());
    }
#endif
    wordMark = FALSE;
    dragMarking   = FALSE;
    textDirty = FALSE;
    d->isHandlingEvent = TRUE;
    if ( markAnchorY == markDragY && markAnchorX == markDragX )
	turnMark( FALSE );

#ifndef QT_NO_CLIPBOARD
#if defined(Q_WS_X11)
    else if ( echoMode() == Normal )
	copy();
#endif

    if ( e->button() == MidButton && !readOnly ) {
#if defined(Q_WS_X11)
	paste();		// Will repaint the cursor line.
#else
#ifndef QT_NO_COMPAT
	if ( style().styleHint(QStyle::SH_GUIStyle) == Qt::MotifStyle )
	    paste();
#endif
#endif
    }
#endif

    d->isHandlingEvent = FALSE;

    if ( !readOnly && textDirty )
	emit textChanged();
}


/*!\reimp
*/
void QtMultiLineEdit::mouseDoubleClickEvent( QMouseEvent *m )
{
    if ( m->button() == LeftButton ) {
	if ( m->state() & ShiftButton ) {
	    int newX = cursorX;
	    int newY = cursorY;
	    extendSelectionWord( newX, newY);
	    newMark( newX, newY, FALSE );
	} else {
	    markWord( cursorX, cursorY );
	}
	dragMarking    = TRUE;
	wordMark = TRUE;
	updateCell( cursorY, 0, FALSE );

    }
}

#ifndef QT_NO_DRAGANDDROP

/*!
  \reimp
*/

void QtMultiLineEdit::dragEnterEvent( QDragEnterEvent * )
{
    cursorOn = TRUE;
    updateCell( cursorY, 0, FALSE );
}

/*!\reimp
*/
void QtMultiLineEdit::dragMoveEvent( QDragMoveEvent* event )
{
    if ( readOnly ) return;
    event->accept( QTextDrag::canDecode(event) );
    d->dnd_forcecursor = TRUE;
    setCursorPixelPosition(event->pos(), FALSE);
    d->dnd_forcecursor = FALSE;
    QRect inside_margin(scroll_margin, scroll_margin,
			width()-scroll_margin*2, height()-scroll_margin*2);
    if ( !inside_margin.contains(event->pos()) ) {
	startAutoScroll();
    }
    if ( event->source() == this && event->action() == QDropEvent::Move )
	event->acceptAction();
}

/*!\reimp
*/
void QtMultiLineEdit::dragLeaveEvent( QDragLeaveEvent* )
{
    if ( cursorOn ) {
	cursorOn = FALSE;
	updateCell( cursorY, 0, FALSE );
    }
    stopAutoScroll();
}

/*!\reimp
*/
void QtMultiLineEdit::dropEvent( QDropEvent* event )
{
    if ( readOnly ) return;
    QString text;
    QCString fmt = pickSpecial(event,FALSE,event->pos());
    if ( QTextDrag::decode(event, text, fmt) ) {
	int i = -1;
	while ( ( i = text.find( '\r' ) ) != -1 )
	    text.replace( i,1,"" );
	if ( event->source() == this && event->action() == QDropEvent::Move ) {
	    event->acceptAction();
	    // Careful not to tread on my own feet
	    int newX, newY;
	    pixelPosToCursorPos( event->pos(), &newX, &newY );
	    if ( afterMark( newX, newY ) ) {
		// The tricky case
		int x1, y1, x2, y2;
		getMarkedRegion( &y1, &x1, &y2, &x2 );
		if ( newY == y2 ) {
		    newY = y1;
		    newX = x1 + newX - x2;
		} else {
		    newY -= y2 - y1;
		}
		addUndoCmd( new QBeginCommand );
		del();
		setCursorPosition(newY, newX);
		insert(text, TRUE);
		addUndoCmd( new QEndCommand );
	    } else if ( beforeMark( newX, newY ) ) {
		// Easy
		addUndoCmd( new QBeginCommand );
		del();
		setCursorPosition(newY, newX);
		insert(text, TRUE);
		addUndoCmd( new QEndCommand );
	    } else {
		// Do nothing.
	    }
	} else {
	    setCursorPixelPosition(event->pos());
	    insert(text, TRUE);
	}
	update();
	emit textChanged();
    }
}

#endif // QT_NO_DRAGANDDROP


/*!
  Returns TRUE if line \a line is invisible or partially invisible.
*/

bool QtMultiLineEdit::partiallyInvisible( int line )
{
    int y;
    if ( !rowYPos( line, &y ) )
	return TRUE;
    if ( y < 0 )
	return TRUE;
    else if ( y + cellHeight() - 2 > viewHeight() )
	return TRUE;

    return FALSE;
}

/*!
  Scrolls such that the cursor is visible
*/

void QtMultiLineEdit::makeVisible()
{
    if ( !autoUpdate() )
	return;

    if ( partiallyInvisible( cursorY ) ) {
	if ( cursorY >= lastRowVisible() )
	    setBottomCell( cursorY );
	else
	    setTopCell( cursorY );
    }
    int xPos = mapToView( cursorX, cursorY );
    if ( xPos < xOffset() ) {
	int of = xPos - 10; //###
	setXOffset( of );
    } else if ( xPos > xOffset() + viewWidth() ) {
	int of = xPos - viewWidth() + 10; //###
	setXOffset( of );
    }
}

/*!
  Computes the character position in line \a line which corresponds
  to pixel \a xPos
*/

int QtMultiLineEdit::mapFromView( int xPos, int line )
{
    QString s = stringShown( line );
    if ( !s )
	return 0;
    QFontMetrics fm( font() );
    int index = xPosToCursorPos( s, fm,
				 xPos - d->lr_marg,
				 cellWidth() - 2 * d->lr_marg - d->marg_extra,
				 d->align );
    QtMultiLineEditRow* r = contents->at( line );
    if (r && !r->newline && ((unsigned int)index == r->s.length()) && (index > 0))
	--index;
    return index;
}

/*!
  Computes the pixel position in line \a line which corresponds to
  character position \a xIndex
*/

int QtMultiLineEdit::mapToView( int xIndex, int line )
{
    QString s = stringShown( line );
    xIndex = QMIN( (int)s.length(), xIndex );
    QFontMetrics fm( font() );
    int wcell = cellWidth() - 2 * d->lr_marg;// - d->marg_extra;
    int wrow = contents->at( line )->w;
    int w = textWidthWithTabs( fm, s, 0, xIndex, d->align ) - 1;
    if ( d->align == Qt::AlignCenter || d->align == Qt::AlignHCenter )
	w += (wcell - wrow) / 2;
    else if ( d->align == Qt::AlignRight )
	w += wcell - wrow;
    return d->lr_marg + w;
}

/*!
  Traverses the list and finds an item with the maximum width, and
  updates the internal list box structures accordingly.
*/

void QtMultiLineEdit::updateCellWidth()
{
    QtMultiLineEditRow* r = contents->first();
    int maxW = 0;
    int w;
    switch ( d->echomode ) {
    case Normal:
	while ( r ) {
	    w = r->w;
	    if ( w > maxW )
		maxW = w;
	    r = contents->next();
	}
	break;
    case Password: {
	uint l = 0;
	while ( r ) {
	    l = QMAX(l,  r->s.length() );
	    r = contents->next();
	}
	QString t;
	t.fill(QChar('*'), l);
	maxW = textWidth(t);
    }
    break;
    case NoEcho:
	maxW = textWidth(QString::fromLatin1(""));
    }
    setWidth( maxW );
}


/*!
  Sets the bottommost visible line to \a line.
*/

void QtMultiLineEdit::setBottomCell( int line )
{
    int rowY = cellHeight() * line;
    int newYPos = rowY +  cellHeight() - viewHeight();
    setYOffset( QMAX( newYPos, 0 ) );
}

#ifndef QT_NO_CLIPBOARD

/*!
  Copies text in MIME subtype \a subtype from the clipboard onto the current
  cursor position.
  Any marked text is first deleted.
*/
void QtMultiLineEdit::pasteSubType(const QCString& subtype)
{
    QCString st = subtype;
    addUndoCmd( new QBeginCommand );

    if ( hasMarkedText() )
	del();
    QString t = QApplication::clipboard()->text(st);
    if ( !t.isEmpty() ) {
	if ( hasMarkedText() )
	    turnMark( FALSE );

#if defined(_OS_WIN32_)
	// Need to convert CRLF to NL
	t.replace( "\r\n", "\n" );
#endif

	for (int i=0; (uint)i<t.length(); i++) {
	    if ( t[i] < ' ' && t[i] != '\n' && t[i] != '\t' )
		t[i] = ' ';
	}
	insertAt( t, cursorY, cursorX );
	turnMark( FALSE );
	curXPos  = 0;
	makeVisible();
    }
    if ( textDirty && !d->isHandlingEvent )
	emit textChanged();

    addUndoCmd( new QEndCommand );
}

/*!
  Copies plain text from the clipboard onto the current cursor position.
  Any marked text is first deleted.
*/
void QtMultiLineEdit::paste()
{
    pasteSubType("plain");
}

#ifndef QT_NO_MIMECLIPBOARD
/*!
  Prompts the user for a type from a list of text types available,
  Then copies text from the clipboard onto the current cursor position.
  Any marked text is first deleted.
*/
void QtMultiLineEdit::pasteSpecial(const QPoint& pt)
{
    QCString st = pickSpecial(QApplication::clipboard()->data(),TRUE,pt);
    if ( !st.isEmpty() )
	pasteSubType(st);
}
#endif
#ifndef QT_NO_MIME
QCString QtMultiLineEdit::pickSpecial(QMimeSource* ms, bool always_ask, const QPoint& pt)
{
    if ( ms )  {
	QPopupMenu popup(this);
	QString fmt;
	int n=0;
	QDict<void> done;
	for (int i=0; !(fmt=ms->format(i)).isNull(); i++) {
	    int semi=fmt.find(";");
	    if ( semi >= 0 )
		fmt = fmt.left(semi);
	    if ( fmt.left(5) == "text/" ) {
		fmt = fmt.mid(5);
		if ( !done.find(fmt) ) {
		    done.insert(fmt,(void*)1);
		    popup.insertItem(fmt,i);
		    n++;
		}
	    }
	}
	if ( n ) {
	    int i = n==1 && !always_ask ? popup.idAt(0) : popup.exec(pt);
	    if ( i >= 0 )
		return popup.text(i).latin1();
	}
    }
    return QCString();
}
#endif // QT_NO_MIME
#endif // QT_NO_CLIPBOARD


/*!
  Removes all text.
*/

void QtMultiLineEdit::clear()
{
    addUndoCmd( new QDelTextCmd( 0, text() ) );
    setEdited( TRUE );
    contents->clear();
    cursorX = cursorY = 0;
    int w  = textWidth( QString::fromLatin1("") );
    contents->append( new QtMultiLineEditRow(QString::fromLatin1(""), w) );
    setNumRowsAndTruncate();
    setWidth( w );
    dummy = TRUE;
    turnMark( FALSE );
    if ( autoUpdate() )
	update();
    if ( !d->isHandlingEvent ) //# && not already empty
	emit textChanged();
    update();
}


/*!\reimp
*/

void QtMultiLineEdit::setFont( const QFont &font )
{
    QWidget::setFont( font );
    d->clearChartable();
    QFontMetrics fm( font );
    setCellHeight( fm.lineSpacing() );
    for ( QtMultiLineEditRow* r = contents->first(); r; r = contents->next() )
	r->w = textWidth( r->s  );
    rebreakAll();
    updateCellWidth();
}

/*!
  Sets a new marked text limit, does not repaint the widget.
*/

void QtMultiLineEdit::newMark( int posx, int posy, bool /*copy*/ )
{
    if ( markIsOn && markDragX == posx && markDragY == posy &&
	 cursorX   == posx && cursorY   == posy )
	return;
    markDragX  = posx;
    markDragY  = posy;
    cursorX    = posx;
    cursorY    = posy;
    turnMark( markDragX != markAnchorX || markDragY != markAnchorY );
}

bool QtMultiLineEdit::beforeMark( int posx, int posy ) const
{
    int x1, y1, x2, y2;
    if ( !getMarkedRegion( &y1, &x1, &y2, &x2 ) )
	return FALSE;
    return
	(y1 > posy || y1 == posy && x1 > posx)
	&& (y2 > posy || y2 == posy && x2 > posx);
}

bool QtMultiLineEdit::afterMark( int posx, int posy ) const
{
    int x1, y1, x2, y2;
    if ( !getMarkedRegion( &y1, &x1, &y2, &x2 ) )
	return FALSE;
    return
	(y1 < posy || y1 == posy && x1 < posx)
	&& (y2 < posy || y2 == posy && x2 < posx);
}

bool QtMultiLineEdit::inMark( int posx, int posy ) const
{
    int x1, y1, x2, y2;
    if ( !getMarkedRegion( &y1, &x1, &y2, &x2 ) )
	return FALSE;
    return
	(y1 < posy || y1 == posy && x1 <= posx)
	&& (y2 > posy || y2 == posy && x2 >= posx);
}

/*!
  Marks the word at character position \a posx, \a posy.
 */
void QtMultiLineEdit::markWord( int posx, int posy )
{
    QString& s = contents->at( posy )->s;

    int i = posx - 1;
    while ( i >= 0 && s[i].isPrint() && !s[i].isSpace() )
	i--;
    i++;
    markAnchorY = posy;
    markAnchorX = i;

    while ( s[i].isPrint() && !s[i].isSpace() )
	i++;
    markDragX = i;
    markDragY = posy;
    turnMark( markDragX != markAnchorX || markDragY != markAnchorY );

#ifndef QT_NO_CLIPBOARD
#if defined(Q_WS_X11)
    if ( echoMode() == Normal )
	copy();
#endif
#endif
}

/*!
  This may become a protected virtual member in a future Qt.
  This implementation is an example of a useful classification
  that aids selection of common units like filenames and URLs.
*/
int QtMultiLineEdit::charClass( QChar ch )
{
    if ( !ch.isPrint() || ch.isSpace() ) return 1;
    else if ( ch.isLetter() || ch=='-' || ch=='+' || ch==':'
	      || ch=='.' || ch=='/' || ch=='\\'
	      || ch=='@' || ch=='$' || ch=='~' ) return 2;
    else return 3;
}

#ifndef QT_NO_CLIPBOARD
/*!
  Copies the marked text to the clipboard.  Will copy only
  if echoMode() is Normal.
*/

void QtMultiLineEdit::copy() const
{
    QString t = markedText();
    if ( !t.isEmpty() && echoMode() == Normal ) {
#if defined(Q_WS_X11)
	disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
#endif
#if defined(_OS_WIN32_)
	// Need to convert NL to CRLF
	t.replace( "\n", "\r\n" );
#endif
	QApplication::clipboard()->setText( t );
#if defined(_WS_X11_)
	connect( QApplication::clipboard(), SIGNAL(dataChanged()),
		 this, SLOT(clipboardChanged()) );
#endif
    }
}

/*! \obsolete

  Backward compatibility.
*/
void QtMultiLineEdit::copyText() const
{
    copy();
}

/*!
  Copies the selected text to the clipboard and deletes the selected text.
*/

void QtMultiLineEdit::cut()
{
    if ( hasMarkedText() ) {
	if ( echoMode() == Normal )
	    copy();
	del();
	if ( textDirty && !d->isHandlingEvent )
	    emit textChanged();
    }
}

#endif

/*!
  This private slot is activated when this line edit owns the clipboard and
  some other widget/application takes over the clipboard. (X11 only)
*/

void QtMultiLineEdit::clipboardChanged()
{
#if defined(Q_WS_X11)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()),
		this, SLOT(clipboardChanged()) );
    turnMark( FALSE );
    update();
#endif
}


/*!
   Sets maxLineWidth() and maybe cellWidth() to \a w without updating the entire widget.
 */

void QtMultiLineEdit::setWidth( int w )
{
    if ( w ==d->maxLineWidth )
	return;
    bool u = autoUpdate();
    setAutoUpdate( FALSE );
    d->maxLineWidth = w;
    if ( d->align == AlignLeft )
	setCellWidth( w );
    else
	setCellWidth( QMAX( w, contentsRect().width() ) );
    setAutoUpdate( u );
    if ( autoUpdate() && d->align != AlignLeft )
	update();
}


/*!
  Sets the cursor position to character number \a col in line number \a line.
  The parameters are adjusted to lie within the legal range.

  If \a mark is FALSE, the selection is cleared. otherwise it is extended

  \sa cursorPosition()
*/

void QtMultiLineEdit::setCursorPosition( int line, int col, bool mark )
{
    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    int oldY = cursorY;
    cursorY = QMAX( QMIN( line, numLines() - 1), 0 );
    cursorX = QMAX( QMIN( col,  lineLength( cursorY )), 0 );
    curXPos = 0;
    makeVisible();
    if ( mark ) {
	newMark( cursorX, cursorY, FALSE );
	for ( int i = QMIN(oldY,cursorY); i <= QMAX(oldY,cursorY); i++ )
	    updateCell( i, 0, FALSE );
    } else {
	updateCell( oldY, 0, FALSE );
	turnMark( FALSE );
    }
}



/*! \obsolete

  Use getCursorPosition() instead.
*/

void QtMultiLineEdit::cursorPosition( int *line, int *col ) const
{
    getCursorPosition(line,col);
}


/*!
  Returns the current line and character
  position within that line, in the variables pointed to
  by \a line and \a col respectively.

  \sa setCursorPosition()
*/

void QtMultiLineEdit::getCursorPosition( int *line, int *col ) const
{
    if ( line )
	*line = cursorY;
    if ( col )
	*col = cursorX;
}


/*
  \sa setAutoUpdate()
*/

bool QtMultiLineEdit::autoUpdate() const
{
    return QtTableView::autoUpdate();
}


/*
  Sets the auto-update option of multi-line editor to \a enable.

*/

void QtMultiLineEdit::setAutoUpdate( bool enable )
{
    QtTableView::setAutoUpdate( enable );
}

/*!
  Sets the fixed height of the QtMultiLineEdit so that \a lines text lines
  are visible given the current font.

  \sa setMaxLines(), setFixedHeight()
 */
void QtMultiLineEdit::setFixedVisibleLines( int lines )
{
    int ls = fontMetrics().lineSpacing();
    setFixedHeight( frameWidth()*2 + ls*lines );
    return;
}



/*!
  Returns the top center point where the cursor is drawn
*/

QPoint QtMultiLineEdit::cursorPoint() const
{
    QPoint cp( 0, 0 );

    QFontMetrics fm( font() );
    int col, row;
    col = row = 0;
    cursorPosition( &row, &col );
    QString line = textLine( row );
    ASSERT( line );
    cp.setX( d->lr_marg + textWidthWithTabs( fm, line, 0, col, d->align ) - 1 );
    cp.setY( (row * cellHeight()) + viewRect().y() );
    return cp;
}


/*! \reimp
*/
QSizePolicy QtMultiLineEdit::sizePolicy() const
{
    //### removeme 3.0
    return QWidget::sizePolicy();
}


/*!\reimp
*/
QSize QtMultiLineEdit::sizeHint() const
{
    constPolish();
    int expected_lines;
    if ( d->maxlines >= 0 && d->maxlines <= 6 ) {
	expected_lines = d->maxlines;
    } else {
	expected_lines = 6;
    }
    QFontMetrics fm( font() );
    int h = fm.lineSpacing()*(expected_lines-1)+fm.height() + frameWidth()*2;
    int w = fm.width('x')*35;

    int maxh = maximumSize().height();
    if ( maxh < QWIDGETSIZE_MAX )
	h = maxh;

    return QSize( w, h ).expandedTo( QApplication::globalStrut() );
}


/*!
  Returns a size sufficient for one character, and scroll bars.
*/

QSize QtMultiLineEdit::minimumSizeHint() const
{
    constPolish();
    QFontMetrics fm( font() );
    int h = fm.lineSpacing() + frameWidth()*2;
    int w = fm.maxWidth();
    h += frameWidth();
    w += frameWidth();
    if ( testTableFlags(Tbl_hScrollBar|Tbl_autoHScrollBar) )
	w += verticalScrollBar()->sizeHint().width();
    if ( testTableFlags(Tbl_vScrollBar|Tbl_autoVScrollBar) )
	h += horizontalScrollBar()->sizeHint().height();
    return QSize(w,h);
}



/*!\reimp
*/

void QtMultiLineEdit::resizeEvent( QResizeEvent *e )
{
    int oldw = contentsRect().width();
    QtTableView::resizeEvent( e );
    if ( DYNAMIC_WRAP
	 && (e->oldSize().width() != width()
	     || oldw != contentsRect().width() ) ) {
	bool oldAuto = autoUpdate();
	setAutoUpdate( FALSE );
	rebreakAll();
	if ( oldw != contentsRect().width() )
	    rebreakAll();
	setAutoUpdate( oldAuto );
	if ( autoUpdate() )
	    repaint( FALSE );
    } else if ( d->align != AlignLeft ) {
 	d->maxLineWidth = 0; // trigger update
	updateCellWidth();
    }
    if ( isVisible() )
	deselect();
}

/*
  Sets the alignment to \a flags. Possible values are \c AlignLeft, \c
  Align(H)Center and \c AlignRight.

  \sa alignment(), Qt::AlignmentFlags
*/
void QtMultiLineEdit::setAlignment( int flags )
{
    if ( d->align != flags ) {
	d->align = flags;
	update();
    }
}

/*
  Returns the alignment.

*/
int QtMultiLineEdit::alignment() const
{
    return d->align;
}


/*!
  Not supported at this time.
  \a v is the validator to set.
*/
void QtMultiLineEdit::setValidator( const QValidator *v )
{
    d->val = v;
    // #### validate text now
}

/*!
  Not supported at this time.
*/
const QValidator * QtMultiLineEdit::validator() const
{
    return d->val;
}

/*  \sa edited()
*/
void QtMultiLineEdit::setEdited( bool e )
{
    d->edited = e;
}

/*  \sa setEdited()
*/
bool QtMultiLineEdit::edited() const
{
    return d->edited;
}

/*! \enum QtMultiLineEdit::EchoMode

  This enum type describes the ways in which QLineEdit can display its
  contents.  The currently defined values are: \list

  \i Normal - display characters as they are entered.  This is
  the default.

  \i NoEcho - do not display anything.

  \i Password - display asterisks instead of the characters
  actually entered.

  \endlist

  \sa setEchoMode() echoMode() QLineEdit::EchoMode
*/


/*
  Sets the echo mode to \a em.  The default is \c Normal.

  The display is updated according.

  \sa setEchoMode()
*/
void QtMultiLineEdit::setEchoMode( EchoMode em )
{
    if ( d->echomode != em ) {
	d->echomode = em;
	updateCellWidth();
	update();
    }
}

/*
  Returns the currently set echo mode.

  \sa setEchoMode()
*/
QtMultiLineEdit::EchoMode QtMultiLineEdit::echoMode() const
{
    return d->echomode;
}


/*!
  Returns the string shown at line \a row, including
  processing of the echoMode().
*/

QString QtMultiLineEdit::stringShown(int row) const
{
    QString* s = getString(row);
    if ( !s ) return QString::null;
    switch ( d->echomode ) {
    case Normal:
	if (!*s) return QString::fromLatin1("");
	return *s;
    case Password:
	{
	    QString r;
	    r.fill(QChar('*'), (int)s->length());
	    if ( !r ) r = QString::fromLatin1("");
	    return r;
	}
    case NoEcho:
	return QString::fromLatin1("");
    }
    return QString::fromLatin1("");
}

/*

  \sa maxLength()
*/
void QtMultiLineEdit::setMaxLength(int m)
{
    d->maxlen = m;
}

/*
  \sa setMaxLength()
*/
int QtMultiLineEdit::maxLength() const
{
    return d->maxlen;
}


/*
  Returns the length of the current text.

  \sa setMaxLength()
 */
int QtMultiLineEdit::length() const
{
    int l = 0;
    for ( QtMultiLineEditRow* r = contents->first(); r; r = contents->next() ) {
	l += r->s.length();
	if ( r->newline )
	    ++l;
    }
    return l-1;
}


/*!
  Sets the maximum length of lines to \a m.  Use -1 for unlimited
  (the default).  Existing long lines will be truncated.

  \sa maxLineLength()
*/
void QtMultiLineEdit::setMaxLineLength(int m)
{
    bool trunc = d->maxlinelen < 0 || m < d->maxlinelen;
    d->maxlinelen = m;
    if ( trunc ) {
	QtMultiLineEditRow* r = contents->first();
	while ( r ) {
	    r->s.truncate( m );
	    r = contents->next();
	}
	if ( cursorX > m ) cursorX = m;
	if ( markAnchorX > m ) markAnchorX = m;
	if ( markDragX > m ) markDragX = m;
	update();
	updateCellWidth();
    }
}

/*!
  Returns the currently set line length limit, or -1 if there is
  no limit (this is the default).

  \sa setMaxLineLength()
*/
int QtMultiLineEdit::maxLineLength() const
{
    return d->maxlinelen;
}

/*
  Sets the maximum number of lines to \a m.  Use -1 for unlimited
  (the default).  Existing excess lines will be deleted.

  \sa maxLines(), numLines()
*/
void QtMultiLineEdit::setMaxLines(int m)
{
    if ( m == 0 ) // bad value
	m = -1;
    d->maxlines = m;
    if ( d->maxlines >= 0 && d->maxlines <= 6 ) {
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    } else {
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
    }
    if ( setNumRowsAndTruncate() ) {
	updateCellWidth();
	update();
    }
}

/*
  \sa setMaxLines()
*/
int QtMultiLineEdit::maxLines() const
{
    return d->maxlines;
}

/*
  Sets the horizontal margin to \a m.

  \sa hMargin()
*/
void QtMultiLineEdit::setHMargin(int m)
{
    if ( m != d->lr_marg ) {
	d->lr_marg = m;
	updateCellWidth();
	update();
    }
}

/*

  \sa setHMargin()
*/
int QtMultiLineEdit::hMargin() const
{
    return d->lr_marg;
}

/*!
  Marks the text starting at \a row_from, \a col_from and ending
  at \a row_to, \a col_to.
*/
void QtMultiLineEdit::setSelection( int row_from, int col_from,
				   int row_to, int col_to )
{
    setCursorPosition( row_from, col_from, FALSE );
    setCursorPosition( row_to, col_to, TRUE );
}


/*!
  Moves the cursor one word to the right.  If \a mark is TRUE, the text
  is marked.
  \sa cursorWordBackward()
*/
void QtMultiLineEdit::cursorWordForward( bool mark )
{
    int x = cursorX;
    int y = cursorY;

    if ( x == lineLength( y ) || textLine(y).at(x).isSpace() ) {
	while ( x < lineLength( y ) && textLine(y).at(x).isSpace() )
	    ++x;
	if ( x == lineLength( y ) ) {
	    if ( y < (int)contents->count() - 1) {
		++y;
		x = 0;
		while ( x < lineLength( y ) && textLine(y).at(x).isSpace() )
		    ++x;
	    }
	}
    }
    else {
	while ( x < lineLength( y ) && !textLine(y).at(x).isSpace() )
	    ++x;
	int xspace = x;
	while ( xspace < lineLength( y ) && textLine(y).at(xspace).isSpace() )
	    ++xspace;
	if ( xspace <  lineLength( y ) )
	    x = xspace;
    }
    cursorOn = TRUE;
    int oldY = cursorY;
    setCursorPosition( y, x, mark );
    if ( oldY != cursorY )
	updateCell( oldY, 0, FALSE );
    updateCell( cursorY, 0, FALSE );
    d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
}

/*!
  Moves the cursor one word to the left.  If \a mark is TRUE, the text
  is marked.
  \sa cursorWordForward()
*/
void QtMultiLineEdit::cursorWordBackward( bool mark )
{
    int x = cursorX;
    int y = cursorY;

    while ( x > 0 && textLine(y).at(x-1).isSpace() )
	--x;

    if ( x == 0 ) {
	if ( y > 0 ) {
	    --y;
	    x = lineLength( y );
	    while ( x > 0  && textLine(y).at(x-1).isSpace() )
		--x;
	}
    }
    else {
	while ( x > 0  && !textLine(y).at(x-1).isSpace() )
	    --x;
    }
    cursorOn = TRUE;
    int oldY = cursorY;
    setCursorPosition( y, x, mark );
    if ( oldY != cursorY )
	updateCell( oldY, 0, FALSE );
    updateCell( cursorY, 0, FALSE );
    d->blinkTimer->start( QApplication::cursorFlashTime() / 2, FALSE );
}

#define DO_BREAK {doBreak = TRUE; if ( lastSpace > a ) { \
i = lastSpace; \
linew = lastw; \
} \
else \
i = QMAX( a, i-1 );}

void QtMultiLineEdit::wrapLine( int line, int removed )
{
    QtMultiLineEditRow* r = contents->at( line );
    int yPos;
    (void) rowYPos( line, &yPos );
    QFontMetrics fm( font() );
    int i  = 0;
    QString s = r->s;
    int a = 0;
    int l = line;
    int w = 0;
    int nlines = 0;
    int lastSpace = -1;
    bool doBreak = FALSE;
    int linew = 0;
    int lastw = 0;
    int tabDist = -1; // lazy eval
    while ( i < int(s.length()) ) {
	doBreak = FALSE;
	if ( s[i] == '\t' && d->align == Qt::AlignLeft ) {
	    if ( tabDist<0 )
		tabDist = tabStopDist(fm);
	    linew = ( linew/tabDist + 1 ) * tabDist;
	} else if ( s[i] != '\n' ) {
	    char c = s[i].latin1();
	    if ( c > 0 ) {
		if ( !d->chartable[c] )
		    d->chartable[c] = fm.width( s[i] );
		linew += d->chartable[c];
	    } else {
		linew += fm.width( s[i] );
	    }
	}
	if ( WORD_WRAP &&
	     lastSpace >= a && s[i] != '\n' ) {
	    if ( DYNAMIC_WRAP ) {
		if  (linew >= contentsRect().width() -  2*d->lr_marg - d->marg_extra) {
		    DO_BREAK
		}
	    } else if ( FIXED_COLUMN_WRAP ) {
		if ( d->wrapcol >= 0 && i-a >= d->wrapcol ) {
		    DO_BREAK
		}
	    } else if ( FIXED_WIDTH_WRAP ) {
		if ( d->wrapcol >= 0 && linew > d->wrapcol - d->marg_extra ) {
		    DO_BREAK
		}
	    }
	}
	if ( s[i] == '\n' || doBreak ) {
	    r->s = s.mid( a, i - a + (doBreak?1:0) );
	    r->w = linew - fm.leftBearing(r->s[0]) + 2 * d->lr_marg + d->marg_extra;
	    if ( r->w > w )
		w = r->w;
	    if ( cursorY > l )
		++cursorY;
	    else if ( cursorY == line && cursorX >=a && cursorX <= i +  (doBreak?1:0)) {
		cursorY = l;
		cursorX -= a;
	    }
	    if ( markAnchorY > l )
		++markAnchorY;
	    else if ( markAnchorY == line && markAnchorX >=a && markAnchorX <= i +  (doBreak?1:0)) {
		markAnchorY = l;
		markAnchorX -= a;
	    }
	    a = i + 1;
	    lastSpace = a;
	    linew = 0;
	    bool oldnewline = r->newline;
	    r->newline = !doBreak;
	    r = new QtMultiLineEditRow( QString::null, 0, oldnewline );
	    ++nlines;
	    contents->insert( l + 1, r );
	    ++l;
	}
	if ( s[i].isSpace() || BREAK_WITHIN_WORDS ) {
	    lastSpace = i;
	    lastw = linew;
	}
	if ( lastSpace <= a )
	    lastw = linew;

	++i;
    }
    if ( a < int(s.length()) ){
	r->s = s.mid( a, i - a  );
	r->w = linew - fm.leftBearing( r->s[0] ) + 2 * d->lr_marg + d->marg_extra;
    }
    if ( cursorY == line && cursorX >= a ) {
	cursorY = l;
	cursorX -= a;
    }
    if ( markAnchorY == line && markAnchorX >= a ) {
	markAnchorY = l;
	markAnchorX -= a;
    }
    if ( r->w > w )
	w = r->w;

    setWidth( QMAX( maxLineWidth(), w ) );
    bool oldAuto = autoUpdate();
    setAutoUpdate( FALSE );
    (void)setNumRowsAndTruncate();
    setAutoUpdate( oldAuto );

    yPos += (nlines+1)  * cellHeight();
    int sh = (nlines-removed)  * cellHeight();
    if ( autoUpdate() ) {
	if ( sh && yPos >= contentsRect().top() && yPos < contentsRect().bottom() ) {
	    int h = contentsRect().bottom() - yPos + 1;
	    if ( d->maxlines >= 0 ) {
		int maxy;
		if ( rowYPos( d->maxlines-1, &maxy ) ) {
		    maxy += cellHeight();
		    if ( maxy < contentsRect().bottom() && maxy > yPos )
			h = maxy - yPos + 1;
		}
	    }
	    QWidget::scroll( 0, sh, QRect( contentsRect().left(), yPos,
					   contentsRect().width(),
					   h ) );
	}
	for (int ul = 0; ul <= nlines; ++ul )
	    updateCell( line + ul, 0, FALSE );
    }
}

void QtMultiLineEdit::rebreakParagraph( int line, int removed )
{
    QtMultiLineEditRow* r = contents->at( line );
    if ( WORD_WRAP ) {
	QtMultiLineEditRow* other = 0;
	while (line < int(contents->count())-1 && !r->newline ) {
	    other = contents->at( line + 1 );
	    if ( cursorY > line ) {
		--cursorY;
		if ( cursorY == line ) {
		    cursorX += r->s.length();
		}
	    }
	    if ( markAnchorY > line ) {
		--markAnchorY;
		if ( markAnchorY == line ) {
		    markAnchorX += r->s.length();
		}
	    }
	    r->s.append( other->s );
	    r->newline = other->newline;
	    contents->remove( line + 1 );
	    ++removed;
	}
    }
    wrapLine( line, removed );
}

void QtMultiLineEdit::rebreakAll()
{
    if ( !WORD_WRAP )
	return;
    d->maxLineWidth = 0;
    for (int i = 0; i < int(contents->count()); ++i ) {
	if ( contents->at( i )->newline &&
	     contents->at( i )->w < contentsRect().width() -  2*d->lr_marg - d->marg_extra ) {
	    setWidth( QMAX( d->maxLineWidth, contents->at( i )->w ) );
	    continue;
	}
	rebreakParagraph( i );
	while ( i < int(contents->count() )
		&& !contents->at( i )->newline )
	    ++i;
    }
}


/*! \enum QtMultiLineEdit::WordWrap

  This enum describes the multiline edit's word wrap mode.

  The following values are valid:
    \list
    \i NoWrap - no word wrap at all.
    \i WidgetWidth - word wrap depending on the current
     width of the editor widget
    \i FixedPixelWidth - wrap according to a fix amount
     of pixels ( see wrapColumnOrWidth() )
    \i FixedColumnWidth - wrap according to a fix character
     column. This is useful whenever you need formatted text that
     can also be displayed gracefully on devices with monospaced
     fonts, for example a standard VT100 terminal. In that case
     wrapColumnOrWidth() should typically be set to 80.
  \endlist

 \sa setWordWrap()
*/

/*
  Sets the word wrap mode to \a mode.

 */
void QtMultiLineEdit::setWordWrap( WordWrap mode )
{
    if ( mode == d->wordwrap )
	return;
    d->wordwrap = mode;

    if ( BREAK_WITHIN_WORDS  ) {
	d->arrow = QPixmap( (const char **)arrow_xpm );
	d->marg_extra = 8;
	if ( DYNAMIC_WRAP )
	    clearTableFlags( Tbl_autoHScrollBar );
        else
            setTableFlags( Tbl_autoHScrollBar );
    } else {
	d->marg_extra = 0;
	setTableFlags( Tbl_autoHScrollBar );
    }
    if ( !text().isEmpty() )
        setText( text() );
}

/*
  Returns the current word wrap mode.

  \sa setWordWrap()
 */
QtMultiLineEdit::WordWrap QtMultiLineEdit::wordWrap() const
{
    return d->wordwrap;
}

/*
  Sets the wrap column or wrap width to \a value, depending on the
  word wrap mode.

  \sa setWordWrap()
 */
void QtMultiLineEdit::setWrapColumnOrWidth( int value )
{
    if ( value == d->wrapcol )
	return;
    d->wrapcol = value;
    if ( wordWrap() != NoWrap )
	setText( text() );
}

/*
 */
int QtMultiLineEdit::wrapColumnOrWidth() const
{
    return d->wrapcol;
}


/*! \enum QtMultiLineEdit::WrapPolicy

  Defines where text can be wrapped in word wrap mode.

  The following values are valid:
  \list
  \i AtWhiteSpace - break only after whitespace
  \i Anywhere - break anywhere
   \endlist

   \sa setWrapPolicy()
*/

/*
  Sets the wrap \a policy, i.e. where text can be wrapped in word wrap
  mode.

  \sa setWordWrap(), wrapPolicy()
 */
void QtMultiLineEdit::setWrapPolicy( WrapPolicy policy )
{
    if ( d->wrappolicy == policy )
	return;
    d->wrappolicy = policy;
    WordWrap m = d->wordwrap;
    if ( m != NoWrap ) { // trigger update
	d->wordwrap = NoWrap;
	setWordWrap( m );
    }
}

/*

  Returns the current word wrap policy.

  \sa setWrapPolicy()
 */
QtMultiLineEdit::WrapPolicy QtMultiLineEdit::wrapPolicy() const
{
    return d->wrappolicy;
}

/*!
  Returns wether \a row is the last row in a paragraph.

  This function is only interesting in word wrap mode, otherwise its
  return value is always TRUE.

  \sa setWordWrap()
 */
bool QtMultiLineEdit::isEndOfParagraph( int row ) const
{
    return contents->at( row )->newline;
}

int QtMultiLineEdit::positionToOffsetInternal( int row, int col ) const
{
    row = QMAX( QMIN( row, numLines() - 1), 0 ); // Sanity check
    col = QMAX( QMIN( col,  lineLength( row )), 0 ); // Sanity check
    if ( row == 0 )
	return QMIN( col, lineLength( 0 ));
    else {
	int lastI;
	lastI  = lineLength( row );
	int i, tmp = 0;

	for( i = 0; i < row ; i++ ) {
	    tmp += lineLength( i );
	    if ( contents->at( i )->newline )
		++tmp;
	}

	tmp += QMIN( lastI, col );

	return tmp;
    }

}

// if position is <0 = returns row 0, col 0, if position >= amount of text
// returns pointer to end of text.
void QtMultiLineEdit::offsetToPositionInternal( int position,
				       int *row, int *col ) const
{
    if (position <= 0) {
	*row = 0;
	*col = 0;
	return;
    }
    else {
	int charsLeft = position;
	int i;

	for( i = 0; contents->at( i ); ++i ) {
	    if (lineLength( i ) < charsLeft)
		charsLeft -= lineLength( i );
	    else {
		*row = i;
		*col = charsLeft;
		return;
	    }
	    if ( contents->at( i )->newline )
		--charsLeft;
	}

	if (contents->at( i - 1) && !contents->at( i - 1 )->newline) {
	    *row = i - 1;
	    *col = lineLength( i - 1 );
	}
	else {
	    *row = i;
	    *col = 0;
	}
	return;
    }
}


/*!
  Processes an undo/redo command \a cmd, depending on \a undo.
 */
void QtMultiLineEdit::processCmd( QtMultiLineEditCommand* cmd, bool undo)
{
    QDelTextCmd* delcmd = (QDelTextCmd*) cmd;
    bool ins = TRUE;
    if (cmd->type() == QtMultiLineEditCommand::Delete )
	ins = undo;
    else if (cmd->type() == QtMultiLineEditCommand::Insert )
	ins = !undo;
    else
	return;

    if ( ins ) {
	int row, col;
	offsetToPositionInternal( delcmd->mOffset, &row, &col );
	setCursorPosition( row, col, FALSE );
	insertAt( delcmd->mStr, row, col, FALSE );
	offsetToPositionInternal( delcmd->mOffset+delcmd->mStr.length(), &row, &col );
	setCursorPosition( row, col, FALSE );
    } else { // del
	int row, col, rowEnd, colEnd;
	offsetToPositionInternal( delcmd->mOffset, &row, &col );
	offsetToPositionInternal( delcmd->mOffset + delcmd->mStr.length(), &rowEnd, &colEnd );
	markAnchorY    = row;
	markAnchorX    = col;
	setCursorPosition( rowEnd, colEnd, FALSE );
	markDragY    = rowEnd;
	markDragX    = colEnd;
	turnMark( TRUE );
	del();
    }
}

/*!
  Undoes the last text operation.
 */
void QtMultiLineEdit::undo()
{
    if ( d->undoList.isEmpty() || isReadOnly() )
	return;
    textDirty = FALSE;
    int macroLevel = 0;
    bool before = d->undo;
    d->undo = FALSE;
    do {
	QtMultiLineEditCommand *command = d->undoList.take();
	if ( !command )
	    break;
	processCmd( command, TRUE );
	macroLevel += command->terminator();
	if ( d->undoList.isEmpty() )
	    emit undoAvailable( FALSE );
	addRedoCmd( command );
    } while (macroLevel != 0);
    d->undo = before;
    if ( textDirty )
	emit textChanged();
    textDirty = FALSE;
}

/*!
  Redoes the last text operation.
 */
void QtMultiLineEdit::redo()
{
    if ( d->redoList.isEmpty() || isReadOnly() )
	return;
    textDirty = FALSE;
    int macroLevel = 0;
    bool before = d->undo;
    d->undo = FALSE;
    do {
	QtMultiLineEditCommand *command = d->redoList.take();
	if ( !command )
	    break;
	processCmd( command, FALSE );
	macroLevel += command->terminator();
	if ( d->redoList.isEmpty() )
	    emit redoAvailable( FALSE );
	if ( d->undoList.isEmpty() )
	    emit undoAvailable(TRUE);
	d->undoList.append( command );
    } while (macroLevel != 0);
    d->undo = before;
    if ( textDirty )
	emit textChanged();
    textDirty = FALSE;
}

/*!
  Inserts \a s at line number \a line, after character number \a col
  in the line.
  If \a s contains newline characters, new lines are inserted.
  If \a mark is TRUE the inserted text is selected.

  The cursor position is adjusted. If the insertion position is equal to
  the cursor position, the cursor is placed after the end of the new text.

 */

void QtMultiLineEdit::insertAt( const QString &s, int line, int col, bool mark )
{
    if ( d->undo ) {
	d->undo = FALSE;
	QString itxt = s;
	int offset = positionToOffsetInternal( line, col );
	if ( d->maxlen >= 0 && length() + int(s.length()) > d->maxlen )
	    itxt.truncate( d->maxlen - length() );
	addUndoCmd( new QInsTextCmd( offset, itxt ) );
	insertAtAux( s, line, col, mark ); // may perform del op
	d->undo = TRUE;
    }
    else
	insertAtAux( s, line, col, mark ); // may perform del op
}

void QtMultiLineEdit::deleteNextChar( int offset, int row, int col )
{
    int row2, col2;
    setCursorPosition( row, col, FALSE );
    offsetToPositionInternal( offset + 1, &row2, &col2 );
    setCursorPosition( row2, col2, TRUE );

    QString str = markedText();
    addUndoCmd( new QDelTextCmd( offset, str ) );

    setCursorPosition( row, col, FALSE );
}

/*!
  Deletes text from the current cursor position to the end of the line.
*/

void QtMultiLineEdit::killLine()
{
    if ( d->undo ) {
	d->undo = FALSE;
	int curY, curX;
	cursorPosition( &curY, &curX );
	int offset = positionToOffsetInternal( curY, curX );
	QtMultiLineEditRow* r = contents->at( curY );
	deselect();

	addUndoCmd( new QBeginCommand );
	if (curX == (int)r->s.length()) {
	    if ( ! atEnd() && r->newline )
		deleteNextChar( offset, curY, curX );
	}
	else {
	    QString str = r->s.mid( curX, r->s.length() );
	    addUndoCmd( new QDelTextCmd( offset, str ) );
	}

	addUndoCmd( new QEndCommand );
	killLineAux();
	d->undo = TRUE;
    }
    else
	killLineAux();
}

/*!
  Deletes the character on the right side of the text cursor. If a
  text has been marked by the user (e.g. by clicking and dragging) the
  cursor is put at the beginning of the marked text and the marked
  text is removed.  \sa backspace()
*/

void QtMultiLineEdit::del()
{
    if (d->undo ) {
	d->undo = FALSE;
	bool oldAuto = autoUpdate();
	setAutoUpdate( FALSE );
	int markBeginX, markBeginY;
	int markEndX, markEndY;

	if ( getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX ) ) {
	    addUndoCmd( new QBeginCommand );
	    int offset = positionToOffsetInternal( markBeginY, markBeginX );
	    QString str = markedText();
	    d->undoList.append( new QDelTextCmd( offset, str ) );
	    addUndoCmd( new QEndCommand );
	}
	else if ( ! atEnd() ) {
	    int crsorY, crsorX;
	    cursorPosition( &crsorY, &crsorX );
	    int offset = positionToOffsetInternal( crsorY, crsorX );
	    QtMultiLineEditRow* r = contents->at( crsorY );
	    if (r) {
		if (crsorX != (int)r->s.length())
		    deleteNextChar( offset, crsorY, crsorX );
		else if (r->newline)
		    deleteNextChar( offset, crsorY, crsorX );
		// else noop
	    }
	}
	setAutoUpdate( oldAuto );
	delAux();
	d->undo = TRUE;
    }
    else
	delAux();
}

/*
  Sets undo enabled to \a enable.

  \sa isUndoEnabled()
*/
void QtMultiLineEdit::setUndoEnabled( bool enable )
{
    if ( d->undo == enable )
	return;
    d->undo = enable;
    if ( !enable ) {
	CLEAR_UNDO
    }
}


/*
  Returns whether the multilineedit is currently undo enabled or not.

  \sa setUndoEnabled()
 */
bool QtMultiLineEdit::isUndoEnabled() const
{
    return d->undo;
}


/*
  Sets the maximum number of operations that can be stored on the undo
  stack to \a depth.

  \sa undoDepth()
 */
void QtMultiLineEdit::setUndoDepth( int depth )
{
    d->undodepth = depth;
}


/*
 */
int QtMultiLineEdit::undoDepth() const
{
    return d->undodepth;
}

void QtMultiLineEdit::blinkTimerTimeout()
{
    cursorOn = !cursorOn;
    updateCell( cursorY, 0, FALSE );
}

void QtMultiLineEdit::scrollTimerTimeout()
{
    QPoint p = mapFromGlobal( QCursor::pos() );
    if ( d->scrollAccel-- <= 0 && d->scrollTime ) {
	d->scrollAccel = initialScrollAccel;
	d->scrollTime--;
	d->scrollTimer->stop();
	d->scrollTimer->start( d->scrollTime );
    }
    int l = QMAX(1,(initialScrollTime-d->scrollTime)/5);

	// auto scrolling is dual-use - for highlighting and DND
    int margin = d->dnd_primed ? scroll_margin : 0;
    bool mark = !d->dnd_primed;
    bool clear_mark = d->dnd_primed ? FALSE : !mark;

    for (int i=0; i<l; i++) {
	if ( p.y() < margin ) {
	    cursorUp( mark, clear_mark );
	} else if ( p.y() > height()-margin ) {
	    cursorDown( mark, clear_mark );
	} else if ( p.x() < margin ) {
	    cursorLeft( mark, clear_mark, FALSE );
	} else if ( p.x() > width()-margin ) {
	    cursorRight( mark, clear_mark, FALSE );
	} else {
	    stopAutoScroll();
	    break;
	}
    }
}

void QtMultiLineEdit::dndTimeout()
{
#ifndef QT_NO_DRAGANDDROP
    doDrag();
#endif
}

int QtMultiLineEdit::setNumRowsAndTruncate()
{
    int n = contents->count();
    int r = 0;
    while ( d->maxlines >= 0 && n > d->maxlines ) {
	// truncate
	contents->at(n-2)->newline = TRUE;
	contents->removeLast();
	if ( markAnchorY == n-1 )
	    markAnchorY--;
	if ( markDragY == n-1 )
	    markDragY--;
	if ( cursorY == n-1 ) {
	    cursorY--;
	    cursorX = contents->at(cursorY)->s.length();
	}
	n--;
	r++;
    }
    setNumRows( n );
    return r;
}

/*! \reimp
*/
bool QtMultiLineEdit::event( QEvent * e )
{
    if ( e->type() == QEvent::AccelOverride && !isReadOnly() ) {
	QKeyEvent* ke = (QKeyEvent*) e;
	if ( ke->state() & ControlButton ) {
	    switch ( ke->key() ) {
	    case Key_A:
	    case Key_E:
#if defined (Q_WS_WIN)
	    case Key_Insert:
#endif
	    case Key_X:
	    case Key_V:
	    case Key_C:
	    case Key_Left:
	    case Key_Right:
	    case Key_Up:
	    case Key_Down:
	    case Key_Home:
	    case Key_End:
		ke->accept();
	    default:
		break;
	    }
	} else {
	    switch ( ke->key() ) {
	    case Key_Delete:
	    case Key_Home:
	    case Key_End:
	    case Key_Backspace:
		ke->accept();
	    default:
		break;
	    }
	}
    }
    return QWidget::event( e );
}

/*! \reimp
*/

bool QtMultiLineEdit::focusNextPrevChild( bool )
{
    return FALSE;
}

#endif
