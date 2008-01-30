// This module implements the portability layer for the Qt port of Scintilla.
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


#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <qapplication.h>
#include <qwidget.h>
#include <qfont.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qlibrary.h>

#include <qpolygon.h>

#include "Platform.h"
#include "XPM.h"

#include "Qsci/qsciscintillabase.h"
#include "SciClasses.h"


// Type convertors.
static QFont *PFont(FontID id)
{
    return reinterpret_cast<QFont *>(id);
}

static QWidget *PWindow(WindowID id)
{
    return reinterpret_cast<QWidget *>(id);
}

static SciPopup *PMenu(MenuID id)
{
    return reinterpret_cast<SciPopup *>(id);
}


// Create a Point instance from a long value.
Point Point::FromLong(long lpoint)
{
    return Point(Platform::LowShortFromLong(lpoint),
            Platform::HighShortFromLong(lpoint));
}


// Colour palette management.  The Qt interface to colours means this class
// doesn't have to do anything.
Palette::Palette()
{
    used = 0;
    allowRealization = false;
}

Palette::~Palette()
{
    Release();
}

void Palette::Release()
{
    used = 0;
}

void Palette::WantFind(ColourPair &cp, bool want)
{
    if (!want)
        cp.allocated.Set(cp.desired.AsLong());
}

void Palette::Allocate(Window &)
{
}


// Font management.
Font::Font() : id(0)
{
}

Font::~Font()
{
}

void Font::Create(const char *faceName, int, int size, bool bold, bool italic,
        bool)
{
    Release();

    QFont *f = new QFont();

    // If name of the font begins with a '-', assume, that it is an XLFD.
    if (faceName[0] == '-')
        f->setRawName(faceName);
    else
    {
        f->setFamily(faceName);
        f->setPointSize(size);
        f->setBold(bold);
        f->setItalic(italic);
    }

    id = f;
}

void Font::Release()
{
    if (id)
    {
        delete PFont(id);
        id = 0;
    }
}


// A surface abstracts a place to draw.
class SurfaceImpl : public Surface
{
public:
    SurfaceImpl();
    virtual ~SurfaceImpl();

    void Init(WindowID wid);
    void Init(SurfaceID sid, WindowID);
    void Init(QPainter *p);
    void InitPixMap(int width, int height, Surface *, WindowID);

    void Release();
    bool Initialised() {return painter;}
    void PenColour(ColourAllocated fore);
    int LogPixelsY() {return 72;}
    int DeviceHeightFont(int points) {return points;}
    void MoveTo(int x_,int y_);
    void LineTo(int x_,int y_);
    void Polygon(Point *pts, int npts, ColourAllocated fore,
            ColourAllocated back);
    void RectangleDraw(PRectangle rc, ColourAllocated fore,
            ColourAllocated back);
    void FillRectangle(PRectangle rc, ColourAllocated back);
    void FillRectangle(PRectangle rc, Surface &surfacePattern);
    void RoundedRectangle(PRectangle rc, ColourAllocated fore,
            ColourAllocated back);
    void AlphaRectangle(PRectangle rc, int cornerSize, ColourAllocated fill,
            int alphaFill, ColourAllocated outline, int alphaOutline,
            int flags);
    void Ellipse(PRectangle rc, ColourAllocated fore, ColourAllocated back);
    void Copy(PRectangle rc, Point from, Surface &surfaceSource);

    void DrawTextNoClip(PRectangle rc, Font &font_, int ybase, const char *s,
            int len, ColourAllocated fore, ColourAllocated back);
    void DrawTextClipped(PRectangle rc, Font &font_, int ybase, const char *s,
            int len, ColourAllocated fore, ColourAllocated back);
    void DrawTextTransparent(PRectangle rc, Font &font_, int ybase,
            const char *s, int len, ColourAllocated fore);
    void MeasureWidths(Font &font_, const char *s, int len, int *positions);
    int WidthText(Font &font_, const char *s, int len);
    int WidthChar(Font &font_, char ch);
    int Ascent(Font &font_);
    int Descent(Font &font_);
    int InternalLeading(Font &font_) {return 0;}
    int ExternalLeading(Font &font_);
    int Height(Font &font_);
    int AverageCharWidth(Font &font_) {return WidthChar(font_, 'n');}

    int SetPalette(Palette *, bool) {return 0;}
    void SetClip(PRectangle rc);
    void FlushCachedState();

    void SetUnicodeMode(bool unicodeMode_) {unicodeMode = unicodeMode_;}
    void SetDBCSMode(int codePage) {}

    void DrawXPM(PRectangle rc, const XPM *xpm);

private:
    QFontMetrics metrics(Font &font_);
    QString convertText(const char *s, int len);
    static QRgb convertQRgb(const ColourAllocated &col, unsigned alpha);
    static QColor convertQColor(const ColourAllocated &col,
            unsigned alpha = 0xff);

    bool unicodeMode;
    QPaintDevice *pd;
    QPainter *painter;
    bool my_resources;
    int pen_x, pen_y;
};

Surface *Surface::Allocate()
{
    return new SurfaceImpl;
}

SurfaceImpl::SurfaceImpl()
    : unicodeMode(false), pd(0), painter(0), my_resources(false), pen_x(0),
      pen_y(0)
{
}

SurfaceImpl::~SurfaceImpl()
{
    Release();
}

void SurfaceImpl::Init(WindowID wid)
{
    Release();

    pd = reinterpret_cast<QWidget *>(wid);
}

void SurfaceImpl::Init(SurfaceID sid, WindowID)
{
    Release();

    // This method, and the SurfaceID type, is only used when printing.  As it
    // is actually a void * we pass (when using SCI_FORMATRANGE) a pointer to a
    // QPainter rather than a pointer to a SurfaceImpl as might be expected.
    QPainter *p = reinterpret_cast<QPainter *>(sid);

    pd = p->device();
    painter = p;
}

void SurfaceImpl::Init(QPainter *p)
{
    Release();

    pd = p->device();
    painter = p;
}

void SurfaceImpl::InitPixMap(int width, int height, Surface *, WindowID)
{
    Release();

    pd = new QPixmap(width, height);
    painter = new QPainter(pd);
    my_resources = true;
}

void SurfaceImpl::Release()
{
    if (my_resources)
    {
        if (painter)
            delete painter;

        if (pd)
            delete pd;

        my_resources = false;
    }

    painter = 0;
    pd = 0;
}

void SurfaceImpl::MoveTo(int x_, int y_)
{
    Q_ASSERT(painter);

    pen_x = x_;
    pen_y = y_;
}

void SurfaceImpl::LineTo(int x_, int y_)
{
    Q_ASSERT(painter);

    painter->drawLine(pen_x, pen_y, x_, y_);

    pen_x = x_;
    pen_y = y_;
}

void SurfaceImpl::PenColour(ColourAllocated fore)
{
    Q_ASSERT(painter);

    painter->setPen(convertQColor(fore));
}

void SurfaceImpl::Polygon(Point *pts, int npts, ColourAllocated fore,
        ColourAllocated back)
{
    Q_ASSERT(painter);

    QPolygon qpts(npts);

    for (int i = 0; i < npts; ++i)
        qpts.setPoint(i, pts[i].x, pts[i].y);

    painter->setPen(convertQColor(fore));
    painter->setBrush(convertQColor(back));
    painter->drawPolygon(qpts);
}

void SurfaceImpl::RectangleDraw(PRectangle rc, ColourAllocated fore,
        ColourAllocated back)
{
    Q_ASSERT(painter);

    painter->setPen(convertQColor(fore));
    painter->setBrush(convertQColor(back));
    painter->drawRect(rc.left, rc.top, rc.right - rc.left - 1,
            rc.bottom - rc.top - 1);
}

void SurfaceImpl::FillRectangle(PRectangle rc, ColourAllocated back)
{
    Q_ASSERT(painter);

    painter->setPen(Qt::NoPen);
    painter->setBrush(convertQColor(back));
    painter->drawRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

void SurfaceImpl::FillRectangle(PRectangle rc, Surface &surfacePattern)
{
    Q_ASSERT(painter);

    SurfaceImpl &si = static_cast<SurfaceImpl &>(surfacePattern);
    QPixmap *pm = static_cast<QPixmap *>(si.pd);

    if (pm)
    {
        QBrush brsh(Qt::black, *pm);

        painter->setPen(Qt::NoPen);
        painter->setBrush(brsh);
        painter->drawRect(rc.left, rc.top, rc.right - rc.left,
                rc.bottom - rc.top);
    }
    else
        FillRectangle(rc, ColourAllocated(0));
}

void SurfaceImpl::RoundedRectangle(PRectangle rc, ColourAllocated fore,
        ColourAllocated back)
{
    Q_ASSERT(painter);

    painter->setPen(convertQColor(fore));
    painter->setBrush(convertQColor(back));
    painter->drawRoundRect(rc.left, rc.top, rc.right - rc.left,
            rc.bottom - rc.top);
}

void SurfaceImpl::AlphaRectangle(PRectangle rc, int cornerSize,
        ColourAllocated fill, int alphaFill, ColourAllocated outline,
        int alphaOutline, int)
{
    Q_ASSERT(painter);

    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    // Assume that "cornerSize" means outline width.
    if (cornerSize > 0)
        painter->setPen(QPen(QColor(convertQRgb(outline, alphaOutline)), cornerSize));
    else
        painter->setPen(Qt::NoPen);

    painter->setBrush(QColor(convertQRgb(fill, alphaFill)));
    painter->drawRect(rc.left, rc.top, w, h);
}

void SurfaceImpl::Ellipse(PRectangle rc, ColourAllocated fore,
        ColourAllocated back)
{
    Q_ASSERT(painter);

    painter->setPen(convertQColor(fore));
    painter->setBrush(convertQColor(back));
    painter->drawEllipse(rc.left, rc.top, rc.right - rc.left,
            rc.bottom - rc.top);
}

void SurfaceImpl::Copy(PRectangle rc, Point from, Surface &surfaceSource)
{
    Q_ASSERT(painter);

    SurfaceImpl &si = static_cast<SurfaceImpl &>(surfaceSource);

    if (si.pd)
    {
        QPixmap *pm = static_cast<QPixmap *>(si.pd);

        painter->drawPixmap(rc.left, rc.top, *pm, from.x, from.y,
                rc.right - rc.left, rc.bottom - rc.top);
    }
}

void SurfaceImpl::DrawTextNoClip(PRectangle rc, Font &font_, int ybase,
        const char *s, int len,ColourAllocated fore, ColourAllocated back)
{
    Q_ASSERT(painter);

    FillRectangle(rc, back);
    DrawTextTransparent(rc, font_, ybase, s, len, fore);
}

void SurfaceImpl::DrawTextClipped(PRectangle rc, Font &font_, int ybase,
        const char *s, int len, ColourAllocated fore, ColourAllocated back)
{
    Q_ASSERT(painter);

    SetClip(rc);
    DrawTextNoClip(rc, font_, ybase, s, len, fore, back);
    painter->setClipping(false);
}

void SurfaceImpl::DrawTextTransparent(PRectangle rc, Font &font_, int ybase,
        const char *s, int len, ColourAllocated fore)
{
    Q_ASSERT(painter);

    QString qs = convertText(s, len);

    QFont *f = PFont(font_.GetID());

    if (f)
        painter->setFont(*f);

    painter->setPen(convertQColor(fore));
    painter->drawText(rc.left, ybase, qs);
}

void SurfaceImpl::DrawXPM(PRectangle rc, const XPM *xpm)
{
    Q_ASSERT(painter);

    int x, y;
    const QPixmap &qpm = xpm->Pixmap();

    x = rc.left + (rc.Width() - qpm.width()) / 2;
    y = rc.top + (rc.Height() - qpm.height()) / 2;

    painter->drawPixmap(x, y, qpm);
}

void SurfaceImpl::MeasureWidths(Font &font_, const char *s, int len,
        int *positions)
{
    QFontMetrics fm = metrics(font_);
    QString qs = convertText(s, len);
    int totalWidth = 0, ui = 0;

    for (int i = 0; i < qs.length(); ++i)
    {
        totalWidth += fm.width(qs[i]);

        int l = (unicodeMode ? QString(qs[i]).toUtf8().length() : 1);

        while (l--)
            positions[ui++] = totalWidth;
    }
}

int SurfaceImpl::WidthText(Font &font_, const char *s, int len)
{
    QString qs = convertText(s, len);

    return metrics(font_).width(qs, qs.length());
}

int SurfaceImpl::WidthChar(Font &font_, char ch)
{
    return metrics(font_).width(ch);
}

int SurfaceImpl::Ascent(Font &font_)
{
    return metrics(font_).ascent();
}

int SurfaceImpl::Descent(Font &font_)
{
    // Qt doesn't include the baseline in the descent, so add it.  Note that
    // a descent from Qt4 always seems to be 2 pixels larger (irrespective of
    // font or size) than the same descent from Qt3.  This means that text is a
    // little more spaced out with Qt4 - and will be more noticeable with
    // smaller fonts.
    return metrics(font_).descent() + 1;
}

int SurfaceImpl::ExternalLeading(Font &font_)
{
    // Scintilla doesn't use this at the moment, which is good because Qt4 can
    // return a negative value.
    return metrics(font_).leading();
}

int SurfaceImpl::Height(Font &font_)
{
    return metrics(font_).height();
}

void SurfaceImpl::SetClip(PRectangle rc)
{
    Q_ASSERT(painter);

    painter->setClipRect(rc.left, rc.top, rc.right - rc.left,
            rc.bottom - rc.top);
}

void SurfaceImpl::FlushCachedState()
{
}

// Get the metrics for a font.
QFontMetrics SurfaceImpl::metrics(Font &font_)
{
    QFont *f = PFont(font_.GetID()), fnt;

    if (f)
        fnt = *f;
    else
        fnt = QApplication::font();

    return QFontMetrics(fnt, pd);
}

// Convert a Scintilla string to a Qt Unicode string.
QString SurfaceImpl::convertText(const char *s, int len)
{
    if (unicodeMode)
        return QString::fromUtf8(s, len);

    return QString::fromLatin1(s, len);
}

// Convert a Scintilla colour and alpha component to a Qt QRgb.
QRgb SurfaceImpl::convertQRgb(const ColourAllocated &col, unsigned alpha)
{
    long c = col.AsLong();

    unsigned r = c & 0xff;
    unsigned g = (c >> 8) & 0xff;
    unsigned b = (c >> 16) & 0xff;

    QRgb rgba = (alpha << 24) | (r << 16) | (g << 8) | b;

    return rgba;
}

// Convert a Scintilla colour, and optional alpha component, to a Qt QColor.
QColor SurfaceImpl::convertQColor(const ColourAllocated &col, unsigned alpha)
{
    return QColor(convertQRgb(col, alpha));
}


// Window (widget) management.
Window::~Window()
{
}

void Window::Destroy()
{
    QWidget *w = PWindow(id);

    if (w)
    {
        // Delete the widget next time round the event loop rather than
        // straight away.  This gets round a problem when auto-completion lists
        // are cancelled after an entry has been double-clicked, ie. the list's
        // dtor is called from one of the list's slots.  There are other ways
        // around the problem but this is the simplest and doesn't seem to
        // cause problems of its own.
        w->deleteLater();
        id = 0;
    }
}

bool Window::HasFocus()
{
    return PWindow(id)->hasFocus();
}

PRectangle Window::GetPosition()
{
    QWidget *w = PWindow(id);

    // Before any size allocated pretend its big enough not to be scrolled.
    PRectangle rc(0,0,5000,5000);

    if (w)
    {
        const QRect &r = w->geometry();

        rc.right = r.right() - r.left() + 1;
        rc.bottom = r.bottom() - r.top() + 1;
    }

    return rc;
}

void Window::SetPosition(PRectangle rc)
{
    PWindow(id)->setGeometry(rc.left, rc.top, rc.right - rc.left,
            rc.bottom - rc.top);
}

void Window::SetPositionRelative(PRectangle rc, Window relativeTo)
{
    QWidget *rel = PWindow(relativeTo.id);
    QPoint pos = rel->mapToGlobal(rel->pos());

    int x = pos.x() + rc.left;
    int y = pos.y() + rc.top;

    PWindow(id)->setGeometry(x, y, rc.right - rc.left, rc.bottom - rc.top);
}

PRectangle Window::GetClientPosition()
{
    return GetPosition();
}

void Window::Show(bool show)
{
    QWidget *w = PWindow(id);

    if (show)
        w->show();
    else
        w->hide();
}

void Window::InvalidateAll()
{
    QWidget *w = PWindow(id);

    if (w)
        w->update();
}

void Window::InvalidateRectangle(PRectangle rc)
{
    QWidget *w = PWindow(id);

    if (w)
        w->update(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

void Window::SetFont(Font &font)
{
    PWindow(id)->setFont(*PFont(font.GetID()));
}

void Window::SetCursor(Cursor curs)
{
    Qt::CursorShape qc;

    switch (curs)
    {
    case cursorText:
        qc = Qt::IBeamCursor;
        break;

    case cursorUp:
        qc = Qt::UpArrowCursor;
        break;

    case cursorWait:
        qc = Qt::WaitCursor;
        break;

    case cursorHoriz:
        qc = Qt::SizeHorCursor;
        break;

    case cursorVert:
        qc = Qt::SizeVerCursor;
        break;

    case cursorHand:
        qc = Qt::PointingHandCursor;
        break;

    default:
        qc = Qt::ArrowCursor;
    }

    PWindow(id)->setCursor(qc);
}

void Window::SetTitle(const char *s)
{
    PWindow(id)->setWindowTitle(s);
}


// Menu management.
Menu::Menu() : id(0)
{
}

void Menu::CreatePopUp()
{
    Destroy();
    id = new SciPopup();
}

void Menu::Destroy()
{
    SciPopup *m = PMenu(id);

    if (m)
    {
        delete m;
        id = 0;
    }
}

void Menu::Show(Point pt, Window &)
{
    PMenu(id)->popup(QPoint(pt.x, pt.y));
}


class DynamicLibraryImpl : public DynamicLibrary
{
public:
    DynamicLibraryImpl(const char *modulePath)
    {
        m = new QLibrary(modulePath);
        m->load();
    }

    virtual ~DynamicLibraryImpl()
    {
        if (m)
            delete m;
    }

    virtual Function FindFunction(const char *name)
    {
        if (m)
            return m->resolve(name);

        return 0;
    }

    virtual bool IsValid()
    {
        return m && m->isLoaded();
    }

private:
    QLibrary* m;
};

DynamicLibrary *DynamicLibrary::Load(const char *modulePath)
{
    return new DynamicLibraryImpl(modulePath);
}


// Elapsed time.  This implementation assumes that the maximum elapsed time is
// less than 48 hours.
ElapsedTime::ElapsedTime()
{
    QTime now = QTime::currentTime();

    bigBit = now.hour() * 60 * 60 + now.minute() * 60 + now.second();
    littleBit = now.msec();
}

double ElapsedTime::Duration(bool reset)
{
    long endBigBit, endLittleBit;
    QTime now = QTime::currentTime();

    endBigBit = now.hour() * 60 * 60 + now.minute() * 60 + now.second();
    endLittleBit = now.msec();

    double duration = endBigBit - bigBit;

    if (duration < 0 || (duration == 0 && endLittleBit < littleBit))
        duration += 24 * 60 * 60;

    duration += (endLittleBit - littleBit) / 1000.0;

    if (reset)
    {
        bigBit = endBigBit;
        littleBit = endLittleBit;
    }

    return duration;
}


// Manage system wide parameters.
ColourDesired Platform::Chrome()
{
    return ColourDesired(0xe0,0xe0,0xe0);
}

ColourDesired Platform::ChromeHighlight()
{
    return ColourDesired(0xff,0xff,0xff);
}

const char *Platform::DefaultFont()
{
    static QByteArray def_font;

    def_font = QApplication::font().family().toAscii();

    return def_font.constData();
}

int Platform::DefaultFontSize()
{
    return QApplication::font().pointSize();
}

unsigned int Platform::DoubleClickTime()
{
    return QApplication::doubleClickInterval();
}

bool Platform::MouseButtonBounce()
{
    return true;
}

#if defined(__APPLE__)
bool Platform::WaitMouseMoved(Point pt)
{
    // For the moment don't call ::WaitMouseMoved() and see if anybody
    // complains.  If we need it then we will need to define SCI_NAMESPACE.
    // However the proper solution would be to use QApplication's
    // startDragTime() and startDragDistance() for all platforms.
    return true;
}
#endif

void Platform::DebugDisplay(const char *s)
{
    qDebug("%s", s);
}

bool Platform::IsKeyDown(int)
{
    return false;
}

long Platform::SendScintilla(WindowID w, unsigned int msg,
        unsigned long wParam, long lParam)
{
    return static_cast<QsciScintillaBase *>(PWindow(w)->parentWidget())->SendScintilla(msg, wParam, lParam);
}

long Platform::SendScintillaPointer(WindowID w, unsigned int msg,
        unsigned long wParam, void *lParam)
{
    return static_cast<QsciScintillaBase *>(PWindow(w)->parentWidget())->SendScintilla(msg, wParam, reinterpret_cast<long>(lParam));
}

bool Platform::IsDBCSLeadByte(int, char)
{
    // We don't support DBCS.
    return false;
}

int Platform::DBCSCharLength(int, const char *)
{
    // We don't support DBCS.
    return 1;
}

int Platform::DBCSCharMaxLength()
{
    // We don't support DBCS.
    return 2;
}

int Platform::Minimum(int a, int b)
{
    return (a < b) ? a : b;
}

int Platform::Maximum(int a, int b)
{
    return (a > b) ? a : b;
}

int Platform::Clamp(int val, int minVal, int maxVal)
{
    if (val > maxVal)
        val = maxVal;

    if (val < minVal)
        val = minVal;

    return val;
}


//#define TRACE

#ifdef TRACE
void Platform::DebugPrintf(const char *format, ...)
{
    char buffer[2000];
    va_list pArguments;

    va_start(pArguments, format);
    vsprintf(buffer, format, pArguments);
    va_end(pArguments);

    DebugDisplay(buffer);
}
#else
void Platform::DebugPrintf(const char *, ...)
{
}
#endif

static bool assertionPopUps = true;

bool Platform::ShowAssertionPopUps(bool assertionPopUps_)
{
    bool ret = assertionPopUps;

    assertionPopUps = assertionPopUps_;

    return ret;
}

void Platform::Assert(const char *c, const char *file, int line)
{
    qFatal("Assertion [%s] failed at %s %d\n", c, file, line);
}
