#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =========================================================================
//
//				<IV-Mac/canvas.c>
//
// Macintosh dependent Canvas representation.  This canvas type renders
// into an Macintosh window.  
//
// 1.6
// $Date:   4 Aug 1996
//
// =========================================================================

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
#include <IV-Mac/bitmap.h>
#include <IV-Mac/brush.h>
#include <IV-Mac/color.h>
#include <IV-Mac/canvas.h>
#include <IV-Mac/font.h>
#include <IV-Mac/window.h>
#include <IV-Mac/raster.h>


#include <OS/math.h>
#include <OS/list.h>

// ---- libc includes ----
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#ifdef DEBUG
#include <stdio.h>
//#endif

extern "C" {extern void debugfile(const char*, ...);}

// ---- templates used ----
declarePtrList(MACtransformPtrList,Transformer)
implementPtrList(MACtransformPtrList,Transformer)
declarePtrList(MACclipList,Rect)
implementPtrList(MACclipList,Rect)

const int TEXT_BUFFER_LEN = 80;			// max buffered text length
PathRenderInfo MACcanvas::path_;

// ----------------------------------------------------------------------
// utility functions
// ----------------------------------------------------------------------

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

// ########################################################
// ############  class Canvas
// ########################################################

Canvas::Canvas()
{
}

Canvas::~Canvas()
{
}

void Canvas::front_buffer(){
}
void Canvas::back_buffer(){
}
//handled by MACcanvas -- I think that this is just necessary for linking
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
// ##################  class MACcanvas
// ##################  
// ##############################################################

// -----------------------------------------------------------------------
// constructors/destructors
// -----------------------------------------------------------------------
MACcanvas::MACcanvas()
{
	// ---- initialize transformation stack -----
	transformers_ = new MACtransformPtrList;
	Transformer* identity = new Transformer;
    transformers_->append(identity);
	transformed_ = false;

	// ---- initialize the clipping stack ----
	clippers_ = new MACclipList;
	clipping_ = new Rect;
	SetRect(clipping_,0, 0, 0, 0);
	
	// ---- initialize the path information ----
	PathRenderInfo* p = &path_;
	if (p->point_ == nil)
	{
		p->point_ = new Point[10];
        p->cur_point_ = p->point_;
		p->end_point_ = p->point_ + 10;
	}
	
	// ---- initially we have no logical attributes ----
	lg_color_ = nil;
	lg_brush_ = nil;
	lg_font_ = nil;

	// ---- initialize the text handling ----
	 text_item_.count = 0;
	
	//-- set damage area to 0
	SetRect(&damageArea, 0, 0, 0, 0);
	
	dpy = Session::instance()->default_display();
}

MACcanvas::~MACcanvas()
{
	// ---- free up tranformation stack ----
	for (ListItr(MACtransformPtrList) i(*transformers_); i.more(); i.next())
    {
		Transformer* t = i.cur();
		delete t;
    }
	delete transformers_;

	delete clipping_;
	for (long j = clippers_->count()-1; j >= 0; --j) {
		delete (clippers_->item(j));
	}
	delete clippers_;
	
}

Window* MACcanvas::window() const
{
	return win_->ivWindowOf();
}

PixelCoord MACcanvas::to_pixels(Coord val, DimensionName d) const
{ 
	return dpy->to_pixels(val, d); 
}

Coord MACcanvas::to_coord(PixelCoord val, DimensionName d) const
{
	return dpy->to_coord(val, d);
}

static int s_nTime = -1, s_nTotalTime;

// -----------------------------------------------------------------------
// beginPaint() is used before MACPaint calls the Glyph::draw in order to
// initialize clipping to the damage area. This was added 1/3/99 to allow
// fast drawing of certain glyphs without having the Background erase all
// existing info in the pixrect
// -----------------------------------------------------------------------
void MACcanvas::beginPaint()
{
//	--damageArea.left; ++damageArea.right; --damageArea.top; ++damageArea.bottom;
//debugfile("beginPaint %lx damage l=%d t=%d r=%d b=%d\n",(long)this, damageArea.left, damageArea.top, damageArea.right, damageArea.bottom);
//	SetRect(clipping_,0, 0, pwidth(), pheight());
	*clipping_ = damageArea;
	ClipRect(clipping_);	
}

// -----------------------------------------------------------------------
// endPaint() is used at the end of MACPaint to ensure that all characters
// have been printed.
// -----------------------------------------------------------------------
void MACcanvas::endPaint()
{
	flush();
}

// ----------------------------------------------------------------------
// set the canvas size in terms of points... This is normally called as
// a result of the geometry negotiations done by the glyph hierarchy.
// ----------------------------------------------------------------------
void MACcanvas::size(
	Coord w,    		// width of the canvas in points
	Coord h)            // height of the canvas in points
{
	psize( to_pixels(w, Dimension_X), to_pixels(h, Dimension_Y));
}

// ----------------------------------------------------------------------
// This function currently does nothing because the canvas stores its size
// in the portRect field of the Mac window.  This limits the canvas to 
// being the window content regions size, but it also means that we let
// the system keep track of the size.
// ----------------------------------------------------------------------
// 2/17/96  there have been considerable difficulties and confusion resulting
// from calling height when there is no macWindow. This happens due to wanting
// to know width and height in order to do proper placement before mapping
// since there is incessant conversion from a mac coord system where the origin of screen
// and window is top/left and the interviews coord system where the origin of screen
// and window is bottom/left. We will leave out for a moment the question of window
// decorations and exterior vs interior vs canvas.  Anyway, glyph request is called
// on Window::map -> compute_geometry which is how we get here (and we were
// throwing away the info) then the map calls windowrep::bind which
// binds the canvas and calls glyph::request again and sets the
// params_->bounds_->(bottom,right) which is immediately used in macwindow::bind
// to create theMacWindow. After that, even with the old implementation
// we can do whatever we want since all pointers are now set up.
// Now here are the issues/problems so far:
//  Window::align which calls height is often called before mapping.
//  Just before theMacWindow creation in Macwindow::bind,
// next_window_y needs the height if
// the window had been placed before mapping.
// So. What is proposed is to call Glyph::request when Window is constructed
// after the rep is constructed and fill the params_->bounds_ fields.
// when there is no macWindow, this info is used for width and height.
// also, probably redundantly but who knows, psize calls can store it as well.
// my guess is that it will not be used since the window is soon mapped anyway.
// Request on window creation was too soon since the size requested was often 0.So align
// remains a problem til after binding. However, at least the next_window_y seems to work now.
void MACcanvas::psize(
	PixelCoord w,		// physical width in pixels
	PixelCoord h) 		// physical height in pixels
{
//	canvas_width = w;
//	canvas_height = h;
	if (!win_->macWindow()) {
		win_->params_->bounds_->bottom = h;
		win_->params_->bounds_->right = w;
	}
}

// ----------------------------------------------------------------------
// Width and Height information
// ----------------------------------------------------------------------
Coord MACcanvas::width() const
{
  if (win_->macWindow()) {
#if carbon
	Rect contentRect;
	GetPortBounds(GetWindowPort(win_->macWindow()), &contentRect);
#else
	Rect contentRect = (win_->macWindow())->portRect;
#endif
	return to_coord((contentRect.right - contentRect.left), Dimension_X );
  }else{
  	return to_coord(win_->params_->bounds_->right, Dimension_X); // assuming left is 0
  }
}	
Coord MACcanvas::height() const
{ 
  if (win_->macWindow()) {
#if carbon
	Rect contentRect;
	GetPortBounds(GetWindowPort(win_->macWindow()), &contentRect);
#else
	Rect contentRect = (win_->macWindow())->portRect;
#endif
	return to_coord((contentRect.bottom - contentRect.top), Dimension_Y);
  }else{
  	return to_coord(win_->params_->bounds_->bottom, Dimension_Y); // assuming top is 0
  }
}
PixelCoord MACcanvas::pwidth() const
{
  if (win_->macWindow()) {
#if carbon
	Rect contentRect;
	GetPortBounds(GetWindowPort(win_->macWindow()), &contentRect);
#else
	Rect contentRect = (win_->macWindow())->portRect;
#endif
	return (contentRect.right - contentRect.left);
  }else{
  	return win_->params_->bounds_->right; // assuming left is 0
  }
}
PixelCoord MACcanvas::pheight() const
{
  if (win_->macWindow()) {
#if carbon
	Rect contentRect;
	GetPortBounds(GetWindowPort(win_->macWindow()), &contentRect);
#else
	Rect contentRect = (win_->macWindow())->portRect;
#endif
	return (contentRect.bottom - contentRect.top);
  }else{
  	return win_->params_->bounds_->bottom; // assuming top is 0
  }
}


// ----------------------------------------------------------------------
// Transformation stack management.  This is largely the same as the 
// Windows implementation. 
// ----------------------------------------------------------------------
void MACcanvas::push_transform()
{
	MACtransformPtrList& s = * transformers_;
	int index = s.count() - 1;
	Transformer* tmp = s.item(index);
	Transformer* m = new Transformer(*tmp);
	s.append(m);
}

void MACcanvas::pop_transform()
{
	MACtransformPtrList& s = * transformers_;
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

void MACcanvas::transform(const Transformer& t)
{
    matrix().premultiply(t);
    transformed_ = ! matrix().identity();
}

void MACcanvas::transformer(const Transformer& t)
{
 	matrix() = t;
    transformed_ = ! t.identity();   
}

const Transformer& MACcanvas::transformer() const
{
    return matrix();
}

Transformer& MACcanvas::matrix() const
{
	MACtransformPtrList& s = *transformers_;
    return *(s.item(s.count() - 1));
}

// ----------------------------------------------------------------------
// Clipping stack management -- again largely the same as the Windows 
// implementation
// ----------------------------------------------------------------------
void MACcanvas::initClip() {
	SetRect(clipping_, 0, 0, pwidth(), pheight());
	ClipRect(clipping_);
}

void MACcanvas::push_clipping(bool all)
{
	Rect* old_clip = clipping_;
	Rect* new_clip = new Rect;
	if (all) {
		SetRect(new_clip,0, 0, pwidth(), pheight());
	}else{
		SetRect(new_clip,old_clip->left, old_clip->top, old_clip->right, old_clip->bottom);
	}
	//CombineRgn(new_clip, old_clip, old_clip, RGN_COPY);
    clippers_->append(old_clip);
	clipping_ = new_clip;
//debugfile("pushclip %lx, %d %d %d %d\n", (long)this, new_clip->left, new_clip->right, new_clip->top, new_clip->bottom);
	ClipRect(new_clip);
}

void MACcanvas::pop_clipping()
{
	MACclipList& s = * clippers_;
	int n = s.count();
	if (n == 0)
	{
		// stack underflow--should raise exception
		return;
    }

	Rect* clip = clipping_;
	delete clip;

    clip = s.item(n - 1);
    s.remove(n - 1);
	clipping_ = clip;
	if (clip->right <= clip->left || clip->bottom <= clip->top) {
		SetRect(clip, 0,0,pwidth(),pheight());
	}
//debugfile("popclip %lx, %d %d %d %d\n", (long)this, clip->left, clip->right, clip->top, clip->bottom);
		ClipRect(clip);
	
}

void MACcanvas::clip()
{
	flush();
//if (printing) return;
#if 0
// fresco
    RgnHandle region = select_path();
    RgnHandle clip = NewRgn();
    GetClip(clip);
    SectRgn(clip, region, region);
    DisposeRgn(clip);
    SetClip(region);
    DisposeRgn(region);
#endif
    // ---- make sure there are multiple points ----
    PathRenderInfo* p = &path_;
	Point* pt = p->point_;
	int n = (int) (p->cur_point_ - p->point_);
	if (n <= 2)
	{
		return;
	}
//	if (xrect(pt, n))
		// ---- rectangular clipping area ----
		Rect intersect;
		Rect& xr = intersect;
		//Rect& xr = *clipping_;
		xr.left = Math::min(pt[0].h, pt[2].h);
		xr.top = Math::min(pt[0].v, pt[2].v);
		xr.right = Math::max(pt[0].h, pt[2].h);
		xr.bottom = Math::max(pt[0].v, pt[2].v);

	// ---- merge with existing clipping area ----
#if 0
	if (xr.right > xr.left && xr.bottom > xr.top) {
//debugfile("clip %lx, %d %d %d %d\n", (long)this, xr.left, xr.right, xr.top, xr.bottom);
		ClipRect(&xr);
	}
#else
	//Rect intersect;
	SectRect(&xr, clipping_, &intersect);
	// ---- set new clipping area ----
	if (intersect.right > intersect.left && intersect.bottom > intersect.top) {
//debugfile("clip %lx, %d %d %d %d\n", (long)this, intersect.left, intersect.right, intersect.top, intersect.bottom);
		ClipRect(&intersect);
	}
#endif
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
// ----------------------------------------------------------------------
// Double buffer management
// ----------------------------------------------------------------------
void MACcanvas::front_buffer()
{
	//if (win_)
		//win_->frontBuffer();
}

void MACcanvas::back_buffer()
{
	//if (win_)
		//win_->backBuffer();
}

// -----------------------------------------------------------------------
// Canvas damaging interface.  The Macintosh has a window damaging 
// support, but Fresco avoided its use.  Instead, each MACwindow has
// a boolen flag to update.  Whenever a window is damaged, setWinToUpdate
// is called.  If the window is already set to be updated, the new damaged 
// region is merely added to the old damaged region.  Otherwise, the damage
// region is set to the new damage, and the window is put on the update list.
// -----------------------------------------------------------------------
void MACcanvas::setWinToUpdate(void){
	if(!(win_->update_)){
		win_->update_ = true;
		WindowRep::update_list.append(win_);
	}
}

void MACcanvas::damage(const Extension& ext)
{
    damage(ext.left(), ext.bottom(), ext.right(), ext.top());
}

void MACcanvas::damage(Coord left, Coord bottom, Coord right, Coord top)
{
	GrafPtr oldPort;
	
	Rect area;
	SetRect(&area, toPixelX(left), toPixelY(top), toPixelX(right), toPixelY(bottom));
	//debugfile("damage %lx old  l:%d r:%d t:%d b:%d\n", (long)this, damageArea.left, damageArea.right, damageArea.top, damageArea.bottom);
	//debugfile("damage this  l:%d r:%d t:%d b:%d\n", area.left, area.right, area.top, area.bottom);
	if(EmptyRect(&damageArea)){
		damageArea = area;
	} else {
		UnionRect(&damageArea, &area, &damageArea);
	}
	//debugfile("damage new  l:%d r:%d t:%d b:%d\n", damageArea.left, damageArea.right, damageArea.top, damageArea.bottom);

	// ---- if we are bound to a window.. invalidate it's area ----
	if (win_)
	{
		setWinToUpdate();

#if defined(DEBUG)
		printf("damage total l:%d r:%d t:%d b:%d\n", damageArea.left, damageArea.right,
		 damageArea.top, damageArea.bottom);
#endif
	}
	
}

bool MACcanvas::damaged(const Extension& ext) const
{
    return damaged(ext.left(), ext.bottom(), ext.right(), ext.top());
}

bool MACcanvas::damaged(
	Coord left,
	Coord bottom,
	Coord right,
	Coord top) const
{
	Rect area;
	SetRect(&area, toPixelX(left), toPixelY(top), toPixelX(right), toPixelY(bottom));
	// problem is that intersection of boundary doesn't work
	//return SectRect(&damageArea, &area, &area);
	const Rect& d = damageArea;
	return (
	area.left < d.right && area.right > d.left &&
	area.bottom > d.top && area.top < d.bottom
    );
}

void MACcanvas::damage_area(Extension& ext) 
{
	ext.set_xy(nil, fromPixelX(damageArea.left), fromPixelY(damageArea.bottom),
		fromPixelX(damageArea.right), fromPixelY(damageArea.top)); 
}
void MACcanvas::damage_all()
{
	GrafPtr oldPort;

	SetRect(&damageArea, 0, 0, pwidth(), pheight()); 
	if (window())
	{
		setWinToUpdate();
	}
}

bool MACcanvas::any_damage() const
{
	return (!(EmptyRect(&damageArea)));
}

void MACcanvas::restrict_damage(const Extension& ext)
{
   restrict_damage(ext.left(), ext.bottom(), ext.right(), ext.top());
}

void MACcanvas::restrict_damage(
	Coord left,
	Coord bottom,
	Coord right,
	Coord top)
{
	GrafPtr oldPort;
	Rect area;
	SetRect(&area, toPixelX(left), toPixelY(top), toPixelX(right), toPixelY(bottom));
	damageArea = area;

	if (window())
	{
		setWinToUpdate();
	}
}

//
// Force a portion of the canvas to be redrawn.  This is typically caused
// by an X expose event.  We just damage the canvas.
//
void MACcanvas::redraw(Coord left, Coord bottom, Coord right, Coord top)
{
	// At the moment, there is no memory HGC holding a copy, so we
	// simply damage the canvas.
	damage(left, bottom, right, top);
}

void MACcanvas::repair()
{
	GrafPtr oldPort;
	
	printf("MACcanvas::repair Port code disabled\n");
	//GetPort(&oldPort);
	//SetPort(win_->macWindow());	
	//ValidRect(&win_->macWindow()->portRect);
	//SetPort(oldPort);
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
//
// NOT YET IMPLEMENTED FOR THE MACHINTOSH
// --------------------------------------------------------------------------
void MACcanvas::stencilFill(const Bitmap* b, const Color* c)
{

}

void MACcanvas::new_path()
{
   PathRenderInfo* p = &path_;
    p->curx_ = 0;
    p->cury_ = 0;
	Point* xp = p->point_;
    xp->h = 0;
    xp->v = 0;
    p->cur_point_ = xp; 
}

void MACcanvas::move_to(Coord x, Coord y)
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
	
	// -- convert to MAC pixels --
	Point* xp = p->point_;
	xp->h = toPixelX(tx);
	xp->v = toPixelY(ty);
//if (printing) debugfile("move_to %g %g -> %d %d\n", x, y, xp->h, xp->v);
	p->cur_point_ = xp + 1;
}

void MACcanvas::line_to(Coord x, Coord y)
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

	if (p->cur_point_ == p->end_point_)
	{
		int old_size = (int) (p->cur_point_ - p->point_);
		int new_size = 2 * old_size;
        Point* new_path = new Point[new_size];
		for (int i = 0; i < old_size; i++)
		{
            new_path[i] = p->point_[i];
        }
        delete p->point_;
        p->point_ = new_path;
		p->cur_point_ = p->point_ + old_size;
		p->end_point_ = p->point_ + new_size;
	}

	// -- convert to MAC Pixels --
	Point * xp = p->cur_point_;
	xp->h = toPixelX(tx);
	xp->v = toPixelY(ty);
//if (printing) debugfile("line_to %g %g -> %d %d\n", x, y, xp->h, xp->v);
	p->cur_point_ = xp + 1;
}

void MACcanvas::curve_to(
	Coord x,
    Coord y,
	Coord x1,
	Coord y1,
	Coord x2,
	Coord y2)
{
	PathRenderInfo* p = &MACcanvas::path_;
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

void MACcanvas::close_path()
{
	PathRenderInfo* p = &MACcanvas::path_;
	Point* startp = p->point_;
	Point* xp = p->cur_point_;
    xp->h = startp->h;
    xp->v = startp->v;
    p->cur_point_ = xp + 1;
}

void MACcanvas::stroke(const Color* c, const Brush* b)
{
	PathRenderInfo* p = &MACcanvas::path_;
	
	// --- determine the number of points, and if valid ---
	int n = (int) (p->cur_point_ - p->point_);
	if (n < 2)
	{
		return;
    }
	flush();
    color(c);
	brush(b);

	// ---- render it ----
	Point* pt = p->point_;
	MoveTo(pt[0].h, pt[0].v);
	for (int i = 1; i < n; i++){
		LineTo(pt[i].h, pt[i].v);
	}
}

void MACcanvas::fill(const Color* c)
{
//if (printing) return;
	
	if (c->rep()->stipple)
	{
		// ---- stipple some color into the area ----
		stencilFill(c->rep()->stipple, c);
	}
	else
	{
		PathRenderInfo* p = &MACcanvas::path_;
		int n = (int) (p->cur_point_ - p->point_);
		if (n <= 2)
		{
			return;
		}

		flush();
		color(c);

		//Setup Region and render an outline to fill
//		RgnHandle region = NewRgn();
//		OpenRgn();
		PolyHandle poly;
		poly = OpenPoly();
		Point* pt = p->point_;
		MoveTo(pt[0].h, pt[0].v);
		for (int i = 1; i < n; i++){
			LineTo(pt[i].h, pt[i].v);
		}
//		CloseRgn(region);		
//		PaintRgn(region);
//    	DisposeRgn(region);
		ClosePoly();
		PaintPoly(poly);
		KillPoly(poly);
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
void MACcanvas::color(const Color* c)
{
	
	if (c != nil && c != lg_color_){
		// ---- render anything that was buffered ----
		flush();

		// ---- reference new color ----
		Resource::ref(c);
		Resource::unref(lg_color_);
		lg_color_ = c;

		// ---- set new pen color ----
		RGBForeColor(c->rep()->MACcolor());

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
			//brush_stats_.lbStyle = BS_SOLID;
			//brush_stats_.lbColor = c->rep()->msColor();
		}
		//brush_ = CreateBrushIndirect(&brush_stats_);
		//DeleteObject( SelectObject(drawable_, brush_));
	
		// ---- set color operation ----
		ColorRep* r = c->rep();
		if (r->op == Color::Copy)
		{
			PenMode(srcCopy);
		}
		else if (r->op == Color::Xor)
		{
			PenMode(srcXor);
		}
		else if (r->op == Color::Invisible)
		{
			//SetROP2(drawable_, R2_NOP);
		}
		else
		{
			// ----- unrecognized color mode ----
			printf("unrecognized color mode\n");
		}
	}
}

// -----------------------------------------------------------------------
// Changes the brush to use when rendering a path -- size implemented, no dashes
// -----------------------------------------------------------------------
void MACcanvas::brush(const Brush* b)
{
	if (b != nil && b != lg_brush_)
	{
    	// ---- reference the new brush ----
		Resource::ref((const Resource*) b);
		Resource::unref((const Resource*) lg_brush_);
		lg_brush_ = b;
		BrushRep* r = b->rep(nil);
		short w = (r->width_ > 1.) ? short(r->width_) : 1;
		PenSize(w, w);
		
        // ---- get ready to create a new pen ----
		//BrushRep& br = * b->rep(nil);
		//pen_stats_.lopnStyle = (br.dashCount) ? PS_DASH : PS_SOLID;

		//pen_stats_.lopnWidth.x = COORD2TWIPS(br.penWidth);

		// ---- establish the new pen ----
		//pen_ = CreatePenIndirect(&pen_stats_);
		//DeleteObject( SelectObject(drawable_, pen_));

    }
}

// -----------------------------------------------------------------------
// Changes the font to use when rendering.  The actual creation and
// selection of the GDI object is delayed until the actual text is to be
// rendered, since further scaling and rotation might be desired.
// -----------------------------------------------------------------------
void MACcanvas::font(const Font* f)
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

void MACcanvas::begin_item(PixelCoord x, PixelCoord y, PixelCoord w) {
	text_item_.x = x;// toPixelX(x); //now done in flush
	text_item_.y = y;// toPixelY(y);
	text_item_.count = 0;
}

// -----------------------------------------------------------------------
// Render a character onto the canvas.  This is a buffered operation that
// tries to draw entire words at once.  This mode is to provide compati-
// bility with the original InterViews.  
// -----------------------------------------------------------------------
void MACcanvas::character(
	const Font* f, 					// font to use
	long ch,                        // character to print
	Coord width,                    // width of the character
	const Color* c,             	// color to use
	Coord x,                        // x-coordinate
	Coord y)                        // y-coordinate
{
	font(f);
	color(c);
	// Transform the coordinates before testing for expected position
	// so that we can work in terms of pixels and deal with slop in
	// position better.
	// 3/9/97 M Hines. Well, not really. text_item_.y was in mac pixels and ty was in
	// interviews coords. If ever to_pixels is not 1 there will have to be a lot of changes.
	Transformer& m = matrix();
	Coord tx = x;
	Coord ty = y;
	if (transformed_)
	{
		m.transform(tx, ty);

		// ROTATION SUPPORT TO BE ADDED
	}
	// the collection into a buffer on the mac is a significant efficiency improvement
	// however it is not clear to me if the next_x should be in pixel coords or interviews
	// coords. InterViews coords did not work well after resizing a Graph since it
	// started flushing every character. If we don't check x, then we don't flush enough
	// expecially during text rubberbanding.
	static Coord next_x;
	PixelCoord ptx = toPixelX(tx);
	PixelCoord pty = toPixelY(ty);
	PixelCoord pwidth = toPixelX(width);
	
	if (text_item_.count == 0) {
		begin_item(ptx, pty, pwidth);
	} else if (
	    (text_item_.count >= text_item_.size) ||
	    (pty != text_item_.y)
	    //|| (!Math::equal(x, next_x, .01))){ 
	    || (!Math::equal(tx, next_x, float(1.1)))) {//allow 1 pixel slop
		flush();
		begin_item(ptx, pty, pwidth);
	} 
	next_x = tx + width;    
	text_item_.buffer[text_item_.count++] = (char) ch;
//if(printing) debugfile("character %c %g %g\n", ch, x, y);
}

// -----------------------------------------------------------------------
// Flush the text buffer.  The text drawing is buffered until something
// changes and causes a flush (by calling this function).  
// -----------------------------------------------------------------------
/* need to come to terms with fact that CreateFontIndirect slows things down
to an extent which makes Win32s unusable. This hack allows only one font.
*/


//static HFONT new_fnt;
//void cleanup_new_fnt() {
	//if (new_fnt) {
	//	DeleteObject(new_fnt);
	//}
//}

void MACcanvas::flush()
{
	// ---- check if there is anything to do ----
	//int nchars = (int) (text_ptr_ - text_buff_);
	//if ((nchars == 0) || (lg_font_ == nil))
	//	return;
		
	// ---- render the text ----
	if (text_item_.count > 0) {
		//set font
		FontRep* fr = lg_font_->rep(nil);
		TextFont(fr->font_);
		TextFace(fr->face_);
		TextSize(fr->size_);
		TextMode(fr->mode_);
//if (printing) debugfile("flush %d %d\n",toPixelX(text_item_.x), toPixelY(text_item_.y)); 		
        MoveTo(text_item_.x, text_item_.y);
        DrawText(text_item_.buffer, 0, text_item_.count);
// text_item_.buffer[text_item_.count] = '\0';
// debugfile("|%s|\n", text_item_.buffer);
    }
    text_item_.count = 0;	
	
	return;
	
#if 0		//MS Windows code for reference

	// ---- render the text ----
    SetBkMode(drawable_, TRANSPARENT);
	FontRep* fr = lg_font_->rep(nil);
	MWassert(fr);
	fr->orientation(transformAngle());
//	HFONT new_fnt = fr->Create();
	if (!new_fnt) {
		new_fnt = fr->Create();
	}
	HFONT old_fnt = (HFONT) SelectObject(drawable_, new_fnt);
	SetTextAlign(drawable_, TA_LEFT | TA_BASELINE | TA_NOUPDATECP);
	TextOut(drawable_, text_x0_, text_y0_, text_buff_, nchars);
//	DeleteObject( SelectObject(drawable_, old_fnt));
	// ---- reset the buffer ----
	text_ptr_ = text_buff_;
#endif
}
// -----------------------------------------------------------------------
// Determine the angle of transformation in terms of tenths-of-a-degree.
// The method used is to transform a couple of points, and then measure
// the angle of rotation on the sample points.  There is probably a better
// way to do this, but it works.
// NOT YET USED ON THE MACINTOSH
// -----------------------------------------------------------------------
int MACcanvas::transformAngle() const
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
// NOT YET IMPLEMENTED ON THE MACINTOSH
// -----------------------------------------------------------------------
void MACcanvas::stencil(
    const Bitmap* mask, 			// bitmap to render
	const Color* c, 				// color to use
	Coord x, 						// x-coordinate
	Coord y) 						// y-coordinate
{
	//just fill a rectangle for now.
	fill_rect(x + mask->left_bearing(), y + mask->descent(), x + mask->right_bearing(),
			y + mask->ascent(), c);
	
}

// -----------------------------------------------------------------------
// Render an image onto the GDI surface.  The image is transformed in 
// terms of scaling only initially (all that Windows 3.1 supports).  
// Rotation is initially not supported.
// NOT YET SUPPORTED ON THE MACINTOSH
// -----------------------------------------------------------------------
void MACcanvas::image(const Raster* ras, Coord x, Coord y)
{
	if (!ras || !ras->rep()->cg_) return;
	Coord tx, ty, tw, th;
	if (transformed_) {
		matrix().transform(x, y, tx, ty);
		matrix().transform(x+ras->width(), y+ras->height(), tw, th);
		tw -= tx; th -= ty;
	} else {
		tx = x;
		ty = y;
		tw = ras->width();
		th = ras->height();
	}
	Rect sr, dr;
	sr.left = 0; sr.top = 0; sr.right = ras->pwidth(); sr.bottom = ras->pheight();
	dr.left = toPixelX(tx); dr.right = toPixelX(tx + tw);
	dr.top = toPixelY(ty+th); dr.bottom = toPixelY(ty);

	// ---- blt... the destination will be transformed ----
	CGrafPtr cg;
	GDHandle gd;
	GetGWorld(&cg, &gd);
	CopyBits(
#if carbon
		GetPortBitMapForCopyBits(ras->rep()->cg_), GetPortBitMapForCopyBits(cg),
#else
		&GrafPtr(ras->rep()->cg_)->portBits, &GrafPtr(cg)->portBits,
#endif
		&sr, &dr, srcCopy, nil
	);
	
}
