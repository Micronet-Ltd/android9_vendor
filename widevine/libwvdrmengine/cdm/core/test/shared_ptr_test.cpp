// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#include <string>
#include <errno.h>
#include <getopt.h>
#include <gtest/gtest.h>

#include "shared_ptr.h"

#include "log.h"

namespace wvcdm {

class SharedPtrTest : public testing::Test {
 public:

  static void NoteDeletion() {
    deletions++;
  }

  class Dummy {
   public:
    explicit Dummy(int v) : value_(v) { exists_ = true; }
    ~Dummy() {
      NoteDeletion();
      exists_ = false;
    }
    bool exists() { return exists_; }
    int getValue() { return value_; }
    void setValue(int v) { value_ = v; }
   private:
    bool exists_;
    int value_;
  };

  static void ExpectedDeletions(int count) {
    ASSERT_TRUE(deletions == count);
    deletions = 0;
  }

  virtual void SetUpTest() {
    deletions = 0;
  }
  virtual void TearDownTest() {
    deletions = 0;
  }
 private:
  static int deletions;
};

int SharedPtrTest::deletions = 0;

TEST_F(SharedPtrTest, NullSingletonCreate) {
  shared_ptr<Dummy> sd1;
  ASSERT_TRUE(sd1.unique());
  ASSERT_TRUE(sd1.get() == NULL);
  ExpectedDeletions(0);
}

TEST_F(SharedPtrTest, SingletonCreate) {
  {
    Dummy* d1 = new Dummy(42);
    shared_ptr<Dummy> sd1(d1);
    ASSERT_TRUE(sd1.unique());
    ASSERT_TRUE(sd1.get() == d1);
    ASSERT_TRUE(sd1->getValue() == 42);
  }
  ExpectedDeletions(1);
}

TEST_F(SharedPtrTest, ResetToNull) {
  Dummy* d1 = new Dummy(42);
  shared_ptr<Dummy> sd1(d1);
  ASSERT_TRUE(sd1->getValue() == 42);
  sd1.reset();
  ExpectedDeletions(1);
  ASSERT_TRUE(sd1.get() == NULL);
}

TEST_F(SharedPtrTest, SharedCreate) {
  {
    Dummy* d1 = new Dummy(42);
    shared_ptr<Dummy> sd1(d1);
    {
      shared_ptr<Dummy> sd2(sd1);
      ASSERT_FALSE(sd1.unique());
      ASSERT_TRUE(sd1.get() == d1);
      ASSERT_TRUE(sd2.get() == d1);
      ASSERT_TRUE(sd1.use_count() == 2);
    }
    ExpectedDeletions(0);
    ASSERT_TRUE(sd1.use_count() == 1);
  }
  ExpectedDeletions(1);
}

TEST_F(SharedPtrTest, SharedInstance) {
  Dummy* d1 = new Dummy(42);
  {
    shared_ptr<Dummy> sd1(d1);
    {
      shared_ptr<Dummy> sd2(sd1);
      ASSERT_FALSE(sd1.unique());
      ASSERT_TRUE(sd1.get() == d1);
      ASSERT_TRUE(sd2.get() == d1);
      sd2->setValue(55);
      ASSERT_TRUE(sd1.use_count() == 2);
    }
    ExpectedDeletions(0);
    ASSERT_TRUE(sd1.use_count() == 1);
    ASSERT_TRUE(sd1->getValue() == 55);
  }
  ExpectedDeletions(1);
}

TEST_F(SharedPtrTest, Reset) {
  {
    Dummy* d1 = new Dummy(42);
    Dummy* d2 = new Dummy(96);
    shared_ptr<Dummy> sd1(d1);
    sd1.reset(d2);
    ExpectedDeletions(1);
    ASSERT_TRUE(sd1->getValue() == 96);
  }
  ExpectedDeletions(1);
}

}  // namespace wvcdm
