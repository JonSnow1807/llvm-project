//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20, c++23
// UNSUPPORTED: no-localization

// <random>

// template<class UIntType, size_t w, size_t n, size_t r, UIntType... consts>
// class philox_engine;

// template<class Sseq> explicit philox_engine(Sseq& q);

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

#include "philox_engines.h"
#include "test_macros.h"

template <class E>
std::string state(const E& e) {
  std::ostringstream os;
  os << e;
  return os.str();
}

// Records how many values are requested from the seed sequence.
struct CountingSeedSeq {
  using result_type     = std::uint32_t;
  std::size_t requested = 0;
  template <class It>
  void generate(It first, It last) {
    std::uint32_t v = 0x12345678;
    for (; first != last; ++first, ++requested)
      *first = v++;
  }
};

int main(int, char**) {
  static_assert(!std::is_convertible_v<std::seed_seq&, std::philox4x32>);

  {
    // K_k is built from p = ceil(w / 32) values of the seed sequence, and X is 0.
    std::seed_seq sseq = {1, 2, 3, 4, 5};
    assert(state(std::philox4x32(sseq)) == "900843130 653102001 0 0 0 0 3");
    assert(state(std::philox4x64(sseq)) == "17039546833952701275 12621438843259549211 0 0 0 0 3");
    assert(state(philox2x32(sseq)) == "2748548493 0 0 1");
    assert(state(philox4x48(sseq)) == "177643796429659 100887553734171 0 0 0 0 3");
  }
  {
    CountingSeedSeq q;
    std::philox4x32 e(q);
    assert(q.requested == 2);
    assert(state(e) == "305419896 305419897 0 0 0 0 3");
  }
  {
    CountingSeedSeq q;
    std::philox4x64 e(q);
    assert(q.requested == 4);
    // K_0 = a_0 + a_1 * 2^32, K_1 = a_2 + a_3 * 2^32
    assert(state(e) == "1311768469468108408 1311768478058043002 0 0 0 0 3");
  }
  {
    CountingSeedSeq q;
    philox4x48 e(q);
    assert(q.requested == 4);
    // K_k = (a_2k + a_2k+1 * 2^32) mod 2^48
    assert(state(e) == "95077996451448 95086586386042 0 0 0 0 3");
  }

  return 0;
}
