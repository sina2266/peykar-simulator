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
// 1997/03/28 17:36:02
//
// ========================================================================
#ifndef iv_win_font_h
#define iv_win_font_h

// ---- InterViews includes ----
#include <InterViews/iv.h>

// ---- MS-Windows includes ----
#include <IV-Win/MWlib.h>


class FontRep
{
public:
	// ---- recognized style flags ----
	enum { normal = 0, bold = 1, italic = 2, underline = 4, strikeout = 8 };

    FontRep(const char* face_name, int height, int sytle_flags = normal);
	FontRep(LOGFONT&);
    ~FontRep();

	// now storing with the font (hfont_)
	//	HFONT Create();					// create for drawing... client responsible
	HFONT	HFont();	// return the hfont_
	void Scale(float);			    // scale the font
	int Height();					// return font height
	TEXTMETRIC& Metrics();			// return font metric information
	float CharWidth(int ch);			// return width of a character
	bool AssociateWith(HDC);		// associate with a device for metrics
	const char* TypefaceName();		// name of typeface 

	void orientation(int);
		// Establish the orientation angle to render the font.  The font
		// must be true-type for this function to affect the rendering.  
		// The angle given is in tenths of a degree, 0 to 3600.  This function
		// can be called repeatedly since the actual font is created when
		// needed and then destroyed.

protected:
	bool CheckAssociation();     // makes sure metric data is available
	HWND fontWindow();				// window to get font information

protected:
	LOGFONT font_;					// font creation template
	HFONT	hfont_;					// the actual font
	TEXTMETRIC metrics;				// metric info for the font
	float* char_widths_;				// character width information

};

// ---- FontRep inline functions ----
// now storing the HFONT representation with the font itself
/*  inline HFONT FontRep::Create() */
/*  { */
/*  	return CreateFontIndirect(&font_); */
/*  } */
inline int FontRep::Height()
{
	return font_.lfHeight;
}
inline TEXTMETRIC& FontRep::Metrics()
{
	CheckAssociation();
	return metrics;
}
inline HFONT FontRep::HFont()
{
	CheckAssociation();
	return hfont_;
}
inline const char* FontRep::TypefaceName()
	{ return font_.lfFaceName; }
inline void FontRep::orientation(int angle)
	{ font_.lfOrientation = angle; }

class MWfamilyPtrList;

class FontFamilyRep
{
public:
	FontFamilyRep(const char* family);
	~FontFamilyRep();

	bool font(int size, const char* style,	// translate to font name
		const char*& name, float& scale);

protected:
	static MWfamilyPtrList name_list_;
    char* family_name_;
};


#endif  // iv_win_font_h

