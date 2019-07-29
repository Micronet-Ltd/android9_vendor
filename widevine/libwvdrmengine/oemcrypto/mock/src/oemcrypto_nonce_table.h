// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
//  Mock implementation of OEMCrypto APIs
//
#ifndef MOCK_OEMCRYPTO_NONCE_TABLE_H_
#define MOCK_OEMCRYPTO_NONCE_TABLE_H_

#include <stdint.h>

namespace wvoec_mock {

class NonceTable {
 public:
  static const int kTableSize = 16;
  NonceTable() {
    for (int i = 0; i < kTableSize; ++i) {
      state_[i] = kNTStateInvalid;
    }
  }
  ~NonceTable() {}
  void AddNonce(uint32_t nonce);
  bool CheckNonce(uint32_t nonce);
  void Flush();

 private:
  enum NonceTableState {
    kNTStateInvalid,
    kNTStateValid,
    kNTStateFlushPending
  };
  NonceTableState state_[kTableSize];
  uint32_t age_[kTableSize];
  uint32_t nonces_[kTableSize];
};

}  // namespace wvoec_mock

#endif  // MOCK_OEMCRYPTO_NONCE_TABLE_H_
