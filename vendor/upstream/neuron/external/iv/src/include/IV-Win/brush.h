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

// ==========================================================================
//
//                       <IV-Win/brush.h>
//
//
// MS-Windows implementation of the InterViews Brush class
//
// If a programmer that needs full access to the windows capabilities and
// can afford portability problems, this class can be manipulated directly
// to control the pen and brush used in the canvas.  The way to do this is:
//
// 	a) create the Brush instance.
//  b) fill in the BrushRep values desired.
//  c) use the Brush.
//
//
// 1.1
// 1997/03/28 17:35:58
//
// ========================================================================
#ifndef iv_win_brush_h
#define iv_win_brush_h

#include <IV-Win/MWlib.h>

class BrushRep
{
public:
	DWORD* dashList;
    int dashCount;
	Coord penWidth;
};

#endif
