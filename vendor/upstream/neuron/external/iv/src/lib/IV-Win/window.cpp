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
//                     <IV-Win/Window.c>
//
//  MS-Windows implementation of the InterViews Window classes.  The
//  WndProc callback procedure is used to dispatch the received messages
//  to their associated WindowRep instance.  The common messages have a
//  corresponding function in the WindowRep class.  Those messages not
//  directly turned into function calls have the opportunity to be trapped
//  by reimplimenting the WindowRep::WndProc function which handles any
//  unrecognized messages.
//
//
// ========================================================================

/*
 * THIS FILE CONTAINS PORTIONS OF THE InterViews 3.1 DISTRIBUTION THAT 
 * CONTAINED THE FOLLOWING COPYRIGHT:
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
#include <InterViews/glyph.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <InterViews/canvas.h>
#include <InterViews/event.h>
#include <InterViews/handler.h>
#include <InterViews/hit.h>
#include <InterViews/box.h>
#include <InterViews/cursor.h>
#include <InterViews/color.h>
#include <InterViews/display.h>
#include <InterViews/action.h>
#include <OS/list.h>
#include <OS/string.h>
#include <OS/table.h>
#include <IV-Win/window.h>
#include <IV-Win/canvas.h>
#include <IV-Win/event.h>
#include <IV-Win/cursor.h>
#include <IV-Win/color.h>
#include <IV-Win/session.h>
#include <IV-Win/MWapp.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
// #include "\nrn\src\winio\debug.h"
#include <stdio.h>
// ---- templates ----
declarePtrList(MWcursorPtrList, Cursor)
implementPtrList(MWcursorPtrList, Cursor)
declareTable(MWcontrolTable, long, MWchild*)
implementTable(MWcontrolTable, long, MWchild*)
declareTable(MWcontrolHWND, HWND, MWchild*)
implementTable(MWcontrolHWND, HWND, MWchild*)


// --- global event for rapid processing ----
static Event* input_e = 0;
static EventRep* input_er = 0;

// Names of the classes that get registered by InterViews applications.
const char* WINDOW_CLASSNAME = "ivWindow";
const char* POPUP_CLASSNAME = "ivPopupWindow";
const char* TOPLEVEL_CLASSNAME = "ivTopLevelWindow";

// Properties used to hold the pointer to the C++ window object that is
// associated with the MS-Windows window.
const char* PROP_PTR = "mw_ptr";

// #######################################################################
// ################## MS-Window to C++ window object translation
// #######################################################################

// -----------------------------------------------------------------------
// This is the callback function through which MS-Windows communicates
// back to the application.  We extract a pointer to the associated
// InterViews Window objects from two properties stored in the MS-Window.
// If the properties can't be found, then the window doesn't have a
// c++ object associated with it.
//
// -----------------------------------------------------------------------

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ > __SIZEOF_LONG__
#define cp2int (unsigned long long)
#else
#define cp2int /**/
#endif

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMessage,
	WPARAM wParam, LPARAM lParam)
{
	// Pointer to the (C++ object that is the) window.
	MWwindow* pWindow = (MWwindow*) GetProp(hwnd, PROP_PTR);

    
	if ((pWindow == 0) || (iMessage == WM_DESTROY))
    {
		return DefWindowProc( hwnd, iMessage, wParam, lParam );
	}
	else
    {
		return pWindow->WndProc( iMessage, wParam, lParam );
	}
}

// #######################################################################
// #################  MWcreateParams class
// #######################################################################


MWcreateParams::MWcreateParams()
{
	title = 0;
	width = CW_USEDEFAULT;                  // width of the window
	height = CW_USEDEFAULT;                 // height of the window
	x = CW_USEDEFAULT;                      // x-coordinate screen position
	y = CW_USEDEFAULT;						// y-coordinate screen position
	parent = 0;								// window parent (if any)
	id = 0;									// control id
	style = WS_OVERLAPPED;                  // default window style

	const char* defaultClassname = Session::instance()->classname();
	classname = 0;
	classStyle = CS_HREDRAW | CS_VREDRAW;

	// ----- default names ----
	titleOf(defaultClassname);
	classnameOf(defaultClassname);
}

MWcreateParams::~MWcreateParams()
{
	delete [] title;
	delete [] classname;
}

void MWcreateParams::titleOf(const char* t)
{
	MWassert(t);
	if (title)
		delete [] title;
	title = new char[strlen(t)+1];
	strcpy(title, t);
}

const char* MWcreateParams::titleOf()
{
	return title;
}

void MWcreateParams::classnameOf(const char* nm)
{
	MWassert(nm);
	if (classname)
		delete [] classname;
	classname = new char[strlen(nm)+1];
	strcpy(classname, nm);
}

const char* MWcreateParams::classnameOf()
{
	return classname;
}


// #######################################################################
// #################  MWwindow class
// #######################################################################

// -----------------------------------------------------------------------
// constructors and destructors
// -----------------------------------------------------------------------
MWwindow::MWwindow()
{
	hwnd = 0;
    params = new MWcreateParams;
}

MWwindow::~MWwindow()
{
	delete params;
}

// -----------------------------------------------------------------------
// The windows callback function forwards the message translation to this
// function, which passes common messages to member functions and passes
// any unrecognized messages to the windows default handler.
// -----------------------------------------------------------------------
long MWwindow::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// ---- process the message ----
	switch(message)
	{
	case WM_CREATE:
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
    return 0;
}


// -----------------------------------------------------------------------
// Window binding functions.  bind() actually creates the window and binds
// it to this object.  unbind() removes the attachment to the MS-Windows
// window and destroys it.
// -----------------------------------------------------------------------

// mingw + launch python needs all windows to be bound from a specific
// thread.
#if defined(MINGW)
extern "C" {
int (*iv_bind_enqueue_)(void(*)(void*), void*);

static void wmap1(void* v) {
	Window* w = (Window*)v;
	w->map();
}
static void hide1(void* v) {
	HWND hwnd = (HWND)v;
	ShowWindow(hwnd, SW_HIDE);
}
static void unbind1(void* v) {
	HWND hwnd = (HWND)v;
	RemoveProp(hwnd, PROP_PTR);
	DestroyWindow(hwnd);
}
static void mwmap1(void* v) {
	HWND hwnd = (HWND)v;
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
}

}
#endif

void MWwindow::bind()
{
	// ---- if already bound... nothing to do ----
	if (hwnd)
		return;

	// ---- make sure window class is registered ----
	HINSTANCE hInstance = theApp.hinst;
	registerClass(hInstance);

	// ---- create the window ----
	hwnd = CreateWindowEx(
		params->styleEx,			// extended style information
		params->classnameOf(),		// See registerClass() call.
		params->titleOf(),			// Text for window title bar.
		params->style,				// Window style.
		params->x,					// horizontal position.
		params->y,					// vertical position.
		params->width,			 	// window width.
		params->height,				// window height.
		params->parent,				// parent of this window.
		(HMENU)((intptr_t)params->id),			// control id
		hInstance,				// This instance owns this window.
		(LPSTR) this			// pointer to c++ object
	);
	MWassert(hwnd);

    // --- free parameters now that they aren't used ----
	delete params;
	params = 0;

	// ---- store our pointer in the properties ----
	MWassert(SetProp(hwnd, PROP_PTR, (HANDLE) this));
}

void MWwindow::unbind()
{
	if (!hwnd) {
//		MessageBox(NULL,"MWwindow::unbind has no binding", "zzz", MB_OK);
		return;
	}
#if defined(MINGW)
	// if not correct thread enqueue and unmap will be called later
	if (iv_bind_enqueue_ && (*iv_bind_enqueue_)(unbind1, hwnd)) {
		//printf("MWwindow::unbind defer hwnd=%p\n", hwnd);
		hwnd = 0;
		return;
	}
#endif
//printf("MWwindow::unbind %p\n", this);
	// ---- remove properties... stops C++ messages -----
	RemoveProp(hwnd, PROP_PTR);

	// ---- tell MS-Windows to toss it ----
	DestroyWindow(hwnd);
	hwnd = 0;
}

// -----------------------------------------------------------------------
// Register the MS-Windows window class with Windows.  This is supposed to
// only be done once, so we first check to see if it exists.
// -----------------------------------------------------------------------
void MWwindow::registerClass(
	HINSTANCE hInstance)            // application instance
{
	MWassert(hInstance);
	WNDCLASS  wc;
	const char* className = params->classnameOf();
	MWassert(className);

	if (GetClassInfo(hInstance, className, &wc) == 0)
	{
		// ---- class not yet registered ----
		wc.style         = params->classStyle;
		wc.lpfnWndProc   = ::WndProc;
		wc.cbClsExtra    = 0;
		// Reserve extra bytes for each instance of the window.
		// These are used to store a pointer to the C++ object associated
        // with the MS-Windows window.
		wc.cbWndExtra    = sizeof(MWwindow*);
		wc.hInstance     = hInstance;
		wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName  = NULL;
		wc.lpszClassName = className;

		// Register the window class
		RegisterClass(&wc);
	}
}

// -----------------------------------------------------------------------
// Window mapping functions.  map() will map the window to the display.
// unmap() will remove the window from the display (hide it).  isMapped()
// will test whether or not the window is visible.
// -----------------------------------------------------------------------
bool MWwindow::map()
{
	if (hwnd)
	{
#if defined(MINGW)
		if (iv_bind_enqueue_ && (*iv_bind_enqueue_)(mwmap1, hwnd)) {
			return 1;
		}
#endif
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
		return 1;
	}
	 return 0;
}

void MWwindow::unmap()
{
//printf("enter MWwindow::unmap()\n");
	if (hwnd)
#if defined(MINGW)
	// if not correct thread enqueue and unmap will be called later
	if (iv_bind_enqueue_ && (*iv_bind_enqueue_)(hide1, hwnd)) {
		//printf("MWwindow::unmap ShowWindow defer hwnd=%p\n", hwnd);
		return;
	}
#endif
		ShowWindow(hwnd, SW_HIDE);
//printf("leave MWwindow::unmap()\n");
}

bool MWwindow::isMapped()
{
	if (hwnd)
		return IsWindowVisible(hwnd);
	else
		return 0;
}


// #######################################################################
// #################  WindowRep class
// #######################################################################

// -----------------------------------------------------------------------
// A window is needed to calculate font metrics and such, so the first
// window created gets registered as the "main window", and is used for
// such purposes by the MS-Windows specific layer.
// -----------------------------------------------------------------------
static WindowRep* rep = nil;

HWND WindowRep::mainWindow()
{
	return (rep) ? rep->msWindow() : nil;
}

// -----------------------------------------------------------------------
// crude error handler... notifies user and bails out.  Needs to be
// refined in the future for better behavior.
// -----------------------------------------------------------------------
void WindowRep::errorMessage(const char* msg)
{
	HWND hwnd = mainWindow();
	MessageBox(hwnd, msg, "InterViews Fatal Error", MB_ICONSTOP);
	exit(1);
}


// -----------------------------------------------------------------------
// constructors and destructors
// -----------------------------------------------------------------------
WindowRep::WindowRep(Window* w)
{
	win = w;
	styleOf(WS_OVERLAPPED);
	styleOfEx(0);
	palette = ColorRep::defaultPalette();
	left_ = CW_USEDEFAULT;
	bottom_ = CW_USEDEFAULT;
	cursor_stack_ = new MWcursorPtrList;
	cursor_ = NULL;
	controls = NULL;
	cntrlHWND = NULL;
	frontBuff = NULL;
	doubleBuffered = true;
   offscreen_ = false;

	// --- stash first window created ----
	 if (!rep)
		rep = this;

	// Create the global event used by WMinput() if it hasn't yet
	// been allocated.
	if (! input_e)
	{
		input_e = new Event;
		input_er = (input_e) ? input_e->rep() : 0;
		if (!input_e || !input_er)
			WindowRep::errorMessage("allocation failure");
	 }

	// ---- double-buffer initialize ---
	back_bitmap_ = nil;
}

WindowRep::~WindowRep()
{
	// --- release resources ----
	if (back_bitmap_)
		DeleteObject(back_bitmap_);
	if (frontBuff)
		ReleaseDC(msWindow(), frontBuff);
}

// -----------------------------------------------------------------------
// When binding to the MS-Window, we also bind
// this WindowRep to the CanvasRep, so that it can handle things like
// invalidation of parts of the window.
//
// The Window class (and all derived classes) stash their desired window
// dimensions into the canvas, so we get our desired size from there.
// -----------------------------------------------------------------------
void WindowRep::bind()
{
	// ---- bind to the CanvasRep ----
	MWcanvas* c = (MWcanvas*) win->canvas_;
	c->bind(this);

	// ---- update window properties ----
	int winHeight = c->to_pixels( win->height(), Dimension_Y );
	int winWidth = c->to_pixels( win->width(), Dimension_X );
	widthOf(winWidth);
	heightOf(winHeight);
	if (bottom_ == CW_USEDEFAULT)
		yposOf(CW_USEDEFAULT);
	else
	{
		MWassert(ivWindowOf());
		Display* d = ivWindowOf()->display();
		MWassert(d);
		yposOf(bottom_ - winHeight);
	}
    xposOf(left_);

	// ---- bind to MS-Window ----
	MWwindow::bind();
}

// -----------------------------------------------------------------------
// double-buffer management 
// -----------------------------------------------------------------------
void WindowRep::rebuffer()
{
	// --- release old double-buffer resources ----
	if (back_bitmap_)
		DeleteObject(back_bitmap_);

	// ---- create new double-buffer resources ----
	PixelCoord width = win->canvas_->pwidth();
	PixelCoord height = win->canvas_->pheight();
	HWND hwnd = msWindow();
	HDC winDC = GetDC(hwnd);
	back_bitmap_ = CreateCompatibleBitmap(winDC, width, height);
	ReleaseDC(hwnd, winDC);
}

void WindowRep::frontBuffer()
{
	RECT clientArea;
	GetClientRect(hwnd, &clientArea);
	frontBuff = GetDC(hwnd);

	MWassert(win);
	MWassert(win->canvas_);
	MWcanvas* c = (MWcanvas*) win->canvas_;
	c->beginPaint(frontBuff, clientArea);
}

void WindowRep::backBuffer()
{
	if (frontBuff)
	{
		// ---- update the affected part of the display ----
		if (doubleBuffered)
		{
			MWassert(win);
			MWassert(win->canvas_);
			MWcanvas* c = (MWcanvas*) win->canvas_;
			c->endPaint();

			// --- freshen the off-screen bitmap ----
			RECT updateArea;
			GetClientRect(hwnd, &updateArea);
			HDC backBuff = CreateCompatibleDC(frontBuff);
			SelectObject(backBuff, back_bitmap_);
			SelectClipRgn(frontBuff, NULL);
			SelectClipRgn(backBuff, NULL);
			BitBlt(backBuff, updateArea.left, updateArea.top,
				updateArea.right - updateArea.left,
				updateArea.bottom - updateArea.top,
				frontBuff, updateArea.left, updateArea.top, SRCCOPY);
			DeleteDC(backBuff);
		}

		ReleaseDC(hwnd, frontBuff);
		frontBuff = NULL;
	}
}


// -----------------------------------------------------------------------
// The windows callback function forwards the message translation to this
// function, which passes common messages to member functions and passes
// any unrecognized messages to the windows default handler.
// -----------------------------------------------------------------------

long WindowRep::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// ----- check if derived class wants to trap it -----
	if (win)
	{
		// --- fill in the event record ----
		input_er->setMessage(win, message, wParam, lParam);
        input_er->result = 0L;

		// ---- see the window wants to grab the event ---
		if (win->receive(*input_e))
		{
			return input_er->result;
		}
		else
		{
			// ---- process input messages ----
			long id;
			HWND h = NULL;
			switch(message)
			{
			case WM_PAINT:
				WMpaint(wParam, lParam);
				break;
			case WM_ERASEBKGND:
				break;
			case WM_LBUTTONDOWN:
				if (wParam & MK_CONTROL) {
				WMinput(wParam, lParam, Event::down, Event::middle);
				//}else if (wParam & MK_CONTROL) {
				//WMinput(wParam, lParam, Event::down, Event::right);
				}else{
				WMinput(wParam, lParam, Event::down, Event::left);
				}
				break;
			case WM_LBUTTONUP:
				if (wParam & MK_CONTROL) {
				WMinput(wParam, lParam, Event::up, Event::middle);
				//}else if (wParam & MK_CONTROL) {
				//WMinput(wParam, lParam, Event::up, Event::right);
				}else{
				WMinput(wParam, lParam, Event::up, Event::left);
				}
				break;
			case WM_MBUTTONDOWN:
				WMinput(wParam, lParam, Event::down, Event::middle);
				break;
			case WM_MBUTTONUP:
				WMinput(wParam, lParam, Event::up, Event::middle);
				break;
			case WM_RBUTTONDOWN:
				WMinput(wParam, lParam, Event::down, Event::right);
				break;
			case WM_RBUTTONUP:
				WMinput(wParam, lParam, Event::up, Event::right);
				break;
			case WM_MOUSEMOVE:
				WMinput(wParam, lParam, Event::motion, Event::none);
				break;
			case WM_CHAR:
				WMinput(wParam, lParam, Event::key, Event::none);
				break;
			case WM_SIZE:
				WMsize(wParam, lParam);
				break;
			case WM_MOVE:
				if (offscreen_) {
            	//printf("redraw for move\n");
					WMsize(wParam, lParam);
				}
				break;
			case WM_COMMAND:
				id = LOWORD(wParam);
				return childEvent(id, *input_e);
			case WM_HSCROLL:
			case WM_VSCROLL:
#ifdef WIN16
				h = (HWND) HIWORD(lParam);
#else
				h = (HWND) lParam;
#endif
				if (h != 0)
					return childEventHWND(h, *input_e);
				break;
			case WM_PALETTECHANGED:
				if ((HWND) wParam == hwnd)
					break;
			case WM_QUERYNEWPALETTE:
				{
					HDC hdc = GetDC(hwnd);
					int i = palette->realizeInto(hdc, FALSE);
					if (i)
						InvalidateRect(hwnd, NULL, FALSE);
					ReleaseDC(hwnd, hdc);
					return i;
				}
			case WM_KEYDOWN: {
			  //				extern bool iv_user_keydown(long);
			  //				if (iv_user_keydown(wParam)){
			  //					break;
			  //				}
				if (wParam >0x22 && wParam <0x2F) {
					int key = 0;
					switch (wParam) {
					case VK_DELETE:
						key = 'D';
						break;
					case VK_LEFT:
						key = 'B';
						break;
					case VK_RIGHT:
						key = 'F';
						break;
					case VK_HOME:
						key = 'A';
						break;
					case VK_END:
						key = 'E';
						break;
					}
					if (key) {
						WMinput(key&0x1F, lParam, Event::key, Event::none);
						break;
					}
				}
            }
			default:
				return MWwindow::WndProc(message, wParam, lParam);
			}
			return 0;
		}
	}

	// This shouldn't happen... a WindowRep without it's associated Window.
	// If it does, the window will still limp along with the default message
	// handler.
	return DefWindowProc( hwnd, message, wParam, lParam );
}

// -----------------------------------------------------------------------
// process a WM_PAINT message for this window
// -----------------------------------------------------------------------
long WindowRep::WMpaint(WPARAM, LPARAM)
{
	HDC hdc;								// display context to use
	HDC hdcBuff;							// double buffer display context
	RECT updateArea;						// update area in window

	MWassert(win);
	MWassert(win->canvas_);

	// 
	// This unsafe cast is ok since it's to the base class of all canvas
	// types used under windows.
	//
	MWcanvas* c = (MWcanvas*) win->canvas_;

	if (frontBuff)
	{
		// ---- immediate mode already in effect ----
   		HRGN hrgn;
	   	hrgn = CreateRectRgnIndirect(&updateArea);
		SelectClipRgn(frontBuff, hrgn);
	   	DeleteObject(hrgn);
		if (win->glyph_)
			win->glyph_->draw(win->canvas_, win->allocation_);
	}
	else
	{
		// ---- get the update area and device context ----
		if (GetUpdateRect(hwnd, &updateArea, FALSE))
		{
			//
			// !!!! Windows bug workaround !!!!!
			// For some reason the update area is sometimes off, which causes
			// clipping to be incorrect on updates.  A result of this is that
			// BeginPaint() can't be used because it sets clipping to the 
			// wrong value.  It needs to be called for windows to work however, 
			// so it is done after the rendering has been done.  It seems
			// like something else would be the cause and this be the effect,
			// but the InvalidateRect() calls have been checked and look good.
			//
			updateArea.right++;
			updateArea.bottom++;
			updateArea.left = Math::max(int(0), int(updateArea.left-1));
			updateArea.top = Math::max(int(0), int(updateArea.top-1));

			// ---- prepare canvas for painting ----
			bool doPaint;
			HDC hdcPaint = GetDC(hwnd);
			palette->realizeInto(hdcPaint, TRUE);
			if (doubleBuffered)
			{
				hdcBuff = CreateCompatibleDC(hdcPaint);
				SelectObject(hdcBuff, back_bitmap_);
				palette->realizeInto(hdcBuff, TRUE);
				hdc = hdcBuff;
				doPaint = c->any_damage();
			}
			else
			{
				hdc = hdcPaint;
				doPaint = true;
			}

			// ---- actual painting ----
			if (doPaint)
			{
//				DebugEndSection((void *)10342, "Other");
//				DebugStartSection(c, "Canvas");
				c->beginPaint(hdc, updateArea);
//				DebugEndSection(c, "Canvas");

				// ---- have the glyph hierarchy draw itself ----
//				DebugStartSection(win, "Window");
				if (win->glyph_)
					win->glyph_->draw(c, win->allocation_);
//				DebugEndSection(win, "Window");

				// ---- clean up cavas resources ----
//				DebugStartSection(c, "Canvas");
				c->endPaint();
#if 0
				DebugEndSection(c, "Canvas");
				int n;
				float f;
				n = DebugGetSectionTime(c, "Canvas", &f);
				DebugMessage("Canvas %x : TotalTime = %d, percent of time = %.3f\n", c, n, f);
				n = DebugGetSectionTime(win, "Window", &f);
				DebugMessage("Window %x : TotalTime = %d, percent of time = %.3f\n", win, n, f);
				n = DebugGetSectionTime((void *)10342, "Other", &f);
				DebugMessage("Other           : TotalTime = %d, percent of time = %.3f\n\n", n, f);
				DebugStartSection((void *)10342, "Other");
#endif
			}

			// ---- cleanup ----
			if (doubleBuffered)
			{
				// ---- update the affected part of the display ----
				SelectClipRgn(hdcPaint, NULL);
				SelectClipRgn(hdcBuff, NULL);
//DebugMessage
GetUpdateRect(hwnd, &updateArea, FALSE);
				POINT pt[2];
				pt[0].x = updateArea.left;
				pt[0].y = updateArea.top;
				pt[1].x = updateArea.right;
				pt[1].y = updateArea.bottom;
				MWassert( DPtoLP(hdcBuff, pt, 2) );
				StretchBlt(hdcPaint, updateArea.left, updateArea.top,
					updateArea.right - updateArea.left,
					updateArea.bottom - updateArea.top,
					hdcBuff, pt[0].x, pt[0].y,
					(pt[1].x - pt[0].x), (pt[1].y - pt[0].y),
					SRCCOPY);
				DeleteDC(hdcBuff);
			}
// MODIFIED by NL for WIN32s:
// this seems like a bug. didn't release one of the DCs before
			ReleaseDC(hwnd, hdcPaint);
			
			// hack to get around incorrect update area problem
			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
		}
	}
	// check if any part of window off screen
	RECT r;
	GetWindowRect(hwnd, &r);
	Display* d = win->display();
	if (r.right > d->pwidth() || r.bottom > d->pheight()
		|| r.top < 0 || r.left < 0) {
			offscreen_ = true;
	}else {
		offscreen_ = false;
	}
	return 0;
}

// -----------------------------------------------------------------------
// process a mouse/keyboard event.  A global object is used to decrease the
// construction/destruction time since mouse movement will generate a lot
// of these!  "input_er" is a pointer to the EventRep of the global Event
// "input_e".  No pointer checks are made as the library will bail out if
// the Event and associated EventRep can't be constructed in the first
// place.
// -----------------------------------------------------------------------
long WindowRep::WMinput(
	WPARAM wParam,					// windows word parameter
	LPARAM lParam,                  // windows long parameter
	int t,                          // type of event
	int b)                          // which button
{
	// ---- fill in the information ----
	input_er->window_ = win;
	input_er->type_ =  t;
	input_er->button_ =  b;
	input_er->wparam_ = wParam;
	input_er->lparam_ = lParam;

    // ---- go process it ----
	EventRep::handleCallback(*input_e);

	return 0;
}

// -----------------------------------------------------------------------
// Process a window resize message... basically allocating the associated
// glyph and damaging the new area.  This is driven by the WM_SIZE messge
// which is sent by the system telling us what our allocated size is.
// -----------------------------------------------------------------------
long WindowRep::WMsize(WPARAM, LPARAM)
{
	RECT clientArea;
	GetClientRect(hwnd, &clientArea);
   if (clientArea.right < 1 || clientArea.bottom < 1) {
   	// happens when iconify on win95 desktop. Used to end
      // in floating exception in scene2view
   	return 0;
   }

	int width = clientArea.right;
	int height = clientArea.bottom;

	// ---- invalidate stored canvases ----
	win->glyph_->undraw();

	// ---- initialize for allocation ----
	win->canvas_->psize(width, height);
	if (request_on_resize_) {
		Box::full_request(true);
		win->glyph_->request(win->shape_);
		Box::full_request(false);
	}
	const Requirement& rx = win->shape_.requirement(Dimension_X);
    const Requirement& ry = win->shape_.requirement(Dimension_Y);
    Coord xsize = win->canvas_->width();
    Coord ysize = win->canvas_->height();
    Coord ox = xsize * rx.alignment();
    Coord oy = ysize * ry.alignment();
    win->allocation_.allot(Dimension_X, Allotment(ox, xsize, ox / xsize));
    win->allocation_.allot(Dimension_Y, Allotment(oy, ysize, oy / ysize));
    Extension ext;
    ext.clear();

	// ---- tell the glyphs what they have been allocated ----
	win->glyph_->allocate(win->canvas_, win->allocation_, ext);

	// ---- rebuild the buffer ---
	rebuffer();
	win->canvas_->damage_all();

	return 0;
}

// -----------------------------------------------------------------------
// The following are to support child-window controls.
//
// WMcommand distributes command messages to the C++ object that correponds 
// to the child-window control that is trying to tell us something.
//
// registerControl() Registers a predefined control, or custom control
// (really any child window) with this window.  This allows the window to 
// route child window callbacks to their C++ control object which handles
// distribution of the callbacks.
//
// There is an HWND version as well as id, because some controls (such as
// a scrollbar) have only a handle to their window available from the
// incoming messages.
// -----------------------------------------------------------------------
long WindowRep::childEvent(long id, Event& e)
{
	MWchild* c;

	// If a hash table exists, then try to look up the pointer to the C++ 
	// control object and invoke the WMcommand function.
	if (controls)
	{
		if (controls->find(c, id))
		{
			c->receive(e);
			return 0;
		}
	}
	return 1;
}

void WindowRep::registerControl(long id, MWchild* c)
{
	// ---- make sure there's a hash table ----
	// We don't allocate the table unless child window controls are actually
	// being used.  If we get a request to register, than they obviously are
	// being used and we allocate the hash table if it hasn't already been
	// allocated.
	if (! controls)
	{
		controls = new MWcontrolTable(20);
		MWassert(controls);
	}

	// ---- add the association to the table ----
	controls->insert(id, c);
}

long WindowRep::childEventHWND(HWND h, Event& e)
{
	MWchild* c;

	// If a hash table exists, then try to look up the pointer to the C++ 
	// control object and invoke the WMcommand function.
	if (cntrlHWND)
	{
		if (cntrlHWND->find(c, h))
		{
			c->receive(e);
			return 0;
		}
	}
	return 1;
}

void WindowRep::registerControlHWND(HWND h, MWchild* c)
{
	// ---- make sure there's a hash table ----
	// We don't allocate the table unless child window controls are actually
	// being used.  If we get a request to register, than they obviously are
	// being used and we allocate the hash table if it hasn't already been
	// allocated.
	if (! cntrlHWND)
	{
		cntrlHWND = new MWcontrolHWND(20);
		MWassert(cntrlHWND);
	}

	// ---- add the association to the table ----
	cntrlHWND->insert(h, c);
}

// 
// #######################################################################
// #####################  Window class
// #######################################################################

extern void ivcleanup_add_window(Window*);
extern void ivcleanup_remove_window(Window*);
extern void ivcleanup_after_window(Window*);

Window::Window(Glyph* g)
{
	rep_ = new WindowRep(this);
	rep_->request_on_resize_ = false;

	//
	// The type of canvas created depends upon the capabilities of the 
	// platform being used.  This can be done at runtime to allow the
	// same binary for Win32s and Win32. 
	//
	DWORD vers = HIWORD(GetVersion());
	if (vers & 0x8000)
	{
		// running Win32s on Windows 3.1
		canvas_ = new MWcanvas16;
	}
	else
	{
		// running Windows NT
//		canvas_ = new MWcanvas32;
		canvas_ = new MWcanvas16;
	}

	glyph_ = g;
	glyph_->ref();
	style_ = nil;
	focus_in_ = nil;
	focus_out_ = nil;

	rep_->windowClass(WINDOW_CLASSNAME);
	rep_->classStyle(CS_HREDRAW | CS_VREDRAW);
	ivcleanup_add_window(this);
}

Window::Window(WindowRep* w)
{
	 rep_ = w;
	 ivcleanup_add_window(this);
}

Window::~Window()
{
//printf("Window::~Window %p\n", this);
	if (bound()) {
	 Window::unbind();
	}
	delete rep_;
    rep_ = nil;

	//
	// An undraw() message must be passed down the glyph hierarchy to 
	// disassociate the canvas with it before deletion of the canvas... and
	// ultimately the glyphs which might have kept a pointer to the canvas.
	//
	if (glyph_)
		glyph_->undraw();
	Resource::unref_deferred(glyph_);
	delete canvas_;
	Resource::unref_deferred(style_);
    Resource::unref_deferred(focus_in_);
	Resource::unref_deferred(focus_out_);
	 //Resource::unref_deferred(wm_delete);
	 ivcleanup_remove_window(this);
}

void Window::repair()
{
}

// -----------------------------------------------------------------------
// attribute functions of window
// -----------------------------------------------------------------------

void Window::style(Style* s)
{
	Resource::ref(s);
    Resource::unref(style_);
	style_ = s;
}

Style* Window::style() const
{
	return style_;
}

void Window::display(Display*)
{
	// no such thing under MS-Windows
}

Display* Window::display() const
{
	Session* s = Session::instance();
    MWassert(s);
	return s->default_display();
}

// -----------------------------------------------------------------------
// cursor functions
// -----------------------------------------------------------------------

void Window::cursor(Cursor* c)
{
    WindowRep& w = *rep();
	w.cursor_ = c;

	if (c == nil)
	{
                 HCURSOR hc = LoadCursor(NULL, IDC_ARROW);
#if defined(GCLP_HCURSOR)
                 SetClassLongPtr(rep()->msWindow(), GCLP_HCURSOR, (LONG_PTR)hc);
#else
                 SetClassLong(rep()->msWindow(), GCL_HCURSOR, (long)hc);
#endif
                 SetCursor(hc);
	}
	else
	{
		HCURSOR hc = c->rep()->cursorOf();
#if defined(GCLP_HCURSOR)
                (HCURSOR)SetClassLongPtr(rep()->msWindow(), GCLP_HCURSOR, (LONG_PTR)hc);
#else
                (HCURSOR)SetClassLong(rep()->msWindow(), GCL_HCURSOR, (long)hc);
#endif
        SetCursor(hc);
	}
}

Cursor* Window::cursor() const
	{ return rep()->cursor_; }

void Window::push_cursor()
{
    WindowRep& w = *rep();
    w.cursor_stack_->prepend(w.cursor_);
}

void Window::pop_cursor()
{
    WindowRep& w = *rep();
	if (w.cursor_stack_->count() > 0)
	{
		cursor(w.cursor_stack_->item(0));
		w.cursor_stack_->remove(0);
    }
}

// -----------------------------------------------------------------------
// window placement functions
// -----------------------------------------------------------------------
void Window::place(Coord left, Coord bottom)
{
    WindowRep& w = *rep();
	if (w.bound())
	{
		move(left, bottom);
	}
	else
	 {
		Display* d = display();
		w.left_ = d->to_pixels(left, Dimension_X);
		w.bottom_ = d->pheight() - d->to_pixels(bottom, Dimension_Y);
    }
}

void Window::pplace(IntCoord pleft, IntCoord pbottom)
{
	WindowRep& w = *rep();
	if (w.bound())
	{
		move((Coord) pleft, (Coord) pbottom);
	}
	else
    {
		w.left_ = pleft;
		w.bottom_ = display()->pheight() - pbottom;
    }
}

// Align the window around the current origin using the given alignment.
// As typical with InterViews, the alignment is from 0 to 1.  An alignment
// of 0 would keep the origin the same, and an alignment of 1 would shift
// the origin to the other side.
void Window::align(float x, float y)
{
	if (!is_mapped()) {default_geometry();}
	Coord l = left();
	Coord b = bottom();
	l = l - (width() * x);
	b = b - (height() * y);
	 place(l, b);
}

// Return the x-coordinate of the left side of the window in terms
// of InterViews display coordinates.
Coord Window::left() const
{
	WindowRep& w = *rep();
	Display* d = display();
	if (w.bound())
	{
		// ---- get the MS-Windows screen coordinates ----
		HWND hwnd = w.msWindow();
		RECT winRect;
		GetWindowRect(hwnd, &winRect);
		  return d->to_coord(winRect.left, Dimension_X);
	}
	else
	{
		return d->to_coord(w.left_, Dimension_X);
	}
}

// Return the y-coordinate of the bottom side of the window in terms
// of InterViews display coordinates.  This is opposite the coordinate
// system used by MS-Windows, and is therefore adjusted.
Coord Window::bottom() const
{
	WindowRep& w = *rep();
	Display* d = display();
	if (w.bound())
	{
		// ---- find the Windows screen coordinates ----
		HWND hwnd = w.msWindow();
		RECT winRect;
		GetWindowRect(hwnd, &winRect);

		  // ---- convert to InterViews coordinates ----
		MWassert(d);
		return (d->height() - d->to_coord(winRect.bottom, Dimension_Y));
	}
	else
	{
		return d->height() - d->to_coord(w.bottom_, Dimension_Y);
	}
}

void iv_window_coords(const Event& e, Window* w1, Coord& x1, Coord& y1) {
	// given event return coords in w1
	if (e.window() == w1) {
		x1 = e.pointer_x();
		y1 = e.pointer_y();
	}else{
		x1 = e.pointer_root_x() - w1->left(); // off by w1 left decoration
		y1 = e.pointer_root_y() - w1->bottom(); // off by w1 bottom decor
		RECT rw, rc;
		HWND h = w1->rep()->msWindow();
		GetWindowRect(h, &rw);
		GetClientRect(h, &rc);
		POINT pt;
		pt.x = rc.left;
		pt.y = rc.bottom;
		ClientToScreen(h, &pt);
		int xdec = pt.x - rw.left;
		int ydec = pt.y - rw.bottom;
		x1 -= w1->display()->to_coord(xdec, Dimension_X);
		y1 -= w1->display()->to_coord(ydec, Dimension_Y);
	}
}
// -----------------------------------------------------------------------
// Window dimensions.  The canvas is queried to get the dimensions as the
// WindowRep class keeps the canvas up-to-date with respect to size.  To
// set the size, one uses the style attributes in the typical X11 manner
// which is emulated with the ".ini" files in MS-Windows.
// -----------------------------------------------------------------------
Coord Window::width() const
{
	return canvas_->width();
}

Coord Window::height() const
{
	return canvas_->height();
}

// -----------------------------------------------------------------------
// Window mapping functions
//
// Many operations upon the real window are postponed until a request is
// made to map the window to the display.  At this point, any attributes
// desired of the window have already been set, and we are ready to really
// create the window and map it.  If the window is already bound to an
// MS-Window, than we simply map it.
// -----------------------------------------------------------------------

void Window::map()
{
	// ---- check to see if we are already mapped ----
    if (is_mapped())
		return;

	// ---- check to see if we are bound to an MS-Windows window ----
	if (!bound())
	{
#if defined(MINGW)
		// if not correct thread enqueue and map will be called later
		if (iv_bind_enqueue_ && (*iv_bind_enqueue_)(wmap1, this)) {
			return;
		}
#endif
		if (style_ == nil)
			style(new Style(Session::instance()->style()));

		configure();
		default_geometry();
		compute_geometry();
		set_props();
		bind();
	 }
#if 1
// ensure top on screen if window small enough
	Coord above = display()->height()-(bottom() + canvas()->height());
	// dont ask me why the above does not work with the
	// managedwindow height()
	if (above < 0. && ((bottom() + above) > 0)
	) {
//	DebugMessage("%g %g %g %g\n", bottom(), canvas()->height(),height(), display()->height()-(bottom() + height()));
		place(left(), bottom() + above);
	}
#endif
    do_map();
}
void Window::unmap()
{
//printf("enter Window::unmap %p\n", this);
	if (is_mapped())
	{
		WindowRep& w = *rep();
		glyph_->undraw();
		w.unmap();
	}
//printf("leave Window::unmap()\n");
}

bool Window::is_mapped() const
{
    WindowRep& w = *rep();
	return w.isMapped();
}

// -----------------------------------------------------------------------
// Binding functions.
//
// These functions bind and unbind the InterViews windows with their real
// associated windows in the window-system dependant world.
// -----------------------------------------------------------------------
void Window::bind()
{
    WindowRep& w = *rep();
	w.bind();
}

void Window::unbind()
{
	WindowRep& w = *rep();
	if (glyph_)
		glyph_->undraw();
    w.unbind();
}

bool Window::bound() const
{
	WindowRep& w = *rep();
	return w.bound();
}

// -----------------------------------------------------------------------

void Window::set_attributes()
{
}

/*
 * Search for a handler for the given event.
 * For events that have no associated pointer location, return nil.
 * Otherwise, use pick on the glyph to find a handler.
 */

Handler* Window::target(const Event& e) const
{
	Hit hit(&e);
	glyph_->pick(canvas_, allocation_, 0, hit);

	 Handler* h = hit.handler();
#if 0
    if (h != nil && (e.grabber() == nil || e.is_grabbing(h))) 
	{
		return h;
	 }
#endif
	return h;}
// -----------------------------------------------------------------------
// Look at an event that has been received for this window.  We directly
// dispatch focus and delete events.  The events come in from the
// WindowRep object and get dispatched to this function which can be
// re-implimented by derived classes (which should call their base class
// if they don't handle a message).
//
// If the event was used, true is returned.  If the event was not something
// recognized, then false is returned.  This is a difference from the SGI
// distribution, but is necessary because MS-Windows provides a default
// handler for those messages not directly processed.  There will be no
// effect on existing code since it will not be looking for a return value.
// -----------------------------------------------------------------------
bool Window::receive(const Event& e)
{
	UINT msg = e.rep()->messageOf();

	switch(msg)
	{
	case WM_SETFOCUS:
		if (focus_in_)
		{
        	Event fie(e);
			focus_in_->event(fie);
        }
        break;
	case WM_KILLFOCUS:
		if (focus_out_)
		{
        	Event foe(e);
			focus_out_->event(foe);
		}
		break;
	 default:
		return false;
	}
	return true;
}

// -----------------------------------------------------------------------
// Pointer grabs
//
// grab_pointer() grabs control of the display pointer and uses the given
// cursor when it is outside the window.  ungrab_pointer() relases the
// control over the pointer and allows it to be used by other windows.
// -----------------------------------------------------------------------
void Window::grab_pointer(Cursor* c) const
{
	WindowRep& w = *rep();
	HWND hwnd = w.msWindow();
    SetCapture(hwnd);
	if (c)
	{
		Window* win = (Window*) this;
    	win->cursor(c);
    }
}

void Window::ungrab_pointer() const
{
	ReleaseCapture();
}

void Window::raise()
{
	HWND hwnd = rep_->msWindow();
	SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void Window::lower()
{
	HWND hwnd = rep_->msWindow();
	SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

// Move the window to the position given by the InterViews display
// coordinates for the left and bottom.  These must be converted to
// MS-Windows screen coordinates, which are different in the y-axis.
void Window::move(Coord left, Coord bottom)
{
	HWND hwnd = rep_->msWindow();
	Display* d = display();
	MWassert(d);

	// ----- determine current size ----
//	int width = canvas_->pwidth();
//	int height = canvas_->pheight();
	RECT r;
	GetWindowRect(hwnd, &r);
	int width = r.right - r.left;
	int height = r.bottom - r.top;

	int y = (d->pheight() - d->to_pixels(bottom, Dimension_Y)) - height;
	int x = d->to_pixels(left, Dimension_X);

	// ---- move the window ----
	MoveWindow(hwnd, x, y, width, height, TRUE);
}

void Window::resize()
{
	HWND hwnd = rep_->msWindow();
	RECT curr_pos;
	GetWindowRect(hwnd, &curr_pos);

	// ----- determine desired size ----
	int width = canvas_->pwidth();
	int height = canvas_->pheight();

	// ---- resize the window ----
	MoveWindow(hwnd, curr_pos.left, curr_pos.top, width, height, TRUE);
}

// Protected operation called by map()
void Window::configure() 
{ 
	if (style_)
		rep()->doubleBuffer(style_->value_is_on("double_buffered"));
}

// Protected operation called by map().
//
// Determines the desired geometry by sending the request() protocol
// down through the glyph hierarchy.
void Window::default_geometry()
{
	if (glyph_ && canvas_)
	{
		glyph_->request(shape_);
    	Coord width = shape_.requirement(Dimension_X).natural();
		Coord height = shape_.requirement(Dimension_Y).natural();
		canvas_->size(width, height);
	}
	else
	{
		WindowRep::errorMessage("Window::default_geometry");
		// NOT REACHED
    }
}

// Protected operation called by map()
void Window::compute_geometry() { }

// protected function called by map()
void Window::set_props()
{
}

// protected function called by map()
void Window::do_map()
{
	WindowRep& w = *rep();
    w.map();
}

// #######################################################################
// ##################### class ManagedWindow
// #######################################################################


ManagedWindow::ManagedWindow(Glyph* g) : Window(g)
{
	mrep_ = new ManagedWindowRep(this);
	mrep_->wm_delete_ = nil;
    mrep_->group_leader_ = nil;
    mrep_->transient_for_ = nil;
    mrep_->icon_ = nil;
    mrep_->icon_bitmap_ = nil;
	mrep_->icon_mask_ = nil;
}

ManagedWindow::~ManagedWindow()
{
	delete mrep_;
}

// -----------------------------------------------------------------------
//  The ManagedWindow is a window visible the the window management
//  facilities of the window system.  In MS-Windows these would be
//  captioned windows.  The size of these windows is different from the
//  size of the canvas which represents the client area.  We query the
//  system metrics to determine the size of the borders and caption to
//  determine the size of the window relative to the canvas.
// -----------------------------------------------------------------------
Coord ManagedWindow::width() const
{
	int xAdjust = 2 * GetSystemMetrics(SM_CXFRAME); 
	return canvas_->width() + canvas_->to_coord(xAdjust, Dimension_X);
}

Coord ManagedWindow::height() const
{
	int yAdjust = GetSystemMetrics(SM_CYCAPTION) + 
		(2 * GetSystemMetrics(SM_CYFRAME));
	return canvas_->height() + canvas_->to_coord(yAdjust, Dimension_Y);
}

// -----------------------------------------------------------------------
//  Handle window events.
// -----------------------------------------------------------------------
bool ManagedWindow::receive(const Event& e)
{
	EventRep* er = e.rep();
	switch(er->messageOf())
	{
	case WM_GETMINMAXINFO:
		mrep_->WMminmax(er->wparamOf(), er->lparamOf());
        break;
	case WM_CLOSE:
		mrep_->WMclose(er->wparamOf(), er->lparamOf());
	    break;
	default:
    	return false;
	}
	return true;
}

ManagedWindow* ManagedWindow::icon() const
	{ return rep()->icon_; }
Bitmap* ManagedWindow::icon_bitmap() const
	{ return rep()->icon_bitmap_; }
Bitmap* ManagedWindow::icon_mask() const
	{ return rep()->icon_mask_; }

void ManagedWindow::icon(ManagedWindow* i)
{
    ManagedWindowRep& w = *rep();
    w.icon_ = i;
    //w.do_set(this, &ManagedWindowRep::set_icon);
}

void ManagedWindow::icon_bitmap(Bitmap* b)
{
    ManagedWindowRep& w = *rep();
    w.icon_bitmap_ = b;
    //w.do_set(this, &ManagedWindowRep::set_icon_bitmap);
}

void ManagedWindow::icon_mask(Bitmap* b)
{
	 ManagedWindowRep& w = *rep();
    w.icon_mask_ = b;
    //w.do_set(this, &ManagedWindowRep::set_icon_mask);
}

void ManagedWindow::iconify()
{
	WindowRep& w = *Window::rep();
	HWND hwnd = w.msWindow();

    ShowWindow(hwnd, SW_MINIMIZE);
}

void ManagedWindow::deiconify()
{
	WindowRep& w = *Window::rep();
	HWND hwnd = w.msWindow();

	ShowWindow(hwnd, SW_RESTORE);
}

void ManagedWindow::resize()
{
    default_geometry();
	Window::resize();
}

// -----------------------------------------------------------------------
// Install "focus in" and "focus out" handlers.  Activation of these
// handlers is through Window::receive().
// -----------------------------------------------------------------------
void ManagedWindow::focus_event(Handler* in, Handler* out)
{
	Resource::ref(in);
	 Resource::ref(out);
    Resource::unref(focus_in_);
    Resource::unref(focus_out_);
    focus_in_ = in;
    focus_out_ = out;
}

void ManagedWindow::wm_delete(Handler* h)
{
	Resource::ref(h);
	Resource::unref(mrep_->wm_delete_);
	mrep_->wm_delete_ = h;
}

void ManagedWindow::set_props()
{
	// ---- try to set the title ----
	Style* s = style();
	MWassert(s);
	String v;
	if (s->find_attribute("name", v) || s->find_attribute("title", v)) 
	{
		NullTerminatedString ns(v);
		rep_->windowTitle(ns.string());
	}
}

// ----------------------------------------------------------------------
// The X-Windows function XParseGeometry() appears at the end of this
// file and is used to parse the geometry specifications.  
// ----------------------------------------------------------------------
const int NoValue = 0;
const int XValue = 1;
const int YValue = 2;
const int WidthValue = 4;
const int HeightValue = 8;
const int XNegative = 16;
const int YNegative = 32;
int XParseGeometry (const char*, int*, int*, unsigned int*, unsigned int*);

void ManagedWindow::compute_geometry()
{
	Style* s = style();
	MWassert(s);
	String v;
	if (s->find_attribute("geometry", v))
	{
		NullTerminatedString g(v);
		WindowRep& wr = *Window::rep();
		Canvas& c = * canvas();
		Display& d = * display();
		unsigned int spec = 0;
		unsigned int xw, xh;
		int xpos, ypos;
		spec = XParseGeometry(g.string(), &xpos, &ypos, &xw, &xh);
		if ((spec & WidthValue) != 0) 
		{
			c.psize(xw, c.pheight());
		}
		if ((spec & HeightValue) != 0) 
		{
			c.psize(c.pwidth(), xh);
		}
		if ((spec & XValue) != 0)
		{
			if ((spec & XNegative) != 0)
				wr.left_ = d.pwidth() + xpos;
			else
				wr.left_ = xpos;
		}
		if ((spec & YValue) != 0)
		{
			if ((spec & YNegative) != 0)
				wr.bottom_ = d.pheight() + ypos - c.pheight();
			else
				wr.bottom_ = ypos + c.pheight();
		}
	}
}

// #######################################################################
// ############### class ManagedWindowRep
// #######################################################################

ManagedWindowRep::ManagedWindowRep(ManagedWindow* w)
{
	win = w;
}

ManagedWindowRep::~ManagedWindowRep()
{
}


// -----------------------------------------------------------------------
// MS-Windows sends this message to determine how it can resize the window,
// so we let it know based upon the information from the last request()
// query of the glyph hierarchy.
// -----------------------------------------------------------------------
long ManagedWindowRep::WMminmax(WPARAM, LPARAM lParam)
{
	MINMAXINFO* mmi = (MINMAXINFO*) lParam;

	Requirement& rx = win->shape_.requirement(Dimension_X);
	Requirement& ry = win->shape_.requirement(Dimension_Y);
#if 1
	// the original
	unsigned int xAdjust = 2 * (GetSystemMetrics(SM_CXBORDER) + 
		GetSystemMetrics(SM_CXFRAME));
	unsigned int yAdjust = GetSystemMetrics(SM_CYBORDER) +
		GetSystemMetrics(SM_CYCAPTION) +
		(2 * GetSystemMetrics(SM_CYFRAME));
#else
	// this suddenly increased the window size when moved
	unsigned int xAdjust = GetSystemMetrics(SM_CXMINTRACK);
	unsigned int yAdjust = GetSystemMetrics(SM_CYMINTRACK);
#endif

	Display* dpy = win->display();
	MWassert(dpy);

	mmi->ptMinTrackSize.x = Math::max(2 + xAdjust,
		dpy->to_pixels(rx.natural() - rx.shrink(), Dimension_X) + xAdjust);
	mmi->ptMinTrackSize.y = Math::max(2 + yAdjust,
		dpy->to_pixels(ry.natural() - ry.shrink(), Dimension_Y) + yAdjust);
	mmi->ptMaxTrackSize.x = Math::min(
		(unsigned int) GetSystemMetrics(SM_CXMAXTRACK),
		dpy->to_pixels(rx.natural() + rx.stretch(), Dimension_X) + xAdjust);
	mmi->ptMaxTrackSize.y = Math::min(
		(unsigned int) GetSystemMetrics(SM_CYMAXTRACK),
		dpy->to_pixels(ry.natural() + ry.stretch(), Dimension_Y) + yAdjust);
	mmi->ptMaxSize.x = mmi->ptMaxTrackSize.x;
	mmi->ptMaxSize.y = mmi->ptMaxTrackSize.y;

	return 0;
}

long ManagedWindowRep::WMclose(WPARAM, LPARAM)
{
    Handler* handler = wm_delete_;
	if (handler == nil)
	{
		Session::instance()->quit();
	}
	else
	{
		Event e;
		handler->event(e);
	}
	return 0;
}

// #######################################################################
// ###################  class ApplicationWindow
// #######################################################################

ApplicationWindow::ApplicationWindow(Glyph* g) : ManagedWindow(g)
{
	rep_->styleOf(WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN);
}

ApplicationWindow::~ApplicationWindow()
{
}

void ApplicationWindow::compute_geometry()
{
#ifdef IS_IMPLIMENTED
    WindowRep& wr = *Window::rep();
    CanvasRep& c = *wr.canvas_->rep();
    Display& d = *wr.display_;
    unsigned int spec = 0;
    String v;
    if (wr.style_ != nil && wr.style_->find_attribute("geometry", v)) {
	NullTerminatedString g(v);
	unsigned int xw, xh;
	spec = XParseGeometry(g.string(), &wr.xpos_, &wr.ypos_, &xw, &xh);
	const unsigned int userplace = XValue | YValue;
	if ((spec & userplace) == userplace) {
	    wr.placed_ = true;
	}
	if ((spec & WidthValue) != 0) {
	    c.pwidth_ = PixelCoord(xw);
	    c.width_ = d.to_coord(c.pwidth_, Dimension_X);
	}
	if ((spec & HeightValue) != 0) {
	    c.pheight_ = PixelCoord(xh);
	    c.height_ = d.to_coord(c.pheight_, Dimension_Y);
	}
	if ((spec & XValue) != 0 && (spec & XNegative) != 0) {
	    wr.xpos_ = d.pwidth() + wr.xpos_ - c.pwidth_;
	}
	if ((spec & YValue) != 0 && (spec & YNegative) != 0) {
	    wr.ypos_ = d.pheight() + wr.ypos_ - c.pheight_;
	}
	}
#endif
    ManagedWindow::compute_geometry();
}

void ApplicationWindow::set_props()
{
	ManagedWindow::set_props();
}

// #######################################################################
// ############### class TopLevelWindow
// #######################################################################


TopLevelWindow::TopLevelWindow(Glyph* g) : ManagedWindow(g) 
{ 
	rep_->styleOf(WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN);
// toplevel is basically just the same as application -- need the special buttons and stuff
//	rep_->styleOf(WS_CAPTION | WS_OVERLAPPED | WS_THICKFRAME);
//  	rep_->styleOfEx(NULL);
//  	rep_->windowClass(TOPLEVEL_CLASSNAME);
//  	rep_->classStyle(CS_HREDRAW | CS_VREDRAW); 
}

TopLevelWindow::~TopLevelWindow() 
{ 
}

void TopLevelWindow::group_leader(Window* primary) 
{
    ManagedWindow::rep()->group_leader_ = primary;
}

Window* TopLevelWindow::group_leader() const 
{ 
	return ManagedWindow::rep()->group_leader_; 
}

void TopLevelWindow::set_props() 
{
    ManagedWindow::set_props();
}

// #######################################################################
// ############### class TransientWindow
// #######################################################################

TransientWindow::TransientWindow(Glyph* g) : TopLevelWindow(g) 
{ 
}

TransientWindow::~TransientWindow() 
{ 
}

void TransientWindow::transient_for(Window* primary) 
{
	 ManagedWindow::rep()->transient_for_ = primary;
	 if (primary) {
		HWND pw = primary->Window::rep()->msWindow();
		if (!bound()) {
			Window::rep()->parentOf(pw);
		}
	 }
}

Window* TransientWindow::transient_for() const 
{
    return ManagedWindow::rep()->transient_for_;
}

/*
 * Don't do the normal geometry property lookup, etc. for transients.
 */
void TransientWindow::configure() 
{
    Window::configure();
}

void TransientWindow::set_attributes() 
{
    Style& s = *style();
    s.alias("TransientWindow");
    TopLevelWindow::set_attributes();
}

// #######################################################################
// ###################  class PopupWindow
// #######################################################################

PopupWindow::PopupWindow(Glyph* g) : Window(g)
{
	rep_->styleOf(WS_POPUP);
	rep_->windowClass(POPUP_CLASSNAME);
	rep_->classStyle(CS_HREDRAW | CS_VREDRAW);

	// This must be called here so that the size of the canvas gets set
	// before the window gets manipulated.  The menu functionality starts
	// trying to align the window around some point which would fail because
	// the canvas had no size.  Note that this won't call a derived version
	// of default_geometry() (although it will later in time in the base class 
	// implementation), so it's not a great solution.
	default_geometry();
	ivcleanup_after_window(this);
}

PopupWindow::~PopupWindow()
{
}

void PopupWindow::set_attributes()
{
    Style& s = *style();
    s.alias("PopupWindow");
    Window::set_attributes();
}

bool PopupWindow::receive(const Event& e)
{
	UINT msg = e.rep()->messageOf();

	switch(msg)
	{
	case WM_PAINT:
		// Apparently, popup windows don't receive a size message... and
		// the WindowRep processing of it fetches the client rect rather than
		// use the parameters, so we manually call WMresize() with bogus
		// parameters.  We still want the normal processing of the message
		// so we return false.
		rep_->WMsize(0,0);
		break;
	}
	return false;
}


// #########################################################################
// 
// XParseGeometry - this function comes from the source to X-Windows that
// parses a geometry specification.  This ensures that the MS-Windows 
// version parses the specification in the same way.  The mask values are
// dummied up to be independant of the X11 include files.
// 
// #########################################################################

/* Copyright 	Massachusetts Institute of Technology  1985, 1986, 1987 */
/* $XConsortium: XParseGeom.c,v 11.18 91/02/21 17:23:05 rws Exp $ */

/*
Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
*/

/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */

static int ReadInteger(char* string, char** NextString)
{
    int Result = 0;
    int Sign = 1;
    
    if (*string == '+')
	string++;
    else if (*string == '-')
    {
	string++;
	Sign = -1;
    }
    for (; (*string >= '0') && (*string <= '9'); string++)
    {
	Result = (Result * 10) + (*string - '0');
    }
    *NextString = string;
    if (Sign >= 0)
	return (Result);
    else
	return (-Result);
}

int XParseGeometry (
	const char *string,
	int *x,
	int *y,
	unsigned int *width,    /* RETURN */
	unsigned int *height)    /* RETURN */
{
	int mask = NoValue;
	char *strind;
	unsigned int tempWidth, tempHeight;
	int tempX, tempY;
	char *nextCharacter;

	if ( (string == NULL) || (*string == '\0')) return(mask);
	if (*string == '=')
		string++;  /* ignore possible '=' at beg of geometry spec */

	strind = (char *)string;
	if (*strind != '+' && *strind != '-' && *strind != 'x') {
		tempWidth = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter) 
		    return (0);
		strind = nextCharacter;
		mask |= WidthValue;
	}

	if (*strind == 'x' || *strind == 'X') {	
		strind++;
		tempHeight = ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return (0);
		strind = nextCharacter;
		mask |= HeightValue;
	}

	if ((*strind == '+') || (*strind == '-')) {
		if (*strind == '-') {
  			strind++;
			tempX = -ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return (0);
			strind = nextCharacter;
			mask |= XNegative;

		}
		else
		{	strind++;
			tempX = ReadInteger(strind, &nextCharacter);
			if (strind == nextCharacter)
			    return(0);
			strind = nextCharacter;
		}
		mask |= XValue;
		if ((*strind == '+') || (*strind == '-')) {
			if (*strind == '-') {
				strind++;
				tempY = -ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
				mask |= YNegative;

			}
			else
			{
				strind++;
				tempY = ReadInteger(strind, &nextCharacter);
				if (strind == nextCharacter)
			    	    return(0);
				strind = nextCharacter;
			}
			mask |= YValue;
		}
	}
	
	/* If strind isn't at the end of the string the it's an invalid
		geometry specification. */

	if (*strind != '\0') return (0);

	if (mask & XValue)
	    *x = tempX;
 	if (mask & YValue)
	    *y = tempY;
	if (mask & WidthValue)
            *width = tempWidth;
	if (mask & HeightValue)
            *height = tempHeight;
	return (mask);
}
