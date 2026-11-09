//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef TEST_STD_NUMERICS_RAND_RAND_ENG_RAND_ENG_PHILOX_PHILOX_ENGINES_H
#define TEST_STD_NUMERICS_RAND_RAND_ENG_RAND_ENG_PHILOX_PHILOX_ENGINES_H

#include <cstdint>
#include <random>

// Philox2x32 and Philox2x64 with the constants from the Random123 library
using philox2x32 = std::philox_engine<std::uint_fast32_t, 32, 2, 10, 0xD256D193, 0x9E3779B9>;
using philox2x64 = std::philox_engine<std::uint_fast64_t, 64, 2, 10, 0xD2B74407B1CE6E93, 0x9E3779B97F4A7C15>;

// A word size that is smaller than the width of result_type
using philox4x48 =
    std::philox_engine<std::uint64_t, 48, 4, 10, 0xCA5A82639512, 0x9E3779B97F4A, 0xD2E7470EE14C, 0xBB67AE8584CA>;

// A small result_type
using philox2x16 = std::philox_engine<unsigned short, 16, 2, 7, 0xD256, 0x9E37>;

#endif // TEST_STD_NUMERICS_RAND_RAND_ENG_RAND_ENG_PHILOX_PHILOX_ENGINES_H
