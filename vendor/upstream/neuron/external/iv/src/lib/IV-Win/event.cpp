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
//                     <IV-Win/event.c>
//
//  MS-Windows implementation of the InterViews Event classes.  The
//  flow of control is significantly different from X11 in MS-Windows.
//  Messages come from a callback rather than read from a socket, and the
//  messages are distributed to a WindowRep object.  Therefore, the events
//  are actually created by a WindowRep-based object which then turns
//  around and calls Event::handle(), which is what Session::run did after
//  it read an event from the socket.  The end result should be identical,
//  unless an application called anything other than Session::run to do
//  it's event dispatching.
//
//
// ========================================================================

#include <IV-Win/MWlib.h>
#include <InterViews/resource.h>
#include <InterViews/event.h>
#include <InterViews/window.h>
#include <InterViews/handler.h>
#include <InterViews/canvas.h>
#include <InterViews/display.h>
#include <IV-Win/event.h>
#include <IV-Win/window.h>
#include <OS/list.h>

declarePtrList(MWhandlerPtrList, Handler)
implementPtrList(MWhandlerPtrList, Handler)

// ---- grabbing event handler list ----
static MWhandlerPtrList grabberList(100);



Event::Event()
{
	rep_ = new EventRep;
}

Event::Event(const Event& e)
{
	rep_ = new EventRep;
	copy_rep(e);
}

Event::~Event()
{
    if (rep_)
		delete rep_;
}

Event& Event::operator =(const Event& e)
{
    copy_rep(e);
	return *this;
}

void Event::copy_rep(const Event& e)
{
    *rep_ = *e.rep_;
}

void Event::display(Display*)
{  
}

Display* Event::display() const
{ 
	return rep_->windowOf()->display(); 
}

void Event::window(Window*)
{
	// This is unsupported under the MS-Windows version.  When the event
	// was received, it was already known exactly which window the event
	// was for, and changing it is dangerous.  Supporting this call caused
	// strange behavior, so it has been changed to do nothing!!
}

Window* Event::window() const
{ 
	return rep_->windowOf(); 
}

// -----------------------------------------------------------------------
// The following operations deal with events coming from a UNIX style file
// (actually socket) where events can be read, pushed back, or tested to
// see if any new ones are pending.  These have only minimal support under
// MS-Windows and somewhat different.  The event cannot be manipulated
// after a read, handle must be called.
//
// These should not be used anyway... the programmer should be running
// things with Session::run() and using the Dispatch class for any other
// forms of event handling.
// -----------------------------------------------------------------------
bool Event::pending() const
{
	WindowRep::errorMessage("Event::pending - unsupported");
	return false;
}

void Event::read()
{
	WindowRep::errorMessage("Event::read - unsupported");
	//Session::instance()->read(*this);
}

bool Event::read(long , long )
{
	Event::read();
	//return Session::instance()->read(s, u, *this);
    return false;
}

void Event::unread()
{
	WindowRep::errorMessage("Event::unread - unsupported");
	//rep()->display_->put(*this);
}

void Event::poll()
{
	// used to query the mouse position, which could be synthsized, but
	// for now it's unimplimented... I consider this a rather unclean
	// interface to begin with.
	WindowRep::errorMessage("Event::poll - unsupported");
}

// -----------------------------------------------------------------------
// Find an event handler for the window associated with this event.
// -----------------------------------------------------------------------
Handler* Event::handler() const
{
    Handler* h = nil;
	Window* w = rep_->windowOf();
	if (w != nil)
	{
		h = w->target(*this);
    }
    return h;
}

// -----------------------------------------------------------------------
// Go process this event with the current handler.  In the MS-Windows 
// version, this is not done directly.  The EventRep::handlerCallback()
// does the actual work, but this function queues up the event (really
// MS-Windows message) so that it will come through the WndProc().
//
// The event must be the one used the Session::read() (or Event::read()),
// so that the message is sitting in the MSG structure.  
// -----------------------------------------------------------------------
void Event::handle()
{
	TranslateMessage(&(rep_->msg));
	DispatchMessage(&(rep_->msg));
}

// -----------------------------------------------------------------------
// event handler grabs
// -----------------------------------------------------------------------
void Event::grab(Handler* h) const
{
	// ---- push on grabber stack ----
    Resource::ref(h);
	grabberList.append(h);
}

void Event::ungrab(Handler* h) const
{
	// ---- remove from collection of grabbers ----
    for (ListUpdater(MWhandlerPtrList) i(grabberList); i.more(); i.next()) 
	{
	 	Handler* curr_h = i.cur();
        if (curr_h == h) 
		{
            i.remove_cur();
            Resource::unref(h);
            break;
        }
    }
}

Handler* Event::grabber() const
{ 
    if (grabberList.count() == 0)
		return nil;
	return grabberList.item(0);
}

bool Event::is_grabbing(Handler* h) const
{
	for (ListItr(MWhandlerPtrList) i(grabberList); i.more(); i.next())
	{
	 	Handler* curr_h = i.cur();
        if (curr_h == h) 
		{
			return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------
// Return the type of event.  Since all the work is done at the time the
// event is synthesized, this simply returns the event type that was
// stashed in the EventRep.
// -----------------------------------------------------------------------
EventType Event::type() const
{
	return rep_->typeOf();
}

unsigned long Event::time() const
{
	return GetMessageTime();
}

// -----------------------------------------------------------------------
// The pointer locations have already been determined, so we simply return
// them here.
// -----------------------------------------------------------------------
Coord Event::pointer_x() const
{
	Display* dpy = display();
	return dpy->to_coord(rep_->windowXpos(), Dimension_X);
}

Coord Event::pointer_y() const
{
	Display* dpy = display();
	return dpy->to_coord(rep_->windowYpos(), Dimension_Y);
}

Coord Event::pointer_root_x() const
{
	Display* dpy = display();
	return dpy->to_coord(rep_->displayXpos(), Dimension_X);
}

Coord Event::pointer_root_y() const
{
	Display* dpy = display();
	return dpy->to_coord(rep_->displayYpos(), Dimension_Y);
}


EventButton Event::pointer_button() const
{
	return rep_->buttonOf();
}

unsigned int Event::keymask() const
{
	WindowRep::errorMessage("Event::keymask() - unsupported");
    return 0;
}


// -------------------------------------------------------------------------
// key tests - It is presumed that this query is being made in response 
// to some key/pointer event, in which case the wParam of the message would
// hold valid information. 
// -------------------------------------------------------------------------
bool Event::control_is_down() const
	{ return (rep_->wparamOf() & MK_CONTROL) ? true : false; }
bool Event::shift_is_down() const
	{ return (rep_->wparamOf() & MK_SHIFT) ? true : false; }
bool Event::left_is_down() const
	{ return (rep_->wparamOf() & MK_LBUTTON) ? true : false; }
bool Event::middle_is_down() const
	{ return (rep_->wparamOf() & MK_MBUTTON) ? true : false; }
bool Event::right_is_down() const
	{ return (rep_->wparamOf() & MK_RBUTTON) ? true : false; }

bool Event::capslock_is_down() const
	{ return false; }
bool Event::meta_is_down() const
	{ return false; }

unsigned char Event::keycode() const
{
	if (rep_->typeOf() == key)
		return rep_->keycode();
	else
		return 0;
}

// from 0x20 to 0x2F
unsigned int Event::mapkey(char* buff, unsigned int bufflen) const
{
	if (bufflen)
	{
		buff[0] = (char)rep_->wparamOf();
		return 1;
	}
	return 0;
}

unsigned long Event::keysym() const
{
	// TBD
	MWassert(0);
	return 0;
}

// #######################################################################
// ########################## class EventRep
// #######################################################################

void EventRep::setMessage(Window* w, UINT m, WPARAM wp, LPARAM lp)
{
	// ---- stash raw data ----
	window_ = w;
	message_ = m;
	wparam_ = wp;
	lparam_ = lp;

	// ---- check for pointer position ----
	if (message_ == WM_MOUSEMOVE)
	{
		ptrX_ = LOWORD(lparam_);
		ptrY_ = HIWORD(lparam_);
	}
}

PixelCoord EventRep::displayXpos() const
{
	POINT pt;								// point to be converted.
	HWND h = window_->rep()->msWindow();	// handle of MS-Windows window
	pt.x = (ptrX_ > 32768) ? ptrX_- 0x10000 : ptrX_;							// x in client coords
	pt.y = ptrY_;							// y in client coords
	ClientToScreen(h, &pt);
	 
	return  pt.x;
}

PixelCoord EventRep::displayYpos() const
{
	POINT pt;								// point to be converted
	HWND h = window_->rep()->msWindow();	// handle of MS-Windows window
	pt.x = ptrX_;							// x in client coords
	pt.y = (ptrY_ > 32768) ? ptrY_ - 0x10000 : ptrY_;							// x in client coords
	ClientToScreen(h, &pt);

	Display* d = window_->display();
	MWassert(d);
	int height = d->pheight();
		 
	return (height - pt.y);
}

unsigned char EventRep::keycode()
{
	return wparam_;
}

// -----------------------------------------------------------------------
// This function used to be Event::handle(), but the functionality is
// really a callback in the MS-Windows version because that's the way
// Windows likes to operate.  When a message comes in for a window that
// is use input related, this function gets called, which invokes the
// current handler.
// -----------------------------------------------------------------------
void EventRep::handleCallback(Event& e)
{
	Handler* h = e.grabber();

	if (h == nil)
	{
		h = e.handler();
    }
	if (h != nil)
	{
		bool b = Resource::defer(true);
		h->ref();
		h->event(e);
		h->unref();
		Resource::flush();
		Resource::defer(b);
    }
}
