#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif

#if carbon
#undef MAC
#endif

/*
 * Copyright (c) 1991 Stanford University
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

// =======================================================================
//
// 1.11
// 1999/08/11 19:13:36
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
// =======================================================================

/* 3/16/95 under borland 4.5 and win32s readdir, opendir
work only on directories that contain files (opendir returns
nil on a directory that contains only subdirectories and
readdir returns only files (not subdirectories or ..)
This is contrary to the borland documentation. Work around
is to use WIN32_FIND_DATA
*/
#ifdef WIN32
#include <IV-Win/MWlib.h>
// cygwin needs ctype to be included!
#ifdef CYGWIN
#include <ctype.h>
#endif
#endif

#include <OS/directory.h>
#include <OS/memory.h>
#include <OS/string.h>

/*
 * BSD tends to have things in <sys/dir.h>, System V uses <dirent.h>.
 * So far as I can tell, POSIX went with <dirent.h>.  Ultrix <dirent.h>
 * includes <sys/dir.h>, which is silly because <sys/dir.h>
 * needs <sys/types.h>.
 */
#if MAC
#if !carbon
#include <Files.h>
#endif
#include <ctype.h>
#endif


#ifndef MAC
	#include <OS/types.h>
#endif

#ifdef HAVE_OSFCN_H
#include <osfcn.h>
#endif

/* These lines copied out of the autoconfig documentation: */
#if HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# if HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

#if defined(WIN32)
// cygwin has a real dirent -- no need to hack it!
#ifdef CYGWIN
#include <dirent.h>
#else
#include <OS/dirent.h>
#endif
#include <windows.h>
//#include "/nrn/src/winio/debug.h"
#else

#ifndef MAC
	#include <dirent.h>
#endif

#endif /* WIN32 */

#if !defined (WIN32) && !defined(MAC)
#include <pwd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef MAC
	#include <sys/stat.h>
#endif

#if !defined (WIN32) && !defined (MAC)
/*
 * These hide in mysterious places on various systems.
 * For now, it seems safest just to declare them explicitly.
 */

extern "C" {
    extern uid_t getuid();
    extern void qsort(
	void*, size_t, size_t, int (*) (const void*, const void*)
    );
#ifdef __DECCXX
    extern struct passwd* getpwent();
    extern struct passwd* getpwnam(const char*);
    extern struct passwd* getpwuid(uid_t);
#endif
}

#endif /* WIN32 */

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

/*
 * Buffer size for internal path name computation.
 * The path stuff should really be reimplemented
 * with variable-length strings.
 */

static const int path_buffer_size = 1024 + 1;
class DirectoryImpl;

class DirectoryEntry {
public:
    const String& name() const;
    void set_is_dir(DirectoryImpl*);
    bool is_dir() { return is_dir_;}
private:
    friend class Directory;
    friend class DirectoryImpl;

    String* name_;
    bool is_dir_;
};

inline const String& DirectoryEntry::name() const { return *name_; }

class DirectoryImpl {
private:
    friend class Directory;
    friend class DirectoryEntry;

#ifdef MAC
	DirectoryImpl(String*);
	CInfoPBRec info_;
#else
#ifdef WIN32
	 DirectoryImpl(HANDLE, String*);
	HANDLE hwfd_;
#else
	 DirectoryImpl(DIR*, String*);
	 DIR* dir_;
#endif
#endif
    ~DirectoryImpl();

	 String* name_;
    DirectoryEntry* entries_;
    int count_;
    int used_;
    bool filled_;

    static unsigned int overflows_;

    DirectoryImpl& filled();
    void do_fill();

#if MAC
	static CopyString* mac_canonical(CopyString*);
#endif
    static bool dot_slash(const char*);
    static bool dot_dot_slash(const char*);
    static const char* home(const char*);
    static const char* eliminate_dot(const char*);
    static bool collapsed_dot_dot_slash(char*, char*& start);
    static const char* eliminate_dot_dot(const char*);
    static const char* interpret_slash_slash(const char*);
    static const char* interpret_tilde(const char*);
    static const char* expand_tilde(const char*, int);
    static const char* real_path(const char*);
    static bool ifdir(const char*);
};

	unsigned int DirectoryImpl::overflows_ = 0;

Directory::Directory() {
    impl_ = nil;
}

Directory::~Directory() {
    close();
    delete impl_;
}

Directory* Directory::current() {
    return open(".");
}

Directory* Directory::open(const String& name) {
#if MAC
	Directory* d = new Directory;
	CopyString* s1;
	if (name.length() == 0 || strcmp(name.string(), ".") == 0) {
		s1 = new CopyString(":");
	}else{
		if (name[name.length() - 1] == ':') {
			s1 = new CopyString(name);
		}else{
			char buf[256];
			snprintf(buf, 256, "%s:", name.string());
			s1 = new CopyString(buf);
		}
	}
	s1 = DirectoryImpl::mac_canonical(s1);
	d->impl_ = new DirectoryImpl(s1);
	CInfoPBRec& info = d->impl_->info_;
	char* c = (char*)&info;
	for (int i=0; i < sizeof(CInfoPBRec); ++i) {
		c[i] = 0;
	}
	info.dirInfo.ioCompletion = 0;
	info.dirInfo.ioFDirIndex = 0;
	info.dirInfo.ioVRefNum = 0;
	Str255 s;
	s[0] = strlen(d->impl_->name_->string());
	strcpy((char*)&s[1], d->impl_->name_->string()); 
	info.dirInfo.ioNamePtr = s;
	OSErr err = PBGetCatInfoSync(&info);
	if (err == noErr && (info.dirInfo.ioFlAttrib & 16 /*bit 4 */)) {
		return d;
	}else{
		delete d;
		return nil;
	}
#else
	 String* s = canonical(name);
    int not_dir = 1;
//DebugMessage("Directory::open name |%s|\n", name.string());
//DebugMessage("Directory::open s |%s|\n", s->string());
    /* cast is to workaround bug in some opendir prototypes */
#ifdef WIN32
	WIN32_FIND_DATA wfd;
//	HANDLE dir = FindFirstFile(s->string(), &wfd);
	HANDLE dir;
   dir = FindFirstFile(s->string(), &wfd);
   // WIN95 has trouble with the ./ directory
   if (dir == INVALID_HANDLE_VALUE) {
//   	printf("couldn't open |%s|\n", s->string());
      	char buf[256];
      	snprintf(buf, 256, "%s*", s->string());
		dir = FindFirstFile(buf, &wfd);
         if (dir == INVALID_HANDLE_VALUE) {
//         	printf("couldn't open |%s|\n", buf);
         }else{
         	not_dir = 0;
//         	printf("successful |%s| %d |%s|\n", buf,
//            (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0, wfd.cFileName);
         }
   }
	if (dir == INVALID_HANDLE_VALUE) {
//		MessageBox(NULL, s->string(), "Directory::open failure", MB_OK);
		dir = 0;
	}else if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		FindClose(dir);
		dir = 0;
	}
#else
	 DIR* dir = opendir((char*)s->string());
#endif
	if (dir == nil && not_dir) {
	delete s;
	return nil;
	 }
    Directory* d = new Directory;
    d->impl_ = new DirectoryImpl(dir, s);
    return d;
#endif
}

void Directory::close() {
	
	 DirectoryImpl& d = *impl_;
#ifdef WIN32
	if (d.hwfd_ != nil) {
		FindClose(d.hwfd_);
		d.hwfd_ = nil;
	}
#endif
#if defined(WIN32) || MAC
	if (d.entries_) {
#else
	 if (d.dir_ != nil) {
	closedir(d.dir_);
	d.dir_ = nil;
#endif
	DirectoryEntry* end = &d.entries_[d.used_];
	for (DirectoryEntry* e = &d.entries_[0]; e < end; e++) {
	    delete e->name_;
	}
	delete [] d.entries_;
	d.entries_ = nil;    
    }
}

const String* Directory::path() const {
    DirectoryImpl& d = *impl_;
    return d.name_;
}

int Directory::count() const {
    DirectoryImpl& d = impl_->filled();
    return d.used_;
}

const String* Directory::name(int i) const {
    DirectoryImpl& d = impl_->filled();
    if (i < 0 || i >= d.used_) {
	/* raise exception -- out of range */
	return nil;
    }
    return d.entries_[i].name_;
}

int Directory::index(const String& name) const {
    NullTerminatedString ns(name);
    const char* s = ns.string();
    DirectoryImpl& d = impl_->filled();
    int i = 0, j = d.used_ - 1;
    while (i <= j) {
	int k = (i + j) / 2;
	int cmp = strcmp(s, d.entries_[k].name_->string());
	if (cmp == 0) {
	    return k;
	}
	if (cmp > 0) {
	    i = k + 1;
	} else {
	    j = k - 1;
	}
    }
    return -1;
}

bool Directory::is_directory(int i) const {

    DirectoryImpl& d = impl_->filled();
    if (i < 0 || i >= d.used_) {
	/* raise exception -- out of range */
	return false;
    }
    DirectoryEntry& e = d.entries_[i];
    return e.is_dir_;
}

void DirectoryEntry::set_is_dir(DirectoryImpl* d) {
#if MAC
#else
	struct stat* s = new (struct stat);
        auto sz = d->name_->length() + name_->length() + 2;
	char* tmp = new char[sz];
#ifdef WIN32
	snprintf(tmp, sz, "%s%s", d->name_->string(), name_->string());
#else
	snprintf(tmp, sz, "%s/%s", d->name_->string(), name_->string());
#endif
	int i = stat(tmp, s);
	delete [] tmp;
	if (i != 0) {
		is_dir_ = false;
	}else{
		is_dir_ =  S_ISDIR(s->st_mode);
	}
	delete s;
#endif
}

inline bool DirectoryImpl::dot_slash(const char* path) {
    return path[0] == '.' && (path[1] == '/' || path[1] == '\0');
}

inline bool DirectoryImpl::dot_dot_slash(const char* path) {
    return (path[0] == '.' && path[1] == '.' &&
	(path[2] == '/' || path[2] == '\0')
    );
}

String* Directory::canonical(const String& name) {
#ifdef MAC
	return new CopyString(name);
#else
    NullTerminatedString ns(name);
	 const char* path = ns.string();
#ifdef WIN32
	{
		char* cp;
		for (cp = (char*)path; *cp; ++cp) {
			if (*cp == '\\') {
				*cp = '/';
			}
		}
	}
#endif
    static char newpath[path_buffer_size];
    const char* s = DirectoryImpl::interpret_slash_slash(path);
    s = DirectoryImpl::eliminate_dot(s);
    s = DirectoryImpl::eliminate_dot_dot(s);
    s = DirectoryImpl::interpret_tilde(s);
    if (s[0] == '\0' || strcmp(s, ".") == 0) {
	snprintf(newpath, path_buffer_size, "./");
#ifndef WIN32
    } else if (!DirectoryImpl::dot_slash(s) &&
	!DirectoryImpl::dot_dot_slash(s) && s[0] != '/'
    ) {
	snprintf(newpath, path_buffer_size, "./%s", s);
#endif
    } else if ((DirectoryImpl::ifdir(s) && s[strlen(s) - 1] != '/')
    					|| s[strlen(s) -1] == ':'
    		) {
	snprintf(newpath, path_buffer_size, "%s/", s);
    } else {
	snprintf(newpath, path_buffer_size, "%s", s);
    }
    return new CopyString(newpath);
#endif
}

#if MAC
CopyString* DirectoryImpl::mac_canonical(CopyString* name) {
	//replace :name:: with :
	// just enough to work most of the time, ie at end
	CopyString* s = name;
	CopyString& nm = *name;
	int n = name->length();
	if (n > 3 && nm[--n] == ':' && nm[--n] == ':' && nm[--n] != ':') {
		while (--n >= 0 && nm[n] != ':') {
			;
		}
		if (n >= 0) {
			s = new CopyString(nm.left(n+1));
		}else{
			s = new CopyString(":");
		}
		delete name;
	}		
	return s;
}
#endif

static bool s_eq_p(const char* s, const char* p) {
#if defined(WIN32) || MAC
	return toupper(*s) == toupper(*p);
#else
	return *s == *p;
#endif
}

bool Directory::match(const String& name, const String& pattern) {
    const char* s = name.string();
    const char* end_s = s + name.length();
    const char* p = pattern.string();
    const char* end_p = p + pattern.length();
#if 0
    for (; p < end_p; p++, s++) {
	if (*p == '*') {
	    const char* pp = p + 1;
	    if (pp == end_p) {
		return true;
	    }
	    for (; s < end_s && *s != *pp; s++);
	    p = pp;
	} else if (s >= end_s || *p != *s) {
	    return false;
	}
    }
    return s == end_s;
#else
    // ro2m: re-written so that it actually works..
    const char* prev_p = NULL;
    const char* prev_s = NULL;
    while((p < end_p) && (s < end_s)) {
      if (*p == '*') {
	const char* pp = p + 1;
	if (pp == end_p)	/* if last one is a star, must be ok! */
	  return true;
	while((s < end_s) && (!s_eq_p(s, pp))) s++; /* skip to matching s */
	if(s == end_s)
	  return false;		/* didn't get it */
	p = pp;
	prev_p = p;		/* if we get stuck, go back here */
	prev_s = s + 1;
      }
      if(!s_eq_p(p, s)) {
	if(prev_p == NULL)
	  return false;
	p = prev_p;
	s = prev_s;
	while((s < end_s) && (!s_eq_p(s, p))) s++; /* skip to matching s */
	if(s == end_s)
	  return false;		/* didn't get it */
	prev_s = s + 1;
      }
      p++; s++;
      if(s == end_s) {
	if((p == end_p) || (*p == '*'))
	  return true;
	return false;
      }
    }
    return false;
#endif
}

/** class DirectoryImpl **/


#if MAC
DirectoryImpl::DirectoryImpl(String* name) {
#else
#ifdef WIN32
DirectoryImpl::DirectoryImpl(HANDLE d, String* name) {
	hwfd_ = d;
#else
DirectoryImpl::DirectoryImpl(DIR* d, String* name) {
	 dir_ = d;
#endif
#endif
    entries_ = nil;
    count_ = 50*overflows_;
    entries_ = new DirectoryEntry[count_];
    used_ = 0;
    filled_ = false;
	name_ = name;
}

DirectoryImpl::~DirectoryImpl() {
    delete name_;
}

DirectoryImpl& DirectoryImpl::filled() {
    if (!filled_) {
	do_fill();
	filled_ = true;
    }
    return *this;
}

// directories in alpha order then files
static int compare_entries(const void* k1, const void* k2) {
    DirectoryEntry* e1 = (DirectoryEntry*)k1;
    DirectoryEntry* e2 = (DirectoryEntry*)k2;
	if (e1->is_dir() != e2->is_dir()) {
		return e1->is_dir() ? -1 : 1;
	}
    return strcmp(e1->name().string(), e2->name().string());
}
#if MAC
void DirectoryImpl::do_fill() {
	int i;
	DirInfo& dibase = info_.dirInfo;
	CInfoPBRec info;
	char* c = (char*)&info;
	for (int i=0; i < sizeof(CInfoPBRec); ++i) {
		c[i] = 0;
	}

	DirInfo& di = info.dirInfo;
	int cnt = dibase.ioDrNmFls;
	int dirid = dibase.ioDrDirID;
	int volid = dibase.ioVRefNum;
	Str255 s;
	di.ioNamePtr = s;
	for (i=0; i <= cnt; ++i) {
	  if (i == 0) {
	  	if (1) {
	  		s[0] = 1; s[1] = ':';
	  	}else{
	  		continue;
	  	}
	  }else{
		di.ioDrDirID = dirid;
		di.ioVRefNum = volid;
	 	di.ioFDirIndex = i;
	 	di.ioACUser = 0;
	 	OSErr err = PBGetCatInfoSync(&info);
	 	if (err != noErr) {
	 		printf("error in DirectoryImpl::do_fill\n");
	 	}
	  }
	 	s[s[0] + 1] = '\0';
		if (used_ >= count_) {
		 	++overflows_;
		 	int new_count = count_ + 50*overflows_;
		 	DirectoryEntry* new_entries = new DirectoryEntry[new_count];
		 	Memory::copy(
				entries_, new_entries, count_ * sizeof(DirectoryEntry)
		 	);
		 	delete [] entries_;
		 	entries_ = new_entries;
		 	count_ = new_count;
		}
		DirectoryEntry& e = entries_[used_];
		e.name_ = new CopyString((char*)&s[1]);
//printf("%d %s\n", di.ioFDirIndex, e.name_->string());
		if (i == 0){ // the parent directory
			e.is_dir_ = true;
		}else{
			e.is_dir_ = (di.ioFlAttrib & 16) != 0;
		}
		++used_;
	 }
	 qsort(entries_, used_, sizeof(DirectoryEntry), &compare_entries);
}
#else
#ifdef WIN32
void DirectoryImpl::do_fill() {
	 WIN32_FIND_DATA fd;
    HANDLE h;
         auto sz = strlen(name_->string()) + 3;
	 char * buf = new char[sz];
	 snprintf(buf, sz, "%s%s", name_->string(), "*");
	 for (h = FindFirstFile(buf, &fd); FindNextFile(h, &fd);) {
	if (used_ >= count_) {
		 ++overflows_;
		 int new_count = count_ + 50*overflows_;
		 DirectoryEntry* new_entries = new DirectoryEntry[new_count];
		 Memory::copy(
		entries_, new_entries, count_ * sizeof(DirectoryEntry)
		 );
		 delete [] entries_;
		 entries_ = new_entries;
		 count_ = new_count;
	}
	DirectoryEntry& e = entries_[used_];
	e.name_ = new CopyString(fd.cFileName);
	e.set_is_dir(this);
	++used_;
	 }
	 delete [] buf;
	 FindClose(h);
	 qsort(entries_, used_, sizeof(DirectoryEntry), &compare_entries);
}
#else
void DirectoryImpl::do_fill() {
//#ifdef apollo  // Not needed any more because on apollo we do #define dirent direct.
//	 for (struct direct* d = readdir(dir_); d != nil; d = readdir(dir_)) {
//#else
	 for (struct dirent* d = readdir(dir_); d != nil; d = readdir(dir_)) {
//#endif
	if (used_ >= count_) {
		 ++overflows_;
		 int new_count = count_ + 50*overflows_;
		 DirectoryEntry* new_entries = new DirectoryEntry[new_count];
		 Memory::copy(
		entries_, new_entries, count_ * sizeof(DirectoryEntry)
		 );
		 delete [] entries_;
		 entries_ = new_entries;
		 count_ = new_count;
	}
	DirectoryEntry& e = entries_[used_];
	e.name_ = new CopyString(d->d_name);
	e.set_is_dir(this);
	++used_;
	 }
	 qsort(entries_, used_, sizeof(DirectoryEntry), &compare_entries);
}
#endif
#endif

const char* DirectoryImpl::home(const char* name) {
#if defined(WIN32) || MAC
	 return nil;
#else
	 struct passwd* pw;
	 if (name == nil) {
	pw = getpwuid(getuid());
	 } else {
	pw = getpwnam(name);
	 }
	 return (pw == nil) ? nil : pw->pw_dir;
#endif
}

const char* DirectoryImpl::eliminate_dot(const char* path) {
	 static char newpath[path_buffer_size];
	 const char* src;
	 char* dest = newpath;

	 const char* end = &path[strlen(path)];
	 for (src = path; src < end; src++) {
	if (dot_slash(src) && dest > newpath && *(dest - 1) == '/') {
		 src++;
	} else {
		 *dest++ = *src;
	}
	 }
	 *dest = '\0';
	 return newpath;
}

bool DirectoryImpl::collapsed_dot_dot_slash(char* path, char*& start) {
	 if (path == start || *(start - 1) != '/') {
	return false;
	 }
	 if (path == start - 1 && *path == '/') {
	return true;
	 }
	 if (path == start - 2) {	/* doesn't handle double-slash correctly */
	start = path;
	return *start != '.';
	 }
	 if (path < start - 2 && !dot_dot_slash(start - 3)) {
	for (start -= 2; path <= start; --start) {
		 if (*start == '/') {
		++start;
		return true;
		 }
	}
	start = path;
	return true;
    }
    return false;
}

const char* DirectoryImpl::eliminate_dot_dot(const char* path) {
    static char newpath[path_buffer_size];
    const char* src;
    char* dest = newpath;

    const char* end = &path[strlen(path)];
    for (src = path; src < end; src++) {
	if (dot_dot_slash(src) && collapsed_dot_dot_slash(newpath, dest)) {
	    src += 2;
	} else {
	    *dest++ = *src;
	}
    }
    *dest = '\0';
    return newpath;
}

const char* DirectoryImpl::interpret_slash_slash(const char* path) {
    for (int i = strlen(path) - 1; i > 0; --i) {
	if (path[i] == '/' && path[i - 1] == '/') {
	    return &path[i];
	}
    }
    return path;
}

const char* DirectoryImpl::interpret_tilde(const char* path) {
    static char realpath[path_buffer_size];
    const char* beg = strrchr(path, '~');
    bool valid = (beg != nil && (beg == path || *(beg - 1) == '/'));
    if (valid) {
	const char* end = strchr(beg, '/');
	int length = (end == nil) ? strlen(beg) : (end - beg);
	const char* expanded = expand_tilde(beg, length);
	if (expanded == nil) {
	    valid = false;
	} else {
	    strcpy(realpath, expanded);
	    if (end != nil) {
		strcat(realpath, end);
	    }
	}
    }
    return valid ? realpath : path;
}

const char* DirectoryImpl::expand_tilde(const char* tilde, int length) {
    const char* name = nil;
    if (length > 1) {
	static char buf[path_buffer_size];
	strncpy(buf, tilde + 1, length - 1);
	buf[length - 1] = '\0';
	name = buf;
    }
    return home(name);
}

const char* DirectoryImpl::real_path(const char* path) {
    const char* realpath;
    if (*path == '\0') {
	realpath = "./";
    } else {
	realpath = interpret_tilde(interpret_slash_slash(path));
    }
    return realpath;
}

bool DirectoryImpl::ifdir(const char* path) {
#if MAC
	Directory* dir = Directory::open(path);
	if (dir) {
		delete dir;
		return true;
	}
	return false;
#else
    struct stat st;
    return stat((char*) path, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}


