#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =========================================================================
//
//						<IV-Mac/raster.c>
//
// MS-Windows implementation of the InterViews Raster class. 
//
// A memory device context is used to store the raster.  The peek and poke
// operations simply get translated to calls that get and set pixel values
// the the memory device context.
//
// 1.3
// 1999/01/11 12:03:41
//
// Windows 3.1/NT Port 
// Copyright (c) 1993 Tim Prinzing
//
// This media contains programs and data which are proprietary
// to Tim Prinzing.
//
// These contents are provided under a Tim Prinzing software source
// license, which prohibits their unauthorized resale or distribution 
// outside of the buyer's organization.
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
// =========================================================================
#include <assert.h>
#include <InterViews/raster.h>
#include <InterViews/session.h>
#include <InterViews/display.h>
#include <IV-Mac/raster.h>
#include <IV-Mac/color.h>

Raster::Raster(
	unsigned long width, 				// width of bitmap in pixels
	unsigned long height)				// height of bitmap in pixels
{
	rep_ = new RasterRep();
	rep_->width_ = width;
	rep_->height_ = height;
	Rect c;
	c.left = 0;
	c.top = 0;
	c.right = width;
	c.bottom = height;
	QDErr error = NewGWorld(&rep_->cg_, 0, &c, nil, nil, 0);
	
#if 0
	RGBColor cc; cc.red = cc.green = cc.blue = 0x0;
	CGrafPtr cg;
	GDHandle gd;
	GetGWorld(&cg, &gd);
	SetGWorld(rep_->cg_, nil);
	RGBBackColor(&cc);
	SetGWorld(cg, gd);
#endif
}

Raster::Raster(const Raster& ras)
{
	assert(0);
	rep_ = new RasterRep();
	rep_->width_ = 0;
	rep_->height_ = 0;
	rep_->cg_ = nil;
}

Raster::~Raster()
{
	if (rep_->cg_) {
		DisposeGWorld(rep_->cg_);
	}
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
	RGBColor c;
	CGrafPtr cg;
	GDHandle gd;
	GetGWorld(&cg, &gd);
	SetGWorld(rep_->cg_, nil);
	GetCPixel(x, rep_->height_ - y, &c);
	SetGWorld(cg, gd);
	
	// --- convert from RGB to intensity ----
	ColorIntensity scaleFactor(0xffff);
	red = c.red / scaleFactor;
	green = c.green / scaleFactor;
	blue = c.blue / scaleFactor;
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
	RGBColor c;
	ColorIntensity scaleFactor(0xffff);
	c.red = (unsigned short) (red*scaleFactor);
	c.green = (unsigned short) (green*scaleFactor);
	c.blue = (unsigned short) (blue*scaleFactor);

	CGrafPtr cg;
	GDHandle gd;
	GetGWorld(&cg, &gd);
	SetGWorld(rep_->cg_, nil);
	SetCPixel(x, rep_->height_ - y, &c);
	SetGWorld(cg, gd);
}

void Raster::flush() const
{
	// This is a no-op for the MS-Windows implementation.  
}

// #####################################################################
// ###################### class RasterRep
// #####################################################################
RasterRep::RasterRep() {}
RasterRep::~RasterRep(){}

