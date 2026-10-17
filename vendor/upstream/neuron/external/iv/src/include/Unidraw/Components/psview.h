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
 * PostScriptView - idraw-compatible PostScript external representation 
 * for graphical components.
 */

#ifndef unidraw_components_psview_h
#define unidraw_components_psview_h

#include <Unidraw/Components/externview.h>

#include <IV-2_6/_enter.h>

class PostScriptView : public PreorderView {
public:
    virtual bool Emit(std::ostream&);
    GraphicComp* GetGraphicComp();

    virtual ClassId GetClassId();
    virtual bool IsA(ClassId);
protected:
    PostScriptView(GraphicComp* = nil);

    virtual void Comments(std::ostream&);

    virtual void PSVersion(std::ostream&);
    virtual void Creator(std::ostream&);
    virtual void FontNames(std::ostream&);
    virtual void Pages(std::ostream&);
    virtual void BoundingBox(std::ostream&);

    virtual void Prologue(std::ostream&);
    virtual void Version(std::ostream&);
    virtual void GridSpacing(std::ostream&);
    virtual void Trailer(std::ostream&);

    virtual void MinGS(std::ostream&);
    virtual void FullGS(std::ostream&);
    virtual void TextGS(std::ostream&);
    virtual void Brush(std::ostream&);
    virtual void FgColor(std::ostream&);
    virtual void BgColor(std::ostream&);
    virtual void Font(std::ostream&);
    virtual void Pattern(std::ostream&);
    virtual void Transformation(std::ostream&);

    virtual void SetPSFonts(UList* = nil);
    virtual UList* GetPSFonts();

    virtual void GetBox(Coord&, Coord&, Coord&, Coord&);
    virtual void GetGridSpacing(float&, float&);

    virtual void ConstProcs(std::ostream&);
    virtual void BeginProc(std::ostream&);
    virtual void EndProc(std::ostream&);

    virtual void SetGSProcs(std::ostream&);
    virtual void SetBrushProc(std::ostream&);
    virtual void SetFgColorProc(std::ostream&);
    virtual void SetBgColorProc(std::ostream&);
    virtual void SetFontProc(std::ostream&);
    virtual void SetPatternProc(std::ostream&);

    virtual void ObjectProcs(std::ostream&);
    virtual void BSplineProc(std::ostream&);
    virtual void CircleProc(std::ostream&);
    virtual void ClosedBSplineProc(std::ostream&);
    virtual void EllipseProc(std::ostream&);
    virtual void LineProc(std::ostream&);
    virtual void MultiLineProc(std::ostream&);
    virtual void PolygonProc(std::ostream&);
    virtual void RectangleProc(std::ostream&);
    virtual void TextProc(std::ostream&);

    virtual void MiscProcs(std::ostream&);
    virtual void DefinitionProc(std::ostream&);
    virtual void FillProc(std::ostream&);
    virtual void StrokeProc(std::ostream&);
    virtual void ShowProc(std::ostream&);
    virtual void PatternProc(std::ostream&);
    virtual void MinMaxProcs(std::ostream&);
    virtual void MidpointProc(std::ostream&);
    virtual void ThirdpointProc(std::ostream&);
    virtual void SubsplineProc(std::ostream&);
    virtual void StoreVerticesProc(std::ostream&);

    PSFont* GetFont(UList*);
    PostScriptView* View(UList*);
    PostScriptView* CreatePSView(GraphicComp*);
private:
    UList* _fonts;
};

class PostScriptViews : public PostScriptView {
public:
    PostScriptViews(GraphicComps* = nil);
    virtual ~PostScriptViews();

    virtual bool Emit(std::ostream&);
    virtual bool Definition(std::ostream&);
    virtual void Update();
    GraphicComps* GetGraphicComps();

    virtual ExternView* GetView(Iterator);
    virtual void SetView(ExternView*, Iterator&);

    virtual void First(Iterator&);
    virtual void Last(Iterator&);
    virtual void Next(Iterator&);
    virtual void Prev(Iterator&);
    virtual bool Done(Iterator);

    virtual ClassId GetClassId();
    virtual bool IsA(ClassId);
protected:
    UList* Elem(Iterator);
    void DeleteView(Iterator&);
    void DeleteViews();
protected:
    UList* _views;
};

#include <IV-2_6/_leave.h>

#endif
