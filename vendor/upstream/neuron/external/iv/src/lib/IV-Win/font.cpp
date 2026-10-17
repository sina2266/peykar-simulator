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
// ========================================================================

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

// ---- InterViews includes ----
#include <IV-Win/MWlib.h>
#include <InterViews/font.h>
#include <OS/string.h>
#include <OS/list.h>
#include <IV-Win/window.h>
#include <IV-Win/font.h>
#include <IV-Win/session.h>

// ---- font family name list type -----
declarePtrList(MWfamilyPtrList, char)
implementPtrList(MWfamilyPtrList, char)

MWfamilyPtrList FontFamilyRep::name_list_(10);

// ------------------------------------------------------------------
// window used to determine font dimensions.  This is needed because
// character width information may be needed before any windows have
// been created by this library (This is the case with InterViews
// which builds up the entire glyph hierarchy before creating the
// window to hold it).  The desktop window is used for this purpose.
// ------------------------------------------------------------------
HWND FontRep::fontWindow()
{
	return GetDesktopWindow();
}

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
	FontRep* rep = new FontRep(nm, 12 * 20);
	return rep;
}

static void nofont(const char* name) {
printf("nofont: |%s|\n", name);
	static bool seen = 0;
	if (!seen) {
		seen = true;
		char buf[512];
		snprintf(buf, 512, "Can't find font:\n%s\nUsing default font. This is the\
 last time this message will be seen in this session", name);
		MessageBox(NULL, buf, "Missing Font", MB_OK);
   }
}

// ------------------------------------------------------------------
// Preferred constructor... provides a simple interface to creating
// fonts as they are typically desired.
// ------------------------------------------------------------------
FontRep::FontRep(
	const char* face_name,			// name of typeface
	int height,                     // desired height of the font
	int style_flags)       			// desired style attributes
{
	char_widths_ = nil;
	hfont_ = nil;

	font_.lfWidth = 0;
	font_.lfEscapement = 0;
	font_.lfOrientation = 0;
	font_.lfWeight = 0;
	font_.lfItalic = 0;
	font_.lfUnderline = 0;
	font_.lfStrikeOut = 0;
	font_.lfCharSet = 0;
	font_.lfOutPrecision = OUT_TT_PRECIS;
	font_.lfClipPrecision = 0;
	font_.lfQuality = 0;
	font_.lfPitchAndFamily = 0;

	// ---- set style ----
	if (style_flags & bold)
		font_.lfWeight = FW_BOLD;
	if (style_flags & italic)
		font_.lfItalic = 1;
	if (style_flags & underline)
		font_.lfUnderline = 1;
	if (style_flags & strikeout)
		font_.lfStrikeOut = 1;

	// ---- required fields ----
	font_.lfHeight = height;
	strncpy(font_.lfFaceName, face_name, LF_FACESIZE-1);
	font_.lfFaceName[LF_FACESIZE-1] = 0;

}

FontRep::~FontRep()
{
  if(hfont_ != nil)
    DeleteObject(hfont_);
  hfont_ = nil;
	delete char_widths_;
}

// -----------------------------------------------------------------------
// Makes sure that the font has been associated with a device, and
// associates it with the main window if it has not already been
// associated with a device.  This should be successfully called before
// any attempt is made to fetch metric data.
// -----------------------------------------------------------------------
void dpy_setmapmode(HDC);
bool FontRep::CheckAssociation()
{
	if (char_widths_ == nil)
	{
		// has not been associated with anything yet
		HWND hwnd = fontWindow();
		HDC hdc = GetDC(hwnd);
		MWassert(hdc);
//		MWassert( SetMapMode(hdc, MM_TWIPS) );
		dpy_setmapmode(hdc);
		bool success = AssociateWith(hdc);
		ReleaseDC(hwnd, hdc);
		return success;
	}
	return true;
}

// -------------------------------------------------------------------
// Associates the font metric data with the given device context. 
// Returns the success of finding all of the data.
// -------------------------------------------------------------------
bool FontRep::AssociateWith(
	HDC hdc)						// device context to use (for printer)
{
  if(hfont_ == nil)
    hfont_ = CreateFontIndirect(&font_);

	HFONT old_fnt;
	bool status = false;

	// ---- make this font active for check ----
	if ((hfont_ != nil) &&
		((old_fnt = (HFONT) SelectObject(hdc, hfont_))!=0))
	{
		status = true;

		// ---- fetch the font metrics ----
		if (!GetTextMetrics(hdc, &metrics))
			status = false;

		// ---- fetch character width metrics ----
		if (status == true)
		  {
			int nchars = 0;
			if (char_widths_ == nil)
			{
				nchars = metrics.tmLastChar - metrics.tmFirstChar + 1;
				char_widths_ = new float[nchars];
				MWassert(char_widths_);
			}

			ABC width;
//			int	nWidth;
// char mbuf[256];
// snprintf(mbuf, 256, "%d %s", metrics.tmFirstChar, TypefaceName());
// MessageBox(NULL, mbuf, "idemo", MB_OK);
			for (int i = 0; i < nchars; i++)
			{
				if( GetCharABCWidths(hdc, metrics.tmFirstChar + i,
						metrics.tmFirstChar + i, &width) )
				{
					char_widths_ [i] =
						float(width.abcA + width.abcB + width.abcC);
				}
/*				else if (GetCharWidth(hdc, metrics.tmFirstChar + i,
						metrics.tmFirstChar + i, &nWidth) )
				{
					char_widths_ [i] = float(nWidth);
				}
*/				else
				{
					char_widths_ [i] = float(metrics.tmAveCharWidth);
				}
			}
		}

		// ---- release the font and restore original ----
		SelectObject(hdc, old_fnt);
	 }

	return status;
}

// ----------------------------------------------------------------------
// return the width of the given character, or -1 if there is a problem.
// The widths are in terms of the display context which the font was 
// associated with.
// ----------------------------------------------------------------------
float FontRep::CharWidth(int ch)
{
	// --- check it ----
	MWassert(CheckAssociation());
	ch = ch - metrics.tmFirstChar;
	int nchars = metrics.tmLastChar - metrics.tmFirstChar + 1;

	// --- fetch it ----
	if( ((ch >= 0) && (ch < nchars)) )
		return char_widths_[ch];
	else
		return 0;
}

// -----------------------------------------------------------------------
// Scale the font.  This will work as a result of scaling the height in
// the LOGFONT structure because True-Type fonts are used, which will scale
// to whatever is asked for.  A side effect of scaling however, is that
// all the metrics need to be recalculated.  Since metrics are associated
// with a particular device under windows, we need a device context to use.
// -----------------------------------------------------------------------
void FontRep::Scale(
	float s)					// scale to set to
{
	font_.lfHeight = (int) (font_.lfHeight * s);

	// ---- recalculate the metrics ----
	HWND hwnd = fontWindow();
	HDC hdc = GetDC(hwnd);
    MWassert(hdc);
//	MWassert( SetMapMode(hdc, MM_TWIPS) );
	dpy_setmapmode(hdc);
	MWassert( AssociateWith(hdc) );
	ReleaseDC(hwnd, hdc);
}

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
	FontRep* rep = new FontRep(facename.string(), size * 20, style);
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
// printf("font4 %s\n", name.string());
	FontRep* rep = StringToFont(name);
	if (! rep)
	{
	        nofont(name.string());
		NullTerminatedString nm(name);
		rep = defaultFontRep(nm.string());
	}
	MWassert(rep);

	 impl_ = (FontImpl*) rep;
	if (scale != 1.0)
	{
		rep->Scale(scale);
	}
}

Font::Font(const char* name, float scale)
{
//printf("font3 %s\n", name);
	FontRep* rep = StringToFont(String(name));
	if (! rep)
	{
	        nofont(name);
		rep = defaultFontRep(name);
	}
	MWassert(rep);

	 impl_ = (FontImpl*) rep;
	if (scale != 1.0)
	{
		rep->Scale(scale);
	}
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
	FontRep* rep;
	if ((rep = StringToFont(name)) != nil)
	 {
		return new Font((FontImpl*)rep);
	 }
	else
	{
	        nofont(name.string());
		NullTerminatedString nm(name);
		rep = defaultFontRep(nm.string());
		return new Font((FontImpl*)rep);
	}
}

const Font* Font::lookup(const char* name)
{
	FontRep* rep;
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
	 FontRep* rep = (FontRep*) impl_;
	if (rep)
		return rep->TypefaceName();
	return nil;
}

const char* Font::encoding() const
{
#ifdef IS_IMPLIMENTED
	 FontRep* f = impl_->default_rep();
	return f->encoding_ == nil ? nil : f->encoding_->string();
#endif
	return nil;
}

Coord Font::size() const
{
	// The size is stored in terms of twips, so we divide by 20 to get it
	// back to points.
	FontRep* rep = (FontRep*) impl_;
	return Coord(rep->Height()) / Coord(20);
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
	MWassert(rep);

	TEXTMETRIC& tm = rep->Metrics();

	Coord TWIPS(20.0);
	b.left_bearing_ = Coord(tm.tmOverhang) / TWIPS;
	b.right_bearing_ = Coord(tm.tmMaxCharWidth + tm.tmOverhang) / TWIPS;
	b.width_ = Coord(tm.tmMaxCharWidth) / TWIPS;
	b.ascent_ = Coord(tm.tmAscent) / TWIPS;
	b.descent_ = Coord(tm.tmDescent) / TWIPS;
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
	MWassert(rep);

	TEXTMETRIC& tm = rep->Metrics();

	Coord TWIPS(20.0);
	b.left_bearing_ = Coord(tm.tmOverhang) / TWIPS;
	b.right_bearing_ = Coord(tm.tmMaxCharWidth + tm.tmOverhang) / TWIPS;
	b.width_ = width(c);
	b.ascent_ = Coord(tm.tmAscent) / TWIPS;
	b.descent_ = Coord(tm.tmDescent) / TWIPS;
	b.font_ascent_ = b.ascent_;
	b.font_descent_ = b.descent_;
}

void Font::string_bbox(
	const char* s, 						// string to measure length of
	int len, 							// number of characters in string
	FontBoundingBox& b) const			// return information
{
    FontRep* rep = (FontRep*) impl_;
	MWassert(rep);

	TEXTMETRIC& tm = rep->Metrics();

	Coord TWIPS(20.0);
	b.left_bearing_ = Coord( tm.tmOverhang) / TWIPS;
//	b.right_bearing_ = Coord( tm.tmMaxCharWidth + tm.tmOverhang) / TWIPS;
	b.width_ = width(s, len);
//Added by hines
	b.right_bearing_ = b.width_ + Coord(tm.tmOverhang) / TWIPS;
//
	b.ascent_ = Coord(tm.tmAscent) / TWIPS;
	b.descent_ = Coord(tm.tmDescent) / TWIPS;
	b.font_ascent_ = b.ascent_;
	b.font_descent_ = b.descent_;
}

Coord Font::width(long c) const
{
    FontRep* rep = (FontRep*) impl_;
    if ((rep == nil) || (c < 0))
	{
		return 0;
    }

	return Coord(rep->CharWidth((int) c)) / Coord(20);
}

Coord Font::width(
	const char* s, 				// string to measure
	int len) const				// number of characters in string
{
    FontRep* rep = (FontRep*) impl_;
	MWassert(rep);

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
	const char*,
	int,
	float /* offset */,
	bool /* between */) const
{
#ifdef IS_IMPLIMENTED
	 const char* p;
	 int n, w;
	 int coff, cw;

	 if (offset < 0 || *s == '\0' || len == 0) {
		  return 0;
	 }
	 FontRep* f = impl_->default_rep();
	 XFontStruct* xf = f->font_;
	 int xoffset = f->display_->to_pixels(Coord(offset * f->scale_));
	 if (xf->min_bounds.width == xf->max_bounds.width) {
		  cw = xf->min_bounds.width;
		  n = xoffset / cw;
		  coff = xoffset % cw;
    } else {
        w = 0;
        for (p = s, n = 0; *p != '\0' && n < len; ++p, ++n) {
            cw = XTextWidth(xf, p, 1);
            w += cw;
            if (w > xoffset) {
                break;
            }
        }
        coff = xoffset - w + cw;
    }
    if (between && coff > cw/2) {
        ++n;
    }
	return Math::min(n, len);
#endif
return 0;
}

/* anachronisms */
#ifdef IS_IMPLIMENTED
int Font::Baseline() const {
	 FontBoundingBox b;
	 font_bbox(b);
    return impl_->default_rep()->display_->to_pixels(b.descent()) - 1;
}

int Font::Height() const {
    FontBoundingBox b;
    font_bbox(b);
    return impl_->default_rep()->display_->to_pixels(b.ascent() + b.descent());
}

int Font::Width(const char* s) const {
    return impl_->default_rep()->display_->to_pixels(width(s, strlen(s)));
}

int Font::Width(const char* s, int len) const {
    return impl_->default_rep()->display_->to_pixels(width(s, len));
}

int Font::Index(const char* s, int offset, bool between) const {
    return impl_->default_rep()->display_->to_pixels(
	index(s, strlen(s), float(offset), between)
    );
}

int Font::Index(const char* s, int len, int offset, bool between) const {
    return impl_->default_rep()->display_->to_pixels(
	index(s, len, float(offset), between)
    );
}

bool Font::FixedWidth() const {
    FontRep* f = impl_->default_rep();
    XFontStruct* xf = f->font_;
    return xf->min_bounds.width == xf->max_bounds.width;
}
#endif

// #######################################################################
// #####################   class FontFamily
// #######################################################################


// -----------------------------------------------------------------------
// constructors and destructors.  The X11 version passes a full name of a
// font back on font() queries... and the FontRep owns the storage, so we
// free up the storage of these mocked up names upon deletion.
// -----------------------------------------------------------------------
FontFamilyRep::FontFamilyRep(const char* name)
{
	// ---- record the name ----
	int len = strlen(name);
	family_name_ = new char[len + 1];
    strcpy(family_name_, name);
}

FontFamilyRep::~FontFamilyRep()
{
	int len = name_list_.count();
	int i;

	// ---- delete stored font names ----
	for (i=0; i<len; i++)
	{
		char* rm_ptr = name_list_.item(0);
		name_list_.remove(0);
        delete rm_ptr;
	}

	 delete family_name_;
}

// -----------------------------------------------------------------------
// Translate to a full font name.  This is really pretty X11 specific, so
// we just dummy up a name that could be used in the X11 world.  It is of
// the form:
//    *facename*style*--size*
// -----------------------------------------------------------------------
bool FontFamilyRep::font(
	int size,
	const char* style,
	const char*& name,
	float& scale)
{
	char buff[256];					// name creation workspace
	int i;							// index through name list
    scale = 1.0;                    // scale is unused

	// ---- generate font name ----
	snprintf(buff, 256, "*%s*%s*--%d*", family_name_, style, size);

	// ---- Search for the name in the list ----
	for (i = 0; i < name_list_.count(); i++)
	{
		if (! strcmp(buff, name_list_.item(i)))
		{
			name = name_list_.item(i);
			return true;
        }
	}

	// ---- create a new name in the list ----
	int len = strlen(buff);
	char* nm = new char[len+1];
	strcpy(nm, buff);
	name_list_.append(nm);
	name = nm;

	return true;
}

// #######################################################################
// ##########################  FontFamily class
// #######################################################################

FontFamily::FontFamily(const char* familyname)
{
    impl_ = (FontFamilyImpl*) new FontFamilyRep(familyname);
}

FontFamily::~FontFamily()
{
	FontFamilyRep* rep = (FontFamilyRep*) impl_;
    delete rep;
}

// -----------------------------------------------------------------------
// return the MS-Windows representation.  Since Windows is not a network
// based graphics system, Display is unused.
// -----------------------------------------------------------------------
FontFamilyRep* FontFamily::rep(Display*) const
{
	FontFamilyRep* f = (FontFamilyRep*) impl_;
    return f;
}

// -----------------------------------------------------------------------
// translate a desired set of font characteristics into a fontname.  This
// is a pretty X11-specific thing, but a name is dummied up for use.
// -----------------------------------------------------------------------
bool FontFamily::font(int size, const char*& name, float& scale) const
{
    return font(size, "normal", name, scale);
}

bool FontFamily::font(
	int size,
	const char* style,
	const char*& name,
	float& scale) const
{
	FontFamilyRep* rep = (FontFamilyRep*) impl_;
	return rep->font(size, style, name, scale);
}
