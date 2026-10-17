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
// 1997/03/28 17:36:09
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

//The name of the class has been changed so as to be able to coexist without
// conflicting with the IV-2_6/InterViews version
// They are almost identical but this creates and maintain it own copy
// of the buffer whereas the IV-2_6 version holds a pointer to the buffer which
// is allocated and freed elsewhere (and, e.g, idraw code explicitly changes
// the outiside buffer and expects the inside buffer to mirror it)

#ifndef iv3TextBuffer_h
#define iv3TextBuffer_h

#include <InterViews/enter-scope.h>
#include <OS/string.h>

#undef TextBuffer
#undef Text
#define TextBuffer iv3_TextBuffer
#define Text iv3_Text

class Regexp;

class TextBuffer 
{
public:
    TextBuffer(const char* buffer, int length, int size);
    virtual ~TextBuffer();

    int Search(Regexp* regexp, int index, int range, int stop);
    int ForwardSearch(Regexp* regexp, int index);
    int BackwardSearch(Regexp* regexp, int index);

    int Match(Regexp* regexp, int index, int stop);
    bool ForwardMatch(Regexp* regexp, int index);
    bool BackwardMatch(Regexp* regexp, int index);

    virtual int Insert(int index, const char* string, int count);
    virtual int Delete(int index, int count);
    int Copy(int index, char* buffer, int count);

    int Height() const;
    int Width() const;
    
    
#if MAC
#undef Length
#endif


    int Length() const;

    const char* Text() const;
    const char* Text(int index) const;
    const char* Text(int index1, int index2) const;
	 char Char (int index) const;

	String getNth(int line) const;
		// Returns a string containing the text of the given line index.
		// This is primarily a convenience function.  The returned string
		// should be considered temporary... the contents of which will
		// become invalid with the next edit operation.

	 int LineIndex(int line) const;
    int LinesBetween(int index1, int index2) const;
    int LineNumber(int index) const;
    int LineOffset (int index) const;
		// Map between text indices and line and offset positions.  lineIndex()
		// returns the index of the beginning of the line.  lineNumber() 
		// returns the number of the line that contains the given index.  
		// lineOffset() returns the offset of index from the beginning of it's
		// containing line.  linesBetween() returns the difference between 
		// the line numbers containing two indices.  A return value of zero
		// indicates the same line.  A positive number index2 is after
		// index1.  A negative number indicates index2 is before index1.  
		// Lines are numbered starting from zero.

    int PreviousCharacter(int index) const;
    int NextCharacter(int index) const;

    bool IsBeginningOfText(int index) const;
    int BeginningOfText() const;

    bool IsEndOfText(int index) const;
    int EndOfText() const;

    bool IsBeginningOfLine(int index) const;
    int BeginningOfLine(int index) const;
    int BeginningOfNextLine(int index) const;

    bool IsEndOfLine(int index) const;
    int EndOfLine(int index) const;
    int EndOfPreviousLine(int index) const;

    bool IsBeginningOfWord(int index) const;
    int BeginningOfWord(int index) const;
    int BeginningOfNextWord(int index) const;

    bool IsEndOfWord(int index) const;
    int EndOfWord(int index) const;
    int EndOfPreviousWord(int index) const;

protected:
    char* text;
    int length;
    int size;

private:
    int linecount;
    int lastline;
    int lastindex;
};

inline char TextBuffer::Char (int i) const
{
    return (i<0) ? text[0] : (i>length) ? text[length] : text[i];
}
inline const char* TextBuffer::Text () const
{
    return text;
}
inline const char* TextBuffer::Text (int i) const
{
    return text + ((i<0) ? 0 : (i>length) ? length : i);
}
inline const char* TextBuffer::Text (int i, int) const
{
    return text + ((i<0) ? 0 : (i>length) ? length : i);
}
inline int TextBuffer::PreviousCharacter (int i) const
{
    return (i<=0) ? 0 : i-1;
}
inline int TextBuffer::NextCharacter (int i) const
{
    return (i>=length) ? length : i+1;
}
inline bool TextBuffer::IsBeginningOfText (int i) const
{
    return i <= 0;
}
inline int TextBuffer::BeginningOfText () const
{
    return 0;
}
inline bool TextBuffer::IsEndOfText (int i) const
{
    return i >= length;
}
inline int TextBuffer::EndOfText () const
{
    return length;
}
inline int TextBuffer::Height () const
{
    return linecount;
}
inline int TextBuffer::Length () const
	{ return length; }

#endif /* iv3TextBuffer_h */
