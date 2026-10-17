/* If libX11 is not available, an Application can still work without it.
 * Everything works normally if it is available.
 */
 
#ifndef ivx11_dynam_h
#define ivx11_dynam_h

extern "C" {

#if defined(IVX11_DYNAM)
/** @brief dlopen libivx11dynam.so and call its ivx11_assign.
 *  The library must be in the same directory as the shared library
 *  that contains the address of ivx11_dyload.
 *  Return 0 if libivx11dynam is successfully loaded.
 *  InterViews must be built with the cmake option
 *  -DIV_ENABLE_X11_DYNAMIC=ON
 */
int ivx11_dyload();
#endif

}
#endif
