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


#include <IV-Win/MWapp.h>

// ---- ansi/posix includes ----
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

// If not a DLL, a global application is used
#ifndef __DLL__
MWapp theApp;
#endif
//MWapp* MWapp::registeredApps = NULL;

// #######################################################################
// #####################  MWapp class
// #######################################################################
MWapp::MWapp()
{
	hinst = 0;
	pinst = 0;
	cmdLine = 0;
	cmdShow = 0;

}

MWapp::~MWapp()
{
}

void MWapp::setModule(HINSTANCE h, HINSTANCE p, const char* cmd, int show)
{
	hinst = h;
	pinst = p;
	cmdLine = cmd;
	cmdShow = show;
}


// -----------------------------------------------------------------------
// main event loop
// -----------------------------------------------------------------------
int MWapp::run()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

// -----------------------------------------------------------------------
// application name
// -----------------------------------------------------------------------
const char* MWapp::appName()
{
	return "TBD";
}

void MWapp::appName(const char*)
{
}

