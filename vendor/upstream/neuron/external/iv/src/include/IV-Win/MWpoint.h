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
//                   <IV-Win/MWpoint.h>
//
//  Defines the class for a point.  This class used to use templates and has
//  been converted for use of the Microsoft compiler which is behind the times.
//
//
// 1.1
// 1997/03/28 17:35:56
//
// ========================================================================
#ifndef	Point_h
#define	Point_h

#include <InterViews/iv.h>
#include <InterViews/coord.h>
#include <OS/math.h>

// #################################################################
// ##############  class MWcoordPoint
// #################################################################

class MWcoordPoint
{
public:
	MWcoordPoint()				
		{ xpos = ypos = 0; }
	MWcoordPoint(Coord newx, Coord newy)
		{ xpos=newx; ypos=newy; }
	Coord x() const
		{ return xpos; }
	Coord x(Coord newx)
		{ return xpos = newx; }
	Coord y() const
		{ return ypos; }
	Coord y(Coord newy)
		{ return ypos = newy; }

	// ---- arithmetic operations ----
	MWcoordPoint operator+(const MWcoordPoint& p) const	
		{ return MWcoordPoint(xpos+p.xpos, ypos+p.ypos); }
	MWcoordPoint operator-() const			
		{ return MWcoordPoint(-xpos,-ypos); }
	MWcoordPoint operator-(const MWcoordPoint& p) const	
		{ return MWcoordPoint(xpos-p.xpos, ypos-p.ypos); }
	friend MWcoordPoint operator*(const MWcoordPoint& p, int i) 
		{ return MWcoordPoint(i*p.xpos, i*p.ypos); }
	friend MWcoordPoint operator*(int i, const MWcoordPoint& p) 
		{ return MWcoordPoint(i*p.xpos, i*p.ypos); }
	int operator*(const MWcoordPoint& p) const	
		{ return (int) (xpos*p.xpos + ypos*p.ypos); }

	// ---- logical operations ----
	bool operator==(const MWcoordPoint& p) const
		{ return (xpos==p.xpos && ypos==p.ypos); }
	bool operator!=(const MWcoordPoint& p) const
		{ return (xpos!=p.xpos || ypos!=p.ypos); }
	bool operator<(const MWcoordPoint& p) const
		{ return (ypos<p.ypos && xpos<p.xpos); }
	bool operator<=(const MWcoordPoint& p) const
		{ return (ypos<=p.ypos && xpos<=p.xpos); }
	bool operator>(const MWcoordPoint& p) const
		{ return (ypos>p.ypos && xpos>p.xpos); }
	bool operator>=(const MWcoordPoint& p) const
		{ return (ypos>=p.ypos && xpos>=p.xpos); }

	// ---- assignment operators ----
	void operator+=(const MWcoordPoint& p)		
		{ xpos += p.xpos; ypos += p.ypos; }
	void operator-=(const MWcoordPoint& p)
		{ xpos -= p.xpos; ypos -= p.ypos; }
	void operator*=(float s)			
		{ xpos *= s; ypos *= s; }

	// ---- relationship ----
	MWcoordPoint max(const MWcoordPoint&) const;
	MWcoordPoint min(const MWcoordPoint&) const;
	MWcoordPoint transpose() const
		{ return MWcoordPoint(ypos,xpos); }
	bool isBelow(const MWcoordPoint& p) const
		{ return ypos < p.ypos; }
	bool isAbove(const MWcoordPoint& p) const
		{ return ypos > p.ypos; }
	bool isLeft(const MWcoordPoint& p) const
		{ return xpos < p.xpos; }
	bool isRight(const MWcoordPoint& p) const
		{ return xpos > p.xpos; }

private:
	Coord xpos;									// x coordinate
	Coord ypos;           							// y coordinate
};

inline MWcoordPoint MWcoordPoint::max(const MWcoordPoint& p) const
{
	return MWcoordPoint(Math::max(xpos,p.xpos),Math::max(ypos,p.ypos));
}

inline MWcoordPoint MWcoordPoint::min(const MWcoordPoint& p) const
{
	return MWcoordPoint(Math::min(xpos,p.xpos),Math::min(ypos,p.ypos));
}

// #################################################################
// ##############  class MWpixelPoint
// #################################################################

class MWpixelPoint
{
public:
	MWpixelPoint()				
		{ xpos = ypos = 0; }
	MWpixelPoint(PixelCoord newx, PixelCoord newy)
		{ xpos=newx; ypos=newy; }
	PixelCoord x() const
		{ return xpos; }
	PixelCoord x(PixelCoord newx)
		{ return xpos = newx; }
	PixelCoord y() const
		{ return ypos; }
	PixelCoord y(PixelCoord newy)
		{ return ypos = newy; }

	// ---- arithmetic operations ----
	MWpixelPoint operator+(const MWpixelPoint& p) const	
		{ return MWpixelPoint(xpos+p.xpos, ypos+p.ypos); }
	MWpixelPoint operator-() const			
		{ return MWpixelPoint(-xpos,-ypos); }
	MWpixelPoint operator-(const MWpixelPoint& p) const	
		{ return MWpixelPoint(xpos-p.xpos, ypos-p.ypos); }
	friend MWpixelPoint operator*(const MWpixelPoint& p, int i) 
		{ return MWpixelPoint(i*p.xpos, i*p.ypos); }
	friend MWpixelPoint operator*(int i, const MWpixelPoint& p) 
		{ return MWpixelPoint(i*p.xpos, i*p.ypos); }
	int operator*(const MWpixelPoint& p) const	
		{ return xpos*p.xpos + ypos*p.ypos; }

	// ---- logical operations ----
	bool operator==(const MWpixelPoint& p) const
		{ return (xpos==p.xpos && ypos==p.ypos); }
	bool operator!=(const MWpixelPoint& p) const
		{ return (xpos!=p.xpos || ypos!=p.ypos); }
	bool operator<(const MWpixelPoint& p) const
		{ return (ypos<p.ypos && xpos<p.xpos); }
	bool operator<=(const MWpixelPoint& p) const
		{ return (ypos<=p.ypos && xpos<=p.xpos); }
	bool operator>(const MWpixelPoint& p) const
		{ return (ypos>p.ypos && xpos>p.xpos); }
	bool operator>=(const MWpixelPoint& p) const
		{ return (ypos>=p.ypos && xpos>=p.xpos); }

	// ---- assignment operators ----
	void operator+=(const MWpixelPoint& p)		
		{ xpos += p.xpos; ypos += p.ypos; }
	void operator-=(const MWpixelPoint& p)
		{ xpos -= p.xpos; ypos -= p.ypos; }
	void operator*=(float s)			
		{ xpos = PixelCoord(xpos * s); ypos = PixelCoord(ypos * s); }

	// ---- relationship ----
	MWpixelPoint max(const MWpixelPoint&) const;
	MWpixelPoint min(const MWpixelPoint&) const;
	MWpixelPoint transpose() const
		{ return MWpixelPoint(ypos,xpos); }
	bool isBelow(const MWpixelPoint& p) const
		{ return ypos < p.ypos; }
	bool isAbove(const MWpixelPoint& p) const
		{ return ypos > p.ypos; }
	bool isLeft(const MWpixelPoint& p) const
		{ return xpos < p.xpos; }
	bool isRight(const MWpixelPoint& p) const
		{ return xpos > p.xpos; }

private:
	PixelCoord xpos;									// x coordinate
	PixelCoord ypos;           							// y coordinate
};

inline MWpixelPoint MWpixelPoint::max(const MWpixelPoint& p) const
{
	return MWpixelPoint(Math::max(xpos,p.xpos),Math::max(ypos,p.ypos));
}

inline MWpixelPoint MWpixelPoint::min(const MWpixelPoint& p) const
{
	return MWpixelPoint(Math::min(xpos,p.xpos),Math::min(ypos,p.ypos));
}

#endif /* Point_h */
