#ifndef ManagedArrayPtr_h
#define ManagedArrayPtr_h
#include "nullptr.h" //temporary, pending compiler update
/**
* smart pointer for arrays which manages size
*/
template <class T> class ManagedArrayPtr {
		size_t size;
		T * ptr;
		ManagedArrayPtr(const ManagedArrayPtr &);
		void operator =(const ManagedArrayPtr &);
public:
	/**
	* construct with array of size n
	* @param n size of new array
	*/
	ManagedArrayPtr(size_t n)
		:size(n),
		ptr(new T[size])
	{}
	/**
	* assume ownership of existing  array
	* @param n size of new array
	* @param existing heap allocated array 
	*/
	ManagedArrayPtr(size_t n, T * existing)
		:size(n),
		ptr(existing)
	{}
	/**
	* delete managed array
	*/
	~ManagedArrayPtr( ) { delete[] ptr; }
	/**
	* access array
	* @return managed array
	*/
	T * get( ) const { return ptr; }

	/**
	* access array element, modifiable
	* @param i index
	*/
	T & operator[](size_t i) const {
		assert(i < size);
		return ptr[i];
	}


	/**
	* release ownership of managed array. Future
	* references will be invalid 
	* @return managed array */
	T * release( ) const { 
		T * temp = ptr;
		ptr = nullptr;
		return temp; 
	}

	/**
	* ensure array has minimum size
	* deletes existing data if array too small
	*@param min new minimum size
	*/
	void minimumSize(size_t min) {
		if (min > size) {
			size = min;
			delete[] ptr;
			ptr = new T[size];
		}
	}
};

/**
* array proxy which allocates off stack 
* if size at runtime greater than N falls back to heap allocating
* @parm T type of array
* @parm N stack size 
*/
template <class T, int N> class StackPtr {
		size_t size_;
		T stack[N];
		T * heap;
		T *current;
		//copy ctor
		StackPtr(const StackPtr &);
		void operator = (const StackPtr &);
public:
	/**
	* construct with array of size n
	* @param n size of new array
	*/
	StackPtr(size_t n)
		:size_(n),
		//stack( ), leave uninitialized to match behavior if heap allocated
		heap(n > N ? new T[size_] : nullptr),
		current(n > N ? heap : stack)
	{}
	/**
	* delete managed array, if heap allocated
	*/
	~StackPtr( ) { delete[] heap; }
	/**
	* access array
	* @return managed array
	*/
	T * get( ) const { return current; }

	/**
	* access array element, modifiable
	* @param i index
	*/
	T & operator[](size_t i) const {
		assert(i < size_);
		return current[i];
	}

	bool onStack( ) const {
		return current == stack;
	}

	size_t size( ) const {
		return size_;
	}
};

#endif
