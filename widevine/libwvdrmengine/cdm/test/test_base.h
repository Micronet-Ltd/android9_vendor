// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.

#include <gtest/gtest.h>

namespace wvcdm {
class WvCdmTestBase : public ::testing::Test {
 public:
  WvCdmTestBase() {}
  virtual ~WvCdmTestBase() {}

 protected:
  virtual void SetUp() {
    const ::testing::TestInfo* const test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();
    LOGD("Running test %s.%s", test_info->test_case_name(),
         test_info->name());
  }
};
}  // namespace wvcdm
