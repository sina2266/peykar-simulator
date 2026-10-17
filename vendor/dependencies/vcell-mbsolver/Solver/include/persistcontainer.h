#ifndef persistcontainer
#define persistcontainer
#include <ostream>
#include <typeinfo>
#include <vector>
#include <type_traits>
#include <persist.h>
/**
* set of functions for persisting from containers. Because addition of elements to containers
* varies, easier to write custom methods for each type
*/
namespace vcell_persist {
//
// Write section
//
	/**
	* default "W" writer which using "persist" for objects of type Persistent
	* and binaryWrite for objects of other types
	*/
	struct persistWrite {
		/**
		* operator for classes which derived from Persistent -- calls U.persist( .. )
		*/
		template <typename U>
		typename std::enable_if<std::is_base_of<Persistent,U>::value,void>::type 
		operator( )(std::ostream &os, const U & u) const {
			u.persist(os);
		}

		/**
		* operator for classes which do not derived from Persistent -- calls binaryWrite 
		*/
		template <typename U>
		typename std::enable_if< ! std::is_base_of<Persistent,U>::value,void>::type 
		operator( )(std::ostream &os, const U & u) const {
			binaryWrite(os,u);
		}
	};

	/**
	* std::string persistence
	* @tparam S unsigned integral type used to store length
	*/
	template <class S>
	struct stringWrite {
		void operator( )(std::ostream &os, const std::string & s) const {
			StdString<S>::save(os,s);
		}
	};

	template <typename W = persistWrite>
	struct writeFunctor {
		std::ostream &os;
		const W & w;
		writeFunctor(std::ostream &os_, const W & w_ = W( ) ) 
			:os(os_),
			w(w_) {}

		template <typename U>
		void operator( )(const U & u) const {
			w(os,u);
		}
	};

//
// Read section; enable_if did not work so well because templated values are return values 
// and more difficult for the compiler to deduce. Therefore we explicitly name the two types
// and use SFINAE later in the restore methods
//
	/**
	* generator which returns new object from stream; supports
	* classes with constructor of form U(std::istream &is)
	*/
	template <class U>
	struct persistReadPersistentType {
		U generate(std::istream &is) const {
			return U(is);
		}
	};

	/**
	* generator which returns new object from stream; supports
	* binaryReads 
	*/
	template <class U>
	struct persistReadBinaryType {
		U generate(std::istream &is) const {
			U u;
			binaryRead(is,u);
			return u;
		}
	};

	/**
	* std::string persistence
	* @tparam S unsigned integral type used to store length
	*/
	template<typename S>
	struct persistReadString {
		std::string generate(std::istream &is) const {
			return StdString<S>::restore(is);
		}
	};

	/**
	* for each compatible function object
	* @tparam R generator; must implement U generate(std::istream &is) 
	*/
	template <typename R>
	struct readFunctor {
		std::istream &is;
		const R & r;
		readFunctor(std::istream &is_, const R & r_ = R( )) 
			:is(is_),
			r(r_) {}

		/**
		* operator for classes which derived from Persistent -- calls U(istream &)
		*/
		template <typename U>
		void operator( )(U &u) const {
			u = r.generate(is);
		}
	};

	/**
	* vector insertion function object
	* @tparam R generator; must implement U generate(std::istream &is) 
	*/
	template <typename R>
	struct InsertFrom {
		const R & r;
		InsertFrom(const R & r_ = R( ))
			:r(r_) {}

		template <typename U>
		void operator ( )(std::istream &is, std::vector<U> &vec)  const {
			//vec.push_back( r.generate<U>(is) );
			vec.push_back( r.generate(is) );
		}
	};

//--------------------------------
// Vector
//--------------------------------
	/**
	* write vector of T (Persistent or binary) 
	* @ tparam T type to restore
	* @ tparam W type which implements void operator(const T &t) to
	* write t to os
	*/
	template<typename T, typename W>
	void save(std::ostream &os,  const std::vector<T> & vec, const W & w) {
		binaryWrite(os, vec.size( ));
		std::for_each(vec.begin( ), vec.end( ),w);
	}

	/**
	* write vector of T (Persistent or binary) using writeFunctor<persistWrite>
	* @ tparam T type to restore
	*/
	template<typename T>
	typename std::enable_if< ! std::is_same<std::string,T>::value,void>::type 
	save(std::ostream &os,  const std::vector<T> & vec) {
		save(os,vec,writeFunctor<>(os) );
	}
	/**
	* std::string persistence
	* @tparam S unsigned integral type used to store length
	*/
	template <typename S>
	void saveStrings(std::ostream &os,  const std::vector<std::string> & vec) {
		typedef stringWrite<S> StringWriteFunctor;
		save(os,vec,writeFunctor<StringWriteFunctor>(os, StringWriteFunctor( ) ) );
	}

	/**
	* restore vector of T (Persistent or binary) 
	* @ tparam T type to restore
	* @ tparam I inserter which implements I(std::istream &is, std::vector<U> &vec)  to insert
	* new object from stream into vector (e.g. InsertFrom)
	*/
	template<typename T, class I>
	void restore(std::istream &is, std::vector<T> & vec, const I & inserter) {
		typedef typename std::vector<T>::size_type Stype;
		Stype size; 
		binaryRead(is,size);

		vec.clear( );
		vec.reserve(size);
		for (Stype i = 0; i < size; ++i) {
			inserter(is,vec);
		}
	}

	/**
	* restore vector of Persistent T  using InsertFrom 
	* @ tparam T type to restore
	*/
	template<typename T>
	typename std::enable_if< std::is_base_of<Persistent,T>::value,void>::type 
	restore(std::istream &is, std::vector<T> & vec) {
		restore(is,vec,InsertFrom<persistReadPersistentType<T> >( ));
	}

	/**
	* restore vector of binary readable T using InsertFrom 
	* @ tparam T type to restore
	*/
	template<typename T>
	typename std::enable_if< ! std::is_base_of<Persistent,T>::value,void>::type 
	restore(std::istream &is, std::vector<T> & vec) {
		static_assert( ! std::is_same<std::string,T>::value, "use restoreStrings for std::string");
		restore(is,vec,InsertFrom<persistReadBinaryType<T> >( ));
	}

	/**
	* std::string persistence
	* @tparam S unsigned integral type used to store length
	*/
	template<typename S>
	void restoreStrings(std::istream &is, std::vector<std::string> & vec) {
		restore(is,vec,InsertFrom<persistReadString<S> >( ));
	}

	
//--------------------------------
// array 
//--------------------------------
	/**
	* write array of T (Persistent or binary) 
	* @ tparam T array type
	* @ tparam n array size 
	* @ tparam W type which implements void operator(const T &t) to
    *	write t to stream
	*/
	template<typename T, size_t N, typename W>
	void save(std::ostream &os,  const std::array<T,N> & arr, const W & w) {
		std::for_each(arr.begin( ), arr.end( ),w);
	}

	/**
	* write array of T (Persistent or binary) 
	* @ tparam T array type
	* @ tparam n array size 
	*/
	template<typename T, size_t N>
	void save(std::ostream &os,  const std::array<T,N> & arr) {
		save(os,arr,writeFunctor<>(os));
	}

	/**
	* restore array of T (Persistent or binary) 
	* @ tparam T array type
	* @ tparam n array size 
	* @ tparam R type which implements void operator(T &t) to
    * set t from stream	
	*/
	template<typename T, size_t N, typename R>
	void restore(std::istream &is, std::array<T,N> & arr, const R &r) {
		std::for_each(arr.begin( ), arr.end( ),r);
	}

	/**
	* restore array of Persistent T using readFunctor 
	* @ tparam T array type
	* @ tparam n array size 
	*/
	template<typename T, size_t N>
	typename std::enable_if< std::is_base_of<Persistent,T>::value,void>::type 
	restore(std::istream &is, std::array<T,N> & arr) {
		restore(is,arr,readFunctor<persistReadPersistentType<T> >(is));
	}

	/**
	* restore array of binary readable T using readFunctor 
	* @ tparam T array type
	* @ tparam n array size 
	*/
	template<typename T, size_t N>
	typename std::enable_if< ! std::is_base_of<Persistent,T>::value,void>::type 
	restore(std::istream &is, std::array<T,N> & arr) {
		restore(is,arr,readFunctor<persistReadBinaryType<T> >(is));
	}
}	
#endif
