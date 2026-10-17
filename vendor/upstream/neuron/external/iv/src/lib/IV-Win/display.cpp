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
//                     <IV-Win/display.c>
//
//
// ========================================================================

#include <IV-Win/MWlib.h>
#include <InterViews/display.h>
#include <InterViews/style.h>
#include <InterViews/resource.h>
#include <InterViews/event.h>
#include <InterViews/session.h>
#include <IV-Win/MWlib.h>
#include <OS/math.h>
#include <stdio.h>
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
static int pixel_twips_;
static int logpixelsx_, logpixelsy_;
Coord display_xsize_, display_ysize_;
extern Style* iv_display_style_;
static float fudge_;
//#include "/nrn/src/winio/debug.h"

static void set_pixel_twips(HDC hdc) {
#if 1
//	int pw = GetDeviceCaps(hdc, HORZRES); // width of screen in pixels
//	int ph = GetDeviceCaps(hdc, VERTRES); // height of screen in pixels
	int pw = GetSystemMetrics(SM_CXVIRTUALSCREEN); // width of screen in pixels
	int ph = GetSystemMetrics(SM_CYVIRTUALSCREEN); // width of screen in pixels
	logpixelsx_ = GetDeviceCaps(hdc, LOGPIXELSX);//pixels/logical inch
	logpixelsy_ = -GetDeviceCaps(hdc, LOGPIXELSY);//pixels/logical inch
	display_xsize_ = Coord(pw)/logpixelsx_*72.; // screen width in points
	display_ysize_ = Coord(ph)/logpixelsy_*72.; // screen height in points
	pixel_twips_ = 1440; // a twip is 1/20th of a point
	fudge_ = 1.;
#else
	logpixelsx_ = GetDeviceCaps(hdc, LOGPIXELSX);//pixels/logical inch
	logpixelsy_ = -GetDeviceCaps(hdc, LOGPIXELSY);//pixels/logical inch
//	DebugMessage("logpixelsx logpixelsy %d %d\n", logpixelsx_,
//	logpixelsy_);
//	DebugMessage("HORZSIZE VERTSIZE %d %d\n",
//   GetDeviceCaps(hdc, HORZSIZE), GetDeviceCaps(hdc, VERTSIZE));
	Coord x1, x2;
	POINT p;
	p.x = 72 * 20;
	p.y = 72 * 20;
	MWassert(SetMapMode(hdc, MM_ANISOTROPIC));
	MWassert(SetWindowExtEx(hdc, 1440, 1440, NULL));
	MWassert(SetViewportExtEx(hdc, logpixelsx_, logpixelsy_, NULL));
	MWassert( LPtoDP(hdc, &p, 1) );
	x1 = Coord(p.x)/72.;
	p.x = 72 * 20;
	p.y = 72 * 20;
	MWassert(SetMapMode(hdc, MM_TWIPS));
	MWassert( LPtoDP(hdc, &p, 1) );
	x2 = Coord(p.x)/72.;
	fudge_ = x1/x2;
	display_xsize_ = Coord(GetDeviceCaps(hdc, HORZRES)) * 72./Coord(p.x);
	display_ysize_ = Math::abs(Coord(GetDeviceCaps(hdc, VERTRES)) * 72./Coord(p.y));
//DebugMessage("HORZRES = %d   VERTRES = %d\n",
//GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES));
	double mswin_scale = 1.0;
	pixel_twips_ = 1440;
	pixel_twips_ = int(1440.*1200./double(GetDeviceCaps(hdc, HORZRES)));
	Style* s = iv_display_style_;
	if (s->find_attribute("mswin_scale", mswin_scale)) {
		pixel_twips_ = int(1440. * mswin_scale + .01);
	}else if (GetDeviceCaps(hdc, HORZRES) < 900) {
		char buf[512];
      const char* il = Session::installLocation();
      if (!il) il = "";
		snprintf(buf, 512, "This is a low resolution screen and there \
is no mswin_scale resource in %s\\app-defa\\intervie.\n\
Select \"Yes\" to choose\n\
	*mswin_scale: %g\n\
for this session. \"No\" to choose a scale of 1.0",
			il,
			pixel_twips_/1440.);
		if (IDNO == MessageBox(NULL, buf, "*mswin_scale: x",
			MB_YESNO))
		{
			pixel_twips_ = 1440;
		}
	}else{
		pixel_twips_ = 1440;
	}
#endif
}

int iv_mswin_to_pixel(int winpix) {
	return int(double(winpix)*double(pixel_twips_)/1440. + .01);
}
int iv_pixel_to_mswin(int pixel) {
	return int(double(pixel)*1440./double(pixel_twips_) + .01);
}

void dpy_setmapmode(HDC hdc) {
// if the MM_TWIPS map mode is used then on a dual screen
// with windows 2000, the pixels/point is twice what it should be.
  if (0 && pixel_twips_ == 1440) {
	MWassert(SetMapMode(hdc, MM_TWIPS));
	return;
  }else{
	MWassert(SetMapMode(hdc, MM_ANISOTROPIC));
	MWassert(SetWindowExtEx(hdc, pixel_twips_, pixel_twips_, NULL));
	MWassert(SetViewportExtEx(hdc, logpixelsx_, logpixelsy_, NULL));
  }
}

extern void iv_rescale_map();
extern void iv_rescale_unmap();

void iv_display_scale(float scale) {
	Display*d = Session::instance()->default_display();
	if (0 && scale <= 1.01) {
		pixel_twips_ = 1440;
	}else{
		float scl = scale * fudge_;
		pixel_twips_ = int(1440. * scl);
	}
	iv_rescale_unmap();
	d->rescale();
	iv_rescale_map();
}

void iv_display_scale(Coord x, Coord y) { // scale so fits onto screen
	float x1 = x/display_xsize_;
	float y1 = y/display_ysize_;
	float scale = ((x1 > y1) ? x1 : y1);
	scale = (scale < 1.01) ? 1. : scale;
	iv_display_scale(scale);
}

void Display::rescale() {
	HDC hdc = GetDC(NULL);
	MWassert(hdc);
	dpy_setmapmode(hdc);
	POINT p;
	p.x = 72 * 20;
	p.y = 72 * 20;
	MWassert( LPtoDP(hdc, &p, 1) );

	x_point_ = Coord(p.x) / Coord(72.0);
	y_point_ = (Coord) Math::abs( Coord(p.y) / Coord(72.0) );
	x_pixel_ = Coord(72.0) / Coord(p.x);
	y_pixel_ = (Coord) Math::abs( Coord(72.0) / Coord(p.y) );
	// ---- determine display size ----
//	rep_->pwidth_ = GetDeviceCaps(hdc, HORZRES);
//	rep_->pheight_ = GetDeviceCaps(hdc, VERTRES);
	rep_->pwidth_ = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	rep_->pheight_ = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	rep_->width_ = to_coord( rep_->pwidth_, Dimension_X);
	rep_->height_ = to_coord( rep_->pheight_, Dimension_Y);
	ReleaseDC(NULL, hdc);
//	DebugMessage("rescale pw ph w h %d %d %g %g\n",
//	rep_->pwidth_, rep_->pheight_, rep_->width_, rep_->height_);
}

Display::Display(DisplayRep* d)
{
	 rep_ = d;
//DebugMessage("Display::Display\n");
	HDC hdc = GetDC(NULL);
	MWassert(hdc);
//	MWassert( SetMapMode(hdc, MM_TWIPS) );
	set_pixel_twips(hdc);
	dpy_setmapmode(hdc);
	//
	// set conversion factors, these are different between the X and Y
	// dimension generally (and definitely in MS-Windows), so this is
	// handled differently than the InterViews distribution.  The logical
	// to device point conversion function is used to convert an inch worth
	// of printer points from page space to device space to determine what
	// the scale factors are.
	//
	POINT p;
	p.x = 72 * 20;
	p.y = 72 * 20;
	MWassert( LPtoDP(hdc, &p, 1) );

	x_point_ = Coord(p.x) / Coord(72.0);
	y_point_ = (Coord) Math::abs( Coord(p.y) / Coord(72.0) );
	x_pixel_ = Coord(72.0) / Coord(p.x);
	y_pixel_ = (Coord) Math::abs( Coord(72.0) / Coord(p.y) );
	// ---- determine display size ----
//	rep_->pwidth_ = GetDeviceCaps(hdc, HORZRES);
//	rep_->pheight_ = GetDeviceCaps(hdc, VERTRES);
	rep_->pwidth_ = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	rep_->pheight_ = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	rep_->width_ = to_coord( rep_->pwidth_, Dimension_X);
	rep_->height_ = to_coord( rep_->pheight_, Dimension_Y);
	ReleaseDC(NULL, hdc);
//	DebugMessage("Display pw ph w h %d %d %g %g\n",
//	rep_->pwidth_, rep_->pheight_, rep_->width_, rep_->height_);
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
	Resource::unref_deferred(d->style_);
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

void Display::set_screen(int)
{
}

void Display::repair()
{
}

void Display::flush()
{
}

void Display::sync()
{
}

void Display::ring_bell(int)
{
#if 0
	MessageBeep(-1);
#else
	// since arg is unsigned int avoid warning even though MS
	// recommends -1
	MessageBeep(~0);
#endif
}

void Display::set_key_click(int)
{
}

void Display::set_auto_repeat(bool)
{
}

void Display::set_pointer_feedback(int, int)
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
    MWassert(0);
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
	MWassert(0);
	return false;
}

void Display::put(const Event&)
{
    MWassert(0);
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

