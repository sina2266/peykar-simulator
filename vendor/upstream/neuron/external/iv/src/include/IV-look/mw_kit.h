// =======================================================================
//
// MWkit -- object for creating common UI MS-Windows components
//
// 1.1
// 1997/03/28 22:03:50
//
// InterViews Port to the Windows 3.1/NT operating systems
// Copyright (c) 1993 Tim Prinzing
//
// Permission to use, copy, modify, distribute, and sell this software and 
// its documentation for any purpose is hereby granted without fee, provided
// that (i) the above copyright notice and this permission notice appear in
// all copies of the software and related documentation, and (ii) the name of
// Tim Prinzing may not be used in any advertising or publicity relating to 
// the software without the specific, prior written permission of Tim Prinzing.
// 
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
//
// IN NO EVENT SHALL Tim Prinzing BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
// INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER 
// RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE 
// POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR 
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// =======================================================================
#ifndef ivlook_mwkit_h
#define ivlook_mwkit_h


#include <IV-look/kit.h>
class MWkitImpl;

class MWkit : public WidgetKit 
{
public:
    MWkit();
    virtual ~MWkit();

    virtual const char* gui() const;

public:	// --------------- buttons ------------------------

    virtual Button* push_button(const char*, Action*) const;
    virtual Button* push_button(const String&, Action*) const;

    virtual Button* radio_button(TelltaleGroup*, const char*, Action*) const;
    virtual Button* radio_button(TelltaleGroup*, const String&, Action*) const;

    virtual Button* check_box(const char*, Action*) const;
    virtual Button* check_box(const String&, Action*) const;


public:	// -------------- menus ----------------------------

    virtual Menu* menubar() const;
    virtual Menu* pulldown() const;
    virtual Menu* pullright() const;

    virtual MenuItem* menubar_item(const char*) const;
    virtual MenuItem* menubar_item(const String&) const;
    virtual MenuItem* menubar_item(Glyph*) const;

    virtual MenuItem* menu_item(const char*) const;
    virtual MenuItem* menu_item(const String&) const;
    virtual MenuItem* menu_item(Glyph*) const;

    virtual MenuItem* check_menu_item(const char*) const;
    virtual MenuItem* check_menu_item(const String&) const;
    virtual MenuItem* check_menu_item(Glyph*) const;

    virtual MenuItem* radio_menu_item(TelltaleGroup*, const char*) const;
    virtual MenuItem* radio_menu_item(TelltaleGroup*, const String&) const;
    virtual MenuItem* radio_menu_item(TelltaleGroup*, Glyph*) const;

    virtual MenuItem* menu_item_separator() const;


public:	// -------------- scrolling ----------------------------

    virtual Glyph* hscroll_bar(Adjustable*) const;
    virtual Glyph* vscroll_bar(Adjustable*) const;

public:
	//
	// items currently without mswin-specific implementation
	//

    virtual void style_changed(Style*);

    virtual MonoGlyph* outset_frame(Glyph*) const;
    virtual MonoGlyph* inset_frame(Glyph*) const;
    virtual MonoGlyph* bright_inset_frame(Glyph*) const;

    virtual Glyph* menubar_look() const;
    virtual Glyph* pulldown_look() const;
    virtual Glyph* menubar_item_look(Glyph*, TelltaleState*) const;
    virtual Glyph* menu_item_look(Glyph*, TelltaleState*) const;
    virtual Glyph* check_menu_item_look(Glyph*, TelltaleState*) const;
    virtual Glyph* radio_menu_item_look(Glyph*, TelltaleState*) const;
    virtual Glyph* menu_item_separator_look() const;

    virtual Glyph* push_button_look(Glyph*, TelltaleState*) const;
    virtual Glyph* default_button_look(Glyph*, TelltaleState*) const;
    virtual Glyph* check_box_look(Glyph*, TelltaleState*) const;
    virtual Glyph* palette_button_look(Glyph*, TelltaleState*) const;
    virtual Glyph* radio_button_look(Glyph*, TelltaleState*) const;

    virtual Glyph* slider_look(DimensionName, Adjustable*) const;
    virtual Glyph* scroll_bar_look(DimensionName, Adjustable*) const;
    virtual Glyph* panner_look(Adjustable*, Adjustable*) const;

    virtual Glyph* enlarger_look(TelltaleState*) const;
    virtual Glyph* reducer_look(TelltaleState*) const;
    virtual Glyph* up_mover_look(TelltaleState*) const;
    virtual Glyph* down_mover_look(TelltaleState*) const;
    virtual Glyph* left_mover_look(TelltaleState*) const;
    virtual Glyph* right_mover_look(TelltaleState*) const;
private:
    MWkitImpl* impl_;
};

#endif /* ivlook_mwkit_h */
