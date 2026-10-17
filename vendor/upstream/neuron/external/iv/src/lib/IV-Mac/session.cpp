#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =======================================================================
//
//                      <IV-Mac/session.c>
//
// Session management for an application.  This class is a clearing house
// for the control part of an application.
//
//
// 1.5
// $Date:   4 AUG 1996 
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
// =======================================================================

#include <InterViews/window.h>
#include <InterViews/style.h>
#include <InterViews/session.h>
#include <InterViews/cursor.h>
#include <InterViews/display.h>
#include <InterViews/event.h>
#include <IV-Mac/session.h>
#include <IV-Mac/event.h>
#include <IV-Mac/window.h>
#include <OS/file.h>
#include <OS/string.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#if !carbon
#include <sound.h>
#include <Icons.h>	// add by jijun 6/19/97
#include <SIOUXGlobals.h>
#endif

#include <stdlib.h>


extern "C" { void debugfile(const char*, ...);}

#include <stdarg.h>
void debugfile(const char* format, ...) {
        va_list args;
        static FILE* df;
        if (!df) {
                df = fopen("debugfile", "w");
        }
        va_start(args, format);
        vfprintf(df, format, args);
        fflush(df);
}

//Constant setting the time to yield when no events have been received
#define SLEEP_TIME 10000


//-------------------------------------------------------------------
#ifdef MAC

static PropertyData properties[] = {
{"*gui", "sgimotif"},
{"*PopupWindow*overlay", "true"},
{"*PopupWindow*saveUnder", "on"},
{"*TransientWindow*saveUnder", "on"},
{"*background", "#ffffff"},
{"*brush_width", "0"},
//{"*double_buffered", "on"},
{"*double_buffered", "off"},
{"*flat", "#aaaaaa"},
{"*font", "*Arial*bold*--12*"},
{"*MenuBar*font", "*Arial*bold*--12*"},
{"*MenuItem*font", "*Arial*bold*--12*"},
{"*foreground", "#000000"},
{"*synchronous", "off"},
{"*malloc_debug", "on"},

{"*Scene_background", "#ffffff"},
{"*Scene_foreground", "#000000"},
{"*FieldEditor*background", "#ffffff"},
{"*background", "#cfffff"},
{"*default_brush", "0"},
{"*view_margin", ".25"},
{"*dismiss_button", "on"},
{"*use_transient_windows", "yes"},
{"*nrn_library", " $(NEURONHOME)/lib"},
{"*view_pick_epsilon", "2"},
{"*pwm_canvas_height", "120"},
{"*pwm_paper_height", "11"},
{"*pwm_paper_width", "8.5"},
{"*pwm_paper_resolution", ".5"},
{"*pwm_pixel_resolution", "0"},
{"*window_manager_offset_x", "5."},
{"*window_manager_offset_y", "26."},
{"*pwm_print_file_filter", "*.ps"},
{"*pwm_idraw_file_filter", "*.id"},
{"*pwm_ascii_file_filter", "*"},
{"*pwm_save_file_filter", "*.ses"},
{"*pwm_idraw_prologue", "$(NEURONHOME)/lib/prologue.id"},
{"*pwm_postscript_filter", "sed 's;/Adobe-;/;'"},
{"*SlowSlider*autorepeatStart", "0."},
{"*scene_print_border", "1"},
{"*radioScale", "1.2"},
{"*xvalue_format", "%.5g"},
{"*graph_axis_default", "0"},
{"*shape_scale_file", "$(NEURONHOME)/lib/shape.cm2"},
{"*CBWidget_ncolor", "10"},
{"*CBWidget_nbrush", "10"},
	 { nil }
};

#endif

Session* SessionRep::instance_;

// I DO NOT KNOW IF THIS IS NECESSARY ... BUT I DIDN'T WANT TO TAKE IT OUT
#if 0
const char* SECTION_NAME = "InterViews";
	// This is the name of the section in the WIN.INI file where the
	// configuration information should be fetched from. The .ini files
	// are used in Win32 so that things will work with Win32s.

const char* LOCATION_KEY = "location";
	// This is the key used to fetch the value of the location of the
	// top of the configuration directory.  Things like application
	// defaults live at this location.

const char* APPDEF_DEFAULT = SECTION_NAME;
	// Name of the default application defaults file.  This should contain
	// style definitions that are defaults for the entire library.
const char* APPDEF_DEFAULT_ALT = "intervie"; // win3.1 install and using NT

const char* APPDEF_DIRECTORY = "app-defaults";
	// This is the name of the directory where the default style information
	// lives.  In X-Windows this is called app-defaults.  Each application
	// is expected to have a file in this directory to establish reasonable
	// default behavior.
const char* APPDEF_DIR_ALT = "app-defa"; // win3.1 install and using NT

static char* INSTALL_LOCATION = NULL;
	// pathname of the installation location.... determined by reading the
	// win.ini file and querying the location key.
#endif

// -----------------------------------------------------------------------
// Predefined command-line options.
// -----------------------------------------------------------------------
static OptionDesc defoptions[] =
{
	 { "-background", "*background", OptionValueNext },
	 { "-bg", "*background", OptionValueNext },
	 { "-dbuf", "*double_buffered", OptionValueImplicit, "on" },
	 { "-display", "*display", OptionValueNext },
	 { "-dpi", "*dpi", OptionValueNext },
	 { "-fg", "*foreground", OptionValueNext },
	 { "-flat", "*flat", OptionValueNext },
	 { "-fn", "*font", OptionValueNext },
	 { "-font", "*font", OptionValueNext },
	 { "-foreground", "*foreground", OptionValueNext },
	 { "-geometry", "*geometry", OptionValueNext },
	 { "-iconic", "*iconic", OptionValueImplicit, "on" },
	 { "-monochrome", "*gui", OptionValueImplicit, "monochrome" },
	 { "-motif", "*gui", OptionValueImplicit, "Motif" },
	 { "-mswin", "*gui", OptionValueImplicit, "mswin" },
	 { "-name", "*name", OptionValueNext },
	 { "-nodbuf", "*double_buffered", OptionValueImplicit, "off" },
	 { "-noshape", "*shaped_windows", OptionValueImplicit, "off" },
	 { "-openlook", "*gui", OptionValueImplicit, "OpenLook" },
	 { "-reverse", "*reverseVideo", OptionValueImplicit, "on" },
	 { "-rv", "*reverseVideo", OptionValueImplicit, "on" },
	 { "-shape", "*shaped_windows", OptionValueImplicit, "on" },
	 { "-smotif", "*gui", OptionValueImplicit, "SGIMotif" },
	 { "-synchronous", "*synchronous", OptionValueImplicit, "on" },
	 { "+synchronous", "*synchronous", OptionValueImplicit, "off" },
	 { "-title", "*title", OptionValueNext },
	 { "-visual", "*visual", OptionValueNext },
	 { "-visual_id", "*visual_id", OptionValueNext },
	 { "-xrm", nil, OptionPropertyNext },
#ifdef sgi
	 { "-malloc", "*malloc_debug", OptionValueImplicit, "on" },
#endif
	 { nil }
};



// #######################################################################
// #####################  SessionRep class
// #######################################################################

#if USE_SIOUX_WINDOW
extern "C"{
		short InstallConsole(short fd);
		extern void hoc_quit();
		extern Boolean SIOUXQuitting;
}
#endif

/************************************************************************/
/* Purpose..: Check if the window belongs to us							*/
/* Input....: Pointer to the window										*/
/* Returns..: TRUE ours                    								*/
/* Jijun 4/26/97 copy form: SIOUXWindows.c								*/
/************************************************************************/
static void FlashControl(ControlHandle theControl)
{
	unsigned long aLong;

	HiliteControl(theControl, 1);
	Delay(3L, &aLong);
	HiliteControl(theControl, 0);
}

/************************************************************************/
/* Purpose..: Frame a control											*/
/* Input....: Control Handle											*/
/* Returns..: ---                    									*/
/* Jijun 4/26/97 copy form: SIOUXWindows.c								*/
/************************************************************************/
static void FrameControl(ControlHandle theControl)
{
#if carbon
	//use by LowMemoryBox
#else
	Rect myRect;
	
	HLock((Handle)theControl);
	myRect.top = (*theControl)->contrlRect.top - 4;
	myRect.left = (*theControl)->contrlRect.left - 4;
	myRect.bottom = (*theControl)->contrlRect.bottom + 4;
	myRect.right = (*theControl)->contrlRect.right + 4;
	HUnlock((Handle)theControl);
	
	PenNormal();
	PenSize(3,3);
	FrameRoundRect(&myRect,16,16);
	PenNormal();
#endif
}

/****************************************************************/
/* Purpose..: Display a low-memory box	  						*/
/* Input....: ---												*/
/* Returns..: ---												*/
/* Jijun 4/25/97 reference: SIOUXWindows.c						*/
/****************************************************************/
static void LowMemoryBox(void)
{
#if carbon
	// why not use some kind of alert?
#else
	WindowPtr wp, theWindow;
	GrafPtr savePort;
	EventRecord theEvent;
	Handle theIcon;
	ControlHandle okButton, dummyCntl;
	Rect aRect = {0, 0, 75, 270};
	Boolean done = FALSE;
	short height, width;

	SetCursor(&qd.arrow);
	GetPort(&savePort);
	
	//	Center it horizontally and 20% down vertically
	width = (qd.screenBits.bounds.right - qd.screenBits.bounds.left - aRect.right) / 2;
	height = (qd.screenBits.bounds.bottom - qd.screenBits.bounds.top - LMGetMBarHeight() - aRect.bottom) / 5 + LMGetMBarHeight();
	
	OffsetRect(&aRect, width, height);
	
	//	Create a simple window with an OK button
	if ((wp = NewWindow(0L, &aRect, "\p", TRUE, dBoxProc,
						(WindowPtr)-1L, FALSE, 0L)) == 0L)
	{
		SysBeep(10);
		return;
	}

	SetPort(wp);
	SetRect(&aRect, 80, 15, 260, 46);
	TETextBox("Low Memory", 10, &aRect, teForceLeft);

	SetRect(&aRect, 8, 8, 40, 40);
	theIcon = GetIcon(0);
	PlotIcon(&aRect, theIcon);

	SetRect(&aRect, 105, 50, 165, 68);
	okButton = NewControl(wp, &aRect, "\pContinue", TRUE, 0, 0, 0, pushButProc|kControlUsesOwningWindowsFontVariant, 0);

	//	Outline the default button ...
	FrameControl(okButton);
	
	while (!done) {
		if(GetNextEvent(mDownMask | keyDownMask, &theEvent)) {
			if (theEvent.what == mouseDown) {
				if ((FindWindow(theEvent.where, &theWindow) == inContent) && (theWindow == wp)) {
					GlobalToLocal(&theEvent.where);
					if (FindControl(theEvent.where, wp, &dummyCntl) == kControlButtonPart)
						if (TrackControl(dummyCntl, theEvent.where, 0L) != 0) {
							done = TRUE;
						}
				}
			} else if (theEvent.what == keyDown) {
				char key;
				
				key = theEvent.message & charCodeMask;
				if (key == 0x0d || key == 0x03) {
					//FlashControl(okButton);
					done = TRUE;
				}
			}
		}
	}
	
	DisposeControl(okButton);
	DisposeWindow(wp);
	ReleaseResource(theIcon);
	SetPort(savePort);
#endif
}

/****************************************************************/
/* Purpose..: Low-memory Dialog Box	  							*/
/* Input....: ---												*/
/* Returns..: ---												*/
/* Jijun 4/28/97 												*/
/****************************************************************/
static void DMemory(void) {
#if carbon
	// why not use an alert?
#else
	DialogPtr lm;
	WindowPtr wp;
	GrafPtr savePort;
	EventRecord theEvent;
	Handle theIcon;
	Rect aRect = {0, 0, 100, 240};
	Boolean done = FALSE;
	short length;
	short height, width;
	short contButton;

	SetCursor(&qd.arrow);
	GetPort(&savePort);
	
	/* Center it horizontally and 20% down vertically */
	width = (qd.screenBits.bounds.right - qd.screenBits.bounds.left - aRect.right) / 2;
	height = (qd.screenBits.bounds.bottom - qd.screenBits.bounds.top - LMGetMBarHeight() - aRect.bottom) / 5 + LMGetMBarHeight();
		
	OffsetRect(&aRect, width, height);
	contButton=StopAlert(stopIcon, 0);
	//debugfile("contButton=%d",contButton);
	
	//	Create a simple dialog
	if ((lm = NewDialog(0L,&aRect,"\p",TRUE,dBoxProc,(WindowPtr)-1L, 0, 0,0)) == 0L)
	{
		SysBeep(10);
		return;
	}

	SetPort(lm);	
	SetRect(&aRect, 0, 5, 240, 55);
	TETextBox("Not Enough Memory", 17, &aRect, teCenter);

	SetRect(&aRect, 8, 8, 40, 40);
	theIcon = GetIcon(0);
	PlotIcon(&aRect, theIcon);
			
	while (!GetNextEvent(mDownMask | keyDownMask, &theEvent))
		/* just loop */ ;

	DisposeDialog(lm);
	SetPort(savePort);
#endif
}

// add by jijun 5/30/97
//extern "C" { int hoc_xopen1(const char* filename, const char* rcs);}
extern "C" {
#if !carbon
void mac_open_doc(const char* s);
void mac_open_app();
bool is_mac_dll(FSSpec*);
bool mac_open_dll(const char*, FSSpec*);
#endif
}

extern "C" {
/****************************************************************/
/* Purpose..: Create a full path name for a FSSpec file			*/
/* Input....: FSSpec											*/
/* Returns..: String[255] Ptr									*/
/* Jijun 5/30/97 												*/
/****************************************************************/
static char* fullname(const FSSpec &file){
	short	refNum;
    CInfoPBRec myPB;
    char prePath[256];
    static char fullPath[256];
    OSErr	err;	
    Str255 dirName;
    char * s;
       
    fullPath[0]='\0';
    myPB.dirInfo.ioVRefNum = file.vRefNum;
    myPB.dirInfo.ioDrParID = file.parID;
    myPB.dirInfo.ioFDirIndex= -1;
    myPB.dirInfo.ioNamePtr = dirName;
    do {
    		myPB.dirInfo.ioDrDirID = myPB.dirInfo.ioDrParID;
    		err = PBGetCatInfoSync(&myPB);
			//debugfile("\nerror after PBGetCatInfoSync: %ld", err);
    		if (err) {
    			return nil;
    		}
    		else {
    			dirName[dirName[0]+1]='\0';
    			strcpy(prePath, (char*)&dirName[1]);
    			strcat(prePath, ":");
    			strcat(prePath, fullPath);
    			strcpy(fullPath, prePath);
				//debugfile("\nfullPath= %s", fullPath);
				//debugfile("\nmyPB.dirInfo.ioDrDirID= %ld", myPB.dirInfo.ioDrDirID);
    		}
    } while (myPB.dirInfo.ioDrDirID > fsRtDirID);
   
	s=fullPath;
   	return s;
}
}	// for external C

/****************************************************************/
/* Purpose..: Handling Open Application AppleEvent				*/
/* Input....: theAppleEvent, reply, handleRefcon				*/
/* Returns..: OSErr												*/
/* Jijun 5/29/97 reference: Interapplication (4-15)				*/
/****************************************************************/
static pascal OSErr MyHandleOApp (const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon) {
	AEDescList	docList;
	
	//debugfile("\nWe are in MyHandleOApp");
	//debugfile("\ntheAppleEvent Descriptor type: %ld", theAppleEvent.descriptorType);
	//debugfile("\ntheAppleEvent Descriptor handle*: %ld", *(theAppleEvent.dataHandle));
	//debugfile("\ntheAppleEvent Descriptor handle**: %ld", **(theAppleEvent.dataHandle));
	//debugfile("\nreply Descriptor type: %ld", reply.descriptorType);
	//debugfile("\nhandlerRefcon: %ld", handlerRefcon);

#if !carbon
	mac_open_app();	
#endif
	//OSErr myErr=AEGetParamDesc(&theAppleEvent, keyDirectObject, typeAEList, &docList);
	//debugfile("\nerror after AEGetParamDesc: %ld", myErr);
	return (0);
}

/****************************************************************/
/* Purpose..: Handling Open Documents AppleEvent				*/
/* Input....: theAppleEvent, reply, handleRefcon				*/
/* Returns..: OSErr												*/
/* Jijun 5/28/97 reference: Interapplication (4-15)				*/
/****************************************************************/
static pascal OSErr MyHandleODoc (const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon) {
//void MyHandleODoc (AppleEvent &theAppleEvent, AppleEvent &reply, long handlerRefcon) {
	FSSpec		myFSS;
	AEDescList	docList;
	long		itemInList;
	AEKeyword	keywd;
	DescType	returnedType;
	Size		actualSize;

	//debugfile("\n\nWe are in MyHandleODoc");
	//debugfile("\ntheAppleEvent Descriptor type: %ld", theAppleEvent.descriptorType);
	//debugfile("\ntheAppleEvent Descriptor handle*: %ld", *(theAppleEvent.dataHandle));
	//debugfile("\ntheAppleEvent Descriptor handle**: %ld", **(theAppleEvent.dataHandle));
	//debugfile("\nreply Descriptor type: %ld", reply.descriptorType);
	//debugfile("\nhandlerRefcon: %ld", handlerRefcon);
	
	// get the direct parameter--a descriptor list-- and put it into docList
	OSErr myErr=AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	//debugfile("\nerror after AEGetParamDesc: %ld", myErr);
	if (myErr) {
		return myErr;
	}
	else {
		// check for missing required parameters
		//myErr = MyGotRequiredParams(theAppleEvent);
		if (myErr) {
			return myErr;
		}
		else {
			// count the number of descriptor records int the list
			myErr = AECountItems(&docList, &itemInList);
			//debugfile("\nerror after AECountItems: %ld", myErr);
			//debugfile("\nitemInList: %ld", itemInList);
			if (myErr) {
				return myErr;
			}
			else {
				// get each descriptor record from list, coerce the returned data to an FSSpec record,
				// and open the associated file
				// first all the dll's
#if !carbon
				for (long index=1; index<=itemInList; index++) {
					myErr = AEGetNthPtr(&docList, index, typeFSS, &keywd, &returnedType,
										&myFSS, sizeof(myFSS), &actualSize);
					//debugfile("\nerror after AEGetNthPtr: %ld", myErr);
					if (myErr) {
						return myErr;
					}
					else {
						char* s=fullname(myFSS);
						myFSS.name[myFSS.name[0]+1]='\0';
						strcat(s,(char*)&myFSS.name[1]);						
						//debugfile("\nMyOpenFile is %s", s);
						if (is_mac_dll(&myFSS)) {
							mac_open_dll(s, &myFSS);
						}
					}	// noerr for AEGetNthPtr
				}	// for loop
#endif
#if !carbon
				// then all the hoc files
				for (long index=1; index<=itemInList; index++) {
					myErr = AEGetNthPtr(&docList, index, typeFSS, &keywd, &returnedType,
										&myFSS, sizeof(myFSS), &actualSize);
					//debugfile("\nerror after AEGetNthPtr: %ld", myErr);
					if (myErr) {
						return myErr;
					}
					else {
						char* s=fullname(myFSS);
						myFSS.name[myFSS.name[0]+1]='\0';
						strcat(s,(char*)&myFSS.name[1]);						
						//debugfile("\nMyOpenFile is %s", s);
						if (!is_mac_dll(&myFSS)) {
							mac_open_doc(s);
						}
						//hoc_xopen1(s, 0);
						//myErr=FSpDelete(&myFSS);
					}	// noerr for AEGetNthPtr
				}	// for loop
#endif
			}	// noerr for AECountItems
		}	// noerr for MyGotRequiredParams
	OSErr ingoreErr=AEDisposeDesc(&docList);
	//debugfile("\nerror after AEDisposeDesc: %ld", ingoreErr);
	}	// noerr for AEGetParamDesc
	
	return myErr;
}

#define EmergencyMemorySize 100000
extern "C" {
static char** emergency_memory_;
static pascal long emergency_memory(Size) {
	int a5 = SetCurrentA5();
	long rval = 0;
    //debugfile("before emergency_memory \n");
	LowMemoryBox();
	DMemory();
	if (emergency_memory_) {
		EmptyHandle(emergency_memory_);
		emergency_memory_ = 0;
		rval = EmergencyMemorySize;
		//printf("Warning: released emergency memory. Next call here will abort.\n");
    	//debugfile("after emergency_memory\n");
	}
	SetA5(a5);
	if (rval == 0) {
		abort();
	}
	return rval;
}
}

SessionRep::SessionRep()
{
	GrowZoneUPP	myGrowZone;				/* for growzone 4/28/97 jijun */
	AEEventHandlerUPP	myHandleODoc;	// add by jijun 5/28/97 for handle open doc
	AEEventHandlerUPP	myHandleOApp;	// add by jijun 5/29/97 for handle open app
	AEEventHandlerUPP	handler;
	long				handleRefcon;
#if carbon
	// I don't know if memory still needs to be managed
#else
	MaxApplZone();						/* take all the memory we can */
	emergency_memory_ = NewHandle(EmergencyMemorySize);
	myGrowZone = NewGrowZoneProc(emergency_memory);
	SetGrowZone(myGrowZone);
#endif
	SessionRep::makeChecks();
	SessionRep::initToolboxFunctions();
	// add by jijun 5/28/97 for handle open doc
	//debugfile("\nWe will begin install AppleEvent handler");
	AEGetEventHandler(kCoreEventClass, kAEOpenDocuments, &handler, &handleRefcon, FALSE);
	//debugfile("\nDoc handleRefcon:%ld", handleRefcon);
	AEGetEventHandler(kCoreEventClass, kAEOpenApplication, &handler, &handleRefcon, FALSE);
	//debugfile("\nApp handleRefcon:%ld", handleRefcon);
#if carbon
	myHandleOApp=NewAEEventHandlerUPP(&MyHandleOApp);
	myHandleODoc=NewAEEventHandlerUPP(&MyHandleODoc);
#else
	myHandleOApp=NewAEEventHandlerProc(&MyHandleOApp);
	myHandleODoc=NewAEEventHandlerProc(&MyHandleODoc);
#endif
	OSErr error=AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, myHandleOApp, 0, FALSE);
	//debugfile("\nerror=%ld", error);
	if (!error) {
		//debugfile("\nNo error in install open application handler");
		error=AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, myHandleODoc, 0, FALSE);
		//debugfile("\nerror=%ld", error);
	}
#if USE_SIOUX_WINDOW
	InstallConsole(0);
#endif
}

SessionRep::~SessionRep()
{
	delete name_;
    Resource::unref(style_);
	delete argv_;
}


Style* iv_display_style_; // see display.cpp

// makeChecks -
//	Make sure that the system is able to handle the inteviews code
// ie. has color QuickDraw, and is System 7.0 or better.
// Checkout EasyApp file init.c line 109
void SessionRep::makeChecks(void){
	OSErr	error;
	long 	systemVersion;
	long 	response;
	
	/* 	get the system version, must be 7 or better	 */
	error = Gestalt(gestaltSystemVersion, &systemVersion);
	if (error){
		printf("Error reading system\n");
		exit(1);
	}
	if (systemVersion < 0x700){ 
		printf("InterViews requires System 7.0 or later\n");
		exit(1);
	}
	
	/* get quickdraw version */
	error = Gestalt(gestaltQuickdrawVersion, &response);
	if (error){
		printf("Error reading system\n");
		exit(1);
	}
	// must be color quick draw
	if(response < gestalt8BitQD){
		printf("InterViews requires Color QuickDraw\n");
		exit(1);
	}
	
	//If drag and drop supported an additional check will need to be made for it.
}


// initToolboxFunctions -
//	Initialize the Macintosh toolboxes for use by the program
void SessionRep::initToolboxFunctions (void)
{
	/* Initialize all the needed managers. */
#if carbon
//	MoreMasterPointers(32);
	InitCursor();
#else
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	FlushEvents(everyEvent,0);
#endif
}


void SessionRep::set_style(Display* d)
{
#if 0 //!OCSMALL
	char buf[512];
	snprintf(buf, 512, "%s\\%s\\%s", Session::installLocation(), APPDEF_DIRECTORY, APPDEF_DEFAULT);
	FILE* f;
	if ((f = fopen(buf, "r")) == (FILE*)0) {
		char buf2[512];
		snprintf(buf2, 512, "%s\\%s\\%s", Session::installLocation(), APPDEF_DIR_ALT, APPDEF_DEFAULT_ALT);
		if ((f = fopen(buf2, "r")) == (FILE*)0) {
			char buf3[1024];
			snprintf(buf3, 1024, "Can't open InterViews resources file in either\n%s or\n%s",
				buf, buf2);
			//MessageBox(NULL, buf3, "Invalid Installation", MB_OK);
			//abort();
		}
		APPDEF_DIRECTORY = APPDEF_DIR_ALT;
		APPDEF_DEFAULT = APPDEF_DEFAULT_ALT;
	}
	fclose(f);
	 Style* s = new Style(*style_);
	 load_app_defaults(s, APPDEF_DEFAULT, -5);
	 load_props(s, props_, -10);
	 load_app_defaults(s, classname_, -5);
#else
	 Style* s = new Style(*style_);

#if defined(MAC)
	load_props(s, properties, -5);
#else
#endif
	load_props(s, props_, -10);


#endif
	iv_display_style_ = s;
	s->ref();
	//d->style(s);
}

void SessionRep::load_app_defaults(Style* s, const char* leafName, int priority)
{
#if 0
	const char* topDir = Session::installLocation();
	if (topDir)
	{
		char subPath[80];
		snprintf(subPath, 80, "/%s/%s", APPDEF_DIRECTORY, leafName);
		load_path(s, topDir, subPath, priority);
	 }
#endif
}

// -----------------------------------------------------------------------
// Property parsing errors
// -----------------------------------------------------------------------
void SessionRep::missing_colon(const String& s)
{
	char buff[135];
	const char* property = s.string();
	 snprintf(buff, 135, "Missing colon in property: %s", property);
	//WindowRep::errorMessage(buff);
	// NOT REACHED
}

void SessionRep::bad_property_name(const String& s)
{
	char buff[135];
	const char* property = s.string();
	snprintf(buff, 135, "Bad property name: %s", property);
	//WindowRep::errorMessage(buff);
	// NOT REACHED
}

void SessionRep::bad_property_value(const String& s)
{
	char buff[135];
	const char* property = s.string();
	snprintf(buff, 135, "Bad property value: %s", property);
	//WindowRep::errorMessage(buff);
	// NOT REACHED
}

// -----------------------------------------------------------------------
// Use ICCCM rules to find an application's instance name from the command
// line or an environment variable.
// -----------------------------------------------------------------------
String* SessionRep::find_name()
{
	 String name;
	if (find_arg(String("-name"), name))
	{
		return new String(name);
    }

	if (argc_ > 0)
	{
		String s(argv_[0]);
		int slash = s.rindex('/');
		if (slash >= 0) {
	    	s = s.right(slash + 1);
		}
		return new String(s);
	 }

	 return new String("noname");
}

// -----------------------------------------------------------------------
// Open the default display and initialize its style information.  For the
// current Mac version, this doesn't do much since we are only considering
// one display.
// -----------------------------------------------------------------------
void SessionRep::init_display()
{
	set_style(nil);
	display_ = Display::open();
	display_->style(iv_display_style_);
	connect(display_);
}

void SessionRep::connect(Display* d)
{
	 set_style(d);
}


// -----------------------------------------------------------------------
// Report that an argument is bad and exit.  A caller of this function
// may assume that it does not return.
//
// We also assume that arg is null-terminated (because it came
// from argv).
// -----------------------------------------------------------------------
void SessionRep::bad_arg(const char* fmt, const String& arg)
{
	char buff[135];
	snprintf(buff, 135, fmt, arg.string());
	//WindowRep::errorMessage(buff);
	 // NOT REACHED
}



// #######################################################################
// #####################  Session (unportable part)
// #######################################################################

// -----------------------------------------------------------------------
// This function is an extension of the InterViews distribution.
// A pathname of location of the installation directory tree is
// returned, that can be used to locate various pieces of configuration
// information (such as application defaults).
//
// This location is found under MS-Windows using the win.ini file (Uses
// the Win16 interface so that Win32s works... which doesn't support the
// registry at this time).
// -----------------------------------------------------------------------
const char* Session::installLocation()
{
	printf("Session::installLocation() not implemented\n");
	return nil;	
}



static Point where;
// Session::read -
//	This function is called by Session::Run when there is an event pending.  Because
// we are generating mouse moved events with every pixel of movement, events should
// be coming fast and furiously.  This style of handling mouse movement was taken from
// the Fresco ... natively, the Macintosh does not consider general mouse movements an
// event.
void Session::read(Event& e)
{				
assert(0);
// see iv_carbon_dialog_handle
#if 0
	RgnHandle region = NewRgn();
	SetRectRgn(region, where.h, where.v, (where.h + 1), (where.v +1));	
	WaitNextEvent(everyEvent, e.rep()->getEventRecord(), SLEEP_TIME, region);
	DisposeRgn(region);
	where = e.rep()->getEventRecord()->where;	
#endif
}

// our problem is that pending does not deal with move events.
bool read_if_pending(Event& e);
bool read_if_pending(Event& e)
{
#if 0
	RgnHandle region = NewRgn();
	SetRectRgn(region, where.h, where.v, (where.h + 1), (where.v +1));	
	bool b = WaitNextEvent(everyEvent, e.rep()->getEventRecord(), 0, region);
	DisposeRgn(region);
	if (b) {
		where = e.rep()->getEventRecord()->where;
	}
	return b;	
#else
#if defined(carbon)
	EventQueueRef eqr = GetMainEventQueue();
	EventTargetRef target = GetEventDispatcherTarget();
	EventRef er;
	while (ReceiveNextEvent(0, NULL, kEventDurationNoWait, true, &er) == noErr) {
		SendEventToEventTarget(er, target);
		ReleaseEvent(er);
	}
#else
	assert(0);
#endif
	return false;
#endif
}

/*
 * Read an event as above, but time out after a given (sec, usec) delay.
 * Return true if an event was read, false if the time-out expired.
 */
bool Session::read(long, long, Event&)
{
	printf("Session::read - unsupported\n");
	// NOT REACHED
	 return false;
}

/*
 * Check for a pending event, returning it if there is one.
 */
bool SessionRep::srcheck(Event&)
{
	printf("Session::check - unsupported\n");
	return false;
}

/*
 * Put an event back from whence it came.
 */
void Session::unread(Event&)
{
	printf("Session::unread - unsupported\n");
}

/*
 * Poll an event (implies the event already has an associated display).
 */
void Session::poll(Event&)
{
	printf("Session::poll - unsupported\n");
}

// the stdin event from another thread needs to go to the main thread
extern "C" {

static EventTypeSpec etype[] = {{'nrn1', 'nrn2'}};

static OSStatus ehandler(EventHandlerCallRef x, EventRef er, void*) {
	OSStatus result;
	QuitApplicationEventLoop();
	result = noErr;
	return result;
}

// after Apple-H to hide the windows. showing them leaves them blank,
// so must handle the show event
// when a menu is open and one clicks in another application then the
// menu should be closed so we must handle the deactivate event.
static EventTypeSpec showtype[] = {
  {kEventClassApplication, kEventAppShown},
  {kEventClassApplication, kEventAppDeactivated}
};

bool session_deactivating_;
bool need_motion_on_deactivate_;

static OSStatus show_handler(EventHandlerCallRef x, EventRef er, void*) {
	OSStatus result = noErr;
	switch (GetEventKind(er)) {
	case kEventAppShown:
//	printf("kEventAppShown\n");
		Session::instance()->screen_update();
		break;
	case kEventAppDeactivated:
//	printf("kEventAppDeactivated\n");
		Event e;
		if (e.grabber()) {
//printf("grabbing\n");
		    if (need_motion_on_deactivate_) {
			// motion out of window sometimes leaves a button highlighted
			e.rep()->set(Event::motion, Event::left, -1000, -1000);
		    }else{
			e.rep()->set(Event::up, Event::left, -1000, -1000);
		    }
			session_deactivating_ = true;
			EventRep::handleCallback(e);
			session_deactivating_ = false;
			Session::instance()->screen_update();;
// is still grabbing but it will go away as soon as any mouse press
// in any InterViews window
		}
		break;
	}
	return result;
}

int stdin_event_ready();
int dialog_running_ = 0;
int stdin_event_ready() {
	if (dialog_running_) {
		return 0;
	}
	OSStatus err;
	EventRef er;
	err = CreateEvent(NULL, 'nrn1' , 'nrn2', 0, kEventAttributeNone, &er);
	err = PostEventToQueue(GetMainEventQueue(), er, kEventPriorityStandard);
	ReleaseEvent(er);
	return 1;
}
}

// #######################################################################
// ###############  Session class (portable part)
// #######################################################################

Session::Session(
	const char* classname,
	int& argc,
	char** argv,
	const OptionDesc* opts,
	const PropertyData* initprops)
{
    SessionRep::instance_ = this;
    rep_ = new SessionRep();
    rep_->init(classname, argc, argv, opts, initprops);

    EventHandlerUPP hupp = NewEventHandlerUPP(ehandler);
    OSStatus err = InstallEventHandler(GetApplicationEventTarget(), hupp, 1, etype, 0, NULL);
    hupp = NewEventHandlerUPP(show_handler);
    err = InstallEventHandler(GetApplicationEventTarget(), hupp, 2, showtype, 0, NULL);
}

Session::~Session()
{
    delete rep_;
}

Session* Session::instance()
{
	return SessionRep::instance_;
}

const char* Session::name() const { return rep_->name_->string(); }
const char* Session::classname() const { return rep_->classname_; }
int Session::argc() const { return rep_->argc_; }
char** Session::argv() const { return rep_->argv_; }

Style* Session::style() const
{
	 SessionRep* s = rep_;
	if (s->display_ != nil)
		return s->display_->style(); 
	return s->style_;
}

// -----------------------------------------------------------------------
// These have no equivalent in MS-Windows as there is no network-based
// graphics available.  HASN'T BEEN THOUGHT ABOUT FOR THE MAC
// -----------------------------------------------------------------------
void Session::default_display(Display*)
{
}
Display* Session::default_display() const
{
	return rep_->display_;
}

Display* Session::connect(const String&)
{
	return rep_->display_;
}

Display* Session::connect(const char*)
{
	return rep_->display_;
}
    
void Session::disconnect(Display*)
{
}

#if USE_SIOUX_WINDOW  
//These functions and variables are included for the SIOUX interface which 
//Neuron uses
extern "C" {
	long SIOUXHandleOneEvent(EventRecord *);
	long ReadCharsFromConsole(char *buffer, long n);
//	extern WindowPtr SIOUXTextWindow;
	extern Boolean	SIOUXUseWaitNextEvent;
	Boolean IVOCGoodLine;
	char* hoc_console_buffer;		
}
#endif

void Session::screen_update() {
    		while(WindowRep::update_list.count() > 0){
    			WindowRep::update_list.item(0)->repair();
    			WindowRep::update_list.item(0)->update_ = false;
    			WindowRep::update_list.remove(0);
    		}
}

// -----------------------------------------------------------------------
// Event loops
// -----------------------------------------------------------------------
#if carbon
// event handling is claimed to be simplified in carbon. We assume
// event handlers have been installed. I'm hoping that the sioux console
// window will happily coexist with the interviews windows in the case
// that the console is waiting for input and all events will be nicely
// delivered. 
int Session::run() {
	screen_update();
//printf("RunApplicationEventLoop\n");
#if 1
	RunApplicationEventLoop();
#else
	EventTargetRef target = GetEventDispatcherTarget();
	EventRef er;
	while (ReceiveNextEvent(0, NULL, kEventDurationForever, true, &er) == noErr) {
		SendEventToEventTarget(er, target);
		ReleaseEvent(er);
	}
#endif
//printf("Return from RunApplicationEventLoop\n");
	return 0;
}


#else

int Session::run()
{
    Event e;
    bool& done = rep_->done_;
    EventRecord event;
    done = false;
    
#if USE_SIOUX_WINDOW     
   	WindowPtr temp = 0;
   	bool eventHandled;
   	SIOUXUseWaitNextEvent = false;
   	char buffer[100];
    int n=0;
    static int cnt=0;
#endif
    
    do {

#if USE_SIOUX_WINDOW    	
    	eventHandled = false;
    	IVOCGoodLine = false; //also set to false in SIOUX.c in Read from console
    	
    	//This code was an attempt to compensate for the case that there is only a SIOUX
		//window on the screen.  Haven't really had the opportunity to test it out.  
   		//if(MACwindow::noInterviewsWindows()){
   		//	SIOUXUseWaitNextEvent = true;
   		//	n = ReadCharsFromConsole(*hoc_console_buffer, 256);
		//	if(IVOCGoodLine){
		//		eventHandled = true;
		//		(*hoc_console_buffer)[n] = '\0';
		//		(*hoc_console_buffer)[(n - 1)] = '\n';
		//		return 0;
		//	}
		//}
#endif 	
    	//if there are no events available repair damaged Interviews Windows	
    	if (!(EventAvail(everyEvent, &event))) {
    		screen_update();
		}
#if 0 // promising but a lot of work just to get the title bar and scrollbar for free
#if USE_SIOUX_WINDOW
		eventHandled = SIOUXHandleOneEvent(&event);
		if (eventHandled) {	
				//n = ReadCharsFromConsole(hoc_console_buffer, 256);
				if(IVOCGoodLine){
					eventHandled = true;
					hoc_console_buffer[n] = '\0';
					hoc_console_buffer[(n - 1)] = '\n';
					return 0;
				}
		
		}
#endif
#else
#if USE_SIOUX_WINDOW 	
		//check to see if the SIOUX window needs to handle the event
		if(event.what != osEvt){  //avoid checking with the mouse movement calls
			FindWindow(event.where, &temp);
			if(((event.what == updateEvt) || (event.what == activateEvt)) &&
			   ((WindowPtr)(event.message) == SIOUXTextWindow->window)){ 
			    eventHandled = true;
			    SIOUXUseWaitNextEvent = true;
				SIOUXHandleOneEvent(nil);
				SIOUXUseWaitNextEvent = false;
			} else if(temp && (temp == SIOUXTextWindow->window) &&
			   (FrontWindow() == temp)){
				n = ReadCharsFromConsole(hoc_console_buffer, 256);
				if(IVOCGoodLine){
					eventHandled = true;
					hoc_console_buffer[n] = '\0';
					hoc_console_buffer[(n - 1)] = '\n';
					return 0;
				}
			}
		}
#endif
#endif
		
		//let Interviews handle the event if SIOUX did not.
#if USE_SIOUX_WINDOW
		if (SIOUXQuitting) {
			hoc_quit();
		} 
		if(!(eventHandled)){
#endif
			read(e);
			e.handle();

#if USE_SIOUX_WINDOW 
			
		}
#endif
		
	} while (!done);
#if USE_SIOUX_WINDOW //This code was merely for testing with the SIOUX console in
					 //the InterViews project. 
	//Reinstated for ivoc testing
	printf("InterViews has exited");
	SIOUXUseWaitNextEvent = true;
	ReadCharsFromConsole(buffer, 100);
#endif 
    return 0;
}
#endif

int Session::run_window(Window* w)
{
    w->map();
    return run();
}

void Session::quit()
{
    rep_->done_ = true;
    QuitApplicationEventLoop();
}
void Session::unquit() {
	rep_->done_ = false;
}

/*
 * Return loop status.
 */

bool Session::done() const
{
	return rep_->done_;
}

/*
 * Check if an event is pending on any display.
 */
bool Session::pending() const
{
	EventRecord event;
    return EventAvail(everyEvent, &event);
}

// -----------------------------------------------------------------------
// Initialize.... loading the styles based upon command line arguments
// and the application defaults settings.
// -----------------------------------------------------------------------
void SessionRep::init(
	const char* name,
	int& argc,
	char** argv,
	const OptionDesc* opts,
	const PropertyData* initprops)
{
	done_ = false;
    argc_ = argc;
    argv_ = new char*[argc + 1];
	for (int i = 0; i < argc; i++)
	{
		argv_[i] = argv[i];
    }
    argv_[argc_] = nil;

    init_style(name, initprops);
	if (opts != nil)
	{
		parse_args(argc, argv, opts);
    }
	parse_args(argc, argv, defoptions);
    init_display();

	// ---- build default cursors ----
	Cursor::init();

#ifdef sgi
    if (style_->value_is_on("malloc_debug")) {
	mallopt(M_DEBUG, 1);
    }
#endif
}

/*
 * Parse the argument list, setting any properties that are specified
 * by the option list.  Matching arguments are removed (in-place)
 * from the argument list.
 */
void SessionRep::parse_args(int& argc, char** argv, const OptionDesc* opts)
{
    int i;
    int newargc = 1;
    char* newargv[1024];
    newargv[0] = argv[0];
	for (i = 1; i < argc; i++)
	{
		bool matched = false;
		String arg(argv[i]);
		for (const OptionDesc* o = &opts[0]; o->name != nil; o++)
		{
			if (match(arg, *o, i, argc, argv))
			{
				matched = true;
				break;
	    	}
		}
		if (!matched)
		{
	    	newargv[newargc] = argv[i];
	    	++newargc;
		}
    }
	if (newargc < argc)
	{
		for (i = 1; i < newargc; i++)
		{
	    	argv[i] = newargv[i];
		}
		argc = newargc;
		argv[argc] = nil;
    }
}

/*
 * See if the given argument matches the option description.
 */

bool SessionRep::match(
	const String& arg,
	const OptionDesc& o,
	int& i,
	int argc,
	char** argv)
{
    String opt(o.name);
	if (arg != opt)
	{
		if (o.style == OptionValueAfter)
		{
	    	int n = opt.length();
			if (opt == arg.left(n))
			{
				style_->attribute(String(o.path), arg.right(n));
				return true;
	    	}
		}
		return false;
    }
    String name, value;
    extract(arg, o, i, argc, argv, name, value);
    style_->attribute(name, value);
    return true;
}

/*
 * Extract an attribute <name, value> from a given argument.
 */

void SessionRep::extract(
	const String& arg,
	const OptionDesc& o,
	int& i,
	int argc,
	char** argv,
	String& name,
	String& value)
{
    int colon;
	switch (o.style)
	{
    case OptionPropertyNext:
		value = next_arg(i, argc, argv, "missing property after '%s'", arg);
		colon = value.index(':');
		if (colon < 0)
		{
	    	bad_arg("missing ':' in '%s'", value);
		}
		else
		{
	    	name = value.left(colon);
	    	value = value.right(colon+1);
		}
		break;
    case OptionValueNext:
		name = o.path;
		value = next_arg(i, argc, argv, "missing value after '%s'", arg);
		break;
    case OptionValueImplicit:
		name = o.path;
		value = o.value;
		break;
    case OptionValueIsArg:
		name = o.path;
		value = arg;
		break;
    case OptionValueAfter:
		bad_arg("missing value in '%s'", arg);
		break;
    }
}

/*
 * Make sure there is another argument--if not generate an error.
 */

String SessionRep::next_arg(
	int& i,
	int argc,
	char** argv,
	const char* message,
	const String& arg)
{
    ++i;
	if (i == argc)
	{
		bad_arg(message, arg);
    }
    return String(argv[i]);
}

/*
 * Find the value for a specific argument.
 */
bool SessionRep::find_arg(
	const String& arg,
	String& value)
{
    int last = argc_ - 1;
	for (int i = 1; i < last; i++)
	{
		if (arg == argv_[i])
		{
	    	value = String(argv_[i+1]);
	    	return true;
		}
    }
    return false;
}

/*
 * Initialize style information for the session.
 */
void SessionRep::init_style(
	const char* name,
	const PropertyData* props)
{
    classname_ = name;
    name_ = find_name();
    style_ = new Style(*name_);
    Resource::ref(style_);
    style_->alias(classname_);
    props_ = props;
}

void SessionRep::load_props(
	Style* s,
	const PropertyData* props,
	int priority)
{
	if (props != nil)
	{
		for (const PropertyData* p = &props[0]; p->path != nil; p++)
		{
	    	s->attribute(String(p->path), String(p->value), priority);
		}
    }
}

void SessionRep::load_path(
	Style* s,
	const char* head,
	const char* tail,
	int priority)
{
    String h(head);
    String t(tail);
    auto sz = strlen(head) + strlen(tail) + 1;
    char* buff = new char[sz];
    snprintf(buff, sz, "%s%s", head, tail);
    load_file(s, buff, priority);
    delete buff;
}

void SessionRep::load_file(
	Style* s,
	const char* filename,
	int priority)
{
    InputFile* f = InputFile::open(String(filename));
	if (f == nil)
	{
		return;
    }
    const char* start;
    int len = f->read(start);
	if (len > 0)
	{
		load_list(s, String(start, len), priority);
    }
    f->close();
    delete f;
}

void SessionRep::load_list(
	Style* s,
	const String& str,
	int priority)
{
    const char* p = str.string();
    const char* q = p + str.length();
    const char* start = p;
	for (; p < q; p++)
	{
		if (*p == '\n')
		{
			if (p > start && *(p-1) != '\\')
			{
				load_property(s, String(start, (int)(p - start)), priority);
				start = p + 1;
	    	}
		}
    }
}

void SessionRep::load_property(
	Style* s,
	const String& prop,
	int priority)
{
	String p(strip(prop));
	if (p.length() == 0 || p[0] == '!')
	{
		return;
	}
	int colon = p.index(':');
	if (colon < 0)
	{
		missing_colon(p);
	}
	else
	{
		String name(strip(p.left(colon)));
		String value(strip(p.right(colon + 1)));
		if (name.length() <= 0)
		{
			bad_property_name(name);
		}
		else if (value.length() <= 0)
		{
			bad_property_value(value);
		}
		else
		{
			s->attribute(name, value, priority);
		}
	}
}

String SessionRep::strip(const String& s)
{
    int i = 0;
    int len = s.length();
    for (i = 0; i < len && isspace(s[i]); i++);
    int j = len - 1;
    for (; j >= 0 && isspace(s[j]); j--);
    return s.substr(i, j - i + 1);
}

