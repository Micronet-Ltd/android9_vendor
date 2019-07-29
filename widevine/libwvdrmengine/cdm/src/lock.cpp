// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
// Lock class - provides a simple android specific mutex implementation

#include "lock.h"
#include <utils/Mutex.h>

namespace wvcdm {

class Lock::Impl {
 public:
  android::Mutex lock_;
};

Lock::Lock() : impl_(new Lock::Impl()) {
}

Lock::~Lock() {
  delete impl_;
}

void Lock::Acquire() {
  impl_->lock_.lock();
}

void Lock::Release() {
  impl_->lock_.unlock();
}

}  // namespace wvcdm
