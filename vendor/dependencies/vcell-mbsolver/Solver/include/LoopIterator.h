#ifndef LoopIterator_h
#define LoopIterator_h
namespace vcell_util {
	/**
	* iterator class which loops continously (circular) around compatable containers (e.g. std::vector and std::deque)
	* if (closed is true, the container is assumed to have same entry in front and back and the back is never accessed
	*/
	template <class CONTAINER>
	struct LoopIterator  {
		LoopIterator(CONTAINER &c, bool closed_ = true)
			:container(c),
			implIter(c.begin( )),
			closed(closed_) {}

		LoopIterator(CONTAINER &c, const typename CONTAINER::iterator &rhs , bool closed_ = true)
			:container(c),
			implIter(rhs),
			closed(closed_) {}
		/*
		LoopIterator(const LoopIterator<CONTAINER> &rhs) 
			:container(rhs.container),
			implIter(rhs.implIter),
			closed(rhs.closed) {}
			*/

		typename CONTAINER::value_type & operator*( ) {
			return *implIter;
		}
		LoopIterator & operator++( ) {
			++implIter;
			if (closed && implIter == container.end( ) - 1) {
				implIter = container.begin( );
			}
			else if (implIter == container.end( )) {
				implIter = container.begin( );
			}
			return *this;
		}
		//postfix
		LoopIterator operator++(int) {
			LoopIterator<CONTAINER> copy(*this);
			++(*this);
			return copy;
		}
		LoopIterator & operator--( ) {
			if (implIter == container.begin( )) {
				implIter = container.end( );
				if (closed) {
					--implIter;
				}
			}
			--implIter;
			return *this;
		}
		//postfix
		LoopIterator operator--(int) {
			LoopIterator<CONTAINER> copy(*this);
			--(*this);
			return copy;
		}
		bool operator==(typename CONTAINER::iterator &rhs) const {
			return implIter == rhs;
		}
		bool operator!=(typename CONTAINER::iterator &rhs) const {
			return !(implIter == rhs);
		}

	private:
		//typename 
		CONTAINER &container;
		typename CONTAINER::iterator implIter; 
		bool closed;
	};

	template <class CONTAINER>
	struct ConstLoopIterator  {
		ConstLoopIterator(const CONTAINER &c, bool closed_ = true)
			:container(c),
			implIter(c.begin( )),
			closed(closed_) {}

		ConstLoopIterator(CONTAINER &c, const typename CONTAINER::iterator &rhs , bool closed_ = true)
			:container(c),
			implIter(rhs),
			closed(closed_) {}
		/*
		ConstLoopIterator(const ConstLoopIterator<CONTAINER> &rhs) 
			:container(rhs.container),
			implIter(rhs.implIter),
			closed(rhs.closed) {}
			*/

		const typename CONTAINER::value_type & operator*( ) {
			return *implIter;
		}

		ConstLoopIterator & operator=(typename CONTAINER::const_iterator &rhs) {
			implIter = rhs;
			return *this;
		}
		ConstLoopIterator & operator++( ) {
			++implIter;
			if (closed && implIter == container.end( ) - 1) {
				implIter = container.begin( );
			}
			else if (implIter == container.end( )) {
				implIter = container.begin( );
			}
			return *this;
		}
		//postfix
		ConstLoopIterator operator++(int) {
			ConstLoopIterator<CONTAINER> copy(*this);
			++(*this);
			return copy;
		}
		ConstLoopIterator & operator--( ) {
			if (implIter == container.begin( )) {
				implIter = container.end( );
				if (closed) {
					--implIter;
				}
			}
			--implIter;
			return *this;
		}
		//postfix
		ConstLoopIterator operator--(int) {
			ConstLoopIterator<CONTAINER> copy(*this);
			--(*this);
			return copy;
		}
		bool operator==(typename CONTAINER::const_iterator &rhs) const {
			return implIter == rhs;
		}
		bool operator!=(typename CONTAINER::const_iterator &rhs) const {
			return !(implIter == rhs);
		}
		bool operator==(ConstLoopIterator<CONTAINER> &rhs) const {
			return implIter == rhs.implIter;
		}
		bool operator!=(ConstLoopIterator<CONTAINER> &rhs) const {
			return !operator==(rhs);
		}

	private:
		const CONTAINER &container;
		typename CONTAINER::const_iterator implIter; 
		bool closed;
	};
}
#endif
