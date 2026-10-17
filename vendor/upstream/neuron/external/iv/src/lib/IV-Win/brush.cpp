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

// =======================================================================
//
//   <IV-Win/brush.c>
//
//
// ========================================================================

/*
 * THIS FILE CONTAINS PORTIONS OF THE InterViews 3.1 DISTRIBUTION THAT 
// This media contains programs and data which are proprietary
// to Tim Prinzing.
//
// These contents are provided under a Tim Prinzing software source
// license, which prohibits their unauthorized resale or distribution 
// outside of the buyer's organization.
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

#include <IV-Win/MWlib.h>
#include <InterViews/brush.h>
#include <IV-Win/brush.h>


// -----------------------------------------------------------------------
// constructors and destructors
// -----------------------------------------------------------------------
Brush::Brush(Coord w)
{
	init(nil, 0, w);
}

Brush::Brush(
	const int* p,			// pattern dash/space pixel counts
	int c,					// number of elements in the pattern array
	Coord w)				// width of the brush
{
	init(p, c, w);
}

Brush::Brush(
	int pat,				// 16-bit specification of dash/space pattern
	Coord w)				// width of the brush
{
    int dash[16];
    int count;

    calc_dashes(pat, dash, count);
    init(dash, count, w);
}

Brush::~Brush()
{
	BrushRep* r = rep_;
	delete r;
}

void Brush::calc_dashes(int pat, int* dash, int& count) 
{
    unsigned int p = pat & 0xffff;

    if (p == 0 || p == 0xffff) 
	{
        count = 0;
    } 
	else 
	{
		const unsigned int MSB = 1 << 15;
		while ((p & MSB) == 0) 
		{
			p <<= 1;
		}

		if (p == 0x5555 || p == 0xaaaa) 
		{
			dash[0] = 1;
			dash[1] = 3;
			count = 2;
		} 
		else if (p == 0xaaaa) 
		{
			dash[0] = 1;
			dash[1] = 1;
			count = 2;
		} 
		else if (p == 0xcccc) 
		{
			dash[0] = 2;
			dash[1] = 2;
			count = 2;
		} 
		else if (p == 0xeeee) 
		{
			dash[0] = 3;
			dash[1] = 1;
			count = 2;
		} 
		else 
		{
			unsigned int m = MSB;
			int index = 0;
			while (m != 0) 
			{
				/* count the consecutive one bits */
				int length = 0;
				while (m != 0 && (p & m) != 0) 
				{
					++length;
					m >>= 1;
				}
				dash[index++] = length;

				/* count the consecutive zero bits */
				length = 0;	
				while (m != 0 && (p & m) == 0) 
				{
					++length;
					m >>= 1;
				}
				if (length > 0) 
				{
					dash[index++] = length;
				}
			}
			count = index;
		}
    }
}

void Brush::init(const int* pattern, int count, Coord w) 
{
	rep_ = new BrushRep;

	// ---- fill in the values for solid brush ----
	rep_->penWidth = w;
    rep_->dashCount = count;
    if (count > 0) 
	{
        rep_->dashList = new DWORD[count];
        for (int i = 0; i < count; ++i) 
		{
            rep_->dashList[i] = pattern[i];
        }
    } 
	else 
	{
        rep_->dashList = nil;
    }
}

BrushRep* Brush::rep(Display*) const
{
    return rep_;
}

Coord Brush::width() const
{
	return 	rep_->penWidth;
}

int Brush::dash_count() const { return rep_->dashCount; }

int Brush::dash_list(int i) const { return int(rep_->dashList[i]); }

