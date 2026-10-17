// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <memory>
#include <iostream>

namespace vcell_util { 

	template<typename T, class SOURCE> 
	class VCellAllocator : public std::allocator<T> {

		typedef VCellAllocator<T,SOURCE> OurType;
	public:
		// std::allocator's nested pointer/size_type were removed in C++20; go
		// through allocator_traits, which still provides them.
		typedef typename std::allocator_traits<std::allocator<T> >::pointer pointer;
		typedef typename std::allocator_traits<std::allocator<T> >::size_type size_type;

		// Used by containers when they want to refer to an allocator of type U.
		template<typename U>
		struct rebind {
			typedef VCellAllocator<U, SOURCE> other;
		};

		// For the straight up copy c-tor, we can share storage.
		VCellAllocator(const VCellAllocator<T, SOURCE>& rhs)
			: std::allocator<T>(), source_(rhs.source_) {
		}

		template<typename U, class OSOURCE >
		VCellAllocator(const VCellAllocator<U, OSOURCE>& other)
			: source_(other.source( )) {
		}

		explicit VCellAllocator(SOURCE* source) : source_(source) {
		}

		// Actually do the allocation. Use the stack buffer if nobody has used it yet
		// and the size requested fits. Otherwise, fall through to the standard
		// allocator.
		pointer allocate(size_type n, void* hint = 0) {
			return static_cast<pointer>(source_->allocate(n, hint));
		}

		// Free: when trying to free the stack buffer, just mark it as free. For
		// non-stack-buffer pointers, just fall though to the standard allocator.
		void deallocate(pointer p, size_type n) {
			source_->free(p,n);
		}

		SOURCE *source( ) const {
			return source_;
		}

	private:
		SOURCE* source_;
	};
}
