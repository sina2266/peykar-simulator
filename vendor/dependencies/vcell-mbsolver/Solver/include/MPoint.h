#ifndef MPoint_h
#define MPoint_h
#pragma warning ( disable : 4996)
#include <limits>
#include <cassert>
#include <array>
#include <stdexcept>
#include <TPoint.h>
#include <infix_iterator.h>
#include <persistcontainer.h>
namespace spatial {
	//forward
	template <int N> struct ElementOffset; 
	template<int N> class IndexInfo;

	/**
	* indexed point 
	* @tparam REAL coordinated type
	* @tparam N number of dimensions
	*/
	template<class REAL, int N>
	struct MPoint : public TPoint<REAL,N> {
		typedef TPoint<REAL,N> base;

		MPoint(const size_t *n, const REAL *values) {
			for (int i = 0; i < N; i++) {
				MPoint & us = *this;
				//us(static_cast<Axis>(i)) = values[i];
				base::coord[i] = values[i];
				index[i] = n[i];
			}
		}

		MPoint(std::istream &is) 
			:base(is)
		{
			vcell_persist::Token::check<MPoint<REAL,N> >(is); 
			vcell_persist::restore(is,index);
		}

		ElementOffset<N> offset(const MPoint<REAL,N> & other) const {
			return ElementOffset<N>(other.index,index);
		}

		size_t indexOf(int dim) const {
			return index[dim];
		}

		const std::array<size_t,N> & indexes() const {
			return index;
		}

		bool sameIndexes(const MPoint &rhs) const {
			return index == rhs.index;
		}

		IndexInfo<N> indexInfo( ) const {
			return IndexInfo<N>(index);
		}

		TPoint<size_t,N> indexPoint( ) const {
			return TPoint<size_t,N>(index);
		}

		void persist(std::ostream &os) const {
			base::persist(os);
			vcell_persist::Token::insert<MPoint<REAL,N> >(os); 
			vcell_persist::save(os,index);
		}

		static void registerType( ) {
			base::registerType( );
			vcell_persist::Registrar::reg< MPoint<REAL,N>,REAL,N>("MPoint");
		}

	protected:
		std::array<size_t,N> index;
	};

	enum SurfacePosition { unsetPosition = -1, interiorSurface, boundarySurface,outsideSurface};

	template<class REAL, int N>
	struct MeshElement : public MPoint<REAL,N> {
		typedef MPoint<REAL,N> base;
		MeshElement(const size_t *n, const REAL *values) 
			:base(n,values),
			mp(unsetPosition)
		{ }

		/**
		* create from binary stream
		*/
		MeshElement(std::istream &is) 
			:base(is)
		{
			vcell_persist::Token::check<MeshElement<REAL,N> >(is); 
			vcell_persist::binaryRead(is,mp);
		}

		SurfacePosition mPos( ) const {
			return mp;
		}
		/**
		* point is inside boundary (#interiorSurface or #boundarySurface)
		*/
		bool isInside( ) const {
			switch (mPos( )) {
			case interiorSurface:
			case boundarySurface:
				return true;
			case outsideSurface:
				return false;
			default:
				throw std::logic_error("isInside unset position");
			}
		}
		/**
		* point is outside boundary (#outsideSurface) 
		*/
		bool isOutside( ) const {
			return !isInside( );
		}

		/**
		* write this to binary stream
		* MPoint.cpp
		*/
		void persist(std::ostream & os ) const {
			base::persist(os);
			vcell_persist::Token::insert<MeshElement<REAL,N> >(os); 
			vcell_persist::binaryWrite(os,mp);
		}
		static void registerType( ) {
			base::registerType( );
			vcell_persist::Registrar::reg<MeshElement<REAL,N>, REAL,N>("MeshElement");
		}
	protected:
		void setPos(SurfacePosition m)  {
			size_t x = this->index[0];
			size_t y = this->index[1];
			mp = m;
		}
		SurfacePosition mp;
	};

	inline std::ostream & operator<<(std::ostream & os, spatial::SurfacePosition mp) {
		switch (mp) {
		case spatial::interiorSurface:
			os << "interior";
			break;
		case spatial::outsideSurface:
			os << "outside";
			break;
		case spatial::boundarySurface:
			os << "boundary";
		break;
		default:
			assert(0);
		}
		return os;
	}

	/**
	* represent difference between MPoint indexes
	*/
	template <int N>
	struct ElementOffset :public vcell_persist::Persistent {
		typedef signed char OffsetType;
		ElementOffset( )
			:offsets() {}

		template <typename C>
		ElementOffset(const C & lhs, const C & rhs ) {
			typename C::const_iterator lIter = lhs.begin( );
			typename C::const_iterator rIter = rhs.begin( );
			typename std::array<OffsetType,N>::iterator outIter = offsets.begin( );
			while (lIter != lhs.end( )) {
				*outIter = subtract(*lIter,*rIter);
				outIter++;
				lIter++;
				rIter++;
			}
		}

		ElementOffset(std::istream &is) {
			vcell_persist::Token::check<ElementOffset<N> >(is); 
			vcell_persist::restore(is,offsets);
		}

		void persist(std::ostream &os) const {
			vcell_persist::Token::insert<ElementOffset<N> >(os); 
			vcell_persist::save(os,offsets);
		}

		bool allZero( ) const {
			for (OffsetType ot : offsets) {
				if (ot != 0) {
					return false;
				}
			}
			return true;
		}

		static void registerType( ) {
			vcell_persist::Registrar::reg<ElementOffset<N>,N>("ElementOffset");
		}

		std::array<OffsetType,N> offsets;
	private:
		/**
		* @tparam T input type, may be unsigned
		*/
		template <typename T>
		OffsetType subtract(const T & lhs, const T &rhs) {
			if (lhs > rhs) {
				T delta = lhs - rhs; 
				assert(delta < std::numeric_limits<OffsetType>::max( ));
				return static_cast<OffsetType>(delta);
			}
			//else, implied
			T delta = rhs - lhs; 
			assert(delta < std::numeric_limits<OffsetType>::max( ));
			return static_cast<OffsetType>(-1 * delta);
		}
	};

	/**
	* proxy class for pretty printing index information
	*/
	template<int N>
	class IndexInfo {
		const std::array<size_t,N> & index;
		mutable std::string str_; //lazily evaluated
	public:
		IndexInfo(const std::array<size_t,N> & i)
			:index(i),
			str_() {}
		void write(std::ostream &os) {
			os << '[';
			std::copy(index.begin( ), index.end( ),
				infix_ostream_iterator<size_t>(os,",") );
			os << ']';
		}
		const std::string &str( ) const;
	};

	template<int N>
	inline std::ostream & operator<<(std::ostream & os, spatial::IndexInfo<N> ii) {
		ii.write(os);
		return os;
	}

}
#endif
