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
 * Printer - draw for PostScript printer
 */
 
#ifdef MAC
	#define WIN32
#endif
#include <ivstream.h>
#include <stdio.h>
#include <string.h>

#include <InterViews/bitmap.h>
#include <InterViews/brush.h>
#include <InterViews/color.h>
#include <InterViews/font.h>
#include <InterViews/printer.h>
#include <InterViews/raster.h>
#include <InterViews/transformer.h>
#include <OS/list.h>
#include <OS/math.h>

static const float PAGE_WIDTH = 8.5 * 72;
static const float PAGE_HEIGHT = 11 * 72;
static const float epsilon = 0.01;

static const char* ps_prolog = "\
save 20 dict begin\n\
\n\
/sf {   % scale /fontName => -  (set current font)\n\
    {findfont} stopped {pop /Courier findfont} if\n\
    exch scalefont setfont\n\
} def\n\
\n\
/ws {\n\
    4 index 6 4 roll moveto sub\n\
    2 index stringwidth pop sub\n\
    exch div 0 8#40 4 3 roll\n\
    widthshow\n\
} def\n\
\n\
/as {\n\
    4 index 6 4 roll moveto sub\n\
    2 index stringwidth pop sub\n\
    exch div 0 3 2 roll\n\
    ashow\n\
} def\n\
\n\
";

static const char* ps_epilog = "\
end restore\n\
";

#ifdef WIN32
declareList(PRtransformerList,Transformer)
implementList(PRtransformerList,Transformer)
#endif

class PrinterInfo {
public:
    const Color* color_;
    const Brush* brush_;
    const Font* font_;
};

class PrinterInfoList;

class PrinterRep {
public:
    std::ostream* out_;
    int page_;
    PrinterInfoList* info_;

    float x_;
    float y_;
    Coord text_curx_;
    Coord text_cury_;
    int text_chars_;
	 int text_spaces_;
#ifdef WIN32
	 Transformer tr_;
	 PRtransformerList trl_;
#endif
};

declareList(PrinterInfoList,PrinterInfo)
implementList(PrinterInfoList,PrinterInfo)

static void do_color(std::ostream& out, const Color* color) {
  // alpha supported added by cd1f 21-may-95
  float r, g, b, a;
  a = color->alpha();
  color->intensities(r, g, b);
  if(a == 1.0) {
    out << r << " " << g << " " << b << " setrgbcolor\n";
  }
  else {
    float c = (r + g + b)/3.0;
    if(c < 0.5)  out  << (1.0 - a) << " " << "setgray\n";
    else out  << a << " " << "setgray\n";
  }
}

static void do_brush(std::ostream& out, const Brush* brush) {
    Coord linewidth = brush ? brush->width() : 1;
    out << linewidth << " setlinewidth\n";
#if !defined(WIN32) && !MAC
 // Should do something about patterned brushes.
 // Maybe something like this:
  
    int dcnt = brush ? brush->dash_count() : 0;
    if(dcnt == 0) 
      out << "[] 0 setdash\n";
    else {
      out << "[";
      for(int i=0; i < dcnt; i++) {
	out << " " << brush->dash_list(i);
      }
      out << "] 0 setdash\n";
    }
#endif
}

static void do_font(std::ostream& out, const Font* font) {
    out << font->size() << " /";
    const char* p = font->name();
    while (*p != '\0') {
        out << char(*p == ' ' ? '-' : *p);
        ++p;
    }
    out << " sf\n";
}

Printer::Printer(std::ostream* out) {
    PrinterRep* p = new PrinterRep;
    rep_ = p;
    p->out_ = out;
    p->page_ = 1;
    p->x_ = 0;
    p->y_ = 0;
    p->text_curx_ = 0;
    p->text_cury_ = 0;
    p->text_chars_ = 0;
    p->text_spaces_ = 0;

    PrinterInfo info;
    info.color_ = nil;
    info.brush_ = nil;
    info.font_ = nil;

    p->info_ = new PrinterInfoList();
    p->info_->append(info);
}

Printer::~Printer() {
    flush();
    delete rep_->info_;
    delete rep_;
}

PixelCoord Printer::to_pixels(Coord p, DimensionName) const { return (PixelCoord)p; }
Coord Printer::to_coord(PixelCoord p, DimensionName) const { return p; }
Coord Printer::to_pixels_coord(Coord p, DimensionName) const { return p; }

#ifdef WIN32
void Printer::size(Coord width, Coord height){}
void Printer::psize(PixelCoord width, PixelCoord height){}

Coord Printer::width() const {return 8.5*72;}
Coord Printer::height() const {return 11.*72;}
PixelCoord Printer::pwidth() const {return (PixelCoord)8.5*72;}
PixelCoord Printer::pheight() const {return (PixelCoord)11.*72;}

void Printer::transformer(const Transformer& t){
	rep_->tr_ = t;
}
const Transformer& Printer::transformer() const {
	return rep_->tr_;
}

void Printer::damage(const Extension&){}
void Printer::damage(Coord, Coord, Coord, Coord){}
bool Printer::damaged(const Extension&) const{return true;}
bool Printer::damaged(
	Coord, Coord, Coord, Coord
	 ) const{return true;}
void Printer::damage_area(Extension& ex){
	ex.set_xy(this, 0.0, 0.0, width(), height());
}
void Printer::damage_all(){}
bool Printer::any_damage() const{return true;}
void Printer::restrict_damage(const Extension&){}
void Printer::restrict_damage(
		Coord, Coord, Coord, Coord
	 ){}

void Printer::redraw(Coord, Coord, Coord, Coord){}
void Printer::repair(){}

#endif

void Printer::resize(Coord left, Coord bottom, Coord right, Coord top) {
    PrinterRep* p = rep_;
    std::ostream& out = *p->out_;
    p->x_ = (float(left + right) - PAGE_WIDTH)/2;
    p->y_ = (float(top + bottom) - PAGE_HEIGHT)/2;
    flush();
    size(right - left, top - bottom);
    damage(left, bottom, right, top);
}

void Printer::prolog(const char* creator) {
    std::ostream& out = *rep_->out_;
    out << "%!PS-Adobe-2.0\n";
    out << "%%Creator: " << creator << "\n";
    out << "%%Pages: atend\n";
    out << "%%EndComments\n";
    out << ps_prolog;
    out << "%%EndProlog\n";
}

void Printer::epilog () {
    PrinterRep* p = rep_;
    std::ostream& out = *p->out_;
    flush();
    out << "showpage\n";
    out << "%%Trailer\n";
    out << ps_epilog;
    out << "%%Pages: " << p->page_ - 1 << "\n";
}

void Printer::comment(const char* text) {
    std::ostream& out = *rep_->out_;
    flush();
    out << "%% " << text << "\n";
}

void Printer::page(const char* label) {
    PrinterRep* p = rep_;
    flush();
    if (p->page_ > 1) {
        *p->out_ << "showpage\n";
    }
    *p->out_ << "%%Page: " << label << " " << p->page_ << "\n";
    *p->out_ << -(p->x_) << " " << -(p->y_) << " translate\n";
    PrinterInfo& info = p->info_->item_ref(p->info_->count() - 1);
    info.font_ = nil;
    info.color_ = nil;
    info.brush_ = nil;
    p->page_ += 1;
}

void Printer::push_transform() {
	PrinterRep* p = rep_;
#ifdef WIN32
	p->trl_.append(p->tr_);
#else
	 Canvas::push_transform();
#endif
	 flush();
    long depth = p->info_->count();
    PrinterInfo info = p->info_->item_ref(depth - 1);
    p->info_->insert(depth, info);
    *p->out_ << "gsave\n";
}

void Printer::pop_transform() {
    PrinterRep* p = rep_;
    flush();
    long depth = p->info_->count();
    p->info_->remove(depth - 1);
	 *p->out_ << "grestore\n";
#ifdef WIN32
	long tcnt = p->trl_.count();
	if (tcnt) {
		p->tr_ = p->trl_.item(tcnt - 1);
		p->trl_.remove(tcnt - 1);
	}
#else
	 Canvas::pop_transform();
#endif
}

void Printer::transform(const Transformer& t) {
	PrinterRep* p = rep_;
#ifdef WIN32
	p->tr_.premultiply(t);
#else
	 Canvas::transform(t);
#endif
	 flush();
    float a00, a01, a10, a11, a20, a21;
    t.matrix(a00, a01, a10, a11, a20, a21);
    *p->out_ << "[" << a00 << " " << a01;
    *p->out_ << " " << a10 << " " << a11;
    *p->out_ << " " << a20 << " " << a21 << "] concat\n";
}

/*
 * The current transform push/pop just saves and restores state,
 * so we can use it for clipping as well.
 */

#if defined(WIN32) || MAC
void Printer::push_clipping(bool) {
#else
void Printer::push_clipping() {
#endif
    PrinterRep* p = rep_;
    flush();
    long depth = p->info_->count();
    PrinterInfo info = p->info_->item_ref(depth - 1);
    p->info_->insert(depth, info);
    *p->out_ << "gsave\n";
}

void Printer::pop_clipping() {
    PrinterRep* p = rep_;
    flush();
    long depth = p->info_->count();
    p->info_->remove(depth - 1);
    *p->out_ << "grestore\n";
}

void Printer::new_path() {
    std::ostream& out = *rep_->out_;
    flush();
    out << "newpath\n";
}

void Printer::move_to(Coord x, Coord y) {
    std::ostream& out = *rep_->out_;
    flush();
    out << x << " " << y << " moveto\n";
}

void Printer::line_to(Coord x, Coord y) {
    std::ostream& out = *rep_->out_;
    flush();
    out << x << " " << y << " lineto\n";
}

void Printer::curve_to(
    Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
) {
    std::ostream& out = *rep_->out_;
    flush();
    out << x1 << " " << y1 << " " << x2 << " " << y2 << " ";
    out << x << " " << y << " curveto\n";
}

void Printer::close_path() {
    std::ostream& out = *rep_->out_;
    flush();
    out << "closepath\n";
}

void Printer::stroke(const Color* color, const Brush* brush) {
    PrinterRep* p = rep_;
    std::ostream& out = *p->out_;
    flush();
    PrinterInfo& info = p->info_->item_ref(p->info_->count() - 1);
    if (info.color_ != color) {
        do_color(out, color);
        info.color_ = color;
    }
    if (info.brush_ != brush) {
        do_brush(out, brush);
        info.brush_ = brush;
    }
    out << "gsave stroke grestore\n";
}

void Printer::fill(const Color* color) {
    PrinterRep* p = rep_;
    std::ostream& out = *p->out_;
    flush();
    PrinterInfo& info = p->info_->item_ref(p->info_->count() - 1);
    if (info.color_ != color) {
        do_color(out, color);
        info.color_ = color;
    }
    out << "gsave eofill grestore\n";
}

void Printer::clip() {
    std::ostream& out = *rep_->out_;
    flush();
    out << "eoclip\n";
}

void Printer::character(
    const Font* font, long c, Coord width, const Color* color, Coord x, Coord y
) {
#if defined(__GNUC__)
    char g3[40];
#endif
    PrinterRep* p = rep_;
    std::ostream& out = *p->out_;
    PrinterInfo& info = p->info_->item_ref(p->info_->count() - 1);
    if (info.color_ != color) {
        flush();
        do_color(out, color);
        info.color_ = color;
    }
    if (!Math::equal(y, p->text_cury_, epsilon)) {
        flush();
    }
    if (!Math::equal(x, p->text_curx_, epsilon)) {
        flush();
    }
    if (info.font_ != font) {
        flush();
        do_font(out, font);
        info.font_ = font;
    }
    if (p->text_chars_ == 0) {
        out << x << " " << y << "(";
    }
    p->text_curx_ = x + width;
    p->text_cury_ = y;
    if (c == '\\' || c == ')' || c == '(') {
        out << "\\" << char(c);
    } else if (c > 127) {
#if defined(__GNUC__)
//	out.form("\\%03o", c);
	snprintf(g3, 40, "\\%03lo", c);
	out << g3;
#else
	out << "\\";
        int old_width = out.width(3);
        char old_fill = out.fill('0');
        out << std::oct << c << std::dec;
        out.width(old_width);
        out.fill(old_fill);
#endif
    } else {
        out << char(c);
    }
    p->text_chars_ += 1;
    if (c == ' ') {
        p->text_spaces_ += 1;
    }
}

void Printer::flush() {
    PrinterRep* p = rep_;
    std::ostream& out = *p->out_;
    if (p->text_chars_ > 0) {
        out << ") ";
        if (p->text_spaces_ > 0) {
            out << p->text_spaces_ << " " << p->text_curx_ << " ws\n";
        } else {
            out << p->text_chars_ << " " << p->text_curx_ << " as\n";
        }
        p->text_chars_ = 0;
        p->text_spaces_ = 0;
    }
}

void Printer::stencil(
    const Bitmap* mask, const Color* color, Coord x, Coord y
) {
#if defined(__GNUC__)
    char g3[24];
#endif
    PrinterRep* p = rep_;
    std::ostream& out = *p->out_;
    flush();
    PrinterInfo& info = p->info_->item_ref(p->info_->count() - 1);
    if (info.color_ != color) {
        do_color(out, color);
        info.color_ = color;
    }
    unsigned long width = mask->pwidth();
    unsigned long height = mask->pheight();
    unsigned long bytes = (width-1)/8 + 1;
    Coord left = x - mask->left_bearing();
    Coord right = x + mask->right_bearing();
    Coord bottom = y - mask->descent();
    Coord top = y + mask->ascent();
    out << "gsave\n";
    out << "/picstr " << bytes << " string def\n";
    out << left << " " << bottom << "  translate\n";
    out << right - left << " " << top - bottom << " scale\n";
    out << width << " " << height << " true\n";
    out << "[" << width << " 0 0 " << height << " 0 0]\n";
    out << "{currentfile picstr readhexstring pop} imagemask\n";
#ifndef __GNUC__
    int old_width = out.width(1);
    out << std::hex;
#endif
    for (int iy = 0; iy < height; ++iy) {
        for (int ix = 0; ix < bytes; ++ix) {
            int byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                if (mask->peek(ix*8 + bit, iy)) {
                    byte |= 0x80 >> bit;
                }
            }
#if defined(__GNUC__)
	    //out.form("%02x", byte);
	snprintf(g3, 24, "%02x", byte);
	out << g3;
#else
            out << ((byte>>4) & 0x0f) <<  (byte & 0x0f);
#endif
        }
        out << "\n";
    }
#ifndef __GNUC__
    out << std::dec;
    out.width(old_width);
#endif
    out << "grestore\n";
}

void Printer::image(const Raster* raster, Coord x, Coord y) {
#if defined(__GNUC__)
    char g3[8];
#endif
    PrinterRep* p = rep_;
    std::ostream& out = *p->out_;
    flush();
    unsigned long width = raster->pwidth();
    unsigned long height = raster->pheight();
    float left = float(x) - raster->left_bearing();
    float right = float(x) + raster->right_bearing();
    float bottom = float(y) - raster->descent();
    float top = float(y) + raster->ascent();
    out << "gsave\n";
    out << "/picstr " << width << " string def\n";
    out << left << " " << bottom << "  translate\n";
    out << right - left << " " << top - bottom << " scale\n";
    out << width << " " << height << " 8\n";
    out << "[" << width << " 0 0 " << height << " 0 0]\n";
    out << "{currentfile picstr readhexstring pop} image\n";
#ifndef __GNUC__
    int old_width = out.width(1);
    out << std::hex;
#endif
    for (int iy = 0; iy < height; ++iy) {
        for (int ix = 0; ix < width; ++ix) {
            float r, g, b, alpha;
            raster->peek(ix, iy, r, g, b, alpha);
            int byte = int(0xff * (r + g + b) / 3);

#if defined(__GNUC__)
//	    out.vform("%02x", byte);
	snprintf(g3, 8, "%02x", byte);
	out << g3;
#else
            out << ((byte>>4) & 0x0f) <<  (byte & 0x0f);
#endif
        }
        out << "\n";
    }
#ifndef __GNUC__
    out << std::dec;
    out.width(old_width);
#endif
    out << "grestore\n";
}
