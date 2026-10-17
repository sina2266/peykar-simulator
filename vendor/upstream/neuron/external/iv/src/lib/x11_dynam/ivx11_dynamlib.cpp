extern "C" {

/* all X11 structures, declarations, etc. */
#define XUTIL_DEFINE_FUNCTIONS
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* just the pointer declarations corresponding to all extern X globals. */
#define IVX11EXTERN extern
#include <IV-X11/ivx11_define.h>

void ivx11_assign() {
  /* assign the X globals to the pointers */
  #include <IV-X11/ivx11_assign.h>
}

} /* extern "C" */
