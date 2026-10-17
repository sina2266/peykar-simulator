// =========================================================================
//
//				<IV-Mac/canvas.h>
// Machintosh dependent Canvas representation.  This canvas type renders
// into an Machintosh window.  
//
// 1.5
// $Date:   4 Aug 1996
//
// =========================================================================
#ifndef ivmac_canvas_h
#define ivmac_canvas_h

// ---- InterViews includes ----
#include <InterViews/canvas.h>
#include <InterViews/iv.h>
class BitmapRep;
class WindowRep;
class Canvas;
class Display;


class MACtransformPtrList;		// PtrList<Transformer*>
class MACclipList;				// List<HRGN>

class PathRenderInfo
{
public:
    Coord curx_;
    Coord cury_;
	Point* point_;
	Point* cur_point_;
	Point* end_point_;
};

class MACcanvas : public Canvas
{
public:
	MACcanvas();
	virtual ~MACcanvas();

	void bind(WindowRep* w)				// bind canvas to a window
    	{ win_ = w; }

	
	void setWinToUpdate(void);
	
	virtual void initClip(); // just initializes clipping so no bug on resize
	virtual void beginPaint();
	virtual void endPaint(); 				
		// These two functions determine when the canvas can be rendered
		// upon, and when it can't.  The canvas is not valid for drawing
		// outside of a begin/end pair.  
	inline int toPixelX(Coord x) const;
	inline int toPixelY(Coord y) const;
	inline Coord fromPixelX(int x) const;
	inline Coord fromPixelY(int x) const;
	inline void setDamage(Rect * rectangle);
	inline void addToDamage(Rect * rectangle);
	inline Rect * getDamage();
	inline void clearDamage();
	void begin_item(PixelCoord x, PixelCoord y, PixelCoord w);
	
	// ---------------- InterViews interface ---------------------

    virtual Window* window() const;

    virtual void size(Coord width, Coord height);
    virtual void psize(PixelCoord width, PixelCoord height);

    virtual Coord width() const;
    virtual Coord height() const;
    virtual PixelCoord pwidth() const;
    virtual PixelCoord pheight() const;

    virtual PixelCoord to_pixels(Coord, DimensionName) const;
    virtual Coord to_coord(PixelCoord, DimensionName) const;

    virtual void push_transform();
    virtual void transform(const Transformer&);
    virtual void pop_transform();
    virtual void transformer(const Transformer&);
    virtual const Transformer& transformer() const;

	virtual void push_clipping(bool all = false);
    virtual void pop_clipping();
    virtual void clip();

    virtual void front_buffer();
    virtual void back_buffer();

    virtual void damage(const Extension&);
    virtual void damage(Coord left, Coord bottom, Coord right, Coord top);
    virtual bool damaged(const Extension&) const;
    virtual bool damaged( 
		Coord left, Coord bottom, Coord right, Coord top) const;
    virtual void damage_area(Extension&);
    virtual void damage_all();
    virtual bool any_damage() const;
    virtual void restrict_damage(const Extension&);
    virtual void restrict_damage(Coord left, Coord bottom, 
		Coord right, Coord top);

    virtual void redraw(Coord left, Coord bottom, Coord right, Coord top);
    virtual void repair();

	Transformer& matrix() const;		
		// current transformation matrix in effect for the rendering 
		// surface.

	void stencilFill(const Bitmap*, const Color*);
		// This function fills the current path with a stenciled pattern
		// in the given color.  This function is basically used to simulate
		// the alpha blending of color which is not directly supported by
		// the GDI interface (ie PatBlt doesn't allow raster operations that
		// specify source as part of the operation... so no stencil).  Since
		// the regions stenciled are typically not that large, this shouldn't
		// be too big of a shortcoming... it's too bad though because some
		// smart framebuffers can do this in hardware wicked-fast :-)
		//

virtual void new_path();
	virtual void move_to(Coord x, Coord y);
	virtual void line_to(Coord x, Coord y);
	virtual void curve_to(Coord x, Coord y, 
		Coord x1, Coord y1, Coord x2, Coord y2);
	virtual void close_path();

    virtual void stroke(const Color*, const Brush*);
    virtual void fill(const Color*);
    virtual void character(const Font*, long ch, Coord width, 
		const Color*, Coord x, Coord y);
    virtual void stencil(const Bitmap*, const Color*, Coord x, Coord y);
    virtual void image(const Raster*, Coord x, Coord y);

protected:
	virtual void flush();					// flush any buffered operations

	int transformAngle() const;
		// determines the current transformation angle in terms of tenths
		// of a degree.  Returns a number between 0 and 3600.

    // ---- attribute setting functions ----
	void color(const Color*);
	void brush(const Brush*);
	void font(const Font*);


	WindowRep* win_;				// associated window
	Display* dpy;					// display
	
	bool transformed_;
	MACtransformPtrList* transformers_;
	
	Rect* clipping_;
	MACclipList* clippers_;
    
//private:
protected: // for MacPrinterCanvas

	Rect damageArea;			
		// area of canvas currently damaged.  This is maintained in terms of
		// world coordinates since it is used primarily by clients of the 
		// canvas.

	//PixelCoord canvas_width;
	//PixelCoord canvas_height;
	
	#if 0			
		// size of canvas in pixels.
	HPEN pen_;						// current pen to render with
    HBRUSH brush_;					// current brush to render with
	LOGPEN pen_stats_;				// what the pen was constructed from
	LOGBRUSH brush_stats_;			// what the brush was constructed from

	HPEN old_pen_;					// pen before our pen was selected into the DC
	HBRUSH old_brush_;				// brush before our brush was selected into the DC
	#endif
	
	// ---- logical attributes -----
	const Color* lg_color_;			// last IV color set
	const Brush* lg_brush_;			// last IV brush set
	const Font* lg_font_;			// last IV font set
	static PathRenderInfo path_;	// path info (filled by Canvas members)

	// ---- text support ----
	struct TextItem {
        int count;
        PixelCoord x, y;
		enum { size = 256 };
        char buffer[size];
    	//PixelCoord next_x;
    };
    TextItem text_item_;
};

inline int MACcanvas::toPixelX(Coord x) const
	{return to_pixels(x, Dimension_X);}
inline int MACcanvas::toPixelY(Coord y) const
	{return pheight() - to_pixels(y, Dimension_Y);}	
inline Coord MACcanvas::fromPixelX(int x) const
	{return to_coord(x, Dimension_X);}
inline Coord MACcanvas::fromPixelY(int y) const
	{return to_coord((pheight() - y), Dimension_Y);}	
inline void MACcanvas::setDamage(Rect * rectangle)
	{ damageArea = *rectangle; }
inline void MACcanvas::addToDamage(Rect * rectangle)
	{ UnionRect(&damageArea, rectangle, &damageArea); }
inline Rect * MACcanvas::getDamage()
	{ return &damageArea; }
inline void MACcanvas::clearDamage(void)
	{ SetRect(&damageArea, 0, 0, 0, 0); }
#endif /* ivmac_canvas_h */
