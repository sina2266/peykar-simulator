#include <MPoint.h>
#include <sstream>
#include <algorithm>
#include <persist.h>

using spatial::IndexInfo;
using spatial::MPoint;
using spatial::MeshElement;
using spatial::SurfacePosition;
using namespace vcell_persist;
namespace {

	/*
	struct Registrar {
		Registrar ( ) {
			//VCELL_PERSIST_REGISTER_MACRO2(spatial::MPoint<double,2>)
			//VCELL_PERSIST_REGISTER_MACRO2(spatial::MeshElement<double,2>)
		}

	};
	*/
}
template<int N>
const std::string & IndexInfo<N>::str( ) const {
	if (str_.length( ) > 0) {
		return str_;
	}
	std::ostringstream oss;
	oss << *this << std::ends;
	str_ = oss.str( );
	return str_;
}
template MPoint<double,2>::MPoint(std::istream &);
template void MeshElement<double,2>::persist(std::ostream &) const;
template MeshElement<double,2>::MeshElement(std::istream &);



template class spatial::IndexInfo<1>;
template class spatial::IndexInfo<2>;
template class spatial::IndexInfo<3>;
