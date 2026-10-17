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
//						<IV-Win/raster.c>
//
// MS-Windows implementation of the InterViews Raster class. 
//
// A memory device context is used to store the raster.  The peek and poke
// operations simply get translated to calls that get and set pixel values
// the the memory device context.
//
// ========================================================================

#include <IV-Win/MWlib.h>
#include <InterViews/raster.h>
#include <InterViews/session.h>
#include <InterViews/display.h>
#include <IV-Win/raster.h>
#include <IV-Win/color.h>

Raster::Raster(
	unsigned long width, 				// width of bitmap in pixels
	unsigned long height)				// height of bitmap in pixels
{
	//
	// Create a memory device context of the size needed for the bitmap,
	// that is compatible with the desktop for later blt operations.
	// This form of raster is completely uninitialized, and must be filled
	// with poke() calls.
	//
	HDC hdc = GetDC(NULL);
	rep_ = new RasterRep(hdc);
	SelectObject(rep_->deviceContext(), 
		CreateCompatibleBitmap(hdc, width, height));
	ReleaseDC(NULL, hdc);

	rep_->width_ = width;
	rep_->height_ = height;
}

Raster::Raster(const Raster& ras)
{
	//
	// copy constructor.  We basically create another memory device context
	// compatible with the current one and copy the contents with a blt.
	//
	rep_ = new RasterRep(ras.rep()->deviceContext());
	BitBlt(rep_->deviceContext(), 0, 0, ras.pwidth(), ras.pheight(),
		ras.rep()->deviceContext(), 0, 0, SRCCOPY);

	rep_->width_ = ras.pwidth();
	rep_->height_ = ras.pheight();
}

Raster::~Raster()
{
	delete rep_;
}


// -------------------------------------------------------------------------
// These function simply query the resolution of the memory device context.
// The width() and height() functions also scale for the resolution of the
// current display in the session.
// -------------------------------------------------------------------------
Coord Raster::width() const
{
	Display* d = Session::instance()->default_display();
    return d->to_coord(rep_->width_, Dimension_X);
}

Coord Raster::height() const
{
	Display* d = Session::instance()->default_display();
    return d->to_coord(rep_->height_, Dimension_Y);
}

unsigned long Raster::pwidth() const
{
	return rep_->width_;
}

unsigned long Raster::pheight() const
{
	return rep_->height_;
}

// -----------------------------------------------------------------------
// should provide reasonable return values.
// -----------------------------------------------------------------------
Coord Raster::left_bearing() const
	{ return Coord(0); }
Coord Raster::right_bearing() const
	{ return width(); }
Coord Raster::ascent() const
	{ return height(); }
Coord Raster::descent() const
	{ return Coord(0); }

// -----------------------------------------------------------------------
// Pixel access.  Note the the orgin of the Raster is the lower left 
// corner, but for the MS-Windows bitmap it is the upper left corner
// so translation is done for pixel access.
// -----------------------------------------------------------------------
void Raster::peek(
	unsigned long x, 					// x coordinate of pixel queried
	unsigned long y,					// y coordinate of pixel queried
	ColorIntensity& red, 
	ColorIntensity& green, 
	ColorIntensity& blue,
	float& alpha) const
{
	// ---- get the pixel value ----
	COLORREF pixelColor = GetPixel(rep_->deviceContext(), x, rep_->height_-y);

	// --- convert from RGB to intensity ----
	ColorIntensity scaleFactor(255.0);
	red = GetRValue(pixelColor) / scaleFactor;
	green = GetGValue(pixelColor) / scaleFactor;
	blue = GetBValue(pixelColor) / scaleFactor;
	alpha = 1.0;
}

void Raster::poke(
	unsigned long x, 
	unsigned long y,
	ColorIntensity red, 
	ColorIntensity green, 
	ColorIntensity blue,
	float alpha)
{
	// ---- get the desired RGB components ----
	ColorIntensity scaleFactor(255.0);
	int r = (int) (red * scaleFactor);
	int g = (int) (green * scaleFactor);
	int b = (int) (blue * scaleFactor);

	// ---- make sure it's in the palette ----
	// In MS-Windows the palette stuff works even if there isn't a palette
	// as long as you don't use palette index values.
	ColorRep::defaultPalette()->addEntry(r, g, b);

	// ---- set the pixel ----
	COLORREF pixelColor = PALETTERGB( LOBYTE(r), LOBYTE(g), LOBYTE(b));
	SetPixel(rep_->deviceContext(), x, rep_->height_ - y, pixelColor);
}

void Raster::flush() const
{
	// This is a no-op for the MS-Windows implementation.  
}

// #####################################################################
// ###################### class RasterRep
// #####################################################################

RasterRep::RasterRep(HDC hdc)
{
	hdcMem = CreateCompatibleDC(hdc);
}

RasterRep::~RasterRep()
{
	DeleteDC(hdcMem);
}

