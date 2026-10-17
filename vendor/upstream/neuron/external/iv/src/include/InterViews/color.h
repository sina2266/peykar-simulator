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

#if defined(WIN32)
// =========================================================================
//
//  MS-Windows version.  The InterViews distribution contains some
//  X11-specific things in the interface, so this version is slightly
//  different.
//
// =========================================================================
#endif

#if !defined(WIN32) && !defined(MAC)
#define UNIX 1
#endif

#ifndef iv_color_h
#define iv_color_h

#include <InterViews/resource.h>

#include <InterViews/_enter.h>

class ColorImpl;
class ColorRep;
class Display;
class String;
#if UNIX
class WindowVisual;
#endif

typedef float ColorIntensity;
typedef unsigned int ColorOp;

class Color : public Resource {
public:
    enum { Copy, Xor, Invisible };

    Color(
	ColorIntensity r, ColorIntensity g, ColorIntensity b,
	float alpha = 1.0, ColorOp = Copy
    );
    Color(const Color&, float alpha = 1.0, ColorOp = Copy);
    virtual ~Color();

    static bool find(
	const Display*, const String&,
	ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
    );
    static bool find(
	const Display*, const char* name,
	ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
    );

    static const Color* lookup(Display*, const String& name);
    static const Color* lookup(Display*, const char* name);

    virtual bool distinguished(Display*, const Color*) const;
    virtual void intensities(
	Display*, ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
    ) const;
    virtual float alpha() const;
#if UNIX
    virtual ColorOp op() const;
#endif

    virtual const Color* brightness(float adjust) const;

    /* use default display */
    bool distinguished(const Color*) const;
    void intensities(
	ColorIntensity& r, ColorIntensity& g, ColorIntensity& b
    ) const;

#if UNIX
    ColorRep* rep(WindowVisual*) const;
private:
    ColorImpl* impl_;

    void remove(WindowVisual*) const;

    ColorRep* create(
	WindowVisual*, ColorIntensity, ColorIntensity, ColorIntensity,
	float, ColorOp
    ) const;
    void destroy(ColorRep*);

    /* anachronisms */
public:
    Color(int r, int g, int b);
    void Intensities(int& r, int& g, int& b) const;
    int PixelValue() const;
#else
    ColorRep* rep() const;
private:
    ColorRep* impl_;
#endif

};

#if !UNIX
inline ColorRep* Color::rep() const
	{ return impl_; }
#endif

#include <InterViews/_leave.h>

#endif
