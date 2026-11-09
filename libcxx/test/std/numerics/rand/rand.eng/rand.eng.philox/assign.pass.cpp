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

// philox_engine& operator=(const philox_engine&);

#include <cassert>
#include <random>

#include "philox_engines.h"
#include "test_macros.h"

template <class E>
void test() {
  E e1(2);
  (void)e1();
  E e2(5);
  e2 = e1;
  assert(e1 == e2);
  assert(e1() == e2());
  typename E::result_type k = e1();
  assert(e1 != e2);
  assert(e2() == k);
  assert(e1 == e2);
}

int main(int, char**) {
  test<std::philox4x32>();
  test<std::philox4x64>();
  test<philox2x32>();
  test<philox4x48>();

  return 0;
}
