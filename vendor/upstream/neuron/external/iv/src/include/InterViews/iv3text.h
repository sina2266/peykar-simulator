/*
 * Copyright (c) 1991 Redwood Design Automation
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the name of
 * Redwood Design Automation may not be used in any advertising or publicity
 * relating to the software without the specific, prior written permission of
 * Redwood Design Automation.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL REDWOOD DESIGN AUTOMATION BE LIABLE FOR ANY SPECIAL,
 * INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT
 * ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

// =======================================================================
//
// Reworked to remove use of the Redwood utility classes, and use the
// InterViews classes instead.
//
// 1.1
// 1997/03/28 17:36:08
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

#ifndef iv3Text_h
#define iv3Text_h

#include <InterViews/iv3textbuffer.h>
#include <InterViews/action.h>
#include <InterViews/patch.h>
#include <InterViews/observe.h>
#include <InterViews/adjust.h>
#include <OS/list.h>

#undef TextLine
#undef TextRegion
#undef TextAnnotation
#undef TextLocation
#undef Text
#undef TextLineAdjuster

#define TextLine iv3_TextLine
#define TextRegion iv3_TextRegion
#define TextAnnotation iv3_TextAnnotation
#define TextLocation iv3_TextLocation
#define Text iv3_Text
#define TextLineAdjuster iv3_TextLineAdjuster

class Requisition;
class Canvas;
class Allocation;
class Extension;
class Event;
class Font;
class Color;
class PolyGlyph;
class Handler;
class Hit;
class String;

class TextRegion 
{
public:
  TextRegion();
  TextRegion(const TextRegion& region);
  virtual ~TextRegion();

  virtual TextRegion& operator=(const TextRegion& region);

  virtual void line1(unsigned line1);
  virtual unsigned line1() const;

  virtual void column1(unsigned column1);
  virtual unsigned column1() const;

  virtual void line2(unsigned line2);
  virtual unsigned line2() const;

  virtual void column2(unsigned column2);
  virtual unsigned column2() const;

  virtual void color(const Color* color);
  virtual const Color* color() const;

protected:
  unsigned line1_;
  unsigned column1_;
  unsigned line2_;
  unsigned column2_;
  Color* color_;
};

class TextAnnotation : public TextRegion, public Action {
public:
  TextAnnotation();
  virtual ~TextAnnotation();

  virtual void execute();
};

declarePtrList(TextAnnotationArray,TextAnnotation)

class TextLocation {
public:
  unsigned line_;
  unsigned column_;
  Color* color_;
  unsigned width_;
};

declarePtrList(TextActionArray,Action)

class Text : public MonoGlyph, public Adjustable 
{
public:
  Text(unsigned rows = 24, unsigned cols = 80, TextBuffer*  buf = nil);
  ~Text();
 
	void editBuffer(TextBuffer*);
		// Installs a new text buffer underneath the text glyph.  The text
		// buffer is used for all textual interface.  The buffer can be
		// treated as an abstract class, with more sophisticated 
		// implementations than the base class TextBuffer;

	TextBuffer* editBuffer() const;
		// Fetches the current edit buffer being used.

public: // --------------- glyph interface --------------------

  void request(Requisition& requisition) const;
  void allocate(Canvas* canvas, const Allocation& allocation, Extension&);
  void draw(Canvas* canvas, const Allocation& allocation) const;
  void undraw();
  void pick(Canvas* c, const Allocation& a, int depth, Hit& h);
  void press(const Event& event);
  void drag(const Event& event);

  virtual Handler* handler();
  virtual void keystroke(const Event& event);


public:	// ---------------- adjustable interface ----------------

  Coord lower(DimensionName dimension) const;
  Coord upper(DimensionName dimension) const;
  Coord length(DimensionName dimension) const;
  Coord cur_lower(DimensionName dimension) const;
  Coord cur_upper(DimensionName dimension) const;
  Coord cur_length(DimensionName dimension) const;
  void scroll_forward(DimensionName dimension);
  void scroll_backward(DimensionName dimension);
  void page_forward(DimensionName dimension);
  void page_backward(DimensionName dimension);
  void scroll_to(DimensionName dimension, Coord position);

  void reset();

  void copy();
  void cut();
  void paste();
  void paste(const char* buffer, unsigned count);

  TextBuffer* buffer() const;

  void region(unsigned line1, unsigned column1, unsigned line2,
    unsigned column2);
  void getRegion(unsigned& line1, unsigned& column1, unsigned& line2,
    unsigned& column2);

  void location(unsigned line, unsigned column);
  void getLocation(unsigned& line, unsigned& column);

  void annotate(TextAnnotation* annotation);
  void deannotate(TextAnnotation* annotation);
  void deannotate();

  void readOnly(bool readOnly);
  bool readOnly() const;

  void font(const Font*);
  const Font* font() const;

  bool dirty();
  bool delete_selection(); //true if nonempty selection

  virtual void modified();

  void insertDirtyAction(Action* action);
  void removeDirtyAction(Action* action);

protected:
  void drawRegion(const TextRegion& region, unsigned i, Coord x, Coord y,
    const String& line) const;
  void drawLocation(const TextLocation& location, unsigned i, Coord x,
    Coord y, const String& line) const;
  void drawLine(unsigned i, Coord x, Coord y, const String& line) const;
  Coord columnCoord(const String& line, unsigned column) const;
  bool snap(const Event& event, unsigned& line, unsigned& column) const;
  Coord width(char ch) const;
  Coord width(const String& line) const;
  Coord width() const;
  Coord height() const;
  void cur_lower(DimensionName dimension, Coord position);
  void cur_upper(DimensionName dimension, Coord position);
  void repair();
  void damage(const TextLocation& location);
  void damage(const TextRegion& region);
  void damage();
  bool damaged(unsigned line) const;
  void eraseLine();
  void backspace();
  void insertChars(const char* buffer, unsigned count);
  void expose(unsigned line, unsigned column);
  void expose(unsigned line1, unsigned column1, unsigned line2,
    unsigned column2);
  void dirty(bool dirty);
  void context_key(char key);
  
  TextBuffer* text_;
  bool readOnly_;
  Handler* handler_;

  TextRegion selection_;
  TextLocation insertion_;

  TextAnnotationArray annotation_;

  TextBuffer* textBuffer_;

  unsigned initialLines_;
  unsigned initialColumns_;

  const Font* font_;
  const Color* textColor_;

  Canvas* canvas_;
  Allocation* allocation_;

  Coord curLowerX_;
  Coord curUpperX_;
  Coord curLowerY_;
  Coord curUpperY_;

  bool dirty_;

  Coord width_;
  bool needWidth_;

  TextActionArray dirtyActions_;

  int ctl_pn_col_; // if >= 0 then in a sequence of up/down keystrokes.
  int escape_; // chars since last escape
};

declareActionCallback(Text);

class TextLine : public Text 
{
public:
  TextLine(unsigned initialColumns = 10);
  TextLine(const String& line, unsigned initialColumns = 10);
  ~TextLine();

  // Glyph
  void request(Requisition& requisition) const;

  // other handler stuff...
  void keystroke(const Event& event);

  String value();
};

class TextLineAdjuster : public Patch, public Observer {
public:
  TextLineAdjuster(TextLine* textLine);
  ~TextLineAdjuster();

  void update(Observable*);

protected:
  bool needButtons();
  void addButtons();
  void removeButtons();

  TextLine* adjustable_;
  bool have_:1;
  bool updating_:1;
  PolyGlyph* box_;
};

inline void Text::editBuffer(TextBuffer* b)
	{ text_ = b; }
inline TextBuffer* Text::editBuffer() const
	{ return text_; }

#endif
