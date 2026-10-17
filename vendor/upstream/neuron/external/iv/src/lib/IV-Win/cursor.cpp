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
//                   cursor.c
//
// MS-Windows implementation of the InterViews cursor class.
//
//
//
// ========================================================================

#include <IV-Win/MWlib.h>
#include <InterViews/bitmap.h>
#include <InterViews/color.h>
#include <InterViews/cursor.h>
#include <InterViews/session.h>
#include <IV-Win/cursor.h>
#include <IV-Win/bitmap.h>
#include <IV-Win/window.h>
#include <IV-Win/MWapp.h>

#include <stdio.h>

static const CursorPattern textPat = {
    0x0000, 0x4400, 0x2800, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x2800, 0x4400, 0x0000
};

static const CursorPattern textMask = {
    0x0000, 0xCC00, 0x7800, 0x3000, 0x3000, 0x3000, 0x3000, 0x3000, 
	 0x3000, 0x3000, 0x3000, 0x3000, 0x3000, 0x7800, 0xCC00, 0x0000,
};

static const CursorPattern noPat = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

Cursor* defaultCursor;
Cursor* arrow;
Cursor* crosshairs;
Cursor* ltextCursor;
Cursor* rtextCursor;
Cursor* hourglass;
Cursor* upperleft;
Cursor* upperright;
Cursor* lowerleft;
Cursor* lowerright;
Cursor* noCursor;

/*
 * Define the builtin cursors.
 */

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ > __SIZEOF_LONG__
#define cp2int (int)(unsigned long long)
#elif defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ > __SIZEOF_INT__
#define cp2int (int)(unsigned long)
#else
#define cp2int (int)
#endif

void Cursor::init() 
{
	//
	// The MS include file define MAKEINTRESOURCE to cast to a string, which
	// is not the integer that it really is (which we expect as a constructor
	// argument.  Therefore, we cast the IDC_XXX values to an int.
	//
	 arrow = new Cursor(cp2int IDC_ARROW);
	crosshairs = new Cursor(cp2int IDC_CROSS);
	 ltextCursor = new Cursor(4, 8, textPat, textMask);
    rtextCursor = new Cursor(0, 8, textPat, textMask);
    hourglass = new Cursor(cp2int IDC_WAIT);
    upperleft = new Cursor(cp2int IDC_SIZENWSE);
    upperright = new Cursor(cp2int IDC_SIZENESW);
	lowerleft = new Cursor(cp2int IDC_SIZENESW);
    lowerright = new Cursor(cp2int IDC_SIZENWSE);
    noCursor = new Cursor(0, 0, noPat, noPat);
    defaultCursor = arrow;
}

// -----------------------------------------------------------------------
// constructors and destructors for the Cursor class.
// -----------------------------------------------------------------------

/*
 * Create a cursor a specific pattern and mask (16x16 in size)
 */
Cursor::Cursor(
	short xoff,
	short yoff,
	const int* p,
	const int* m,
	const Color*,					// foreground color
	const Color *)                  // background color
{
    rep_ = new CursorRep(xoff, yoff, p, m);
}

/*
 * Create a cursor from bitmaps.
 */

Cursor::Cursor(
	const Bitmap* pat,
	const Bitmap* mask,         
	const Color* ,			   	// foreground color - unsupported
	const Color* )              // background color - unsupported
{
	rep_ = new CursorRep(*pat->rep(), *mask->rep());
}

/*
 * Create a cursor from a font.
 */
Cursor::Cursor(
	const Font*,                   // font to use
	int ,                          // data character
	int ,                          // mask character
	const Color* ,                 // foreground color
	const Color* )                 // background color
{
	// !!!! currently unsupported !!!!!
	WindowRep::errorMessage("TBD - cursor from a font");
}

/*
 * Create a cursor from the predefined cursor font.
 */
Cursor::Cursor(
	int n,                        // resource id
	const Color*,                 // foreground color
	const Color*)                 // background color
{
    rep_ = new CursorRep(n);
}

Cursor::Cursor(
	const char* n)
{
	rep_ = new CursorRep(n);
}

Cursor::~Cursor()
{
    delete rep_;
}

// #######################################################################
// ##################  class CursorRep
// #######################################################################

CursorRep::CursorRep(int id)
{
	// ---- load an user defined cursor resource ----

// MODIFIED by NL for WIN32s:
//		must use NULL as the HINSTANCE, when trying to load standard
//		windows cursors.
	cursor = LoadCursor(NULL, MAKEINTRESOURCE(id));

// OLD CODE
//	char buff[25];
//	snprintf(buff, 25, "#%d", id);
//	HINSTANCE inst = theApp.hinst;
//	cursor = LoadCursor(inst, buff);
}

CursorRep::CursorRep(const char* id)
{
		// ---- load a predefined cursor ----
		cursor = LoadCursor(NULL, id);
}

CursorRep::CursorRep(
	BitmapRep& data,
	BitmapRep& mask)
{
	HINSTANCE inst = theApp.hinst;
	cursor = CreateCursor(inst, data.hot_x_, data.hot_y_, data.bm_.bmWidth,
		data.bm_.bmHeight, mask.bm_.bmBits, data.bm_.bmBits);
}

CursorRep::CursorRep(
	short hot_x,
	short hot_y,
	const int* data,
	const int* mask)
{
	HINSTANCE inst = theApp.hinst;
	int cx = GetSystemMetrics(SM_CXCURSOR);
	int cy = GetSystemMetrics(SM_CYCURSOR);
	int n = cx*cy/16;
	short* m = new short[n];
	short* d = new short[n];
	int i, j;
	for (i=0, j=0; i < 16; i++) {
		m[j] = mask[i]; m[j+1] = 0xffff;
		d[j] = data[i]; d[j+1] = 0;
		j += 2;
	}
	while (j < n) {
		m[j] = 0xffff;
		d[j] = 0;
		j++;
	}
	cursor = CreateCursor(inst, hot_x, hot_y, /*16*/ cx,
		/*16*/ cy, m, d);
	delete [] m;
	delete [] d;
}

CursorRep::~CursorRep()
{
char buf[100];
	DestroyCursor(cursor);
}

