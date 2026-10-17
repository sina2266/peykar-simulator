#include "rSharp.h"

//for logging
#include <fstream>

#define TOPOF(A) CAR(A)
#define POP(A) CDR(A)

using string_t = std::basic_string<char_t>;

const char_t* dotnetlib_path = nullptr;
const auto loadAssemblyDelegate = STR("ClrFacade.ClrFacade+LoadFromDelegate, ClrFacade");
void* create_instance_fn_ptr = nullptr;
void* load_from_fn_ptr = nullptr;
void* call_static_method_fn_ptr = nullptr;
void* call_instance_method_fn_ptr = nullptr;
void* call_free_object_method_ptr = nullptr;
void* get_object_direct_fn_ptr = nullptr;
void* get_full_type_name_fn_ptr = nullptr;
void* create_sexp_wrapper_fn_ptr = nullptr;

//Definition of Delegates
typedef int (CORECLR_DELEGATE_CALLTYPE* CallStaticMethodDelegate)(const char*, const char*, RSharpGenericValue** objects, int num_objects, RSharpGenericValue* returnValue);
typedef int (CORECLR_DELEGATE_CALLTYPE* CallInstanceMethodDelegate)(RSharpGenericValue** instance, const char* mname, RSharpGenericValue** objects, int num_objects, RSharpGenericValue* returnValue);
typedef void (CORECLR_DELEGATE_CALLTYPE* FreeObjectDelegate)(intptr_t instance);
typedef intptr_t(CORECLR_DELEGATE_CALLTYPE* CallFullTypeNameDelegate)(RSharpGenericValue** instance);
typedef int (CORECLR_DELEGATE_CALLTYPE* GetObjectDirectDelegate)(RSharpGenericValue* returnValue);
typedef int (CORECLR_DELEGATE_CALLTYPE* CreateSEXPWrapperDelegate)(intptr_t pointer, RSharpGenericValue* returnValue);
typedef int (CORECLR_DELEGATE_CALLTYPE* CreateInstanceDelegate)(const char*, RSharpGenericValue** objects, int num_objects, RSharpGenericValue* returnValue);
typedef int (CORECLR_DELEGATE_CALLTYPE* LoadFromDelegate)(const char*, RSharpGenericValue* returnValue);

char_t* MergeLibraryPath(const char_t* libraryPath, const char_t* additionalPath);
void freeObject(RSharpGenericValue* instance);
RSharpGenericValue* get_rSharp_generic_value_from_s4(SEXP clrObj);
RSharpGenericValue createInstance(char* ns_qualified_typename, RSharpGenericValue** parameters, const R_len_t numberOfObjects);
RSharpGenericValue getCurrentObjectDirect();

/////////////////////////////////////////
// Functions with R specific constructs, namely SEXPs
/////////////////////////////////////////
SEXP make_int_sexp(int n, int* values) {
	SEXP result;
	long i = 0;
	int* int_ptr;
	PROTECT(result = NEW_INTEGER(n));
	int_ptr = INTEGER_POINTER(result);
	for (i = 0; i < n; i++) {
		int_ptr[i] = values[i];
	}
	UNPROTECT(1);
	return result;
}

/////////////////////////////////////////
// Initialization and disposal of the CLR
/////////////////////////////////////////
SEXP rSharp_create_domain(SEXP args)
{
	try
	{
		args = CDR(args);

		const SEXP element = CAR(args);
		const char* libraryPath = CHAR(STRING_ELT(element, 0));

		// if already loaded in this process, do not load again
		if (load_assembly_and_get_function_pointer != nullptr && dotnetlib_path != nullptr)
		{
			int returnValue = 0;
			return make_int_sexp(1, &returnValue);
		}

		// STEP 1: Load HostFxr and get exported hosting functions
		if (!load_hostfxr())
		{
			assert("Failure: load_hostfxr()");
			throw std::runtime_error("Failure: load_hostfxr()");
		}

#ifdef WINDOWS
		// STEP 2: Initialize and start the .NET Core runtime
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		auto basic_string = converter.from_bytes(libraryPath);
		const char_t* wideStringLibraryPath = basic_string.c_str();
#else
		const char_t* wideStringLibraryPath = libraryPath;
#endif

		char_t* wideStringPath = MergeLibraryPath(wideStringLibraryPath, STR("/RSharp.runtimeconfig.json"));
		dotnetlib_path = MergeLibraryPath(wideStringLibraryPath, STR("/ClrFacade.dll"));

		load_assembly_and_get_function_pointer = nullptr;
		load_assembly_and_get_function_pointer = get_dotnet_load_assembly(wideStringPath);

		if(load_assembly_and_get_function_pointer == nullptr)
		{
			assert("Failure: get_dotnet_load_assembly()");
			throw std::runtime_error("Failure: get_dotnet_load_assembly()");
		}

		delete[] wideStringPath;
	}
	catch (const std::runtime_error& ex)
	{
		error_return(ex.what())
	}

	int returnValue = 0;
	return make_int_sexp(1, &returnValue);

}

#ifdef WINDOWS
wchar_t* MergeLibraryPath(const wchar_t* libraryPath, const wchar_t* additionalPath)
{
	size_t libraryPathLength = wcslen(libraryPath);
	size_t additionalPathLength = wcslen(additionalPath);
	size_t totalLength = libraryPathLength + additionalPathLength + 1; // +1 for null terminator

	wchar_t* mergedPaths = new wchar_t[totalLength];

	wcscpy_s(mergedPaths, totalLength, libraryPath);
	wcscat_s(mergedPaths, totalLength, additionalPath);

	return mergedPaths;
}
#else
char* MergeLibraryPath(const char* libraryPath, const char* additionalPath)
{

	size_t libraryPathLength = strlen(libraryPath);
	size_t additionalPathLength = strlen(additionalPath);
	size_t totalLength = libraryPathLength + additionalPathLength + 1; // +1 for null terminator

	char* mergedPaths = new char[totalLength];

	strcpy(mergedPaths, libraryPath);
	strcat(mergedPaths, additionalPath);

	return mergedPaths;
}
#endif

void rSharp_shutdown_clr()
{
	ms_rSharp_cleanup();
}

void ms_rSharp_cleanup()
{
	close_fptr(cxt);
}

/////////////////////////////////////////
// Helper Functions
/////////////////////////////////////////

static void rsharp_object_finalizer(SEXP clrSexp) {
	RsharpObjectHandle* clroh_ptr;

	if (TYPEOF(clrSexp) == EXTPTRSXP) {
		RSharpGenericValue* objptr;
		clroh_ptr = static_cast<RsharpObjectHandle*> (R_ExternalPtrAddr(clrSexp));
		objptr = clroh_ptr->objptr;
		freeObject(objptr);
		delete objptr;
	}
}

/////////////////////////////////////////
// Function used to load and activate .NET Core
/////////////////////////////////////////
namespace {
	// Forward declarations
	void* load_library(const char_t*);
	void* get_export(void*, const char*);

#ifdef WINDOWS
	void* load_library(const char_t* path)
	{
		HMODULE h = ::LoadLibraryW(path);
		assert(h != nullptr);
		return (void*)h;
	}
	void* get_export(void* h, const char* name)
	{
		void* f = ::GetProcAddress((HMODULE)h, name);
		assert(f != nullptr);
		return f;
	}
#else
	void* load_library(const char_t* path)
	{
		void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		assert(h != nullptr);
		return h;
	}
	void* get_export(void* h, const char* name)
	{
		void* f = dlsym(h, name);
		assert(f != nullptr);
		return f;
	}
#endif

	// <SnippetLoadHostFxr>
   // Using the nethost library, discover the location of hostfxr and get exports
	bool load_hostfxr()
	{
		// Pre-allocate a large buffer for the path to hostfxr
		char_t buffer[MAX_PATH];
		size_t buffer_size = sizeof(buffer) / sizeof(char_t);
		int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
		if (rc != 0)
			return false;

		// Load hostfxr and get desired exports
		void* lib = load_library(buffer);

		init_for_config_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
		get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
		close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

		return (init_for_config_fptr && get_delegate_fptr && close_fptr);
	}
	// </SnippetLoadHostFxr>

	// <SnippetInitialize>
	// Load and initialize .NET Core and get desired function pointer for scenario
	load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* config_path)
	{
		// Load .NET Core
		void* load_assembly_and_get_function_pointer = nullptr;
		cxt = nullptr;
		int rc = init_for_config_fptr(config_path, nullptr, &cxt);
		if (rc != 0 || cxt == nullptr)
		{
			std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
			close_fptr(cxt);
			return nullptr;
		}

		// Get the load assembly function pointer
		rc = get_delegate_fptr(
			cxt,
			hdt_load_assembly_and_get_function_pointer,
			&load_assembly_and_get_function_pointer);
		if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
			std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

		close_fptr(cxt);
		return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
	}
	// </SnippetInitialize>
}

/////////////////////////////////////////
// Functions to initialize the delegates
/////////////////////////////////////////

void initializeCreateInstance()
{
	const char_t* dotnet_type = STR("ClrFacade.ClrFacade, ClrFacade");
	auto functionDelegate = STR("ClrFacade.ClrFacade+CreateInstanceDelegate, ClrFacade");

	int rc_1 = load_assembly_and_get_function_pointer(
		dotnetlib_path,
		dotnet_type,
		STR("CreateInstance"),
		functionDelegate,//delegate_type_name
		nullptr,
		&create_instance_fn_ptr);

	assert(rc_1 == 0 && create_instance_fn_ptr != nullptr && "Failure: CreateInstance()");
}

void initializeLoadAssembly()
{
	const char_t* dotnet_type = STR("ClrFacade.ClrFacade, ClrFacade");
	auto functionDelegate = STR("ClrFacade.ClrFacade+LoadFromDelegate, ClrFacade");

	int rc_1 = load_assembly_and_get_function_pointer(
		dotnetlib_path,
		dotnet_type,
		STR("LoadFrom"),
		loadAssemblyDelegate,//delegate_type_name
		nullptr,
		&load_from_fn_ptr);

	assert(rc_1 == 0 && load_from_fn_ptr != nullptr && "Failure: LoadFrom()");
}

void initializeGetFullTypeNameFunction()
{
	const char_t* dotnet_type = STR("ClrFacade.ClrFacade, ClrFacade");
	auto functionDelegate = STR("ClrFacade.ClrFacade+GetObjectTypeNameDelegate, ClrFacade");

	int rc_1 = load_assembly_and_get_function_pointer(
		dotnetlib_path,
		dotnet_type,
		STR("GetObjectTypeName"),
		functionDelegate,//delegate_type_name
		nullptr,
		&get_full_type_name_fn_ptr);

	assert(rc_1 == 0 && get_full_type_name_fn_ptr != nullptr && "Failure: GetObjectTypeName()");
}

void initializeGetObjectDirectFunction()
{
	const char_t* dotnet_type = STR("ClrFacade.ClrFacade, ClrFacade");
	auto functionDelegate = STR("ClrFacade.ClrFacade+CurrentObjectDelegate, ClrFacade");

	int rc_1 = load_assembly_and_get_function_pointer(
		dotnetlib_path,
		dotnet_type,
		STR("CurrentObject"),
		functionDelegate,//delegate_type_name
		nullptr,
		&get_object_direct_fn_ptr);

	assert(rc_1 == 0 && get_object_direct_fn_ptr != nullptr && "Failure: get_CurrentObject()");
}

void initializeCreateSEXPFunction()
{
	const char_t* dotnet_type = STR("ClrFacade.ClrFacade, ClrFacade");
	auto functionDelegate = STR("ClrFacade.ClrFacade+CreateSexpWrapperDelegate, ClrFacade");

	int rc_1 = load_assembly_and_get_function_pointer(
		dotnetlib_path,
		dotnet_type,
		STR("CreateSexpWrapperLong"),
		functionDelegate,//delegate_type_name
		nullptr,
		&create_sexp_wrapper_fn_ptr);

	assert(rc_1 == 0 && create_sexp_wrapper_fn_ptr != nullptr && "Failure: CreateSexpWrapper()");

}

void initializeCallInstanceFunction()
{
	const char_t* dotnet_type = STR("ClrFacade.ClrFacade, ClrFacade");
	auto functionDelegate = STR("ClrFacade.ClrFacade+CallInstanceMethodDelegate, ClrFacade");

	int rc_1 = load_assembly_and_get_function_pointer(
		dotnetlib_path,
		dotnet_type,
		STR("CallInstanceMethod"),
		functionDelegate,//delegate_type_name
		nullptr,
		&call_instance_method_fn_ptr);

	assert(rc_1 == 0 && call_instance_method_fn_ptr != nullptr && "Failure: CallStatic()");
}

void initializeFreeObjectFunction()
{
	const char_t* dotnet_type = STR("ClrFacade.ClrFacade, ClrFacade");
	auto functionDelegate = STR("ClrFacade.ClrFacade+FreeObjectDelegate, ClrFacade");

	int rc_1 = load_assembly_and_get_function_pointer(
		dotnetlib_path,
		dotnet_type,
		STR("FreeObject"),
		functionDelegate,//delegate_type_name
		nullptr,
		&call_free_object_method_ptr);

	assert(rc_1 == 0 && call_free_object_method_ptr != nullptr && "Failure: FreeObject()");

}

void initializeCallStaticFunction()
{
	const char_t* dotnet_type = STR("ClrFacade.ClrFacade, ClrFacade");
	auto functionDelegate = STR("ClrFacade.ClrFacade+CallStaticMethodDelegate, ClrFacade");

	int rc_1 = load_assembly_and_get_function_pointer(
		dotnetlib_path,
		dotnet_type,
		STR("CallStaticMethod"),
		functionDelegate,//delegate_type_name
		nullptr,
		&call_static_method_fn_ptr);

	assert(rc_1 == 0 && call_static_method_fn_ptr != nullptr && "Failure: CallStatic()");
}

SEXP make_numeric_sexp(int n, double* values) {
	SEXP result;
	long i = 0;
	double* dbl_ptr;
	PROTECT(result = NEW_NUMERIC(n));
	dbl_ptr = NUMERIC_POINTER(result);
	for (i = 0; i < n; i++) {
		dbl_ptr[i] = values[i];
	}
	UNPROTECT(1);
	return result;
}

SEXP make_char_sexp(int n, char** values) {
	SEXP result;
	long i = 0;
	PROTECT(result = NEW_CHARACTER(n));
	for (i = 0; i < n; i++) {
		SET_STRING_ELT(result, i, mkChar((const char*)values[i]));
	}
	UNPROTECT(1);
	return result;
}

SEXP make_char_sexp_one(char* values) {
	return make_char_sexp(1, &values);
}

SEXP make_class_from_numeric(int n, double* values, int numClasses, char** classnames) {
	SEXP result;
	result = make_numeric_sexp(n, values);
	PROTECT(result);
	setAttrib(result, R_ClassSymbol, make_char_sexp(numClasses, classnames));
	UNPROTECT(1);
	return result;
}

SEXP make_bool_sexp(int n, bool* values) {
	SEXP result;
	long i = 0;
	int* intPtr;
	PROTECT(result = NEW_LOGICAL(n));
	intPtr = LOGICAL_POINTER(result);
	for (i = 0; i < n; i++) {
		intPtr[i] =
			// found many tests in the mono codebase such as below. Trying by mimicry then.
			// 		if (*(MonoBoolean *) mono_object_unbox(res)) {
			values[i]; // See http://r2clr.codeplex.com/workitem/34;
	}
	UNPROTECT(1);
	return result;
}

SEXP make_uchar_sexp(int n, unsigned char* values) {
	SEXP result;
	long i = 0;
	Rbyte* bPtr;
	PROTECT(result = NEW_RAW(n));
	bPtr = RAW_POINTER(result);
	for (i = 0; i < n; i++) {
		bPtr[i] = values[i];
	}
	UNPROTECT(1);
	return result;
}

//ToDo: to remove when we use it and replace with just a call to mkString()
SEXP make_char_single_sexp(const char* str) {
	return mkString(str);
}

bool isObjectArray(RSharpGenericValue** parameterArray, int i)
{
	return parameterArray[i]->type == RSharpValueType::OBJECT_ARRAY;
}

void free_params_array(RSharpGenericValue** parameterArray, int size, bool* should_free = nullptr)
{
	for (int i = 0; i < size; i++)
	{
		if (isObjectArray(parameterArray, i))
			free_params_array(reinterpret_cast<RSharpGenericValue**>(parameterArray[i]->value), parameterArray[i]->size);
		else if (should_free != nullptr && should_free[i])
			// Release the .NET-side allocation that was created when this argument was
			// marshalled R -> .NET via the rdotnet wrapper path. Args derived from extptr/s4
			// share their GCHandle with an R-side extptr finalizer and must NOT be freed here.
			freeObject(parameterArray[i]);

		free(parameterArray[i]);
	}
	delete[] parameterArray;
	parameterArray = nullptr;
	delete[] should_free;
	should_free = nullptr;
}

RSharpGenericValue** sexp_to_parameters(SEXP args, bool*& should_free)
{
	int i;
	int nargs = Rf_length(args);
	int lengthArg = 0;
	SEXP el;
	const char* name;

	should_free = nullptr;
	if (nargs == 0) {
		return NULL;
	}

	auto mparams = new RSharpGenericValue * [nargs];
	should_free = new bool[nargs];

	for (i = 0; args != R_NilValue; i++, args = CDR(args)) {
		name = isNull(TAG(args)) ? "" : CHAR(PRINTNAME(TAG(args)));
		el = CAR(args);

		int sexpType = TYPEOF(el);
		mparams[i] = new RSharpGenericValue(ConvertToRSharpGenericValue(el));
		// This call must free the arg iff its RSharpGenericValue holds a freshly-allocated
		// .NET GCHandle (the rdotnet wrapper path). EXTPTR/S4 args share a GCHandle with
		// an R-side extptr finalizer and must NOT be freed here. The scalar/array fallback
		// (use_rdotnet=0) stores raw R-side data pointers, also not freeable as GCHandles.
		// VECSXP (OBJECT_ARRAY) is recursed into separately by free_params_array.
		should_free[i] = (use_rdotnet
		              && sexpType != EXTPTRSXP
		              && sexpType != S4SXP
		              && sexpType != VECSXP);
	}
	return mparams;
}

//##################################################

SEXP r_get_null_reference() {
	return R_MakeExternalPtr(0, R_NilValue, R_NilValue);
}

SEXP r_show_args(SEXP args)
{
	const char* name;
	int i, j, nclass;
	SEXP el, names, klass;
	int nargs, nnames;
	args = CDR(args); /* skip 'name' */

	nargs = Rf_length(args);
	for (i = 0; i < nargs; i++, args = CDR(args)) {
		name =
			isNull(TAG(args)) ? "<unnamed>" : CHAR(PRINTNAME(TAG(args)));
		el = CAR(args);
		Rprintf("[%d] '%s' R type %s, SEXPTYPE=%d\n", i + 1, name, type2char(TYPEOF(el)), TYPEOF(el));
		Rprintf("[%d] '%s' length %d\n", i + 1, name, LENGTH(el));
		names = getAttrib(el, R_NamesSymbol);
		nnames = Rf_length(names);
		Rprintf("[%d] names of length %d\n", i + 1, nnames);
		klass = getAttrib(el, R_ClassSymbol);
		nclass = length(klass);
		for (j = 0; j < nclass; j++) {
			Rprintf("[%d] class '%s'\n", i + 1, CHAR(STRING_ELT(klass, j)));
		}
	}
	return(R_NilValue);
}

SEXP r_get_sexp_type(SEXP par) {
	SEXP p = par, e;
	int typecode;
	p = CDR(p); e = CAR(p);
	typecode = TYPEOF(e);
	return make_int_sexp(1, &typecode);
}

/////////////////////////////////////////
// Calling ClrFacade methods
/////////////////////////////////////////

SEXP rSharp_load_assembly(SEXP args)
{
	args = CDR(args);

	const SEXP element = CAR(args);
	const char* filename = CHAR(STRING_ELT(element, 0));

	if (load_from_fn_ptr == nullptr)
		initializeLoadAssembly();

	const auto load_from = reinterpret_cast<LoadFromDelegate>(load_from_fn_ptr);
	RSharpGenericValue returnValue;
	if (load_from(filename, &returnValue) < 0)
	{
		error_return(reinterpret_cast<char*>(returnValue.value))
	}

	return ConvertToSEXP(returnValue);
}

SEXP r_create_clr_object(SEXP parameters)
{
	SEXP sExpressionParameterStack = parameters;
	RSharpGenericValue** methodParameters;
	bool* paramShouldFree = nullptr;
	SEXP sExpressionMethodParameter;
	RSharpGenericValue return_value;
	char* ns_qualified_typename = NULL;
	sExpressionParameterStack = POP(sExpressionParameterStack); /* skip the first parameter: function name*/
	get_FullTypeName(sExpressionParameterStack, &ns_qualified_typename);
	sExpressionParameterStack = POP(sExpressionParameterStack);
	sExpressionMethodParameter = sExpressionParameterStack;

	try
	{
		methodParameters = sexp_to_parameters(sExpressionMethodParameter, paramShouldFree);
	}
	catch (const std::runtime_error& ex)
	{
		free(ns_qualified_typename);
		error_return(ex.what())
	}
	R_len_t numberOfObjects = Rf_length(sExpressionMethodParameter);

	try
	{
		auto return_value = createInstance(ns_qualified_typename, methodParameters, numberOfObjects);
		free(ns_qualified_typename);
		free_params_array(methodParameters, numberOfObjects, paramShouldFree);
		return ConvertToSEXP(return_value);
	}
	catch (const std::runtime_error& ex)
	{
		free(ns_qualified_typename);
		free_params_array(methodParameters, numberOfObjects, paramShouldFree);
		error_return(ex.what())
	}
}

RSharpGenericValue createInstance(char* ns_qualified_typename, RSharpGenericValue** parameters, const R_len_t numberOfObjects)
{
	RSharpGenericValue returnValue;

	if (create_instance_fn_ptr == nullptr)
		initializeCreateInstance();

	auto create_instance = reinterpret_cast<CreateInstanceDelegate>(create_instance_fn_ptr);

	auto result = create_instance(ns_qualified_typename, parameters, numberOfObjects, &returnValue);

	if (result < 0)
	{
		char* errorMessage = (char*)returnValue.value;
		throw std::runtime_error(errorMessage);
	}

	return returnValue;
}

RSharpGenericValue callInstance(RSharpGenericValue** instance, const char* calledMethodName, char* ns_qualified_typename, RSharpGenericValue** parameters, const R_len_t numberOfObjects)
{
	RSharpGenericValue returnValue;

	if (call_instance_method_fn_ptr == nullptr)
		initializeCallInstanceFunction();

	const auto call_instance = reinterpret_cast<CallInstanceMethodDelegate>(call_instance_method_fn_ptr);

	auto result = call_instance(instance, calledMethodName, parameters, numberOfObjects, &returnValue);

	if (result < 0)
	{
		char* errorMessage = (char*)returnValue.value;
		throw std::runtime_error(errorMessage);
	}

	return returnValue;
}

void freeObject(RSharpGenericValue* instance)
{
	if (call_free_object_method_ptr == nullptr)
		initializeFreeObjectFunction();

	const auto free_object = reinterpret_cast<FreeObjectDelegate>(call_free_object_method_ptr);
	free_object(reinterpret_cast<intptr_t>(instance));
}

RSharpGenericValue callStatic(const char* calledMethodName, char* ns_qualified_typename, RSharpGenericValue** parameters, const R_len_t numberOfObjects)
{
	RSharpGenericValue returnValue;
	if (call_static_method_fn_ptr == nullptr)
		initializeCallStaticFunction();

	const auto call_static = reinterpret_cast<CallStaticMethodDelegate>(call_static_method_fn_ptr);

	auto result = call_static(ns_qualified_typename, calledMethodName, parameters, numberOfObjects, &returnValue);

	if (result < 0)
	{
		char* errorMessage = (char*)returnValue.value;
		throw std::runtime_error(errorMessage);
	}

	return returnValue;
}

RSharpGenericValue rclr_convert_element_rdotnet(SEXP p)
{
	RSharpGenericValue return_value;

	if (create_sexp_wrapper_fn_ptr == nullptr)
		initializeCreateSEXPFunction();

	const auto call_static = reinterpret_cast<CreateSEXPWrapperDelegate>(create_sexp_wrapper_fn_ptr);

	auto result = call_static(reinterpret_cast<intptr_t>(p), &return_value);



	if (result < 0)
		throw std::runtime_error("Error calling get object direct");

	return return_value;
}

SEXP r_get_object_direct()
{
	try
	{
		auto return_value = getCurrentObjectDirect();
		return ConvertToSEXP(return_value);
	}
	catch (const std::runtime_error& ex)
	{
		error_return(ex.what())
	}
}

RSharpGenericValue getCurrentObjectDirect()
{
	RSharpGenericValue returnValue;

	if (get_object_direct_fn_ptr == nullptr)
		initializeGetObjectDirectFunction();

	const auto get_object_direct = reinterpret_cast<GetObjectDirectDelegate>(get_object_direct_fn_ptr);

	auto result = get_object_direct(&returnValue);

	if (result < 0)
	{
		char* errorMessage = (char*)returnValue.value;
		throw std::runtime_error(errorMessage);
	}

	return returnValue;
}

const char* get_type_full_name(RSharpGenericValue** genericValue) {
	char* ns_qualified_typename = NULL;

	if (get_full_type_name_fn_ptr == nullptr)
		initializeGetFullTypeNameFunction();

	const auto call_full_type_name = reinterpret_cast<CallFullTypeNameDelegate>(get_full_type_name_fn_ptr);
	auto result = call_full_type_name(genericValue);

	return (char*)(result);
}

SEXP r_get_typename_externalptr(SEXP parameters)
{
	SEXP sExpressionParameterStack = parameters, sExpressionMethodParameter;

	sExpressionParameterStack = POP(sExpressionParameterStack); /* skip the first parameter: function name*/
	sExpressionMethodParameter = TOPOF(sExpressionParameterStack);
	SEXP sExpressionNameParameter = TOPOF(sExpressionMethodParameter);

	try
	{
		auto return_value = get_type_full_name(reinterpret_cast<RSharpGenericValue**>(sExpressionNameParameter));
		return mkString(return_value);
	}
	catch (const std::runtime_error& ex)
	{
		error_return(ex.what())
	}
}

SEXP rsharp_object_to_SEXP(RSharpGenericValue& objptr) {
	RsharpObjectHandle* clroh_ptr;

	const auto returnPointer = new RSharpGenericValue(objptr);

	if (objptr.type == RSharpValueType::NULL_VALUE)
		return R_NilValue;

	clroh_ptr = static_cast<RsharpObjectHandle*>(malloc(sizeof(RsharpObjectHandle)));
	clroh_ptr->objptr = returnPointer;
	clroh_ptr->handle = 0;
	SEXP result = R_MakeExternalPtr(clroh_ptr, R_NilValue, R_NilValue);
	R_RegisterCFinalizerEx(result, rsharp_object_finalizer, (Rboolean)1/*TRUE*/);
	return result;
}

SEXP r_call_method(SEXP parameters)
{
	SEXP sExpressionParameterStack = parameters, instance, sExpressionParameter;
	const char* methodName = 0;
	RSharpGenericValue** params;
	bool* paramShouldFree = nullptr;

	sExpressionParameter = TOPOF(sExpressionParameterStack);
	auto functionName = CHAR(STRING_ELT(sExpressionParameter, 0));	// should be "r_call_method"
	sExpressionParameterStack = POP(sExpressionParameterStack);

	instance = TOPOF(TOPOF(sExpressionParameterStack));				// object instance is the second SEXP
	sExpressionParameterStack = POP(sExpressionParameterStack);

	sExpressionParameter = TOPOF(sExpressionParameterStack);			// instance method name is the third SEXP
	methodName = CHAR(STRING_ELT(sExpressionParameter, 0));
	sExpressionParameterStack = POP(sExpressionParameterStack);
	try
	{
		params = sexp_to_parameters(sExpressionParameterStack, paramShouldFree);
	}
	catch (const std::runtime_error& ex)
	{
		error_return(ex.what())
	}
	const R_len_t numberOfObjects = Rf_length(sExpressionParameterStack);

	try
	{
		auto return_value = callInstance(reinterpret_cast<RSharpGenericValue**>(instance), methodName, "ClrFacade.ClrFacade,ClrFacade", params, numberOfObjects);
		free_params_array(params, numberOfObjects, paramShouldFree);
		return ConvertToSEXP(return_value);
	}
	catch (const std::runtime_error& ex)
	{
		free_params_array(params, numberOfObjects, paramShouldFree);
		error_return(ex.what())
	}
}

SEXP r_call_static_method(SEXP parameters)
{
	SEXP sExpressionParameterStack = parameters, sExpressionParameter;
	SEXP sExpressionMethodParameter;
	const char* methodName;
	char* ns_qualified_typename = NULL; // My.Namespace.MyClass,MyAssemblyName
	RSharpGenericValue** methodParameters;
	bool* paramShouldFree = nullptr;

	sExpressionParameterStack = POP(sExpressionParameterStack); /* skip the first parameter: function name*/
	get_FullTypeName(sExpressionParameterStack, &ns_qualified_typename);
	sExpressionParameterStack = POP(sExpressionParameterStack);
	sExpressionParameter = TOPOF(sExpressionParameterStack);
	methodName = CHAR(STRING_ELT(sExpressionParameter, 0));
	sExpressionParameterStack = POP(sExpressionParameterStack);
	sExpressionMethodParameter = sExpressionParameterStack;

	try
	{
		methodParameters = sexp_to_parameters(sExpressionMethodParameter, paramShouldFree);
	}
	catch (const std::runtime_error& ex)
	{
		free(ns_qualified_typename);
		error_return(ex.what())
	}
	if (TYPEOF(sExpressionParameter) != STRSXP || LENGTH(sExpressionParameter) != 1)
	{
		free(ns_qualified_typename);
		error_return("r_call_static_method: invalid method name");
	}

	const R_len_t numberOfObjects = Rf_length(sExpressionMethodParameter);
	try
	{
		auto return_value = callStatic(methodName, ns_qualified_typename, methodParameters, numberOfObjects);
		free(ns_qualified_typename);
		free_params_array(methodParameters, numberOfObjects, paramShouldFree);
		return ConvertToSEXP(return_value);
	}
	catch (const std::runtime_error& ex)
	{
		free(ns_qualified_typename);
		free_params_array(methodParameters, numberOfObjects, paramShouldFree);
		error_return(ex.what())
	}
}


template <typename T>
std::vector<T> GetArray(const RSharpGenericValue& value) {
	std::vector<T> result;

	switch (value.type)
	{
	case RSharpValueType::INT:
	case RSharpValueType::FLOAT:
	case RSharpValueType::DOUBLE:
	case RSharpValueType::BOOL: {
		result.reserve(1);
		result.push_back(static_cast<T>(*reinterpret_cast<const T*>(value.value)));
		break;
	}

	case RSharpValueType::STRING: {
		const T strValue = *reinterpret_cast<const T*>(value.value);
		result.push_back(strValue);
		break;
	}
	case RSharpValueType::INT_ARRAY: {
		const T* ptr = reinterpret_cast<const T*>(value.value);
		result.reserve(value.size);  // Reserve space to avoid reallocation
		std::copy(ptr, ptr + value.size, std::back_inserter(result));
		break;
	}
	case RSharpValueType::FLOAT_ARRAY: {
		const T* ptr = reinterpret_cast<const T*>(value.value);
		result.reserve(value.size);
		std::copy(ptr, ptr + value.size, std::back_inserter(result));
		break;
	}
	case RSharpValueType::DOUBLE_ARRAY: {
		const T* ptr = reinterpret_cast<const T*>(value.value);
		result.reserve(value.size);
		std::copy(ptr, ptr + value.size, std::back_inserter(result));
		break;
	}
	case RSharpValueType::BOOL_ARRAY: {
		const T* ptr = reinterpret_cast<const T*>(value.value);
		result.reserve(value.size);
		std::copy(ptr, ptr + value.size, std::back_inserter(result));
		break;
	}
	case RSharpValueType::STRING_ARRAY: {
		const T* ptr = reinterpret_cast<const T*>(value.value);
		result.reserve(value.size);
		std::copy(ptr, ptr + value.size, std::back_inserter(result));
		break;
	}
	case RSharpValueType::OBJECT: {
		// Implement conversion for your custom object type
		// ...

		// Example: Returning an empty vector for unsupported type
		break;
	}
								// Handle other value types as needed
	default:
		break;
	}

	return result;
}

SEXP PackFloatIntoSEXP(float value) {
	SEXP result = PROTECT(Rf_allocVector(REALSXP, 1));
	REAL(result)[0] = static_cast<double>(value);  // Set the first element of the numeric vector
	UNPROTECT(1);
	return result;
}

SEXP PackDoubleIntoSEXP(double value) {
	SEXP result = PROTECT(Rf_allocVector(REALSXP, 1));
	REAL(result)[0] = value;  // Set the first element of the numeric vector
	UNPROTECT(1);
	return result;
}


SEXP ConvertToSEXP(RSharpGenericValue& value) {
	switch (value.type)
	{
	case RSharpValueType::NULL_VALUE: {
		return R_NilValue;
	}
	case RSharpValueType::INTPTR: {
		return reinterpret_cast<SEXP>(value.value);
	}
	case RSharpValueType::INT: {
		int intValue = *reinterpret_cast<const int*>(value.value);
		return make_int_sexp(1, &intValue);
	}
	case RSharpValueType::FLOAT: {
		float floatValue = *reinterpret_cast<const float*>(value.value);
		return PackFloatIntoSEXP(floatValue);
	}
	case RSharpValueType::DOUBLE: {
		double doubleValue = *reinterpret_cast<const double*>(value.value);
		return PackDoubleIntoSEXP(doubleValue);
	}
	case RSharpValueType::BOOL: {
		bool boolValue = *reinterpret_cast<const bool*>(value.value);
		return Rf_ScalarLogical(boolValue);
	}
	case RSharpValueType::STRING: {
		const char* stringValue = (char*)value.value;
		return make_char_single_sexp(stringValue);
	}
								/*case RSharpValueType::INT_ARRAY: {
									std::vector<int> array = GetArray<int>(value);
									int length = static_cast<int>(array.size());
									SEXP result = PROTECT(Rf_allocVector(INTSXP, length));
									std::copy(array.begin(), array.end(), INTEGER(result));
									UNPROTECT(1);
									return result;
								}
								case RSharpValueType::FLOAT_ARRAY: {
									std::vector<float> array = GetArray<float>(value);
									int length = static_cast<int>(array.size());
									SEXP result = PROTECT(Rf_allocVector(REALSXP, length));
									std::copy(array.begin(), array.end(), REAL(result));
									UNPROTECT(1);
									return result;
								}
								case RSharpValueType::DOUBLE_ARRAY: {
									std::vector<double> array = GetArray<double>(value);
									int length = static_cast<int>(array.size());
									SEXP result = PROTECT(Rf_allocVector(REALSXP, length));
									std::copy(array.begin(), array.end(), REAL(result));
									UNPROTECT(1);
									return result;
								}
								case RSharpValueType::BOOL_ARRAY: {
									std::vector<bool> array = GetArray<bool>(value);
									int length = static_cast<int>(array.size());
									SEXP result = PROTECT(Rf_allocVector(LGLSXP, length));
									std::copy(array.begin(), array.end(), LOGICAL(result));
									UNPROTECT(1);
									return result;
								}
								case RSharpValueType::STRING_ARRAY: {
									std::vector<const char*> array = GetArray<const char*>(value);
									int length = static_cast<int>(array.size());
									SEXP result = PROTECT(Rf_allocVector(STRSXP, length));
									for (int i = 0; i < length; ++i) {
										SET_STRING_ELT(result, i, Rf_mkChar(array[i]));
									}
									UNPROTECT(1);
									return result;
								}*/
	case RSharpValueType::OBJECT: {
		return rsharp_object_to_SEXP(value);

	default:
		return R_NilValue; // Returning NULL for unsupported types
	}
	}
}

void get_FullTypeName(SEXP p, char** tname) {
	SEXP e;
	e = CAR(p); /* second is the namespace */
	if (TYPEOF(e) != STRSXP || LENGTH(e) != 1)
		error("get_FullTypeName: cannot parse type name: need a STRSXP of length 1");
	(*tname) = STR_DUP(CHAR(STRING_ELT(e, 0))); // is all this really necessary? I recall trouble if using less, but this still looks over the top.
}

RSHARP_BOOL r_has_class(SEXP s, const char* classname) {
	SEXP klasses;
	R_xlen_t j;
	int n_classes;
	klasses = getAttrib(s, R_ClassSymbol);
	n_classes = length(klasses);
	if (n_classes > 0) {
		for (j = 0; j < n_classes; j++)
			if (strcmp(CHAR(STRING_ELT(klasses, j)), classname) == 0)
				return TRUE_BOOL;
	}
	return FALSE_BOOL;
}


RSharpGenericValue* get_rSharp_generic_value_from_extptr(SEXP a)
{
	auto extptr_ptr = R_ExternalPtrAddr(a);
	if(extptr_ptr == nullptr)
		error("External pointer to unallocated memory was found");
	return ((RsharpObjectHandle*)extptr_ptr)->objptr;
}

RSharpGenericValue* get_rSharp_generic_value_from_s4(SEXP clrObj)
{
	SEXP a, clrobjSlotName;
	SEXP s4classname = getAttrib(clrObj, R_ClassSymbol);
	if (strcmp(CHAR(STRING_ELT(s4classname, 0)), "cobjRef") == 0)
	{
		PROTECT(clrobjSlotName = NEW_CHARACTER(1));
		SET_STRING_ELT(clrobjSlotName, 0, mkChar("clrobj"));
		a = getAttrib(clrObj, clrobjSlotName);
		UNPROTECT(1);
		if (a != NULL && a != R_NilValue && TYPEOF(a) == EXTPTRSXP) {
			return get_rSharp_generic_value_from_extptr(a);
		}
		return NULL;
	}
	error("Incorrect type of S4 Object: Not of type 'cobjRef'");
	return NULL;
}

/////////////////////////////////////////
// Functions without R specific constructs
/////////////////////////////////////////

RSharpGenericValue ConvertArrayToRSharpGenericValue(SEXP s)
{
	RSharpGenericValue result;
	result.type = RSharpValueType::OBJECT;
	result.value = 0;
	result.size = 0; // Default size for non-array types

	switch (TYPEOF(s)) {
	case INTSXP: {
		result.type = RSharpValueType::INT_ARRAY;
		result.value = reinterpret_cast<intptr_t>(INTEGER(s));
		result.size = LENGTH(s);
		break;
	}
	case REALSXP: {
		result.type = RSharpValueType::DOUBLE_ARRAY;
		result.value = reinterpret_cast<intptr_t>(REAL(s));
		result.size = LENGTH(s);
		break;
	}
	case STRSXP: {
		result.type = RSharpValueType::STRING_ARRAY;
		result.value = reinterpret_cast<intptr_t>(CHAR(STRING_ELT(s, 0)));
		result.size = LENGTH(s);
		break;
	}
	case LGLSXP: {
		result.type = RSharpValueType::BOOL_ARRAY;
		result.value = reinterpret_cast<intptr_t>(LOGICAL(s));
		result.size = LENGTH(s);
		break;
	}
	default:
		break;
	}

	return result;
}


RSharpGenericValue ConvertToRSharpGenericValue(SEXP s)
{
	RSharpGenericValue result;

	int typeof = TYPEOF(s);
	switch (typeof)
	{
	case S4SXP:
		return *get_rSharp_generic_value_from_s4(s);
	case EXTPTRSXP:
		return *get_rSharp_generic_value_from_extptr(s);
	case VECSXP:
		result.type = RSharpValueType::OBJECT_ARRAY;
		result.size = LENGTH(s);
		auto sharp_generic_value = new RSharpGenericValue * [result.size];
		result.value = reinterpret_cast<intptr_t>(sharp_generic_value);
		for (int i = 0; i < result.size; i++)
		{
			sharp_generic_value[i] = new RSharpGenericValue(ConvertToRSharpGenericValue(VECTOR_ELT(s, i)));
		}
		return result;
	}

	if (use_rdotnet)
	{
		return rclr_convert_element_rdotnet(s);
	}

	switch (typeof) {
	case INTSXP: {
		result.type = RSharpValueType::INT;
		result.value = reinterpret_cast<intptr_t>(INTEGER(s));
		break;
	}
	case REALSXP: {
		result.type = RSharpValueType::DOUBLE;
		result.value = reinterpret_cast<intptr_t>(REAL(s));
		break;
	}
	case LGLSXP: {
		result.type = RSharpValueType::BOOL;
		result.value = reinterpret_cast<intptr_t>(LOGICAL(s));
		break;
	}
	case STRSXP: {
		result.type = RSharpValueType::STRING;
		result.value = reinterpret_cast<intptr_t>(CHAR(STRING_ELT(s, 0)));
		break;
	}
	default:
		result = *get_rSharp_generic_value_from_s4(s);
		break;
	}

	return result;
}