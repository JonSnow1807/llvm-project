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

// template<class Sseq> void seed(Sseq& q);

#include <cassert>
#include <random>

#include "philox_engines.h"
#include "test_macros.h"

template <class E>
void test() {
  unsigned a[] = {3, 5, 7};
  std::seed_seq sseq(a, a + 3);
  E e1;
  E e2(sseq);
  assert(e1 != e2);
  e1.discard(3);
  e1.seed(sseq);
  assert(e1 == e2);
  assert(e1() == e2());
}

int main(int, char**) {
  test<std::philox4x32>();
  test<std::philox4x64>();
  test<philox2x32>();
  test<philox4x48>();

  return 0;
}
