//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23

// <random>

// using philox4x32 =
//       philox_engine<uint_fast32_t, 32, 4, 10,
//        0xCD9E8D57, 0x9E3779B9, 0xD2511F53, 0xBB67AE85>;
//
// Required behavior: The 10000th consecutive invocation of a default-constructed object of type philox4x32
// produces the value 1955073260.

#include <random>
#include <cassert>
#include <cstdint>
#include <type_traits>

#include "test_macros.h"

int main(int, char**) {
  static_assert(std::is_same_v<
                std::philox4x32,
                std::philox_engine<std::uint_fast32_t, 32, 4, 10, 0xCD9E8D57, 0x9E3779B9, 0xD2511F53, 0xBB67AE85>>);
  {
    std::philox4x32 e;
    e.discard(9999);
    assert(e() == 1955073260u);
  }
  {
    std::philox4x32 e;
    for (int i = 0; i != 9999; ++i)
      (void)e();
    assert(e() == 1955073260u);
  }

  return 0;
}
