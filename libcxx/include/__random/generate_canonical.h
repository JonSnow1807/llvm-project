//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___RANDOM_GENERATE_CANONICAL_H
#define _LIBCPP___RANDOM_GENERATE_CANONICAL_H

#include <__config>
#include <__cstddef/size_t.h>
#include <__random/log2.h>
#include <__type_traits/conditional.h>
#include <cstdint>
#include <initializer_list>
#include <limits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

_LIBCPP_BEGIN_NAMESPACE_STD

// generate_canonical

#ifndef _LIBCPP_CXX03_LANG

// This implements the specification from P0952R2, which is applied as a DR to C++11. The result is
// floor(S / x) / 2^d, where S is built from k calls to the URNG, and S is rejected and regenerated
// when S >= x * 2^d. The result is always in [0, 1). When the range R of the URNG is a power of two,
// no attempt is ever rejected and S is only shifted.

#  if _LIBCPP_HAS_INT128
using __generate_canonical_wide_uint _LIBCPP_NODEBUG = __uint128_t;
#  else
using __generate_canonical_wide_uint _LIBCPP_NODEBUG = unsigned long long;
#  endif

// The type used for the values returned by the URNG minus min()
template <class _URNG>
using __generate_canonical_range_t _LIBCPP_NODEBUG =
    __conditional_t<(_URNG::max() - _URNG::min() > numeric_limits<unsigned long long>::max()),
                    __generate_canonical_wide_uint,
                    unsigned long long>;

// Returns 2^-__e. Every step is exact.
template <class _RealType>
_LIBCPP_HIDE_FROM_ABI constexpr _RealType __generate_canonical_exp2_neg(size_t __e) {
  return __e == 0  ? _RealType(1)
       : __e >= 32 ? std::__generate_canonical_exp2_neg<_RealType>(__e - 32) / _RealType(4294967296.0)
                   : std::__generate_canonical_exp2_neg<_RealType>(__e - 1) / _RealType(2);
}

// Returns R^k for the smallest k such that R^k >= 2^__d, or 0 if that value doesn't fit into _Up.
template <class _Up>
_LIBCPP_HIDE_FROM_ABI constexpr _Up __generate_canonical_rk(_Up __r, size_t __d, _Up __pow = 1) {
  return __d < static_cast<size_t>(numeric_limits<_Up>::digits) && __pow >= (_Up(1) << __d) ? __pow
       : __pow > numeric_limits<_Up>::max() / __r
           ? _Up(0)
           : std::__generate_canonical_rk<_Up>(__r, __d, __pow * __r);
}

// Returns the number of bits needed to represent __x.
template <class _Up>
_LIBCPP_HIDE_FROM_ABI constexpr size_t __generate_canonical_bit_width(_Up __x) {
  return __x == 0 ? 0 : 1 + std::__generate_canonical_bit_width<_Up>(__x >> 1);
}

// Returns the smallest k such that __r^k >= __rk, where __rk is a power of __r.
template <class _Up>
_LIBCPP_HIDE_FROM_ABI constexpr size_t __generate_canonical_k(_Up __r, _Up __rk) {
  return __rk == 1 ? 0 : 1 + std::__generate_canonical_k<_Up>(__r, __rk / __r);
}

// R = 2^__n: k = ceil(d / n) calls are concatenated, and the low k * n - d bits are dropped.
template <class _RealType, size_t __d, class _URNG>
_LIBCPP_HIDE_FROM_ABI _RealType __generate_canonical_pow2(_URNG& __g) {
  using _Rng _LIBCPP_NODEBUG  = __generate_canonical_range_t<_URNG>;
  using _UInt _LIBCPP_NODEBUG = __conditional_t<__d <= 64, unsigned long long, __generate_canonical_wide_uint>;
  constexpr _Rng __rm1        = _URNG::max() - _URNG::min();
  constexpr size_t __n        = std::__generate_canonical_bit_width<_Rng>(__rm1);
  constexpr size_t __k        = (__d + __n - 1) / __n;
  constexpr size_t __drop     = __k * __n - __d;
  constexpr _RealType __scale = std::__generate_canonical_exp2_neg<_RealType>(__d);
  _UInt __q                   = static_cast<_UInt>(static_cast<_Rng>(__g() - _URNG::min()) >> __drop);
  for (size_t __i = 1; __i < __k; ++__i)
    __q |= static_cast<_UInt>(static_cast<_Rng>(__g() - _URNG::min())) << (__i * __n - __drop);
  return static_cast<_RealType>(__q) * __scale;
}

// R is not a power of two, and R^k fits into _Up.
template <class _RealType, size_t __d, class _URNG, class _Up>
_LIBCPP_HIDE_FROM_ABI _RealType __generate_canonical_any(_URNG& __g) {
  using _Rng _LIBCPP_NODEBUG  = __generate_canonical_range_t<_URNG>;
  using _UInt _LIBCPP_NODEBUG = __conditional_t<__d <= 64, unsigned long long, _Up>;
  constexpr _Up __r           = static_cast<_Up>(static_cast<_Rng>(_URNG::max() - _URNG::min())) + 1;
  constexpr _Up __rk          = std::__generate_canonical_rk<_Up>(__r, __d);
  constexpr size_t __k        = std::__generate_canonical_k<_Up>(__r, __rk);
  constexpr _Up __x           = __rk >> __d;
  constexpr _Up __limit       = __x << __d;
  constexpr _RealType __scale = std::__generate_canonical_exp2_neg<_RealType>(__d);
  while (true) {
    _Up __s   = 0;
    _Up __pow = 1;
    for (size_t __i = 0; __i < __k; ++__i) {
      __s += static_cast<_Up>(static_cast<_Rng>(__g() - _URNG::min())) * __pow;
      __pow *= __r;
    }
    if (__s < __limit)
      return static_cast<_RealType>(static_cast<_UInt>(__s / __x)) * __scale;
  }
}

// Fallback for when R^k doesn't fit into the widest available integer type, e.g. for long double with more than 64
// digits or on targets without __int128. S and R^k are kept as little-endian arrays of 32-bit limbs.

// __acc += (__src * __m) << (32 * __shift)
template <size_t _Np>
_LIBCPP_HIDE_FROM_ABI void
__generate_canonical_muladd(uint32_t (&__acc)[_Np], const uint32_t (&__src)[_Np], uint32_t __m, size_t __shift) {
  unsigned long long __carry = 0;
  for (size_t __i = 0; __i + __shift < _Np; ++__i) {
    unsigned long long __v = static_cast<unsigned long long>(__src[__i]) * __m + __acc[__i + __shift] + __carry;
    __acc[__i + __shift]   = static_cast<uint32_t>(__v);
    __carry                = __v >> 32;
  }
}

// Returns __a >> __e, which must fit into _Tp.
template <class _Tp, size_t _Np>
_LIBCPP_HIDE_FROM_ABI _Tp __generate_canonical_shr(const uint32_t (&__a)[_Np], size_t __e) {
  size_t __w   = __e / 32;
  size_t __b   = __e % 32;
  _Tp __result = __w < _Np ? static_cast<_Tp>(__a[__w] >> __b) : 0;
  for (size_t __j = 1; __j <= sizeof(_Tp) / 4 && __w + __j < _Np; ++__j) {
    size_t __shift = 32 * __j - __b;
    if (__shift < static_cast<size_t>(numeric_limits<_Tp>::digits))
      __result |= static_cast<_Tp>(__a[__w + __j]) << __shift;
  }
  return __result;
}

template <class _RealType, size_t __d, class _URNG>
_LIBCPP_HIDE_FROM_ABI _RealType __generate_canonical_bigint(_URNG& __g) {
  using _Rng _LIBCPP_NODEBUG     = __generate_canonical_range_t<_URNG>;
  constexpr size_t __range_limbs = sizeof(_Rng) / 4;
  // S < R^k < R * 2^d <= 2^(32 * __range_limbs + d)
  constexpr size_t __n        = (__d + 32 * __range_limbs) / 32 + 1;
  constexpr _Rng __rm1        = _URNG::max() - _URNG::min();
  constexpr _RealType __scale = std::__generate_canonical_exp2_neg<_RealType>(__d);
  // R needs an additional limb, since it can be 2^64 or 2^128.
  uint32_t __r[__range_limbs + 1];
  for (size_t __j = 0; __j != __range_limbs; ++__j)
    __r[__j] = static_cast<uint32_t>((__rm1 + 1) >> (32 * __j));
  __r[__range_limbs] = __rm1 + 1 == 0 ? 1 : 0;
  while (true) {
    uint32_t __s[__n]   = {};
    uint32_t __pow[__n] = {1};
    // Loop while R^i < 2^d, which makes exactly k calls to __g.
    while (std::__generate_canonical_shr<_Rng>(__pow, __d) == 0) {
      _Rng __v = static_cast<_Rng>(__g() - _URNG::min());
      for (size_t __j = 0; __j != __range_limbs; ++__j)
        std::__generate_canonical_muladd(__s, __pow, static_cast<uint32_t>(__v >> (32 * __j)), __j);
      uint32_t __next[__n] = {};
      for (size_t __j = 0; __j != __range_limbs + 1; ++__j)
        std::__generate_canonical_muladd(__next, __pow, __r[__j], __j);
      for (size_t __i = 0; __i != __n; ++__i)
        __pow[__i] = __next[__i];
    }
    // x = floor(R^k / 2^d) < R, and S < x * 2^d iff floor(S / 2^d) < x.
    _Rng __x = std::__generate_canonical_shr<_Rng>(__pow, __d);
    if (std::__generate_canonical_shr<_Rng>(__s, __d) >= __x)
      continue;
    // q = floor(S / x) < 2^d, computed by binary long division.
    uint32_t __q[__n] = {};
    _Rng __rem        = 0;
    for (size_t __bit = __n * 32; __bit-- > 0;) {
      bool __carry = (__rem >> (numeric_limits<_Rng>::digits - 1)) != 0;
      __rem        = (__rem << 1) | ((__s[__bit / 32] >> (__bit % 32)) & 1);
      if (__carry || __rem >= __x) {
        __rem -= __x;
        __q[__bit / 32] |= uint32_t(1) << (__bit % 32);
      }
    }
    // Every partial result is an integer smaller than 2^d, so this is exact.
    _RealType __result = 0;
    for (size_t __i = __n; __i-- > 0;)
      __result = __result * _RealType(4294967296.0) + static_cast<_RealType>(__q[__i]);
    return __result * __scale;
  }
}

template <class _RealType, size_t __digits, class _URNG>
[[__nodiscard__]] _LIBCPP_HIDE_FROM_ABI _RealType generate_canonical(_URNG& __g) {
  static_assert(
      numeric_limits<_RealType>::radix == 2, "generate_canonical only supports floating-point types with radix 2");
  using _Wide _LIBCPP_NODEBUG    = __generate_canonical_wide_uint;
  using _Rng _LIBCPP_NODEBUG     = __generate_canonical_range_t<_URNG>;
  constexpr size_t __real_digits = numeric_limits<_RealType>::digits;
  constexpr size_t __d           = __digits < __real_digits ? __digits : __real_digits;
  constexpr _Rng __rm1           = _URNG::max() - _URNG::min();
  constexpr bool __is_pow2       = (__rm1 & (__rm1 + 1)) == 0;
  if constexpr (__d == 0) {
    return _RealType(0);
  } else if constexpr (__is_pow2 && __d <= static_cast<size_t>(numeric_limits<_Wide>::digits)) {
    return std::__generate_canonical_pow2<_RealType, __d>(__g);
  } else if constexpr (!__is_pow2 && __rm1 < numeric_limits<unsigned long long>::max() &&
                       std::__generate_canonical_rk<unsigned long long>(
                           static_cast<unsigned long long>(__rm1) + 1, __d) != 0) {
    return std::__generate_canonical_any<_RealType, __d, _URNG, unsigned long long>(__g);
  } else if constexpr (!__is_pow2 && std::__generate_canonical_rk<_Wide>(_Wide(__rm1) + 1, __d) != 0) {
    return std::__generate_canonical_any<_RealType, __d, _URNG, _Wide>(__g);
  } else {
    return std::__generate_canonical_bigint<_RealType, __d>(__g);
  }
}

#else // _LIBCPP_CXX03_LANG

template <class _RealType, size_t __bits, class _URNG>
[[__nodiscard__]] _LIBCPP_HIDE_FROM_ABI _RealType generate_canonical(_URNG& __g) {
  const size_t __dt    = numeric_limits<_RealType>::digits;
  const size_t __b     = __dt < __bits ? __dt : __bits;
  const size_t __log_r = __log2<uint64_t, _URNG::_Max - _URNG::_Min + uint64_t(1)>::value;
  const size_t __k     = __b / __log_r + (__b % __log_r != 0) + (__b == 0);
  const _RealType __rp = static_cast<_RealType>(_URNG::max() - _URNG::min()) + _RealType(1);
  _RealType __base     = __rp;
  _RealType __sp       = __g() - _URNG::min();
  for (size_t __i = 1; __i < __k; ++__i, __base *= __rp)
    __sp += (__g() - _URNG::min()) * __base;
  return __sp / __base;
}

#endif // _LIBCPP_CXX03_LANG

_LIBCPP_END_NAMESPACE_STD

_LIBCPP_POP_MACROS

#endif // _LIBCPP___RANDOM_GENERATE_CANONICAL_H
