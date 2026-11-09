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

// template<class charT, class traits>
//   friend basic_ostream<charT, traits>&
//     operator<<(basic_ostream<charT, traits>& os, const philox_engine& x);
// template<class charT, class traits>
//   friend basic_istream<charT, traits>&
//     operator>>(basic_istream<charT, traits>& is, philox_engine& x);

#include <cassert>
#include <ios>
#include <random>
#include <sstream>
#include <string>

#include "philox_engines.h"
#include "test_macros.h"

template <class E>
void test_round_trip() {
  // The textual representation doesn't contain the output buffer Y, so operator>> has to reconstruct it.
  for (unsigned start = 0; start != 3 * E::word_count; ++start) {
    E e1(7);
    e1.discard(start);
    std::ostringstream os;
    os << e1;
    std::istringstream is(os.str());
    E e2(11);
    is >> e2;
    assert(!is.fail());
    assert(e1 == e2);
    for (int i = 0; i != 10; ++i)
      assert(e1() == e2());
  }
}

int main(int, char**) {
  test_round_trip<std::philox4x32>();
  test_round_trip<std::philox4x64>();
  test_round_trip<philox2x32>();
  test_round_trip<philox2x64>();
  test_round_trip<philox4x48>();
  test_round_trip<philox2x16>();

  {
    // The textual representation is K_0, ..., K_{n/2-1}, X_0, ..., X_{n-1}, i.
    std::philox4x32 e;
    (void)e();
    (void)e();
    (void)e();
    std::ostringstream os;
    os << e;
    assert(os.str() == "20111115 0 1 0 0 0 2");
  }
  {
    philox2x32 e;
    e.discard(3);
    std::ostringstream os;
    os << e;
    assert(os.str() == "20111115 2 0 0");
  }
  {
    // The stream flags are restored.
    std::philox4x32 e;
    std::ostringstream os;
    os << std::hex << std::showbase;
    os << e;
    assert(os.str() == "20111115 0 0 0 0 0 3");
    assert(os.flags() & std::ios_base::hex);
    assert(os.flags() & std::ios_base::showbase);
  }
  {
    // Counter values around the carry
    std::philox4x32 e1(5);
    e1.set_counter({0, 0, 1, 0});
    (void)e1();
    std::ostringstream os;
    os << e1;
    assert(os.str() == "5 0 1 1 0 0 0");
    std::istringstream is(os.str());
    std::philox4x32 e2;
    is >> e2;
    assert(e1 == e2);
    assert(e1() == e2());
  }
  {
    // An index that is out of range is rejected.
    std::philox4x32 e(5);
    std::istringstream is("1 2 3 4 5 6 4");
    is >> e;
    assert(is.fail());
    assert(e == std::philox4x32(5));
  }
#ifndef TEST_HAS_NO_WIDE_CHARACTERS
  {
    std::philox4x64 e1;
    e1.discard(5);
    std::wostringstream os;
    os << e1;
    assert(os.str() == L"20111115 0 2 0 0 0 0");
    std::wistringstream is(os.str());
    std::philox4x64 e2;
    is >> e2;
    assert(e1 == e2);
    assert(e1() == e2());
  }
#endif

  return 0;
}
