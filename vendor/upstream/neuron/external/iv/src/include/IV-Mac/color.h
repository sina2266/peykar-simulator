// ===========================================================================
//
//                     <IV-Mac/color.h>
//
//  Machintosh implementation of the InterViews Color classes.
//
//
// 1.1
// $Date:   4 Aug 1996
//
// ===========================================================================
#ifndef iv_mac_color_h
#define iv_mac_color_h

#include <InterViews/iv.h>

class Bitmap;

class ColorRep
{
public:
	ColorRep(int r, int g, int b);
	~ColorRep();
	
	RGBColor* MACcolor();						// mac color representation
	
	float alpha;
	ColorOp op;
	Bitmap* stipple;						// stipple pattern
	
	static const char* nameToRGB(const char* colormap, const char* name);
		// translates a color name to the X11 string format of an rgb
		// specification (ie #??????).  The colormap name is basically a
		// section in the colormap.ini file.

	static const Color* rgbToColor(const char* name);
		// translates an rgb string (#?????? format) to a color instance,
		// or returns null if there is a problem.  This is for support of 
		// X11 style color names.

private:
	RGBColor color_;
};

inline RGBColor* ColorRep::MACcolor()
	{ return &color_; }

#endif
