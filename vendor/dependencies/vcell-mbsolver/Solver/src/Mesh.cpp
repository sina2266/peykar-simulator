#include <cstdint>
#include <stdexcept>
#include <limits>
#pragma warning ( disable: 4018 )
#include <Mesh.h>
#include <VCellException.h>

//using namespace spatial;
template <class REAL, int N> 
std::vector<REAL> spatial::MeshDef<REAL,N>::coordinateValues(spatial::Axis a) const {
	const size_t np = nPoints[a];
	const REAL step = intervals[a];
	REAL v = origin[a] + step / 2;
	std::vector<REAL> rval ( np ); 
	for (int i = 0; i < np; i++, v += step) {
		rval[i] = v;
	}
	return rval;
}

#ifndef NDEBUG
template <typename T> 
void spatial::MeshPosition::check( ) const {
	if (index > std::numeric_limits<T>::max( ) ) {
		VCELL_EXCEPTION(out_of_range, "index value " << index <<  " greater max permitted by type " //<< typeid(T).name( )
			<< ", " << std::numeric_limits<T>::max( ) );
	}
}
#endif



/**
* template instantiation
*/
namespace spatial {
	template struct MeshDef<double,2>; 
	template struct MeshDef<int16_t,2>; 
	template struct MeshDef<int32_t,2>; 
	template struct MeshDef<int64_t,2>; 

	template unsigned int MeshPosition::to<unsigned int>( ) const;
	template size_t MeshPosition::to<size_t>( ) const;
	template int MeshPosition::to<int>( ) const;
}
