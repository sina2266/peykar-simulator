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
//				<IV-Win/canvas32.c>
//
// MS-Windows implementation of the InterView Canvas class using NT-specific
// features.  Specifically, this version of the canvas uses the world 
// transform and paths features available when a device context is placed
// into "GM_ADVANCED" mode.
//
//
// ========================================================================

#include <InterViews/canvas.h>
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
#include <IV-Win/raster.h>

//
// converts a Coordinate to TWIPS, which is what is expected for most
// of the Windows operations (ie those using logical coordainates).
//
inline int COORD2TWIPS(Coord c)
{
	return int( c * 20.0 + ((c > 0) ? 0.5 : -0.5));
}



MWcanvas32::MWcanvas32()
{
	pathBracketStarted = false;
}

MWcanvas32::~MWcanvas32()
{
}

void MWcanvas32::beginPaint(
	HDC hdc,							// device context to use
	const RECT& r)             			// area of update region
{
	//
	// set the "advanced" graphic mode, to use paths and transforms.
	//
	MWassert(SetGraphicsMode(hdc, GM_ADVANCED));

	MWcanvas::beginPaint(hdc, r);
}

// ----------------------------------------------------------------------
// Transformation 
//
// Set the world-coordinate transformation matrix to match the new one
// we get from the changed stack.
//
// Note that the "translate" component of the transformation matrix is
// in terms of printer points and the page space is in terms of twips,
// so we multiply the traslate component by 20 to adjust it to match
// the MS-Windows translation.
//
// ----------------------------------------------------------------------

void MWcanvas32::pop_transform()
{
	// ---- base functionality ----
	MWcanvas::pop_transform();

	// ---- update MS-Windows matrix ----
	XFORM x;
	matrix().matrix(x.eM11, x.eM12, x.eM21, x.eM22, x.eDx, x.eDy);
	x.eDx = x.eDx * FLOAT(20.0);
	x.eDy = x.eDy * FLOAT(20.0);
	SetWorldTransform(drawable_, &x);
}

void MWcanvas32::transform(const Transformer& t)
{
	// ---- base functionality ----
	MWcanvas::transform(t);

	// ---- update MS-Windows matrix ----
	XFORM x;
	matrix().matrix(x.eM11, x.eM12, x.eM21, x.eM22, x.eDx, x.eDy);
	x.eDx = x.eDx * FLOAT(20.0);
	x.eDy = x.eDy * FLOAT(20.0);
	SetWorldTransform(drawable_, &x);
}

void MWcanvas32::transformer(const Transformer& t)
{
	// ---- base functionality ----
	MWcanvas::transformer(t);

	// ---- update MS-Windows matrix ----
	XFORM x;
	matrix().matrix(x.eM11, x.eM12, x.eM21, x.eM22, x.eDx, x.eDy);
	x.eDx = x.eDx * FLOAT(20.0);
	x.eDy = x.eDy * FLOAT(20.0);
	SetWorldTransform(drawable_, &x);
}

// ----------------------------------------------------------------------
// clipping
// ----------------------------------------------------------------------

void MWcanvas32::clip()
{
	// ---- close the path bracket ---
	MWassert(pathBracketStarted);
	MWassert(EndPath(drawable_));
	pathBracketStarted = false;

	// ---- set the new clipping ----
	HRGN rgn;
	MWassert(rgn = PathToRegion(drawable_));
	MWassert(ExtSelectClipRgn(drawable_, rgn, RGN_AND) != ERROR);

	// ---- replace the old region -----
	DeleteObject(clipping_);
	clipping_ = rgn;
}

void MWcanvas32::new_path()
{
	MWassert( BeginPath(drawable_) );
	pathBracketStarted = true;
}

void MWcanvas32::move_to(Coord x, Coord y)
{
	if (! pathBracketStarted )
	{
		new_path();
	}

	int xTwip = int(x * 20.0);
	int yTwip = int(y * 20.0);
	MWassert(MoveToEx(drawable_, xTwip, yTwip, NULL));
}

void MWcanvas32::line_to(Coord x, Coord y)
{
	int xTwip = int(x * 20.0);
	int yTwip = int(y * 20.0);
	MWassert(LineTo(drawable_, xTwip, yTwip));
}

void MWcanvas32::curve_to(
	Coord x,
    Coord y,
	Coord x1,
	Coord y1,
	Coord x2,
	Coord y2)
{
	POINT pts[3];
	pts[0].x = int( x * 20.0 );
	pts[0].y = int( y * 20.0 );
	pts[1].x = int( x1 * 20.0 );
	pts[1].y = int( y1 * 20.0 );
	pts[2].x = int( x2 * 20.0 );
	pts[2].y = int( y2 * 20.0 );
	MWassert( PolyBezierTo(drawable_, (const POINT*) &pts, 3) );
}

void MWcanvas32::close_path()
{
	MWassert( CloseFigure(drawable_) );
}

void MWcanvas32::stroke(const Color* color, const Brush* b)
{
	// ---- close the path bracket ---
	MWassert(pathBracketStarted);
	MWassert(EndPath(drawable_));
	pathBracketStarted = false;

	// ---- ready pen data ----
	BrushRep brep;
	BrushRep* br;
	if (b) {
		br = b->rep(nil);
	}else{
		br = &brep;
		brep.penWidth = 0;
		brep.dashList = nil;
		brep.dashCount = 0;
	}
	DWORD styOption = (br->dashList) ? PS_USERSTYLE : PS_SOLID;
	DWORD style = PS_GEOMETRIC | styOption;
	LOGBRUSH brushData;
	brushData.lbStyle = BS_SOLID;
	brushData.lbColor = color->rep()->msColor();
	brushData.lbHatch = NULL;
	if (color->rep()->op == Color::Xor)
	{
		SetROP2(drawable_, R2_XORPEN);
	}
	else if (color->rep()->op == Color::Invisible)
	{
		SetROP2(drawable_, R2_NOP);
	}
	else
	{
		SetROP2(drawable_, R2_COPYPEN);
	}

	// ---- create the pen, and make it current ----
	HPEN pen = ExtCreatePen(style, (DWORD) (br->penWidth * 20.0),
			&brushData, br->dashCount, br->dashList);
	HPEN old = (HPEN)SelectObject(drawable_, pen);

	// ---- stroke the path ----
	MWassert( StrokePath(drawable_) );

	// --- remove and destroy pen ---
	SelectObject(drawable_, old);
	MWassert( DeleteObject(pen) );
}

void MWcanvas32::fill(const Color* color)
{
	if (color->rep()->stipple)
	{
		// ---- stipple some color into the area ----
		stencilFill(color->rep()->stipple, color);
	}
	else
	{
		// ---- close the path bracket ---
		MWassert(pathBracketStarted);
		MWassert(EndPath(drawable_));
		pathBracketStarted = false;

		// ---- create the brush, and make it current ----
		HBRUSH brush;
		HBRUSH old;
		HBITMAP hbm = NULL;
		brush = CreateSolidBrush(color->rep()->msColor());
		MWassert(brush);
		old = (HBRUSH)SelectObject(drawable_, brush);

		// ---- fill the path ----
		MWassert( FillPath(drawable_) );

		// --- remove and destroy resources ---
		SelectObject(drawable_, old);
		MWassert( DeleteObject(brush) );
		if (hbm)
			DeleteObject(hbm);
	}
}

// -------------------------------------------------------------------------
// text interface
// -------------------------------------------------------------------------
void MWcanvas32::character(
	const Font* f, 					// font to use
	long ch,                        // character to print
	Coord width,                    // width of the character
	const Color* color,             // color to use
	Coord x,                        // x-coordinate
	Coord y)                        // y-coordinate
{
	char c = (char) ch;

    SetBkMode(drawable_, TRANSPARENT);
	SetTextColor(drawable_, color->rep()->msColor());
	FontRep* fr = f->rep(nil);
	MWassert(fr);
	HFONT new_fnt = fr->Create();
	HFONT old_fnt = (HFONT) SelectObject(drawable_, new_fnt);
	SetTextAlign(drawable_, TA_LEFT | TA_BASELINE | TA_NOUPDATECP);
	TextOut(drawable_, int(x * Coord(20)), int(y * Coord(20)), &c, 1);
	DeleteObject( SelectObject(drawable_, old_fnt));
}

// -----------------------------------------------------------------------
// Stencil the "set" bits of the bitmap onto the canvas in the given color
// at the given origin.  
// -----------------------------------------------------------------------
void MWcanvas32::stencil(
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
	HBRUSH brush = CreateSolidBrush(c->rep()->msColor());
	MWassert(brush);
	HBRUSH old = (HBRUSH)SelectObject(drawable_, brush);

	// For some reason, text color must be black when stenciling or the
	// colors of the stencil are affected. 
	SetTextColor(drawable_, RGB(0, 0, 0));

	// ---- blt... the destination will be transformed ----
	MWassert( StretchBlt(drawable_, x0, y0 + height, width, - height, 
		mem_hdc, 0, 0, br.bm_.bmWidth, br.bm_.bmHeight, 0xB8074AL) );

	// ---- cleanup ----
	DeleteDC(mem_hdc);
	DeleteObject(hbm);
	SelectObject(drawable_, old);
	DeleteObject(brush);
}

// -----------------------------------------------------------------------
// Render an image onto the GDI surface.  The image is transformed in 
// terms of scaling only initially (all that Windows 3.1 supports).  
// Rotation is initially not supported.
// -----------------------------------------------------------------------
void MWcanvas32::image(const Raster* ras, Coord x, Coord y)
{
	//
	// convert the coordinates to TWIPS for the page space to device space
	// mapping.  
	//
	int x0 = int( x * 20.0);
	int y0 = int( y * 20.0);
	int width = ras->pwidth() * 20;
	int height = ras->pheight() * 20;

	// ---- blt... the destination will be transformed ----
	MWassert( StretchBlt(drawable_, x0, y0 + height, width, - height, 
		ras->rep()->deviceContext(), 
		0, 0, ras->pwidth(), ras->pheight(), SRCCOPY) );

}


