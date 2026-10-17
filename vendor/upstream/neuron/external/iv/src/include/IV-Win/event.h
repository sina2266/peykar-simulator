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
//                     <IV-Win/Event.h>
//
//  MS-Windows implementation of the InterViews Event classes.  
//
//  The flow of control is significantly different from X11 in MS-Windows.
//  Messages come from a callback rather than read from a socket, and the
//  messages are distributed to a WindowRep object.  Therefore, the events
//  are actually created by a WindowRep-based object which then turns
//  around and calls EventRep::handleCallback(), which occurs as a result
//  of calling Event::handle() (which is what Session::run() does).  This
//  should support modified event loops that programmers wrote rather
//  than using Session::run().  
//
//  WARNING - Any assumption that the event is valid after a Session::read()
//            is guaranteed to be incorrect since the event is actually
//            filled out in the callback.  The only safe thing to do after
//            a read is eventually call Event::handle()!!
//
//
//  The EventRep stores the type and button information directly as the
//  values used by the Event class.  The modifier_ field contains keycode
//  information... MK_? if mouse event and VK_? if keyboard event.  All
//  information is stashed directly in the EventRep with no translation
//  unless the information is asked for through the Event class.  This is
//  to facilitate rapid creation since a large number of these will be
//  created (especially mouse motion events).
//
//
// 1.1
// 1997/03/28 17:36:01
//
// ========================================================================
#ifndef iv_win_event_h
#define iv_win_event_h

// ---- InterViews includes ----
#include <InterViews/iv.h>
#include <InterViews/coord.h>
#include <InterViews/window.h>

// ---- MS-Windows includes ----
#include <IV-Win/MWlib.h>

class EventRep
{
public:
	int typeOf();						// event type
	int buttonOf();						// mouse button
	bool isShift();					// is shift key pressed
	bool isControl();				// is control key pressed
	unsigned char keycode();			// key pressed

	Window* windowOf() const;
		// window associated with this event (message).  

 	void windowSet(Window* w);	// set the window pointer: need this if you're faking an event
 	// and the Event::window(Window*) call is apparently bad and therefore unsupported

	PixelCoord windowXpos() const;
	PixelCoord windowYpos() const;
		// fetch window-relative mouse pointer coordinates.  These are
		// in terms of the InterViews coordinate system (not MS-Windows).
		// The pointer position is saved, and can be queried at any time,
		// independently of the message type of the event.

	PixelCoord displayXpos() const;
	PixelCoord displayYpos() const;
		// fetch display-relative mouse pointer coordinates.  These are
		// in terms of the InterViews coordinate system (not MS-Windows).
		// The pointer position is saved, and can be queried at any time,
		// independently of the message type of the event.

	void setMessage(Window*, UINT, WPARAM, LPARAM);
		// establish the current message and parameters for this event
		// record.

	UINT messageOf() const;
	WPARAM wparamOf() const;
	LPARAM lparamOf() const;
		// fetch the raw message data.  If an alternative form of the data
		// is available, it should be used to maximize portibility between
		// win16 and win32.

	static void handleCallback(Event&);
		// This function is called by WindowRep as a result an earlier call
		// to Event::handle().

public:

	LRESULT result;						
		// message processing return value.  This is the return value for
		// the WndProc() function that was registered as the callback function
		// for the MS-Window messages.  This will be initialized to zero with
		// each message placed into this record for processing.  If a return
		// other than zero is desired, it is up to the function processing
		// the message to place a different value in this field.

	MSG msg;
		// Current MS-Windows message being processed.  This is basically
		// to support alternative event loops... ie via read() and handle()
		// calls.

private:
	friend class WindowRep;				// this class fills in the event info
	int type_;                          // Event::type enum
	int button_;                        // Event::button enum

	Window* window_;

	UINT message_;
	WPARAM wparam_;
	LPARAM lparam_;
		// These values are the message and it's parameters passed to the 
		// WndProc() function that was registered as the callback function
		// for the MS-Window messages.  The meaning of these is defined by
		// Microsoft as part of their API.

	WORD ptrX_;
	WORD ptrY_;
		// window relative pointer position.  This is the value of the 
		// pointer position in the last WM_MOUSEMOVE message.  This is needed
		// to keep the pick requests working when a handler is being searched
		// for.
};

// ---- inline functions for EventRep ----
inline int EventRep::typeOf()
	{ return type_; }
inline int EventRep::buttonOf()
	{ return button_; }
inline bool EventRep::isShift()
	{ return (wparam_ & MK_SHIFT) ? 1 : 0; }
inline bool EventRep::isControl()
	{ return (wparam_ & MK_CONTROL) ? 1 : 0; }
inline UINT EventRep::messageOf() const
	{ return message_; }
inline WPARAM EventRep::wparamOf() const
	{ return wparam_; }
inline LPARAM EventRep::lparamOf() const
	{ return lparam_; }
inline Window* EventRep::windowOf() const
	{ return window_; }
inline void EventRep::windowSet(Window* w)
	{ window_ = w; }
inline PixelCoord EventRep::windowXpos() const
	{ return ptrX_ > 32768 ? ptrX_ - 0x10000 : ptrX_; }
inline PixelCoord EventRep::windowYpos() const
	{ Canvas* c = window_->canvas(); return (c->pheight() - ((ptrY_ > 32768)? ptrY_ - 0x10000 : ptrY_)); }


#endif
