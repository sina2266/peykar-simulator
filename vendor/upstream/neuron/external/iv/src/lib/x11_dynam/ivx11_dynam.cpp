extern "C" {

/* All X11 structures, declarations, etc. */
/* Do not know if this is needed here. */
#define XUTIL_DEFINE_FUNCTIONS

/* X11 structures, #define, and pointer definitions corresponding
 * to all extern X globals.
*/
#include <IV-X11/ivx11_declare.h>
#define IVX11EXTERN /**/
#include <IV-X11/ivx11_define.h>

}

#include <stdio.h>
#include <dlfcn.h>
#include <string>
#include <IV-X11/ivx11_dynam.h>

static void (*p_ivx11_assign)();

/** @brief dlopen libivx11dynam.so and call its ivx11_assign.
 *  The library must be in the same directory as the shared library
 *  that contains the address of ivx11_dyload.
 */
int ivx11_dyload() { // return 0 on success
  /* only load once */
  if (p_ivx11_assign) {
    return 0;
  }

  /* see if ivx11_assign already loaded and if so use that */
  p_ivx11_assign = (void(*)())dlsym(RTLD_DEFAULT, "ivx11_assign");
  if (p_ivx11_assign) {
    (*p_ivx11_assign)();
    return 0;
  }
  /* dynamically load libivx11dynam.so and call its ivx11_assign() */

  /* figure out path of libivx11dynam.so
   * Assumes that library is in the same location as the library containing
   * this function.
   */
  Dl_info info;
  int rval = dladdr((void*)ivx11_dyload, &info);
  std::string name;
  if (rval) {
    if (info.dli_fname) {
      name = info.dli_fname;
      if (info.dli_fname[0] == '/') { // likely full path
        // dlopen this with RTLD_GLOBAL to make sure the dlopen of libivx11dynam
        // will get its externs resolved (needed when launch python).
        if (!dlopen(name.c_str(), RTLD_NOW | RTLD_NOLOAD | RTLD_GLOBAL)) {
          printf("%s: RTLD_GLOBAL for %s\n", dlerror(), name.c_str());
          return -1;
        }

        /* From the last '/' to the next '.' gets replaced by libivx11dynam */
        size_t last_slash = name.rfind("/");
        size_t dot = name.find(".", last_slash);
        if (dot == std::string::npos) {
            printf("Can't determine the basename (last '/' to next '.') in \"%s\"\n", name.c_str());
            return -1;
        }
        size_t len = dot - (last_slash + 1);
        name.replace(last_slash+1, len, "libivx11dynam"); // keeps the .so or .dylib
      }else{
        printf("Not a full path \"%s\"\n", name.c_str());
        return -1;
      }
    }else{
      printf("dladdr no DL_info.dli_fname\n");
      return -1;
    }
  }else{
    printf("%s\n", dlerror());
    return -1;
  }

  int flag = RTLD_NOW | RTLD_GLOBAL;
  void* handle = dlopen(name.c_str(), flag);
  if (!handle) {
    //be quiet
    //printf("%s: for %s\n", dlerror(), name.c_str());
    return -1;
  }
  p_ivx11_assign = (void(*)())dlsym(handle, "ivx11_assign");
  if (p_ivx11_assign) {
    (*p_ivx11_assign)();
  }else{
    return -1;
  }
  return 0;
}
