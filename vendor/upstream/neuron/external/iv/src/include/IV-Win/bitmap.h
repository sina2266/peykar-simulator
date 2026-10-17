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
//                            <IV-Win/bitmap.h>
//
// MS-Windows implementation of the InterViews bitmap class.  
//
// The windows data structure BITMAP is used until the bitmap is needed 
// for actual rendering in the canvas, at which time a temporary GDI object 
// is created.  Operations on this bitmap are therefore very light-weight.
//
//
// 1.1
// 1997/03/28 17:35:58
//
// ========================================================================
#ifndef iv_win_bitmap_h
#define iv_win_bitmap_h

// ---- InterViews includes ----
#include <InterViews/coord.h>
#include <InterViews/iv.h>

// ---- windows includes ----
#include <windows.h>

class BitmapRep
{
public:
	friend class MWcanvas32;
	friend class MWcanvas16;
	friend class CursorRep;

    enum { copy, fliph, flipv, rot90, rot180, rot270, inv };

	BitmapRep();
	BitmapRep(const void*, unsigned int, unsigned int, int, int);
	BitmapRep(const BitmapRep&);
    ~BitmapRep();

	// ---- load from a resource ----
	bool Load(const char* name);

    // ---- should phase this stuff out ------
	Coord left_;
    Coord bottom_;
    Coord right_;
    Coord top_;
    Coord width_;
	Coord height_;

	// ---- data access functions ----
	virtual void poke(bool set, int x, int y);		// set or clear a bit
	virtual bool peek(int x, int y) const;           // test a bit

	unsigned int pwidth()								// width in pixels
		{ return bm_.bmWidth; }
	unsigned int pheight()								// height in pixels
		{ return bm_.bmHeight; }

protected:
	void SyncSize();			// sync size fields to bitmap size

protected:
	// ---- windows bitmap representation ----
	BITMAP bm_;					// logical bitmap representation

	int hot_x_;					// x-coordinate of hot spot (for cursors)
	int hot_y_;					// y-coordinate of hot spot (for cursors)

};


#endif
