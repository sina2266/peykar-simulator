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
//                      <IV-Win/session.c>
//
// Session management for an application.  This class is a clearing house
// for the control part of an application.
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
#ifdef CYGWIN
#include <Dispatch/iohandler.h>
#include <Dispatch/dispatcher.h>
#endif
#include <InterViews/window.h>
#include <InterViews/style.h>
#include <InterViews/session.h>
#include <InterViews/cursor.h>
#include <InterViews/display.h>
#include <InterViews/event.h>
#include <IV-Win/session.h>
#include <IV-Win/event.h>
#include <IV-Win/window.h>
#include <IV-Win/MWapp.h>
#include <OS/file.h>
#include <OS/string.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef CYGWIN
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include <OS/dirent.h>
#endif

Session* SessionRep::instance_;

extern "C" { int bad_install_ok = 0; }

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


#ifdef CYGWIN
/*
 * in cygwin, this is a no-op -- just a way of using select to wait for events
 */

static int windows_messages_fd = -1;	// file number for select for windows messages 

class SessionIOHandler : public IOHandler {
public:
    SessionIOHandler(SessionRep*, Display*);

    virtual int inputReady(int);
private:
    SessionRep* session_;
    Display* display_;
};

SessionIOHandler::SessionIOHandler(SessionRep* s, Display* d) {
    session_ = s;
    display_ = d;
}

int SessionIOHandler::inputReady(int) {
  //    session_->handle_display_input(display_);
    return 0;
}
#endif

// #######################################################################
// #####################  SessionRep class
// #######################################################################

SessionRep::SessionRep()
{
#ifdef WIN32
	// The nice thing about NT is that you don't have to have the WinMain()
	// style of program entry... so under NT we enter under main so we get
	// a normal main(int,char**) and our source can go unchanged.  Also, this
	// way printf and friends work as well :-).
	//
	// The only side effect is that the module instance handle is still
	// needed, so we set it here.
	theApp.hinst = GetModuleHandle(NULL);
#endif
}

SessionRep::~SessionRep()
{
	delete name_;
    Resource::unref(style_);
	delete argv_;
}

// -----------------------------------------------------------------------
// Set the style information for the given display (which is ignored by
// the MS-Windows version since there is only one display!  The Windows
// version uses the profile string out of the users win.ini file to locate
// the file containing the application defaults
// -----------------------------------------------------------------------
// Hines: need to read the style in app-defa/intervie before
// creating a display since we need to deal with mswin_scale.

Style* iv_display_style_; // see display.cpp

void SessionRep::set_style(Display* d)
{
Style* s;
#if !OCSMALL
 if (Session::installLocation()) {
	char buf[512];
	snprintf(buf, 512, "%s\\%s\\%s", Session::installLocation(), APPDEF_DIRECTORY, APPDEF_DEFAULT);
	FILE* f;
	if ((f = fopen(buf, "r")) == (FILE*)0) {
		char buf2[512];
		snprintf(buf2, 512, "%s\\%s\\%s", Session::installLocation(), APPDEF_DIR_ALT, APPDEF_DEFAULT_ALT);
		if ((f = fopen(buf2, "r")) == (FILE*)0) {
#ifdef CYGWIN
		  // if we don't find the app defaults, just don't read them!
	 s = new Style(*style_);
	 load_props(s, props_, -5);
	iv_display_style_ = s;
	s->ref();
#else
			char buf3[1024];
			snprintf(buf3, 1024, "Can't open InterViews resources file in either\n%s or\n%s",
				buf, buf2);
			MessageBox(NULL, buf3, "Invalid Installation", MB_OK);
			abort();
#endif
		}
		APPDEF_DIRECTORY = APPDEF_DIR_ALT;
		APPDEF_DEFAULT = APPDEF_DEFAULT_ALT;
	}
	fclose(f);
    s = new Style(*style_);
	 load_app_defaults(s, APPDEF_DEFAULT, -5);
	 load_props(s, props_, -10);
	 load_app_defaults(s, classname_, -5);
 }else
#endif
 {	 s = new Style(*style_);
	 load_props(s, props_, -5);
 }

	iv_display_style_ = s;
	s->ref();
	//d->style(s);
}

void SessionRep::load_app_defaults(Style* s, const char* leafName, int priority)
{
	const char* topDir = Session::installLocation();
	if (topDir)
	{
		char subPath[80];
		snprintf(subPath, 80, "/%s/%s", APPDEF_DIRECTORY, leafName);
		load_path(s, topDir, subPath, priority);
	 }
}

// -----------------------------------------------------------------------
// Property parsing errors
// -----------------------------------------------------------------------
void SessionRep::missing_colon(const String& s)
{
	char buff[135];
	const char* property = s.string();
	snprintf(buff, 135, "Missing colon in property: %s", property);
	WindowRep::errorMessage(buff);
	// NOT REACHED
}

void SessionRep::bad_property_name(const String& s)
{
	char buff[135];
	const char* property = s.string();
	snprintf(buff, 135, "Bad property name: %s", property);
	WindowRep::errorMessage(buff);
	// NOT REACHED
}

void SessionRep::bad_property_value(const String& s)
{
	char buff[135];
	const char* property = s.string();
	snprintf(buff, 135, "Bad property value: %s", property);
	WindowRep::errorMessage(buff);
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
// MS-Windows version, this doesn't do much since there is only one display.
// -----------------------------------------------------------------------
void SessionRep::init_display()
{
#if defined(CYGWIN) || defined(MINGW)
  bad_install_ok = true;	// we're going to be ok with this!
#endif
	set_style(nil);
	display_ = Display::open();
	display_->style(iv_display_style_);
	connect(display_);
}



void SessionRep::connect(Display* d)
{
#ifdef CYGWIN
  windows_messages_fd = open("/dev/windows", O_RDONLY);
  if(windows_messages_fd < 0) {
    printf("error: could not connect to /dev/windows device for windows messages\n");
    return;
  }
  Dispatcher::instance().link
    (windows_messages_fd, Dispatcher::ReadMask, new SessionIOHandler(this, d));
#endif
//	 set_style(d);
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
	WindowRep::errorMessage(buff);
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
	static int first = 1;
	if (first && INSTALL_LOCATION == NULL)
	{
   	first = 0;
		// ---- get the location of the configuration directory ----
		const int topLen = 256;
		char buff[topLen];
		// maybe it hasn't been installed. So try NEURONHOME first
		// in case, say, we are running from a cd-rom.
		char* nh = getenv("NEURONHOME");
		int len = 0;
		if (nh) {
			snprintf(buff, topLen, "%s\\iv", nh);
			DIR* dirp = opendir(buff);
			if (dirp) {
				closedir(dirp);
				len = strlen(buff);
			}
		}
		if (len == 0) {
			len = GetProfileString(SECTION_NAME, LOCATION_KEY, "",
				buff, topLen);
		}
		if (len == 0)
		{
			//
			// The installation directory can't be found... so we bail out
			// here because all sorts of things won't work when this is the
			// case.
			//
		if (!bad_install_ok) {
			snprintf(buff, topLen, "win.ini is missing `%s' in section `%s'", LOCATION_KEY,
				SECTION_NAME);
			MessageBox(0, buff, "Invalid Installation",
				MB_OK | MB_ICONSTOP | MB_TASKMODAL);
			abort();
		}
		}
		else
		{
			INSTALL_LOCATION = new char[len + 1];
			strncpy(INSTALL_LOCATION, buff, len);
			INSTALL_LOCATION [len] = 0;
		}
	}
	return INSTALL_LOCATION;
}

// -----------------------------------------------------------------------
// The following are initially completely unsupported under MS-Windows.
// Some crude support could be accomplished here, but this interface really
// should be discouraged!
// -----------------------------------------------------------------------
void Session::read(Event& e)
{
#ifdef CYGWIN
  if(windows_messages_fd >= 0) {
    // if we have set select to monitor /dev/windows, then we can use one event loop!
    // if we have waiting messages, process them!
    MSG msg;
    if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
      // if we got a windows event, process it
      LPMSG emsg = &(e.rep()->msg);
      rep_->done_ = (GetMessage(emsg, NULL, 0, 0)) ? false : true;
      return;
    }
    // otherwise, wait for a msg
    Dispatcher::instance().dispatch();// wait till we get an event to process
    // if it is a windows message, do that
    if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
      // if we got a windows event, process it
      LPMSG emsg = &(e.rep()->msg);
      rep_->done_ = (GetMessage(emsg, NULL, 0, 0)) ? false : true;
    }
    // otherwise do nothing!
  }
  else {
    long sec = 0;
    long usec = 100;
    Dispatcher::instance().dispatch(sec, usec); // send any keyboard events
#endif
    LPMSG msg = &(e.rep()->msg);
    rep_->done_ = (GetMessage(msg, NULL, 0, 0)) ? false : true;
    // but then it blocks here at getmessage, so keyboard events need to be
    // enabled by touching a window and thereby generating events.
    // need to somehow have all events go through single select-like function
    // but GetMessage doesn't work on console input, and select doesnt!
#ifdef CYGWIN
  }
#endif
}

/*
 * Read an event as above, but time out after a given (sec, usec) delay.
 * Return true if an event was read, false if the time-out expired.
 */
bool Session::read(long, long, Event&)
{
	WindowRep::errorMessage("Session::read - unsupported");
	// NOT REACHED
	 return false;
}

/*
 * Check for a pending event, returning it if there is one.
 */

bool SessionRep::check(Event&)
{
	WindowRep::errorMessage("Session::check - unsupported");
	return false;
}

/*
 * Put an event back from whence it came.
 */
void Session::unread(Event&)
{
	WindowRep::errorMessage("Session::unread - unsupported");
}

/*
 * Poll an event (implies the event already has an associated display).
 */
void Session::poll(Event&)
{
	WindowRep::errorMessage("Session::poll - unsupported");
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
// graphics available.
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
#ifdef CYGWIN
  if(windows_messages_fd >= 0) {
    Dispatcher::instance().unlink(windows_messages_fd);
    close(windows_messages_fd);
    windows_messages_fd = -1;
  }
#endif
}

// -----------------------------------------------------------------------
// Event loops
// -----------------------------------------------------------------------
int Session::run()
{
    Event e;
    bool& done = rep_->done_;
    done = false;
    do {
		read(e);
		e.handle();
    } while (!done);
    return 0;
}

int Session::run_window(Window* w)
{
    w->map();
    return run();
}

void Session::quit()
{
    rep_->done_ = true;
    //    PostQuitMessage(1);
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
//	WindowRep::errorMessage("Session::pending - unsupported");
//	return false;
	MSG msg;
	return PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
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
    delete [] buff;
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
         	const char* q = p;
            if (*(q-1) == '\r') {q--;}
				load_property(s, String(start, (int)(q - start)), priority);
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

