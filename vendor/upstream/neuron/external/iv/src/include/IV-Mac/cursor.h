// =======================================================================
//
//                     <IV-Win/Cursor.h>
//
//  MS-Windows implementation of the InterViews cursor class.  
// 
//  The CursorRep class is basically a small wrapper around a Machintosh 
//  CursHandle.  
//
// 1.1
// 1997/03/28 17:35:41
//
// =======================================================================
#ifndef iv_mac_cursor_h
#define iv_mac_cursor_h

class Color;
class Display;
class WindowVisual;

class CursorRep
{
public:

	CursorRep(int);
	CursorRep(const char *);
	CursorRep(BitmapRep& data, BitmapRep& mask);
	CursorRep(short, short, const int* data, const int* mask);
    ~CursorRep();
	
	CursHandle theCursor;
};


#endif
