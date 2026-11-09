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

// void seed(result_type value = default_seed);

#include <cassert>
#include <random>

#include "philox_engines.h"
#include "test_macros.h"

template <class E>
void test() {
  for (int s = 0; s < 20; ++s) {
    E e1(s);
    E e2;
    (void)e2();
    e2.seed(s);
    assert(e1 == e2);
    assert(e1() == e2());
  }
  {
    E e1;
    E e2(3);
    e2.discard(5);
    e2.seed();
    assert(e1 == e2);
    assert(e1() == e2());
  }
  {
    // seed() also resets the counter
    E e1(7);
    E e2(7);
    e2.set_counter({1, 2});
    e2.seed(7);
    assert(e1 == e2);
  }
}

int main(int, char**) {
  test<std::philox4x32>();
  test<std::philox4x64>();
  test<philox2x32>();
  test<philox4x48>();

  return 0;
}
