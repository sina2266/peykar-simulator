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
//				<IV-Win/canvas.h>
//
// MS-Windows implementation of the InterView Canvas class.  
//
// This canvas type renders into an MS-Windows window.  Because it is 
// largely GDI based, most of the CanvasRep can be reused for printing 
// which also uses the GDI interface.
//
// True-Type fonts are used to provide scaling and rotation capabilies,
// which greatly simplifies (and increases performance of) the font scale
// and rotate capabilies of InterViews.  This class is therefore dependant
// upon MS-Windows 3.1 or later since that is the version that introduced
// True-Type fonts.
//
// THIS FILE USED TO USE TEMPLATES, BUT WAS CONVERTED TO BE COMPATIBLE WITH
// THE MICROSOFT COMPILER WHICH IS BEHIND THE TIMES!!
//
// 1.4
// 1999/07/05 15:34:37
//
// ========================================================================
#ifndef ivwin_canvas_h
#define ivwin_canvas_h

// ---- InterViews includes ----
#include <InterViews/canvas.h>
#include <InterViews/iv.h>
#include <IV-Win/MWlib.h>
#include <IV-Win/MWrect.h>
class BitmapRep;
class WindowRep;
class Canvas;
class Display;

class MWtransformPtrList;		// PtrList<Transformer*>
class MWclipList;				// List<HRGN>



class MWcanvas : public Canvas
{
public:
	MWcanvas();
	virtual ~MWcanvas();

	void bind(WindowRep* w)				// bind canvas to a window
    	{ win_ = w; }

	HDC hdcOf() const;
		// Returns the current device context of the canvas, or NULL if the
		// canvas is not currently drawable.

	virtual void beginPaint(HDC, const RECT&);
	virtual void endPaint(); 				
		// These two functions determine when the canvas can be rendered
		// upon, and when it can't.  The canvas is not valid for drawing
		// outside of a begin/end pair.  


public:	// ---------------- InterViews interface ---------------------

    virtual Window* window() const;

    virtual void size(Coord width, Coord height);
    virtual void psize(PixelCoord width, PixelCoord height);

    virtual Coord width() const;
    virtual Coord height() const;
    virtual PixelCoord pwidth() const;
    virtual PixelCoord pheight() const;

    virtual PixelCoord to_pixels(Coord, DimensionName) const;
    virtual Coord to_coord(PixelCoord, DimensionName) const;

    virtual void push_transform();
    virtual void transform(const Transformer&);
    virtual void pop_transform();
    virtual void transformer(const Transformer&);
    virtual const Transformer& transformer() const;

    virtual void push_clipping(bool all = false);
    virtual void pop_clipping();

    virtual void front_buffer();
    virtual void back_buffer();

    virtual void damage(const Extension&);
    virtual void damage(Coord left, Coord bottom, Coord right, Coord top);
    virtual bool damaged(const Extension&) const;
    virtual bool damaged( 
		Coord left, Coord bottom, Coord right, Coord top) const;
    virtual void damage_area(Extension&);
    virtual void damage_all();
    virtual bool any_damage() const;
    virtual void restrict_damage(const Extension&);
    virtual void restrict_damage(Coord left, Coord bottom, 
		Coord right, Coord top);

    virtual void redraw(Coord left, Coord bottom, Coord right, Coord top);
    virtual void repair();

	Transformer& matrix() const;		
		// current transformation matrix in effect for the rendering 
		// surface.

protected:

	void stencilFill(const Bitmap*, const Color*);
		// This function fills the current path with a stenciled pattern
		// in the given color.  This function is basically used to simulate
		// the alpha blending of color which is not directly supported by
		// the GDI interface (ie PatBlt doesn't allow raster operations that
		// specify source as part of the operation... so no stencil).  Since
		// the regions stenciled are typically not that large, this shouldn't
		// be too big of a shortcoming... it's too bad though because some
		// smart framebuffers can do this in hardware wicked-fast :-)
		//
	virtual void setmapmode(HDC);
		// the map mode generally uses cached information with
		// regard to the display. But for a MacPrinterCanvas
		// we use different info.
protected:

	WindowRep* win_;				// associated window
	Display* dpy;					// display
	HDC drawable_;					// display context to use for drawing

	bool transformed_;
	MWtransformPtrList* transformers_;
		// Transformation matrix.  The transformed variable allows a fast
		// test to see if we have any transformations in effect.

	HRGN clipping_;
	MWclipList* clippers_;
		// ---- clipping support ----
		// The region held in clipping_ is the current clipping region, and
		// can be changed by a new clip request via the clip function.  The
		// region may be replaced, but the old region should be destroyed.
		// The clippers_ holds previous clip regions and should be adjusted
		// only by push and pop of the stack.

private:

	MWcoordRect damageArea;			
		// area of canvas currently damaged.  This is maintained in terms of
		// world coordinates since it is used primarily by clients of the 
		// canvas.

	MWpixelPoint pixelSize;			
		// size of canvas in pixels.

};

// #########################################################################
// ############# 
// #############            MWcanvas32 - Win32 specific canvas
// ############# 
// #############  The following class is an implementation of the Win32
// #############  varient of functionality that differs between the versions
// #############  of MS-Windows.  The Win32 functionality uses the device-
// #############  context in it's advanced mode using the path and world
// #############  world coordinate transform support.
// ############# 
// #########################################################################
#if 1//0 && (!defined(WIN32S)) && (!defined(WIN16))

class MWcanvas32 : public MWcanvas
{
public:
	MWcanvas32();
	virtual ~MWcanvas32();

	virtual void beginPaint(HDC hdc, const RECT& r);

	virtual void new_path();
	virtual void move_to(Coord x, Coord y);
	virtual void line_to(Coord x, Coord y);
	virtual void curve_to(Coord x, Coord y, 
		Coord x1, Coord y1, Coord x2, Coord y2);
	virtual void close_path();

    virtual void stroke(const Color*, const Brush*);
    virtual void fill(const Color*);
	virtual void clip();
    virtual void character(const Font*, long ch, Coord width, 
		const Color*, Coord x, Coord y);
    virtual void stencil(const Bitmap*, const Color*, Coord x, Coord y);
    virtual void image(const Raster*, Coord x, Coord y);

	virtual void pop_transform();
	virtual void transform(const Transformer& t);
	virtual void transformer(const Transformer& t);

private:
	bool pathBracketStarted;
};

#endif


// #########################################################################
// ############# 
// #############            MWcanvas16 - Win16/Win32s
// ############# 
// #############  The following class is an implementation of the Win16
// #############  varient of functionality.  It provides extra code to 
// #############  implement transformations and paths in MS-Windows.
// ############# 
// #########################################################################

class PathRenderInfo
{
public:
    Coord curx_;
    Coord cury_;
	POINT* point_;
	POINT* cur_point_;
	POINT* end_point_;
};

class MWcanvas16 : public MWcanvas
{
public:
	MWcanvas16();
	virtual ~MWcanvas16();


	virtual void beginPaint(HDC, const RECT&);
	virtual void endPaint(); 				
		// These two functions determine when the canvas can be rendered
		// upon, and when it can't.  The canvas is not valid for drawing
		// outside of a begin/end pair.  

	virtual void new_path();
	virtual void move_to(Coord x, Coord y);
	virtual void line_to(Coord x, Coord y);
	virtual void curve_to(Coord x, Coord y, 
		Coord x1, Coord y1, Coord x2, Coord y2);
	virtual void close_path();

	virtual void push_clipping(bool all = false);
	virtual void pop_clipping();
	virtual void clip();

    virtual void stroke(const Color*, const Brush*);
    virtual void fill(const Color*);
    virtual void character(const Font*, long ch, Coord width, 
		const Color*, Coord x, Coord y);
    virtual void stencil(const Bitmap*, const Color*, Coord x, Coord y);
    virtual void image(const Raster*, Coord x, Coord y);

	virtual void push_transform();
	virtual void pop_transform();
	virtual void transform(const Transformer& t);
	virtual void transformer(const Transformer& t);
    virtual const Transformer& transformer() const;

protected:
	void flush();					// flush any buffered operations

	int transformAngle() const;
		// determines the current transformation angle in terms of tenths
		// of a degree.  Returns a number between 0 and 3600.

    // ---- attribute setting functions ----
	void color(const Color*);
	void brush(const Brush*);
	void font(const Font*);

private:

	HPEN pen_;						// current pen to render with
    HBRUSH brush_;					// current brush to render with
	LOGPEN pen_stats_;				// what the pen was constructed from
	LOGBRUSH brush_stats_;			// what the brush was constructed from

	HPEN old_pen_;					// pen before our pen was selected into the DC
	HBRUSH old_brush_;				// brush before our brush was selected into the DC

	// ---- logical attributes -----
	const Color* lg_color_;			// last IV color set
	const Brush* lg_brush_;			// last IV brush set
	const Font* lg_font_;			// last IV font set
	static PathRenderInfo path_;	// path info (filled by Canvas members)

	// ---- text support ----
	char* text_buff_;				// text buffer
	char* text_ptr_;				// pointer to current position in buffer
	int text_x0_;					// x-coordinate of text start
	int text_y0_;					// y-coordinate of text start
	int text_width_;				// width of text string
	int text_x_ptr_;				// x-coordinate of next insert position
	int text_y_ptr_;				// y-coordinate of next insert position
};



// ---- inline functions for MWcanvas ----
inline HDC MWcanvas::hdcOf() const
	{ return drawable_; }


#endif /* ivwin_canvas_h */
