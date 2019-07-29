// Copyright 2018 Google LLC. All Rights Reserved. This file and proprietary
// source code may only be used and distributed under the Widevine Master
// License Agreement.
//
// Compute CRC32 Checksum. Needed for verification of WV Keybox.
//
#ifndef WVCRC32_H_
#define WVCRC32_H_

#include <stdint.h>

uint32_t wvcrc32(const uint8_t* p_begin, int i_count);

// Convert to network byte order
uint32_t wvcrc32n(const uint8_t* p_begin, int i_count);

#endif  // WVCRC32_H_
