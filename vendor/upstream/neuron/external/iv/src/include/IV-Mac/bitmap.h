// ===========================================================================
//
//                            <IV-mac/bitmap.h>
//
// MS-Windows implementation of the InterViews bitmap class.  
//
// The windows data structure BITMAP is used until the bitmap is needed 
// for actual rendering in the canvas, at which time a temporary GDI object 
// is created.  Operations on this bitmap are therefore very light-weight.
//
//
// 1.1
// 1997/03/28 17:35:39
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
// ============================================================================
#ifndef iv_mac_bitmap_h
#define iv_mac_bitmap_h

// ---- InterViews includes ----
#include <InterViews/coord.h>
#include <InterViews/iv.h>

// ---- windows includes ----
#if !carbon
#include <windows.h>
#endif

class BitmapRep
{
public:
	
	friend class CursorRep;

    enum { copy, fliph, flipv, rot90, rot180, rot270, inv };

	BitmapRep();
    ~BitmapRep();

	// 2/22/97 don't know what I'm doing. Just experimenting to get a checkbox
	const void* data_;
	unsigned width_;
	unsigned height_;
	int xhot_;
	int yhot_;
	
};


#endif
