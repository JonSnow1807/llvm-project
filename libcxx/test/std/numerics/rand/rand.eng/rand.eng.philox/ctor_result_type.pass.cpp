//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23
// UNSUPPORTED: no-localization

// <random>

// template<class UIntType, size_t w, size_t n, size_t r, UIntType... consts>
// class philox_engine;

// explicit philox_engine(result_type value);

#include <cassert>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

#include "philox_engines.h"
#include "test_macros.h"

template <class E>
std::string state(const E& e) {
  std::ostringstream os;
  os << e;
  return os.str();
}

int main(int, char**) {
  static_assert(!std::is_convertible_v<std::philox4x32::result_type, std::philox4x32>);
  static_assert(std::is_constructible_v<std::philox4x32, std::philox4x32::result_type>);

  // The textual representation is K, X and i: K_0 is set to value mod 2^w, everything else is 0, and i is n - 1.
  assert(state(std::philox4x32(5)) == "5 0 0 0 0 0 3");
  assert(state(std::philox4x64(0xFFFFFFFFFFFFFFFF)) == "18446744073709551615 0 0 0 0 0 3");
  assert(state(philox2x32(0)) == "0 0 0 1");
  // uint_fast32_t can be wider than 32 bits
  assert(state(std::philox4x32(std::philox4x32::result_type(0xFFFFFFFF))) == "4294967295 0 0 0 0 0 3");
  assert(state(philox4x48(0x123456789ABCDEF)) == "76310993685999 0 0 0 0 0 3");
  assert(state(philox2x16(9029)) == "9029 0 0 1");

  {
    // The first values only depend on the key.
    std::philox4x32 e(20111115u);
    assert(e() == 3587538684u);
  }

  return 0;
}
