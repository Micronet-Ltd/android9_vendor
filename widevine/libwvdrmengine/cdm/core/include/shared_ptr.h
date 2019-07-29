// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
// from google3/util/gtl/shared_ptr.h
// from protobuf/src/google/protobuf/stubs/shared_ptr.h

#ifndef WVCDM_CORE_SHARED_PTR_H__
#define WVCDM_CORE_SHARED_PTR_H__

#include <algorithm>  // for swap
#include <stddef.h>
#include <memory>

#include "lock.h"
#include "wv_cdm_types.h"

namespace wvcdm {

extern Lock shared_ptr_ref_count_lock_;

}  // namespace wvcdm

namespace {

bool Barrier_AtomicIncrement(volatile uint32_t* ptr, uint32_t value) {
  *ptr += value;
  return *ptr;
}
bool NoBarrier_AtomicIncrement(volatile uint32_t* ptr, uint32_t value) {
  *ptr += value;
  return *ptr;
}

inline bool RefCountDec(volatile uint32_t *ptr) {
  wvcdm::AutoLock auto_lock(wvcdm::shared_ptr_ref_count_lock_);
  return Barrier_AtomicIncrement(ptr, -1) != 0;
}

inline void RefCountInc(volatile uint32_t *ptr) {
  wvcdm::AutoLock auto_lock(wvcdm::shared_ptr_ref_count_lock_);
  NoBarrier_AtomicIncrement(ptr, 1);
}

}  // namespace

namespace wvcdm {

template <typename T> class shared_ptr;

// This class is an internal implementation detail for shared_ptr. If two
// shared_ptrs point to the same object, they also share a control block.
// An "empty" shared_pointer refers to NULL and also has a NULL control block.
// It contains all of the state that's needed for reference counting or any
// other kind of resource management. In this implementation the control block
// consists of a single reference count (the number of shared_ptrs that
// share ownership of the object).
class SharedPtrControlBlock {
  template <typename T> friend class shared_ptr;
 private:
  SharedPtrControlBlock() : refcount_(1) {}
  uint32_t refcount_;
};

// Forward declaration. The class is defined below.
template <typename T> class enable_shared_from_this;

template <typename T>
class shared_ptr {
 public:
  typedef T element_type;

  shared_ptr() : ptr_(NULL), control_block_(NULL) {}

  explicit shared_ptr(T* ptr)
      : ptr_(ptr),
        control_block_(ptr != NULL ? new SharedPtrControlBlock : NULL) {
  }

  // Copy constructor: makes this object a copy of ptr, and increments
  // the reference count.
  template <typename U>
  shared_ptr(const shared_ptr<U>& ptr)
      : ptr_(NULL),
        control_block_(NULL) {
    Initialize(ptr);
  }
  // Need non-templated version to prevent the compiler-generated default
  shared_ptr(const shared_ptr<T>& ptr)
      : ptr_(NULL),
        control_block_(NULL) {
    Initialize(ptr);
  }

  // Assignment operator. Replaces the existing shared_ptr with ptr.
  // Increment ptr's reference count and decrement the one being replaced.
  template <typename U>
  shared_ptr<T>& operator=(const shared_ptr<U>& ptr) {
    if (ptr_ != ptr.ptr_) {
      shared_ptr<T> me(ptr);   // will hold our previous state to be destroyed.
      swap(me);
    }
    return *this;
  }

  // Need non-templated version to prevent the compiler-generated default
  shared_ptr<T>& operator=(const shared_ptr<T>& ptr) {
    if (ptr_ != ptr.ptr_) {
      shared_ptr<T> me(ptr);   // will hold our previous state to be destroyed.
      swap(me);
    }
    return *this;
  }

  ~shared_ptr() {
    if (ptr_ != NULL) {
      if (!RefCountDec(&control_block_->refcount_)) {
        delete ptr_;
      }
    }
  }

  // Replaces underlying raw pointer with the one passed in.  The reference
  // count is set to one (or zero if the pointer is NULL) for the pointer
  // being passed in and decremented for the one being replaced.
  //
  // If you have a compilation error with this code, make sure you aren't
  // passing NULL, nullptr, or 0 to this function.  Call reset without an
  // argument to reset to a null ptr.
  template <typename Y>
  void reset(Y* p) {
    if (p != ptr_) {
      shared_ptr<T> tmp(p);
      tmp.swap(*this);
    }
  }

  void reset() {
    reset(static_cast<T*>(NULL));
  }

  // Exchanges the contents of this with the contents of r.  This function
  // supports more efficient swapping since it eliminates the need for a
  // temporary shared_ptr object.
  void swap(shared_ptr<T>& r) {
    using std::swap;  // http://go/using-std-swap
    swap(ptr_, r.ptr_);
    swap(control_block_, r.control_block_);
  }

  // The following function is useful for gaining access to the underlying
  // pointer when a shared_ptr remains in scope so the reference-count is
  // known to be > 0 (e.g. for parameter passing).
  T* get() const {
    return ptr_;
  }

  T& operator*() const {
    return *ptr_;
  }

  T* operator->() const {
    return ptr_;
  }

  long use_count() const {
    return control_block_ ? control_block_->refcount_ : 1;
  }

  bool unique() const {
    return use_count() == 1;
  }

 private:
  // If r is non-empty, initialize *this to share ownership with r,
  // increasing the underlying reference count.
  // If r is empty, *this remains empty.
  // Requires: this is empty, namely this->ptr_ == NULL.
  template <typename U>
  void Initialize(const shared_ptr<U>& r) {
    // This performs a static_cast on r.ptr_ to U*, which is a no-op since it
    // is already a U*. So initialization here requires that r.ptr_ is
    // implicitly convertible to T*.
    InitializeWithStaticCast<U>(r);
  }

  // Initializes *this as described in Initialize, but additionally performs a
  // static_cast from r.ptr_ (V*) to U*.
  // NOTE(gfc): We'd need a more general form to support const_pointer_cast and
  // dynamic_pointer_cast, but those operations are sufficiently discouraged
  // that supporting static_pointer_cast is sufficient.
  template <typename U, typename V>
  void InitializeWithStaticCast(const shared_ptr<V>& r) {
    if (r.control_block_ != NULL) {
      RefCountInc(&r.control_block_->refcount_);

      ptr_ = static_cast<U*>(r.ptr_);
      control_block_ = r.control_block_;
    }
  }

  T* ptr_;
  SharedPtrControlBlock* control_block_;
};

}  // namespace wvcdm

#endif  // WVCDM_CORE_SHARED_PTR_H__
