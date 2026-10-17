// =======================================================================
//
//                     <IV-Mac/Event.h>
//
//  Machintosh implementation of the InterViews Event classes.  
//
//  Machintosh implementation of the InterViews Event classes.  The
//  flow of control is significantly different from MS-Windows.  Events
//	are held on a que, and are read in by Session::read.  The event handler
//  then parses the event.  Machintosh native calls are performed on most
//  events.  Mouse motion, key down events, and mouse down events in the 
//	content area of a window, however, are given to the function 
//	MACwindow::MACinput.  This function calls EventRep::handleCallBack
//	which essentially allows Inteviews to handle the event.
//
//  The EventRep stores the type and button information directly as the
//  values used by the Event class. All information is stashed directly 
//  in the EventRep with no translation unless the information is asked 
//  for through the Event class.  MS-Windows Note -- This is to facilitate 
//	rapid creation since a large number of these will be created (especially 
//	mouse motion events).
//
//
// 1.1
// 1997/03/28 17:35:42
//
// =======================================================================
#ifndef iv_mac_event_h
#define iv_mac_event_h

// ---- InterViews includes ----
#include <InterViews/iv.h>
#include <InterViews/coord.h>
#include <InterViews/window.h>


class EventRep
{
public:
		EventRep();
		~EventRep();
		
#if carbon
		EventRef getEventRef();
		static Point mouse_loc(EventRef);
		void handle();
		void set(int type, int button, int x, int y);
#else
		EventRecord * getEventRecord();
#endif
		Window* windowOf();
		void setWindow(WindowPtr aMacWindow);
		int typeOf();						// event type
		int buttonOf();						// mouse button
		
		void mouseDownEventHook(void);
		void mouseUpEventHook(void);
		void mouseMotionEventHook(void);
		void keyDownEventHook(void);
		void appleEventHook(void);
		void nullEventHook(void);
		void osEventHook(void);
		void diskEventHook(void);
		void activateEventHook(void);
		void updateEventHook(void);
		void appleMenuHook(short menuItem);
		
#if carbon
		void setEventRef(EventRef theEventRef);
#else
		void setEventRecord(EventRecord * theEventRecord);
#endif
		static void handleCallback(Event&);
		// This function is called by WindowRep as a result an earlier call
		// to Event::handle().
		
		int ivlocalMouse_x(void);
		int ivlocalMouse_y(void);
		int ivglobalMouse_x(void);
		int ivglobalMouse_y(void);
		void setMouseLocationToCurrent(void);
private:
		friend class WindowRep;				// this class fills in the event info
#if carbon
		EventRef theEvent_;
#else
		EventRecord * theEvent_;
#endif
		Window* win_;
		int type_;                          // Event::type enum
		int button_;                        // Event::button enum
		Point localMouseLocation_;
		
};

// --- inline functions ---
#if carbon
inline EventRef EventRep::getEventRef(void)
	{return theEvent_;}
#else
inline EventRecord * EventRep::getEventRecord(void)
	{return theEvent_;}
#endif
inline Window* EventRep::windowOf(void)
	{return win_;}
inline int EventRep::typeOf()
	{ return type_; }
inline int EventRep::buttonOf()
	{ return button_; }
#if carbon
inline void EventRep::setEventRef(EventRef theEventRef)
	{theEvent_ = theEventRef;}
#else
inline void EventRep::setEventRecord(EventRecord * theEventRecord)
	{*theEvent_ = *theEventRecord;}
#endif
inline int EventRep::ivlocalMouse_x(void)
	{return localMouseLocation_.h;}
inline int EventRep::ivlocalMouse_y(void)
	{Canvas* c = win_->canvas(); return (c->pheight() - localMouseLocation_.v);}
inline void EventRep::setMouseLocationToCurrent(void)
	{GetMouse(&localMouseLocation_);}
#endif
