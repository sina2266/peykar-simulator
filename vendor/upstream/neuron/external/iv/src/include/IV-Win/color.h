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

// ===========================================================================
//
//                     <IV-Win/color.cpp>
//
//  MS-Windows implementation of the InterViews Color classes.
//
//
// 1.1
// 1997/03/28 17:36:00
//
// ========================================================================
#ifndef iv_win_color_h
#define iv_win_color_h

#include <InterViews/iv.h>

#include <IV-Win/MWlib.h>
class MWpalette;
class Bitmap;

class ColorRep
{
public:
	ColorRep(int r, int g, int b);
	~ColorRep();

	COLORREF msColor();						// mswin color representation

	static MWpalette* defaultPalette();		// default palette to use
	float alpha;
	ColorOp op;
	Bitmap* stipple;						// stipple pattern

	static const char* nameToRGB(const char* colormap, const char* name);
		// translates a color name to the X11 string format of an rgb
		// specification (ie #??????).  The colormap name is basically a
		// section in the colormap.ini file.

	static const Color* rgbToColor(const char* name);
		// translates an rgb string (#?????? format) to a color instance,
		// or returns null if there is a problem.  This is for support of 
		// X11 style color names.

private:
	COLORREF color;

};

inline COLORREF ColorRep::msColor()
	{ return color; }

class MWpalette
{
public:
	MWpalette();							// construct an empty palette
    MWpalette(LOGPALETTE*);					// construct from logical palette
	~MWpalette();

	COLORREF addEntry(int r, int g, int b);
		// Add an entry into the palette for the given color rgb specification.
		// If the color already exists, nothing is done, otherwise it is added
		// and a reference is returned.

	bool findEntry(int r, int g, int b, COLORREF& value);
		// Looks for the given rgb color specification in the palette.  If
		// found, true is returned and the value set to the corresponding 
		// reference.

	int realizeInto(HDC, BOOL);

private:
	class MWcolorTable* table;				// color lookup table
	HPALETTE palette; 						// the MS-Windows palette
	int numEntries;							// number of palette entries
	int palSize;							// size of the palette
};

#endif
