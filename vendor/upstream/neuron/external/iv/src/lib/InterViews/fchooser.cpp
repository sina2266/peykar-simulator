#ifdef HAVE_CONFIG_H
#include <../../config.h>
#endif
/*
 * Copyright (c) 1991 Stanford University
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

/*
 * FileChooser -- select a file
 */
#ifdef WIN32
#include <windows.h>
#endif
#include <IV-look/choice.h>
#include <IV-look/dialogs.h>
#include <IV-look/fbrowser.h>
#include <IV-look/kit.h>
#include <InterViews/action.h>
#include <InterViews/event.h>
#include <InterViews/font.h>
#include <InterViews/hit.h>
#include <InterViews/input.h>
#include <InterViews/layout.h>
#include <InterViews/scrbox.h>
#include <InterViews/style.h>
#include <InterViews/target.h>
#include <InterViews/session.h>
#include <InterViews/color.h>
#include <InterViews/label.h>
#include <OS/directory.h>
#include <OS/string.h>
#include <stdio.h>

class FileChooserImpl {
private:
    friend class FileChooser;

    String* name_;
    WidgetKit* kit_;
    FileChooser* fchooser_;
    FileBrowser* fbrowser_;
    FieldEditor* editor_;
    FieldEditor* filter_;
    FieldEditor* directory_filter_;
    int* filter_map_;
    Directory* dir_;
    FileChooserAction* action_;
    const String* selected_;
    Style* style_;
    Action* update_;

    void init(FileChooser*, Style*, FileChooserAction*);
    void fcfree();
    void build();
    void clear();
    void load();
    FieldEditor* add_filter(
	Style*,
	const char* pattern_attribute, const char* default_pattern,
	const char* caption_attribute, const char* default_caption,
	Glyph*, FieldEditorAction*
    );
    bool filtered(const String&, FieldEditor*);
    void accept_dir();
    void accept_browser();
    void cancel_browser();
    void accept_editor(FieldEditor*);
    void cancel_editor(FieldEditor*);
    void accept_filter(FieldEditor*);
    bool chdir(const String&);
    static const Color* disable_color();
    bool choose_dir_;
};

declareActionCallback(FileChooserImpl)
implementActionCallback(FileChooserImpl)

declareFieldEditorCallback(FileChooserImpl)
implementFieldEditorCallback(FileChooserImpl)

FileChooser::FileChooser(
    const String& dir, WidgetKit* kit, Style* s, FileChooserAction* a
) : Dialog(nil, s) {
    impl_ = new FileChooserImpl;
    FileChooserImpl& fc = *impl_;
    fc.name_ = new CopyString(dir);
    fc.kit_ = kit;
    fc.init(this, s, a);
    fc.editor_->select(fc.editor_->text()->length());
}

FileChooser::~FileChooser() {
    impl_->fcfree();
    delete impl_;
}

const String* FileChooser::selected() const {
    return impl_->selected_;
}

const String* FileChooser::dir() const {
    return impl_->dir_->path();
}

void FileChooser::reread() {
    FileChooserImpl& fc = *impl_;
    if (!fc.chdir(*fc.dir_->path())) {
	/* should generate an error message */
    }
}

void FileChooser::dismiss(bool accept) {
    Dialog::dismiss(accept);
    FileChooserImpl& fc = *impl_;
    if (fc.action_ != nil) {
	fc.action_->execute(this, accept);
    }
}

/** class FileChooserImpl **/

void FileChooserImpl::init(
    FileChooser* chooser, Style* s, FileChooserAction* a
) {
    fchooser_ = chooser;
    fbrowser_ = nil;
    editor_ = nil;
    filter_ = nil;
    directory_filter_ = nil;
    filter_map_ = nil;
    dir_ = Directory::open(*name_);
    if (dir_ == nil) {
	dir_ = Directory::current();
	/* and what if we can't read the current directory? */
    }
#ifdef WIN32
if (dir_ == nil) {
MessageBox(NULL, "FileChooserImpl::init dir is nil", "Internal error", MB_OK);
}
#endif
    Resource::ref(a);
    action_ = a;
    style_ = new Style(s);
    Resource::ref(style_);
    style_->alias("FileChooser");
    style_->alias("Dialog");
    update_ = new ActionCallback(FileChooserImpl)(
	this, &FileChooserImpl::build
    );
    style_->add_trigger_any(update_);
    choose_dir_ = style_->value_is_on("choose_directory");
    build();
}

void FileChooserImpl::fcfree() {
    delete name_;
    delete dir_;
    delete [] filter_map_;
    Resource::unref(action_);
    style_->remove_trigger_any(update_);
    Resource::unref(style_);
}

void FileChooserImpl::build() {
    WidgetKit& kit = *kit_;
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = style_;
    kit.push_style();
    kit.style(s);
    String caption("");
    s->find_attribute("caption", caption);
    String subcaption("Enter filename:");
    if (choose_dir_) {
    	subcaption = "Enter directory name:";
    }
    s->find_attribute("subcaption", subcaption);
    String open("Open");
    s->find_attribute("open", open);
    String close("Cancel");
    s->find_attribute("cancel", close);
    long rows = 10;
    s->find_attribute("rows", rows);
    const Font* f = kit.font();
    FontBoundingBox bbox;
    f->font_bbox(bbox);
    Coord height = rows * (bbox.ascent() + bbox.descent()) + 1.0;
    Coord width;
    if (!s->find_attribute("width", width)) {
	width = 16 * f->width('m') + 3.0;
    }

    Action* accept = new ActionCallback(FileChooserImpl)(
	this, &FileChooserImpl::accept_browser
    );
    Action* cancel = new ActionCallback(FileChooserImpl)(
	this, &FileChooserImpl::cancel_browser
    );
    if (editor_ == nil) {
	editor_ = DialogKit::instance()->field_editor(
	    *dir_->path(), s,
	    new FieldEditorCallback(FileChooserImpl)(
		this, &FileChooserImpl::accept_editor,
		&FileChooserImpl::cancel_editor
	    )
	);
    }
    // added by ro2m: check for style for default selection
    String defsel("");
    if(s->find_attribute("defaultSelection", defsel)) {
      editor_->field(defsel);
    }

    fbrowser_ = new FileBrowser(kit_, accept, cancel);

    fchooser_->remove_all_input_handlers();
    fchooser_->append_input_handler(editor_);
    fchooser_->append_input_handler(fbrowser_);

    Glyph* g = layout.vbox();
    if (caption.length() > 0) {
	g->append(layout.rmargin(kit.fancy_label(caption), 5.0, fil, 0.0));
    }
    if (subcaption.length() > 0) {
	g->append(layout.rmargin(kit.fancy_label(subcaption), 5.0, fil, 0.0));
    }
    g->append(layout.vglue(5.0, 0.0, 2.0));
    g->append(editor_);
    g->append(layout.vglue(15.0, 0.0, 12.0));
    g->append(
	layout.hbox(
	    layout.vcenter(
		kit.inset_frame(
		    layout.margin(
			layout.natural_span(fbrowser_, width, height), 1.0
		    )
		),
		1.0
	    ),
	    layout.hspace(4.0),
	    kit.vscroll_bar(fbrowser_->adjustable())
	)
    );
    g->append(layout.vspace(15.0));
    if (s->value_is_on("filter")) {
	FieldEditorAction* action = new FieldEditorCallback(FileChooserImpl)(
	    this, &FileChooserImpl::accept_filter, nil
	);
	filter_ = add_filter(
	    s, "filterPattern", "", "filterCaption", "Filter:", g, action
	);
	if (s->value_is_on("directoryFilter")) {
	    directory_filter_ = add_filter(
		s, "directoryFilterPattern", "",
		"directoryFilterCaption", "Directory Filter:", g, action
	    );
	} else {
	    directory_filter_ = nil;
	}
    } else {
	filter_ = nil;
	directory_filter_ = nil;
    }
    g->append(
	layout.hbox(
	    layout.hglue(10.0),
		choose_dir_ ?
	    layout.vcenter(kit.default_button("List Dir", accept))
		:
	    layout.vcenter(kit.default_button(open, accept)),
	    layout.hglue(10.0, 0.0, 5.0),
	    layout.vcenter(kit.push_button(close, cancel)),
	    layout.hglue(10.0)
	)
    );
    if (choose_dir_) {
	    Action* accept_d = new ActionCallback(FileChooserImpl)(
		this, &FileChooserImpl::accept_dir
	    );
	g->append(layout.vglue(5.0, 0.0, 2.0));
    	g->append(layout.hbox(layout.hglue(10.0),layout.vcenter(kit.push_button(open, accept_d)), layout.hglue(10.0)));
    }
    fchooser_->body(
	layout.back(
	    layout.vcenter(kit.outset_frame(layout.margin(g, 5.0)), 1.0),
	    new Target(nil, TargetPrimitiveHit)
	)
    );
    fchooser_->focus(editor_);
    kit.pop_style();
    load();
}

void FileChooserImpl::clear() {
    Browser& b = *fbrowser_;
    b.select(-1);
    GlyphIndex n = b.count();
    for (GlyphIndex i = 0; i < n; i++) {
	b.remove_selectable(0);
	b.remove(0);
    }
}

static const Color* disable_color_;

const Color* FileChooserImpl::disable_color() {
	if (!disable_color_) {
		Style* s = WidgetKit::instance()->style();
		String dc;
		if (s->find_attribute("disable_color", dc)) {
disable_color_ = Color::lookup(Session::instance()->default_display(), dc);
		}
		if (!disable_color_) {
			disable_color_ = new Color(.5,.5,.5,1.);
		}
		Resource::ref(disable_color_);
	}
	return disable_color_;
}

void FileChooserImpl::load() {
    Directory& d = *dir_;
    FileBrowser& b = *fbrowser_;
    WidgetKit& kit = *kit_;
    kit.push_style();
    kit.style(style_);
    const LayoutKit& layout = *LayoutKit::instance();
    int dircount = d.count();
    delete [] filter_map_;
    int* index = new int[dircount];
    filter_map_ = index;
    for (int i = 0; i < dircount; i++) {
	const String& f = *d.name(i);
	bool is_dir = d.is_directory(i);
	if ((is_dir && filtered(f, directory_filter_)) ||
	    (!is_dir && filtered(f, filter_))
	) {
	    Glyph* name;
if (!is_dir && choose_dir_) {
	    name = new Label(f, kit.font(), disable_color());
}else{
	    name = kit.label(f);
}
	    if (is_dir) {
		name = layout.hbox(name, kit.label("/"));
	    }
	    Glyph* label = new Target(
		layout.h_margin(name, 3.0, 0.0, 0.0, 15.0, fil, 0.0),
		TargetPrimitiveHit
	    );
	    TelltaleState* t = new TelltaleState(TelltaleState::is_enabled);
	    b.append_selectable(t);
	    b.append(new ChoiceItem(t, label, kit.bright_inset_frame(label)));
	    if (!is_dir && choose_dir_) {
		t->set(TelltaleState::is_enabled, false);
	    }
	    *index++ = i;
	}
    }
    b.refresh();
    kit.pop_style();
}

FieldEditor* FileChooserImpl::add_filter(
    Style* s,
    const char* pattern_attribute, const char* default_pattern,
    const char* caption_attribute, const char* default_caption,
    Glyph* body, FieldEditorAction* action
) {
    String pattern(default_pattern);
    s->find_attribute(pattern_attribute, pattern);
    String caption(default_caption);
    s->find_attribute(caption_attribute, caption);
    FieldEditor* e = DialogKit::instance()->field_editor(pattern, s, action);
    fchooser_->append_input_handler(e);
    WidgetKit& kit = *kit_;
    LayoutKit& layout = *LayoutKit::instance();
    body->append(
	layout.hbox(
	    layout.vcenter(kit.fancy_label(caption), 0.5),
	    layout.hspace(2.0),
	    layout.vcenter(e, 0.5)
	)
    );
    body->append(layout.vspace(10.0));
    return e;
}

bool FileChooserImpl::filtered(const String& name, FieldEditor* e) {
    if (e == nil) {
	return true;
    }
    const String* s = e->text();
    if (s == nil || s->length() == 0) {
	return true;
    }
    return s == nil || s->length() == 0 || Directory::match(name, *s);
}

void FileChooserImpl::accept_dir() {
	accept_browser();
	if (choose_dir_) {
		selected_ = dir_->path();
		fchooser_->dismiss(true);
	}
}

void FileChooserImpl::accept_browser() {
    int i = int(fbrowser_->selected());
    if (i == -1) {
	accept_editor(editor_);
	return;
    }
    i = filter_map_[i];
    const String& path = *dir_->path();
    const String& name = *dir_->name(i);
    int length = path.length() + name.length();
    char* tmp = new char[length + 1];
    snprintf(
	tmp, length + 1, "%.*s%.*s",
	path.length(), path.string(), name.length(), name.string()
    );
    editor_->field(tmp);
    selected_ = editor_->text();
    if (dir_->is_directory(i)) {
	if (chdir(String(tmp, length))) {
	    editor_->field(*dir_->path());
	    fchooser_->focus(editor_);
	} else {
	    /* should generate an error message */
	}
    } else if (!choose_dir_){
	fchooser_->dismiss(true);
    } else { // get rid of file and move back to directory
	accept_editor(editor_);
    }
    delete [] tmp;
}

void FileChooserImpl::cancel_browser() {
    selected_ = nil;
    fchooser_->dismiss(false);
}

void FileChooserImpl::accept_editor(FieldEditor* e) {
    String* path = Directory::canonical(*e->text());
    e->field(*path);
    if (chdir(*path)) {
	/* chdir has copied the string */
	delete path;
    }else if (choose_dir_) {
    	e->field(e->text()->left(e->text()->rindex('/')));
    	accept_editor(e);
    } else {
	selected_ = path;
	fchooser_->dismiss(true);
	e->select(path->rindex('/') + 1, path->length());
    }
}

void FileChooserImpl::cancel_editor(FieldEditor*) {
    fchooser_->dismiss(false);
}

void FileChooserImpl::accept_filter(FieldEditor*) {
    clear();
    load();
}

bool FileChooserImpl::chdir(const String& name) {
    Directory* d = Directory::open(name);
    if (d != nil) {
	dir_->close();
	delete dir_;
	dir_ = d;
	clear();
	load();
	return true;
    }
    return false;
}

/** class FileChooserAction **/

FileChooserAction::FileChooserAction() { }
FileChooserAction::~FileChooserAction() { }
void FileChooserAction::execute(FileChooser*, bool) { }
