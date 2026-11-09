//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <random>

// template<class RealType, size_t digits, class URBG>
//     RealType generate_canonical(URBG& g);

// libc++ implements P0952R2 as a DR against C++11. C++03 keeps the old algorithm.
// UNSUPPORTED: c++03

// The result is always smaller than 1, even when the URBG always returns max() (https://llvm.org/PR19141,
// https://llvm.org/PR34280). When the range of the URBG is a power of two, the result is then 1 - 2^-d.

#include <random>

#include <cassert>
#include <cstddef>
#include <limits>

#include "test_macros.h"

template <class UInt, UInt Min, UInt Max>
struct MaxGenerator {
  typedef UInt result_type;
  static TEST_CONSTEXPR result_type min() { return Min; }
  static TEST_CONSTEXPR result_type max() { return Max; }
  result_type operator()() { return Max; }
};

template <class F, std::size_t Digits, class G>
void test() {
  const std::size_t real_digits = std::numeric_limits<F>::digits;
  const std::size_t d           = Digits < real_digits ? Digits : real_digits;
  F scale                       = 1;
  for (std::size_t i = 0; i != d; ++i)
    scale /= 2;

  G g;
  F f = std::generate_canonical<F, Digits>(g);
  assert(f < 1);
  assert(f == 1 - scale);
}

template <class F, class G>
void test_digits() {
  test<F, 1, G>();
  test<F, 2, G>();
  test<F, std::numeric_limits<F>::digits - 1, G>();
  test<F, std::numeric_limits<F>::digits, G>();
  test<F, std::numeric_limits<F>::digits + 1, G>();
  test<F, 64, G>();
  test<F, 1000, G>();
}

template <class G>
void test_types() {
  test_digits<float, G>();
  test_digits<double, G>();
  test_digits<long double, G>();
}

int main(int, char**) {
  test_types<MaxGenerator<unsigned char, 0, 255> >();
  test_types<MaxGenerator<unsigned short, 0, 0xFFFF> >();
  test_types<MaxGenerator<unsigned, 0, 0xFFFFFFFF> >();
  test_types<MaxGenerator<unsigned, 1, 0x80000000> >();
  test_types<MaxGenerator<unsigned long long, 0, 0xFFFFFFFFFFFFFFFFULL> >();
  test_types<MaxGenerator<unsigned long long, 0, 0xFFFFFFFFFFFULL> >();

  {
    // uniform_real_distribution(0, 1) uses generate_canonical directly, so it never returns 1 either.
    MaxGenerator<unsigned, 0, 0xFFFFFFFF> g;
    std::uniform_real_distribution<float> dist_f;
    assert(dist_f(g) < 1);
    std::uniform_real_distribution<double> dist_d;
    assert(dist_d(g) < 1);
  }

  return 0;
}
