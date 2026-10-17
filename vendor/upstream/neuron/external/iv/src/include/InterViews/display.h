/*
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

/*
 * Display -- workstation screen(s) and input devices
 */

#ifndef iv_display_h
#define iv_display_h

#include <InterViews/geometry.h>

#include <InterViews/_enter.h>

class Event;
class DisplayRep;
class Handler;
class SelectionManager;
class String;
class Style;
class Window;

class Display {
protected:
    Display(DisplayRep*);
public:
#ifdef WIN32
	 virtual void rescale();
#endif
    static Display* open(const String&);
    static Display* open(const char*);
    static Display* open();
    virtual void close();
    virtual ~Display();

    virtual int fd() const;
    virtual Coord width() const;
    virtual Coord height() const;
    virtual PixelCoord pwidth() const;
    virtual PixelCoord pheight() const;
    virtual Coord a_width() const;
    virtual Coord a_height() const;

    PixelCoord to_pixels(Coord, DimensionName d = Dimension_X) const;
    Coord to_coord(PixelCoord, DimensionName d = Dimension_X) const;
		// device/world coordinate system conversions.  The to_pixels()
		// function converts printer points to pixels, and the to_coord()
		// function converts pixels to printer points.  The dimension argument
		// is an extension of the InterViews 3.1 distribution which doesnt
		// distinguish between the X and Y dimensions.  The dimension defaults
		// to the x dimension to provide backward compatibility (although
		// potentially incorrect behavior).

    virtual bool defaults(String&) const;
    virtual void style(Style*);
    virtual Style* style() const;

    virtual void set_screen(int);

    virtual void repair();
    virtual void flush();
    virtual void sync();

    virtual bool get(Event&);
    virtual void put(const Event&);
    virtual bool closed();

    virtual void grab(Window*, Handler*);
    virtual void ungrab(Handler*, bool all = false);
    virtual Handler* grabber() const;
    virtual bool is_grabbing(Handler*) const;

    virtual void ring_bell(int);
    virtual void set_key_click(int);
    virtual void set_auto_repeat(bool);
    virtual void set_pointer_feedback(int thresh, int scale);
    virtual void move_pointer(Coord x, Coord y);

    virtual SelectionManager* primary_selection();
    virtual SelectionManager* secondary_selection();
    virtual SelectionManager* clipboard_selection();
    virtual SelectionManager* find_selection(const char*);
    virtual SelectionManager* find_selection(const String&);

    DisplayRep* rep() const;
private:
    DisplayRep* rep_;
    Coord x_pixel_;
    Coord y_pixel_;
    Coord x_point_;
    Coord y_point_;
};

inline PixelCoord Display::to_pixels(Coord c, DimensionName d) const 
{
    return PixelCoord( c * ((d == Dimension_X) ? x_point_ : y_point_ ) + 
                           ((c > 0) ? 0.5 : -0.5) );
}

inline Coord Display::to_coord(PixelCoord p, DimensionName d) const 
{ 
	return Coord(p) * ((d == Dimension_X) ? x_pixel_ : y_pixel_); 
}

inline DisplayRep* Display::rep() const { return rep_; }

#include <InterViews/_leave.h>

#endif
