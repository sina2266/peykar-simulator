/*
 * VCell Entrypoint wrapper for smoldyn
 */

#include "smoldynfuncs.h"
#include <VCELL/GitDescribe.h>
#include <iostream>


/* ***************************************************************** */
/* ********************** main() segment *************************** */
/* ***************************************************************** */


#ifdef USE_MESSAGING
	const char * const Variant = "messaging";
#else
const char *const Variant = "stdout";
#endif

/* main */
int main(int argc, char *argv[]) {
    std::cout << "Smoldyn solver (VirtualCell Edition) version " << Variant << " " << g_GIT_DESCRIBE << std::endl;
    return runSmoldyn(argc, argv);
}