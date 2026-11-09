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

// An attempt makes exactly k calls to the URBG, where k is the smallest integer such that R^k >= 2^d.
// Attempts are repeated until S < x * 2^d.

#include <random>

#include <cassert>
#include <cstddef>
#include <limits>

#include "test_macros.h"

// Returns the values in Seq in order and counts the calls.
template <class UInt, UInt Min, UInt Max>
struct SequenceGenerator {
  typedef UInt result_type;
  static TEST_CONSTEXPR result_type min() { return Min; }
  static TEST_CONSTEXPR result_type max() { return Max; }

  const UInt* values;
  std::size_t size;
  std::size_t calls;

  template <std::size_t N>
  explicit SequenceGenerator(const UInt (&v)[N]) : values(v), size(N), calls(0) {}

  result_type operator()() {
    assert(calls < size);
    return values[calls++];
  }
};

// Returns min() and counts the calls.
template <class UInt, UInt Min, UInt Max>
struct CountingGenerator {
  typedef UInt result_type;
  static TEST_CONSTEXPR result_type min() { return Min; }
  static TEST_CONSTEXPR result_type max() { return Max; }

  std::size_t calls;
  CountingGenerator() : calls(0) {}
  result_type operator()() {
    ++calls;
    return Min;
  }
};

template <class F, std::size_t Digits, class G>
void test_calls(std::size_t expected_calls) {
  G g;
  F f = std::generate_canonical<F, Digits>(g);
  assert(f == 0);
  assert(g.calls == expected_calls);
}

int main(int, char**) {
  {
    // R = 3: k is the smallest integer such that 3^k >= 2^d (https://llvm.org/PR53411).
    typedef CountingGenerator<unsigned, 0, 2> G;
    test_calls<float, 0, G>(0);
    test_calls<float, 1, G>(1);
    test_calls<float, 2, G>(2);
    test_calls<float, 24, G>(16);
    test_calls<double, 53, G>(34);
    test_calls<double, 1000, G>(34);
  }
  {
    // R = 2^32
    typedef CountingGenerator<unsigned, 0, 0xFFFFFFFF> G;
    test_calls<float, 24, G>(1);
    test_calls<double, 32, G>(1);
    test_calls<double, 33, G>(2);
    test_calls<double, 53, G>(2);
  }
  {
    // R = 2^31 - 2, like minstd_rand
    typedef CountingGenerator<unsigned, 1, 2147483646> G;
    test_calls<float, 24, G>(1);
    test_calls<double, 31, G>(2);
    test_calls<double, 53, G>(2);
  }
  {
    // R = 3, d = 2: k = 2, x = floor(9 / 4) = 2, so S = 8 is rejected.
    // The second attempt gives S = 1 + 2 * 3 = 7 and the result floor(7 / 2) / 4.
    const unsigned values[] = {2, 2, 1, 2};
    SequenceGenerator<unsigned, 0, 2> g(values);
    float f = std::generate_canonical<float, 2>(g);
    assert(f == 0.75f);
    assert(g.calls == 4);
  }
  {
    // R = 2^31 - 2, d = 24: k = 1, x = 127, so S >= 127 * 2^24 is rejected.
    const unsigned values[] = {2147483646, 2130706433, 2130706432, 4};
    SequenceGenerator<unsigned, 1, 2147483646> g(values);
    float f = std::generate_canonical<float, 24>(g);
    assert(f == 16777215.0f / 16777216.0f);
    assert(g.calls == 3);
  }
  {
    // A power of two range never rejects.
    const unsigned values[] = {0xFFFFFFFF, 0xFFFFFFFF};
    SequenceGenerator<unsigned, 0, 0xFFFFFFFF> g(values);
    double f = std::generate_canonical<double, 53>(g);
    assert(f == 1 - std::numeric_limits<double>::epsilon() / 2);
    assert(g.calls == 2);
  }

  return 0;
}
