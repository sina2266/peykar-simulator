/*
Copyright (C) 2002 Michael Hines
This file contains programs and data originally developed by Michael Hines

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

/*
 * MacPrinter - generate output for a mswindows printer. Derived from the
 * Mac version.
 */

#ifndef iv_mcprinter_h
#define iv_mcprinter_h

#include <InterViews/printer.h>

class MacPrinterCanvas;

#include <ivstream.h>

class MacPrinter : public Printer {
public:
	 MacPrinter(std::ostream* o = nil);
	 virtual ~MacPrinter();
	virtual Window* window() const;
	 virtual Coord width() const;
	 virtual Coord height() const;
	 virtual PixelCoord pwidth() const;
	 virtual PixelCoord pheight() const;

	 virtual void transformer(const Transformer&);
	 virtual const Transformer& transformer() const;

	 virtual void resize(Coord left, Coord bottom, Coord right, Coord top);

	virtual bool get();
	 virtual void prolog(const char* creator = "InterViews");
	 void prolog(float scale);
	 virtual void epilog();
    virtual void setup(bool b = true);

    virtual void comment(const char*);
	 virtual void page(const char*);

	 virtual void push_transform();
	 virtual void transform(const Transformer&);
	 virtual void pop_transform();

	 virtual void push_clipping(bool all = false);
	 virtual void clip();
	 virtual void pop_clipping();

	 virtual void new_path();
	 virtual void move_to(Coord x, Coord y);
	 virtual void line_to(Coord x, Coord y);
	 virtual void curve_to(
		  Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
	 );
	 virtual void close_path();
	 virtual void stroke(const Color*, const Brush*);
	 virtual void fill(const Color*);

    virtual void character(
        const Font*, long c, Coord width, const Color*, Coord x, Coord y
    );
    virtual void stencil(const Bitmap*, const Color*, Coord x, Coord y);
    virtual void image(const Raster*, Coord x, Coord y);
private:
	MacPrinterCanvas* c;
};

#endif
