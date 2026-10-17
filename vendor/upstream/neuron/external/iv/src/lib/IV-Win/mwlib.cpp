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
//                     <IV-Win/MWlib.c>
//
//  General definitions for the MW library, a library implementing an 
//  object oriented structure on top of MS-Windows.
//
//
// ========================================================================

// ---- local includes ----
#include <IV-Win/MWapp.h>
#include <OS/string.h>
#include <InterViews/style.h>

#ifndef WIN32
#include <MMgAlloc.h>
#endif

// ---- ansi/posix includes ----
#include <stdio.h>
#include <stdlib.h>

// ----------------------------------------------------------------------
// Visual assertion message.  This is provided in response to the 
// failure of an assertion test... ie it's a bug trap.
// ----------------------------------------------------------------------
void mwAssertion(
	const char* msg, 					// message to display
	const char* file, 					// file where it happened
	unsigned int line)					// line where it happened
{
	char buff[135];
	snprintf(buff, 512, "at line %u, file %s: `%s'", line, file, msg);
	MessageBeep(0);
	if (MessageBox(0, buff, "Assertion Failed", 
		MB_OKCANCEL | MB_ICONSTOP | MB_TASKMODAL) != IDOK)
	{
		exit(1);
	}
}

#ifndef WIN32
// ----------------------------------------------------------------------
// global new and delete operators
// ----------------------------------------------------------------------

MMgpAlloc allocator;

void* operator new(size_t size)
{
#ifdef __LARGE__
	return allocator.alloc(size);
#else
	NOT SUPPORTED
#endif
}

void operator delete(void* ptr)
{
	if (ptr)
    {
#ifdef __LARGE__
		allocator.free(ptr);
#else
		NOT SUPPORTED
#endif
	}
}

#endif /* WIN32 */

// --------------------------------------------------------------------
// style debug stuff.  
// --------------------------------------------------------------------
void dumpStyle(Style* s, const char* path) 
{
	const NullTerminatedString nm(*s->name());	// style name
	long i;										// tmp index
	char buff[512];

	// ---- load the attributes ----
	long nattr = s->attribute_count();
	for (i = 0; i < nattr; i++)
	{
		String an;
		String av;
		if (s->attribute(i, an, av))
		{
			NullTerminatedString aname(an);
			NullTerminatedString avalue(av);

			const char* adjusted = aname.string();
			while (*(adjusted) == '*')
				adjusted++;
			snprintf(buff, 512, "%s*%s: %s\n", path, adjusted, avalue.string());
			OutputDebugString(buff);
		}
	}

	// ---- do all child styles ----
	long nchild = s->children();
	for (i = 0; i < nchild; i++)
	{
		Style* c = s->child(i);
		const NullTerminatedString cnm(*c->name());	// style name
		if (c)
		{
			snprintf(buff, 512, "%s*%s", path, cnm.string());
			dumpStyle(c, buff);
		}
	}
}


