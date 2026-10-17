#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
#include <stdio.h>
#include <math.h>
#include <InterViews/transformer.h>
#include <InterViews/font.h>
#include <IV-Mac/mprinter.h>
#include <IV-Mac/font.h>

extern "C" {extern void debugfile(const char*, ...);}

MacPrinterCanvas::MacPrinterCanvas() {
#if carbon
#else
	prRecHdl = nil;
#endif
	setup(false);
}
MacPrinterCanvas::~MacPrinterCanvas() {
#if carbon
#else
	if (prRecHdl) {
		DisposeHandle((char**)prRecHdl);
	}
#endif
}
Window* MacPrinterCanvas::window()const {
 //debugfile("MacPrinter::window()\n");
 return nil;
}
void MacPrinterCanvas::size(Coord, Coord) {};
void MacPrinterCanvas::psize(Coord, Coord) {};
void MacPrinterCanvas::resize(Coord, Coord, Coord, Coord) {}
Coord MacPrinterCanvas::width() const {
#if carbon
	return Coord(pwidth());
#else
	Rect& r = (*prRecHdl)->prInfo.rPage;
	return Coord(r.right);
#endif
}
Coord MacPrinterCanvas::height() const {
#if carbon
	return Coord(pheight());
#else
	Rect& r = (*prRecHdl)->prInfo.rPage;
	return Coord(r.bottom);
#endif
}
PixelCoord MacPrinterCanvas::pwidth() const {
#if carbon
	return 8*72;
#else
	Rect& r = (*prRecHdl)->prInfo.rPage;
	return r.right;
#endif
}
PixelCoord MacPrinterCanvas::pheight() const {
#if carbon
	return 10*72;
#else
	Rect& r = (*prRecHdl)->prInfo.rPage;
	return r.bottom;
#endif
}
PixelCoord MacPrinterCanvas::to_pixels(Coord x, DimensionName)const { return int(x); }
Coord MacPrinterCanvas::to_coord(PixelCoord x, DimensionName)const {return Coord(x); }

MacPrinter::MacPrinter(std::ostream*) : Printer(nil){
	c = new MacPrinterCanvas();
}
MacPrinter::~MacPrinter() {
	delete c;
}
Window* MacPrinter::window()const {
// debugfile("MacPrinter::window()\n");
 return nil;
}
void MacPrinter::prolog(const char*) {c->start();}

void MacPrinter::setup() { c->setup();}

bool MacPrinterCanvas::setup(bool s){
#if carbon
	return false;
#else
	GetPort(&oldPort);
	PrOpen();
  if (!prRecHdl) {
	prRecHdl = THPrint(NewHandleClear(sizeof(TPrint)));
	if (PrError()  != noErr) {
		return false;
	}
	PrintDefault(prRecHdl);
	if (PrError()  != noErr) {
		return false;
	}
  }else{
	PrValidate(prRecHdl);
  }
    if (s) {
		PrStlDialog(prRecHdl);
	}
	PrClose();
	if (PrError()  != noErr) {
		return false;
	}
	//debugfile("MacPrinterCanvas::setup  width=%d height=%d\n", pwidth(), pheight());
	SetPort(oldPort);
	return true;
#endif
}

bool MacPrinterCanvas::start() {
#if carbon
	return false;
#else
//debugfile("MacPrinter::init\n");
	GetPort(&oldPort);
	PrOpen();
  if (!prRecHdl) {
	prRecHdl = THPrint(NewHandleClear(sizeof(TPrint)));
	if (PrError()  != noErr) {
		return false;
	}
	PrintDefault(prRecHdl);
	if (PrError()  != noErr) {
		return false;
	}
	PrStlDialog(prRecHdl);
  }else{
	PrValidate(prRecHdl);
  }
	gPrinterPort = PrOpenDoc(prRecHdl, nil, nil);
	if (PrError()  != noErr) {
		return false;
	}
	PrOpenPage(gPrinterPort, nil);
	if (PrError()  != noErr) {
		return false;
	}
	damage_all();
	beginPaint();
	return true;
#endif
}
void MacPrinter::epilog() {c->finish();}
bool MacPrinterCanvas::finish() {
#if carbon
	return false;
#else
	endPaint();
	PrClosePage(gPrinterPort);
	if (PrError()  != noErr) {
		return false;
	}
	PrCloseDoc(gPrinterPort);
	if (PrError()  != noErr) {
		return false;
	}
	if ((**prRecHdl).prJob.bJDocLoop == bSpoolLoop) {
		TPrStatus theStatus;
		PrPicFile(prRecHdl, nil, nil, nil, &theStatus);
		if (PrError()  != noErr) {
			return false;
		}
	}
	PrClose();
	if (PrError()  != noErr) {
		return false;
	}
	SetPort(oldPort);
//debugfile("MacPrinter::finish\n");
	return true;
#endif
}
// for scaling text
void MacPrinterCanvas::flush()
{
	// ---- check if there is anything to do ----
	//int nchars = (int) (text_ptr_ - text_buff_);
	//if ((nchars == 0) || (lg_font_ == nil))
	//	return;
		
	// ---- render the text ----
	if (text_item_.count > 0) {
		//set font
		FontRep* fr = lg_font_->rep(nil);
		TextFont(fr->font_);
		TextFace(fr->face_);
		float a00, a01, a10, a11, a20, a21;
		transformer().matrix(a00, a01, a10, a11, a20, a21);
		int size = int(double(fr->size_)*sqrt(a00*a00 + a01*a01) +.01);
		TextSize(size);
		TextMode(fr->mode_);
//if (printing) debugfile("flush %d %d\n",toPixelX(text_item_.x), toPixelY(text_item_.y)); 		
        MoveTo(toPixelX(text_item_.x), toPixelY(text_item_.y));
        DrawText(text_item_.buffer, 0, text_item_.count);
// text_item_.buffer[text_item_.count] = '\0';
// debugfile("|%s|\n", text_item_.buffer);
    }
    text_item_.count = 0;	
	
	return;
}

void MacPrinter::resize(Coord, Coord, Coord, Coord) {}
Coord MacPrinter::width() const {return c->width();}
Coord MacPrinter::height() const {return c->height();}
PixelCoord MacPrinter::pwidth() const {return c->pwidth();}
PixelCoord MacPrinter::pheight() const {return c->pheight();}
void MacPrinter::character(
	const Font* f, 					// font to use
	long ch,                        // character to print
	Coord width,                    // width of the character
	const Color* co,             	// color to use
	Coord x,                        // x-coordinate
	Coord y)                        // y-coordinate
{
	c->character(f, ch, width, co, x, y);
}
void MacPrinter::transformer(const Transformer& t){c->transformer(t);}
const Transformer& MacPrinter::transformer() const {return c->transformer();}
void MacPrinter::comment(const char* text) {}
void MacPrinter::page(const char* label) {}
void MacPrinter::push_transform() {c->push_transform();}
void MacPrinter::pop_transform() {c->pop_transform();}
void MacPrinter::transform(const Transformer& t) {c->transform(t);}
void MacPrinter::push_clipping(bool) {c->push_clipping();}
void MacPrinter::pop_clipping() {c->pop_clipping();}
void MacPrinter::new_path() {c->new_path();}
void MacPrinter::move_to(Coord x, Coord y) {c->move_to(x,y);}
void MacPrinter::line_to(Coord x, Coord y) {c->line_to(x,y);}
void MacPrinter::curve_to(
    Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
) {c->curve_to(x,y,x1,y1,x2,y2);}
void MacPrinter::close_path() {c->close_path();}
void MacPrinter::stroke(const Color* color, const Brush* brush) {c->stroke(color,brush);}
void MacPrinter::fill(const Color* color) {c->fill(color);}
void MacPrinter::clip() {c->clip();}
void MacPrinter::stencil(
    const Bitmap* mask, const Color* color, Coord x, Coord y
) {c->stencil(mask,color,x,y);}
void MacPrinter::image(const Raster* raster, Coord x, Coord y) {c->image(raster,x,y);}
