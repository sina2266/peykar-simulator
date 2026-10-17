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

// =======================================================================
//
//                     <IV-Win/Cursor.h>
//
//  MS-Windows implementation of the InterViews cursor class.  
// 
//  The CursorRep class is basically a small wrapper around an MS-Widows 
//  HCURSOR.  Cursors have a lot of limitations in MS-Windows, so some 
//  differences from the X11 counterpart are unavoidable.  The biggest 
//  difference is the lack of color (or lack documentation to make them color)!
//
//
// 1.1
// 1997/03/28 17:36:01
//
// ========================================================================
#ifndef iv_win_cursor_h
#define iv_win_cursor_h

#include <IV-Win/MWlib.h>


class Color;
class Display;
class WindowVisual;

class CursorRep
{
public:
	CursorRep(int);
	CursorRep(const char*);
	CursorRep(BitmapRep& data, BitmapRep& mask);
	CursorRep(short, short, const int* data, const int* mask);
    ~CursorRep();

	HCURSOR cursorOf()
    	{ return cursor; }

private:
	HCURSOR cursor;

};


#endif
