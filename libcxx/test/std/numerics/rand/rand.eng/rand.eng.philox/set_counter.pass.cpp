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

// void set_counter(const array<result_type, n>& counter);

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>

#include "philox_engines.h"
#include "test_macros.h"

template <class E, std::size_t N>
void check(E e, const unsigned long long (&expected)[N]) {
  for (std::size_t i = 0; i != N; ++i)
    assert(e() == expected[i]);
}

template <class E>
void test_counter_order() {
  // The counter is the n * w bit number whose least significant word is the last element of the array.
  for (unsigned long long z : {1ULL, 2ULL, 3ULL, 1000ULL, 123456789ULL}) {
    E e1;
    e1.discard(z * E::word_count);
    E e2;
    if constexpr (E::word_count == 4)
      e2.set_counter({0, 0, 0, static_cast<typename E::result_type>(z)});
    else
      e2.set_counter({0, static_cast<typename E::result_type>(z)});
    assert(e1 == e2);
    assert(e1() == e2());
  }
}

template <class E>
void test_resets_index() {
  // set_counter makes the next call start a new block.
  E e1;
  (void)e1();
  (void)e1();
  if constexpr (E::word_count == 4)
    e1.set_counter({0, 0, 0, 0});
  else
    e1.set_counter({0, 0});
  E e2;
  assert(e1 == e2);
  assert(e1() == e2());
}

int main(int, char**) {
  test_counter_order<std::philox4x32>();
  test_counter_order<std::philox4x64>();
  test_counter_order<philox2x32>();
  test_counter_order<philox2x64>();
  test_counter_order<philox4x48>();
  test_resets_index<std::philox4x32>();
  test_resets_index<std::philox4x64>();
  test_resets_index<philox2x32>();

  // The expected values below were generated with the reference implementation of the Random123 library.
  {
    std::philox4x32 e(999);
    e.set_counter({7, 3, 0, 0});
    const unsigned long long expected[] = {
        66473973, 2183661217, 17071251, 3426751099, 2880121847, 194467663, 1721091609, 3595655966};
    check(e, expected);
  }
  {
    // The counter carries into the next words.
    std::philox4x32 e(3);
    e.set_counter({0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE});
    const unsigned long long expected[] = {
        3927721875, 1215658215, 2267399789, 3941680661, 2882573539, 442886971, 1007267061, 65506641};
    check(e, expected);
  }
  {
    // The counter wraps around to 0.
    std::philox4x32 e(4);
    e.set_counter({0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE});
    const unsigned long long expected[] = {
        1202364166, 3820872837, 2858212226, 1984087887, 3422744255, 1497690481, 2633927523, 3648184137};
    check(e, expected);
    // After two blocks, the counter is 0 again.
    e.discard(8);
    std::philox4x32 e2(4);
    for (int i = 0; i != 4; ++i)
      assert(e() == e2());
  }
  {
    std::philox4x64 e(3);
    e.set_counter({0, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE});
    const unsigned long long expected[] = {
        1581255460517574806ULL,
        11601146352809402135ULL,
        18010353852128984673ULL,
        2194406156827350200ULL,
        16493099805287735635ULL,
        2443735053851798230ULL,
        16621815511958954953ULL,
        17573650070276392986ULL};
    check(e, expected);
  }
  {
    std::philox4x64 e(4);
    e.set_counter({0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE});
    const unsigned long long expected[] = {
        11901265861367224416ULL,
        7707656418913253808ULL,
        1234687084681190312ULL,
        6920627949152136150ULL,
        3025634634493761738ULL,
        11075863627333385335ULL,
        243782292557398747ULL,
        15733298976146567693ULL};
    check(e, expected);
  }
  {
    philox2x32 e(1);
    e.set_counter({0, 0xFFFFFFFE});
    const unsigned long long expected[] = {
        3789106206, 2248016298, 2091730476, 3191253723, 2127719656, 2261251608, 3793336190, 1786699810};
    check(e, expected);
  }
  {
    philox2x64 e(4);
    e.set_counter({0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFE});
    const unsigned long long expected[] = {
        6514934157129808698ULL,
        9524504577014353276ULL,
        17923856303929517118ULL,
        8947382338141464444ULL,
        11365991096180195922ULL,
        15645644679409370388ULL,
        4659432815182303879ULL,
        17638148976249050927ULL};
    check(e, expected);
  }
  {
    // The counter is reduced modulo 2^w.
    philox4x48 e1;
    e1.set_counter({0x1000000000001, 0x1000000000002, 0x1000000000003, 0x1000000000004});
    philox4x48 e2;
    e2.set_counter({1, 2, 3, 4});
    assert(e1 == e2);
    assert(e1() == e2());
  }

  return 0;
}
