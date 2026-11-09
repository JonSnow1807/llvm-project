//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23

// <random>

// using philox4x64 =
//       philox_engine<uint_fast64_t, 64, 4, 10,
//        0xCA5A826395121157, 0x9E3779B97F4A7C15, 0xD2E7470EE14C6C93, 0xBB67AE8584CAA73B>;
//
// Required behavior: The 10000th consecutive invocation of a default-constructed object of type philox4x64
// produces the value 3409172418970261260.

#include <random>
#include <cassert>
#include <cstdint>
#include <type_traits>

#include "test_macros.h"

int main(int, char**) {
  static_assert(
      std::is_same_v<std::philox4x64,
                     std::philox_engine<std::uint_fast64_t,
                                        64,
                                        4,
                                        10,
                                        0xCA5A826395121157,
                                        0x9E3779B97F4A7C15,
                                        0xD2E7470EE14C6C93,
                                        0xBB67AE8584CAA73B>>);
  {
    std::philox4x64 e;
    e.discard(9999);
    assert(e() == 3409172418970261260ull);
  }
  {
    std::philox4x64 e;
    for (int i = 0; i != 9999; ++i)
      (void)e();
    assert(e() == 3409172418970261260ull);
  }

  return 0;
}
