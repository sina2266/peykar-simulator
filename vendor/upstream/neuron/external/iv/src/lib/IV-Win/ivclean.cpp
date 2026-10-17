#include <../../config.h>

/*
Copyright (C) 2002 Michael Hines
This file contains programs and data originally developed by Michael Hines

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/*
ivclean.cpp is probably obsolete now. It was needed for mswin 3.1
on exit to avoid leaking of windows resources.
*/

#include <IV-Win/MWlib.h>
#include <InterViews/bitmap.h>
#include <InterViews/color.h>
#include <InterViews/cursor.h>
#include <InterViews/session.h>
#include <InterViews/display.h>
#include <InterViews/geometry.h>
#include <InterViews/layout.h>
#include <InterViews/window.h>
#include <InterViews/style.h>
#include <IV-look/kit.h>
#include <IV-Win/cursor.h>
#include <IV-Win/bitmap.h>
#include <IV-Win/window.h>
#include <IV-Win/MWapp.h>
#include <OS/list.h>

#include <stdio.h>

extern void ivcleanup_add_window(Window*);
extern void ivcleanup_remove_window(Window*);
extern void ivcleanup_after_window(Window*);

declarePtrList(DWList, Window)
implementPtrList(DWList, Window)
static DWList dwl_;
static DWList after_list_;

extern "C" {
void ivcleanup();
int iv_windows_exist();
}
//extern void cleanup_new_fnt();

int iv_windows_exist() {
	return (dwl_.count() + after_list_.count())  ? 1 : 0;
}

void ivcleanup()
{
return;
//	  MessageBox(NULL, "ivcleanup()", "ivclean.cpp", MB_OK);
	  while (dwl_.count()) {
		Window* w = dwl_.item(0);
		delete w;
	  }
	  while (after_list_.count()) {
		Window* w = after_list_.item(0);
		delete w;
	  }

//	 delete arrow;
//	delete crosshairs;       // worked for a few days then complaints from bchk
	 delete ltextCursor;
	 delete rtextCursor;
//	 delete hourglass;
//	 delete upperleft;
//	 delete upperright;
//	delete lowerleft;
//	 delete lowerright;
	 delete noCursor;
	delete WidgetKit::instance();
	delete LayoutKit::instance();
	delete Session::instance();
	//	cleanup_new_fnt();
}

void ivcleanup_add_window(Window* w) {
//return;
//MessageBox(NULL, "ivcleanup_add_window", "xxx", MB_OK);
	dwl_.append(w);
}

void ivcleanup_remove_window(Window* w) {
//return;
	long i, cnt = dwl_.count();
	for (i=0; i < cnt; ++i) {
		if (w == dwl_.item(i)) {
//MessageBox(NULL, "ivcleanup_remove_window - removed", "xxx", MB_OK);
			dwl_.remove(i);
			return;
		}
	}
	cnt = after_list_.count();
	for (i=0; i < cnt; ++i) {
		if (w == after_list_.item(i)) {
			after_list_.remove(i);
			return;
		}
	}
}
void ivcleanup_after_window(Window* w) {
//return;
	// sometimes a window references this window. So if this happens
	// to be deleted first then when the referencing window deletes
	// it, then error. This semi-prevents this from happening.
	// It would be better if Window was a Resource but that is
	// a serious modification to InterViews.
	ivcleanup_remove_window(w);
	after_list_.append(w);
}

static Extension* ext;

void iv_rescale_map() {
	Display& d = *Session::instance()->default_display();
	long i, cnt = dwl_.count();
	RECT r;
	Window* w;
//printf("iv_rescale_map %d\n", cnt);
	for (i=0; i < cnt; ++i) {
		w = dwl_.item(i);
		long dummy;
		if (w->style() && w->style()->find_attribute("nrn_virtual_screen", dummy)) {
			GetWindowRect(dwl_.item(i)->rep()->msWindow(), &r);
			w->resize();
			MoveWindow(w->rep()->msWindow(), r.left, r.top,
				r.right - r.left, r.bottom - r.top, TRUE);
		}else{
			r.left = d.to_pixels(ext[i].left(), Dimension_X);
			r.right = d.to_pixels(ext[i].right(), Dimension_X);
			r.top = d.to_pixels(ext[i].top(), Dimension_Y);
			r.bottom = d.to_pixels(ext[i].bottom(), Dimension_Y);
			MoveWindow(w->rep()->msWindow(), r.left, r.top,
				r.right - r.left, r.bottom - r.top, TRUE);

		}
	}
	delete [] ext;
}

void iv_rescale_unmap() {
	Display& d = *Session::instance()->default_display();
	long i, cnt = dwl_.count();
//printf("iv_rescale_unmap %d\n", cnt);
	ext = new Extension[cnt];
	RECT r;
	for (i=0; i < cnt; ++i) {
		GetWindowRect(dwl_.item(i)->rep()->msWindow(), &r);
		ext[i].set_xy(nil,
			d.to_coord(r.left, Dimension_X),
			d.to_coord(r.bottom, Dimension_Y),
			d.to_coord(r.right, Dimension_X),
			d.to_coord(r.top, Dimension_Y)
		);
	}
}
