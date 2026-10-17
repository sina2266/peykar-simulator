#ifndef __RSHARP_H__
#define __RSHARP_H__

/////////////////////////////////////////
// Imports and includes
/////////////////////////////////////////

// we need to define this as of R 4.3.1 to avoid the C3646 compiler error because of the 
// definition of complex in Rext/Complex.h This error should not be present when compiling with gcc.
#define R_LEGACY_RCOMPLEX

#include <fstream>
#include <iostream>
#include <iterator>

#define NETHOST_USE_AS_STATIC
// Provided by the AppHost NuGet package and installed as an SDK pack
#include <nethost.h>

// Header files copied from https://github.com/dotnet/core-setup
#include <coreclr_delegates.h>
#include <hostfxr.h>

// define these to keep using booleans with MS CPP. Feels kludgy, but so long.
typedef bool RSHARP_BOOL;
#define TRUE_BOOL true;
#define FALSE_BOOL false;

#ifdef WINDOWS
#include <codecvt>
#include <Windows.h>

#include <metahost.h>

#define STR(s) L ## s
#define CH(c) L ## c
#define DIR_SEPARATOR L'\\'

#define string_compare wcscmp

#else
#include <dlfcn.h>
#include <limits.h>

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#define string_compare strcmp

#endif
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>
#include <Rversion.h>
#include <R_ext/Callbacks.h>
#include <stdint.h>
#include <vector>

#pragma comment(lib, "mscoree.lib")



#endif


// Enum to represent the type of the value in the struct
enum class RSharpValueType
{
	INT,
	FLOAT,
	DOUBLE,
	BOOL,
	INT_ARRAY,
	FLOAT_ARRAY,
	DOUBLE_ARRAY,
	BOOL_ARRAY,
	STRING,
	STRING_ARRAY,
	OBJECT,
	NULL_VALUE,
	INTPTR,
	OBJECT_ARRAY
};

// Struct to store a generic value along with its type
struct RSharpGenericValue
{
   RSharpValueType type; // the type of the value
   intptr_t value; // Use intptr_t for IntPtr
   size_t size;    // Size in case of an array

   // Constructor for non-array types
   //RSharpGenericValue(RSharpValueType t, intptr_t v, size_t s) : type(t), value(v), size(s) {}

   // Constructor for array types
   //RSharpGenericValue(RSharpValueType t, intptr_t v) : type(t), value(v), size(0) {}
};

typedef struct {
	RSharpGenericValue* objptr;
	uint32_t handle;
} RsharpObjectHandle;

/////////////////////////////////////////
// Exported methods (exported meaning on Windows platform)
/////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
	SEXP r_create_clr_object( SEXP parameters);
	SEXP r_get_null_reference();
	SEXP r_call_static_method(SEXP parameters);
	SEXP r_get_object_direct();
	SEXP r_call_method(SEXP parameters);
	SEXP r_get_typename_externalptr(SEXP parameters);
	SEXP make_char_single_sexp(const char* str);
	SEXP rsharp_object_to_SEXP(RSharpGenericValue& objptr);
	SEXP r_get_sexp_type(SEXP par);
	SEXP r_show_args(SEXP args);

	/**
	 * \brief	Gets a SEXP, bypassing the custom data converters e.g. offered by RDotNetDataConverter.
	 *
	 * \return	a SEXP representing the object handled by the CLR conversion facade, if any.
	 */
	void get_FullTypeName( SEXP p, char ** tname);
	SEXP rSharp_load_assembly(SEXP args);
	SEXP rSharp_create_domain(SEXP libraryPath);
	int use_rdotnet = 0;


#ifdef __cplusplus
} // end of extern "C" block
#endif


RSharpGenericValue ConvertToRSharpGenericValue(SEXP s);
RSharpGenericValue** sexp_to_parameters(SEXP args, bool*& should_free);
SEXP ConvertToSEXP(RSharpGenericValue& value);


#ifndef  __cplusplus
#define STR_DUP strdup
#else
#define _strdup strdup
#define STR_DUP _strdup
#endif

namespace {
	// Globals to hold hostfxr exports
	//runtime_context context;
	hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
	hostfxr_get_runtime_delegate_fn get_delegate_fptr;
	hostfxr_close_fn close_fptr;
	hostfxr_handle cxt; //probably this is the one we need to keep
	
	// Forward declarations
	bool load_hostfxr();
	load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* assembly);
	load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
}

void ms_rSharp_cleanup();

char * bstr_to_c_string(const wchar_t * src);