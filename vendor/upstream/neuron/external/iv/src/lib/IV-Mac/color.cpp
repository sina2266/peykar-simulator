#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
// =======================================================================
//
//                     <IV-Mac/color.cpp>
//
// Macintosh implementation of the InterViews Color classes.
//
//
// 1.1
// $Date:   4 Aug 1996
//
// =======================================================================

#include <OS/string.h>
#include <OS/math.h>
#include <OS/table.h>
#include <InterViews/color.h>
#include <InterViews/session.h>
#include <InterViews/bitmap.h>
#include <IV-Mac/color.h>
#include <IV-Mac/window.h>

#include <string.h>
#include <stdio.h>

//Appropriate conversion factor for the RGB color codes in the Macintosh
#define BIT14_COLOR ((2 << 15) - 1)

// #######################################################################
// ################# class ColorRep
// #######################################################################

// -----------------------------------------------------------------------
// constructors/destructors
// -----------------------------------------------------------------------
ColorRep::ColorRep(
	int r,					// red component of color
	int g,					// green component of color
	int b)					// blue component of color
{
	color_.red = r;
	color_.green = g;
	color_.blue = b;
}

ColorRep::~ColorRep()
{
}


// ------------------------------------------------------------------
// translates a color name to the X11 string format of an rgb
// specification (ie #??????).  The colormap name is basically a
// section in the colormap.ini file.
// ------------------------------------------------------------------
#ifdef MAC
static struct { char* name; char* value; } cc[] = {
{"red", "#ff0000"},
{"green", "#00ff00"},
{"blue", "#0000ff"},
{"white", "#ffffff"},
{"black", "#000000"},
{"orange", "#ffa500"},
{"brown", "#a52a2a"},
{"violet", "#ee82ee"},
{"yellow", "#ffff00"},
{"gray", "#bebebe"},
{0}
};
#endif

const char* ColorRep::nameToRGB(const char* colormap, const char* name)
{
	if(!colormap && !name)
		return nil;

#ifdef MAC
	int i;
	for (i = 0; cc[i].name; ++i) {
		if (strcmp(cc[i].name, name) == 0) {
			return cc[i].value;
		}
	}
#endif
#if 0
	if (COLORMAP_FILE == NULL)
	{
		const char* loc = Session::installLocation();
		const char* leafname = "/colormap.ini";
		COLORMAP_FILE = new char[ strlen(loc) + strlen(leafname) + 1];
		strcpy(COLORMAP_FILE, loc);
		strcat(COLORMAP_FILE, leafname);
	}

	static char rgbName[10];
	if (GetPrivateProfileString(colormap, name, "", rgbName, 10,
		COLORMAP_FILE))
	{
		return rgbName;
	}
#endif
	return NULL;
}

const Color* ColorRep::rgbToColor(const char* name)
{
	if (name[0] == '#')
	{
		int r;
		int g;
		int b;
		sscanf(&(name[1]), "%2x", &r);
		sscanf(&(name[3]), "%2x", &g);
		sscanf(&(name[5]), "%2x", &b);
		const Color* c = new Color( 
			(ColorIntensity) (r/255.0),
			(ColorIntensity) (g/255.0),
			(ColorIntensity) (b/255.0),
			1.0);
		return c;
	}
	return nil;
}


// #######################################################################
// ################# class Color
// #######################################################################

// The following data is used to create 16 stipple patterns used when the
// alpha value of the color is set to something other than 1.0.
static unsigned char stippleData[16][8] = 
{ 
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x11, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00 },
	{ 0x11, 0x00, 0x44, 0x00, 0x11, 0x00, 0x44, 0x00 }, 
	{ 0x55, 0x00, 0x44, 0x00, 0x55, 0x00, 0x44, 0x00 },
	{ 0x55, 0x00, 0x55, 0x00, 0x55, 0x00, 0x55, 0x00 },
	{ 0x55, 0x22, 0x55, 0x00, 0x55, 0x22, 0x55, 0x00 },
	{ 0x55, 0x22, 0x55, 0x88, 0x55, 0x22, 0x55, 0x88 },
	{ 0x55, 0xAA, 0x55, 0x88, 0x55, 0xAA, 0x55, 0x88 },
	{ 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA },
	{ 0x77, 0xAA, 0x55, 0xAA, 0x77, 0xAA, 0x55, 0xAA },
	{ 0x77, 0xAA, 0xDD, 0xAA, 0x77, 0xAA, 0xDD, 0xAA },
	{ 0xFF, 0xAA, 0xDD, 0xAA, 0xFF, 0xAA, 0xDD, 0xAA },
	{ 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA, 0xFF, 0xAA },
	{ 0xFF, 0xBB, 0xFF, 0xAA, 0xFF, 0xBB, 0xFF, 0xAA },
	{ 0xFF, 0xBB, 0xFF, 0xEE, 0xFF, 0xBB, 0xFF, 0xEE },
	{ 0xFF, 0xFF, 0xFF, 0xEE, 0xFF, 0xFF, 0xFF, 0xEE },
};

Color::Color(
	ColorIntensity r,					// red component of color
	ColorIntensity g,					// green component of color
	ColorIntensity b,                   // blue component of color
	float alpha,
	ColorOp op)
{
	int red = (int) (r * BIT14_COLOR);
	int green = (int) (g * BIT14_COLOR);
	int blue = (int) (b * BIT14_COLOR);

	impl_ = new ColorRep(red, green, blue);
    impl_->op = op;

	// ---- set stipple pattern if dither desired ----
	impl_->alpha = alpha;
	if ((alpha > 0.9999) && (alpha < 1.0001))
	{
		impl_->stipple = NULL;
	}
	else
	{
		int index = int(alpha * 16);
		index = (index > 15) ? 15 : index;
		index = (index < 0) ? 0 : index;
		impl_->stipple = new Bitmap(stippleData[index], 8, 8);
	}
	
	
}

Color::Color(
	const Color& color,
	float alpha,
	ColorOp op)
{
	RGBColor* cref = color.impl_->MACcolor();
	int red = cref->red;
	int green = cref->green;
	int blue = cref->blue;
	
	impl_ = new ColorRep(red, blue, green);
    impl_->op = op;

	// ---- set stipple pattern if dither desired ----
	impl_->alpha = alpha;
	if ((alpha > 0.9999) && (alpha < 1.0001))
	{
		impl_->stipple = NULL;
	}
	else
	{
		int index = int(alpha * 16);
		index = (index > 15) ? 15 : index;
		index = (index < 0) ? 0 : index;
		impl_->stipple = new Bitmap(stippleData[index], 8, 8);
	}	
}

Color::~Color()
{
	delete impl_->stipple;
	delete impl_;
}

void Color::intensities(
	Display*,
	ColorIntensity& r,
	ColorIntensity& g,
	ColorIntensity& b) const
{
	intensities(r, g, b);
}

void Color::intensities(
	ColorIntensity& r,
	ColorIntensity& g,
	ColorIntensity& b) const
{
	RGBColor * cref = impl_->MACcolor();

	r = (ColorIntensity) (((float) cref->red) / (float)BIT14_COLOR);
	g = (ColorIntensity) (((float) cref->green) / (float)BIT14_COLOR);
	b = (ColorIntensity) (((float) cref->blue) / (float)BIT14_COLOR);
}

float Color::alpha() const
{
  return impl_->alpha; 
}

const Color* Color::brightness(float adjust) const
{
    ColorIntensity r, g, b;
    intensities(r, g, b);
	if (adjust >= 0)
	{
		r += (1 - r) * adjust;
		g += (1 - g) * adjust;
		b += (1 - b) * adjust;
	}
	else
	{
		float f = (float) (adjust + 1.0);
		r *= f;
		g *= f;
		b *= f;
    }
    return new Color(r, g, b);
}

bool Color::distinguished(const Color* c) const 
{
     return distinguished(Session::instance()->default_display(), c);
}

bool Color::distinguished(Display* d, const Color* color) const 
{
	RGBColor * cref = color->impl_->MACcolor();
	int red = cref->red;
	int green = cref->green;
	int blue = cref->blue;

	RGBColor * bogus;
	//return ( globalPalette.findEntry(red,green,blue,bogus) ) ? false : true;
	
	return false;
}

// ---------------------------------------------------------------------------
// Lookup color by name.  This is intended to look things up by the X11
// style name, which is partially supported under the Macintosh implementation.
// If the name starts with a '#' then we translate the hex numbers to rgb 
// values directly.  Otherwise, we attempt to look up the color name in
// the default colormap.
// ---------------------------------------------------------------------------
const Color* Color::lookup(Display* d, const char* name) 
{
	if (name)
	{
		// ---- check for rgb specification ----
		if (name[0] == '#')
			return ColorRep::rgbToColor(name);

		// ---- must be a color name ----
		const char* rgb;
		if (rgb = ColorRep::nameToRGB("default", name))
		{
			return ColorRep::rgbToColor(rgb);
		}
	}
	
	return nil;
}

const Color* Color::lookup(Display* d, const String& s) 
{
	// Since the value is not expected to be null terminiated, we simply
	// pass it through to the function that expects a (const char*) arg.
	return lookup(d, s.string());
}

bool Color::find(
    const Display* display, const String& name,
    ColorIntensity& r, ColorIntensity& g, ColorIntensity& b) 
{
    NullTerminatedString nm(name);
    return find(display, nm.string(), r, g, b);
}

bool Color::find(
    const Display*, 
	const char* name,
    ColorIntensity& r, 
	ColorIntensity& g, 
	ColorIntensity& b) 
{
	const char* rgb;
	if (rgb = ColorRep::nameToRGB("default", name))
	{
		if (name[0] == '#')
		{
			int ir;
			int ig;
			int ib;
			sscanf(&(name[1]), "%2x", &ir);
			sscanf(&(name[3]), "%2x", &ig);
			sscanf(&(name[5]), "%2x", &ib);

			r = (ColorIntensity) (ir/255.0);
			g = (ColorIntensity) (ig/255.0);
			b = (ColorIntensity) (ib/255.0);
			return true;
		}
	}
	return false;
}

