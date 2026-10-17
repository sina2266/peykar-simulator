// =========================================================================
//
//				<IV-Win/font.h>
//
// MS-Windows implementation of the InterViews Font class.  
//
// The font information is stored in a LOGFONT structure until the font is 
// needed, at which time it can be created with CreateFontIndirect().  Some 
// functions require a device context to get information (such as metrics), 
// so the desktop window is used for such requests.  
//
// The user is given a constructor with a LOGFONT structure to use as a
// reference.  This is primarily for flexibility where one doesn't care
// about portability (one really shouldn't use the FontRep directly).  The
// user is cautioned however that there is no guarantee the fields won't
// be overwritten (lfEscapement and lfOrientation may change for example).
//
//
// 1.1
// 1997/03/28 17:35:43
//
// Windows 3.1/NT InterViews Port 
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
#ifndef iv_win_font_h
#define iv_win_font_h

// ---- InterViews includes ----
#include <InterViews/iv.h>

class FontRep
{

public:

	FontRep(const char* face_name, int height, int style_flags);
	~FontRep();
	
	
	void defaultFontRep(const char* nm);

	// ---- recognized style flags ----
	enum { normal = 0, bold = 1, italic = 2, underline = 4, strikeout = 8 };

	short font_;		//font id
	int face_;		//extra typing ... bold, underline, ect.
	short size_;		//size for text
	Fixed sp_extra_;	//expanding the standard space
	short char_extra_;  //expanding space between characters
	short mode_;			//printing mode
	int* widths_;
	FontInfo info_;
	Str255 name_;
};

class FontFamilyRep
{

};


#endif  // iv_win_font_h

