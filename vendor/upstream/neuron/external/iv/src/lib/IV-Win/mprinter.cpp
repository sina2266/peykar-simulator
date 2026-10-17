#include <../../config.h>

/*
Copyright (C) 2002 Michael Hines
This file contains programs and data originally developed by Michael Hines

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
mprinter.cpp
Allow use of Window's printer. Code hacked from the Mac version.
*/

#include <stdio.h>

#include <IV-Win/MWlib.h>
#include <IV-Win/canvas.h>
#include <IV-Win/mprinter.h>
#include <InterViews/transformer.h>

//#include "/nrn/src/mswin/winio/debug.h"

class MacPrinterCanvas : public MWcanvas16 {
public:
	MacPrinterCanvas();
	virtual ~MacPrinterCanvas();
	bool get();
	void start(float scale);
	void finish();
	void setup(bool);
	virtual Window* window() const;
	virtual void size(Coord width, Coord height);
	virtual void psize(int width, int height);
	virtual void resize(Coord, Coord, Coord, Coord);
	 virtual Coord width() const;
	 virtual Coord height() const;
	virtual PixelCoord pwidth() const;
	virtual PixelCoord pheight() const;
	virtual PixelCoord to_pixels(Coord x, DimensionName)const;
	virtual Coord to_coord(PixelCoord x, DimensionName)const;
private:
	HDC GetPrinterDC();
	virtual void setmapmode(HDC);
private:
	HDC hdc;
	int hres_, vres_, lpx_, lpy_;
	float scale_;
 };


MacPrinterCanvas::MacPrinterCanvas() {
	hdc = nil;
	setup(false);
}
MacPrinterCanvas::~MacPrinterCanvas() {
	if (hdc) {
		DeleteDC(hdc);
	}
}
Window* MacPrinterCanvas::window()const {
 //DebugMessage("MacPrinter::window()\n");
 return nil;
}
void MacPrinterCanvas::size(Coord, Coord) {};
void MacPrinterCanvas::psize(int, int) {};
void MacPrinterCanvas::resize(Coord, Coord, Coord, Coord) {}
Coord MacPrinterCanvas::width() const {
	return Coord(hres_)/float(lpx_)*72.;
}
Coord MacPrinterCanvas::height() const {
	return Coord(vres_)/float(lpy_)*72.;
}
PixelCoord MacPrinterCanvas::pwidth() const {
	return hres_;
}
PixelCoord MacPrinterCanvas::pheight() const {
	return vres_;
}
PixelCoord MacPrinterCanvas::to_pixels(Coord x, DimensionName)const {
	return int(x/72.*lpx_);
}
Coord MacPrinterCanvas::to_coord(PixelCoord x, DimensionName)const {
	return Coord(x)*float(lpx_)/72.;
}

MacPrinter::MacPrinter(std::ostream*) : Printer(nil){
	c = new MacPrinterCanvas();
}
MacPrinter::~MacPrinter() {
	delete c;
}
Window* MacPrinter::window()const {
//DebugMessage("MacPrinter::window()\n");
 return nil;
}
void MacPrinter::setup(bool b) { c->setup(b); }
void MacPrinterCanvas::setup(bool) {}

void MacPrinterCanvas::setmapmode(HDC hdc) {
	int s = int(1440./scale_);
	MWassert(SetMapMode(hdc, MM_ANISOTROPIC));
	//MWassert(SetWindowExtEx(hdc, 1440, 1440, NULL));
	MWassert(SetWindowExtEx(hdc, s, s, NULL));
	MWassert(SetViewportExtEx(hdc,
		GetDeviceCaps(hdc, LOGPIXELSX),
		-GetDeviceCaps(hdc, LOGPIXELSY),
		NULL));
}

HDC MacPrinterCanvas::GetPrinterDC(){
#if 1
	PRINTDLG pd;

	// Initialize PRINTDLG
	ZeroMemory(&pd, sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner   = NULL;
	pd.hDevMode    = NULL;     // Don't forget to free or store hDevMode
	pd.hDevNames   = NULL;     // Don't forget to free or store hDevNames
	pd.Flags       = PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDC; 
	pd.nCopies     = 1;
	pd.nFromPage   = 0xFFFF; 
	pd.nToPage     = 0xFFFF; 
	pd.nMinPage    = 1; 
	pd.nMaxPage    = 0xFFFF; 

	if (PrintDlg(&pd)==TRUE) {
		return pd.hDC;
	} 
#else
	static char szPrinter[80];
	char* szDevice, *szDriver, *szOutput;
	szDevice=szPrinter;
	GetProfileString("windows", "device", ",,,", szPrinter, 80);
	if (NULL != (szDevice = strtok(szPrinter, ",")) &&
		NULL != (szDriver = strtok(NULL, ", "))
		 //&& NULL != (szOutput = strtok(NULL, ", "))
		) {
			return CreateDC(szDriver, szDevice, NULL, NULL);
	}
#endif
	return 0;
}

void MacPrinter::prolog(float scale) {c->start(scale);}
void MacPrinter::prolog(const char*) {c->start(1.);}
bool MacPrinter::get() { return c->get(); }
bool MacPrinterCanvas::get() {
	if (!hdc) {
		hdc = GetPrinterDC();
	}
	if (!hdc) {
		//DebugMessage("hdc = null\n");
		return false;
	}
	hres_ = GetDeviceCaps(hdc, HORZRES);
	vres_ = GetDeviceCaps(hdc, VERTRES);
	lpx_ = GetDeviceCaps(hdc, LOGPIXELSX);
	lpy_ = GetDeviceCaps(hdc, LOGPIXELSY);
	return true;
}

void MacPrinterCanvas::start(float scale) {
	static char szMsg[] = "NEURON";
	scale_ = scale;
	if (!get()) {
		return;
	}
	//if (!Escape(hdc, STARTDOC, sizeof szMsg-1, szMsg, NULL)) {
		//DebugMessage("STARTDOC failed\n");
	//	abort();
	//}
	DOCINFO di;
  di.cbSize      = sizeof(DOCINFO);
  di.lpszDocName = "NEURON";
  di.lpszOutput  = NULL;
	
	StartDoc(hdc, &di);
	StartPage(hdc);
//	SetMapMode(hdc, MM_TWIPS);
	int a = GetMapMode(hdc);
	SIZE b; GetViewportExtEx(hdc, &b);
	POINT c; GetViewportOrgEx(hdc, &c);
	XFORM d; GetWorldTransform(hdc, &d);
	if (a < -10) { return; }
	damage_all();
	RECT r;
	//SetRectRgn(&r, 0, 0, hres_/2, vres_/2);
	r.left = 0; r.top=0; r.right = hres_; r.bottom=vres_;
	beginPaint(hdc, r);
	//SelectClipRgn(hdc, NULL);

	//d.eM11 = d.eM22 *= 2.;	
	//SetWorldTransform(hdc, &d);
}

void MacPrinter::epilog() {c->finish();}
void MacPrinterCanvas::finish() {
	if (!hdc) {return;}
	endPaint();
	//if (!Escape(hdc, NEWFRAME, 0, NULL, NULL)) {
		//DebugMessage("NEWFRAME failed\n");
	//	abort();
	//}
	//if (!Escape(hdc, ENDDOC, 0, NULL, NULL)) {
		//DebugMessage("ENDDOC failed\n");
	//	abort();
	//}
	EndPage(hdc);
	EndDoc(hdc);
	DeleteDC(hdc);
	hdc = 0;
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
