// =====================================================================
//  dirent.h 
//
//  An implementation of the BSD directory routines for MS-Windows NT.
//  With the alternative filesystems available under NT, filenames can
//  be up to 256 character long.  The berkeley routines have similar
//  counterparts in the WIN32 library.
//
//
// $Revision: 1342 $
// $Date: 2003-02-11 15:21:54 -0500 (Tue, 11 Feb 2003) $
//
// Windows 3.1/NT InterViews Port 
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
// ========================================================================== 
#ifndef nt_dir_h
#define nt_dir_h

#if defined(MINGW)
#include <io.h>
#include <dirent.h>
#include <dir.h>
#include <stdlib.h>
#else
#include <io.h>
#if defined(_MSC_VER)
#define ffblk _finddata_t
#define findfirst(a,b,c) _findfirst(a,b)
#define findnext _findnext
#define ff_name name 
#else
#include <dir.h>
#endif
#include <stdlib.h>

#define	rewinddir(dirp)	seekdir(dirp, 0L)
#define	MAXNAMLEN	_MAX_FNAME

struct dirent
{
    int d_namlen;					// length of d_name 
    char d_name[MAXNAMLEN + 1];		// name of the directory entry
};
// The handle returned by the _findfirst function is all that is needed
// to provide the access, but it must be swapped out to provide random
// access, so we put it inside a structure so the clients pointer can
// remain consistant.  If any assumptions were made about DIR (ie clients
// of the library used more knowledge than provided in the man page),
// you're screwed because it's radically different in implementation here!
typedef struct
{
    long srchHandle;			// handle to use
	struct ffblk data;	// data of current file
	struct dirent conversion;	// data in dirent format (almost anyway)
	short offs;					// offset from first 
} DIR;
   

// ---- prototypes ----
extern DIR *opendir(const char*);
extern struct dirent *readdir(DIR*);
extern void seekdir(DIR*, long);
extern long telldir(DIR*);
extern void closedir(DIR*);

#endif // MINGW undefined

#endif /* nt_dir_h */
