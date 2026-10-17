#include <../../config.h>

/*
Copyright (C) 1993 Tim Prinzing
Copyright (C) 2002 Tim Prinzing, Michael Hines
This file contains programs and data originally developed by Tim Prinzing
with minor changes and improvements by Michael Hines.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// =========================================================================
//
//				<IV-Win/canvas16.c>
//
// MS-Windows implementation of the InterViews Canvas class using only 
// those features of the MS-Windows SDK that are available in Win16 and
// Win32s.
//
//
// ========================================================================

/*
 * THIS FILE CONTAINS PORTIONS OF THE InterViews 3.1 DISTRIBUTION THAT 
 * CONTAINED THE FOLLOWING COPYRIGHT:
 *
 *
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */
 
#include <IV-Win/MWlib.h>
#include <IV-Win/canvas.h>
#include <InterViews/transformer.h>
#include <InterViews/resource.h>
#include <InterViews/brush.h>
#include <InterViews/color.h>
#include <InterViews/bitmap.h>
#include <InterViews/raster.h>
#include <InterViews/font.h>
#include <IV-Win/brush.h>
#include <IV-Win/color.h>
#include <IV-Win/bitmap.h>
#include <IV-Win/raster.h>
#include <IV-Win/font.h>

#include <math.h>
#include <ctype.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#undef Polygon

const int TEXT_BUFFER_LEN = 80;			// max buffered text length
PathRenderInfo MWcanvas16::path_;

//
// converts a Coordinate to TWIPS, which is what is expected for most
// of the Windows operations (ie those using logical coordainates).
//
inline int COORD2TWIPS(Coord c)
{
	return int( c * 20.0 + ((c > 0) ? 0.5 : -0.5));
}


// -----------------------------------------------------------------------
// constructors/destructors
// -----------------------------------------------------------------------
MWcanvas16::MWcanvas16() 
{
	// ---- initialize the path information ----
	PathRenderInfo* p = &MWcanvas16::path_;
	if (p->point_ == nil)
	{
		p->point_ = new POINT[10];
        p->cur_point_ = p->point_;
		p->end_point_ = p->point_ + 10;
	}

	// ---- set pen template to a solid thin black line ----
	pen_stats_.lopnStyle = PS_SOLID;
	pen_stats_.lopnWidth.x = 0;
	pen_stats_.lopnWidth.y = 0; // to avoid UMR with purify
	pen_stats_.lopnColor = RGB(0,0,0);

	// ---- set brush to a solid white ----
	brush_stats_.lbStyle = BS_SOLID;
	brush_stats_.lbColor = RGB(255,255,255);
	brush_stats_.lbHatch = 0; // to avoid UMR with purify

	// ---- initially we have no logical attributes ----
	lg_color_ = nil;
	lg_brush_ = nil;
	lg_font_ = nil;

	// ---- initialize the text buffer ----
	text_buff_ = new char[TEXT_BUFFER_LEN];
	text_ptr_ = text_buff_;
	text_width_ = 0;
	text_x0_ = 0;
	text_y0_ = 0;
	text_x_ptr_ = 0;
    text_y_ptr_ = 0;
}

MWcanvas16::~MWcanvas16()
{
	delete text_buff_;
}

// -----------------------------------------------------------------------
// Initialize for begin of paint sequence... the display context is now
// valid.  Note that this isn't necessarily a result of the WM_PAINT
// message!
// -----------------------------------------------------------------------
void MWcanvas16::beginPaint(
	HDC hdc,							// device context to use
	const RECT& r)             			// area of update region
{
	MWcanvas::beginPaint(hdc, r);

    // ---- establish default pen and brush ----
	pen_ = CreatePenIndirect(&pen_stats_);
	old_pen_ = (HPEN) SelectObject(drawable_, pen_);
	brush_ = CreateBrushIndirect(&brush_stats_);
	old_brush_ = (HBRUSH) SelectObject(drawable_, brush_);
}

void MWcanvas16::endPaint()
{
	// ---- finish any buffered operations ----
	flush();

	// ---- release GDI objects ----
	SelectObject(drawable_, old_pen_);
	DeleteObject(pen_);
	pen_ = nil;
	SelectObject(drawable_, old_brush_);
	DeleteObject(brush_);
	brush_ = nil;

	MWcanvas::endPaint();
}

// ----------------------------------------------------------------------
// utility functions
// ----------------------------------------------------------------------

static bool rectangular(
	int x0,
	int y0,
	int x1,
	int y1,
	int x2,
	int y2,
	int x3,
	int y3)
{
    return (
        (x0 == x1 && y1 == y2 && x2 == x3 && y3 == y0) ||
        (x0 == x3 && y3 == y2 && x2 == x1 && y1 == y0)
    );
}

static bool xrect(const POINT* p, unsigned int n)
{
    return (
	n == 5 && p[0].x == p[4].x && p[0].y == p[4].y &&
	rectangular(
	    p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y, p[3].x, p[3].y
	)
    );
}

static const float smoothness = 10.0;

static bool straight(
    const Transformer& tx,
    Coord x0, Coord y0, Coord x1, Coord y1,
	Coord x2, Coord y2, Coord x3, Coord y3)
{
    Coord tx0, tx1, tx2, tx3;
    Coord ty0, ty1, ty2, ty3;
    tx.transform(x0, y0, tx0, ty0);
    tx.transform(x1, y1, tx1, ty1);
    tx.transform(x2, y2, tx2, ty2);
    tx.transform(x3, y3, tx3, ty3);
    float f = (
        (tx1 + tx2) * (ty0 - ty3) + (ty1 + ty2) * (tx3 - tx0)
        + 2 * (tx0 * ty3 - ty0 * tx3)
    );
    return (f * f) < smoothness;
}

static inline Coord mid(Coord a, Coord b)
{
    return (a + b) / 2;
}

// ----------------------------------------------------------------------
// Transformation 
// ----------------------------------------------------------------------

void MWcanvas16::push_transform()
{
	flush();
	MWcanvas::push_transform();
}

void MWcanvas16::pop_transform()
{
	flush();
	MWcanvas::pop_transform();
}

void MWcanvas16::transform(const Transformer& t)
{
	flush();
	MWcanvas::transform(t);
}

void MWcanvas16::transformer(const Transformer& t)
{
	flush();
	MWcanvas::transformer(t);
}
const Transformer& MWcanvas16::transformer() const
{
	return MWcanvas::transformer();
}

// ----------------------------------------------------------------------
// clipping 
// ----------------------------------------------------------------------

void MWcanvas16::push_clipping(bool)
{
	flush();
	MWcanvas::push_clipping();
}

void MWcanvas16::pop_clipping()
{
	flush();
	MWcanvas::pop_clipping();
}

void MWcanvas16::clip()
{
	flush();

    // ---- make sure there are multiple points ----
    PathRenderInfo* p = &path_;
	POINT* pt = p->point_;
	int n = (int) (p->cur_point_ - p->point_);
	if (n <= 2)
	{
		return;
	}

	//
	// The path must be in terms of device units for specification of 
	// clipping... so we transform the path specifications from logical 
	// units (TWIPS) to device units before setting the clipping.
	//
	if(drawable_ == nil)	// sometimes its null because of event input -- just ignore
	  return;
	MWassert( LPtoDP(drawable_, pt, n) );


	// ---- create a region to represent clipping ----
	HRGN clip;
	if (xrect(pt, n))
	{
		// ---- rectangular clipping area ----
		RECT xr;
		xr.left = Math::min(pt[0].x, pt[2].x);
		xr.top = Math::min(pt[0].y, pt[2].y);
		xr.right = Math::max(pt[0].x, pt[2].x);
		xr.bottom = Math::max(pt[0].y, pt[2].y);
		clip = CreateRectRgn(xr.left, xr.top, xr.right, xr.bottom );
	}
	else
	{
		clip = CreatePolygonRgn(pt, n, WINDING);
    }
#if 1
	// ---- merge with existing clipping area ----
	HRGN intersect = CreateRectRgn(0,0,0,0);
	CombineRgn(intersect, clipping_, clip, RGN_AND);
	DeleteObject(clip);
	clip = intersect;
#endif
//DebugMessage

	// ---- set new clipping area ----
	SelectClipRgn(drawable_, clip);
	DeleteObject(clipping_);
	clipping_ = clip;

#ifdef DEBUG
	// ---- canvas debug ----
	printf("clip ");
	for (int i = 0; i < n; i++)
	{
		printf("[%d,%d] ", pt[i].x, pt[i].y);
	}
	printf("\n");
#endif

}

void MWcanvas16::new_path()
{
    PathRenderInfo* p = &path_;
    p->curx_ = 0;
    p->cury_ = 0;
	POINT* xp = p->point_;
    xp->x = 0;
    xp->y = 0;
    p->cur_point_ = xp;
}

void MWcanvas16::move_to(Coord x, Coord y)
{
    PathRenderInfo* p = &path_;
    p->curx_ = x;
    p->cury_ = y;
    Coord tx, ty;
	if (transformed_)
	{
		matrix().transform(x, y, tx, ty);
	}
	else
	{
		tx = x;
		ty = y;
	}
#if 0
	if (tx < -1500. || tx > 1500. || ty < -1500. || ty > 1500.) {
		p->cur_point_ = p->point_;
		return;
	}
#endif

	// -- convert to TWIPS --
	POINT* xp = p->point_;
	xp->x = COORD2TWIPS(tx);
	xp->y = COORD2TWIPS(ty);
	p->cur_point_ = xp + 1;
}

void MWcanvas16::line_to(Coord x, Coord y)
{
    PathRenderInfo* p = &path_;
    p->curx_ = x;
    p->cury_ = y;
    Coord tx, ty;
	if (transformed_)
	{
		matrix().transform(x, y, tx, ty);
	}
	else
	{
		tx = x;
		ty = y;
	 }

#if 0
	if (tx < -1500. || tx > 1500. || ty < -1500. || ty > 1500.) {
		return;
	}
#endif

	if (p->cur_point_ == p->end_point_)
	{
		int old_size = (int) (p->cur_point_ - p->point_);
		int new_size = 2 * old_size;
        POINT* new_path = new POINT[new_size];
		for (int i = 0; i < old_size; i++)
		{
            new_path[i] = p->point_[i];
        }
        delete p->point_;
        p->point_ = new_path;
		p->cur_point_ = p->point_ + old_size;
		p->end_point_ = p->point_ + new_size;
	}

	// -- convert to TWIPS --
	POINT* xp = p->cur_point_;
	xp->x = COORD2TWIPS(tx);
	xp->y = COORD2TWIPS(ty);
	p->cur_point_ = xp + 1;
}

void MWcanvas16::curve_to(
	Coord x,
    Coord y,
	Coord x1,
	Coord y1,
	Coord x2,
	Coord y2)
{
    PathRenderInfo* p = &MWcanvas16::path_;
    Coord px = p->curx_;
    Coord py = p->cury_;

	if (straight(matrix(), px, py, x1, y1, x2, y2, x, y))
	{
        line_to(x, y);
	}
	else
	{
        Coord xx = mid(x1, x2);
        Coord yy = mid(y1, y2);
        Coord x11 = mid(px, x1);
        Coord y11 = mid(py, y1);
        Coord x22 = mid(x2, x);
        Coord y22 = mid(y2, y);
        Coord x12 = mid(x11, xx);
        Coord y12 = mid(y11, yy);
        Coord x21 = mid(xx, x22);
        Coord y21 = mid(yy, y22);
        Coord cx = mid(x12, x21);
        Coord cy = mid(y12, y21);

        curve_to(cx, cy, x11, y11, x12, y12);
        curve_to(x, y, x21, y21, x22, y22);
    }
}

void MWcanvas16::close_path()
{
    PathRenderInfo* p = &MWcanvas16::path_;
	POINT* startp = p->point_;
	POINT* xp = p->cur_point_;
    xp->x = startp->x;
    xp->y = startp->y;
    p->cur_point_ = xp + 1;
}

void MWcanvas16::stroke(const Color* c, const Brush* b)
{
	PathRenderInfo* p = &MWcanvas16::path_;

	// --- determine the number of points, and if valid ---
	int n = (int) (p->cur_point_ - p->point_);
	if (n < 2)
	{
		return;
    }
	flush();
    color(c);
	brush(b);

	// ---- determine the shape, and render it ----
	POINT* pt = p->point_;
	if (n == 2)
	{
    	// ---- draw a line ----
#ifdef WIN16
		MoveTo(drawable_, pt[0].x, pt[0].y);
#else
		MoveToEx(drawable_, pt[0].x, pt[0].y, NULL);
#endif
		LineTo(drawable_, pt[1].x, pt[1].y);
	}
	else
	{
		// ---- draw a multi-line segment ----
		Polyline(drawable_, pt, n);
	}
}

void MWcanvas16::fill(const Color* c)
{
	if (c->rep()->stipple)
	{
		// ---- stipple some color into the area ----
		stencilFill(c->rep()->stipple, c);
	}
	else
	{
		PathRenderInfo* p = &MWcanvas16::path_;
		int n = (int) (p->cur_point_ - p->point_);
		if (n <= 2)
		{
			return;
		}

		flush();
		color(c);
		HPEN hpen = (HPEN)SelectObject(drawable_, CreatePen(PS_SOLID, 0, lg_color_->rep()->msColor()));

		POINT* pt = p->point_;
		if (xrect(pt, n))
		{
			// ---- draw a rectangle ----
			int x = Math::min(pt[0].x, pt[2].x);
			int y = Math::min(pt[0].y, pt[2].y);
			int x1 = Math::max(pt[0].x, pt[2].x);
			int y1 = Math::max(pt[0].y, pt[2].y);
			Rectangle(drawable_, x, y, x1, y1);
		}
		else
		{
			Polygon(drawable_, pt, n);
		}
		DeleteObject(SelectObject(drawable_, hpen));
	}

#ifdef DEBUG
	// ---- canvas debug ----
	MWassert( LPtoDP(drawable_, pt, n) );
	printf("fill ");
	for (int i = 0; i < n; i++)
	{
		BOOL vis = PtVisible(drawable_,  pt[i].x, pt[i].y);
		if (vis)
			printf("[%d,%d] ", pt[i].x, pt[i].y);
		else
			printf("(%d,%d> ", pt[i].x, pt[i].y);
	}
	printf("\n");
#endif
}

// -----------------------------------------------------------------------
// Changes the Color to use when rendering onto the canvas.
// -----------------------------------------------------------------------
void MWcanvas16::color(const Color* c)
{
	// ---- render anything that was buffered ----
	flush();

	// ---- reference new color ----
	Resource::ref(c);
	Resource::unref(lg_color_);
	lg_color_ = c;

	// ---- set new pen color ----
	pen_stats_.lopnColor = c->rep()->msColor();
	pen_ = CreatePenIndirect(&pen_stats_);
	DeleteObject( SelectObject(drawable_, pen_));

	// ----  set new brush color ----
	if (c->rep()->stipple)
	{
		// THIS NEEDS WORK
		//
		// brush_stats_.lbStyle = BS_PATTERN;
		// brush_stats_.lbHatch = (long) c->rep()->stipple->hbm_;
	}
	else
	{
		brush_stats_.lbStyle = BS_SOLID;
		brush_stats_.lbColor = c->rep()->msColor();
	}
	brush_ = CreateBrushIndirect(&brush_stats_);
	DeleteObject( SelectObject(drawable_, brush_));

	// ---- set text color ----
	SetTextColor(drawable_, c->rep()->msColor());

	// ---- set color operation ----
	ColorRep* r = c->rep();
	if (r->op == Color::Copy)
	{
		SetROP2(drawable_, R2_COPYPEN);
	}
	else if (r->op == Color::Xor)
	{
//		SetROP2(drawable_, R2_XORPEN);
		SetROP2(drawable_, 6);   // this is insane, but...
	}
	else if (r->op == Color::Invisible)
	{
		SetROP2(drawable_, R2_NOP);
	}
	else
	{
		// ----- unrecognized color mode ----
		MWassert(0);
	}
}

// -----------------------------------------------------------------------
// Changes the brush to use when rendering a path
// -----------------------------------------------------------------------
void MWcanvas16::brush(const Brush* b)
{
	if (b != nil && b != lg_brush_)
	{
    	// ---- reference the new brush ----
		Resource::ref((const Resource*) b);
		Resource::unref((const Resource*) lg_brush_);
		lg_brush_ = b;

		  // ---- get ready to create a new pen ----
		BrushRep& br = * b->rep(nil);
		pen_stats_.lopnStyle = (br.dashCount) ? PS_DASH : PS_SOLID;

		pen_stats_.lopnWidth.x = COORD2TWIPS(br.penWidth);

		// ---- establish the new pen ----
		pen_ = CreatePenIndirect(&pen_stats_);
		DeleteObject( SelectObject(drawable_, pen_));

    }
}

// -----------------------------------------------------------------------
// Changes the font to use when rendering.  The actual creation and
// selection of the GDI object is delayed until the actual text is to be
// rendered, since further scaling and rotation might be desired.
// -----------------------------------------------------------------------
void MWcanvas16::font(const Font* f)
{
	if (f != nil && f != lg_font_)
	{
		// ---- flush any buffered operations ----
		flush();

		// ---- register interest in new font, and release old ----
		Resource::ref(f);
		Resource::unref(lg_font_);
		lg_font_ = f;
	}
}

// -----------------------------------------------------------------------
// Render a character onto the canvas.  This is a buffered operation that
// tries to draw entire words at once.  This mode is to provide compati-
// bility with the original InterViews.  An extended interface will also
// be provided that bypasses the buffering for operations less complex
// than document type functions, and requiring higher performance out of
// the text rendering.
// -----------------------------------------------------------------------
void MWcanvas16::character(
	const Font* f, 					// font to use
	long ch,                        // character to print
	Coord width,                    // width of the character
	const Color* c,             	// color to use
	Coord x,                        // x-coordinate
	Coord y)                        // y-coordinate
{
//return;
	font(f);
	color(c);

	// ---- a space or unprintable marks the end of a word ----
	if ((ch == ' ') || (! isprint(ch)))
	{
		flush();
        return;
	}

	// Transform the coordinates before testing for expected position
	// so that we can work in terms of pixels and deal with slop in
	// position better.
	Transformer& m = matrix();
	Coord tx = x;
    Coord ty = y;
	if (transformed_)
	{
		m.transform(tx, ty);

		// ROTATION SUPPORT TO BE ADDED
	}

	int ix = COORD2TWIPS( tx );
	int iy = COORD2TWIPS( ty );

	// test if this is a different word (ie not in the expected
	// position.  If it is a new word, the old word is flushed
	// and this character will be treated as the first of the word.
	// ROTATION SUPPORT TO BE ADDED
	if ((ix > (text_x_ptr_ + 1)) || (ix < (text_x_ptr_ - 1)) ||
		(iy != text_y_ptr_))
	{
		flush();
	}

    // ---- test if this is the first new character ---- 
	if (text_ptr_ == text_buff_)
	{
		// ---- this is the first new character since a flush ----
		// Determine the coordinates, size, and rotation.  Since
		// the rotation needs to be specified in the LOGFONT, we need

		// to extract this information from the transformation matrix.
		text_x0_ = ix;
		text_y0_ = iy;
		text_width_ = 0;
		text_x_ptr_ = text_x0_;
		text_y_ptr_ = text_y0_;
	}

	// ---- record the new character information ----
	text_width_ += COORD2TWIPS( width );
	text_x_ptr_ += COORD2TWIPS( width );
	text_y_ptr_ = text_y0_;
	*text_ptr_++ = char(ch & 0xff);
}

// -----------------------------------------------------------------------
// Flush the text buffer.  The text drawing is buffered until something
// changes and causes a flush (by calling this function).  Nearly all of
// the work was previously done by setting up the LOGFONT structure
// properly
//
// Some adjustment is required to cause proper placement in the y
// direction.  The target position of the text is the text baseline from
// the InterViews point of view, and upper left corner from the Windows
// point of view.  We therefore need to back off the ascent since the two
// coordinate systems have the y-axis going in opposite directions.
// -----------------------------------------------------------------------

// well, what if we cache the HFONT on the font object itself?  lets try it!

/* need to come to terms with fact that CreateFontIndirect slows things down
to an extent which makes Win32s unusable. This hack allows only one font.
*/

//  static HFONT new_fnt;
//  void cleanup_new_fnt() {
//  	if (new_fnt) {
//  		DeleteObject(new_fnt);
//  	}
//  }

void MWcanvas16::flush()
{
	// ---- check if there is anything to do ----
	int nchars = (int) (text_ptr_ - text_buff_);
	if ((nchars == 0) || (lg_font_ == nil))
		return;

	// ---- render the text ----
	 SetBkMode(drawable_, TRANSPARENT);
	FontRep* fr = lg_font_->rep(nil);
	MWassert(fr);

// the one consequence of caching the font is that it is no longer possible
// to transform the angle like this at this point.  so we lose rotated text
// but gain the ability to use different fonts!
	//	fr->orientation(transformAngle()); 
//	HFONT new_fnt = fr->Create();
//  	if (!new_fnt) {
//  		new_fnt = fr->Create();
//  	}
	HFONT old_fnt = (HFONT) SelectObject(drawable_, fr->HFont());
	SetTextAlign(drawable_, TA_LEFT | TA_BASELINE | TA_NOUPDATECP);
	TextOut(drawable_, text_x0_, text_y0_, text_buff_, nchars);
	SelectObject(drawable_, old_fnt);
//	DeleteObject( SelectObject(drawable_, old_fnt));
	// ---- reset the buffer ----
	text_ptr_ = text_buff_;

}

// -----------------------------------------------------------------------
// Determine the angle of transformation in terms of tenths-of-a-degree.
// The method used is to transform a couple of points, and then measure
// the angle of rotation on the sample points.  There is probably a better
// way to do this, but it works.
// -----------------------------------------------------------------------
int MWcanvas16::transformAngle() const
{
	if (! transformed_ )
		return 0;

	Transformer& t = matrix();
	float x0 = float(0); 
	float y0 = x0;
	float y1 = x0;
	float x1 = float(1); 
	t.transform(x0, y0);
	t.transform(x1, y1);
	double dx = x1 - x0;
	double dy = y1 - y0;
	double rad = atan2(dy, dx);
	int tdeg = (int) (rad * double(572.9577951));
	if (tdeg < 0)
		tdeg = 3600 + tdeg;

	return tdeg;
}


// -----------------------------------------------------------------------
// Stencil the "set" bits of the bitmap onto the canvas in the given color
// at the given origin.  The origin is in terms of the point system with
// the origin in the lower left, which must be converted to the device
// coordinate system which has an origin in the upper left.  Further, the
// blt coordinates must be adjusted this way as well.
//
// If the transformation matrix is identity, the bitmap is simply blt'd
// using the raster operation discussed in the Petzold V3.0 book on page
// 624 which transfers only the black bits.  It corresponds to the operation
// ((Destination ^ Pattern) & Source) ^ Pattern.  If the transformer is not
// identity, then the Windows function StretchBlt is used to scale the
// bitmap.  The default mode of STRETCH_ANDSCANS is appropriate since it
// favors the black bits.
//
// Currently, a rotated bitmap is NOT supported!
// -----------------------------------------------------------------------
void MWcanvas16::stencil(
    const Bitmap* mask, 			// bitmap to render
	const Color* c, 				// color to use
	Coord x, 						// x-coordinate
	Coord y) 						// y-coordinate
{
	BitmapRep& br = * mask->rep();
	HBITMAP hbm = CreateBitmapIndirect((BITMAP*) &(br.bm_));
	HDC mem_hdc = CreateCompatibleDC(drawable_);
	SelectObject(mem_hdc, hbm);

	//
	// convert the coordinates to TWIPS for the page space to device space
	// mapping.  
	//
	int x0 = COORD2TWIPS( x + mask->left_bearing() );
	int y0 = COORD2TWIPS( y - mask->descent() );
	int width = COORD2TWIPS( mask->width() );
	int height = COORD2TWIPS( mask->height() );

	
	// ---- set the color to use ----
	flush();
    color(c);

	// For some reason, text color must be black when stenciling or the
	// colors of the stencil are affected. 
	SetTextColor(drawable_, RGB(0, 0, 0));

	// ---- blt... the destination will be transformed ----
	MWassert( StretchBlt(drawable_, x0, y0 + height, width, - height, 
		mem_hdc, 0, 0, br.bm_.bmWidth, br.bm_.bmHeight, 0xB8074AL) );

	// ---- cleanup ----
	DeleteDC(mem_hdc);
	DeleteObject(hbm);
}

// -----------------------------------------------------------------------
// Render an image onto the GDI surface.  The image is transformed in 
// terms of scaling only initially (all that Windows 3.1 supports).  
// Rotation is initially not supported.
// -----------------------------------------------------------------------
void MWcanvas16::image(const Raster* ras, Coord x, Coord y)
{
	//
	// convert the coordinates to TWIPS for the page space to device space
	// mapping.  
	//
	Coord tx, ty, tw, th;
	if (transformed_) {
		matrix().transform(x, y, tx, ty);
		matrix().transform(x+ras->width(), y+ras->height(), tw, th);
		tw -= tx; th -= ty;
	} else {
		tx = x;
		ty = y;
		tw = ras->width();
		ty = ras->height();
	}

	int x0 = COORD2TWIPS( tx );
	int y0 = COORD2TWIPS( ty );
	int width = COORD2TWIPS( tw );
	int height = COORD2TWIPS( th );

	// ---- blt... the destination will be transformed ----
	MWassert( StretchBlt(drawable_, x0, y0 + height, width, - height, 
		ras->rep()->deviceContext(), 
		0, 0, ras->pwidth(), ras->pheight(), SRCCOPY) );

}

