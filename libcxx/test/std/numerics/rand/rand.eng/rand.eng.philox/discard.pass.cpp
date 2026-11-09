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

// void discard(unsigned long long z);

#include <cassert>
#include <cstdint>
#include <limits>
#include <random>

#include "philox_engines.h"
#include "test_macros.h"

template <class E>
void test_small() {
  // Compare discard(z) with z calls to operator(), starting from every position in a block.
  for (unsigned start = 0; start != 2 * E::word_count; ++start) {
    for (unsigned long long z = 0; z != 5 * E::word_count; ++z) {
      E e1;
      e1.discard(start);
      E e2 = e1;
      e1.discard(z);
      for (unsigned long long i = 0; i != z; ++i)
        (void)e2();
      assert(e1 == e2);
      assert(e1() == e2());
    }
  }
}

template <class E>
void test_large() {
  using R = typename E::result_type;
  // z = n * b + j means that b blocks are consumed entirely, and j values of the next block.
  const unsigned long long z = std::numeric_limits<unsigned long long>::max();
  const unsigned long long b = z / E::word_count;
  const unsigned j           = z % E::word_count;
  E e1;
  e1.discard(z);
  E e2;
  if constexpr (E::word_size == 64 || E::word_count == 2) {
    // b fits into one word.
    if constexpr (E::word_count == 4)
      e2.set_counter({0, 0, 0, static_cast<R>(b)});
    else
      e2.set_counter({0, static_cast<R>(b)});
  } else {
    e2.set_counter({0, 0, static_cast<R>(b >> 32), static_cast<R>(b & 0xFFFFFFFF)});
  }
  for (unsigned i = 0; i != j; ++i)
    (void)e2();
  assert(e1 == e2);
  assert(e1() == e2());
}

template <class E>
void test_carry() {
  // Discarding carries into the higher words of the counter.
  E e1;
  if constexpr (E::word_count == 4)
    e1.set_counter({0, 0, E::max(), E::max() - 1});
  else
    e1.set_counter({0, E::max() - 1});
  E e2 = e1;
  e1.discard(3 * E::word_count + 1);
  if constexpr (E::word_count == 4)
    e2.set_counter({0, 1, 0, 1});
  else
    e2.set_counter({1, 1});
  (void)e2();
  assert(e1 == e2);
  assert(e1() == e2());
}

int main(int, char**) {
  test_small<std::philox4x32>();
  test_small<std::philox4x64>();
  test_small<philox2x32>();
  test_small<philox2x64>();
  test_small<philox4x48>();
  test_small<philox2x16>();
  test_large<std::philox4x32>();
  test_large<std::philox4x64>();
  test_large<philox2x64>();
  test_carry<std::philox4x32>();
  test_carry<std::philox4x64>();
  test_carry<philox2x32>();
  test_carry<philox4x48>();

  return 0;
}
