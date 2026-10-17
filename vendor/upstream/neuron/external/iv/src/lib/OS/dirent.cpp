#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =====================================================================
//  dirent.c 
//
//  An implementation of the BSD directory routines for MS-Windows NT.
//  With the alternative filesystems available under NT, filenames can
//  be up to 256 character long.  The berkeley routines have similar
//  counterparts in the WIN32 library.
//
//
// 1.1
// 1997/03/28 22:04:30
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
// ================================================================== */

#include <OS/dirent.h>

#include <stdio.h>
#include <string.h>


// ------------------------------------------------------------------------
// open the directory named by name, and associate a directory stream with
// it.  Opendir returns a pointer to be used to identify the directory 
// stream in subsequent operations.  The pointer NULL is returned if "name"
// cannot be accessed, is not a directory, or store cannot be allocated.
// ------------------------------------------------------------------------
DIR* opendir(const char* name)
{
	char buff[1024];
	DIR* retval = (DIR*) malloc(sizeof(DIR));

	snprintf(buff, 1024, "%s/*", name);
	if (retval)
	{
		retval->offs = 0;
		retval->srchHandle = findfirst(buff, &(retval->data), 0);

		if (retval->srchHandle == -1)
		{
			free(retval);
			return 0;
		}
		return retval;
	}
	return 0;
}

// ------------------------------------------------------------------------
// Closes the directory stream, and frees the DIR structure.
// ------------------------------------------------------------------------
void closedir(DIR* dirp)
{
	if (dirp)
	{
//		findclose(dirp->srchHandle);
		free(dirp);
	}
}

// ------------------------------------------------------------------------
// returns a pointer to the next active entry.  NULL is returned upon 
// reaching the end of the directory, or upon detecting an invalid entry.
//
// The lifetime of the returned pointer should not be counted upon.  This
// is different from typical unix implementations, so applications that
// made assumptions about the implementation will break!!!
// ------------------------------------------------------------------------
struct dirent* readdir(DIR* dirp)
{
	if (dirp)
	{
		if (dirp->offs != 0)
		{
			// The first entry was already fetched when an attempt was
			// made to open the directory.  In this case we simply return
			// it, otherwise we need to try and fetch the next entry.
#if defined(WIN32) && defined (__MWERKS__)
			if (findnext(dirp->srchHandle, &(dirp->data)) == -1)
#else
			if (findnext(&(dirp->data)) == -1)
#endif
				return 0;
		}
		dirp->offs += 1;
		dirp->conversion.d_namlen = strlen(dirp->data.ff_name);
		strcpy(dirp->conversion.d_name, dirp->data.ff_name);
		return &(dirp->conversion);
	}
	return 0;
}

// ------------------------------------------------------------------------
// Sets the position of the next readdir() operation on the directory
// stream.  The new position reverts to the one associated with the value
// returned by a telldir() operation when fed to this routine (ie this is
// a random access function used to revisit a previous location).
//
// Since the NT functions only move forward using an existing handle, we
// have to close the existing and restart the operation.
// ------------------------------------------------------------------------
void seekdir(DIR* dirp, long off)
{
//	fprintf(stderr,"seekdir() - TBD\n");
	printf("seekdir() - TBD\n");
}

// ------------------------------------------------------------------------
// report the current relative position from the start of the directory.
// ------------------------------------------------------------------------
long telldir(DIR* dirp)
{
	if (dirp)
		return dirp->offs;
	else
		return 0;
}
