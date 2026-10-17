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
//						<IV-Win/raster.c>
//
// MS-Windows implementation of the InterViews Raster class. 
//
// A memory device context is used to store the raster.  The peek and poke
// operations simply get translated to calls that get and set pixel values
// the the memory device context.
//
// 1.1
// 1997/03/28 17:36:03
//
// ========================================================================
#ifndef ivwin_raster_h
#define ivwin_raster_h

#include <IV-Win/MWlib.h>

class RasterRep
{
public:
	RasterRep(HDC hdc);
	~RasterRep();

	HDC deviceContext() const;

	unsigned long width_;
	unsigned long height_;
		// dimensions of the display surface... a query with GetDeviceCaps
		// returns the incorrect numbers for a memory-based display context.

private:
	HDC hdcMem;
};

inline HDC RasterRep::deviceContext() const
	{ return hdcMem; }

#endif /* ivwin_raster_h */

