/*
 * Canvas - an area for drawing
 */

/*
 * Copyright (c) 1991 Stanford University
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
// ============================================================================
//
// 1.5
// 1999/01/19 14:25:06
//
// Changes for InterViews Port to the Windows 3.1/NT operating systems
// Copyright (c) 1993 Tim Prinzing
//
// Permission to use, copy, modify, distribute, and sell this software and 
// its documentation for any purpose is hereby granted without fee, provided
// that (i) the above copyright notice and this permission notice appear in
// all copies of the software and related documentation, and (ii) the name of
// Tim Prinzing may not be used in any advertising or publicity relating to 
// the software without the specific, prior written permission of Tim Prinzing.
//
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
//
// IN NO EVENT SHALL Tim Prinzing BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
// INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER 
// RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE 
// POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR 
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// ============================================================================
#ifndef iv_canvas_h
#define iv_canvas_h

#include <InterViews/geometry.h>
#include <InterViews/_enter.h>

class Bitmap;
class Brush;
class CanvasRep;
class Color;
class Extension;
class Font;
class Raster;
class Transformer;
class Window;

#if !defined(WIN32) && !MAC
/* anachronism */
typedef unsigned int CanvasLocation;

class Canvas 
{
public:
    Canvas();
    virtual ~Canvas();

    virtual Window* window() const;
		// Returns the window associated with this canvas, if this canvas
		// is bound to a window.  A null pointer will be returned if this
		// canvas is not bound to a window.

    virtual void size(Coord width, Coord height);
    virtual void psize(PixelCoord width, PixelCoord height);

    virtual Coord width() const;
    virtual Coord height() const;
    virtual PixelCoord pwidth() const;
    virtual PixelCoord pheight() const;
		// width() and height() return the size of the canvas in terms of
		// printer points.  The pwidth() and pheight() functions return the
		// size of the canvas in terms of pixels.

    virtual PixelCoord to_pixels(Coord, DimensionName d=Dimension_X) const;
    virtual Coord to_coord(PixelCoord, DimensionName d=Dimension_X) const;
    virtual Coord to_pixels_coord(Coord, DimensionName d=Dimension_X) const;
		// device/world coordinate system conversions.  The to_pixels()
		// function converts printer points to pixels, and the to_coord()
		// function converts pixels to printer points.  The dimension argument
		// is an extension of the InterViews 3.1 distribution which doesn't
		// distinguish between the X and Y dimensions.  The dimension defaults
		// to the x dimension to provide backward compatibility (although
		// potentially incorrect behavior).


    virtual void new_path();
    virtual void move_to(Coord x, Coord y);
    virtual void line_to(Coord x, Coord y);
    virtual void curve_to(
		Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    );
    virtual void close_path();

    virtual void stroke(const Color*, const Brush*);
    virtual void line(
		Coord x1, Coord y1, Coord x2, Coord y2, const Color*, const Brush*
    );
    virtual void rect(
		Coord l, Coord b, Coord r, Coord t, const Color*, const Brush*
    );
    virtual void fill(const Color*);
    virtual void fill_rect(Coord l, Coord b, Coord r, Coord t, const Color*);

    virtual void character(
		const Font*, long ch, Coord width, const Color*, Coord x, Coord y
    );

    virtual void stencil(const Bitmap*, const Color*, Coord x, Coord y);
    virtual void image(const Raster*, Coord x, Coord y);
		// stencil() stencils the bitmap foreground bits (those set to 1) 
		// on the rendering surface in the given color at the given location.  
		// The bitmap acts as a mask for the operation, as well as the source 
		// of data.
		//
		// image() transfers a Raster (color bitmap) to the canvas surface
		// at the given location.  
		//


    virtual void push_transform();
    virtual void transform(const Transformer&);
    virtual void pop_transform();

    virtual void transformer(const Transformer&);
    virtual const Transformer& transformer() const;

    virtual void push_clipping();
    virtual void clip();
    virtual void clip_rect(Coord l, Coord b, Coord r, Coord t);
    virtual void pop_clipping();

    virtual void damage(const Extension&);
    virtual void damage(Coord left, Coord bottom, Coord right, Coord top);
    virtual bool damaged(const Extension&) const;
    virtual bool damaged(
	Coord left, Coord bottom, Coord right, Coord top
    ) const;
    virtual void damage_area(Extension&);
    virtual void damage_all();
    virtual bool any_damage() const;
    virtual void restrict_damage(const Extension&);
    virtual void restrict_damage(
		Coord left, Coord bottom, Coord right, Coord top
    );

    virtual void front_buffer();
    virtual void back_buffer();
		// These functions are for canvases that are bound to a window.  If
		// the canvas is not bound to a window, these functions will have 
		// no effect.  The front_buffer() function switches the window into
		// immediate-mode rendering and will render directly to the window
		// without double-buffering (and allows rendering outside of the 
		// Glyph::draw() function.  This is typically used for things like
		// rubber-banding.  The back_buffer() function switchs out of
		// immediate-mode and back to being double-buffered if double 
		// buffering is being done.

    virtual void redraw(Coord left, Coord bottom, Coord right, Coord top);
    virtual void repair();

    CanvasRep* rep() const;
private:
    CanvasRep* rep_;

    /* anachronisms */
public:
    enum { mapped, unmapped, offscreen };


    virtual CanvasLocation status() const;
    unsigned int Width() const;
    unsigned int Height() const;
    virtual void SetBackground(const Color*);
};

inline CanvasRep* Canvas::rep() const { return rep_; }
#else	// for mswindows and mac this is pure virtual base class

class Canvas 
{
public:
    Canvas();
    virtual ~Canvas();

    virtual Window* window() const;
		// Returns the window associated with this canvas, if this canvas
		// is bound to a window.  A null pointer will be returned if this
		// canvas is not bound to a window.

    virtual void size(Coord width, Coord height) = 0;
    virtual void psize(PixelCoord width, PixelCoord height) = 0;

    virtual Coord width() const = 0;
    virtual Coord height() const = 0;
    virtual PixelCoord pwidth() const = 0;
    virtual PixelCoord pheight() const = 0;
		// width() and height() return the size of the canvas in terms of
		// printer points.  The pwidth() and pheight() functions return the
		// size of the canvas in terms of pixels.

    virtual PixelCoord to_pixels(Coord, DimensionName d=Dimension_X) const = 0;
    virtual Coord to_coord(PixelCoord, DimensionName d=Dimension_X) const = 0;
    virtual Coord to_pixels_coord(Coord, DimensionName d=Dimension_X) const;
		// device/world coordinate system conversions.  The to_pixels()
		// function converts printer points to pixels, and the to_coord()
		// function converts pixels to printer points.  The dimension argument
		// is an extension of the InterViews 3.1 distribution which doesn't
		// distinguish between the X and Y dimensions.  The dimension defaults
		// to the x dimension to provide backward compatibility (although
		// potentially incorrect behavior).


    virtual void new_path() = 0;
    virtual void move_to(Coord x, Coord y) = 0;
    virtual void line_to(Coord x, Coord y) = 0;
    virtual void curve_to(
		Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    ) = 0;
    virtual void close_path() = 0;

	virtual void ellipse_path(Coord x, Coord y, Coord rx, Coord ry);
		// This function creates a new path that represents an ellipse
		// shape that can be used to stroke, fill, clip, etc.  The x and
		// y coordinates are the center.  The coordinates rx and ry are
		// the radius in the x and y dimensions respectively.

    virtual void stroke(const Color*, const Brush*) = 0;
    virtual void line(
		Coord x1, Coord y1, Coord x2, Coord y2, const Color*, const Brush*
    );
    virtual void rect(
		Coord l, Coord b, Coord r, Coord t, const Color*, const Brush*
    );
    virtual void fill(const Color*) = 0;
    virtual void fill_rect(Coord l, Coord b, Coord r, Coord t, const Color*);

    virtual void character(
		const Font*, long ch, Coord width, const Color*, Coord x, Coord y
    ) = 0;

    virtual void stencil(const Bitmap*, const Color*, Coord x, Coord y) = 0;
    virtual void image(const Raster*, Coord x, Coord y) = 0;
		// stencil() stencils the bitmap foreground bits (those set to 1) 
		// on the rendering surface in the given color at the given location.  
		// The bitmap acts as a mask for the operation, as well as the source 
		// of data.
		//
		// image() transfers a Raster (color bitmap) to the canvas surface
		// at the given location.  
		//


    virtual void push_transform() = 0;
    virtual void transform(const Transformer&) = 0;
    virtual void pop_transform() = 0;

    virtual void transformer(const Transformer&) = 0;
    virtual const Transformer& transformer() const = 0;

    virtual void push_clipping(bool all = false) = 0;
    virtual void clip() = 0;
    virtual void clip_rect(Coord l, Coord b, Coord r, Coord t);
    virtual void pop_clipping() = 0;

    virtual void damage(const Extension&) = 0;
    virtual void damage(Coord left, Coord bottom, Coord right, Coord top) = 0;
    virtual bool damaged(const Extension&) const = 0;
    virtual bool damaged(
	Coord left, Coord bottom, Coord right, Coord top
    ) const = 0;
    virtual void damage_area(Extension&) = 0;
    virtual void damage_all() = 0;
    virtual bool any_damage() const = 0;
    virtual void restrict_damage(const Extension&) = 0;
    virtual void restrict_damage(
		Coord left, Coord bottom, Coord right, Coord top
    ) = 0;

    virtual void front_buffer();
    virtual void back_buffer();
		// These functions are for canvases that are bound to a window.  If
		// the canvas is not bound to a window, these functions will have 
		// no effect.  The front_buffer() function switches the window into
		// immediate-mode rendering and will render directly to the window
		// without double-buffering (and allows rendering outside of the 
		// Glyph::draw() function.  This is typically used for things like
		// rubber-banding.  The back_buffer() function switchs out of
		// immediate-mode and back to being double-buffered if double 
		// buffering is being done.

    virtual void redraw(Coord left, Coord bottom, Coord right, Coord top) = 0;
    virtual void repair() = 0;
};
#endif

#include <InterViews/_leave.h>

#endif
