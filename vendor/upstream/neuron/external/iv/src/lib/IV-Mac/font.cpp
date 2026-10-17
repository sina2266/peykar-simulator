#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =========================================================================
//
//				<IV-Win/font.c>
//
// MS-Windows dependent Font representation.  The font information is stored
// in a LOGFONT structure until the font is needed, at which time it can be
// created with CreateFontIndirect().  
//
// The user is given a constructor with a LOGFONT structure to use as a
// reference.  This is primarily for flexibility where one doesn't care
// about portability (one really shouldn't use the FontRep directly).  The
// user is cautioned however that there is no guarantee the fields won't
// be overwritten (lfEscapement and lfOrientation may change for example).
//
//
// 1.1
// 1997/03/28 17:36:16
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
// ======================================================================

/*
 * PORTIONS OF THIS FILE ARE BASED UPON THE InterViews 3.1 DISTRIBUTION 
 * WHICH CARRIED THE FOLLOWING COPYRIGHT:
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


// ---- libc includes ----
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ---- InterViews includes ----
#include <InterViews/font.h>
#include <OS/string.h>
#include <OS/list.h>
#include <OS/math.h>
#include <InterViews/session.h>
#include <InterViews/display.h>
#include <IV-Mac/window.h>
#include <IV-Mac/font.h>
#include <IV-Mac/session.h>



// ---- font family name list type -----
//declarePtrList(MWfamilyPtrList, char)
//implementPtrList(MWfamilyPtrList, char)


// ------------------------------------------------------------------
// window used to determine font dimensions.  This is needed because
// character width information may be needed before any windows have
// been created by this library (This is the case with InterViews
// which builds up the entire glyph hierarchy before creating the
// window to hold it).  The desktop window is used for this purpose.
// ------------------------------------------------------------------

// ------------------------------------------------------------------
// This function delivers a default FontRep structure... presumably
// because we couldn't determine/create what was really desired and 
// we are forced to have a valid font.  
//
// This function basically lets the MS-Windows font-mapping mechanism
// get as close as it can, and uses a default point size of 12.
// ------------------------------------------------------------------
inline FontRep* defaultFontRep(const char* nm)
{
	FontRep* rep = new FontRep("Chicago", 12, FontRep::normal);
	return rep;
}

static void nofont(const char* name) {
	static bool seen = 0;
	if (!seen) {
		seen = true;
		printf("Can't find font:\n%s\nUsing default font. This is the\
 last time this message will be seen in this session", name);
		//MessageBox(NULL, buf, "Missing Font", MB_OK);
   }
   
}

// ------------------------------------------------------------------
// Preferred constructor... provides a simple interface to creating
// fonts as they are typically desired.
// ------------------------------------------------------------------

#if USE_SIOUX_WINDOW  
#include <SIOUXGlobals.h>
//extern "C" {
//	extern WindowPtr SIOUXTextWindow;	
//}
#endif

FontRep::FontRep(
	const char* face_name,			// name of typeface
	int height,                     // desired height of the font
	int style_flags)       			// desired style attributes
{
	
	size_ = height;
	Str255 name;
	name[0] = strlen(face_name);
	strcpy((char*) &name[1], face_name);
	GetFNum(name, &font_);
	if(!(font_)){
		name[0] = strlen("monaco");
		strcpy((char*) &name[1], "monaco");
		GetFNum(name, &font_);
		if(!(font_))
			font_ = 1;
		size_ = 9;
	}
	
	face_ = normal;	//default
	mode_ = srcOr;
	char_extra_ = 0;
	sp_extra_ = 0;
	
	// ---- set style ----
	if (style_flags & bold)
		face_ = bold;
	if (style_flags & italic)
		face_ = italic;
	if (style_flags & underline)
		face_ = underline;
	if (style_flags & strikeout)
		face_ = shadow;			//this isn't quite right ... needs to be changed
		
	//now must get the info
	CGrafPtr cg;
	GDHandle gd;
	GetGWorld(&cg, &gd);
	if(cg){
#if carbon
	    short save_font = GetPortTextFont(cg);
	    short save_face = GetPortTextFace(cg);
	    short save_size = GetPortTextSize(cg);
	    short save_mode = GetPortTextMode(cg);
#else
	    short save_font = cg->txFont;
	    short save_face = cg->txFace;
	    short save_size = cg->txSize;
	    short save_mode = cg->txMode;
#endif
	    
		TextFont(font_);
		TextFace(face_);
		TextSize(size_);
		TextMode(mode_);	
		GetFontInfo(&info_);
		widths_ = new int[255];
	    for (int c = 0; c < 255; ++c) {
	        widths_[c] = CharWidth(c);
	    }
	    
		TextFont(save_font);
		TextFace(save_face);
		TextSize(save_size);
		TextMode(save_mode);
	} else { 
#if USE_SIOUX_WINDOW  	
	//if the SIOUX window is being used, it must be checked to get the font 
	//before other Interviews windows are created.
#if carbon
		GrafPtr oldPort;
#else
		WindowPtr oldPort;
#endif
		
		GetPort(&oldPort);
#if carbon
		GrafPtr stp = GetWindowPort(SIOUXTextWindow->window);
		SetPort(stp);
		short save_font = GetPortTextFont(stp);
	    short save_face = GetPortTextFace(stp);
	    short save_size = GetPortTextSize(stp);
	    short save_mode = GetPortTextMode(stp);
#else
		SetPort(SIOUXTextWindow->window);
		short save_font = SIOUXTextWindow->window->txFont;
	    short save_face = SIOUXTextWindow->window->txFace;
	    short save_size = SIOUXTextWindow->window->txSize;
	    short save_mode = SIOUXTextWindow->window->txMode;
#endif
	    
		TextFont(font_);
		TextFace(face_);
		TextSize(size_);
		TextMode(mode_);	
		GetFontInfo(&info_);
		widths_ = new int[255];
	    for (int c = 0; c < 255; ++c) {
	        widths_[c] = CharWidth(c);
	    }
	    
		TextFont(save_font);
		TextFace(save_face);
		TextSize(save_size);
		TextMode(save_mode);
		SetPort(oldPort);
#else
		printf("Font information could not be found in font.c 215\n");
		exit(0);
#endif
	}

}

FontRep::~FontRep()
{
	
}
// -----------------------------------------------------------------------
// Makes sure that the font has been associated with a device, and
// associates it with the main window if it has not already been
// associated with a device.  This should be successfully called before
// any attempt is made to fetch metric data.
// -----------------------------------------------------------------------


// -------------------------------------------------------------------
// Associates the font metric data with the given device context. 
// Returns the success of finding all of the data.
// -------------------------------------------------------------------

// ----------------------------------------------------------------------
// return the width of the given character, or -1 if there is a problem.
// The widths are in terms of the display context which the font was 
// associated with.
// ----------------------------------------------------------------------

// -----------------------------------------------------------------------
// Scale the font.  This will work as a result of scaling the height in
// the LOGFONT structure because True-Type fonts are used, which will scale
// to whatever is asked for.  A side effect of scaling however, is that
// all the metrics need to be recalculated.  Since metrics are associated
// with a particular device under windows, we need a device context to use.
// -----------------------------------------------------------------------

// ###################################################################
// ##################   class Font 
// ###################################################################

// -----------------------------------------------------------------------
// The following function attempts to extract font information out of a
// string.  This is a really poor interface, and is provided only for
// compatibility with the X11 version.  The font handling really needs to
// be improved.
//
// The type of string searched for is of the following form:
//
//    *facename*style*--size*
//
// This format will be emitted by the FontFamily class when used with
// MS-Windows.  This form can also be used with X11 as it will uniquely
// match most fonts.  Any other string will cause the function to return a
// null pointer (indicating failure).
// -----------------------------------------------------------------------
static FontRep* StringToFont(const String& name)
{
	int start_mark;					// start of a range
	int end_mark;					// end of a range

	if (name[0] != '*')
		return 0;

    // ---- extract the facename ----
	if ((end_mark = name.search(1, '*')) < 0)
		return 0;
	NullTerminatedString facename = name.substr(1, end_mark - 1);

	// ---- extract the style ----
	start_mark = end_mark + 1;
	if (start_mark >= name.length())
		return 0;
	if ((end_mark = name.search(start_mark, '*')) < 0)
		return 0;
	NullTerminatedString stylename = 
		name.substr(start_mark, end_mark - start_mark);

	// ---- extract the size ----
	start_mark = end_mark + 1;
	if (start_mark >= name.length())
		return 0;
	if ((end_mark = name.search(start_mark, '*')) < 0)
		return 0;
	if (name[start_mark+1] != '-')
		return 0;
	NullTerminatedString sizename = 
		name.substr(start_mark+2, end_mark-(start_mark+2));

	// ---- create the FontRep instance ----
	int size;					// point size
	int style;					// style value
	sizename.convert(size);
	if (stylename == String("normal"))
		style = FontRep::normal;
	else if (stylename == String("bold"))
		style = FontRep::bold;
	else if (stylename == String("italic"))
		style = FontRep::italic;
	else
		style = FontRep::normal;
	FontRep* rep = new FontRep(facename.string(), size /* * 20 */, style);
	return rep;
}

// -----------------------------------------------------------------------
// The following are the constructors and destructors for the Font class.
// The string is limited to a format that StringToFont() can understand.
// The font can be scaled internally by FontRep since only TrueType fonts
// are used by FontRep.
// -----------------------------------------------------------------------
Font::Font(const String& name, float scale)
{
	printf("Font::Font(const String& name ... not implement\n");
}

Font::Font(const char* name, float scale)
{
	printf("Font::Font(const char* name ... not implement\n");
}

Font::Font(FontImpl* i)
{
	 impl_ = i;
}

Font::~Font()
{
	FontRep* rep = (FontRep*) impl_;
	delete rep;
}

// This has no meaning in the MS-Windows implimentation
void Font::cleanup()
{
}

// -----------------------------------------------------------------------
// Get/Create a font from a name.  MS-Windows has it's own mapping scheme
// that always returns a font of some sort.  It seems reasonable to provide
// a font here even if we can't exactly match, since there is a seperate
// function to determine if the font exists.
// -----------------------------------------------------------------------
const Font* Font::lookup(const String& name)
{
	return (lookup(name.string()));
}

const Font* Font::lookup(const char* name)
{
	FontRep* rep;
	//printf("font1 %s\n", name);
	if ((rep = StringToFont(String(name))) != nil)
	 {
		return new Font((FontImpl*)rep);
	}
	else
	 {
		nofont(name);
		rep = defaultFontRep(name);
		return new Font((FontImpl*)rep);
	}
}

// -----------------------------------------------------------------------
// Determine if the named font exists.  Initially, this is just a test of
// whether or not the name parses into something reasonable, but it needs
// to also be compared to the fontlist of available TrueType fonts.  There
// is no display in MS-Windows (ie no networked graphics), so it is ignored.
// -----------------------------------------------------------------------
bool Font::exists(Display*, const String& name)
{
	FontRep* rep = StringToFont(name);
	if (rep != nil)
	{
		delete rep;
		  return true;
	 }
	return false;
}

bool Font::exists(Display*, const char* name)
{
	FontRep* rep = StringToFont(String(name));
	if (rep != nil)
	{
		delete rep;
		  return true;
	 }
	return false;
}

FontRep* Font::rep(Display*) const
{
	return (FontRep*) impl_;
}

const char* Font::name() const
{
	FontRep* r = (FontRep*)impl_;
	GetFontName(r->font_, r->name_);
	r->name_[r->name_[0]+1] = '\0';
	return (const char*)(&r->name_[1]);
}

const char* Font::encoding() const
{
	printf("const char* Font::encoding not yet implemented\n");
	return nil;
}

Coord Font::size() const
{
	FontRep* rep = (FontRep*) impl_;
	return Coord(rep->size_);
}

// ---------------------------------------------------------------------------
// Get the bounding box information for the font.  The FontRep actually
// has more information than available in the FontBoundingBox, so applications
// that need more info can access FontRep info directly, but this will
// affect portability.
// ---------------------------------------------------------------------------
void Font::font_bbox(
	FontBoundingBox& b) const			// return information
{
	FontRep* rep = (FontRep*) impl_;
	if(!rep)
		printf("Font rep failure Line 401 of font.c\n");

	b.left_bearing_ = 0;
	b.right_bearing_ = 0;
	b.width_ = rep->info_.widMax;
	b.ascent_ = rep->info_.ascent;
	b.descent_ = rep->info_.descent;
	b.font_ascent_ = b.ascent_;
	b.font_descent_ = b.descent_;
}

void Font::char_bbox(
	long c, 						// character to measure
	FontBoundingBox& b) const		// return information
{
	if (c < 0)
	{
		b.left_bearing_ = 0;
		b.right_bearing_ = 0;
		b.width_ = 0;
		b.ascent_ = 0;
		b.descent_ = 0;
		b.font_ascent_ = 0;
		b.font_descent_ = 0;
		return;
	 }

	FontRep* rep = (FontRep*) impl_;
	if(!rep)
		printf("Font rep failure Line 401 of font.c\n");

	b.width_ = width(c);
	b.left_bearing_ = 0;
	b.right_bearing_ = b.width_; // guess
	b.ascent_ = rep->info_.ascent;
	b.descent_ = rep->info_.descent;
	b.font_ascent_ = b.ascent_;
	b.font_descent_ = b.descent_;
}

void Font::string_bbox(
	const char* s, 						// string to measure length of
	int len, 							// number of characters in string
	FontBoundingBox& b) const			// return information
{
	FontRep* rep = (FontRep*) impl_;
	if(!rep)
		printf("Font rep failure Line 401 of font.c\n");

	b.width_ = width(s, len);
	b.left_bearing_ = 0;
	b.right_bearing_ = b.width_; // guess (2/21/97 hines) for xvarlabels
	b.ascent_ = rep->info_.ascent;
	b.descent_ = rep->info_.descent;
	b.font_ascent_ = b.ascent_;
	b.font_descent_ = b.descent_;

}

Coord Font::width(long c) const
{
	FontRep* rep = (FontRep*) impl_;
    if ((rep == nil) || (c < 0) || (c > 255))
	{
		return 0;
    }

	return Coord(rep->widths_[c]);

}

Coord Font::width(
	const char* s, 				// string to measure
	int len) const				// number of characters in string
{
	FontRep* rep = (FontRep*) impl_;
	if(!rep)
		printf("Font rep failure Line 439 of font.c\n");
	// ---- associate with main window for metrics ----
	int i;
	Coord swidth = 0;
	for (i=0; i<len; i++)
	{
		swidth += width(s[i]);
	}
	return swidth;
}

int Font::index(
	const char* s,
	int len,
	float offset,
	bool between) const
{
	// not really tested. analogy to xfont.c
	//printf("int Font::index not yet implemented\n");
	//return 0;
	const char* p;
	int n, w;
	int coff, cw;
	
	if (offset < 0 || *s == '\0' || len == 0) {
		return 0;
	}
	int xoffset = Session::instance()->default_display()->to_pixels(Coord(offset));
	w = 0;
	for (p = s, n = 0; *p != '\0' && n < len; ++p, ++n) {
		cw = (int)width(*p);
		w += cw;
		if (w > xoffset) {
			break;
		}
	}
	coff = xoffset - w + cw;
	if (between && coff > cw/2) {
		++n;
	}
	return Math::min(n, len); 
}

