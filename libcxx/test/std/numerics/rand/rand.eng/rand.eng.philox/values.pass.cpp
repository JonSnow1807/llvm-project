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

// static constexpr size_t word_size = w;
// static constexpr size_t word_count = n;
// static constexpr size_t round_count = r;
// static constexpr array<result_type, n / 2> multipliers;
// static constexpr array<result_type, n / 2> round_consts;
// static constexpr result_type min() { return 0; }
// static constexpr result_type max() { return m - 1; }
// static constexpr result_type default_seed = 20111115u;

#include <array>
#include <cassert>
#include <cstdint>
#include <random>

#include "philox_engines.h"
#include "test_macros.h"

template <class T>
void where(const T&) {}

template <class E>
void check_odr_use() {
  where(E::word_size);
  where(E::word_count);
  where(E::round_count);
  where(E::multipliers);
  where(E::round_consts);
  where(E::default_seed);
}

int main(int, char**) {
  {
    using E = std::philox4x32;
    using R = E::result_type;
    static_assert(E::word_size == 32);
    static_assert(E::word_count == 4);
    static_assert(E::round_count == 10);
    static_assert(E::multipliers == std::array<R, 2>{0xCD9E8D57, 0xD2511F53});
    static_assert(E::round_consts == std::array<R, 2>{0x9E3779B9, 0xBB67AE85});
    static_assert(E::min() == 0);
    static_assert(E::max() == 0xFFFFFFFF); // LWG4153
    static_assert(E::default_seed == 20111115u);
    ASSERT_SAME_TYPE(decltype(E::word_size), const std::size_t);
    ASSERT_SAME_TYPE(decltype(E::multipliers), const std::array<R, 2>);
    ASSERT_SAME_TYPE(decltype(E::default_seed), const R);
    ASSERT_SAME_TYPE(decltype(E::min()), R);
    ASSERT_SAME_TYPE(decltype(E::max()), R);
    check_odr_use<E>();
  }
  {
    using E = std::philox4x64;
    using R = E::result_type;
    static_assert(E::word_size == 64);
    static_assert(E::word_count == 4);
    static_assert(E::round_count == 10);
    static_assert(E::multipliers == std::array<R, 2>{0xCA5A826395121157, 0xD2E7470EE14C6C93});
    static_assert(E::round_consts == std::array<R, 2>{0x9E3779B97F4A7C15, 0xBB67AE8584CAA73B});
    static_assert(E::min() == 0);
    static_assert(E::max() == 0xFFFFFFFFFFFFFFFF);
    static_assert(E::default_seed == 20111115u);
    check_odr_use<E>();
  }
  {
    using E = philox2x32;
    using R = E::result_type;
    static_assert(E::word_size == 32);
    static_assert(E::word_count == 2);
    static_assert(E::round_count == 10);
    static_assert(E::multipliers == std::array<R, 1>{0xD256D193});
    static_assert(E::round_consts == std::array<R, 1>{0x9E3779B9});
    static_assert(E::max() == 0xFFFFFFFF);
    check_odr_use<E>();
  }
  {
    using E = philox4x48;
    static_assert(E::word_size == 48);
    static_assert(E::max() == 0xFFFFFFFFFFFF);
    check_odr_use<E>();
  }
  {
    using E = philox2x16;
    static_assert(E::word_size == 16);
    static_assert(E::round_count == 7);
    static_assert(E::max() == 0xFFFF);
    check_odr_use<E>();
  }

  return 0;
}
