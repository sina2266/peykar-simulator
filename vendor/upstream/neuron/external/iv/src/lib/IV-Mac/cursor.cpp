#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =========================================================================
//
//                   cursor.c
//
// Macintosh implementation of the InterViews cursor class.
//
//
//
// 1.1
// $Date:   4 Aug 1996 
// ======================================================================

#if carbon
static Cursor mac_arrow; // before defining the InterViews Cursor
#endif

#include <InterViews/bitmap.h>
#include <InterViews/color.h>
#include <InterViews/cursor.h>
#include <InterViews/session.h>
#include <IV-Mac/cursor.h>
#include <IV-Mac/bitmap.h>
#include <IV-Mac/window.h>

#include <stdio.h>
#include <stdlib.h>

Cursor* defaultCursor;
Cursor* arrow;
Cursor* crosshairs;
Cursor* ltextCursor;
Cursor* rtextCursor;
Cursor* hourglass;
Cursor* upperleft;
Cursor* upperright;
Cursor* lowerleft;
Cursor* lowerright;
Cursor* noCursor;

/*
 * Define the builtin cursors.
 */

void Cursor::init() 
{
	//
	// The MS include file define MAKEINTRESOURCE to cast to a string, which
	// is not the integer that it really is (which we expect as a constructor
	// argument.  Therefore, we cast the IDC_XXX values to an int.
	//
	 //arrow = new Cursor((int) IDC_ARROW);
	//crosshairs = new Cursor((int) IDC_CROSS);
	 
    
    crosshairs = new Cursor(crossCursor);
    hourglass = new Cursor(watchCursor);
    
    
    //Arrow seems to need a special setup since it shares the name with the
    //Mac native definition.  A new cursor is formed by calling a new Cursor with
    //a predefined cursor.  It's data member "theCursor" is then forced to be a cursor 
    //handle to the Macintosh arrow.  This method is ugly but it is the only way I 
    //could think of doing it at the time.
    arrow = new Cursor(crossCursor);
#if carbon
//    following line gives bus error on osx.
//    GetQDGlobalsArrow((arrow->rep())->theCursor);
	GetQDGlobalsArrow(&mac_arrow);
	*(arrow->rep())->theCursor = &mac_arrow;
#else 
    *((arrow->rep())->theCursor) = &(qd.arrow);
#endif
    
    defaultCursor = arrow;
    
}

// -----------------------------------------------------------------------
// constructors and destructors for the Cursor class.
// -----------------------------------------------------------------------

/*
 * Create a cursor a specific pattern and mask (16x16 in size)
 */
Cursor::Cursor(
	short xoff,
	short yoff,
	const int* p,
	const int* m,
	const Color*,					// foreground color
	const Color *)                  // background color
{
	//printf("This Cursor::Cursor(short xoff ... is not yet implemented\n");
    rep_ = new CursorRep(xoff, yoff, p, m);
}

/*
 * Create a cursor from bitmaps.
 */

Cursor::Cursor(
	const Bitmap* pat,
	const Bitmap* mask,         
	const Color* ,			   	// foreground color - unsupported
	const Color* )              // background color - unsupported
{
	//printf("This Cursor::Cursor(const Bitmap ... is not yet implemented\n");
	rep_ = new CursorRep(*pat->rep(), *mask->rep());
}

/*
 * Create a cursor from a font.
 */
Cursor::Cursor(
	const Font*,                   // font to use
	int ,                          // data character
	int ,                          // mask character
	const Color* ,                 // foreground color
	const Color* )                 // background color
{
	// !!!! currently unsupported !!!!!
	//WindowRep::errorMessage("TBD - cursor from a font");
	printf("NOT IMPLEMENTED - Create cursor from font");
	exit(1);
}

/*
 * Create a cursor from the predefined cursor font.
 */
Cursor::Cursor(
	int n,                        // resource id
	const Color*,                 // foreground color
	const Color*)                 // background color
{
    rep_ = new CursorRep(n);
}

Cursor::Cursor(
	const char* n)
{
	rep_ = new CursorRep(n);
}

Cursor::~Cursor()
{
    delete rep_;
}

// #######################################################################
// ##################  class CursorRep
// #######################################################################

CursorRep::CursorRep(int id)
{
	theCursor = GetCursor(id);

}

CursorRep::CursorRep(const char* n)
{
	printf("NOT IMPLEMENTED - CursorRep(const char *n)");
	exit(1);	
}

CursorRep::CursorRep(
	BitmapRep& data,
	BitmapRep& mask)
{
	printf("NOT IMPLEMENTED -  CursorRep(const bitmap)");
	theCursor = GetCursor(4);
}

CursorRep::CursorRep(
	short hot_x,
	short hot_y,
	const int* data,
	const int* mask)
{
	printf("NOT IMPLEMENTED - Data Mask Cursor");
	theCursor = GetCursor(4);
}

CursorRep::~CursorRep()
{
	//char buf[100];
	//DestroyCursor(cursor);
	printf("NOT IMPLEMENTED - destroy cursor");
	exit(1);
}

