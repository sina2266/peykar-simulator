#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =========================================================================
//
//                            <IV-Mac/bitmap.C>
//
// MS-Windows implementation of the InterViews bitmap class.  
//
// The windows data structure BITMAP is used until the bitmap is needed 
// for actual rendering in the canvas, at which time a temporary GDI object 
// is created.  Operations on this bitmap are therefore very light-weight.
//
//
// 1.1
// 1997/03/28 17:36:11
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


#include <InterViews/bitmap.h>
#include <InterViews/session.h>
#include <InterViews/display.h>
#include <IV-Mac/bitmap.h>
#include <OS/math.h>
#include <OS/memory.h>

Bitmap::Bitmap()
{
    rep_ = nil;
}



// -----------------------------------------------------------------------
// Create a named bitmap.  In the Windows world, this means a resource
// based bitmap.  The name of the bitmap is expected to match a name in
// the resource file, or the number of the bitmap (ie "#123").  A null
// pointer is returned if the bitmap can't be found.  This interface is
// the most natural, not favoring a particular windowing system because
// the bitmap is implimented in terms of the native window system.
// -----------------------------------------------------------------------
Bitmap* Bitmap::open(const char* name)
{
	// ---- allocate the Bitmap instance ----
    //Bitmap* bm = new Bitmap;
	//BitmapRep* b = new BitmapRep;
	//if (!bm || !b)
	//	return (Bitmap*) 0;
	//bm->rep_ = b;

    
	return nil;
}

// -----------------------------------------------------------------------
// This constructor takes bitmap data from "compile-time" which in the X11
// world was probably from an XBM file.  A difference from X11 is that
// Windows expects the data to be aligned to 16-bit boundries for each scan
// line.  Because of the X11 heritage, the data passed is 8-bit aligned.
// Also, the data in the X11 world is stored with a mapping of left to right
// as LSB to MSB per byte, which is oposite from MS-Windows which stores
// left to right as MSB to LSB.  Another difference is that a black bit is
// a one in X11 and a zero in Windows.  The interface for this constructor
// is the X11 format data, and conversion is performed into the Windows
// format.  This is to keep compatibility with X11 software directly.  If
// Windows native bitmaps are desired... the name based constructor
// should be used.
// -----------------------------------------------------------------------
Bitmap::Bitmap(
	const void* data,		// bitmap data
	unsigned int w,			// width of bitmap
	unsigned int h,			// height of bitmap
	int x0,					// x-coord of hotspot
	int y0)					// y-coord of hotspot
{
	rep_ = new BitmapRep();
	rep_->data_ = data;
	rep_->width_ = w;
	rep_->height_ = h;
	rep_->xhot_ = x0;	// Dont' know what this means with respect to placement. Assuming
	rep_->yhot_ = y0;	// origin is left,bottom.
	
}

Bitmap::~Bitmap()
{
    delete rep_;
}

Coord Bitmap::width() const { return Coord(rep_->width_); }
Coord Bitmap::height() const { return Coord(rep_->height_); }
unsigned int Bitmap::pwidth() const { return rep_->width_; }
unsigned int Bitmap::pheight() const { return rep_->height_; }

Coord Bitmap::left_bearing() const { return Coord(0); }
Coord Bitmap::right_bearing() const { return Coord(int(rep_->width_)); }
Coord Bitmap::ascent() const { return Coord(int(rep_->height_)); }
Coord Bitmap::descent() const { return Coord(0); }

// ---------------------------------------------------------------------
// Bit manipulation functions... not bounds checked for efficiency.
// ---------------------------------------------------------------------
void Bitmap::poke(bool set, int x, int y) 
{
    ;
}

bool Bitmap::peek(int x, int y) const 
{
   return false;
}

// ---------------------------------------------------------------------
// This function has absolutely no meaning in the MS-Windows 
// implimentation.  I don't think it even has a reason to exist, and 
// should be removed from the interface!!
// ---------------------------------------------------------------------
void Bitmap::flush() const 
{ 
}

// ##################################################################
// #################  class BitmapRep 
// ##################################################################

BitmapRep::BitmapRep() 
{
	
}

BitmapRep::~BitmapRep() 
{
	
}
