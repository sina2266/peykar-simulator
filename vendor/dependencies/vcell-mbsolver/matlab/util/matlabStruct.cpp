#include <sstream>
#include "matlabStruct.h"
#include "matlabAssert.h"
#include "vcGccCompat.h"

using matlabLink::MatlabStruct;
using matlabLink::MData;

MatlabStruct::MatlabStruct(const mxArray *source_) 
	:source(*source_),
	dblStore( ),
	strStore( ) {
		if (!source_) {
			mexErrMsgTxt("null source to matlab struct");
		}
		if (!mxIsStruct(source_)) {
			mexErrMsgTxt("input not of struct type");
		}
}

void * MatlabStruct::getRequired(const char * const name) const {
	 mxArray * f = mxGetField(&source,0,name);
	 if (f == nullptr) {
		std::stringstream err;
		err << "Required field " << name << " not found" << std::ends;
		mexErrMsgTxt(err.str( ).c_str( ));
		return nullptr;
	 }
	 return mxGetData(f);
}

MatlabStruct::ArrayData MatlabStruct::get(const char * const name, bool required,
							const char * const type, validateFunction vf) const {
							
	ArrayData ad;
	ad.marray = mxGetField(&source,0,name);
	if (required && ad.marray == nullptr) {
		std::stringstream err;
		err << "Required field " << name << " not found" << std::ends;
		mexErrMsgTxt(err.str( ).c_str( ));
	}
	if (ad.marray == 0) {
		return ad;
	}
	if (!vf(ad.marray)) {
		std::stringstream err;
		err << "Field " << name << " not correct type " << type << ", is " << mxGetClassName(ad.marray) << std::ends; 
		mexErrMsgTxt(err.str( ).c_str( ));
	}
	const mwSize nDim = mxGetNumberOfDimensions(ad.marray);
	if (nDim != 2) {
		std::stringstream err;
		err << "Field " << name << " should be 2 dimensional, " << nDim << " detected " << std::ends; 
		mexErrMsgTxt(err.str( ).c_str( ));
	}
	const mwSize *dims = mxGetDimensions(ad.marray);
	//set size to the dimension that is not 1
	if (dims[0] == 1) {
		ad.size = dims[1];
	}
	else if (dims[1] == 1) {
		ad.size = dims[0];
	}
	else {
		std::stringstream err;
		err << "Field " << name << " should be 1 x n, " << dims[0] << " x " << dims[1] << " detected " << std::ends; 
		mexErrMsgTxt(err.str( ).c_str( ));
	}
	return ad;
}

MData<double> MatlabStruct::doubles(const char * const name, bool required) const {
	std::string key(name);
	DoubleMap::const_iterator iter = dblStore.find(name);
	if (iter != dblStore.end( )) {
		return iter->second;
	}
	ArrayData ad = get(name,required, "double", mxIsDouble);
	double *ptr = nullptr;
	if (ad.size > 0) {
		ML_ASSERT(ad.marray != nullptr);
		ptr = mxGetPr(ad.marray);
	}
	MData<double> md(ad.size, ptr);
	dblStore[key] = md;
	return md;
}

MData<int64_t> MatlabStruct::int64s(const char * const name, bool required) const {
	std::string key(name);
	Int64Map::const_iterator iter = int64Store.find(name);
	if (iter != int64Store.end( )) {
		return iter->second;
	}
	ArrayData ad = get(name,required, "int64", mxIsInt64);
	int64_t *ptr = nullptr;
	if (ad.size > 0) {
		ML_ASSERT(ad.marray != nullptr);
		//the use of the mex type int64_T (capital tee) is intentional -- make compiler verify it's compatible with int64_t (lower case)
		ptr = static_cast<int64_T *>(mxGetData(ad.marray));
	}

	MData<int64_t> md(ad.size, ptr); 
	int64Store[key] = md;
	return md;
}

std::string MatlabStruct::str(const char *const name, bool required) const {
	std::string key(name);
	StringMap::const_iterator iter = strStore.find(name);
	if (iter != strStore.end( )) {
		return iter->second;
	}
	ArrayData ad = get(name,required, "string",mxIsChar);
	if (ad.marray== 0) {
		return std::string( );
	}
	const int blen = 1024;
	char buff[blen];
	const int rcode = mxGetString(ad.marray,buff,blen-1);
	if (rcode != 0) {
		std::stringstream err;
		err << "Error reading " << name << "; verify length < " << (blen-1) << std::ends;
		mexErrMsgTxt(err.str( ).c_str( ));
	}
	std::string value(buff);
	strStore[key] = value;
	return value;
}

bool MatlabStruct::boolean(const char *const name, bool required) const {
	ArrayData ad = get(name,required, "logical",mxIsLogical);
	if (ad.marray == 0) {
		return false; 
	}
	mxLogical * lgcl = mxGetLogicals(ad.marray);
	//mxLogical is bool typedef
	return *lgcl;
}


std::string MatlabStruct::str(const char *const name, const char * const defaultValue) const {
	std::string r = str(name,false);
	if (r.length( ) == 0) {
		return std::string(defaultValue);
	}
	return r;
}


