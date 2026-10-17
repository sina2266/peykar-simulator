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
//                     <IV-Win/Session.h>
//
//  The MS-Windows implementation of the InterViews Session class.  
//
//
// 1.1
// 1997/03/28 17:36:04
//
// ========================================================================
/*
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
#ifndef ivwin_Session_h
#define ivwin_Session_h

#include <InterViews/session.h>

class SessionRep
{
public:
	SessionRep();
	~SessionRep();

	int run();

    void load_app_defaults(Style*, const char* nm, int priority);
		// Load a file in the application defaults area with the name
		// "nm".  The method of locating the application defaults directory
		// is platform dependent.

public:
	const char* classname_;

// The following are expected to be moved to the platform-independant
// part (ie the Session class).
public:
	void init(
		const char*, int& argc, char** argv,
		const OptionDesc*, const PropertyData*
    );
    void parse_args(int& argc, char** argv, const OptionDesc*);
    bool match(
		const String& arg, const OptionDesc& o, int& i, int argc, char** argv
    );
    void extract(
		const String& arg, const OptionDesc& o, int& i, int argc, char** argv,
		String& name, String& value
    );
    void bad_arg(const char* fmt, const String& arg);
    	String next_arg(
		int& i, int argc, char** argv, const char* message, const String&
    );
    bool find_arg(const String& name, String& value);

    void init_style(const char*, const PropertyData*);
    String* find_name();
    void load_props(Style*, const PropertyData*, int priority);
    void load_environment(Style*, int priority);
    void load_path(Style*, const char*, const char*, int priority);
    const char* home();
    void load_file(Style*, const char* filename, int priority);
    void load_list(Style*, const String&, int priority);
    void load_property(Style*, const String&, int priority);
    String strip(const String&);
	void missing_colon(const String&);
    void bad_property_name(const String&);
    void bad_property_value(const String&);

    void init_display();
    void connect(Display*);
    void set_style(Display*);
    bool check(Event&);

private:
	friend class Session;
	String* name_;
    Style* style_;
	const PropertyData* props_;
	static Session* instance_;
	int argc_;
	char** argv_;
	Display* display_;
	bool done_; 
};


// ---- window classes registered ----
#define IV_WINDOW "ivWindow"

#endif // iv_win_Session_h
