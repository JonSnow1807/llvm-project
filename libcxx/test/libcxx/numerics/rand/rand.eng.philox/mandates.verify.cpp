//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23

// <random>

// template<class UIntType, size_t w, size_t n, size_t r, UIntType... consts>
// class philox_engine;

// Mandates:
// - sizeof...(consts) == n is true, and
// - n == 2 || n == 4 is true, and
// - 0 < r is true, and
// - 0 < w && w <= numeric_limits<UIntType>::digits is true.

#include <cstdint>
#include <random>

void test() {
  {
    // expected-error-re@*:* {{static assertion failed{{.*}}sizeof...(consts) must be equal to n}}
    [[maybe_unused]] std::philox_engine<std::uint32_t, 32, 4, 10, 1, 2> e;
  }
  {
    // expected-error-re@*:* {{static assertion failed{{.*}}n must be 2 or 4}}
    [[maybe_unused]] std::philox_engine<std::uint32_t, 32, 3, 10, 1, 2, 3> e;
  }
  {
    // expected-error-re@*:* {{static assertion failed{{.*}}r must be greater than 0}}
    [[maybe_unused]] std::philox_engine<std::uint32_t, 32, 2, 0, 1, 2> e;
  }
  {
    // expected-error-re@*:* {{static assertion failed{{.*}}w must be in the range}}
    [[maybe_unused]] std::philox_engine<std::uint32_t, 0, 2, 10, 1, 2> e;
  }
  {
    // expected-error-re@*:* {{static assertion failed{{.*}}w must be in the range}}
    [[maybe_unused]] std::philox_engine<std::uint32_t, 33, 2, 10, 1, 2> e;
  }
}
