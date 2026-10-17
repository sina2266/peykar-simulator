// =======================================================================
//
//                     <IV-Mac/window.h>
//
//  Machintosh implimentation of the InterViews Window classes.
//
//	THE MANAGED WINDOW AND OTHER SUB-WINDOW CLASSES HAVE NOT BEEN IMPLEMENTED
//
//  The ManagedWindowRep is associated with the ManagedWindow class, which
//  is a class of window that interacts with the window manager.  Although
//  there is no seperate "Window Manager" in MS-Windows, the functionality
//  present in that class has equivalents in MS-Windows.
//
//
// 1.1
// $Date:   4 AUG 1996 
//
// =======================================================================

#ifndef iv_mac_window_h
#define iv_mac_window_h


#define WindowPtr WindowRef

#include <InterViews/iv.h>
#include <InterViews/geometry.h>
#include <InterViews/window.h>
#include <OS/list.h>

#if !carbon
#include <QDOffscreen.h>
#endif

class Window;
class Bitmap;
class Handler;
class ManagedWindow;
class Style;
class Glyph;
class Cursor;
class Event;

#if carbon
#define USE_SIOUX_WINDOW 0
#else
#define USE_SIOUX_WINDOW 1  //Enables one to remove SIOUX specific support
#endif

declarePtrList(MACWindowRepList, WindowRep);


class MACcursorPtrList;
class MACcreateParams;

class MACwindow
{
public:
	MACwindow();
	virtual ~MACwindow();
	
	Boolean			hasGrow;			/* true if has grow box		 */
	ControlHandle	horizScroll;		/* horizontal scroll bar */
	ControlHandle	vertScroll;			/* vertical scroll bar */
	
	enum { MODAL = 1, MODELESS, MOVABLE_MODAL};

	bool update_;					//whether window currently in list
	
	//Points defining the maximum and minimus sizes of window
	Point shrink_;
	Point stretch_;

	// ---- typical window operations ----
	void bind();                     		// bind to an MS-Windows window
	void unbind();							// unbind from MS-Windows window
	bool bound();						// bound to MS-Windows window?
	bool map();							// map the window to the display
	void unmap();							// unmap window from display
	bool isMapped();						// is window visible?	

	//functions to determine new window placement
	int next_window_x(void);
	int next_window_y(void);
	bool placed_;
	int left_;
	int bottom_;
	
	// MAC specific calls
	void contentRect (void *data);
	void drawGrowIcon(void);
	void activate(bool activ);
	void repair(void);
	void update(void);
#if carbon
	void doDrag(EventRef theEvent);
	void doGrow(EventRef theEvent);
	void doBackgroundClick(EventRef theEvent);
#else
	void doDrag(EventRecord * theEvent);
	void doGrow(EventRecord* theEvent);
	void doBackgroundClick(EventRecord * theEvent);
#endif
	void adjustScrollBar (ControlHandle theControl);
	void doZoom (void *data);
	long MACpaint(void);
	
	void doubleBuffer(bool b);
	
	Window* getIvWindow() const;
#if carbon
	void setport() {SetPort(GetWindowPort(theMacWindow_));}
#else
	void setport() {SetPort(theMacWindow_);}
#endif
	
	// ----- window class parameters ----
	WindowPtr macWindow();
	
	// ----- static functions -----
#if USE_SIOUX_WINDOW
	static bool noInterviewsWindows(void);
	static void setNoInterviewsWindows(bool windows);
#endif

	static bool isOurWindow(WindowPtr theWin);
	static short isDialog(WindowPtr macWindow);
	MACcreateParams * params_;					// creation parameters
		
	private:
		WindowPtr		  theMacWindow_;		// the actual window associated with this
		GWorldPtr 		  buffer_bitmap_;
		bool 		  doubleBuffered_;		// is window double-buffered?
};

// The following are the window/class creation parameters.  
class MACcreateParams
{
public:
	MACcreateParams();
	~MACcreateParams();
	
	void titleOf(const char*);      // set window title
	const char* titleOf();          // fetch window title
	Rect* bounds_;					//size of window
	int  id_;						//type of window
	bool away_;						//whether a window can be closed
	long refCon_input_;  			//what goes into refcon field
	Point where_;
#if carbon
	WindowClass wclass_;
#endif

private:
	char* title_;					//string title
};

// ---- inline functions of MACwindow ----
inline bool MACwindow::bound()
	{ return (theMacWindow_) ? 1 : 0; }
inline WindowPtr MACwindow::macWindow()
	{ return theMacWindow_; }
inline void MACwindow::doubleBuffer(bool b)
	{ doubleBuffered_ = b; }


class WindowRep : public MACwindow
{
public:
	WindowRep(Window*);
	~WindowRep();

	Window* ivWindowOf()                    // associated IV window
		{ return win; }
	
#if carbon
	long MACinput(EventRef theEvent, int type, int button);
	void doGrow(EventRef theEvent);	
#else
	long MACinput(EventRecord* theEvent, int type, int button);
	void doGrow(EventRecord* theEvent);	
#endif
	void bind();
	const Allocation& getAllocation(void);
	static MACWindowRepList update_list;
	
	MACcursorPtrList* cursor_stack_;
	Cursor* cursor_;
	Handler* close_callback_;
	static WindowRep* rc(WindowPtr wp) {
#if carbon
	return (WindowRep*)GetWRefCon(wp);
#else
	return (WindowRep*)((CWindowPeek)wp)->refCon;
#endif
	}
	bool request_on_resize_;
private:	
	Window* win;					// associated InterViews window
	
	
};

inline const Allocation& WindowRep::getAllocation(void)
	{ return win->allocation_;} 
// #######################################################################
// ################## ManagedWindowRep
// #######################################################################
class ManagedWindowRep
{
public:
	ManagedWindowRep(ManagedWindow*);
	~ManagedWindowRep();
	
	void errorMessage(const char* msg);

	// ---- Windows messages recognized ----
	

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
