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
//				<IV-Win/canvas.c>
//
// MS-Windows dependent Canvas representation.  This canvas type renders
// into an MS-Windows window.  Because it is largely GDI based, most of the
// CanvasRep can be reused for printing which also uses the GDI interface.
//
// True-Type fonts are used to provide scaling and rotation capabilies,
// which greatly simplifies (and increases performance of) the font scale
// and rotate capabilies of InterViews.  This class is therefore dependant
// upon MS-Windows 3.1 or later since that is the version that introduced
// True-Type fonts.
//
//
// NOTE:
//	The implementation is now divided into a WIN32s/WIN16 implementation and
//  a WIN32 implementation.  Since using the NT paths (which aren't supported
//	in Win16 or Win32s) allows better buffering accross the rpc interface,
//  it is used on NT systems to provide better performance.
//
//
// ========================================================================

/*
 * THIS FILE CONTAINS PORTIONS OF THE InterViews 3.1 DISTRIBUTION THAT 
 * CONTAINED THE FOLLOWING COPYRIGHT MESSAGE:
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

// ---- InterViews includes -----
#include <IV-Win/MWlib.h>
#include <InterViews/canvas.h>
#include <InterViews/display.h>
#include <InterViews/session.h>
#include <InterViews/bitmap.h>
#include <InterViews/brush.h>
#include <InterViews/font.h>
#include <InterViews/color.h>
#include <InterViews/transformer.h>
#include <InterViews/geometry.h>
#include <InterViews/raster.h>
#include <IV-Win/bitmap.h>
#include <IV-Win/brush.h>
#include <IV-Win/color.h>
#include <IV-Win/canvas.h>
#include <IV-Win/font.h>
#include <IV-Win/window.h>
#include <IV-Win/raster.h>


#include <OS/math.h>
#include <OS/list.h>

// ---- libc includes ----
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include "/nrn/src/mswin/winio/debug.h"
#ifdef DEBUG
#include <stdio.h>
#endif


// ---- templates used ----
declarePtrList(MWtransformPtrList,Transformer)
implementPtrList(MWtransformPtrList,Transformer)
declareList(MWclipList,HRGN)
implementList(MWclipList,HRGN)



// ########################################################
// ############  class Canvas
// ########################################################

Canvas::Canvas()
{
}

Canvas::~Canvas()
{
}

// -------------------------------------------------------------------------
// These functions are defined to do nothing as a convenience to subclasses
// that are not windowed and don't care about these.
// -------------------------------------------------------------------------

void Canvas::front_buffer()
	{ }
void Canvas::back_buffer()
	{ }
Window* Canvas::window() const
	{ return nil; }

// ----------------------------------------------------------------------
// utility functions
// ----------------------------------------------------------------------

static void rect_path(
	Canvas* c,
	Coord l,
	Coord b,
	Coord r,
	Coord t)
{
    c->new_path();
    c->move_to(l, b);
    c->line_to(l, t);
    c->line_to(r, t);
    c->line_to(r, b);
	 c->close_path();
}

// ----------------------------------------------------------------------
// convenience functions that form a typical path and do an operation
// ----------------------------------------------------------------------

void Canvas::line(
	Coord x1,
	Coord y1,
	Coord x2,
	Coord y2,
	const Color* c,
	const Brush* b)
{
    new_path();
    move_to(x1, y1);
	 line_to(x2, y2);
	 stroke(c, b);
}

void Canvas::rect(
	Coord l,
	Coord b,
	Coord r,
	Coord t,
	const Color* c,
	const Brush* br)
{
    rect_path(this, l, b, r, t);
    stroke(c, br);
}

void Canvas::fill_rect(Coord l, Coord b, Coord r, Coord t, const Color* c)
{
    rect_path(this, l, b, r, t);
    fill(c);
}

void Canvas::clip_rect(Coord l, Coord b, Coord r, Coord t)
{
    rect_path(this, l, b, r, t);
    clip();
}

Coord Canvas::to_pixels_coord(Coord p, DimensionName d) const
{
	return to_coord(to_pixels(p, d), d);
}

// --------------------------------------------------------------------------
// The following are convenience functions for making ellipse figures.  The
// function ellipse_path() creates a ellipse path in the canvas that can
// be used to stroke, fill, clip, etc.
// --------------------------------------------------------------------------

// multipliers for establishing bezier points
static float p0 = 1.00000000;
static float p1 = 0.89657547;   // cos 30 * sqrt(1 + tan 15 * tan 15)
static float p2 = 0.70710678;   // cos 45
static float p3 = 0.51763809;   // cos 60 * sqrt(1 + tan 15 * tan 15)
static float p4 = 0.26794919;   // tan 15

void Canvas::ellipse_path(
    Coord x,                    // x coordinate of ellipse center
    Coord y,                    // y coordinate of ellipse center
    Coord rx,                   // radius in x dimension
    Coord ry)                   // radius in y dimension
{
    float px0 = p0 * rx, py0 = p0 * ry;
    float px1 = p1 * rx, py1 = p1 * ry;
    float px2 = p2 * rx, py2 = p2 * ry;
    float px3 = p3 * rx, py3 = p3 * ry;
    float px4 = p4 * rx, py4 = p4 * ry;

    new_path();
    move_to(x + rx, y);
    curve_to(x + px2, y + py2, x + px0, y + py4, x + px1, y + py3);
    curve_to(x, y + ry, x + px3, y + py1, x + px4, y + py0);
    curve_to(x - px2, y + py2, x - px4, y + py0, x - px3, y + py1);
    curve_to(x - rx, y, x - px1, y + py3, x - px0, y + py4);
    curve_to(x - px2, y - py2, x - px0, y - py4, x - px1, y - py3);
    curve_to(x, y - ry, x - px3, y - py1, x - px4, y - py0);
    curve_to(x + px2, y - py2, x + px4, y - py0, x + px3, y - py1);
    curve_to(x + rx, y, x + px1, y - py3, x + px0, y - py4);
}

// ##############################################################
// ##################  
// ##################  class MWcanvas
// ##################  
// ##############################################################

// -----------------------------------------------------------------------
// constructors/destructors
// -----------------------------------------------------------------------
MWcanvas::MWcanvas() : damageArea(0.0, 0.0, 0.0, 0.0)
{
	win_ = nil;
	// ---- initialize transformation stack -----
	transformers_ = new MWtransformPtrList;
	Transformer* identity = new Transformer;
    transformers_->append(identity);
	transformed_ = false;

	// ---- initialize the clipping stack ----
	clippers_ = new MWclipList;
	clipping_ = CreateRectRgn(0, 0, 0, 0);
	// hines thinks this is bug when one wants to recover resources on exit
	// clippers_->append(clipping_);

	dpy = Session::instance()->default_display();
	MWassert(dpy);
}

MWcanvas::~MWcanvas()
{
	// ---- free up tranformation stack ----
	for (ListItr(MWtransformPtrList) i(*transformers_); i.more(); i.next())
    {
		Transformer* t = i.cur();
		delete t;
    }
	delete transformers_;

	DeleteObject(clipping_);
	for (long j = clippers_->count()-1; j >= 0; --j) {
		DeleteObject(clippers_->item(j));
	}
	delete clippers_;
}

Window* MWcanvas::window() const
{
	return win_->ivWindowOf();
}

PixelCoord MWcanvas::to_pixels(Coord val, DimensionName d) const
{ 
	return dpy->to_pixels(val, d); 
}

Coord MWcanvas::to_coord(PixelCoord val, DimensionName d) const
{
	return dpy->to_coord(val, d);
}
#include <mmsystem.h>
static int s_nTime = -1, s_nTotalTime;
// -----------------------------------------------------------------------
// beginPaint() initializes for the begin of paint sequence... the display
// context is now valid.  Note that this isn't necessarily a result of the
// WM_PAINT message!  endPaint() is the end of paint sequence... the
// display context is going invalid.  There should be no rendering outside
// of these calls.
// -----------------------------------------------------------------------
void dpy_setmapmode(HDC);
void MWcanvas::setmapmode(HDC hdc) {
	dpy_setmapmode(hdc);
}
void MWcanvas::beginPaint(
	HDC hdc,							// device context to use
	const RECT& r)             			// area of update region
{
	drawable_ = hdc;
	// ---- use TWIPS mapping mode for convenient conversion ----
//	MWassert(SetMapMode(hdc, MM_TWIPS));
	setmapmode(hdc);
	POINT pt;
	pt.y = pheight();
	pt.x = pwidth();
	MWassert( DPtoLP(hdc, &pt, 1) );
	MWassert( SetWindowOrgEx(hdc, 0, - pt.y, NULL) );

	// ---- initialize clipping to cover update area ----
	SelectClipRgn(hdc, NULL);
	SetRectRgn(clipping_, r.left,r.top,r.right,r.bottom);
	SelectClipRgn(hdc, clipping_);

	// ---- make sure the update area is contained in the damage rect ----
	MWcoordRect area(
		to_coord(r.left, Dimension_X),
		height() - to_coord(r.bottom, Dimension_Y),
		to_coord(r.right, Dimension_X),
		height() - to_coord(r.top, Dimension_Y));
	damageArea = damageArea || area;
}

void MWcanvas::endPaint()
{
	// ---- clear damage ----
	damageArea = MWcoordRect(0.0, 0.0, 0.0, 0.0);
	SelectClipRgn(drawable_, NULL);

	// ---- flag context as no longer valid ----
	drawable_ = nil;
}

// ----------------------------------------------------------------------
// set the canvas size in terms of points... the pixel size is then
// calculated from the DPI of the display.  This is normally called as
// a result of the geometry negotiations done by the glyph hierarchy.
// ----------------------------------------------------------------------
void MWcanvas::size(
	Coord w,    		// width of the canvas in points
	Coord h)            // height of the canvas in points
{
	psize( to_pixels(w, Dimension_X), to_pixels(h, Dimension_Y));
}

// ----------------------------------------------------------------------
// set the canvas size in terms of pixels... the point size is then
// calculated from the DPI of the display.
// ----------------------------------------------------------------------
void MWcanvas::psize(
	PixelCoord w,		// physical width in pixels
	PixelCoord h) 		// physical height in pixels
{
	pixelSize.x(w);
	pixelSize.y(h);
}

// ----------------------------------------------------------------------
// Width and Height information
// ----------------------------------------------------------------------
Coord MWcanvas::width() const
	{ return to_coord( pwidth(), Dimension_X ); }
Coord MWcanvas::height() const
	{ return to_coord( pheight(), Dimension_Y ); }
PixelCoord MWcanvas::pwidth() const
	{ return pixelSize.x(); }
PixelCoord MWcanvas::pheight() const
	{ return pixelSize.y(); }


// ----------------------------------------------------------------------
// Transformation stack management.  The management of the stack is 
// largely the same between the Win32 and Win16 operation.  The main
// difference is that the Win32 functionality uses the SetWorldTransform
// function available in NT which transforms all the other GDI calls, and
// the Win16 functionality tries to transform the data before handing it
// to the MS-Windows API.
// ----------------------------------------------------------------------
void MWcanvas::push_transform()
{
	MWtransformPtrList& s = * transformers_;
	int index = s.count() - 1;
	Transformer* tmp = s.item(index);
	Transformer* m = new Transformer(*tmp);
	s.append(m);
}

void MWcanvas::pop_transform()
{
	MWtransformPtrList& s = * transformers_;
	int i = s.count() - 1;
	if (i == 0)
	{
		// We pushed the first matrix during initialization,
		// so we must be underflowing the stack.  Should be an exception.
		return;
	}

	Transformer* m = s.item(i);
	delete m;
	s.remove(i);
	transformed_ = ! matrix().identity();
}

void MWcanvas::transform(const Transformer& t)
{
    matrix().premultiply(t);
    transformed_ = ! matrix().identity();
}

void MWcanvas::transformer(const Transformer& t)
{
    matrix() = t;
    transformed_ = ! t.identity();
}

const Transformer& MWcanvas::transformer() const
{
    return matrix();
}

Transformer& MWcanvas::matrix() const
{
	MWtransformPtrList& s = *transformers_;
    return *(s.item(s.count() - 1));
}

// ----------------------------------------------------------------------
// Clipping stack management
// ----------------------------------------------------------------------
void MWcanvas::push_clipping(bool)
{
	HRGN old_clip = clipping_;
	HRGN new_clip = CreateRectRgn(0, 0, pwidth(), pheight());
	CombineRgn(new_clip, old_clip, old_clip, RGN_COPY);
    clippers_->append(old_clip);
	clipping_ = new_clip;
	SelectClipRgn(drawable_, new_clip);
}

void MWcanvas::pop_clipping()
{
	MWclipList& s = * clippers_;
	int n = s.count();
	if (n == 0)
	{
		// stack underflow--should raise exception
		return;
    }

	HRGN clip = clipping_;
	DeleteObject(clip);

    clip = s.item(n - 1);
    s.remove(n - 1);
	clipping_ = clip;

	SelectClipRgn(drawable_, clip);
}

// ----------------------------------------------------------------------
// Double buffer management
// ----------------------------------------------------------------------
void MWcanvas::front_buffer()
{
	if (win_)
		win_->frontBuffer();
}

void MWcanvas::back_buffer()
{
	if (win_)
		win_->backBuffer();
}

// -----------------------------------------------------------------------
// Canvas damaging interface.  MS-Windows has a window damage API, so it
// is used to manage the damage.
//
// Damage coordinates are always absolute with respect to the canvas,
// so they are not transformed.  The canvas coordinate system has the
// origin in the lower left, but the MS-Windows coordinate system has
// the origin at the upper left.
// -----------------------------------------------------------------------

void MWcanvas::damage(const Extension& ext)
{
    damage(ext.left(), ext.bottom(), ext.right(), ext.top());
}

void MWcanvas::damage(Coord left, Coord bottom, Coord right, Coord top)
{
	MWcoordRect area(left, bottom, right, top);

	if (damageArea.origin() != damageArea.corner())
		damageArea = damageArea || area;
	else
		damageArea = area;

	// ---- if we are bound to a window.. invalidate it's area ----
	if (win_)
	{
		// ---- convert to device coordinates ----
		RECT wr;
		wr.left = to_pixels(area.left(), Dimension_X);
		wr.right = to_pixels(area.right(), Dimension_X);
		wr.top = pixelSize.y() - to_pixels(area.top(), Dimension_Y);
		wr.bottom = pixelSize.y() - to_pixels(area.bottom(), Dimension_Y);

		// ---- invalidate ----
		InvalidateRect(win_->msWindow(), &wr, TRUE);

#ifdef DEBUG
		printf("damage l:%d r:%d t:%d b:%d\n", wr.left, wr.right, wr.top,
			wr.bottom);
#endif
	}
}

bool MWcanvas::damaged(const Extension& ext) const
{
    return damaged(ext.left(), ext.bottom(), ext.right(), ext.top());
}

bool MWcanvas::damaged(
	Coord left,
	Coord bottom,
	Coord right,
	Coord top) const
{
	MWcoordRect area(left, bottom, right, top);
	return damageArea.intersects(area);
}

void MWcanvas::damage_area(Extension& ext) 
{
	 ext.set_xy(nil, damageArea.left(), damageArea.bottom(),
		damageArea.right(), damageArea.top());
}
void MWcanvas::damage_all()
{
	damageArea.origin(MWcoordPoint(0,0)); 
	damageArea.corner(MWcoordPoint(width(), height()));
	if (win_)
	{
		InvalidateRect(win_->msWindow(), NULL, TRUE);
	}
}

bool MWcanvas::any_damage() const
{
	return (damageArea.origin() != damageArea.corner());
}

void MWcanvas::restrict_damage(const Extension& ext)
{
    restrict_damage(ext.left(), ext.bottom(), ext.right(), ext.top());
}

void MWcanvas::restrict_damage(
	Coord left,
	Coord bottom,
	Coord right,
	Coord top)
{
	MWcoordRect area(left, bottom, right, top);
	damageArea = area;

	if (win_)
	{
		// ---- convert the area to device coordinates ----
		RECT wr;
		wr.left = to_pixels(area.left(), Dimension_X);
		wr.right = to_pixels(area.right(), Dimension_X);
		wr.top = pixelSize.y() - to_pixels(area.top(), Dimension_Y);
		wr.bottom = pixelSize.y() - to_pixels(area.bottom(), Dimension_Y);

		ValidateRect(win_->msWindow(), 0);
		InvalidateRect(win_->msWindow(), &wr, TRUE);
	}
}

//
// Force a portion of the canvas to be redrawn.  This is typically caused
// by an X expose event.  If there is no damage and we have a buffer
// with a copy of the canvas, then we can just copy it to the draw buffer.
// Otherwise, we just damage the area.
//
void MWcanvas::redraw(Coord left, Coord bottom, Coord right, Coord top)
{
	// At the moment, there is no memory HGC holding a copy, so we
	// simply damage the canvas.
	damage(left, bottom, right, top);
}

void MWcanvas::repair()
{
	HWND hwnd = win_->msWindow();
	ValidateRect(hwnd, NULL);
}

// --------------------------------------------------------------------------
// This function fills the current path with a stenciled pattern
// in the given color.  This function is basically used to simulate
// the alpha blending of color which is not directly supported by
// the GDI interface (ie PatBlt doesn't allow raster operations that
// specify source as part of the operation... so no stencil).  Since
// the regions stenciled are typically not that large, this shouldn't
// be too big of a shortcoming... it's too bad though because some
// smart framebuffers can do this in hardware wicked-fast :-)
// --------------------------------------------------------------------------
void MWcanvas::stencilFill(const Bitmap* b, const Color* c)
{
	// ---- clip to the current path (ie area to be filled) ----
	push_clipping();
	clip();

	// ---- determine the clipping bounds (in Coord) ----
	RECT r;
	MWassert( GetClipBox(drawable_, &r) != ERROR );
	Coord left(float(r.left) / float(20));
	Coord right(float(r.right) / float(20));
	Coord top(float(r.top) / float(20));
	Coord bottom(float(r.bottom) / float(20));

	// ---- lay down some stencil ----
	for (Coord x = left; x < right; x += b->width())
	{
		for (Coord y = bottom; y < top; y += b->height())
		{
			stencil(b, c, x, y);
		}
	}

	// ---- restore ----
	pop_clipping();
}
