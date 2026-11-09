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

// Check the exact values specified by [rand.util.canonical] for an engine whose range is not a power of
// two (minstd_rand0) and for one whose range is a power of two (mt19937).

#include <random>

#include <cassert>
#include <cstddef>
#include <limits>

#include "test_macros.h"

// Returns (hi * 2^64 + lo) / 2^d. Every operation is exact as long as the value fits into F.
template <class F>
F exact(unsigned long long hi, unsigned long long lo, std::size_t d) {
  F scale = 1;
  for (std::size_t i = 0; i != d; ++i)
    scale /= 2;
  F two_64 = F(4294967296.0) * F(4294967296.0);
  return (F(hi) * two_64 + F(lo)) * scale;
}

struct Expected {
  unsigned long long hi;
  unsigned long long lo;
};

template <class F, std::size_t Digits, class E>
void test(std::size_t d, std::size_t calls_per_value, Expected e0, Expected e1, Expected e2) {
  E e;
  F f = std::generate_canonical<F, Digits>(e);
  assert(f == exact<F>(e0.hi, e0.lo, d));
  f = std::generate_canonical<F, Digits>(e);
  assert(f == exact<F>(e1.hi, e1.lo, d));
  f = std::generate_canonical<F, Digits>(e);
  assert(f == exact<F>(e2.hi, e2.lo, d));

  E e_ref;
  e_ref.discard(3 * calls_per_value);
  assert(e == e_ref);
}

template <class E, class F, std::size_t Digits>
void test_zero() {
  E e;
  F f = std::generate_canonical<F, Digits>(e);
  assert(f == 0);
  assert(e == E());
}

int main(int, char**) {
  {
    typedef std::minstd_rand0 E;
    test_zero<E, float, 0>();
    test_zero<E, double, 0>();
    test_zero<E, long double, 0>();

    test<float, 1, E>(1, 1, {0, 0}, {0, 0}, {0, 1});
    test<float, 23, E>(23, 1, {0, 65}, {0, 1107746}, {0, 6363333});
    test<float, 24, E>(24, 1, {0, 132}, {0, 2224214}, {0, 12776772});
    test<float, 25, E>(24, 1, {0, 132}, {0, 2224214}, {0, 12776772});

    test<double, 1, E>(1, 1, {0, 0}, {0, 0}, {0, 1});
    test<double, 52, E>(52, 2, {0, 592972605552112}, {0, 2067595696247882}, {0, 987068441624128});
    test<double, 53, E>(53, 2, {0, 1187105627162056}, {0, 4139237568026582}, {0, 1976068524034213});
    test<double, 54, E>(53, 2, {0, 1187105627162056}, {0, 4139237568026582}, {0, 1976068524034213});
    test<double, std::numeric_limits<std::size_t>::max(), E>(
        53, 2, {0, 1187105627162056}, {0, 4139237568026582}, {0, 1976068524034213});

    if (std::numeric_limits<long double>::digits == 53) {
      test<long double, 1000, E>(53, 2, {0, 1187105627162056}, {0, 4139237568026582}, {0, 1976068524034213});
    } else if (std::numeric_limits<long double>::digits == 64) {
      test<long double, 63, E>(
          63, 3, {0, 6969228999147596121ULL}, {0, 2019542035324656170ULL}, {0, 6265403475867652381ULL});
      test<long double, 1000, E>(
          64, 3, {0, 13938458011276392879ULL}, {0, 4039084074411002530ULL}, {0, 12530806963405528198ULL});
    } else if (std::numeric_limits<long double>::digits == 113) {
      test<long double, 112, E>(
          112,
          4,
          {0x75716f64f835ULL, 0x12e3a1dae9875116ULL},
          {0xadd4f14106adULL, 0xb6a01f6ab0b9dcbcULL},
          {0x8500c8af13a5ULL, 0xf4f1d8012ef47181ULL});
      test<long double, 1000, E>(
          113,
          4,
          {0xeaf18ecddd86ULL, 0xcffee71e7b80e15fULL},
          {0x15bbf9fd7e036ULL, 0x9e5d3678432606feULL},
          {0x10a12338b82d9ULL, 0xd05f1dab13256921ULL});
    } else if (std::numeric_limits<long double>::digits == 106) {
      test<long double, 105, E>(
          105,
          4,
          {0xead44df68dULL, 0x89f0d6255c5c5cfcULL},
          {0x15b9453566fULL, 0xa5657e1781370dc9ULL},
          {0x109f1128333ULL, 0xb9db9c4d3ae76a61ULL});
      test<long double, 1000, E>(
          106,
          4,
          {0x1d5a8d6a23dULL, 0x3ecd9981e98fdd46ULL},
          {0x2b728fd9209ULL, 0xd9b189420538bc92ULL},
          {0x213e26782bcULL, 0xb3997709ec7e0128ULL});
    }
  }
  {
    typedef std::mt19937 E;
    test_zero<E, float, 0>();
    test_zero<E, double, 0>();

    test<float, 1, E>(1, 1, {0, 1}, {0, 0}, {0, 1});
    test<float, 23, E>(23, 1, {0, 6834397}, {0, 1136463}, {0, 7598333});
    test<float, 24, E>(24, 1, {0, 13668795}, {0, 2272926}, {0, 15196666});
    test<float, 25, E>(24, 1, {0, 13668795}, {0, 2272926}, {0, 15196666});

    test<double, 52, E>(52, 2, {0, 610134186068251}, {0, 3760544374750751}, {0, 4363392533006371});
    test<double, 53, E>(53, 2, {0, 1220268372136503}, {0, 7521088749501503}, {0, 8726785066012742});
    test<double, 54, E>(53, 2, {0, 1220268372136503}, {0, 7521088749501503}, {0, 8726785066012742});

    if (std::numeric_limits<long double>::digits == 53) {
      test<long double, 1000, E>(53, 2, {0, 1220268372136503}, {0, 7521088749501503}, {0, 8726785066012742});
    } else if (std::numeric_limits<long double>::digits == 64) {
      test<long double, 63, E>(
          63, 2, {0, 1249554813067779502ULL}, {0, 7701594879489539447ULL}, {0, 8936227907597048470ULL});
      test<long double, 1000, E>(
          64, 2, {0, 2499109626135559004ULL}, {0, 15403189758979078894ULL}, {0, 17872455815194096940ULL});
    } else if (std::numeric_limits<long double>::digits == 113) {
      test<long double, 112, E>(
          112,
          4,
          {0xd5c31f79e7e1ULL, 0xfaee22ae9ef6d091ULL},
          {0x3895afe1e9d3ULL, 0x5f807b7df2082ULL},
          {0x8c16a62518f8ULL, 0x68634ee4092ba1e2ULL});
      test<long double, 1000, E>(
          113,
          4,
          {0x1ab863ef3cfc3ULL, 0xf5dc455d3deda123ULL},
          {0x712b5fc3d3a6ULL, 0xbf00f6fbe4104ULL},
          {0x1182d4c4a31f0ULL, 0xd0c69dc8125743c4ULL});
    } else if (std::numeric_limits<long double>::digits == 106) {
      test<long double, 105, E>(
          105,
          4,
          {0x1ab863ef3cfULL, 0xc3f5dc455d3deda1ULL},
          {0x712b5fc3d3ULL, 0xa6000bf00f6fbe41ULL},
          {0x1182d4c4a31ULL, 0xf0d0c69dc8125743ULL});
      test<long double, 1000, E>(
          106,
          4,
          {0x3570c7de79fULL, 0x87ebb88aba7bdb42ULL},
          {0xe256bf87a7ULL, 0x4c0017e01edf7c82ULL},
          {0x2305a989463ULL, 0xe1a18d3b9024ae87ULL});
    }
  }

  return 0;
}
