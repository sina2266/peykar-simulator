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
 * GraphicComp - a component that has a graphical representation.
 * GraphicComps - class for GraphicComp composition.
 */

#ifndef unidraw_components_grcomp_h
#define unidraw_components_grcomp_h

#include <Unidraw/Components/component.h>

#include <IV-2_6/_enter.h>

class Bitmap;
class Clipboard;
class Connector;
class Editor;
class Graphic;
class Iterator;
class PSBrush;
class PSColor;
class PSFont;
class PSPattern;
class Picture;
class Raster;
class Transformer;

class GraphicComp : public Component {
public:
    virtual ~GraphicComp();

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual Graphic* GetGraphic();
    virtual Component* GetParent();

    virtual GraphicComp* GetComp(Iterator);
    virtual void SetComp(GraphicComp*, Iterator&);
    virtual void Bequeath();

    virtual void Append(GraphicComp*);
    virtual void Prepend(GraphicComp*);
    virtual void InsertBefore(Iterator, GraphicComp*);
    virtual void InsertAfter(Iterator, GraphicComp*);
    virtual void Remove(GraphicComp*);
    virtual void Remove(Iterator&);

    virtual Mobility GetMobility();
    virtual void SetMobility(Mobility);

    virtual ClassId GetClassId();
    virtual bool IsA(ClassId);
protected:
    GraphicComp(Graphic* = nil);

    virtual GraphicComp* GetGraphicComp(Graphic*);
    virtual void SetGraphic(Graphic*);

    void Skip(std::istream&);
    void Mark(std::ostream&);

    int ReadBgFilled(std::istream&);
    PSBrush* ReadBrush(std::istream&);
    PSColor* ReadColor(std::istream&);
    PSFont* ReadFont(std::istream&);
    PSPattern* ReadPattern(std::istream&);
    Transformer* ReadTransformer(std::istream&);
    char* ReadString(std::istream&);
    Bitmap* ReadBitmap(std::istream&);
    Raster* ReadGraymap(std::istream&);
    Raster* ReadRaster(std::istream&);
    void ReadVertices(std::istream&, Coord*&, Coord*&, int&);

    void WriteBgFilled(bool, std::ostream&);
    void WriteBrush(PSBrush*, std::ostream&);
    void WriteColor(PSColor*, std::ostream&);
    void WriteFont(PSFont*, std::ostream&);
    void WritePattern(PSPattern*, std::ostream&);
    void WriteTransformer(Transformer*, std::ostream&);
    void WriteString(const char*, std::ostream&);
    void WriteBitmap(Bitmap*, std::ostream&);
    void WriteGraymap(Raster*, std::ostream&);
    void WriteRaster(Raster*, std::ostream&);
    void WriteVertices(const Coord*, const Coord*, int, std::ostream&);
protected:
    Graphic* _gr;
    static UList* _brushes;
    static UList* _colors;
    static UList* _fonts;
    static UList* _patterns;
};    

class GraphicComps : public GraphicComp {
public:
    GraphicComps();
    GraphicComps(Graphic*);
    virtual ~GraphicComps();

    virtual void Interpret(Command*);
    virtual void Uninterpret(Command*);

    virtual void First(Iterator&);
    virtual void Last(Iterator&);
    virtual void Next(Iterator&);
    virtual void Prev(Iterator&);
    virtual bool Done(Iterator);

    virtual GraphicComp* GetComp(Iterator);
    virtual void SetComp(GraphicComp*, Iterator&);
    virtual void Bequeath();

    virtual void Append(GraphicComp*);
    virtual void Prepend(GraphicComp*);
    virtual void InsertBefore(Iterator, GraphicComp*);
    virtual void InsertAfter(Iterator, GraphicComp*);
    virtual void Remove(GraphicComp*);
    virtual void Remove(Iterator&);

    virtual void SetMobility(Mobility);

    virtual Component* Copy();
    virtual void Read(std::istream&);
    virtual void Write(std::ostream&);
    virtual ClassId GetClassId();
    virtual bool IsA(ClassId);
protected:
    GraphicComp* Comp(UList*);
    UList* Elem(Iterator);

    void SelectViewsOf(GraphicComp*, Editor*);
    void SelectClipboard(Clipboard*, Editor*);

    void StorePosition(GraphicComp*, Command*);
    void RestorePosition(GraphicComp*, Command*);

    void Group(Clipboard*, GraphicComp*, Command*);
    void Ungroup(GraphicComp*, Clipboard*, Command*);
protected:
    UList* _comps;
};

#include <IV-2_6/_leave.h>

#endif
