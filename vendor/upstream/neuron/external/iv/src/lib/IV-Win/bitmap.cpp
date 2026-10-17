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
//                            <IV-Win/bitmap.C>
//
// MS-Windows implementation of the InterViews bitmap class.  
//
// The windows data structure BITMAP is used until the bitmap is needed 
// for actual rendering in the canvas, at which time a temporary GDI object 
// is created.  Operations on this bitmap are therefore very light-weight.
//
//
// ========================================================================

#include <IV-Win/MWlib.h>
#include <IV-Win/bitmap.h>
#include <IV-Win/MWapp.h>
#include <InterViews/bitmap.h>
#include <InterViews/session.h>
#include <InterViews/display.h>
#include <OS/math.h>
#include <OS/memory.h>


Bitmap::Bitmap()
{
    rep_ = nil;
}

// copy constructor
Bitmap::Bitmap(const Bitmap& bm) 
{
    rep_ = (bm.rep_) ? new BitmapRep(*bm.rep_) : nil;
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
    Bitmap* bm = new Bitmap;
	BitmapRep* b = new BitmapRep;
	if (!bm || !b)
		return (Bitmap*) 0;
	bm->rep_ = b;

    // ----- try to load the bitmap ----
	if (b->Load(name) == false)
	{
		// ---- failed to load ----
		delete bm;
		return (Bitmap*) 0;
	}

	return bm;
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
	BitmapRep* b = new BitmapRep(data, w, h, x0, y0);
	rep_ = b;

}

Bitmap::~Bitmap()
{
    delete rep_;
}

Coord Bitmap::width() const { return rep()->width_; }
Coord Bitmap::height() const { return rep()->height_; }
unsigned int Bitmap::pwidth() const { return rep()->pwidth(); }
unsigned int Bitmap::pheight() const { return rep()->pheight(); }

Coord Bitmap::left_bearing() const { return -rep()->left_; }
Coord Bitmap::right_bearing() const { return rep()->right_; }
Coord Bitmap::ascent() const { return rep()->top_; }
Coord Bitmap::descent() const { return -rep()->bottom_; }

// ---------------------------------------------------------------------
// Bit manipulation functions... not bounds checked for efficiency.
// ---------------------------------------------------------------------
void Bitmap::poke(bool set, int x, int y) 
{
    BitmapRep* b = rep();
	b->poke(set, x, y);
}

bool Bitmap::peek(int x, int y) const 
{
    BitmapRep* b = rep();
	return b->peek(x,y);
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
	bm_.bmBits = nil;
	hot_x_ = 0;
	hot_y_ = 0;
}

BitmapRep::BitmapRep(const BitmapRep& b)
{
	left_ = b.left_;
    bottom_ = b.bottom_;
    right_ = b.right_;
    top_ = b.top_;
    width_ = b.width_;
	height_ = b.height_;

	hot_x_ = b.hot_x_;
	hot_y_ = b.hot_y_;

	// ---- copy logical bitmap representation ----
	bm_ = b.bm_;					
	unsigned int len = b.bm_.bmWidthBytes * b.bm_.bmHeight;
	bm_.bmBits = new char[len];
	Memory::copy( b.bm_.bmBits, bm_.bmBits, len);
}

static unsigned char FlipBits(unsigned char before)
{
	unsigned char after = 0;
	int i;
	unsigned char mask;

	for (i=0; i<8; i++)
	{
		mask = 1 << i;
        if (before & mask)
			after |= 1 << (7-i);
	}
	return after;
}

BitmapRep::BitmapRep(
	const void* data,		// bitmap data
	unsigned int w,			// width of bitmap
	unsigned int h,			// height of bitmap
	int x0,					// x-coord of hotspot
	int y0)					// y-coord of hotspot
{
	hot_x_ = x0;
	hot_y_ = y0;

	// ---- fill in BITMAP structure ----
	bm_.bmType = 0;
	bm_.bmWidth = w;
	bm_.bmHeight = h;
	bm_.bmPlanes = 1;
	bm_.bmBitsPixel = 1;
	bm_.bmWidthBytes = (w / 16) * 2;
	if ((w % 16) != 0)
		bm_.bmWidthBytes += 2;
	bm_.bmBits = new char[(bm_.bmWidthBytes) * h];

	// ---- initialize the bitmap data ----
	char* curr_byte = (char*) bm_.bmBits;	// current byte of scan line
	unsigned int scan_line;					// current scan line index
	int i;									// index into current line

	if (data != nil)
	{
		// ---- initialize from data ----
		char* copy_data = (char*) data;		// current byte being copied
		int scanBytes = (bm_.bmWidth / 8) + ((bm_.bmWidth  % 8) ? 1 : 0);

		for (scan_line = 0; scan_line < h; scan_line++)
		{
			for (i = 0; i < bm_.bmWidthBytes; i++)
			{
				if (i < scanBytes)
                {
					*(curr_byte++) = ~ FlipBits(*(copy_data++));
				}
				else
				{
					*(curr_byte++) = char(0xFF);
                }
			}
		}
	}
	else
	{
		// ---- create an empty bitmap ----
		for (scan_line = 0; scan_line < h; scan_line++)
		{
			for (i = 0; i < bm_.bmWidthBytes; i++)
			{
				*(curr_byte++) = char(0xFF);
			}
		}
        
	}

	// ---- fill in size fields -----
	SyncSize();
}

// -----------------------------------------------------------------------
// Load a bitmap from a resource.  Since the "Bitmap" class is supposed to
// be mono and depends upon it... bad status will be returned if the bitmap
// is in fact color!
// -----------------------------------------------------------------------
bool BitmapRep::Load(const char* name)
{
	HINSTANCE hinst = theApp.hinst;
	HBITMAP hbmp;

	// ----- try to load the bitmap ----
	if ((hbmp = LoadBitmap(hinst, name)) == NULL)
	{
		// ---- failed to load ----
		return false;
	}

	// ---- load the bitmap into the BITMAP structure ----
	GetObject(hbmp, sizeof(BITMAP), &bm_);
	if ((bm_.bmPlanes != 1) || (bm_.bmBitsPixel != 1))
	{
		// ---- bitmap is color ----
		DeleteObject(hbmp);
        return false;
    }

	// ---- fetch bitmap data ----
	int bm_size = bm_.bmWidthBytes * bm_.bmHeight;
	bm_.bmBits = new char[bm_size];
	long nfetched = GetBitmapBits(hbmp, bm_size, bm_.bmBits);
	DeleteObject(hbmp);

	if (nfetched != 0L)
	{
		SyncSize();
		return true;
	}

	return false;
}

// -----------------------------------------------------------------------
// Synchronize the size fields with the bitmap size and origin.
// NOTE: This later needs to be adjusted to convert to points.
// -----------------------------------------------------------------------
void BitmapRep::SyncSize()
{
	Display* dpy = Session::instance()->default_display();
	MWassert(dpy);

	width_ = dpy->to_coord(bm_.bmWidth, Dimension_X);
	height_ = dpy->to_coord(bm_.bmHeight, Dimension_Y);
	if (hot_x_ == -1 && hot_y_ == -1)
	{
		left_ = 0;
		right_ = width_;
		bottom_ = 0;
		top_ = height_;
	}
	else
	{
		left_ = - dpy->to_coord(hot_x_, Dimension_X);
		right_ = width_ - dpy->to_coord(hot_x_, Dimension_X);         
		bottom_ = dpy->to_coord(hot_y_, Dimension_Y) - height_;
		top_ = dpy->to_coord(hot_y_, Dimension_Y);
	}
}

BitmapRep::~BitmapRep() 
{
	if (bm_.bmBits != nil)
		delete [] (char*)bm_.bmBits;
}

// -----------------------------------------------------------------------
// Determine if the bit at the given coordinate is set or not.  For 
// efficiency the coordinates are not checked, and are presumed to have
// been checked by the client of this function.
// -----------------------------------------------------------------------
bool BitmapRep::peek(int x, int y) const 
{
	int offset = (y * bm_.bmWidthBytes) + (x / 8);
	unsigned char* bits = (unsigned char*) bm_.bmBits;
	unsigned char data = bits[offset];
	unsigned char mask = 0x80 >> (x % 8);

	return (data & mask);
}

// -----------------------------------------------------------------------
// Set or clear the bit at the given coordinate.  For efficiency, the 
// coordinates are not checked, and are presumed to have been checked by
// the client of this function.
// -----------------------------------------------------------------------
void BitmapRep::poke(
	bool set, 					// set or clear?
	int x, 							// x-coordinate 
	int y) 							// y-coordinate
{
	int offset = (y * bm_.bmWidthBytes) + (x / 8);
	unsigned char mask = 0x80 >> (x % 8);
    unsigned char* bits = (unsigned char*) bm_.bmBits;

	if (set)
	{
		// ---- set the bit ----
		bits[offset] |= mask;
	}
	else
	{
		// ---- clear the bit ----
		bits[offset] &= ~mask;
	}
}


