#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
#if !carbon
// =======================================================================
//
//                     <IV-Mac/event.c>
//
//  Macintosh implementation of the InterViews Event classes.  The
//  flow of control is significantly different from MS-Windows.  Events
//	are held on a que, and are read in by Session::read.  The event handler
//  then parses the event.  Macintosh native calls are performed on most
//  events.  Mouse motion, key down events, and mouse down events in the 
//	content area of a window, however, are given to the function 
//	MACwindow::MACinput.  This function calls EventRep::handleCallBack
//	which essentially allows Inteviews to handle the event.
//
//
// 1.3
// $Date:   4 Aug 1996 
//
// =======================================================================

#include <InterViews/resource.h>
#include <InterViews/event.h>
#include <InterViews/window.h>
#include <InterViews/handler.h>
#include <InterViews/canvas.h>
#include <InterViews/display.h>
#include <InterViews/session.h>
#include <IV-Mac/event.h>
#include <IV-Mac/window.h>
#include <OS/list.h>

#include <stdio.h>
#include <stdlib.h>
#if !carbon
#include <sound.h> // SysBeep got moved
#endif

// add by jijun 5/22/97
extern "C" { void debugfile(const char*, ...);}

//Temporarily being used so that we can quit from the program
//Need to still do menu support.
#define APPLE_MENU_ID 32000
#define FILE_MENU_ID 32001
#define QUIT_ITEM 9

static int last_button_;

//extern "C" { void hoc_quit(); void ivoc_dismiss_defer();}

declarePtrList(MAChandlerPtrList, Handler)
implementPtrList(MAChandlerPtrList, Handler)

// ---- grabbing event handler list ----
static MAChandlerPtrList grabberList(100);

#if carbon
static Point mouse_loc_;
Point& EventRep::mouse_loc(EventRef er) {
	GetEventParameter(er, kEventParamMouseLocation, typeQDPoint, NULL,
		sizeof(mouse_loc_), NULL, &mouse_loc_);
	return mouse_loc_;
}
#endif
//global point to determine mouse movement
Point THE_PREVIOUS_MOUSE_POINT;

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
	
	// Macintosh implementation has not been considered
}

Window* Event::window() const
{ 
	return rep_->windowOf(); 
}

// -----------------------------------------------------------------------
// The following operations deal with events coming from a UNIX style file
// (actually socket) where events can be read, pushed back, or tested to
// see if any new ones are pending.
//
// These should not be used anyway... the programmer should be running
// things with Session::run() and using the Dispatch class for any other
// forms of event handling.
//
// Pending has been implemented under the Macintosh because it was fairly
// simple.  The other functions haven't really been considered.
// -----------------------------------------------------------------------
bool Event::pending() const
{
	return Session::instance()->pending();
}

void Event::read()
{
	printf("Event::read - unsupported");
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
	printf("Event::unread - unsupported\n");
	//rep()->display_->put(*this);
}

void Event::poll()
{
	// MS Windows Note
	// used to query the mouse position, which could be synthsized, but
	// for now it's unimplimented... I consider this a rather unclean
	// interface to begin with.
	
	// Haven't really dealt with this for the Machintsoh
	printf("Event::poll - unsupported");
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
// This function parses the Macintosh Event Record and calls an appropriate
// hook to deal with the event.
// -----------------------------------------------------------------------
void Event::handle()
{
	EventRecord * record = rep_->getEventRecord();
	
	if(record){
		switch(record->what){
			case mouseDown:
printf("mouseDown\n");
				rep_->mouseDownEventHook();
				break;
			case mouseUp:
printf("mouseUp\n");
				rep_->mouseUpEventHook();
//				ivoc_dismiss_defer();
				break;
			case keyDown:
			case autoKey:
printf("keyDown\n");
				rep_->keyDownEventHook();
				break;
			case updateEvt:
printf("updateEvt\n");
				rep_->updateEventHook();
				break;
			case activateEvt:
printf("activateEvt\n");
				rep_->activateEventHook();
				break;
			case diskEvt:
printf("diskEvt\n");
				rep_->diskEventHook();
				break;
			case osEvt:
printf("osEvt\n");
				rep_->osEventHook();
				break;
			case kHighLevelEvent:
printf("kHighLevelEvent\n");
				rep_->appleEventHook();
				break;
			case nullEvent:
printf("nullEvent\n");
				rep_->nullEventHook();
				break;
			default:
printf("default\n");
				break;	
		}
	} else {
		printf("Error Allocating memory for record");
		exit(1);
	}
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
    for (ListUpdater(MAChandlerPtrList) i(grabberList); i.more(); i.next()) 
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
	for (ListItr(MAChandlerPtrList) i(grabberList); i.more(); i.next())
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
	EventRecord * temp;
	
	if(temp = (rep()->getEventRecord()))
		return temp->when;
	//Should not reach this point
	return 0;
}

Coord Event::pointer_x() const
{
	Coord x_temp;
	Display* dpy = display();
	return dpy->to_coord(rep_->ivlocalMouse_x(), Dimension_X);
}

Coord Event::pointer_y() const
{
	Display* dpy = display();
	return dpy->to_coord(rep_->ivlocalMouse_y(), Dimension_Y);;
}

Coord Event::pointer_root_x() const
{
//	Display* dpy = display();
//	return dpy->to_coord(rep_->ivglobalMouse_x(), Dimension_X);
	Coord x =  pointer_x() + window()->left();
//	printf("rootx=%g\n", x);
	return x;
}

Coord Event::pointer_root_y() const
{
//	Display* dpy = display();
//	return dpy->to_coord(rep_->ivglobalMouse_y(), Dimension_Y);
	Coord y =  pointer_y() + window()->bottom();
//	printf("rooty=%g\n", y);
	return y;
}


EventButton Event::pointer_button() const
{
	return rep_->buttonOf();
}

unsigned int Event::keymask() const
{
	printf("Event::keymask() - unsupported"); 
    return 0;
}


// -------------------------------------------------------------------------
// key tests - It is presumed that this query is being made in response 
// to some key/pointer event, in which case the modifiers field of the EvenT
//  record would hold valid information. 
// -------------------------------------------------------------------------
bool Event::control_is_down() const
	{ return (rep_->getEventRecord()->modifiers & controlKey) ? true : false; }
bool Event::shift_is_down() const
	{ return (rep_->getEventRecord()->modifiers & shiftKey) ? true : false; }

//These aren't called ... so we'll deal with it later
//
bool Event::left_is_down() const
	{ return true;}
bool Event::middle_is_down() const
	{ return false; }
bool Event::right_is_down() const
	{ return false; }

bool Event::capslock_is_down() const
	{ return (rep_->getEventRecord()->modifiers & alphaLock) ? true : false; }
bool Event::meta_is_down() const
	{ return false; }

unsigned char Event::keycode() const
{
	if (rep_->typeOf() == key){
		return ((char)((rep_->getEventRecord()->message) & charCodeMask));
	} else {
		return 0;
	}
}

// from 0x20 to 0x2F
unsigned int Event::mapkey(char* buff, unsigned int bufflen) const
{
//	printf("Event::mapkey currently not implemented\n");
//	return 0;
	buff[0] = keycode();
	buff[1] = '\0';
	if (buff[0] == '\0') {
		return 0;
	}else{
		return 1;
	}
}

unsigned long Event::keysym() const
{
	// TBD
	//MWassert(0);
	printf("Event::keycode currently not implemented");
	return 0;
}

// #######################################################################
// ########################## class EventRep
// #######################################################################

EventRep::EventRep(){
	theEvent_ = new EventRecord;
	win_ = nil;
}

EventRep::~EventRep(){
	if(theEvent_)
		delete theEvent_;
}

void EventRep::setWindow(WindowPtr aMacWindow){
	WindowRep*	temp;	
	temp = WindowRep::rc(aMacWindow);
	if(temp){
		win_ = temp->ivWindowOf();
	} else {
		win_ = nil;
	}
}

void EventRep::mouseDownEventHook(void){

	WindowPtr	frontWindow       = nil;
	WindowPtr	hitWindow         = nil;
	WindowRep	*ourStructure;
	short		windowPart;
	long		menuChoice;
	Rect		dragRect;
	short		windowType;		/* modal, modeless, etc. */
	
	frontWindow = FrontWindow();
	windowPart = FindWindow(theEvent_->where, &hitWindow);	
	//Set InterViews type
	type_ = Event::down;
	//Set InterViews button
	if(theEvent_->modifiers & (controlKey | cmdKey)){
		button_ = Event::right;
	} else if (theEvent_->modifiers & optionKey){
		button_ = Event::middle;
	} else {
		button_ = Event::left;
	}
	last_button_ = button_;
	
	switch (windowPart)
	{
		case inMenuBar:
			#if 0		
														/* user made a menu selection */
			if(ourStructure = ((CWindowPeek)frontWindow)->refCon)		/* enable/disable menu items */
				ourStructure->updateMenus();
			#endif
			
			menuChoice = MenuSelect(theEvent_->where);	/* drop menu and track choice */
						
			if (menuChoice)	/* if a choice made */
			{
				if((HiWord(menuChoice) == FILE_MENU_ID) && (LoWord(menuChoice) == QUIT_ITEM)){
					//Session::instance()->quit();
//					hoc_quit();
				} else if ((HiWord(menuChoice) == APPLE_MENU_ID)){
					appleMenuHook(LoWord(menuChoice));
				}
				/* dispatch control */
			}
			
			HiliteMenu(0);								/* unhilite the menubar when done */
			break;

		case inDrag:
			/* there has to be a hit window */
			
			/* if clicking a background window & not a command drag */
			if ( (hitWindow != frontWindow) && !(theEvent_->modifiers & cmdKey))
			{
				windowType = MACwindow::isDialog(frontWindow);
				if ( windowType == MACwindow::MODAL || windowType == MACwindow::MOVABLE_MODAL){
					SysBeep(30);	/* modal window in front, sorry Charlie */
				}
				else 
				{
					SelectWindow(hitWindow);	/* activate this window */
					// add by jijun 3/4/97 
					//allow fast drag selected window
					if (ourStructure = WindowRep::rc(hitWindow))
					{
						ourStructure->doDrag(theEvent_);
					}
				}
			}
			
			else if (ourStructure = WindowRep::rc(hitWindow))	/* if it's our window */
			{
				/* call the window's drag behavior */
				ourStructure->doDrag(theEvent_);
			}
			else	/* not our window, e.g. could be moveable dialog */
			{
#if carbon
				GetRegionBounds(GetGrayRgn(), &dragRect);
#else
				dragRect = (**GetGrayRgn()).rgnBBox;	/* drag to any screen */
#endif
				DragWindow(hitWindow, theEvent_->where, &dragRect);
			}
			break;


		case inGrow:
			if (ourStructure = WindowRep::rc(hitWindow))	/* if it's our window */
			{
				/* call the window's grow behavior */
				ourStructure->doGrow(theEvent_);
			}
			break;

		case inGoAway:
			/* track mouse in go-away box */
			if (TrackGoAway(hitWindow, theEvent_->where))
			{
				/* if the option key is down */
				if (theEvent_->modifiers & optionKey)	
				{
					/* close all windows, asking user to save if needed */
					WindowPtr		macWindow  = FrontWindow();	/* first window */
	
					while (macWindow){
 						/* make sure it's our window, if not get next */
						if((MACwindow::isOurWindow(macWindow)) &&
							WindowRep::rc(macWindow)){
							
							
							/* close this window, if cancelled, return error */
							//need to add file saving options
							DisposeWindow(macWindow);
							//if (error){
								//return error;
							//}
						}
						#if carbon
						macWindow = GetNextWindow(macWindow);
						#else
						macWindow = (WindowPtr)((CWindowPeek)macWindow)->nextWindow;
						#endif
					}
				}else { //close this one window
					ourStructure = WindowRep::rc(hitWindow);
					if (ourStructure && ourStructure->close_callback_) {
						// I hope nobody needs a real event.
						Event e;
						ourStructure->close_callback_->event(e);
					}
				}
//				ivoc_dismiss_defer();
			}
			break;

		case inZoomIn:
		case inZoomOut:
			if (ourStructure = WindowRep::rc(hitWindow))
			{
				/* track  mouse in the zoom box */
				if (TrackBox(hitWindow, theEvent_->where, windowPart))
				{
					/* determine if we're zooming in or out */
					short zoomDirection = windowPart;
					
					/* tell the window to do it */
					ourStructure->doZoom((void *)&zoomDirection);
				}
			}
			break;

		case inDesk:	/* do nothing */
			break;

		case inSysWindow: /* Pass along the click to system windows (desk accessories) */
						  /* Even in the System 7 environment you may get one of these */
						  /* if the user opens a DA inside your app's partition */
#if carbon
#else
			SystemClick(theEvent_, hitWindow);
#endif
			break;

		case inContent:
			/* if it is non-front window, activate it */
			if (hitWindow != frontWindow){
				windowType = MACwindow::isDialog(frontWindow);
				if ( windowType == MACwindow::MODAL || windowType == MACwindow::MOVABLE_MODAL){
					SysBeep(30);	/* modal window in front, sorry Charlie */
				} else if (ourStructure = WindowRep::rc(hitWindow))	/* hit a background EasyWindow */
				{
					ourStructure->doBackgroundClick(theEvent_);
					// not mac standard but helpful to handle the event as well as select window
					// give up drag/drop?
					ourStructure->MACinput(theEvent_, type_, button_);
				}
				else /* hit some other window */
				{
					SelectWindow(hitWindow);	/* activate this window */
				}
			}
			
			/* otherwise, if it's my front window do click in content */
			else if ((ourStructure = WindowRep::rc(frontWindow)) &&
#if carbon
					 GetWindowKind(frontWindow) == 8)
#else
					 (((CWindowPeek)frontWindow)->windowKind == 8))
#endif
			{	
				ourStructure->MACinput(theEvent_, type_, button_);
			}
			break;
		default:
			break;
	}	
}

void EventRep::mouseUpEventHook(void){
	WindowPtr		frontWindow;
	WindowRep*		ourStructure;
	
	//Set InterViews type
	type_ = Event::up;
	button_ = last_button_;
	
	frontWindow = FrontWindow();
	if(MACwindow::isOurWindow(frontWindow)){
		ourStructure = WindowRep::rc(frontWindow);
	} else {
		ourStructure = nil;
	}
	
	if(ourStructure)
		ourStructure->MACinput(theEvent_, type_, button_);
}

void EventRep::keyDownEventHook(void){
	WindowPtr		frontWindow;
	WindowRep*		ourStructure;
	long			menuChoice;
	char			theKey;

	frontWindow = FrontWindow();

	//Set InterViews type
	type_ = Event::key;
	button_ = Event::none;
	
	if(MACwindow::isOurWindow(frontWindow)){
		ourStructure = WindowRep::rc(frontWindow);
	} else {
		ourStructure = nil;
	}	
	
	
/*
	If the command key is down, check for menu selection. Do this even
	when there is no window open.
*/
	if (theEvent_->modifiers & cmdKey)	/* see if a menu shortcut was hit. */
	{
		theKey = (char)(theEvent_->message & charCodeMask); /* get the key pressed */
		//UpdateMenusHook(frontWindow);	/* update the menus before selection */
		menuChoice = MenuKey(theKey);	/* find out which menu item it matches */
		if (HiWord(menuChoice))	/* if it matches a menu item */
		{
			/* don't care if it's handled or not, all command-keys filtered out */
			//MenuDispatchHook(frontWindow, menuChoice);	 /* dispatch control	 */
			HiliteMenu(0);
		}
	}

/*
	If key not a command key, and there is a window, pass key event to window.
	Note: front window is our window, we tested early in this routine
*/
	else if (ourStructure)
	{
		ourStructure->MACinput(theEvent_, type_, button_);
	}
}

// Apple Event support is currently unimplamented
void EventRep::appleEventHook(void){
	OSErr	error;
	
//in order to implement this process ... at startup we need to set up a dispatch table
//of procedures to handle these events.  The AEProcessAppleEvent will automatically 
//look these procedures up and call them.

// change by jijun 5/22/97
//debugfile("\nEvent.what=%d",theEvent_->what);
//debugfile("\nEvent.message=%ld",theEvent_->message);
//debugfile("\nEvent.when=%ld",theEvent_->when);
//debugfile("\nEvent.where=%p",theEvent_->where);
//debugfile("\nEvent.modifiers=%d",theEvent_->modifiers);
	error = AEProcessAppleEvent(theEvent_);	/* process the event */
//debugfile("\nAppleProcess, error=%d", error);
#if 0
	error = AEProcessAppleEvent(theEvent_);	/* process the event */
	
	if (error)
	{
		if (AEInteractWithUser(kNoTimeOut, nil, nil) == noErr)
		{
			//debugfile("We get AEInteractWithUser");
			printf("Error in Apple Event Hook");
			exit(1);
		}
	}
#endif
}

void EventRep::nullEventHook(void){

 	//can do idle time tasks
 }

void EventRep::osEventHook(void){
	char eventType;
	eventType = theEvent_->message >> 24;
	if(eventType & mouseMovedMessage){
		WindowPtr aMacWindow = FrontWindow();
		//Set InterViews type
		type_ = Event::motion;
		 
		//THIS COULD FAIL WITH A WINDOW THAT IS NOT OURS
		if(WindowRep::rc(aMacWindow))
			WindowRep::rc(aMacWindow)->MACinput(theEvent_, type_, Event::none);

	} else if(eventType & suspendResumeMessage){
		if(theEvent_->message & resumeFlag){  //resume event
			//possibly adjust sleeptime, set a switchdin to true, and activate front
		} else { //suspend
					
		}
	}
}

void EventRep::diskEventHook(void){
	Point standard;  //should be taken care of by system
	standard.h = standard.v = 90;
	if(HiWord(theEvent_->message) != noErr){
#if carbon
#else
		DILoad();
		DIBadMount(standard, theEvent_->message);
		DIUnload();
#endif
	}
}

void EventRep::activateEventHook(void){
	WindowPtr theWin;

	theWin = ((WindowPtr)theEvent_->message);
	if((MACwindow::isOurWindow(theWin))){
		setWindow(theWin);
		if(windowOf()){
			windowOf()->rep()->activate((theEvent_->modifiers & activeFlag) != 0);
		}
	}
}

void EventRep::updateEventHook(void){
	WindowPtr theWin;
	GrafPtr			oldPort;
	Rect			contentRct;
	
	theWin = ((WindowPtr)(theEvent_->message));
	if((MACwindow::isOurWindow(theWin))){
		setWindow(theWin);
		if(windowOf()){
			windowOf()->rep()->update();
		} else { //the addition of this else helped back menus draw correctly
			GetPort(&oldPort);
#if carbon
			SetPort(GetWindowPort(theWin));
			GetPortBounds(GetWindowPort(theWin), &contentRct);
#else
			SetPort(theWin);
			contentRct = theWin->portRect;
#endif
			BeginUpdate(theWin);
				//EraseRect(&contentRct);
#if carbon
				RgnHandle visRgn;
				visRgn = NewRgn();
				GetPortVisibleRegion(GetWindowPort(theWin), visRgn);
				UpdateControls(theWin, visRgn);
				DisposeRgn(visRgn);
#else			
				UpdateControls(theWin, theWin->visRgn);
#endif
				/* draw contents */
				//easyWindow->DoDraw(macWindow, data);
			EndUpdate(theWin);
			SetPort(oldPort);
		}
	}
}

void EventRep::appleMenuHook(short menuItem){
	Str255		itemName;
	GrafPtr		oldPort;
	
	switch(menuItem)
	{
		case 1:	/* do the about box */
			break;
		
		default:	/* other Apple menu items */
			GetPort(&oldPort);	/* save port - OK, I'm paranoid */
			
			GetMenuItemText(GetMenuHandle(APPLE_MENU_ID), menuItem, itemName); /* get chosen item's name */
/*
	Under System 7, the system immediately launches the "DA" as an application.
	It then comes back here immediately. Your app is still in the foreground.
	The system will then send us events to tell us to go into the background,
	and bring the "DA" into the foreground.
*/
#if carbon
#else
			OpenDeskAcc(itemName);	/* launch it */
#endif
			
			SetPort(oldPort);	/* restore port, just in case */
			break;
	}
}	

int EventRep::ivglobalMouse_x() 
{
	Point globalLocation;
	GrafPtr  oldPort;
	GetPort(&oldPort);
	win_->rep()->setport();
	LocalToGlobal(&globalLocation);
	SetPort(oldPort);
	return (globalLocation.h);
}

int EventRep::ivglobalMouse_y() 
{
	Point globalLocation;
	int height;
	GrafPtr  oldPort;
	GetPort(&oldPort);
	win_->rep()->setport();
	LocalToGlobal(&globalLocation);
	SetPort(oldPort);
	Display* d = win_->display();
	if(d){
		height = d->pheight();
	} else {
		printf("Display not found line 768 event.c\n");
	}	 
	return (height - globalLocation.v);
}



// -----------------------------------------------------------------------
// When a message comes in for a window that is use input related, this 
// function gets called by MACinput.  This function then invokes the
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
#endif
