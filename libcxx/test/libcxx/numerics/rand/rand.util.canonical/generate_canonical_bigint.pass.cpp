//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03

// <random>

// std::generate_canonical falls back to __generate_canonical_bigint when R^k doesn't fit into the widest
// integer type, which can't happen for every floating-point type on every target. Check that the fallback
// produces the same values and makes the same calls as the other code paths. This also covers URBGs whose
// range doesn't fit into 64 bits.

#include <random>

#include <cassert>
#include <cstddef>
#include <limits>

#include "test_macros.h"

template <unsigned long long Min, unsigned long long Max>
struct Generator {
  typedef unsigned long long result_type;
  static TEST_CONSTEXPR result_type min() { return Min; }
  static TEST_CONSTEXPR result_type max() { return Max; }

  std::mt19937_64 engine;
  unsigned calls;
  Generator() : calls(0) {}
  result_type operator()() {
    ++calls;
    // Return max() often, so that some attempts are rejected.
    if (engine() % 4 == 0)
      return Max;
    return Max - Min == ~0ULL ? engine() : Min + engine() % (Max - Min + 1);
  }
};

#ifndef TEST_HAS_NO_INT128
template <unsigned Bits, unsigned long long Add>
struct WideGenerator {
  typedef __uint128_t result_type;
  static TEST_CONSTEXPR result_type min() { return 0; }
  static TEST_CONSTEXPR result_type max() { return Bits == 128 ? ~__uint128_t(0) : (__uint128_t(1) << Bits) + Add - 1; }

  std::mt19937_64 engine;
  unsigned calls;
  WideGenerator() : calls(0) {}
  result_type operator()() {
    ++calls;
    if (engine() % 4 == 0)
      return max();
    __uint128_t v     = (__uint128_t(engine()) << 64) | engine();
    __uint128_t range = max();
    ++range;
    return range == 0 ? v : v % range;
  }
};

// Always returns 2^127.
struct HalfGenerator {
  typedef __uint128_t result_type;
  static TEST_CONSTEXPR result_type min() { return 0; }
  static TEST_CONSTEXPR result_type max() { return ~__uint128_t(0); }
  result_type operator()() { return __uint128_t(1) << 127; }
};
#endif

template <class F, std::size_t D, class G>
void test() {
  const std::size_t real_digits = std::numeric_limits<F>::digits;
  const std::size_t d           = D < real_digits ? D : real_digits;
  G g1;
  G g2;
  for (int i = 0; i != 1000; ++i) {
    F expected = std::generate_canonical<F, D>(g1);
    F actual   = std::__generate_canonical_bigint<F, d>(g2);
    assert(expected == actual);
    assert(g1.calls == g2.calls);
    assert(actual >= 0 && actual < 1);
  }
}

template <class G>
void test_digits() {
  test<float, 1, G>();
  test<float, 24, G>();
  test<double, 2, G>();
  test<double, 33, G>();
  test<double, 53, G>();
  test<long double, 64, G>();
  test<long double, 1000, G>();
}

int main(int, char**) {
  test_digits<Generator<0, 2> >();
  test_digits<Generator<0, 999999> >();
  test_digits<Generator<1, 2147483646> >();
  test_digits<Generator<0, 0xFFFFFFFF> >();
  test_digits<Generator<5, 0x1000000004ULL> >();
  test_digits<Generator<0, (1ULL << 63)> >();
  test_digits<Generator<0, ~0ULL - 58> >();
  test_digits<Generator<0, ~0ULL> >();

#ifndef TEST_HAS_NO_INT128
  test_digits<WideGenerator<128, 0> >();
  test_digits<WideGenerator<70, 0> >();
  test_digits<WideGenerator<100, 3> >();
  test_digits<WideGenerator<64, 1> >();
  {
    HalfGenerator g;
    assert((std::generate_canonical<double, 53>(g) == 0.5));
    assert((std::generate_canonical<float, 24>(g) == 0.5f));
    assert((std::generate_canonical<long double, 1000>(g) == 0.5L));
  }
#endif

  return 0;
}
