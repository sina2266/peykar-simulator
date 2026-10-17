#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =======================================================================
//
//                     <IV-Mac/display.c>
//
//
// 1.2
// $Date:   4 Aug 1996 
//
// =======================================================================

#include <InterViews/display.h>
#include <InterViews/style.h>
#include <InterViews/resource.h>
#include <InterViews/event.h>
#include <InterViews/session.h>
#include <IV-Mac/window.h>
#include <OS/math.h>
#include <stdio.h>
#if !carbon
#include <sound.h> // SysBeep got moved
#endif

class DisplayRep
{
public:
	DisplayRep();
	~DisplayRep();

public:
	
	PixelCoord pwidth_;
	PixelCoord pheight_;
	Coord width_;
	Coord height_;
	Style* style_;
};


DisplayRep::DisplayRep()
{
	style_ = nil;
}

DisplayRep::~DisplayRep()
{
}


// #######################################################################
// ################# class Display
// #######################################################################

extern Style* iv_display_style_;

Display::Display(DisplayRep* d)
{
	 rep_ = d;
	 
	 x_point_ = 1;
	 y_point_ = 1;
	 x_pixel_ = 1;
	 y_pixel_ = 1;
	 
	 // ---- determine display size ----
	 GDHandle 	theGraphicsDevice = GetMainDevice();
	 Rect 		theScreen = (*theGraphicsDevice)->gdRect;	 
	 
	 rep_->pwidth_ =  (theScreen.right) - (theScreen.left);
	 rep_->pheight_ = (theScreen.bottom) - (theScreen.top);;
	 rep_->width_ = to_coord( rep_->pwidth_, Dimension_X);
	 rep_->height_ = to_coord( rep_->pheight_, Dimension_Y);
	 
	 
}

Display* Display::open(const String&)
{
	return open((const char*) 0);
}

Display* Display::open()
{
	 return open(nil);
}

Display* Display::open(const char*)
{
	DisplayRep* d = new DisplayRep;
	return new Display(d);
}

void Display::close()
{
}

Display::~Display()
{
    DisplayRep* d = rep();
	//Resource::unref_deferred(d->style_);
	delete d;
}

int Display::fd() const { return 0; }
Coord Display::width() const { return rep()->width_; }
Coord Display::height() const { return rep()->height_; }
PixelCoord Display::pwidth() const { return rep()->pwidth_; }
PixelCoord Display::pheight() const { return rep()->pheight_; }

/*
 * Convert millimeters to points.  We use 72.0 pts/in and 25.4 mm/in.
 */
// This function is probably unecessary for Macintosh port
static inline double mm_to_points(double mm)
{
    return (72.0 / 25.4) * mm;
}

Coord Display::a_width() const
{
	DisplayRep& d = *rep();
	return d.width_;
}

Coord Display::a_height() const {
    DisplayRep& d = *rep();
	return d.height_;
}

bool Display::defaults(String&) const
{
	return false;
}

void Display::style(Style* s)
{
  	DisplayRep& d = *rep();
    Resource::ref(s);
    Resource::unref(d.style_);
    d.style_ = s; 
}
    
Style* Display::style() const
{
	return rep()->style_;
}

void Display::set_screen(int) //Not implemented in MS-windows version ... not done here
{
}

void Display::repair() //Not implemented in MS-windows version ... not done here
{
}

void Display::flush() //Not implemented in MS-windows version ... not done here
{
}

void Display::sync() //Not implemented in MS-windows version ... not done here
{
}

void Display::ring_bell(int)
{
	SysBeep(30);
}

void Display::set_key_click(int) //Not implemented in MS-windows version ... not done here
{
}

void Display::set_auto_repeat(bool) //Not implemented in MS-windows version ... not done here
{
}

void Display::set_pointer_feedback(int, int) //Not implemented in MS-windows version ... not done here
{
}

void Display::move_pointer(
	Coord,                            // x coordinate
	Coord)                            // y coordinate
{
	// although you can move the pointer... it's only useful when there
	// is no mouse attached.  The instant the mouse is touched the pointer
	// goes back to where it was.  An assertion failure is done if there
	// is an attempt to warp the pointer since this really isn't portable
	// to Windows.
    //MWassert(0);
}

SelectionManager* Display::primary_selection()
{
    return nil; //find_selection("PRIMARY");
}

SelectionManager* Display::secondary_selection()
{
    return nil; //find_selection("SECONDARY");
}

SelectionManager* Display::clipboard_selection()
{
    return nil; //find_selection("CLIPBOARD");
}

SelectionManager* Display::find_selection(const char*)
{
    return nil; //find_selection(String(name));
}

SelectionManager* Display::find_selection(const String&)
{
	return nil;
}


/*
 * Read the next event if one is pending.  Otherwise, return false.
 * Window::receive will be called on the target window for the event,
 * if the window is known and is valid.  Because we don't keep track
 * of subwindows, it is possible to get an event for a subwindow after
 * the main window has been unmapped.  We must ignore such events.
 */

bool Display::get(Event&)
{
	return false;
}

void Display::put(const Event&)
{
  
}

/*
 * Check to see if the display connection just shut down.
 */

bool Display::closed()
{
	return false;
}

/*
 * Add a handler to the grabber list.  The handler is ref'd to ensure
 * that it is not deallocated while on the list.
 */

void Display::grab(Window*, Handler* h)
{
	Event e;
    e.grab(h);
}

/*
 * Remove a handler from the grabber list.
 * This function has no effect if the handler is not presently on the list.
 * If the handler is on the list, it is unref'd.
 */

void Display::ungrab(Handler* h, bool)
{
	Event e;
    e.ungrab(h);
}
/*
 * Return the most recent grabber, or nil if the list is empty.
 */

Handler* Display::grabber() const
{
	Event e;
    return e.grabber();
}

/*
 * Check whether a given handler is on the grabber list.
 */

bool Display::is_grabbing(Handler* h) const
{
	Event e;
    return e.is_grabbing(h);
}

