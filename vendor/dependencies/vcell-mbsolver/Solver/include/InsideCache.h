#ifndef InsideCache_h
#define InsideCache_h
#include <MPoint.h>
#include <flex.h>
#include <algo.h>
#include <boost/logic/tribool.hpp>
namespace spatial {

	/**
	* cache determination of whether MPoints are inside a given front
	* @tparam CFrontType type of front that's passed in (most likely std::vector of TPoint)
	* @tparam CoordinateType MPoint coordinate type, should be type compatable with CFrontType
	*/
	template <class CFrontType, typename CoordinateType>
	struct InsideCache {
		InsideCache( )
			:front(nullptr),
			storage( )
		{
			resetCachedValues( );
		}

		/**
		* set maximum indexes used by MPoints
		*/
		void setIndexes(size_t maxX, size_t maxY) {
			storage.reindex(maxX,maxY);
			resetCachedValues( );
		}

		/**
		* set current front (clears cached values)
		*/
		void setFront(const CFrontType &front_) {
			front = &front_;
			resetCachedValues( );
		}

		/**
		* determine if point inside last set front
		* @returns true if is, false otherwise
		*/
		bool inside(const spatial::MPoint<CoordinateType,2> & point) const {
			const size_t xIndex = point.indexOf(0);
			const size_t yIndex = point.indexOf(1);
			tribool & cachedValue = storage[xIndex][yIndex];
			if (indeterminate(cachedValue)) {
				cachedValue = spatial::inside<typename CFrontType::value_type>(*front,point);
			}
			return (bool)cachedValue;
		}

		/**
		* see if any values are set
		* @return true if there are
		*/
		bool noValuesSet( ) const {
			for (auto iter = storage.begin( ); iter != storage.end( ) ; ++iter) {
				if (!indeterminate(*iter)) {
					return false;
				}
			}
			return true;
		}

	private:
		typedef boost::logic::tribool tribool;
		static tribool unsetValue( ) {
			return boost::indeterminate;
		}

		void resetCachedValues( ) {
			storage.reset(unsetValue( ));
		}

		const CFrontType *front;
		mutable vcell_util::Flex2<tribool> storage;
	};

}
#endif
