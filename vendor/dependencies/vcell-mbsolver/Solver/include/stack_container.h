// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


//gcw simplified 7/23
#ifndef BASE_CONTAINERS_STACK_CONTAINER_H_
#define BASE_CONTAINERS_STACK_CONTAINER_H_

#include <string>
#include <vector>

namespace chromium { //gcw change to chromium

// This allocator can be used with STL containers to provide a stack buffer
// from which to allocate memory and overflows onto the heap. This stack buffer
// would be allocated on the stack and allows us to avoid heap operations in
// some situations.
//
// STL likes to make copies of allocators, so the allocator itself can't hold
// the data. Instead, we make the creator responsible for creating a
// StackAllocator::Source which contains the data. Copying the allocator
// merely copies the pointer to this shared source, so all allocators created
// based on our allocator will share the same stack buffer.
//
// This stack buffer implementation is very simple. The first allocation that
// fits in the stack buffer will use the stack buffer. Any subsequent
// allocations will not use the stack buffer, even if there is unused room.
// This makes it appropriate for array-like containers, but the caller should
// be sure to reserve() in the container up to the stack buffer size. Otherwise
// the container will allocate a small array which will "use up" the stack
// buffer.
template<typename T, size_t stack_capacity>
class StackAllocator : public std::allocator<T> {
	typedef StackAllocator<T,stack_capacity> OurType;
 public:
  typedef typename std::allocator<T>::pointer pointer;
  typedef typename std::allocator<T>::size_type size_type;

  // Backing store for the allocator. The container owner is responsible for
  // maintaining this for as long as any containers using this allocator are
  // live.
  struct Source {
    Source() : used_stack_buffer_(false) {
    }

    // Casts the buffer in its right type.
    T* stack_buffer() { return reinterpret_cast<T *>(stack_buffer_); } //gcw
    const T* stack_buffer() const {
		OurType *us = const_cast<OurType *>(this); //gcw
	    return  us->stack_buffer( );
    }

    // The buffer itself. It is not of type T because we don't want the
    // constructors and destructors to be automatically called. Define a POD
    // buffer of the right size instead.
   char stack_buffer_[sizeof(T) * stack_capacity];
#if defined(__GNUC__) && !defined(ARCH_CPU_X86_FAMILY)
//    COMPILE_ASSERT(ALIGNOF(T) <= 16, crbug_115612);
#endif

    // Set when the stack buffer is used for an allocation. We do not track
    // how much of the buffer is used, only that somebody is using it.
    bool used_stack_buffer_;
  };

  // Used by containers when they want to refer to an allocator of type U.
  template<typename U>
  struct rebind {
    typedef StackAllocator<U, stack_capacity> other;
  };

  // For the straight up copy c-tor, we can share storage.
  StackAllocator(const StackAllocator<T, stack_capacity>& rhs)
      : std::allocator<T>(), source_(rhs.source_) {
  }

  // ISO C++ requires the following constructor to be defined,
  // and std::vector in VC++2008SP1 Release fails with an error
  // in the class _Container_base_aux_alloc_real (from <xutility>)
  // if the constructor does not exist.
  // For this constructor, we cannot share storage; there's
  // no guarantee that the Source buffer of Ts is large enough
  // for Us.
  // TODO: If we were fancy pants, perhaps we could share storage
  // iff sizeof(T) == sizeof(U).
  template<typename U, size_t other_capacity>
  StackAllocator(const StackAllocator<U, other_capacity>& other)
      : source_(NULL) {
  }

  explicit StackAllocator(Source* source) : source_(source) {
  }

  // Actually do the allocation. Use the stack buffer if nobody has used it yet
  // and the size requested fits. Otherwise, fall through to the standard
  // allocator.
  pointer allocate(size_type n, void* hint = 0) {
    if (source_ != NULL && !source_->used_stack_buffer_
        && n <= stack_capacity) {
      source_->used_stack_buffer_ = true;
      return source_->stack_buffer();
    } else {
      return std::allocator<T>::allocate(n, hint);
    }
  }

  // Free: when trying to free the stack buffer, just mark it as free. For
  // non-stack-buffer pointers, just fall though to the standard allocator.
  void deallocate(pointer p, size_type n) {
    if (source_ != NULL && p == source_->stack_buffer())
      source_->used_stack_buffer_ = false;
    else
      std::allocator<T>::deallocate(p, n);
  }

 private:
  Source* source_;
};
}

#endif  // BASE_CONTAINERS_STACK_CONTAINER_H_
