#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
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
// Reworked to use InterViews classes instead of the Redwood utility 
// classes.
//
// 1.3
// 1998/08/16 19:33:28
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


#include <InterViews/action.h>
#include <InterViews/box.h>
#include <InterViews/canvas.h>
#include <InterViews/color.h>
#include <InterViews/event.h>
#include <InterViews/layout.h>
#include <InterViews/font.h>
#include <InterViews/handler.h>
#include <InterViews/geometry.h>
#include <InterViews/session.h>
#include <InterViews/hit.h>
#include <InterViews/style.h>
#include <InterViews/window.h>

#include <IV-look/stepper.h>
#include <IV-look/kit.h>

#include <OS/math.h>
#include <OS/string.h>
#include <OS/memory.h>

#include <InterViews/iv3text.h>

#include <string.h>

implementActionCallback(Text);
implementPtrList(TextAnnotationArray,TextAnnotation)
implementPtrList(TextActionArray,Action)

/////////////////////
// Text Annotation //
/////////////////////

TextAnnotation::TextAnnotation() 
{
}

TextAnnotation::~TextAnnotation() 
{
}

void TextAnnotation::execute() 
{
}

/////////////////
// Text Region //
/////////////////

TextRegion::TextRegion() 
{
  line1_ = 0;
  column1_ = 0;
  line2_ = 0;
  column2_ = 0;

  WidgetKit* kit = WidgetKit::instance();
  Style* style = kit->style();
  String gui("monochrome");
  if (gui == kit->gui()) {
    color_ = new Color(*kit->foreground(), 0.25);
  } else {
    color_ = new Color(.7, .8, 1, 1);
  }
  Resource::ref(color_);
}

TextRegion::~TextRegion() 
{
  Resource::unref(color_);
  color_ = 0;
}

void TextRegion::line1(unsigned line1) 
{
  line1_ = line1;
}

unsigned TextRegion::line1() const 
{
  return line1_;
}

void TextRegion::column1(unsigned column1) 
{
  column1_ = column1;
}

unsigned TextRegion::column1() const 
{
  return column1_;
}

void TextRegion::line2(unsigned line2) 
{
  line2_ = line2;
}

unsigned TextRegion::line2() const 
{
  return line2_;
}

void TextRegion::column2(unsigned column2) 
{
  column2_ = column2;
}

unsigned TextRegion::column2() const 
{
  return column2_;
}

void TextRegion::color(const Color* color) 
{
  Resource::unref(color_);
  color_ = (Color*)color;
  Resource::ref(color_);
}

const Color* TextRegion::color() const 
{
  return color_;
}

TextRegion::TextRegion(const TextRegion& region) 
{
  line1_ = region.line1_;
  column1_ = region.column1_;
  line2_ = region.line2_;
  column2_ = region.column2_;
  Resource::ref(region.color_);
  color_ = region.color_;
}

TextRegion& TextRegion::operator=(const TextRegion& region) 
{
  line1_ = region.line1_;
  column1_ = region.column1_;
  line2_ = region.line2_;
  column2_ = region.column2_;
  Resource::ref(region.color_);
  Resource::unref(color_);
  color_ = region.color_;
  return *this;
}

//////////////////
// Text Handler //
//////////////////

class TextHandler : public Handler 
{
public:
  TextHandler(Text* target);
  virtual bool event(Event& event);
protected:
  Text* target_;
  bool pressed_;
};

TextHandler::TextHandler(Text* target) 
{
  target_ = target;
  pressed_ = false;
}

bool TextHandler::event(Event& e) 
{
  bool handled = true;
  switch (e.type()) {
    case Event::down:
      if (!pressed_) {
        pressed_ = true;
        target_->press(e);
        e.grab(this);
      }
      break;
    case Event::motion:
      if (pressed_) {
        target_->drag(e);
      }
      break;
    case Event::up:
      if (pressed_) {
        pressed_ = false;
        e.ungrab(this);
      }
      break;
    case Event::key:
      target_->keystroke(e);
      break;
    default:
      /* ignore */
      break;
  }
  return handled;
}

//////////
// Text //
//////////

Text::Text(unsigned initialLines, unsigned initialColumns, TextBuffer* t)
{
	handler_ = new TextHandler(this);
	Resource::ref(handler_);
	dirty_ = false;
	text_ = t;
	insertion_.line_ = 0;
	insertion_.column_ = 0;
	WidgetKit* kit = WidgetKit::instance();
	Style* s = kit->style();
	String gui("monochrome");

	if (gui == kit->gui()) 
		insertion_.color_ = new Color(*kit->foreground());
	else 
		insertion_.color_ = new Color(1, .5, .5, 1);
	Resource::ref(insertion_.color_);
	insertion_.width_ = 2;

	initialLines_ = Math::max(initialLines, 1u);
	initialColumns_ = Math::max(initialColumns, 1u);

	font_ = kit->font();
	Resource::ref(font_);
	textColor_ = new Color(*kit->foreground());
	Resource::ref(textColor_);

	readOnly_ = false;

	canvas_ = 0;
	allocation_ = 0;

	curLowerX_ = 0;
	curUpperX_ = 0;
	curLowerY_ = 0;
	curUpperY_ = 0;

	textBuffer_ = 0;

	needWidth_ = false;
	width_ = 0;

	ctl_pn_col_ = -1;
	escape_ = 10;
}

Text::~Text()
{
	Resource::unref(handler_);
	Resource::unref(insertion_.color_);
	Resource::unref(font_);
	Resource::unref(textColor_);
	delete text_;
}

Handler* Text::handler() {
  return handler_;
}

void Text::pick(Canvas*, const Allocation&, int depth, Hit& h) {
  const Event* e = h.event();
  EventButton t = (e == nil) ? Event::none : e->pointer_button();
  EventType t1 = (e == nil) ? Event::undefined : e->type();
  if (t == Event::left || t == Event::right ||
      t == Event::middle || t1 == Event::key) {
    Coord x = e->pointer_x();
    Coord y = e->pointer_y();
    Allocation* a = allocation_;
    if (x >= a->left() && x <= a->right() && y >= a->bottom() && y <= a->top()) {
      h.target(depth, this, 0, handler());
    }
  }
}

inline Coord Text::width(char ch) const {
  return ch == '\t' ? font_->width(' ') * 8 : font_->width(ch);
}

Coord Text::width(const String& line) const 
{
	Coord lineTotal = 0;
	for (unsigned j = 0; j < line.length(); ++j) 
	{
		lineTotal += width(line[j]);
	}
	return lineTotal;
}

Coord Text::width() const 
{
	if (needWidth_) 
	{
		Coord total = 0;
		for (unsigned i = 0; i < text_->Height(); ++i) 
		{
			int index0 = text_->LineIndex(i);
			int index1 = text_->BeginningOfNextLine(index0);
			const String line(text_->Text(index0), index1 - index0);
			total = Math::max(total, width(line));
		}
		((Text*) this)->width_ = total;
		((Text*) this)->needWidth_ = false;
	}
	return width_;
}

void Text::readOnly(bool readOnly) 
{
	if (readOnly_ != readOnly) 
	{
		readOnly_ = readOnly;
		damage(insertion_);
	}
}

bool Text::readOnly() const 
{
	return readOnly_;
}

void Text::font(const Font* f) 
{
	Resource::ref(f);
	Resource::unref(font_);
	font_ = f;

	needWidth_ = true;
	notify_all();
	damage();
}

const Font* Text::font() const 
{
	return font_;
}


void Text::request(Requisition& requisition) const 
{
	FontBoundingBox fbb;
	font_->font_bbox(fbb);
	Requirement rx(width(' ') * initialColumns_, fil,
		width(' ') * (initialColumns_ - 1), 0);
	Requirement ry((fbb.ascent() + fbb.descent()) * initialLines_, fil,
		(fbb.ascent() + fbb.descent()) * (initialLines_ - 1), 0);
	requisition.require(Dimension_X, rx);
	requisition.require(Dimension_Y, ry);
}

void Text::allocate(Canvas* c, const Allocation& a, Extension& extension) 
{
	canvas_ = c;
	if (! allocation_) 
	{
		allocation_ = new Allocation(a);
		curLowerX_ = 0;
		curLowerY_ = 0;
	} 
	else 
	{
		*allocation_ = a;
	}
	curUpperX_ = curLowerX_ + allocation_->allotment(Dimension_X).span();
	curUpperY_ = curLowerY_ + allocation_->allotment(Dimension_Y).span();

	extension.merge(c, a);
	notify_all();
}

void Text::undraw() {
	canvas_ = nil;
}

Coord Text::columnCoord(const String& line, unsigned column) const 
{
	Coord x;

	x = allocation_->left() - curLowerX_;
	if (line.length()) 
	{
		unsigned to = Math::min(column, unsigned(line.length()));
		String part = line.substr(0, to);
		for (unsigned i = 0; i < to; ++i) 
		{
			x += width(line[i]);
		}
		if (column > line.length()) 
		{
			x += font_->width(' ') * (column - line.length());
		}
	} 
	else 
	{
		x += font_->width(' ') * column;
	}

	return x;
}

void Text::drawRegion(const TextRegion& region, unsigned i,
  Coord x, Coord y, const String& line) const 
{
	unsigned line1 = region.line1();
	unsigned line2 = region.line2();
	unsigned column1 = region.column1();
	unsigned column2 = region.column2();
	FontBoundingBox fbb;
	font_->font_bbox(fbb);

	if ((line1 == i) && (line1 == line2) && (column1 < column2)) 
	{
		canvas_->fill_rect(columnCoord(line, column1), y - fbb.descent(),
			columnCoord(line, column2), y + fbb.ascent(), region.color());
	}
	if ((line1 == i) && (line1 < line2)) 
	{
		canvas_->fill_rect(columnCoord(line, column1), y - fbb.descent(),
			allocation_->right(), y + fbb.ascent(), region.color());
	}
	if ((line1 < i) && (i < line2)) 
	{
		canvas_->fill_rect(x, y - fbb.descent(), allocation_->right(), y +
			fbb.ascent(), region.color());
	}
	if ((line2 == i) && (line1 < line2)) 
	{
		canvas_->fill_rect(0, y - fbb.descent(), columnCoord(line, column2),
			y + fbb.ascent(), region.color());
	}
}

void Text::drawLocation(const TextLocation& location, unsigned i,
  Coord /* x */, Coord y, const String& line) const 
{
	FontBoundingBox fbb;
	font_->font_bbox(fbb);
	if (location.line_ == i) 
	{
		Coord lb = columnCoord(line, location.column_);
		canvas_->fill_rect(lb, y - fbb.descent(), lb + location.width_,
			y + fbb.ascent(), location.color_);
	}
}

void Text::drawLine(unsigned /* i */, Coord x, Coord y,
  const String& line) const 
{
	for (unsigned j = 0; j < line.length(); ++j) 
	{
		char ch = line[j];
		Coord w = width(ch);
		if (ch != '\t') 
		{
			canvas_->character(font_, ch, w, textColor_, x, y);
		}
		x += w;
	}
}

bool Text::damaged(unsigned line) const 
{
	FontBoundingBox fbb;
	font_->font_bbox(fbb);
	Coord base = allocation_->top() + curLowerY_;
	Coord lineHeight = fbb.ascent() + fbb.descent();
	Coord top = base - lineHeight * line;
	Coord bottom = base - lineHeight * (line + 1);
	return canvas_->damaged(allocation_->left(), Math::max(bottom,
		allocation_->bottom()), allocation_->right(), Math::min(top,
		allocation_->top()));
}

void Text::draw(Canvas*, const Allocation&) const 
{
	canvas_->push_clipping();
	canvas_->clip_rect(allocation_->left(), allocation_->bottom(),
		allocation_->right(), allocation_->top());

	FontBoundingBox fbb;
	font_->font_bbox(fbb);
	Coord r = curLowerY_ / (fbb.ascent() + fbb.descent());
	unsigned i = unsigned(r);
	Coord y = allocation_->top() + (r - i) * (fbb.ascent() + fbb.descent());
	unsigned max = Math::max(Math::max(selection_.line2(), insertion_.line_),
		(unsigned) text_->Height() ? (unsigned) text_->Height() - 1 : 0);
	for (; i <= max; ++i) 
	{
		y -= fbb.ascent();

		if (damaged(i)) 
		{
			Coord x = allocation_->left() - curLowerX_;
			if (i < text_->Height()) 
			{
				const String line = text_->getNth(i);
				drawRegion(selection_, i, x, y, line);
				if (! readOnly_) 
				{
					drawLocation(insertion_, i, x, y, line);
				}
				for (GlyphIndex j = 0; j < annotation_.count(); ++j) 
				{
					drawRegion(*annotation_.item(j), i, x, y, line);
				}
				drawLine(i, x, y, line);
			} 
			else 
			{
				String line;
				drawRegion(selection_, i, x, y, line);
				if (! readOnly_) 
				{
					drawLocation(insertion_, i, x, y, line);
				}
				for (GlyphIndex j = 0; j < annotation_.count(); ++j) 
				{
					drawRegion(*annotation_.item(j), i, x, y, line);
				}
				drawLine(i, x, y, line);
			}
		}
  
		y -= fbb.descent();
		if (y < (allocation_->bottom() - fbb.ascent())) 
			break;
	}
	canvas_->pop_clipping();
}
 
bool Text::snap(
	const Event& event, 
	unsigned& line, 
	unsigned& column) const 
{
	unsigned originalLine = line;
	unsigned originalColumn = column;

	Coord x = event.pointer_x() - allocation_->left() + curLowerX_;
	Coord y = allocation_->top() + curLowerY_ - event.pointer_y();

	FontBoundingBox fbb;
	font_->font_bbox(fbb);
	line = Math::max(int(y / (fbb.ascent() + fbb.descent())), 0);
	if (line < text_->Height()) 
	{
		const String& string = text_->getNth(line);
		unsigned i;
		for (i = 0; i < string.length(); ++i) 
		{
			x -= width(string[i]) / 2.0;
			if (x < 0) 
				break;
			x -= width(string[i]) / 2.0;
		}
		column = i;
		if (i > 0 && string[i-1] == '\n') {
			--column;
		}
  //		if (i >= string.length())
  //			column += (unsigned) ((x + width(' ') / 2) / width(' '));
	} 
	else if (text_->Height() > 0) { 
		line = text_->Height()-1;
		column = text_->getNth(line).length(); 
		//column = (unsigned) ((x + width(' ') / 2) / width(' '));
	}else{
		line = 0;
		column = 0;
	}
	column = Math::max(column, unsigned(0));

	return (line != originalLine) || (column != originalColumn);
}

void Text::eraseLine() 
{
	if (insertion_.line_ < text_->Height()) 
	{
		int oldWidth = text_->Width();
		int index0 = text_->LineIndex(insertion_.line_);
		int index1 = text_->BeginningOfNextLine(index0);

		// ---- delete the text ----
		text_->Delete(index0, index1 - index0);

		// --- check for width change ----
		if (text_->Width() != oldWidth)
		{
			needWidth_ = true;
			notify_all();
		}
	}
	insertion_.column_ = 0;
	repair();
	damage(insertion_);
	repair();
}

void Text::backspace() 
{
	if ((insertion_.column_ <= 0) && (insertion_.line_ > 0)) 
	{
		--insertion_.line_;
		if (insertion_.line_ >= text_->Height()) 
		{
			insertion_.column_ = 0;
		} 
		else 
		{
			// --- update insertion point ---
			int index = text_->LineIndex(insertion_.line_);
			//insertion_.column_ = text_->LineOffset(index);

			// ---- delete the character ----
			int delIndex = text_->EndOfLine(index);
			insertion_.column_ = delIndex - index;
			text_->Delete(delIndex, 1);
		}
		needWidth_ = true;
		notify_all();
		damage(); // !!! could only damage from insertion to end of window
	} 
	else if (insertion_.column_ > 0) 
	{
		int oldWidth = text_->Width();
		int index = text_->LineIndex(insertion_.line_);

		// ---- delete the text ---
		text_->Delete(index + insertion_.column_ - 1, 1);
		--insertion_.column_;

		if (text_->Width() != oldWidth) 
		{
			needWidth_ = true;
			notify_all();
		}
		repair();
		damage(insertion_);
		repair();
	}
}

void Text::insertChars(const char* txt, unsigned count) 
{
	TextBuffer text(txt, count, count);

	// ---- insert the text info the buffer ----
	text_->Insert(text_->LineIndex(insertion_.line_) + insertion_.column_,
		txt, count);
	dirty(true);

	if (text.Height() > 1) 
	{
		TextRegion area;
		area.line1(insertion_.line_);
		area.column1(0);
		area.line2(insertion_.line_ + text.Height() - 1);
		area.column2(0);
		insertion_.line_ += text.Height() - 1;
		insertion_.column_ = 0;
		damage(); // !!! could only damage from insertion to end of window
		for (unsigned i = 0; i < text.Height(); ++i) 
		{
			width_ = Math::max(width_, width(text.getNth(i)));
		}
		notify_all();
	} 
	else 
	{
		TextLocation old = insertion_;
		insertion_.column_ += count;
		Coord newWidth = width(text_->getNth(insertion_.line_));
		if (newWidth >= width_) 
		{
			width_ = Math::max(width_, newWidth);
			notify_all();
		}
		repair();
		damage(old);
		repair();
	}
}

void Text::context_key(char key) {
	// attempt to preserve column as much as possible for
	// a ctrl-p ctrl-n sequence
	if (key == 14 || key == 16) {
		if (ctl_pn_col_ == -1) {
			ctl_pn_col_ = insertion_.column_;
		}
	}else{
		ctl_pn_col_ = -1;
	}
	// how many chars since last escape
	if (key == 033) {
		escape_ = 0;
	}else{
		++escape_;
	}
}

void Text::keystroke(const Event& event) {
  if (readOnly_) {
    return;
  }
  char buffer[8]; // needs to be dynamically adjusted
  int count = event.mapkey(buffer, 8);
  if (count <= 0) {
    return;
  }

  // return causes a newline
  if (buffer[0] == '\r') {
    buffer[0] = '\n';
  }
	context_key(buffer[0]);

	 if (buffer[0] == 2) // control-b
    {
        if (insertion_.column_ > 0)
        {
            --insertion_.column_;
            damage(insertion_);
            repair();
        }else if (insertion_.line_ > 0) {
        	damage(insertion_);
        	--insertion_.line_;
        	// now just like a control-e
        	int index = text_->LineIndex(insertion_.line_);
        	int end = text_->EndOfLine(index);
        	insertion_.column_ = end - index;
       	 	damage(insertion_);
       	 	repair();
       	}
        return;
    }
	 else if (buffer[0] == 1) // control-a
    {
        insertion_.column_ = 0;
        damage(insertion_);
        repair();
        return;
    }
	 else if (buffer[0] == 6) // control-f
    {
        int index = text_->LineIndex(insertion_.line_);
        int end = text_->EndOfLine(index);
        if (insertion_.column_ < end - index)
        {
            ++insertion_.column_;
            damage(insertion_);
            repair();
        }else if (insertion_.line_ < text_->Height()-1) {
        	damage(insertion_);
        	++insertion_.line_;
        	// now just like control-a
        	insertion_.column_ = 0;
        	damage(insertion_);
        	repair();
        }
        return;
    }
    else if (buffer[0] == 5) // control-e
    {
        int index = text_->LineIndex(insertion_.line_);
        int end = text_->EndOfLine(index);
        insertion_.column_ = end - index;
        damage(insertion_);
        repair();
        return;
    }
    else if (buffer[0] == 16) { // control-p
    	if (insertion_.line_ > 0) {
    		damage(insertion_);
    		--insertion_.line_;
		insertion_.column_ = ctl_pn_col_;
    		int index = text_->LineIndex(insertion_.line_);
        	int col = text_->EndOfLine(index) - index;
        	if (col < insertion_.column_){
        		insertion_.column_ = col;
        	}
        	damage(insertion_);
        	repair();
        }
		return;
    }else if (buffer[0] == 14) { // control-n
    	if (insertion_.line_ < text_->Height()-1) {
    		damage(insertion_);
    		++insertion_.line_;
		insertion_.column_ = ctl_pn_col_;
    		int index = text_->LineIndex(insertion_.line_);
        	int col = text_->EndOfLine(index) - index;
        	if (col < insertion_.column_){
        		insertion_.column_ = col;
        	}
        	damage(insertion_);
        	repair();
        }
		return;
    }else if (escape_ == 1) { // escaped chars
	if (buffer[0] == '>') { // to end of buffer
		damage(insertion_);
		if (text_->Height() >0) {
			insertion_.line_ = text_->Height()-1;
		} else {
			insertion_.line_ = 0;
		}
		int index = text_->LineIndex(text_->EndOfText());
		insertion_.column_ = text_->EndOfText()-index;
		damage(insertion_);
		repair();
	}else if (buffer[0] == '<') {// to beginning of buffer
		damage(insertion_);
		insertion_.line_ = 0;
		insertion_.column_ = 0;
		damage(insertion_);
		repair();
	}
	return;
    }else if (buffer[0] == 033) { // ignore the escape itself
	return;
    }
    else if (buffer[0] == 4) // ctrl-d
    {
      if (!delete_selection()) {
    	// like a ctrl-f then backspace
        int index = text_->LineIndex(insertion_.line_);
        int end = text_->EndOfLine(index);
        if (insertion_.column_ < end - index)
        {
            ++insertion_.column_;
            backspace();
        }else if (insertion_.line_ < text_->Height()-1) {
        	++insertion_.line_;
        	// now just like control-a
        	insertion_.column_ = 0;
        	backspace();
        }
      }
    }

  else if (buffer[0] == 21) {
    // control u
    eraseLine();
  }
  else if ((buffer[0] == '\b') || (buffer[0] == 127)) {
    // backspace and delete
  	if (!delete_selection()) {
	 	backspace();
	}
  }
  else {
  	delete_selection();
	 insertChars(buffer, count);
  }

  dirty(true);
}

bool Text::delete_selection() {
  	int i1 = text_->LineIndex(selection_.line1()) + selection_.column1();
  	int i2 = text_->LineIndex(selection_.line2()) + selection_.column2();
	if (i2 > i1) {
		selection_.column2(selection_.column1());
		selection_.line2(selection_.line1());
		text_->Delete(i1, i2 - i1);
		damage();
	}else if (i1 > i2) {
		selection_.column1(selection_.column2());
		selection_.line1(selection_.line2());
		text_->Delete(i2, i1 - i2);
		damage();
	}else{
		return false;
	}
	return true;
}

void Text::repair() {
  if (canvas_ && canvas_->window()) {
    canvas_->window()->repair();
  }
}

void Text::damage(const TextLocation& location) {
  if (! canvas_) {
    return;
  }
//mlh to get horizontal scrolling. Not well understood yet.
  expose(0, location.column_);
  
  FontBoundingBox fbb;
  font_->font_bbox(fbb);
  Coord base = allocation_->top() + curLowerY_;
  Coord lineHeight = fbb.ascent() + fbb.descent();
  Coord top = Math::min(base - lineHeight * location.line_, allocation_->top());
  Coord bottom = Math::max(base - lineHeight * (location.line_ + 1),
    allocation_->bottom());
  if ((allocation_->left() <= allocation_->right()) && (bottom <= top)) {
    canvas_->damage(allocation_->left(), bottom, allocation_->right(), top);
  }
}

void Text::damage(const TextRegion& region) {
  if (! canvas_) {
    return;
  }

  FontBoundingBox fbb;
  font_->font_bbox(fbb);
  Coord base = allocation_->top() + curLowerY_;
  Coord lineHeight = fbb.ascent() + fbb.descent();
  Coord top = Math::min(base - lineHeight * region.line1(), allocation_->top());
  Coord bottom = Math::max(base - lineHeight * (region.line2() + 1),
    allocation_->bottom());
  if ((allocation_->left() <= allocation_->right()) && (bottom <= top)) {
    canvas_->damage(allocation_->left(), bottom, allocation_->right(), top);
  }
}

void Text::damage() {
  if (! canvas_) {
    return;
  }

  canvas_->damage(allocation_->left(), allocation_->bottom(),
    allocation_->right(), allocation_->top());
}

void Text::press(const Event& event) {
//  if (event.pointer_button() == Event::left) {
  context_key(0);
  if (event.pointer_button() != Event::middle) {
	 TextRegion old = selection_;
    unsigned line1 = selection_.line1();
    unsigned column1 = selection_.column1();
    snap(event, line1, column1);
    selection_.line1(line1);
    selection_.column1(column1);
    selection_.line2(line1);
    selection_.column2(column1);
    repair();
    damage(old);
    repair();
//  } else if (event.pointer_button() == Event::right) {
	 TextLocation oldi = insertion_;
    snap(event, insertion_.line_, insertion_.column_);
    repair();
    damage(oldi);
    repair();
    damage(insertion_);
    repair();
  } else if (event.pointer_button() == Event::middle) {
    unsigned line = 0;
    unsigned column = 0;
    snap(event, line, column);
    for (int i = 0; i < annotation_.count(); ++i) {
      TextAnnotation& a = *annotation_.item(i);
      if (a.line1() <= line && line <= a.line2()) {
        if (!((a.line1() == line && a.column1() > column) || 
              (a.line2() == line && a.column2() < column))) {
          a.execute();
          break;
        }
      }
    }
  }
}

void Text::drag(const Event& event) {
  if (event.left_is_down()) {
    unsigned line = selection_.line2();
    unsigned column = selection_.column2();
    if (snap(event, line, column) && ((line > selection_.line1()) ||
      ((line == selection_.line1()) && (column >= selection_.column1())))) {
      TextRegion area;
      if (selection_.line2() < line) {
        area.line1(selection_.line2());
        area.column1(selection_.column2());
        area.line2(line);
        area.column2(column);
      } else {
        area.line1(line);
        area.column1(column);
        area.line2(selection_.line2());
        area.column2(selection_.column2());
      }
      selection_.line2(line);
      selection_.column2(column);
      repair();
      damage(area);
      repair();
    }
  } else if (event.right_is_down()) {
    TextLocation old = insertion_;
    if (snap(event, insertion_.line_, insertion_.column_)) {
      repair();
      damage(old);
      repair();
      damage(insertion_);
      repair();
    }
  }
}

Coord Text::height() const {
  FontBoundingBox fbb;
  font_->font_bbox(fbb);
  return text_->Height() * (fbb.ascent() + fbb.descent());
}

Coord Text::lower(DimensionName) const {
  return 0;
}

Coord Text::upper(DimensionName dimension) const {
  return (dimension == Dimension_X ? width() : height());
}

Coord Text::length(DimensionName dimension) const {
  return upper(dimension) - lower(dimension);
}

Coord Text::cur_lower(DimensionName dimension) const {
//  return (dimension == Dimension_X ? curLowerX_ : curLowerY_);
  return (dimension == Dimension_X ? curLowerX_ : height() - curUpperY_);
}

Coord Text::cur_upper(DimensionName dimension) const {
//  return (dimension == Dimension_X ? curUpperX_ : curUpperY_);
  return (dimension == Dimension_X ? curUpperX_ : height() - curLowerY_);
}

Coord Text::cur_length(DimensionName dimension) const {
  return cur_upper(dimension) - cur_lower(dimension);
}

void Text::scroll_forward(DimensionName dimension) {
  FontBoundingBox fbb;
  font_->font_bbox(fbb);
  if (dimension == Dimension_X) {
    scroll_to(dimension, cur_lower(Dimension_X) + width(' '));
  } else {
    scroll_to(dimension, cur_lower(Dimension_Y) +
      (fbb.ascent() + fbb.descent()));
  }
}

void Text::scroll_backward(DimensionName dimension) {
  FontBoundingBox fbb;
  font_->font_bbox(fbb);
  if (dimension == Dimension_X) {
    scroll_to(dimension, cur_lower(Dimension_X) - width(' '));
  } else {
    scroll_to(dimension, cur_lower(Dimension_Y) -
      (fbb.ascent() + fbb.descent()));
  }
}

void Text::page_forward(DimensionName dimension) {
  scroll_to(dimension, cur_lower(dimension) + cur_length(dimension));
}

void Text::page_backward(DimensionName dimension) {
  scroll_to(dimension, cur_lower(dimension) - cur_length(dimension));
}

void Text::cur_lower(DimensionName dimension, Coord position) {
  if (dimension == Dimension_X) {
    curLowerX_ = position;
    curUpperX_ = position + allocation_->allotment(Dimension_X).span();
  } else {
//    curLowerY_ = position;
//    curUpperY_ = position + allocation_->allotment(Dimension_Y).span();
    curLowerY_ = height() - position - allocation_->allotment(Dimension_Y).span();
    curUpperY_ = height() - position;
  }
}

void Text::scroll_to(DimensionName dimension, Coord position) {
// let the user scroll to the right (or down) past the current size.
/*
  if (position > (upper(dimension) - cur_length(dimension))) {
    position = upper(dimension) - cur_length(dimension);
  }
*/
//  if (position < lower(dimension)) {
//    position = lower(dimension);
//  }

  if (dimension == Dimension_X) {
    if (position < lower(dimension)) {
      position = lower(dimension);
    }
  } else {
    if (position > (upper(dimension) - cur_length(dimension))) {
      position = upper(dimension) - cur_length(dimension);
    }
  }

  if (position == cur_lower(dimension)) {
    return;
  }

  cur_lower(dimension, position);
  notify(dimension);
  damage();
}

void Text::reset() 
{
	text_->Delete(0, text_->Length());

	insertion_.line_ = 0;
	insertion_.column_ = 0;

	selection_.line1(0);
	selection_.column1(0);
	selection_.line2(0);
	selection_.column2(0);

	if (textBuffer_)
		delete textBuffer_;
	textBuffer_ = 0;

	deannotate();

	needWidth_ = false;
	width_ = 0;

	dirty(false);
	notify_all();
}

void Text::modified() 
{
}

bool Text::dirty() {
  return dirty_;
}

void Text::dirty(bool dirty) {
  if (dirty_ != dirty) {
    dirty_ = dirty;
    modified();
    for (int i = 0; i < dirtyActions_.count(); ++i) 
	{
      dirtyActions_.item(i)->execute();
    }
  }
}

void Text::copy() 
{
	// ---- check for reasonable selection ----
	unsigned line1 = selection_.line1();
	unsigned line2 = selection_.line2();
	unsigned column1 = selection_.column1();
	unsigned column2 = selection_.column2();
	if ((line1 > line2) || ((line1 == line2) && (column1 >= column2))) 
	{
		return;
	}

	// --- create a text buffer that copies the selection region ---
	if (textBuffer_)
	{
		char* buff = (char*) textBuffer_->Text();
		delete textBuffer_;
//		delete buff;                     // hines: why delete twice
	}
	int index0 = text_->LineIndex(line1) + column1;
	int index1 = text_->LineIndex(line2) + column2;
	int len = index1 - index0 + 1;
	char* txt = new char[len + 1];
	Memory::copy(text_->Text(index0, index1), txt, len);
	textBuffer_ = new TextBuffer(txt, len, len);
}

void Text::cut() 
{
	// ---- check for reasonable selection ----
	unsigned line1 = selection_.line1();
	unsigned line2 = selection_.line2();
	unsigned column1 = selection_.column1();
	unsigned column2 = selection_.column2();
	if ((line1 > line2) || ((line1 == line2) && (column1 >= column2))) 
	{
		return;
	}

	// ---- copy to cut/copy/paste buffer ----
	copy();

	// ---- remove from text buffer ---
	int index0 = text_->LineIndex(line1) + column1;
	int index1 = text_->LineIndex(line2) + column2;
	int len = index1 - index0 + 1;
	text_->Delete(index0, len);

	// ---- update view ----
	insertion_.line_ = selection_.line1();
	insertion_.column_ = selection_.column1();
	selection_.line2(selection_.line1());
	selection_.column2(selection_.column1());
	needWidth_ = true;
	dirty(true);
	notify_all();
	damage();
}

void Text::paste() 
{
	// --- see if there is anything to do ----
	if (! textBuffer_)
		return;

	insertChars(textBuffer_->Text(), textBuffer_->Length());
}

void Text::paste(const char* buffer, unsigned count) 
{
	insertChars(buffer, count);
}

TextBuffer* Text::buffer() const 
{
	return textBuffer_;
}

void Text::expose(unsigned line1, unsigned column1, unsigned /*line2*/,
  unsigned /*column2*/) {
  expose(line1, column1);
}

void Text::region(unsigned line1, unsigned column1,
	unsigned line2, unsigned column2) 
{
	TextRegion old = selection_;
	selection_.line1(line1);
	selection_.column1(column1);
	selection_.line2(line2);
	selection_.column2(column2);
	damage(old);
	damage(selection_);
	expose(line1, column1, line2, column2);
}

void Text::getRegion(unsigned& line1, unsigned& column1, unsigned& line2,
  unsigned& column2) 
{
	line1 = selection_.line1();
	column1 = selection_.column1();
	line2 = selection_.line2();
	column2 = selection_.column2();
}

void Text::expose(unsigned line, unsigned column) 
{
	if (! canvas_)
		return;

	String string;
	if (line < text_->Height()) 
	{
		string = text_->getNth(line);
	}
	FontBoundingBox fbb;
	font_->font_bbox(fbb);
	Coord x = columnCoord(string, column) - allocation_->left() + curLowerX_;
	bool invisibleX = (x < curLowerX_) || ((x + width(' ')) > curUpperX_);
	Coord y = line * (fbb.ascent() + fbb.descent());

	bool invisibleY = (y < curLowerY_) || ((y + fbb.ascent() +
		fbb.descent()) > curUpperY_);
	if (invisibleX) 
	{
		scroll_to(Dimension_X, x - (curUpperX_ - curLowerX_) / 2);
	}
	if (invisibleY) 
	{
		scroll_to(Dimension_Y, height() - y - (curUpperY_ - curLowerY_) / 2);
	}
}

void Text::location(unsigned line, unsigned column) 
{
	TextLocation old = insertion_;
	insertion_.line_ = line;
	insertion_.column_ = column;
	damage(old);
	damage(insertion_);
	expose(line, column);
}

void Text::getLocation(unsigned& line, unsigned& column) 
{
	line = insertion_.line_;
	column = insertion_.column_;
}

void Text::annotate(TextAnnotation* annotation) 
{
	annotation_.append(annotation);
	damage(*annotation);
}

void Text::deannotate(TextAnnotation* annotation) 
{
	GlyphIndex len = annotation_.count();
	for (GlyphIndex i = 0; i < len; i++)
	{
		if (annotation_.item(i) == annotation)
		{
			annotation_.remove(i);
			break;
		}
	}
	damage(*annotation);
}

void Text::deannotate() 
{
	while (annotation_.count()) 
	{
		annotation_.remove(0);
	}
	damage();
}

void Text::insertDirtyAction(Action* action) 
{
	dirtyActions_.append(action);
}

void Text::removeDirtyAction(Action* action) 
{
	GlyphIndex len = dirtyActions_.count();
	for (GlyphIndex i = 0; i < len; i++)
	{
		if (dirtyActions_.item(i) == action)
		{
			dirtyActions_.remove(i);
			break;
		}
	}
}

///////////////
// Text Line //
///////////////

TextLine::TextLine(unsigned initialColumns) 
	: Text(1u, initialColumns) 
{
	int len = 2 * initialColumns;
	char* buff = new char[len + 1];
	TextBuffer* b = new TextBuffer(buff, 0, len);
	editBuffer(b);
}

TextLine::TextLine(const String& line, unsigned initialColumns) :
  Text(1u, initialColumns) 
{
	int buflen = 2 * initialColumns;
	char* buff = new char[buflen + 1];
	int len = Math::min(line.length(), buflen);
	Memory::copy(line.string(), buff, len);
	TextBuffer* b = new TextBuffer(buff, len, buflen);
	editBuffer(b);
	delete [] buff;
}

TextLine::~TextLine() 
{
#if 0
// seems to me (hines) that buff is deleted when base class
// Text is deleted
	char* buff = (char*) editBuffer()->Text();
	delete buff;
#endif
}

void TextLine::request(Requisition& requisition) const 
{
	FontBoundingBox fbb;
	font_->font_bbox(fbb);
	Text::request(requisition);
	Requirement ry(fbb.ascent() + fbb.descent(), 0, 0, 0);
	requisition.require(Dimension_Y, ry);
}

void TextLine::keystroke(const Event& event) {
  // ignore carrage returns
  char buffer[2];
  int count = event.mapkey(buffer, 2);
  if (buffer[0] == '\r') {
    return;
  }

  Text::keystroke(event);
}

String TextLine::value() 
{
	if (! text_->Height())
		return "";
	return text_->getNth(0);
}

////////////////////////
// Text Line Adjuster //
////////////////////////

TextLineAdjuster::TextLineAdjuster(TextLine* textLine) 
	: Patch(0), Observer() 
{
  LayoutKit* lkit = LayoutKit::instance();
  adjustable_ = textLine;
  adjustable_->attach(Dimension_X, this);
  adjustable_->attach(Dimension_Y, this);
  updating_ = false;
  have_ = false;

  box_ = lkit->hbox(textLine);
  body(box_);
}

TextLineAdjuster::~TextLineAdjuster() 
{
}

void TextLineAdjuster::update(Observable*) 
{
	if (updating_)
		return;

	updating_ = true;

	bool need = needButtons();
	if (need != have_) 
	{
		if (need)
			addButtons();
		else
			removeButtons();
		reallocate();
		redraw();
		have_ = ! have_;
	}
	updating_ = false;
}

bool TextLineAdjuster::needButtons() 
{
	return allocation().allotment(Dimension_X).span() <
		adjustable_->length(Dimension_X);
}

void TextLineAdjuster::addButtons() 
{
	WidgetKit* kit = WidgetKit::instance();
	LayoutKit* lkit = LayoutKit::instance();
	Style* style = kit->style();

	Stepper* left = kit->left_mover(adjustable_);
	Stepper* right = kit->right_mover(adjustable_);

	Coord mover_size = 15.0;
	style->find_attribute("mover_size", mover_size);

	box_->prepend(lkit->h_fixed_span(left, mover_size));
	box_->append(lkit->h_fixed_span(right, mover_size));
}

void TextLineAdjuster::removeButtons() 
{
	box_->remove(2);
	box_->remove(0);
}
