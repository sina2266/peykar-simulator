#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
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

/*
 * TextBuffer - editable text buffer
 */

// =======================================================================
//
// Moved out of the 2.6 compatibility area and made a bunch of functions
// const.
//
// 1.1
// 1997/03/28 17:36:37
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

//#include "c:/nrn/src/mswin/winio/debug.h"
#include <InterViews/regexp.h>
#include <OS/math.h>
#include <OS/memory.h>
#include <ctype.h>
#if !defined(WIN32) && !defined(__MWERKS__)
#include <memory.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <InterViews/iv3textbuffer.h>

static const char NEWLINE = '\012';

TextBuffer::TextBuffer (const char* t, int l, int s) 
{
	text = new char[s+1];
	Memory::zero(text,s+1);
	if (t && l > 0) {
		Memory::copy(t,text,l);
	}

    length = l;
    size = s;
    Memory::zero(text + length, size - length);
    linecount = 1 + LinesBetween(0, length);
    lastline = 0;
    lastindex = 0;
}

TextBuffer::~TextBuffer() 
{
	delete [] text;
}

// ---------------------------------------------------------------------
// Returns a string containing the text of the given line index.
// This is primarily a convenience function.  The returned string
// should be considered temporary... the contents of which will
// become invalid with the next edit operation.
// ---------------------------------------------------------------------
#if 1
//I (hines) am working around two apparent problems here. First, the string
// did not seem to be terminated. Second, it seemed to be deleted before
// it was returned.
static CopyString* tb_[20];
static int tbi_;
String TextBuffer::getNth(int i) const
{
	int index0 = LineIndex(i);
	int index1 = BeginningOfNextLine(index0);
	tbi_ = (tbi_ + 1)%20;
	if (tb_[tbi_]) { delete tb_[tbi_]; }
	tb_[tbi_] = new CopyString(Text(index0), index1 - index0);
//DebugMessage("%d %d %d %d |%s|\n", i, index0, index1,
//tb_[tbi_]->length(), tb_[tbi_]->string());
	return *tb_[tbi_];
}

#else
String TextBuffer::getNth(int i) const
{
	int index0 = LineIndex(i);
	int index1 = BeginningOfNextLine(index0);
	const String line(Text(index0), index1 - index0);
	return line;
}
#endif
inline int limit (int l, int x, int h) {
    return (x<l) ? l : (x>h) ? h : x;
}

int TextBuffer::Search (Regexp* regexp, int index, int range, int stop) {
    int s = limit(0, stop, length);
    int i = limit(0, index, s);
    return regexp->Search(text, s, i, range);
}

int TextBuffer::BackwardSearch (Regexp* regexp, int index) {
    int i = limit(0, index, length);
    int r = regexp->Search(text, length, i, -i);
    if (r >= 0) {
        return regexp->BeginningOfMatch();
    } else {
        return r;
    }
}

int TextBuffer::ForwardSearch (Regexp* regexp, int index) {
    int i = limit(0, index, length);
    int r = regexp->Search(text, length, i, length - i);
    if (r >= 0) {
        return regexp->EndOfMatch();
    } else {
        return r;
    }
}

int TextBuffer::Match (Regexp* regexp, int index, int stop) {
    int s = limit(0, stop, length);
    int i = limit(0, index, s);
    return regexp->Match(text, length, i);
}

bool TextBuffer::BackwardMatch (Regexp* regexp, int index) {
    int i = limit(0, index, length);
    for (int j = i; j >= 0; --j) {
        if (regexp->Match(text, length, j) == i - j) {
            return true;
        }
    }
    return false;
}

bool TextBuffer::ForwardMatch (Regexp* regexp, int index) {
    int i = limit(0, index, length);
    return regexp->Match(text, length, i) >= 0;
}

int TextBuffer::Insert (int index, const char* string, int count) {
    if (index < 0 || index > length) {
        return 0;
    } else if (count < 0) {
        return Insert(index + count, string, -count);
    } else {
		if (count > size - length)
		{
			int newSize = 2 * size + 1;
			if (count > newSize - length)
				newSize += count;
			char *newText = new char[newSize+1];
			Memory::zero(newText,newSize);
			Memory::copy(text,newText,size+1);			
			delete [] text;
			text = newText;
			size = newSize;
		}

        Memory::copy(text + index, text + index + count, length - index);
        Memory::copy(string, text + index, count);
        length += count;
        int newlines = (count == 1)
            ? (*string == NEWLINE)
            : LinesBetween(index, index + count);
        linecount += newlines;
        if (lastindex > index) {
            lastindex += count;
            lastline += newlines;
        }
        return count;
    }
}

int TextBuffer::Delete (int index, int count) {
    if (index < 0 || index > length) {
        return 0;
    } else if (count < 0) {
        return -Delete(index + count, -count);
    } else {
        count = Math::min(count, length - index);
        int oldlines = (count == 1)
            ? (text[index] == NEWLINE)
            : LinesBetween(index, index + count);
        if (lastindex > index + count) {
            lastindex -= count;
            lastline -= oldlines;
        } else if (lastindex >= index) {
            (void)LineNumber(index);
        }
        Memory::copy(
	    text + index + count, text + index, length - (index+count)
	);
        length -= count;
        Memory::zero(text + length, count);
        linecount -= oldlines;
        return count;
    }
}

int TextBuffer::Copy (int index, char* buffer, int count) {
    if (index < 0 || index > length) {
        return 0;
    } else if (count < 0) {
        return Copy(index + count, buffer, -count);
    } else {
        count = Math::min(count, length - index);
        Memory::copy(text + index, buffer, count);
        return count;
    }
}

int TextBuffer::Width () const
{
    int width = 0;
    int i = 0;
    while (i != length) {
        width = Math::max(width, EndOfLine(i) - i);
        i = BeginningOfNextLine(i);
    }
    return width;
}

int TextBuffer::LineIndex(int line) const
{
    int l = (line<0) ? 0 : (line>=linecount) ? linecount-1 : line;
    while (lastline > l) 
	{
        ((TextBuffer*) this)->lastline --;
        ((TextBuffer*) this)->lastindex = 
			BeginningOfLine(EndOfPreviousLine(lastindex));
    }
    while (lastline < l) 
	{
        ((TextBuffer*) this)->lastline ++;
        ((TextBuffer*) this)->lastindex = BeginningOfNextLine(lastindex);
    }
    if (line >= linecount) 
	{
        return EndOfText();
    } else 
	{
        return lastindex;
    }
}

int TextBuffer::LinesBetween (int index1, int index2) const
{
    if (index1 == index2) {
        return 0;
    } else if (index1 > index2) {
        return -LinesBetween(index2, index1);
    } else {
        const char* start = Text(index1);
        const char* finish = Text(index2);
        const char* tt;
        int l = 0;
	while (start < finish) {
	    tt = (char*)memchr(start, NEWLINE, finish - start);
	    if (tt == nil) {
		break;
	    }
            start = tt + 1;
            ++l;
        }
        return l;
    }
}

int TextBuffer::LineNumber (int index) const
{
    int l = LinesBetween(lastindex, index);
    ((TextBuffer*) this)->lastline += l;
    ((TextBuffer*) this)->lastindex = BeginningOfLine(index);
    return lastline;
}

int TextBuffer::LineOffset (int index) const
{
    return (index<0) ? 0 : (index>length) ? 0 : index-BeginningOfLine(index);
}

bool TextBuffer::IsBeginningOfLine (int index) const
{
    const char* t = Text(index);
    return t <= text || *(t-1) == NEWLINE;
}

int TextBuffer::BeginningOfLine (int index) const
{
    const char* t = Text(index);
    while (t > text && *(t-1) != NEWLINE) {
        --t;
    }
    return t - text;
}

int TextBuffer::BeginningOfNextLine (int index) const
{
    const char* t = Text(index);
    const char* e = text + length;
    t = (char*)memchr(t, NEWLINE, e - t);
    if (t == nil) {
        return length;
    } else {
        return t - text + 1;
    }
}

bool TextBuffer::IsEndOfLine (int index) const
{
    const char* t = Text(index);
    return t >= text + length || *t == NEWLINE;
}

int TextBuffer::EndOfLine (int index) const 
{
    const char* t = Text(index);
    const char* e = text + length;
    if (t == e) {
	return length;
    }
    t = (char*)memchr(t, NEWLINE, e - t);
    if (t == nil) {
        return length;
    } else {
        return t - text;
    }
}

int TextBuffer::EndOfPreviousLine (int index) const
{
    const char* t = Text(index-1);
    while (t > text && *t != NEWLINE) {
        --t;
    }
    return t - text;
}

bool TextBuffer::IsBeginningOfWord (int index) const
{
    const char* t = Text(index);
    return t <= text || (!isalnum(*(t-1)) && isalnum(*t));
}

int TextBuffer::BeginningOfWord (int index) const
{
    const char* t = Text(index);
    while (t > text && !(!isalnum(*(t-1)) && isalnum(*t))) {
        --t;
    }
    return t - text;
}

int TextBuffer::BeginningOfNextWord (int index) const
{
    const char* t = Text(index+1);
    while (t < text+length && !(!isalnum(*(t-1)) && isalnum(*t))) {
        ++t;
    }
    return t - text;
}

bool TextBuffer::IsEndOfWord (int index) const
{
    const char* t = Text(index);
    return t >= text+length || (isalnum(*(t-1)) && !isalnum(*t));
}

int TextBuffer::EndOfWord (int index) const
{
    const char* t = Text(index);
    while (t < text+length && !(isalnum(*(t-1)) && !isalnum(*t))) {
        ++t;
    }
    return t - text;
}

int TextBuffer::EndOfPreviousWord (int index) const
{
    const char* t = Text(index-1);
    while (t > text && !(isalnum(*(t-1)) && !isalnum(*t))) {
        --t;
    }
    return t - text;
}
