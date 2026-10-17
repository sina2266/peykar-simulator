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

// ============================================================================
//
//                    <IV-Win/MWapp.h>
//
// Application class for the Microsoft-Windows layer.  
//
// 1.1
// 1997/03/28 17:35:55
//
// ========================================================================
#ifndef MWapp_h
#define MWapp_h

#include <IV-Win/MWlib.h>

class MWapp
{
public:
	MWapp();
	~MWapp();

	int run();
	void quit(int ret = 0);
	void setModule(HINSTANCE, HINSTANCE, const char*, int);
	const char* appName();
	void appName(const char* nm);

public:
	HINSTANCE hinst;					// application module handle
	char scratch[1024];					// static heap for general use

private:
	HINSTANCE pinst;
	const char* cmdLine;                // command line text
	int cmdShow;

};

// ---- inline functions ----
inline void MWapp::quit(int ret)
	{ PostQuitMessage(ret); }

extern MWapp theApp;

#endif /* MWapp_h */
