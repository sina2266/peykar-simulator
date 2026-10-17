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
//                     <MWlib.h>
//
//  General definitions for the MW library, a library implementing an 
//  object oriented structure on top of MS-Windows.
//
// 1.1
// 1997/03/28 17:35:55
//
// ========================================================================
#ifndef MWlib_h
#define MWlib_h

// ---- MS-Windows includes ----
// since cpp will still read the file even if ifdef'd out, and the windows
// file is quite large, a check for previous inclusion is done outside the
// include directive!
#ifdef WIN32
#ifndef _WINDOWS_
#include <windows.h>
#endif
#else
#ifndef __WINDOWS_H
#include <windows.h>
#endif
#endif

#include <stdlib.h>

void MWinitialize();
void MWcleanup();

// ---- error handling support ----
void mwAssertion(const char* msg, const char* file, unsigned int line);
#ifdef NDEBUG
// cannot be completely turned off becasue 52 cases with side effects
//#define MWassert(test) ((void) 0)
#define MWassert(test) ((void)(test))
#else
#define MWassert(test) ((void)((test) || (mwAssertion(#test,__FILE__,__LINE__),1)))
#endif

// global new and delete operators
//extern void* operator new(size_t size);
//extern void operator delete(void* ptr);

#define EXPORT


#include <InterViews/_enter.h>
class Style;

void dumpStyle(Style*, const char* p = "");
	// dumps the given style to the given file in the form that it is
	// normally read.  If the file is null, the style hierarchy is 
	// written out to the appropriate debug stream for the given platform.
	// This function is recursive, and the last argument is the current
	// path built up so far.  If a fixed path is desired to be prepended
	// to each line that is not present in the style, it can be supplied
	// as the path argument to the dumpStyle() call to the root style.

#include <InterViews/_leave.h>

#endif /* MWlib_h */
