#include <World.h>
#include <persistcontainer.h>
using moving_boundary::Universe;
using spatial::GeoLimit;
/********************************************************************
* Universe 
********************************************************************/
template <int N>
Universe<N>::Universe( )
	:inputLimits( ),
	diagonal_( ),
	initialized(false) {}

template <int N>
Universe<N> & Universe<N>::get( ) {
	static Universe<N> w;
	return w;
}

template <int N>
void Universe<N>::init(std::array<GeoLimit,N> &iValues, std::array<CountType, N> numNodes) {
	nodeNumbers = numNodes;
	typedef moving_boundary::CoordinateType CoordType;
	//validate state 
	if (initialized) {
		VCELL_EXCEPTION(logic_error, "Universe<REAL, " << N << "> already initialized");
	}
	double diagonalScratch = 0; 
	for (int i = 0; i < N; i++) {
		const GeoLimit & gl = iValues[i];
		assert(gl.low( ) <= gl.high( ));
		inputZeroPoint[i] = (gl.low( ) + gl.high( ))/2;
		const double delta = gl.span( );
		diagonalScratch += delta * delta; 
	}
	diagonal_ = sqrt(diagonalScratch);
	inputLimits = iValues;
	initialized = true;
	WorldBase<N> *wb = worlds;
	while (wb != nullptr) {
		wb->init( );
		wb = wb->nextWorld;
	}
}
template <int N>
void Universe<N>::persist(std::ostream &os) const {
	vcell_persist::Token::insert<Universe<N> >(os); 
	vcell_persist::save(os,inputLimits);
	vcell_persist::save(os,nodeNumbers);
	vcell_persist::save(os,inputZeroPoint);
	vcell_persist::binaryWrite(os,diagonal_);
	vcell_persist::binaryWrite(os,initialized);
	//due to the complexity of generating factory methods to
	//restore any type of World, we're going to shift that
	//responsibility to the client
}

template <int N>
void Universe<N>::restore(std::istream &is) {
	vcell_persist::Token::check<Universe<N> >(is); 
	vcell_persist::restore(is,inputLimits);
	vcell_persist::restore(is,nodeNumbers);
	vcell_persist::restore(is,inputZeroPoint);
	vcell_persist::binaryRead(is,diagonal_);
	vcell_persist::binaryRead(is,initialized);
	//due to the complexity of generating factory methods to
	//restore any type of World, we're going to shift that
	//responsibility to the client
}

template <int N>
void Universe<N>::destroy( ) {

	for (int i = 0; i < N; i++) {
		inputLimits[i] = GeoLimit(0,0);
	}
	initialized = false;
	WorldBase<N> *wb = worlds;
	while (wb != nullptr) {
		wb->destroy( );
		wb = wb->nextWorld;
	}
}

template <int N>
bool Universe<N>::locked( ) const {
	return initialized; 
}
template <int N>
void Universe<N>::registerType( ) {
	vcell_persist::Registrar::reg<Universe<N>,N>("Universe");
}


template struct moving_boundary::Universe<2>;
template struct moving_boundary::Universe<3>;
