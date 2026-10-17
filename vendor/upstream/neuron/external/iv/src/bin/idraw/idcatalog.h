/*
 * Copyright (c) 1990, 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Stanford not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Stanford makes no representations about
 * the suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * STANFORD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * IdrawCatalog - can read and write components in traditional idraw 
 * PostScript format.
 */

#ifndef idcatalog_h
#define idcatalog_h

#include <Unidraw/catalog.h>

class GraphicComp;

class IdrawCatalog : public Catalog{
public:
    IdrawCatalog(const char*, Creator*);
    
    virtual bool Save(EditorInfo*, const char*);
    virtual bool Save(Component*, const char*);
    virtual bool Save(Command*, const char*);
    virtual bool Save(Tool*, const char*);

    virtual bool Retrieve(const char*, EditorInfo*&);
    virtual bool Retrieve(const char*, Component*&);
    virtual bool Retrieve(const char*, Command*&);
    virtual bool Retrieve(const char*, Tool*&);
private:
    bool UnidrawFormat(const char*);

    void PSReadGridSpacing(std::istream&, float&, float&);
    void PSReadGS(std::istream&, Graphic*);
    void PSReadPictGS(std::istream&, Graphic*);
    void PSReadTextGS(std::istream&, Graphic*);
    void PSReadBrush(std::istream&, Graphic*);
    void PSReadFgColor(std::istream&, Graphic*);
    void PSReadBgColor(std::istream&, Graphic*);
    void PSReadFont(std::istream&, Graphic*);
    void PSReadPattern(std::istream&, Graphic*);
    void PSReadTransformer(std::istream&, Graphic*);
    void PSReadPoints(std::istream&, const Coord*&, const Coord*&, int&);

    void PSReadChildren(std::istream&, GraphicComp*);
    void PSReadTextData(std::istream&, char*, int);

    GraphicComp* ReadPostScript(std::istream&);
    GraphicComp* ReadPict(std::istream&);
    GraphicComp* ReadBSpline(std::istream&);
    GraphicComp* ReadCircle(std::istream&);
    GraphicComp* ReadClosedBSpline(std::istream&);
    GraphicComp* ReadEllipse(std::istream&);
    GraphicComp* ReadLine(std::istream&);
    GraphicComp* ReadMultiLine(std::istream&);
    GraphicComp* ReadPolygon(std::istream&);
    GraphicComp* ReadRect(std::istream&);
    GraphicComp* ReadText(std::istream&);
    GraphicComp* ReadSStencil(std::istream&);
    GraphicComp* ReadFStencil(std::istream&);
    GraphicComp* ReadRaster(std::istream&);

    void ScaleToScreenCoords(Graphic*);
    float CalcGrayLevel(int);
    void CorrectTextVPos(Graphic*, float);
private:
    static char _buf[CHARBUFSIZE]; // contains storage for reading data
    static float _psversion;       // stores version of drawing read from file
    bool _head, _tail;          // stores arrow state for last GS read
    bool _valid;
};

#endif
