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
//                     <IV-Win/window.h>
//
//  MS-Windows implimentation of the InterViews Window classes.
//
//  The WndProc callback procedure is used to dispatch the received messages
//  to their associated WindowRep instance.  The common messages have a
//  corresponding function in the WindowRep class.  Those messages not
//  directly turned into function calls have the opportunity to be trapped
//  by the Window::receive function... which gets an event with all of
//  the MS-Windows message data embedded into it.
//
//  The ManagedWindowRep is associated with the ManagedWindow class, which
//  is a class of window that interacts with the window manager.  Although
//  there is no seperate "Window Manager" in MS-Windows, the functionality
//  present in that class has equivalents in MS-Windows.
//
//
// 1.1
// 1997/03/28 17:36:05
//
// ========================================================================
#ifndef iv_win_window_h
#define iv_win_window_h

#include <InterViews/iv.h>
#include <InterViews/geometry.h>

#include <IV-Win/MWlib.h>

class Window;
class Bitmap;
class Handler;
class ManagedWindow;
class Style;
class Glyph;
class Cursor;
class Event;

class MWpalette;
class MWcontrol;
class MWcontrolTable;
class MWcontrolHWND;
class MWcreateParams;
class MWcursorPtrList;

class MWwindow
{
public:
	MWwindow();
	virtual ~MWwindow();

	// ---- typical window operations ----
	void bind();                     		// bind to an MS-Windows window
	void unbind();							// unbind from MS-Windows window
	bool bound();						// bound to MS-Windows window?
	bool map();							// map the window to the display
	void unmap();							// unmap window from display
	bool isMapped();						// is window visible?

	// ---- window creation parameters ----
	void windowTitle(const char*);
	void widthOf(int);
	void heightOf(int);
	void xposOf(int);
	void yposOf(int);
	void parentOf(HWND);
	void styleOf(DWORD);
	void styleOfEx(DWORD);
	void idOf(int);

	 // ----- window class parameters ----
	void windowClass(const char*);
	void classStyle(UINT);


	// The following function is the entry point from message callbacks.  It
	// redistributes the messages into other member functions.  This function
	// can be overriden as long as the base class is called to
	 // translate any untrapped messages.
	virtual long WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	void registerClass(HINSTANCE);	 		// register Windows class
											//   (if needed).
	HWND msWindow();						// associated MS-Windows window

protected:
	HWND hwnd;								// associated MS-Windows window

private:
	MWcreateParams* params;					// creation parameters

protected:
	void resize(unsigned int w, unsigned int h);

};

// The following are the window/class creation parameters.  They are only
// available until the window gets mapped (which is the time that they are
// actually used).  Since the window doesn't get mapped until after
// construction, derived classes are free to change the window parameters
// in their constructor and guarantee their effect.  A constructor and
// destructor are provided even though used as a structure, since this
// guarantees creation defaults and freeing of sub-allocated storage.
class MWcreateParams
{
public:
	MWcreateParams();
	~MWcreateParams();

	// ---- window creation parameters ----
	void titleOf(const char*);      // set window title
	 const char* titleOf();          // fetch window title
	UINT width;                     // width of the window
	UINT height;                    // height of the window
	UINT x;                         // x-coordinate screen position
	UINT y;							// y-coordinate screen position
	HWND parent;					// window parent (if any)
	DWORD style;					// window style attributes
	DWORD styleEx;					// extended window style attributes
	int id;							// control/menu id

	// ---- class registration parameters ----
	void classnameOf(const char*);  // set window classname
	const char* classnameOf();      // fetch window classname
	UINT classStyle;				// style of the class

private:
	char* title;                    // title of window
	char* classname;          		// classname to register

};

// ---- inline functions of MWwindow ----
inline bool MWwindow::bound()
	{ return (hwnd) ? 1 : 0; }
inline HWND MWwindow::msWindow()
	{ return hwnd; }
inline void MWwindow::windowTitle(const char* t)
	{ MWassert(params); params->titleOf(t); }
inline void MWwindow::widthOf(int w)
	{ MWassert(params); params->width = w; }
inline void MWwindow::heightOf(int h)
	{ MWassert(params); params->height = h; }
inline void MWwindow::parentOf(HWND p)
	{ MWassert(params); params->parent = p; }
inline void MWwindow::windowClass(const char* c)
	{ MWassert(params); params->classnameOf(c); }
inline void MWwindow::styleOf(DWORD s)
	{ MWassert(params); params->style = s; }
inline void MWwindow::styleOfEx(DWORD s)
	{ MWassert(params); params->styleEx = s; }
inline void MWwindow::xposOf(int xpos)
	{ MWassert(params); params->x = xpos; }
inline void MWwindow::yposOf(int ypos)
	{ MWassert(params); params->y = ypos; }
inline void MWwindow::classStyle(UINT sty)
	{ MWassert(params); params->classStyle = sty; }
inline void MWwindow::idOf(int id)
	{ MWassert(params); params->id = id; }


class MWchild
{
public:

	virtual void receive(Event&) = 0;
		// Child controls/menus receive events through this function.
};


class EXPORT WindowRep : public MWwindow
{
public:
	WindowRep(Window*);
	~WindowRep();

	// The following function is the entry point from message callbacks.  It
	// redistributes the messages into other member functions.  This function
	// can be overriden as long as the base class is called to
	 // translate any untrapped messages.
	virtual long WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	// ---- Windows messages recognized ----
	virtual long WMpaint(WPARAM wParam, LPARAM lParam);
	virtual long WMinput(WPARAM wParam, LPARAM lParam, int type, int button);
	virtual long WMsize(WPARAM wParam, LPARAM lParam);

	long childEvent(long id, Event& e);
	long childEventHWND(HWND h, Event& e);
		// distribute command/other messages to the C++ object that correponds
		// to the child-window control/menu item that is trying to tell us
		// something.  The id is the identification of the control, and the
		// event contains the message information.

	void registerControl(long id, MWchild*);
	void registerControlHWND(HWND, MWchild*);
		// Register a predefined control, or custom control (really any
		// child window) with this window.  This allows the window to route
		// child window callbacks to their C++ control object which handles
		// it as appropriate.


	static HWND mainWindow();				// handle of main (first) window
	static void errorMessage(const char*);	// preliminary error handler

	Window* ivWindowOf()                    // associated IV window
		{ return win; }
	void bind();       					 	// bind to MS-Window & canvas

	void doubleBuffer(bool);
		// Turn double-buffering on if true, off if false.

	void frontBuffer();
		// Set the window to be rendered upon in immediate mode.  The
		// double buffered setting will have no effect when the front
		// buffer is set active.

	void backBuffer();
		// Set the window to be rendered only upon receipt of WMpaint
		// messages.  If double-buffered, that will take effect when
		// the back buffer is set active.

public:
	MWcursorPtrList* cursor_stack_;
	Cursor* cursor_;
	UINT left_;
	UINT bottom_;
	bool request_on_resize_;

protected:

	void rebuffer();
		// rebuilds the memory-based image buffer used for double-buffering
		// the canvas display.

private:
	Window* win;					// associated InterViews window
	MWpalette* palette;				// palette associated with window
	MWcontrolTable* controls;		// hash table of registered controls
	MWcontrolHWND* cntrlHWND;		// hash table of registered control handles

	bool doubleBuffered;			// is window double-buffered?
	HBITMAP back_bitmap_;			// bitmap associated with back buffer
	HDC frontBuff;					// front buffer device context
	bool offscreen_;	// WM_MOVE needs to redraw if portion
   							// was off screen
};

inline void WindowRep::doubleBuffer(bool b)
	{ doubleBuffered = b; }

// #######################################################################
// ################## ManagedWindowRep
// #######################################################################
class ManagedWindowRep
{
public:
	ManagedWindowRep(ManagedWindow*);
	~ManagedWindowRep();

	// ---- Windows messages recognized ----
	long WMminmax(WPARAM wParam, LPARAM lParam);
	long WMclose(WPARAM wParam, LPARAM lParam);

private:
	ManagedWindow* win;					// associated window

public:
	Handler* wm_delete_;
	 ManagedWindow* icon_;
	 Bitmap* icon_bitmap_;
	 Bitmap* icon_mask_;
	 Window* group_leader_;
	 Window* transient_for_;

	 //void do_set(Window*, HintFunction);

};

#endif
