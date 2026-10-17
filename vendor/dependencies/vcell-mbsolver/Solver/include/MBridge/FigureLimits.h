#ifndef FigureLimits_h
#define FigureLimits_h
#include <iostream>
namespace matlabBridge {
	/**
	* limits of a Figure. Allow multiple clients to add their boundaries so that a final figure can be created without the
	* matlab scale having to shift
	*/
	template <typename T>
	struct FigureLimits {
		FigureLimits( )
			:minX_( ),
			minY_( ),
			maxX_( ),
			maxY_( ) {}

		FigureLimits(T minX, T minY, T maxX, T maxY)
			:minX_(minX),
			minY_(minY),
			maxX_(maxX),
			maxY_(maxY) {}

		/**
		* merge other into this; results is largest area (union, if you will)
		* @param m other limits
		*/
		void merge(const FigureLimits &m) {
			minX_ = std::min(minX_,m.minX_);
			minY_ = std::min(minY_,m.minY_);
			maxX_ = std::max(maxX_,m.maxX_);
			maxY_ = std::max(maxY_,m.maxY_);
		}

		T minX( ) const {
			return minX_;
		}

		T minY( ) const {
			return minY_;
		}

		T maxX( ) const {
			return maxX_;
		}

		T maxY( ) const {
			return maxY_;
		}

	private:
		T minX_;
		T minY_;
		T maxX_;
		T maxY_;
	};

	template <typename T>
	inline std::ostream & operator<<(std::ostream &os, const FigureLimits<T> fl) {
		const char comma = ',';
		os << "axis([" << fl.minX( ) << comma <<  fl.maxX( ) << comma 
			<< fl.minY( ) << comma << fl.maxY( ) << "]);" << std::endl;; 
		return os;
	}
};
#endif
