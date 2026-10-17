#ifndef vhd5_flex_h
#define vhd5_flex_h
#include <cassert>
#include <vector>
namespace vcell_util {


	template <class T, typename sizeType> struct Flex2Col;

	/**
	*
	* provide flexible interpretation of static buffer for writing
	* different size blocks to HDF5
	* e.g.  vcell_util::Flex2<double> buffer(3,4);
	* buffer[2][3] = 4;
	* dataset.write(buffer.ptr( ),doublePointType,memoryspace,dataspace);
	*
	*  vcell_util::Flex3<double> data(3,4,5);
	*  @tparam T stored type
	*  @tparam sizeType unsigned intergal type used for specifying sizes 
	*/
#pragma warning ( disable : 4351 )
	template <class T, typename sizeType = size_t>
	struct Flex2 {
		Flex2(sizeType columnSize, sizeType rowSize)
			:storage(columnSize* rowSize ),
			colSize_(columnSize),
			rowSize_(rowSize) {}

		/**
		* allow construction with same array used to create DataSpace
		* array must have size >= 2
		*/
		Flex2(sizeType *dims)
			:storage(dims[0] * dims[1]),
			colSize_(dims[0]),
			rowSize_(dims[1]) {}
		/**
		* default ctor -- must call reindex before use
		*/
		Flex2( )
			:storage(0),
			colSize_(0),
			rowSize_(0) {}

		sizeType rowSize( ) const {
			return rowSize_;
		}
		sizeType columnSize( ) const {
			return colSize_;
		}

		Flex2Col<T,sizeType> operator[](sizeType column) {
			assert(column < colSize_);
			return Flex2Col<T,sizeType>(*this,column);
		}
		const Flex2Col<T,sizeType> operator[](sizeType column) const {
			Flex2<T,sizeType> & us = const_cast<Flex2<T,sizeType> & >(*this); 
			return Flex2Col<T,sizeType>(us,column);
		}
		const void *ptr( ) const {
			return storage.data( );
		}
		/**
		* reconfigure to difference layout; previous values no
		* longer accessible using previous indexing scheme
		*/
		Flex2 & reindex(sizeType columnSize, sizeType rowSize) {
			colSize_ = columnSize;
			rowSize_ = rowSize;
			storage.resize(columnSize * rowSize);
			return *this;
		}
		/**
		* allow setting with same array used to create DataSpace
		* array must have size >= 2
		* reconfigure to difference layout; previous values no
		* longer accessible using previous indexing scheme
		*/
		Flex2 & reindex(sizeType *dims) {
			return reindex( dims[0],dims[1]);
		}
		/**
		* rest all values to default constructor value
		*/
		void reset( ) {
			std::fill(storage.begin( ),storage.end( ),T( ));
		}
		/**
		* rest all values to value
		* @param value value to ste to
		*/
		void reset(const T & value) {
			std::fill(storage.begin( ),storage.end( ),value);
		}
		typename std::vector<T>::const_iterator begin( ) const {
			return storage.begin( );
		}
		typename std::vector<T>::const_iterator end( ) const {
			return storage.end( );
		}
	private:
		friend Flex2Col<T,sizeType>;
		std::vector<T> storage;
		sizeType colSize_;
		sizeType rowSize_;
	};

	template <class T, typename sizeType>
	struct Flex2Col {
		T & operator[](sizeType r) {
			assert(r < flex.rowSize( ));
			const sizeType index = col * flex.rowSize( ) + r; 
			return flex.storage[index];
		}
		const T & operator[](sizeType r) const {
			assert(r < flex.rowSize( ));
			const sizeType index = col * flex.rowSize( ) + r; 
			return flex.storage[index];
		}

	private:
		friend Flex2<T, sizeType>;
		Flex2Col(Flex2<T,sizeType> & parent, sizeType c)
			:flex(parent),
			col(c) {}
		Flex2<T,sizeType> & flex;
		const sizeType col;
	};

// ------------------------------------------------	
// 3 dimensional 
// ------------------------------------------------	
	template <class T, typename sizeType> struct Flex3Depth;
	template <class T, typename sizeType> struct Flex3Col;

#pragma warning ( disable : 4351 )
	template <class T, typename sizeType = size_t>
	struct Flex3 {
		Flex3(sizeType depthSize, sizeType columnSize, sizeType rowSize)
			:storage(depthSize * columnSize* rowSize ),
			depthSize_(depthSize),
			colSize_(columnSize),
			rowSize_(rowSize), 
			depthBlock(colSize_ * rowSize_) {}

		/**
		* allow construction with same array used to create DataSpace
		* array must have size >= 3
		*/
		Flex3(sizeType *dims)
			:storage(dims[0] * dims[1] * dims[2]),
			depthSize_(dims[0]),
			colSize_(dims[1]),
			rowSize_(dims[2]),
			depthBlock(colSize_ * rowSize_) {}

		/**
		* default ctor -- must call reindex before use
		*/
		Flex3( )
			:storage(0),
			depthSize_(0),
			colSize_(0),
			rowSize_(0),
			depthBlock(0) {}

		sizeType rowSize( ) const {
			return rowSize_;
		}
		sizeType columnSize( ) const {
			return colSize_;
		}
		sizeType depthSize( ) const {
			return depthSize_;
		}

		Flex3Depth<T,sizeType> operator[](sizeType depth) {
			assert(depth < depthSize_);
			return Flex3Depth<T,sizeType>(*this,depth);
		}
		const Flex3Depth<T,sizeType> operator[](sizeType depth) const {
			assert(depth < depthSize_);
			Flex3<T,sizeType> & us = const_cast<Flex3<T,sizeType> & > (*this);
	
			return Flex3Depth<T,sizeType>(us,depth);
		}
		const void *ptr( ) const {
			return storage.data( );
		}
		/**
		* reconfigure to difference layout; previous values no
		* longer accessible using previous indexing scheme
		*/
		Flex3 & reindex(sizeType depthSize, sizeType columnSize, sizeType rowSize) {
			depthSize_ = depthSize;
			colSize_ = columnSize;
			rowSize_ = rowSize;
			depthBlock = columnSize * rowSize;
			storage.resize(depthSize * depthBlock);
			return *this;
		}
		/*
		* allow setting with same array used to create DataSpace
		* array must have size >= 2
		* reconfigure to difference layout; previous values no
		* longer accessible using previous indexing scheme
		*/
		Flex3 & reindex(sizeType *dims) {
			return reindex(dims[0],dims[1],dims[2]);
		}
		/**
		* rest all values to default constructor value
		*/
		void reset( ) {
			std::fill(storage.begin( ),storage.end( ),T( ));
		}
	private:
		friend Flex3Col<T,sizeType>;
		std::vector<T> storage;
		sizeType depthSize_;
		sizeType colSize_;
		sizeType rowSize_;
		sizeType depthBlock;
	};

	template <class T, typename sizeType>
	struct Flex3Depth {
		Flex3Col<T,sizeType> operator[](sizeType c) {
			return Flex3Col<T,sizeType>(flex,depth,c);
		}
		const Flex3Col<T,sizeType> operator[](sizeType c) const {
			return Flex3Col<T,sizeType>(flex,depth,c);
		}
  private:
		friend Flex3<T,sizeType>;
		Flex3Depth(Flex3<T,sizeType> & parent, sizeType d)
			:flex(parent),
			depth(d) {}
		Flex3<T,sizeType> & flex;
		const sizeType depth;
	};

	template <class T, typename sizeType>
	struct Flex3Col {
		T & operator[](sizeType r) {
			assert(r < flex.rowSize( ));
			const sizeType index = depth * flex.depthBlock + col * flex.rowSize( ) + r; 
			return flex.storage[index];
		}
		const T & operator[](sizeType r) const {
			assert(r < flex.rowSize( ));
			const sizeType index = depth * flex.depthBlock + col * flex.rowSize( ) + r; 
			return flex.storage[index];
		}

	private:
		friend Flex3<T,sizeType>;
		friend Flex3Depth<T,sizeType>;
		Flex3Col(Flex3<T,sizeType> & parent, sizeType d, sizeType c)
			:flex(parent),
			depth(d),
			col(c) {}
		Flex3<T,sizeType> & flex;
		const sizeType depth;
		const sizeType col;
	};

}
#endif
