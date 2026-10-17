#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =======================================================================
//
//                     <IV-Mac/Window.c>
//
//  Macintosh implementation of the InterViews Window classes.  Primary
//  event handling is contaied within the event.c file, but the actual
//  implementation of the different events on the windows is done by the
//  MACwindow class.  Mouse down/up and keydown events within the content
//  region of an active window are handled by Interviews event handler, and
//  are passed by the function MACinput.  
//
//
// 1.7
// $Date:   4 Aug 1996 
//

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
/*
 * THIS FILE CONTAINS PORTIONS OF THE Fresco DISTRIBUTION THAT 
 * CONTAINED THE FOLLOWING COPYRIGHT:
 */
// Copyright (c) 1995 Silicon Graphics, Inc.
// Copyright (c) 1995 Fujitsu, Ltd.
//
// Permission to use, copy, modify, distribute, and sell this software and 
// its documentation for any purpose is hereby granted without fee, provided
// that (i) the above copyright notices and this permission notice appear in
// all copies of the software and related documentation, and (ii) the names of
// Silicon Graphics and Fujitsu may not be used in any advertising or
// publicity relating to the software without the specific, prior written
// permission of Silicon Graphics and Fujitsu.
//
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
//
// IN NO EVENT SHALL SILICON GRAPHICS OR FUJITSU BE LIABLE FOR
// ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
// OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
// LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
// OF THIS SOFTWARE.
//
// =======================================================================

#include <InterViews/glyph.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <InterViews/canvas.h>
#include <InterViews/event.h>
#include <InterViews/handler.h>
#include <InterViews/hit.h>
#include <InterViews/cursor.h>
#include <InterViews/color.h>
#include <InterViews/display.h>
#include <InterViews/action.h>
#include <InterViews/box.h>
#include <OS/list.h>
#include <OS/string.h>
#include <OS/table.h>
#include <OS/math.h>
#include <IV-Mac/window.h>
#include <IV-Mac/canvas.h>
#include <IV-Mac/event.h>
#include <IV-Mac/cursor.h>
#include <IV-Mac/color.h>
#include <IV-Mac/session.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
extern "C" {void debugfile(const char*, ...);}

#define MenuID altDBoxProc
//#define MenuID plainDBox

//This is used to ensure that the Inteviews loop will not be called the SIOUX window
//if there are no Interviews windows
#if USE_SIOUX_WINDOW
static bool NO_IV_WINDOWS = true;
#endif

//Variable to keep track of next window placement
static int NEXT_WINDOWS_PLACE_X = 0;
static int NEXT_WINDOWS_PLACE_Y = 40;

// --- global event for rapid processing ----
static Event* input_e = 0;
static EventRep* input_er = 0;

// Names of the classes that get registered by InterViews applications.
// THESE NAMES AREN'T CURRENTLY USED?IMPLEMENTED
const char* WINDOW_CLASSNAME = "ivWindow";
const char* POPUP_CLASSNAME = "ivPopupWindow";
const char* TOPLEVEL_CLASSNAME = "ivTopLevelWindow";

// List of damged windows
implementPtrList(MACWindowRepList, WindowRep)
MACWindowRepList WindowRep::update_list;

// ---- templates ----
declarePtrList(MACcursorPtrList, Cursor)
implementPtrList(MACcursorPtrList, Cursor)

#if carbon
extern "C" {
#define NTYPE 6
static EventTypeSpec etype[] = {
	{kEventClassMouse, kEventMouseDown},
	{kEventClassMouse, kEventMouseMoved},
	{kEventClassMouse, kEventMouseDragged},
	{kEventClassMouse, kEventMouseUp},
	{kEventClassKeyboard, kEventRawKeyDown},
	{kEventClassKeyboard, kEventRawKeyRepeat}
};
	                                
static OSStatus ehandler(EventHandlerCallRef x, EventRef er, void* v) {
//printf("ehandler class=%d kind=%d\n", GetEventClass(er), GetEventKind(er));
	OSStatus result;
	result = CallNextEventHandler(x, er);
	if (result == eventNotHandledErr) {
		input_er->setEventRef(er);
		input_e->handle();
		result = noErr;
	}
//	UInt32 ek;
//	ek = GetEventKind(er);
//	printf("%d\n", ek);
	// wish we could do following only on last of a sequence of events.
	Session::instance()->screen_update();
	return result;
}

static EventTypeSpec showtype[] = {{kEventClassWindow, kEventWindowShown}};
static OSStatus show_handler(EventHandlerCallRef x, EventRef er, void* v) {
        OSStatus result;
	WindowRef wr;
//        printf("kEventWindowShown\n");
        result = GetEventParameter(er, kEventParamDirectObject, typeWindowRef,
		NULL, sizeof(WindowRef), NULL, &wr);
	WindowRep* wrep = (WindowRep*)GetWRefCon(wr);
	wrep->ivWindowOf()->canvas()->damage_all();
	result = noErr;
        return result;
}

}

int iv_carbon_in_menu_;

void iv_carbon_dialog_handle(WindowRef w) {
	EventRef er;
	OSStatus result;
	result = ReceiveNextEvent(0, NULL, kEventDurationForever, true, &er);
//printf("%d %c%c%c%c %d %d\n", result,
//GetEventClass(er)>>24,GetEventClass(er)>>16,GetEventClass(er)>>8,GetEventClass(er)>>0,
//GetEventKind(er), IsWindowActive(w));
	
	WindowRef wr;
	if (GetEventClass(er) == kEventClassMouse
		&& GetEventParameter(er, kEventParamWindowRef,
			typeWindowRef, NULL, sizeof(wr), NULL, &wr) == 0) {
		if (FrontWindow() != w) {
//printf("SelectWindow\n");
//			SelectWindow(w);
		}
	}else if ( GetEventClass(er) == kEventClassKeyboard) {
		wr = FrontWindow();
	}else{
		wr = nil;
	}
//printf("dialog iv_carbon_in_menu = %d\n", iv_carbon_in_menu_);
	if (wr == w || iv_carbon_in_menu_) {
		SendEventToEventTarget(er, GetEventDispatcherTarget());
	}
	ReleaseEvent(er);
}

#endif

// #######################################################################
// #################  MACcreateParams class
// #######################################################################

MACcreateParams::MACcreateParams()
{
	//These settings are defaults ... the glyph sets many of these in WindowRep::bind
	bounds_ = new Rect;
	title_ = 0;
	where_.v = 70;
	where_.h = 555;
	id_ = documentProc;
	away_ = true;
	refCon_input_ = 0;
	SetRect(bounds_, 0, 0, 200 , 150);
	titleOf("Neuron\0");
#if carbon
	wclass_ = kDocumentWindowClass;
	id_ =	kWindowCloseBoxAttribute| kWindowResizableAttribute | kWindowCollapseBoxAttribute;
#endif
}

MACcreateParams::~MACcreateParams()
{
	if(title_)
		delete title_;
	if(bounds_)
		delete bounds_;
}

void MACcreateParams::titleOf(const char* t)
{
	if(t){
		if (title_)
			delete title_;
		title_ = new char[strlen(t)+1];
		strcpy(title_, t);
	}
}

const char* MACcreateParams::titleOf()
{
	return title_;
}

// #######################################################################
// #################  MACwindow class
// #######################################################################

// -----------------------------------------------------------------------
// constructors and destructors
// -----------------------------------------------------------------------
MACwindow::MACwindow()
{
 	theMacWindow_ = nil;
 	buffer_bitmap_ = nil;
 	doubleBuffered_ = true;
 	params_ = new MACcreateParams;					
	hasGrow = true;			
	horizScroll = nil;		
	vertScroll = nil;
	update_ = false;
	placed_ = false;			
}

MACwindow::~MACwindow()
{
	if(params_)
		delete params_;
		
	if (buffer_bitmap_) {
        DisposeGWorld(buffer_bitmap_);
    }
    //shouldn't have to call this ... unbind should take care of disposing 
    //of the window.  It is merely a safety precaution.
    if (theMacWindow_)
    	DisposeWindow(theMacWindow_);
}


//The next_window functions place new windows according to Macintosh standards
int MACwindow::next_window_x(void){
	if (placed_) return left_;
	int this_window_position = NEXT_WINDOWS_PLACE_X;
	NEXT_WINDOWS_PLACE_X += 20;
	if(NEXT_WINDOWS_PLACE_X >= getIvWindow()->display()->pwidth())
		NEXT_WINDOWS_PLACE_X = 20;
	return this_window_position;
}

int MACwindow::next_window_y(void){
	if (placed_) {
		Window* w = getIvWindow();
		int t = bottom_ - w->display()->to_pixels(w->height());//bottom_ measured from top of screen
		return (t>40) ? t : 40;
	}
	int this_window_position = NEXT_WINDOWS_PLACE_Y;
	NEXT_WINDOWS_PLACE_Y += 20;
	if(NEXT_WINDOWS_PLACE_Y >= getIvWindow()->display()->pheight())
		NEXT_WINDOWS_PLACE_Y = 40;
	return this_window_position;
}

// -----------------------------------------------------------------------
// Window binding functions.  bind() actually creates the window and binds
// it to this object.  unbind() removes the attachment to the Macintosh 
// window and destroys it.
// -----------------------------------------------------------------------

void MACwindow::bind()
{

//If we have a SIOUX_window ... set up event looping to support both windows
#if USE_SIOUX_WINDOW
	MACwindow::setNoInterviewsWindows(false);
#endif

	long totalSpace, contigSize;
	Requisition r;
	
	// ---- if already bound... nothing to do ----
	if (theMacWindow_)
		return;

	// ---- check for available memory ----
	PurgeSpace(&totalSpace, &contigSize);

#if carbon
#else	
	if (contigSize < (sizeof(CWindowRecord))){
		printf("Error Message");
		exit(1);
	}
#endif
	
	//pointer to the MACwindow structure in the Refcon field
	params_->refCon_input_ = (long)this;

	// ---- create the window ----
#if carbon
	OSStatus st = CreateNewWindow(
		params_->wclass_,
//		(params_->away_?kWindowCloseBoxAttribute:0) | kWindowResizableAttribute | kWindowCollapseBoxAttribute,
		params_->id_,
		params_->bounds_,
		&theMacWindow_
	);
	if (st != noErr) {
		printf("CreateNewWindow error %d\n", st);
	}
	SetWRefCon(theMacWindow_, params_->refCon_input_);
	Str255 ti;
	CopyCStringToPascal(params_->titleOf(), ti);
	SetWTitle(theMacWindow_, ti);
	EventHandlerUPP hupp = NewEventHandlerUPP(ehandler);
	InstallWindowEventHandler(theMacWindow_, hupp, NTYPE, etype, 0, NULL);
	hupp = NewEventHandlerUPP(show_handler);
	InstallWindowEventHandler(theMacWindow_, hupp, 1, showtype, 0, NULL);
#else
	theMacWindow_ = NewCWindow(nil,						//where the window record will be stored
							  params_->bounds_,			//size of window
							  C2PStr((Ptr)params_->titleOf()),	//string title
							  false,					//not visible on screen till mapped
							  params_->id_,				//type of window
							  nil,						//places window in front
							  params_->away_,			//whether a window can be closed
							  params_->refCon_input_);	//associated window
#endif
	// ---- move the window  ----
	//This probably needs to be changed to conform with Mac "human interface" standards
	//but it gives you some flexibility
	if(theMacWindow_){
		MoveWindow(theMacWindow_, next_window_x(), next_window_y(), false);
//		MoveWindowStructure(theMacWindow_, next_window_x(), next_window_y());
	} else {
		printf("Memory allocation error");
		exit(1);
	}
	
	//Force an update ... otherwise window stays blank unitl a key press
	getIvWindow()->canvas()->damage_all();
}

void MACwindow::unbind()
{
	if (!theMacWindow_) {
//		MessageBox(NULL,"MWwindow::unbind has no binding", "zzz", MB_OK);
		return;
	}

	if (update_) {
		int i, cnt = WindowRep::update_list.count();
		for (i=0; i < cnt; ++i) {
			if (WindowRep::update_list.item(i) == (WindowRep*)this) {
				WindowRep::update_list.remove(i);
				break;
			}
		}
	}
	
	// ---- close window and destroy the memory allocated ----
	DisposeWindow(theMacWindow_);
	theMacWindow_ = 0;
}

bool MACwindow::map()
{
	if(theMacWindow_){
		ShowWindow(theMacWindow_);
		SelectWindow(theMacWindow_);
		getIvWindow()->canvas()->damage_all();
		//Needed for Neuron
		Event e;
		getIvWindow()->receive(e);
		return 1;
	}
	
	return 0;
}

void MACwindow::unmap()
{
	if(theMacWindow_)
		HideWindow(theMacWindow_);
}

bool MACwindow::isMapped()
{
	if(theMacWindow_){
#if carbon
		return IsWindowVisible(theMacWindow_)?true:false;
#else
		return(((CWindowPeek)theMacWindow_)->visible);
#endif
	} else {
		return 0;
	}
}



//SIOUX Specific stuff
#if USE_SIOUX_WINDOW
bool MACwindow::noInterviewsWindows(void){
	return NO_IV_WINDOWS;
}

void MACwindow::setNoInterviewsWindows(bool windows){
	NO_IV_WINDOWS = windows;
}
#endif


// getIvWindow -
//	Since MACwindow is always the superclass of a WindowRep we are able to
//make this type-cast in order to get the associated InterViews Window.
Window * MACwindow::getIvWindow() const{
#if 0
	if(params_->refCon_input_){
		return ((WindowRep*)params_->refCon_input_)->ivWindowOf();
	} else {
		return nil;
	}
#else
	return ((WindowRep*)this)->ivWindowOf();
#endif
}

// isOurWindow -
// 	I honestly don't know how effective/necessary this function is yet ... may fail to
// distinguish between one of our graphics windows and the SIOUX window
bool MACwindow::isOurWindow(WindowPtr theWin){
	if(theWin){
#if carbon
		return GetWindowKind(theWin) == userKind;
#else
		return(((CWindowPeek)theWin)->windowKind == userKind);
#endif
	} else {
		return false;
	}
}

//This function is necessary since the Macintosh handles events for dialogs
//differently than events for normal windows.  DIALOG EVENT HANDLING STILL NEEDS
//TO BE IMPLEMENTED.
short MACwindow::isDialog(WindowPtr macWindow)
{
	Boolean result = 0;
	short	windowVariant;

	if (macWindow)
	{
#if carbon
		if (GetWindowKind(macWindow) == dialogKind)	/* window accessor */
#else
		if (((CWindowPeek)macWindow)->windowKind == dialogKind)	/* window accessor */
#endif
		{
			windowVariant = GetWVariant(macWindow);
			switch (windowVariant)
			{
				case dBoxProc:
				case plainDBox:
				case altDBoxProc:
					result = MODAL;
					break;
				
				case noGrowDocProc:
					result = MODELESS;
					break;
					
				case movableDBoxProc:
					result = MOVABLE_MODAL;
					break;
			}
		}
	}
	return(result);
}

// contentRect -
// 	Useful for actually finding the content area of the window when clipping.
// May not be terribly useful, however, if Macintosh scroll bars are ignored.
void MACwindow::contentRect (void *data){
	Rect			*contentRect = (Rect*)data;
#if carbon
	GetPortBounds(GetWindowPort(theMacWindow_), contentRect);
#else
	*contentRect = theMacWindow_->portRect;
#endif
	if (vertScroll){
		contentRect->right -=15;
		if (contentRect->right < contentRect->left)
			contentRect->right = contentRect->left;
	}
	if (horizScroll){
		contentRect->bottom -=15;
		if (contentRect->bottom < contentRect->top)
			contentRect->bottom = contentRect->top;
	}
}

// drawGrowIcon -
// 	This function is one that I had some trouble with.  It is designed to prevent you from
// drawing a blanked out scroll bar when you don't need one, but since we don't have any
// scroll bars ... DrawGrowIcon is never called.  That's where we get our mysterious
// grow corner from.  We could just force the issue by drawing the empty scroll bars and 
// grow icon, but they must be maintained.  We also would make some of the drawing in the
// canvas look awkward.	
void MACwindow::drawGrowIcon(void){
	RgnHandle		savedClip, contentRgn, windowRgn;
	Rect			contentRct;

#if 1	
/* create necessary regions */
	savedClip = NewRgn();
	contentRgn = NewRgn();
	windowRgn = NewRgn();

	GetClip(savedClip);

/* get content region */
	contentRect(&contentRct);
	RectRgn(contentRgn, &contentRct);
	
/* exclude it from the clip */
#if carbon
	Rect winRect;
	GetPortBounds(GetWindowPort(theMacWindow_), &winRect);
	RectRgn(windowRgn, &winRect);
#else
	RectRgn(windowRgn, &(theMacWindow_->portRect));
#endif
	DiffRgn(windowRgn, contentRgn, contentRgn);
	SetClip(contentRgn);
	
	DrawGrowIcon(theMacWindow_);
	
	SetClip(savedClip);
	
	DisposeRgn(savedClip);
	DisposeRgn(contentRgn);
	DisposeRgn(windowRgn);
#else
	if(hasGrow)
		DrawGrowIcon(theMacWindow_);
#endif
}

// activate -
// 	Responds to an activate event from the system on one of our windows
void MACwindow::activate(bool activ){
	short			hiliteCode;
	ControlHandle	theControl;
	GrafPtr			oldPort;
	
	GetPort(&oldPort);
	setport();
		
#if carbon
	// I wish I knew what I was doing
	GetRootControl(theMacWindow_, &theControl);
	if (activ) {
		ActivateControl(theControl);
	}else{
		DeactivateControl(theControl);
	}
#else
	if (activ)	/* activating window */
	{
		hiliteCode = 0;
	} else {
		hiliteCode = 255;
	}
	
	//theControl = ((CWindowPeek)theMacWindow_)->controlList;
	theControl = GetControlListFromWindow(theMacWindow_);
	while(theControl)
	{
		if ((theControl == horizScroll) || (theControl == vertScroll)){
			if (activ)
			{
				ShowControl(theControl);
			} else {
				HideControl(theControl);
			}
		} else {
			HiliteControl(theControl, hiliteCode);
		}
		theControl = (**theControl).nextControl;
	}
#endif
	
	if (hasGrow)
	{
		drawGrowIcon();
	}

	SetPort(oldPort);

/* allow window to modify menu bar */
	if(activ){
		//UpdateMenusHook(); 
	}
}

// repair -
// 	This function responds to the repair list that is maintained and cycled through
// after every event.  It redraws the windows damaged region
void MACwindow::repair(void){
	GrafPtr			oldPort;
	Rect			contentRct;
	
	GetPort(&oldPort);
	setport();
	MACcanvas* can = (MACcanvas*) getIvWindow()->canvas();
#if carbon
	RgnHandle visRgn;
	visRgn = NewRgn();
	GetPortVisibleRegion(GetWindowPort(theMacWindow_), visRgn);
	UpdateControls(theMacWindow_, visRgn);
//	DisposeRgn(visRgn);
#else	
	UpdateControls(theMacWindow_, theMacWindow_->visRgn);
#endif
	
//	Rect * damage = can->getDamage();
//	debugfile("Damage area is l: %d r: %d t: %d b: %d\n", damage->left, damage->right, damage->top, damage->bottom);
	/* draw contents */
	MACpaint();
	
	if (hasGrow){
		drawGrowIcon();
	}
#if carbon
	Rect r;
	GetPortBounds(GetWindowPort(theMacWindow_), &r);
	ValidWindowRect(theMacWindow_, &r);
	QDFlushPortBuffer(GetWindowPort(theMacWindow_), visRgn);
	DisposeRgn(visRgn);
#else	
	ValidRect(&theMacWindow_->portRect);
#endif

	
	can->clearDamage();
	

	SetPort(oldPort);
}	

// update -
// 	This function responds to a system event of updating a window.  It is modeled after
// the Fresco code.  An offscreen graphics world is used for double buffering.
void MACwindow::update(void){
	GrafPtr			oldPort;
	Rect			contentRct;
	
	BeginUpdate(theMacWindow_);

#if carbon
		SetGWorld(GetWindowPort(theMacWindow_), GetMainDevice());
		Rect r;
		GetPortBounds(GetWindowPort(theMacWindow_), &r);
		ClipRect(&r);
		RgnHandle vr;
		vr = NewRgn();
		GetPortVisibleRegion(GetWindowPort(theMacWindow_), vr);
		Rect pr;
		GetPortBounds(GetWindowPort(theMacWindow_), &pr);
#else	
		SetGWorld((CGrafPort*) theMacWindow_, GetMainDevice());
		Rect r = theMacWindow_->portRect;
		ClipRect(&r);	
		RgnHandle vr = theMacWindow_->visRgn;
		Rect pr = theMacWindow_->portRect;
#endif
		Rect content;
		content.top = pr.top;
		content.left = pr.left;
		contentRect(&content);
#if 0
		if (vertScroll){
			content.right = pr.right - 15;
		} else {
			content.right = pr.right;
		}
		if (horizScroll){
			content.bottom = pr.bottom - 15;
		} else {
			content.bottom = pr.bottom;
		}
#endif
		RgnHandle cr = NewRgn();
		RectRgn(cr, &content);
		RgnHandle ir = NewRgn();
		SectRgn(vr, cr, ir);
		if (! EqualRgn(vr, ir)) {
		    // only controls need to be updated
			RgnHandle wr = NewRgn();
			DiffRgn(vr, cr, wr);
			EraseRgn(wr);
			UpdateControls(theMacWindow_, wr);
			if (hasGrow) {
				DrawGrowIcon(theMacWindow_);
			}
			DisposeRgn(wr);
		}
		if (! EmptyRgn(ir)) {
#if carbon
			Rect damage;
			GetRegionBounds(ir, &damage);
#else
		    Rect damage = (*ir)->rgnBBox;
#endif
		    Boolean good = false;
		    
//This fresco implementation is probably better/faster, but having trouble with the size
//of the pixmap ... I think.
		    if (0 /*buffer_bitmap_ != nil*/) {
				PixMapHandle pixmap = GetGWorldPixMap(buffer_bitmap_);
				good = LockPixels(pixmap);
				if (good) {
			    	// Blast the damaged region from the buffer to the window.
					CopyBits(
#if carbon
						GetPortBitMapForCopyBits(buffer_bitmap_), GetPortBitMapForCopyBits(GetWindowPort(theMacWindow_)),
#else
						&GrafPtr(buffer_bitmap_)->portBits, &GrafPtr(theMacWindow_)->portBits,
#endif
						&damage, &damage, srcCopy, nil
					);
					UnlockPixels(pixmap);
				}
			}
			if (! good) {
				//THIS NEEDS WORK -- this rectangular call is ignored and paint is called
				//  I think that is why we can see the pixels being blasted to the screen 
				//  on resizing.
				MACcanvas* can = (MACcanvas*) getIvWindow()->canvas();
//				can->setDamage(&damage);
				can->addToDamage(&damage);
// 2/24/97 setDamage insufficient because the update make take place immediately after
// a redraw. ie  a popup window may be dismissed over the window it wants to draw a large
// part of.
				
				
				// damage content area
			    //Coord l = to_coord(damage.left - 1, X_axis);
			    //Coord b = to_coord(damage.bottom - 1, Y_axis);
			    //Coord r = to_coord(damage.right + 1, X_axis);
			    //Coord t = to_coord(damage.top + 1, Y_axis);
		    	//redraw(l, b, r - l, t - b);
		    	
		    	MACpaint();
		    }
		}
		DisposeRgn(cr);
		DisposeRgn(ir);
#if carbon
		DisposeRgn(vr);
#endif
	
	EndUpdate(theMacWindow_);
}	

// doDrag -
//	implements window dragging ... the neuron section is for the Print 
// Manager window
#if carbon
void MACwindow::doDrag(EventRef theEvent){
#else
void MACwindow::doDrag(EventRecord * theEvent){
#endif
	Rect		r;
	
	/* get dimensions of all monitors */
#if carbon
	Point p;
	GetRegionBounds(GetGrayRgn(), &r);
	DragWindow(theMacWindow_, EventRep::mouse_loc(theEvent), &r);
#else
	r = (**GetGrayRgn()).rgnBBox;
	/* drag the window within the rectangle defining all monitors */
	DragWindow(theMacWindow_, theEvent->where, &r);
#endif
	
	//Needed for Neuron
	Event e;
	getIvWindow()->receive(e);
}

// doGrow -
//	Responds to a click to the grow icon(which isn't currently displayed)
// It allows the user to resize the window.  Again there is neuron code to
// alert the print manager of the change, and the canvas is damages so that
// the window's contents will be updated.
#if carbon
void MACwindow::doGrow(EventRef theEvent){
#else
void MACwindow::doGrow(EventRecord* theEvent){
#endif
#if carbon
	Boolean result;
	Rect			rout;
#else
	long result;
#endif
	Rect			r;
	short			width, height;
	short			newWidth;
	short			newHeight;
	GrafPtr			oldPort;
	ControlHandle	theControl;

	width = getIvWindow()->canvas()->pwidth();
	height = getIvWindow()->canvas()->pheight();
	
	if(vertScroll){
		width += 16;	/* width of scroll bar */
	}
	
	if(horizScroll){
		height += 16;
	}
	
	/* Set minimum/maximum bounds for window */
	r.left   = shrink_.h;					
	r.top    = shrink_.v;
	r.right  = stretch_.h;
	r.bottom = stretch_.v;

	/* track mouse and resize window outline as we go */
#if carbon
	result = ResizeWindow(theMacWindow_, EventRep::mouse_loc(theEvent), &r, &rout);
#else
	result = GrowWindow(theMacWindow_, theEvent->where, &r);
#endif	
	if (result)
	{
		GetPort(&oldPort);
		setport();
	
		if (hasGrow)	/* must ensure this gets invalidated */
		{
			Rect growBoxRect;
#if carbon
			Rect r = rout;
			GetPortBounds(GetWindowPort(theMacWindow_), &r);
			SetRect(&growBoxRect, r.right - 15, r.bottom - 15, r.right, r.bottom);
			InvalWindowRect(theMacWindow_, &growBoxRect);
#else	
	   		SetRect(&growBoxRect, theMacWindow_->portRect.right - 15,
	    					  theMacWindow_->portRect.bottom - 15,
	    					  theMacWindow_->portRect.right,
	    					  theMacWindow_->portRect.bottom);
	    	InvalRect(&growBoxRect);
#endif
		}
		
#if carbon
#else
		newWidth = LoWord((long*)result);
		newHeight  = HiWord((long*)result);
		SizeWindow(theMacWindow_, newWidth, newHeight, true);
#endif

		/* resize scroll bars */
		theControl = (horizScroll);
		if (theControl){
			HideControl(theControl);
			adjustScrollBar(theControl);
			ShowControl(theControl);
		}
		theControl = (vertScroll);
		if (theControl){
			HideControl(theControl);
			adjustScrollBar(theControl);
			ShowControl(theControl);
		}

	if (hasGrow)
	{
		drawGrowIcon();
	}
	
	// reset clipping to entire window
	((MACcanvas*)getIvWindow()->canvas())->initClip();
	
	//force an update
	getIvWindow()->canvas()->damage_all();
	
	//Needed for Neuron
	Event e;
	getIvWindow()->receive(e);
	
	/* restore port */
	SetPort(oldPort);

	}
}	

// doZoom -
//	Currently doesNothing.  If one wanted to implement this code they would
// need to get the zoom.c file from EasyApp and include its relative headers.  Zoom.c
// provides a zooming algorithm which is supposedly good -- at least it is complex.  
// This has not been done because it wasn't a priority
void MACwindow::doZoom (void *data)
{
#if 0
	ControlHandle	theControl;
	short			zoomDirection = *(short*)data;
	Rect 			idealSize;
	
	idealSize.left   = 0;
	idealSize.top    = 0;
	idealSize.right  = docWidth;  //this would need to be calculated
	idealSize.bottom = docHeight; //this would need to be calculated
	
	if(vertScroll)
	{
		idealSize.right += 16;	/* width of scroll bar */
	}
	
	if(horizScroll)
	{
		idealSize.bottom += 16;
	}
	
	/* erase window and resize it, using Dean Yu's zooming code */
	//ZoomTheWindow( (WindowPeek)theMacWindow_, zoomDirection, &idealSize);

	/* resize scrollbars, if any */
	theControl = (horizScroll);
	if (theControl)
	{
		HideControl(theControl);
		adjustScrollBar(theControl);
		ShowControl(theControl);
	}
	theControl = (vertScroll);
	if (theControl)
	{
		HideControl(theControl);
		adjustScrollBar(theControl);
		ShowControl(theControl);
	}

	if (hasGrow)
	{
		drawGrowIcon();
	}
#endif
}

// doBackgroundClick -
//  	Handles mouse down events to content regions of non-active windows.
// Currently it just activates the window ... but it is set up to also handle drag and
// drop support.
#if carbon
void MACwindow::doBackgroundClick(EventRef theEvent){
#else
void MACwindow::doBackgroundClick(EventRecord * theEvent){
#endif
	Boolean			activate = true;	/* Bring window forward? Assume yes. */
	Point			localPoint;
	GrafPtr			oldPort;
	
	//  This function is an outline for drag and drop support of window items.  It is
	//  not currently implemented, but is based on the EasyApp design
	if(false)	
	{
		GetPort(&oldPort);
		setport();
	
		/* convert to local coordinates */
		//EasyGlobalToLocal(macWindow, theEvent->where, &localPoint);
	
		/* now see if click is in selected item */
		
		/*
			For example, let's say the user clicks an object. You would call
			WaitMouseMoved() to determine if a drag was beginning, and handle 
			it like this
			
#if carbon
			if (WaitMouseMoved(EventRep::mouse_loc(theEvent)))
			{
				easyWindow->DoDragItems(macWindow, theEvent);
				activate = false;      // don't activate background window
			}
#else
			if (WaitMouseMoved(theEvent->where))
			{
				easyWindow->DoDragItems(macWindow, theEvent);
				activate = false;      // don't activate background window
			}
#endif			
			This is for drag and drop only! Do not allow user to move anything
			in a background window other than dragging to a new window. If the
			drag begins in a background window and ends in the same window, 
			nothing should happen. The original sender window is NOT an acceptable
			destination when the sender is a background window.
			
			Notice that a click but not a drag on a background selection results
			in the window being activated!
		*/


		SetPort(oldPort);
	}

	if(activate)
	{
		SelectWindow(theMacWindow_);
	}
}

// adjustScrollBar -
//	Sets scroll tab and bar positioning appropriately in a window.
// Will be useful if Mac scroll bars are actually used/implemented.
void MACwindow::adjustScrollBar (ControlHandle theControl)
{
#if carbon
	Rect r;
	GetPortBounds(GetWindowPort(theMacWindow_), &r);
	short top = r.top;
	short left = r.left;
	short bottom = r.bottom;
	short right = r.right;
#else
	short			top 		= theMacWindow_->portRect.top;
	short			left		= theMacWindow_->portRect.left;
	short			bottom		= theMacWindow_->portRect.bottom;
	short			right		= theMacWindow_->portRect.right;
#endif
	short			max;

	if (theControl == vertScroll)
	{
		MoveControl(theControl, right - 15, -1);
		SizeControl(theControl, 16, bottom - 13);
		max = getIvWindow()->canvas()->pheight() - ((bottom - 15) - top);
	}
	else
	{
		MoveControl(theControl, -1, bottom - 15);
		SizeControl(theControl, right - 13, 16);
		max = getIvWindow()->canvas()->pwidth() - ((right - 15) - left);
	}

	if (max < 0)
	{
		max = 0;
	}
	SetControlMaximum (theControl, max);

}
// -----------------------------------------------------------------------
// MACpaint -
//	This is the primary function for drawing into a window.  It is basically
// Fresco code which has been adapted to Inteviews needs.  Drawing is limited
// to a damaged region which is maintained by the window. 
// -----------------------------------------------------------------------
long MACwindow::MACpaint()
{

	MACcanvas* can = (MACcanvas*) getIvWindow()->canvas();

     	PixMapHandle pixmap = 0;
	if (doubleBuffered_) {
        // Make sure the buffer is allocated.
		Rect c;
		c.left = 0;
		c.top = 0;
		c.right = can->pwidth();
		c.bottom = can->pheight();
	    if (buffer_bitmap_ == nil) {
	    	//printf("Set NewGWorld\n");
			QDErr error = NewGWorld(&buffer_bitmap_, 0, &c, nil, nil, 0);
			if (error != noErr)
				printf("Buffer Allocation Error\n");
		} else {
			GWorldFlags flags = UpdateGWorld(&buffer_bitmap_, 0, &c, nil, nil, 0);
			if (flags & gwFlagErr) {
	       		DisposeGWorld(buffer_bitmap_);
	       		//printf("I can't handle any more captain\n");
	       		buffer_bitmap_ = nil;
	       	} else if (flags & reallocPix) {
	       		can->setDamage(&c);
	       	}
	    }
			
		if (buffer_bitmap_ != nil) {
		    // Make sure the pixels are allocated
		    pixmap = GetGWorldPixMap(buffer_bitmap_);
			Boolean good = LockPixels(pixmap);
			if (! good) {
			    // Don't use double buffering.
				pixmap = 0;
			}
		}
	}    
	
	// Setup the graphics world for drawing.
	CGrafPtr cg;
	GDHandle gd;
	GetGWorld(&cg, &gd);
	if (pixmap != 0) {
		// Draw into the buffer.
		SetGWorld(buffer_bitmap_, nil);
	} else {
	    // Draw directly to the window without using double buffering.
#if carbon
		SetGWorld(GetWindowPort(theMacWindow_), GetMainDevice());
#else
	    SetGWorld((CGrafPort*)theMacWindow_, GetMainDevice());
#endif
	}
	
	// Paint the image.
	if (getIvWindow()->glyph()) {
//	Rect * dam = can->getDamage();
//	debugfile("draw Damage area is l: %d r: %d t: %d b: %d\n", dam->left, dam->right, dam->top, dam->bottom);
		can->beginPaint();
		getIvWindow()->glyph()->draw(can, ((WindowRep*)this)->getAllocation());
		can->endPaint();
//	 dam = can->getDamage();
//	debugfile("after draw Damage area is l: %d r: %d t: %d b: %d\n", dam->left, dam->right, dam->top, dam->bottom);
	}
	    
	if (pixmap != 0) {
//	Rect * dam = can->getDamage();
//	debugfile("paint Damage area is l: %d r: %d t: %d b: %d\n", dam->left, dam->right, dam->top, dam->bottom);
		// Blast the damaged region from the buffer to the window.
#if carbon
		SetGWorld(GetWindowPort(theMacWindow_), GetMainDevice());
		CopyBits(
			GetPortBitMapForCopyBits(buffer_bitmap_), GetPortBitMapForCopyBits(GetWindowPort(theMacWindow_)),
			can->getDamage(), can->getDamage(), srcCopy, nil
		);
		ValidWindowRect(theMacWindow_, can->getDamage());
#else
	  	SetGWorld((CGrafPort*)theMacWindow_, GetMainDevice());
		CopyBits(
			&GrafPtr(buffer_bitmap_)->portBits, &GrafPtr(theMacWindow_)->portBits,
			can->getDamage(), can->getDamage(), srcCopy, nil
		);
		ValidRect(can->getDamage());
#endif
		UnlockPixels(pixmap);
	}
	SetGWorld(cg, gd);
	return 0;
}

// #######################################################################
// #################  WindowRep class
// #######################################################################

// -----------------------------------------------------------------------
// constructors and destructors
// -----------------------------------------------------------------------
WindowRep::WindowRep(Window* w)
{
	win = w;
	cursor_ = nil;
	close_callback_ = nil;
	// Create the global event used by MACinput() if it hasn't yet
	// been allocated.
	if (! input_e)
	{
		input_e = new Event;
		input_er = (input_e) ? input_e->rep() : 0;
		if (!input_e || !input_er){
			printf("allocation failure");
			exit(1);
		}
	 }	
}

WindowRep::~WindowRep()
{
	// When closebox is selected, need to dismiss the Window. And don't want to
	// cast to a ManagedWindow.
	Resource::unref_deferred(close_callback_);	
}

// -----------------------------------------------------------------------
// When binding to the MAC-Window, we also bind
// this WindowRep to the CanvasRep, so that it can handle things like
// invalidation of parts of the window.
//
// The Window class (and all derived classes) stash their desired window
// dimensions into the canvas, so we get our desired size from there.
// -----------------------------------------------------------------------
void WindowRep::bind()
{
	Requisition r;
	// ---- bind to the CanvasRep ----
	MACcanvas* c = (MACcanvas*) win->canvas_;
	c->bind(this);
	Display* d = getIvWindow()->display();

	//setup window sizing for glyph creation
	win->glyph_->request(r);
	params_->bounds_->bottom = d->to_pixels(r.y_requirement().natural(), Dimension_Y);
	params_->bounds_->right = d->to_pixels(r.x_requirement().natural(), Dimension_X);
	
	//setup window expansion paramaters
	shrink_.h = d->to_pixels((r.x_requirement().natural() - r.x_requirement().shrink()), Dimension_X);
	shrink_.v = d->to_pixels((r.y_requirement().natural() - r.y_requirement().shrink()), Dimension_Y);
	stretch_.h = d->to_pixels(Math::min((r.x_requirement().natural() + r.x_requirement().stretch()),(float)2000.), Dimension_X);
	stretch_.v = d->to_pixels(Math::min((r.y_requirement().natural() + r.y_requirement().stretch()), (float)2000.), Dimension_Y);
		
	// ---- bind to MS-Window ----
	MACwindow::bind();
	
	// ---- initialize for allocation ----
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
}

//------------------------------------------------------------------------
// This function is a wrapper for the MACwindow::doGrow function.  The
// MACwindow function does the actual resizing, but it does not have access
// to the InterViews Windows private member.  It can therefore not allocate
// new space for the Glyph.
//------------------------------------------------------------------------
#if carbon
void WindowRep::doGrow(EventRef theEvent){
#else
void WindowRep::doGrow(EventRecord* theEvent){
#endif	
	MACwindow::doGrow(theEvent);
	
	// ---- invalidate stored canvases ----
	win->glyph_->undraw();

	// ---- initialize for allocation ----
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
}


// -----------------------------------------------------------------------
// process a mouse/keyboard event.  A global object is used to decrease the
// construction/destruction time since mouse movement will generate a lot
// of these!  "input_er" is a pointer to the EventRep of the global Event
// "input_e".  No pointer checks are made as the library will bail out if
// the Event and associated EventRep can't be constructed in the first
// place.
// -----------------------------------------------------------------------
#if carbon
long WindowRep::MACinput(EventRef theEvent, int type, int button){
#else
long WindowRep::MACinput(EventRecord* theEvent, int type, int button){
#endif
	Point 	theMouse;
	GrafPtr oldPort;
	// ---- set the EventRecord ----
#if carbon
	input_er->setEventRef(theEvent);
#else
	input_er->setEventRecord(theEvent);
#endif
	input_er->type_ = type;
	input_er->win_ = win;
	input_er->button_ =  button;
	GetPort(&oldPort);
	setport();
#if carbon
	theMouse = EventRep::mouse_loc(theEvent);
#else
	theMouse = theEvent->where;
#endif
	GlobalToLocal(&theMouse);
	input_er->localMouseLocation_ = theMouse;
	SetPort(oldPort);
//printf("Macinput %lx %lx %d %d %d\n", (long)win, (long)win->canvas(), type, theMouse.h, theMouse.v);	
    // ---- go process it ----
	EventRep::handleCallback(*input_e);

	return 0;
}

// 
// #######################################################################
// #####################  Window class
// #######################################################################


Window::Window(Glyph* g)
{
	rep_ = new WindowRep(this);
	rep_->request_on_resize_ = false;

	canvas_ = new MACcanvas;
	((MACcanvas*)canvas_)->bind(rep_);
	glyph_ = g;
	glyph_->ref();
	style_ = nil;
	focus_in_ = nil;
	focus_out_ = nil;
	
	//rep_->windowClass(WINDOW_CLASSNAME);
	//rep_->classStyle(CS_HREDRAW | CS_VREDRAW);
}

Window::Window(WindowRep* w)
{
	rep_ = w;
}

Window::~Window()
{
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

}

void Window::repair()
{
	//I am not sure if this is what is wanted ... this needs to be changed?
	rep_->repair();
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
	// no such thing under MS-Windows ... HAVEN'T THOUGHT ABOUT ON THE MAC
}

Display* Window::display() const
{
	Session* s = Session::instance();
    if(s){
		return s->default_display();
	} else {
		printf("Window::display error\n");
	}
	//Should not be reached
	return nil;
}

// -----------------------------------------------------------------------
// cursor functions
// -----------------------------------------------------------------------

void Window::cursor(Cursor* c)
{
     WindowRep& w = *rep();
	w.cursor_ = c;
	if (c == nil){
#if carbon
		SetCursor(*(arrow->rep()->theCursor));
#else
         SetCursor(&qd.arrow);
#endif
	}else{
		if((c->rep()) && (c->rep()->theCursor)){
        	SetCursor(*(c->rep()->theCursor));
        } else {
        	printf("This cursor not implemented\n");
        }
	}
}

Cursor* Window::cursor() const
	{ return rep_->cursor_; }

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
   w.placed_ = true;
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
	w.placed_ = true;
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
#if carbon
		Rect r;
		GetWindowBounds(w.macWindow(), kWindowStructureRgn, &r);
		return d->to_coord(r.left, Dimension_X);
#else
		// ---- get the Macintosh screen coordinates ----
		GrafPtr oldPort;
		WindowPtr theWin = w.macWindow();
		GetPort(&oldPort);
		w.setport();
		Point lowerLeft;
		
		lowerLeft.h = theWin->portRect.left;
		lowerLeft.v = theWin->portRect.bottom; 
		LocalToGlobal(&lowerLeft);
		SetPort(oldPort);
		  return d->to_coord(lowerLeft.h, Dimension_X);
#endif
		  
	}
	else
	{
		return d->to_coord(w.left_, Dimension_X);
	}
}

// Return the y-coordinate of the bottom side of the window in terms
// of InterViews display coordinates.  This is opposite the coordinate
// system used by Macintosh, and is therefore adjusted.
Coord Window::bottom() const
{
	WindowRep& w = *rep();
	Display* d = display();
	if (w.bound())
	{
#if carbon
		Rect r;
		GetWindowBounds(w.macWindow(), kWindowStructureRgn, &r);
		return d->height() - d->to_coord(r.bottom, Dimension_Y);
#else
		// ---- get the Macintosh screen coordinates ----
		GrafPtr oldPort;
		WindowPtr theWin = w.macWindow();
		GetPort(&oldPort);
		w.setport();
		Point lowerLeft;
		lowerLeft.h = theWin->portRect.left;
		lowerLeft.v = theWin->portRect.bottom; 
		LocalToGlobal(&lowerLeft);
		SetPort(oldPort);
		  	return (d->height() - d->to_coord(lowerLeft.v, Dimension_Y));
#endif
	}
	else
	{
			return d->height() - d->to_coord(w.bottom_, Dimension_Y);
	};
}


// -----------------------------------------------------------------------
// Window dimensions.  The canvas is queried to get the dimensions.  The 
// canvas checks the size of the windows portRect and returns a value.  
// Since there are borders/tile bars, appropriate values are added so that
// the returned window size is correct.
// -----------------------------------------------------------------------
Coord Window::width() const
{
#if carbon
	WindowRep& w = *rep();
	Display* d = display();
	if (w.bound()) {
		Rect r;
		GetWindowBounds(w.macWindow(), kWindowStructureRgn, &r);
		return d->to_coord(r.right - r.left, Dimension_X);
	}else{
		return 100;
	}
#else
	if (rep_->params_->id_ == MenuID) return canvas_->width();
	return ((canvas_->width()) + 3);
#endif
}

Coord Window::height() const
{
#if carbon
	WindowRep& w = *rep();
	Display* d = display();
	if (w.bound()) {
		Rect r;
		GetWindowBounds(w.macWindow(), kWindowStructureRgn, &r);
		return d->to_coord(r.bottom - r.top, Dimension_Y);
	}else{
		return 100;
	}
#else
	if (rep_->params_->id_ == MenuID) return canvas_->height();
	return ((canvas_->height()) + 17);
#endif
}

// -----------------------------------------------------------------------
// Window mapping functions
//
// Many operations upon the real window are postponed until a request is
// made to map the window to the display.  At this point, any attributes
// desired of the window have already been set, and we are ready to really
// create the window and map it.  If the window is already bound to an
// macWindow, than we simply map it.
// -----------------------------------------------------------------------
void Window::map()
{
	// ---- check to see if we are already mapped ----
    if (is_mapped())
		return;

	// ---- check to see if we are bound to a macWindow ----
	if (!bound())
	{
		if (style_ == nil)
			style(new Style(Session::instance()->style()));

		configure();
		default_geometry();
		compute_geometry();
		set_props();
		bind();
		String s;
		if (style_->find_attribute("name", s)) {
#if carbon
		Str255 s1;
		CopyCStringToPascal(s.string(), s1);
		SetWTitle(rep()->macWindow(), s1);
#else
		CopyString s1(s);
			SetWTitle(rep()->macWindow(), C2PStr((char*)s1.string()));
#endif
		} 
	 }

// This may be needed later in the Mac implementation...
#if 0
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
	if (is_mapped())
	{
		WindowRep& w = *rep();
		glyph_->undraw();
		w.unmap();
	}
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

void Window::set_attributes()  //this is not implemented in the MS-Windows Version
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

	return h;
}
// -----------------------------------------------------------------------
// Window::receive
//		This function is only called inside MAC native functions under the
// current implementation.  It is used as a function to communicte with the
// Window/Print Manager.
// -----------------------------------------------------------------------
bool Window::receive(const Event& e)
{
	//printf("Window::receive not implemented\n");
	return false;
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
	//The MAC seems to be handling this natively
	// however a menu may persist after a mouse release and if the next
	// click is off the application it should disappear. Since there is
	// no equivalent to SetCapture on mswin or XGrabPointer on X11
	// we make use of the application deactivate event in session.cpp
	// and that will check Event::grabber
}

void Window::ungrab_pointer() const
{
	//The MAC seems to be handling this natively
}

void Window::raise()
{
	printf("Window::raise not implemented\n");
}

void Window::lower()
{
	printf("Window::lower not implemented\n");
}

// Move the window to the position given by the InterViews display
// coordinates for the left and bottom.  These must be converted to
// Macintosh screen coordinates, which are different in the y-axis.
void Window::move(Coord left, Coord bottom)
{
	WindowPtr theWin = rep_->macWindow();
	Display* d = display();
	if(!d)
		printf("Error finding display in Window::move\n");
#if carbon
	Rect r;
	//GetPortBounds(GetWindowPort(theWin), &r);
	GetWindowBounds(theWin, kWindowStructureRgn, &r);
#else
	Rect r = theWin->portRect;
#endif
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	int y = (d->pheight() - d->to_pixels(bottom, Dimension_Y)) - height;
	int x = d->to_pixels(left, Dimension_X);

	// ---- move the window ----
	//x = (x > 0 ? x : 0);
	//y = (y > 0 ? y : 0);
	//MoveWindow(theWin, x, y, true);
	MoveWindowStructure(theWin, x, y);
}

void Window::resize()
{
	WindowPtr theWin = rep_->macWindow();
	Rect* curr_pos;

	if (rep_->request_on_resize_) {
	    Box::full_request(true);
	    glyph_->request(shape_);
	    Box::full_request(false);
	}

	// ----- determine desired size ----
	int width = canvas_->pwidth();
	int height = canvas_->pheight();

	// ---- resize the window ----
	SizeWindow(theWin, width, height, true);
	
	//force an update
	canvas_->damage_all();
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
		printf("Window::default_geometry not functioning properly\n");
		// NOT REACHED
    }
}

// Protected operation called by map()
void Window::compute_geometry() { } //not implemented by MS-Windows version

// protected function called by map()
void Window::set_props()	//not implemented by MS-Windows version
{
}

// protected function called by map()
void Window::do_map()
{
	WindowRep& w = *rep();
    w.map();
}



// #######################################################################
// 		THE FOLLOWING WINDOW TYPES HAVE NOT YET BEEN IMPLEMENTED.
// #######################################################################


// #######################################################################
// ##################### class ManagedWindow
// #######################################################################


ManagedWindow::ManagedWindow(Glyph* g) : Window(g)
{
	mrep_ = new ManagedWindowRep(this);
	mrep_->wm_delete_ = nil; // not used. see WindowRep::close_callback_;
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
// Window::height is now checking the type of mac window. So use that for now.
// -----------------------------------------------------------------------
Coord ManagedWindow::width() const
{ 
	return Window::width();
}

Coord ManagedWindow::height() const
{
	return Window::height();
}

// -----------------------------------------------------------------------
//  Handle window events.
// -----------------------------------------------------------------------

ManagedWindow* ManagedWindow::icon() const
	{ return nil; }
Bitmap* ManagedWindow::icon_bitmap() const
	{ return nil; }
Bitmap* ManagedWindow::icon_mask() const
	{ return nil; }
	

void ManagedWindow::icon(ManagedWindow* i)
{
   
}

void ManagedWindow::icon_bitmap(Bitmap* b)
{
   
}

void ManagedWindow::icon_mask(Bitmap* b)
{
	 
}

void ManagedWindow::iconify()
{
	
}

void ManagedWindow::deiconify()
{
	
}

void ManagedWindow::resize()
{
   
}

// -----------------------------------------------------------------------
// Install "focus in" and "focus out" handlers.  Activation of these
// handlers is through Window::receive().
// -----------------------------------------------------------------------
void ManagedWindow::focus_event(Handler* in, Handler* out)
{

}

void ManagedWindow::wm_delete(Handler* h)
{
	Resource::ref(h);
	Resource::unref(rep_->close_callback_);
	rep_->close_callback_ = h;
	
}

void ManagedWindow::set_props()
{
	
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


// #######################################################################
// ###################  class ApplicationWindow
// #######################################################################

ApplicationWindow::ApplicationWindow(Glyph* g) : ManagedWindow(g)
{
	
}

ApplicationWindow::~ApplicationWindow()
{
}

void ApplicationWindow::compute_geometry()
{

    
}

void ApplicationWindow::set_props()
{
	
}

// #######################################################################
// ############### class TopLevelWindow
// #######################################################################


TopLevelWindow::TopLevelWindow(Glyph* g) : ManagedWindow(g) 
{ 
	
}

TopLevelWindow::~TopLevelWindow() 
{ 
}

void TopLevelWindow::group_leader(Window* primary) 
{
    
}

Window* TopLevelWindow::group_leader() const 
{ 
	return nil; 
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
#if carbon
	rep_->params_->wclass_ = kPlainWindowClass;
	rep_->params_->id_ = 0;
#else
   rep_->params_->id_ = MenuID;
#endif
	
}

PopupWindow::~PopupWindow()
{
}

void PopupWindow::set_attributes()
{
}
