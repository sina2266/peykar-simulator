#ifndef vcGccCompat_h
#define vcGccCompat_h
#pragma warning ( disable : 4067 )
#if __GNUC__ == 4 and __GNUC_MINOR__ < 8
	#define nullptr 0
#endif
#endif
