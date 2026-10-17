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
//                   	<IV-Win/Rect.h>
//
//  Defines the class for a rectangle.  The coordinate system used is the
//  right hand coordinate system typically found when dealing with world
//  coordinates (ie the coordinate system used in InterViews).  The x value 
//  increases to the right, and the y value increases to the top.  The origin 
//  is at the bottom left corner.  Note this is not the same as the coordinate 
//  system found in X-Windows and MS-Windows.
//
//  This class used to use templates and has been converted for use of the 
//  Microsoft compiler which is behind the times.
//
//  Positions are maintained as Point, and coordinate values and
//  length are maintained as Coord or PixelCoord (Since no longer 
//  parameterized).  
//
//
// 1.1
// 1997/03/28 17:35:57
//
// ========================================================================
#ifndef	Rect_h
#define	Rect_h

// ---- local includes ----
#include <IV-Win/MWpoint.h>


// ################################################################
// ##################  class MWcoordRect
// ################################################################

class MWcoordRect
{
public:
	MWcoordRect(Coord x0 = 0, Coord y0 = 0, Coord x1 = 0, Coord y1 = 0)
		: bl(x0,y0), tr(x1,y1) {}
	MWcoordRect(const MWcoordPoint& p0, const MWcoordPoint& p1) 
		: bl(p0), tr(p1) {}
	MWcoordRect(const MWcoordRect& r) : bl(r.origin()), tr(r.corner()) {}

	// ---- positions ----
	MWcoordPoint origin() const
		{ return bl; }
	MWcoordPoint origin(const MWcoordPoint& p)	
		{ return bl = p; }
	MWcoordPoint corner() const
		{ return tr; }
	MWcoordPoint corner(const MWcoordPoint& p)
		{ return tr = p; }
	MWcoordPoint topLeft() const
		{ return MWcoordPoint(bl.x(),tr.y()); }
	MWcoordPoint topCenter() const
		{ return MWcoordPoint((tr.x()+bl.x())/2,tr.y()); }
	MWcoordPoint topRight() const
		{ return tr; }
	MWcoordPoint rightCenter() const
		{ return MWcoordPoint(tr.x(),(tr.y()+bl.y())/2); }
	MWcoordPoint bottomRight() const
		{ return MWcoordPoint(tr.x(),bl.y()); }
	MWcoordPoint bottomCenter() const
		{ return MWcoordPoint((tr.x()+bl.x())/2,bl.y()); }
	MWcoordPoint bottomLeft() const
		{ return bl; }
	MWcoordPoint leftCenter() const
		{ return MWcoordPoint(bl.x(),(tr.y()+bl.y())/2); }
	MWcoordPoint center() const
		{ return MWcoordPoint((tr.x()+bl.x())/2,(tr.y()+bl.y())/2); }

	Coord left() const
		{ return bl.x(); }
	Coord right() const
		{ return tr.x(); }
	Coord top() const
		{ return tr.y(); }
	Coord bottom() const
		{ return bl.y(); }

	// ---- measurements ----
	MWcoordPoint extent() const
		{ return MWcoordPoint(tr.x()-bl.x(),tr.y()-bl.y()); }
	Coord area() const		
		{ return (tr.x()-bl.x())*(tr.y()-bl.y()); }
	Coord width() const		
		{ return tr.x()-bl.x(); }
	Coord height() const		
		{ return tr.y()-bl.y(); }
	
	// ---- operations ----
	bool operator==(const MWcoordRect&) const;
	bool operator!=(const MWcoordRect& r) const
		{ return !(*this==r); }
	MWcoordRect operator&&(const MWcoordRect&) const;		// intersection 
	MWcoordRect operator||(const MWcoordRect&) const;		// union
	void operator+=(const MWcoordPoint&);					// translate
	void operator-=(const MWcoordPoint&);
	bool contains(const MWcoordPoint&) const;
	bool contains(const MWcoordRect&) const;
	bool intersects(const MWcoordRect&) const;
	void moveTo(const MWcoordPoint&);

private:
	MWcoordPoint bl;			// bottom left corner (origin)
	MWcoordPoint tr;			// top right corner (corner)

};


inline bool MWcoordRect::operator==(const MWcoordRect& r) const
{
	return (bl==r.bl && tr==r.tr);
}

inline MWcoordRect MWcoordRect::operator&&(const MWcoordRect& r) const
{
	return MWcoordRect(bl.max(r.bl), tr.min(r.tr));
}

inline MWcoordRect MWcoordRect::operator||(const MWcoordRect& r) const
{
	return MWcoordRect(bl.min(r.bl), tr.max(r.tr));
}

inline void MWcoordRect::operator+=(const MWcoordPoint& p)
{
	bl += p;
	tr += p;
}

inline void MWcoordRect::operator-=(const MWcoordPoint& p)
{
	bl -= p;
	tr -= p;
}

inline bool MWcoordRect::contains(const MWcoordPoint& p) const
{
	return (bl <= p) && (p <= tr);
}

inline bool MWcoordRect::contains(const MWcoordRect& r) const
{
	return (contains(r.bl) && contains(r.tr));
}

inline bool MWcoordRect::intersects(const MWcoordRect& r) const
{
	if (bl.max(r.bl) <= tr.min(r.tr)) return true;
	return false;
}

inline void MWcoordRect::moveTo(const MWcoordPoint& p)
{
	tr += p-bl;
	bl = p;
}



// ################################################################
// ##################  class MWpixelRect
// ################################################################

class MWpixelRect
{
public:
	MWpixelRect(PixelCoord x0 = 0, PixelCoord y0 = 0, PixelCoord x1 = 0, 
		PixelCoord y1 = 0) : bl(x0,y0), tr(x1,y1) {}
	MWpixelRect(const MWpixelPoint& p0, const MWpixelPoint& p1) 
		: bl(p0), tr(p1) {}
	MWpixelRect(const MWpixelRect& r) : bl(r.origin()), tr(r.corner()) {}

	// ---- positions ----
	MWpixelPoint origin() const
		{ return bl; }
	MWpixelPoint origin(const MWpixelPoint& p)	
		{ return bl = p; }
	MWpixelPoint corner() const
		{ return tr; }
	MWpixelPoint corner(const MWpixelPoint& p)
		{ return tr = p; }
	MWpixelPoint topLeft() const
		{ return MWpixelPoint(bl.x(),tr.y()); }
	MWpixelPoint topCenter() const
		{ return MWpixelPoint((tr.x()+bl.x())/2,tr.y()); }
	MWpixelPoint topRight() const
		{ return tr; }
	MWpixelPoint rightCenter() const
		{ return MWpixelPoint(tr.x(),(tr.y()+bl.y())/2); }
	MWpixelPoint bottomRight() const
		{ return MWpixelPoint(tr.x(),bl.y()); }
	MWpixelPoint bottomCenter() const
		{ return MWpixelPoint((tr.x()+bl.x())/2,bl.y()); }
	MWpixelPoint bottomLeft() const
		{ return bl; }
	MWpixelPoint leftCenter() const
		{ return MWpixelPoint(bl.x(),(tr.y()+bl.y())/2); }
	MWpixelPoint center() const
		{ return MWpixelPoint((tr.x()+bl.x())/2,(tr.y()+bl.y())/2); }

	PixelCoord left() const
		{ return bl.x(); }
	PixelCoord right() const
		{ return tr.x(); }
	PixelCoord top() const
		{ return tr.y(); }
	PixelCoord bottom() const
		{ return bl.y(); }

	// ---- measurements ----
	MWpixelPoint extent() const
		{ return MWpixelPoint(tr.x()-bl.x(),tr.y()-bl.y()); }
	PixelCoord area() const		
		{ return (tr.x()-bl.x())*(tr.y()-bl.y()); }
	PixelCoord width() const		
		{ return tr.x()-bl.x(); }
	PixelCoord height() const		
		{ return tr.y()-bl.y(); }
	
	// ---- operations ----
	bool operator==(const MWpixelRect&) const;
	bool operator!=(const MWpixelRect& r) const
		{ return !(*this==r); }
	MWpixelRect operator&&(const MWpixelRect&) const;			// intersection 
	MWpixelRect operator||(const MWpixelRect&) const;			// union
	void operator+=(const MWpixelPoint&);					// translate
	void operator-=(const MWpixelPoint&);
	bool contains(const MWpixelPoint&) const;
	bool contains(const MWpixelRect&) const;
	bool intersects(const MWpixelRect&) const;
	void moveTo(const MWpixelPoint&);

private:
	MWpixelPoint bl;				// bottom left corner (origin)
	MWpixelPoint tr;				// top right corner (corner)

};

inline bool MWpixelRect::operator==(const MWpixelRect& r) const
{
	return (bl==r.bl && tr==r.tr);
}

inline MWpixelRect MWpixelRect::operator&&(const MWpixelRect& r) const
{
	return MWpixelRect(bl.max(r.bl), tr.min(r.tr));
}

inline MWpixelRect MWpixelRect::operator||(const MWpixelRect& r) const
{
	return MWpixelRect(bl.min(r.bl), tr.max(r.tr));
}

inline void MWpixelRect::operator+=(const MWpixelPoint& p)
{
	bl += p;
	tr += p;
}

inline void MWpixelRect::operator-=(const MWpixelPoint& p)
{
	bl -= p;
	tr -= p;
}

inline bool MWpixelRect::contains(const MWpixelPoint& p) const
{
	return (bl <= p) && (p <= tr);
}

inline bool MWpixelRect::contains(const MWpixelRect& r) const
{
	return (contains(r.bl) && contains(r.tr));
}

inline bool MWpixelRect::intersects(const MWpixelRect& r) const
{
	if (bl.max(r.bl) < tr.min(r.tr)) return true;
	return false;
}

inline void MWpixelRect::moveTo(const MWpixelPoint& p)
{
	tr += p-bl;
	bl = p;
}


#endif /* Rect_h */
