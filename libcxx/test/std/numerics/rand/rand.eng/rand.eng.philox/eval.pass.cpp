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

// result_type operator()();

// The expected values were generated with the reference implementation of the Random123 library.

#include <cassert>
#include <cstddef>
#include <random>

#include "philox_engines.h"
#include "test_macros.h"

template <class E, std::size_t N>
void test(const unsigned long long (&expected)[N]) {
  E e;
  for (std::size_t i = 0; i != N; ++i) {
    typename E::result_type v = e();
    assert(v == expected[i]);
    assert(v <= E::max());
  }
}

int main(int, char**) {
  {
    const unsigned long long expected[] = {
        3587538684, 1324224816, 3068087177, 2030706281, 1694797232, 3200855668, 284762628, 612470539};
    test<std::philox4x32>(expected);
  }
  {
    const unsigned long long expected[] = {
        4854577551194240716ULL,
        11024447680751626801ULL,
        6491473261962256061ULL,
        17735969495851009945ULL,
        13826806250750822200ULL,
        16700215933986118703ULL,
        14905284484073033320ULL,
        5288335737392948403ULL};
    test<std::philox4x64>(expected);
  }
  {
    const unsigned long long expected[] = {
        429918632, 2445805855, 924533025, 443322697, 30426172, 3687180296, 2870325986, 2802471804};
    test<philox2x32>(expected);
  }
  {
    const unsigned long long expected[] = {
        709466296749222363ULL,
        3729519840899645291ULL,
        15147500311653449311ULL,
        10457761022206342332ULL,
        11302470134137274435ULL,
        2750859226678029066ULL,
        12097101262717454110ULL,
        8433110651587408211ULL};
    test<philox2x64>(expected);
  }
  {
    // All values are in [min(), max()] when w is smaller than the width of result_type.
    philox4x48 e;
    for (int i = 0; i != 1000; ++i)
      assert(e() <= philox4x48::max());
    philox2x16 f;
    for (int i = 0; i != 1000; ++i)
      assert(f() <= philox2x16::max());
  }
  {
    // A multiplier that doesn't fit into w bits: mulhi(1, 65537, 16) is floor(65537 / 65536) = 1.
    std::philox_engine<unsigned, 16, 2, 1, 65537, 0> e(0);
    e.set_counter({0, 1});
    assert(e() == 1);
    assert(e() == 1);
  }

  return 0;
}
