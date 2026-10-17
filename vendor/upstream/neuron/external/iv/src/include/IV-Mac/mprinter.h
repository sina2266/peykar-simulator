/*
 * Printer - generate output for a printer
 */

#ifndef iv_mcprinter_h
#define iv_mcprinter_h

#include <InterViews/printer.h>
#include <IV-Mac/canvas.h>
#include <ivstream.h>

class MacPrinterCanvas : public MACcanvas {
public:
	MacPrinterCanvas();
	virtual ~MacPrinterCanvas();
	bool start();
	bool finish();
	bool setup(bool s = true);
	virtual Window* window() const;
	virtual void size(Coord width, Coord height);
	virtual void psize(Coord width, Coord height);
	virtual void resize(Coord, Coord, Coord, Coord);
    virtual Coord width() const;
    virtual Coord height() const;
	virtual PixelCoord pwidth() const;
	virtual PixelCoord pheight() const;
	virtual PixelCoord to_pixels(Coord x, DimensionName)const;
	virtual Coord to_coord(PixelCoord x, DimensionName)const;
	virtual void flush();
private:
	GrafPtr oldPort;
#if carbon
	PMPrintSession pmses;
	PMPageFormat pmform;
	PMRect pmrect;
#else	
	THPrint prRecHdl;
	TPPrPort gPrinterPort;
#endif
 };

class MacPrinter : public Printer {
public:
    MacPrinter(std::ostream* o = nil);
	 virtual ~MacPrinter();
	 void setup();
	virtual Window* window() const;
	 virtual Coord width() const;
	 virtual Coord height() const;
	 virtual PixelCoord pwidth() const;
	 virtual PixelCoord pheight() const;

	 virtual void transformer(const Transformer&);
	 virtual const Transformer& transformer() const;

    virtual void resize(Coord left, Coord bottom, Coord right, Coord top);

    virtual void prolog(const char* creator = "InterViews");
    virtual void epilog();

    virtual void comment(const char*);
    virtual void page(const char*);

    virtual void push_transform();
    virtual void transform(const Transformer&);
    virtual void pop_transform();

    virtual void push_clipping(bool all = false);
    virtual void clip();
    virtual void pop_clipping();

    virtual void new_path();
    virtual void move_to(Coord x, Coord y);
    virtual void line_to(Coord x, Coord y);
    virtual void curve_to(
        Coord x, Coord y, Coord x1, Coord y1, Coord x2, Coord y2
    );
    virtual void close_path();
    virtual void stroke(const Color*, const Brush*);
    virtual void fill(const Color*);

    virtual void character(
        const Font*, long c, Coord width, const Color*, Coord x, Coord y
    );
    virtual void stencil(const Bitmap*, const Color*, Coord x, Coord y);
    virtual void image(const Raster*, Coord x, Coord y);
private:
	MacPrinterCanvas* c;
};

#endif
