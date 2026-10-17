// =======================================================================
//
// Windows BMP format raster loader.
//
// 1.1
// 1997/03/28 17:36:10
//
// InterViews Port to the Windows 3.1/NT operating systems
// Copyright (c) 1993 Tim Prinzing
//
// Permission to use, copy, modify, distribute, and sell this software and 
// its documentation for any purpose is hereby granted without fee, provided
// that (i) the above copyright notice and this permission notice appear in
// all copies of the software and related documentation, and (ii) the name of
// Tim Prinzing may not be used in any advertising or publicity relating to 
// the software without the specific, prior written permission of Tim Prinzing.
//
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
//
// IN NO EVENT SHALL Tim Prinzing BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
// INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER 
// RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE 
// POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR 
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// =======================================================================
#ifndef iv_winbmp_h
#define iv_winbmp_h

#include <InterViews/enter-scope.h>

#include <InterViews/_enter.h>

class BMPRaster
{
public:
  static Raster* load(const char *filename);
};

#include <InterViews/_leave.h>

#endif
