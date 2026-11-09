//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___RANDOM_PHILOX_ENGINE_H
#define _LIBCPP___RANDOM_PHILOX_ENGINE_H

#include <__config>
#include <__cstddef/size_t.h>
#include <__random/is_seed_sequence.h>
#include <__type_traits/enable_if.h>
#include <array>
#include <cstdint>
#include <iosfwd>
#include <limits>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_PUSH_MACROS
#include <__undef_macros>

#if _LIBCPP_STD_VER >= 26

_LIBCPP_BEGIN_NAMESPACE_STD

// Returns {__consts[__offset], __consts[__offset + 2], ...}, i.e. the multipliers (offset 0) or the round
// constants (offset 1) of a philox_engine.
template <class _UIntType, size_t __n, size_t __offset, _UIntType... __consts>
_LIBCPP_HIDE_FROM_ABI constexpr array<_UIntType, __n / 2> __philox_select_consts() {
  array<_UIntType, __n / 2> __result{};
  if constexpr (sizeof...(__consts) == __n) {
    const _UIntType __all[] = {__consts...};
    for (size_t __k = 0; __k != __n / 2; ++__k)
      __result[__k] = __all[2 * __k + __offset];
  }
  return __result;
}

template <class _UIntType, size_t __w, size_t __n, size_t __r, _UIntType... __consts>
class philox_engine {
  static_assert(sizeof...(__consts) == __n, "philox_engine: sizeof...(consts) must be equal to n");
  static_assert(__n == 2 || __n == 4, "philox_engine: n must be 2 or 4");
  static_assert(0 < __r, "philox_engine: r must be greater than 0");
  static_assert(0 < __w && __w <= numeric_limits<_UIntType>::digits,
                "philox_engine: w must be in the range [1, numeric_limits<UIntType>::digits]");

public:
  // types
  using result_type = _UIntType;

private:
  static constexpr size_t __array_size = __n / 2;
  static constexpr result_type __mask =
      __w >= numeric_limits<result_type>::digits
          ? result_type(~result_type(0))
          : result_type((result_type(1) << __w) - 1);

public:
  // engine characteristics
  static constexpr size_t word_size   = __w;
  static constexpr size_t word_count  = __n;
  static constexpr size_t round_count = __r;
  static constexpr array<result_type, __array_size> multipliers =
      std::__philox_select_consts<result_type, __n, 0, __consts...>();
  static constexpr array<result_type, __array_size> round_consts =
      std::__philox_select_consts<result_type, __n, 1, __consts...>();
  [[__nodiscard__]] _LIBCPP_HIDE_FROM_ABI static constexpr result_type min() { return 0; }
  [[__nodiscard__]] _LIBCPP_HIDE_FROM_ABI static constexpr result_type max() { return __mask; }
  static constexpr result_type default_seed = static_cast<result_type>(20111115u);

  // constructors and seeding functions
  _LIBCPP_HIDE_FROM_ABI philox_engine() : philox_engine(default_seed) {}
  _LIBCPP_HIDE_FROM_ABI explicit philox_engine(result_type __value) { seed(__value); }
  template <class _Sseq, __enable_if_t<__is_seed_sequence_v<_Sseq, philox_engine>, int> = 0>
  _LIBCPP_HIDE_FROM_ABI explicit philox_engine(_Sseq& __q) {
    seed(__q);
  }

  _LIBCPP_HIDE_FROM_ABI void seed(result_type __value = default_seed) {
    for (size_t __k = 0; __k != __array_size; ++__k)
      __k_[__k] = 0;
    __k_[0] = __value & __mask;
    __reset_counter();
  }

  template <class _Sseq, __enable_if_t<__is_seed_sequence_v<_Sseq, philox_engine>, int> = 0>
  _LIBCPP_HIDE_FROM_ABI void seed(_Sseq& __q) {
    constexpr size_t __p = (__w + 31) / 32;
    uint_least32_t __a[__array_size * __p];
    __q.generate(__a + 0, __a + __array_size * __p);
    for (size_t __k = 0; __k != __array_size; ++__k) {
      result_type __key = 0;
      for (size_t __j = 0; __j != __p; ++__j)
        __key |= static_cast<result_type>(static_cast<result_type>(__a[__k * __p + __j] & 0xFFFFFFFFu) << (32 * __j));
      __k_[__k] = __key & __mask;
    }
    __reset_counter();
  }

  _LIBCPP_HIDE_FROM_ABI void set_counter(const array<result_type, __n>& __counter) {
    for (size_t __j = 0; __j != __n; ++__j)
      __x_[__j] = __counter[__n - 1 - __j] & __mask;
    __i_ = __n - 1;
  }

  // equality operators
  // The values in __y_ are fully determined by __k_ and __x_ whenever they will be used again, so they don't have to be
  // compared.
  _LIBCPP_HIDE_FROM_ABI friend bool operator==(const philox_engine& __x, const philox_engine& __y) {
    if (__x.__i_ != __y.__i_)
      return false;
    for (size_t __k = 0; __k != __array_size; ++__k)
      if (__x.__k_[__k] != __y.__k_[__k])
        return false;
    for (size_t __j = 0; __j != __n; ++__j)
      if (__x.__x_[__j] != __y.__x_[__j])
        return false;
    return true;
  }

  // generating functions
  [[__nodiscard__]] _LIBCPP_HIDE_FROM_ABI result_type operator()() {
    if (++__i_ == __n) {
      __generate_block();
      __increment_counter();
      __i_ = 0;
    }
    return __y_[__i_];
  }

  // Philox is a counter-based engine, so discarding values only requires adjusting the counter.
  _LIBCPP_HIDE_FROM_ABI void discard(unsigned long long __z) {
    const size_t __left_in_block = __n - 1 - __i_;
    if (__z <= __left_in_block) {
      __i_ += static_cast<size_t>(__z);
      return;
    }
    __z -= __left_in_block;
    // We are now at the end of a block, and __z > 0 more values have to be generated.
    __add_to_counter((__z - 1) / __n);
    __generate_block();
    __increment_counter();
    __i_ = static_cast<size_t>((__z - 1) % __n);
  }

  // inserters and extractors
  template <class _CharT, class _Traits>
  _LIBCPP_HIDE_FROM_ABI friend basic_ostream<_CharT, _Traits>&
  operator<<(basic_ostream<_CharT, _Traits>& __os, const philox_engine& __x) {
    __save_flags<_CharT, _Traits> __lx(__os);
    using _Ostream = basic_ostream<_CharT, _Traits>;
    __os.flags(_Ostream::dec | _Ostream::left);
    _CharT __sp = __os.widen(' ');
    __os.fill(__sp);
    for (size_t __k = 0; __k != __array_size; ++__k)
      __os << __x.__k_[__k] << __sp;
    for (size_t __j = 0; __j != __n; ++__j)
      __os << __x.__x_[__j] << __sp;
    __os << __x.__i_;
    return __os;
  }

  template <class _CharT, class _Traits>
  _LIBCPP_HIDE_FROM_ABI friend basic_istream<_CharT, _Traits>&
  operator>>(basic_istream<_CharT, _Traits>& __is, philox_engine& __x) {
    __save_flags<_CharT, _Traits> __lx(__is);
    using _Istream = basic_istream<_CharT, _Traits>;
    __is.flags(_Istream::dec | _Istream::skipws);
    result_type __k[__array_size];
    result_type __c[__n];
    size_t __i;
    for (size_t __j = 0; __j != __array_size; ++__j)
      __is >> __k[__j];
    for (size_t __j = 0; __j != __n; ++__j)
      __is >> __c[__j];
    __is >> __i;
    if (__is.fail())
      return __is;
    if (__i >= __n) {
      __is.setstate(_Istream::failbit);
      return __is;
    }
    for (size_t __j = 0; __j != __array_size; ++__j)
      __x.__k_[__j] = __k[__j] & __mask;
    for (size_t __j = 0; __j != __n; ++__j)
      __x.__x_[__j] = __c[__j] & __mask;
    __x.__i_ = __i;
    // Y is the output of the block for the previous value of the counter.
    __x.__decrement_counter();
    __x.__generate_block();
    __x.__increment_counter();
    return __is;
  }

private:
  result_type __x_[__n];          // counter
  result_type __k_[__array_size]; // keys
  result_type __y_[__n];          // output of the last block
  size_t __i_;                    // index of the last value returned from __y_

  _LIBCPP_HIDE_FROM_ABI void __reset_counter() {
    for (size_t __j = 0; __j != __n; ++__j) {
      __x_[__j] = 0;
      __y_[__j] = 0;
    }
    __i_ = __n - 1;
  }

  // Computes the high and the low __w bits of the product __a * __b, where __a < 2^w. If __b >= 2^w, the high
  // part is reduced modulo 2^w as well, so that every element of the state stays within __w bits.
  template <bool __narrow>
  _LIBCPP_HIDE_FROM_ABI static void __mulhilo(result_type __a, result_type __b, result_type& __hi, result_type& __lo)
      _LIBCPP_DISABLE_UBSAN_UNSIGNED_INTEGER_CHECK {
    if constexpr (__narrow && __w <= 32) {
      uint_least64_t __p = static_cast<uint_least64_t>(__a) * __b;
      __hi               = static_cast<result_type>(__p >> __w);
      __lo               = static_cast<result_type>(__p & __mask);
    } else {
#  if _LIBCPP_HAS_INT128
      __uint128_t __p = static_cast<__uint128_t>(__a) * __b;
      __hi            = static_cast<result_type>((__p >> __w) & __mask);
      __lo            = static_cast<result_type>(__p & __mask);
#  else
      uint_least64_t __a_lo = __a & 0xFFFFFFFFu;
      uint_least64_t __a_hi = static_cast<uint_least64_t>(__a) >> 32;
      uint_least64_t __b_lo = __b & 0xFFFFFFFFu;
      uint_least64_t __b_hi = static_cast<uint_least64_t>(__b) >> 32;
      uint_least64_t __ll   = __a_lo * __b_lo;
      uint_least64_t __lh   = __a_lo * __b_hi;
      uint_least64_t __hl   = __a_hi * __b_lo;
      uint_least64_t __mid  = (__ll >> 32) + (__lh & 0xFFFFFFFFu) + (__hl & 0xFFFFFFFFu);
      uint_least64_t __p_hi = __a_hi * __b_hi + (__lh >> 32) + (__hl >> 32) + (__mid >> 32);
      uint_least64_t __p_lo = (__mid << 32) | (__ll & 0xFFFFFFFFu);
      // __p >> __w < 2^64, since __a < 2^w
      if constexpr (__w == 64) {
        __hi = static_cast<result_type>(__p_hi);
        __lo = static_cast<result_type>(__p_lo);
      } else {
        __hi = static_cast<result_type>(((__p_hi << (64 - __w)) | (__p_lo >> __w)) & __mask);
        __lo = static_cast<result_type>(__p_lo & __mask);
      }
#  endif
    }
  }

  // __y_ = Philox(__k_, __x_)
  _LIBCPP_HIDE_FROM_ABI void __generate_block() _LIBCPP_DISABLE_UBSAN_UNSIGNED_INTEGER_CHECK {
    result_type __v[__n];
    result_type __key[__array_size];
    for (size_t __j = 0; __j != __n; ++__j)
      __v[__j] = __x_[__j];
    for (size_t __k = 0; __k != __array_size; ++__k)
      __key[__k] = __k_[__k];
    for (size_t __q = 0; __q != __r; ++__q) {
      if constexpr (__n == 2) {
        result_type __hi, __lo;
        __mulhilo<__fits(0)>(__v[0], multipliers[0], __hi, __lo);
        __v[0] = (__hi ^ __key[0] ^ __v[1]) & __mask;
        __v[1] = __lo;
      } else {
        // The permutation is V = (X2, X1, X0, X3).
        result_type __hi0, __lo0, __hi1, __lo1;
        __mulhilo<__fits(0)>(__v[2], multipliers[0], __hi0, __lo0);
        __mulhilo<__fits(1)>(__v[0], multipliers[1], __hi1, __lo1);
        __v[0] = (__hi0 ^ __key[0] ^ __v[1]) & __mask;
        __v[1] = __lo0;
        __v[2] = (__hi1 ^ __key[1] ^ __v[3]) & __mask;
        __v[3] = __lo1;
      }
      for (size_t __k = 0; __k != __array_size; ++__k)
        __key[__k] = (__key[__k] + __round_const(__k)) & __mask;
    }
    for (size_t __j = 0; __j != __n; ++__j)
      __y_[__j] = __v[__j];
  }

  // Whether the multiplier fits into __w bits
  _LIBCPP_HIDE_FROM_ABI static constexpr bool __fits(size_t __k) { return multipliers[__k] <= __mask; }
  // The keys are computed modulo 2^w, so the round constants can be reduced.
  _LIBCPP_HIDE_FROM_ABI static constexpr result_type __round_const(size_t __k) { return round_consts[__k] & __mask; }

  // Z = Z + 1 (mod 2^(n * w))
  _LIBCPP_HIDE_FROM_ABI void __increment_counter() _LIBCPP_DISABLE_UBSAN_UNSIGNED_INTEGER_CHECK {
    for (size_t __j = 0; __j != __n; ++__j) {
      __x_[__j] = (__x_[__j] + 1) & __mask;
      if (__x_[__j] != 0)
        return;
    }
  }

  // Z = Z - 1 (mod 2^(n * w))
  _LIBCPP_HIDE_FROM_ABI void __decrement_counter() _LIBCPP_DISABLE_UBSAN_UNSIGNED_INTEGER_CHECK {
    for (size_t __j = 0; __j != __n; ++__j) {
      bool __borrow = __x_[__j] == 0;
      __x_[__j]     = (__x_[__j] - 1) & __mask;
      if (!__borrow)
        return;
    }
  }

  // Z = Z + __v (mod 2^(n * w))
  _LIBCPP_HIDE_FROM_ABI void __add_to_counter(unsigned long long __v) _LIBCPP_DISABLE_UBSAN_UNSIGNED_INTEGER_CHECK {
    bool __carry = false;
    for (size_t __j = 0; __j != __n && (__v != 0 || __carry); ++__j) {
      result_type __digit;
      if constexpr (__w >= numeric_limits<unsigned long long>::digits) {
        __digit = static_cast<result_type>(__v);
        __v     = 0;
      } else {
        __digit = static_cast<result_type>(__v & __mask);
        __v >>= __w;
      }
      result_type __old = __x_[__j];
      result_type __sum = (__old + __digit + __carry) & __mask;
      __carry           = __sum < __old || (__carry && __sum == __old);
      __x_[__j]         = __sum;
    }
  }
};

using philox4x32 = philox_engine<uint_fast32_t, 32, 4, 10, 0xCD9E8D57, 0x9E3779B9, 0xD2511F53, 0xBB67AE85>;

using philox4x64 =
    philox_engine<uint_fast64_t,
                  64,
                  4,
                  10,
                  0xCA5A826395121157,
                  0x9E3779B97F4A7C15,
                  0xD2E7470EE14C6C93,
                  0xBB67AE8584CAA73B>;

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP_STD_VER >= 26

_LIBCPP_POP_MACROS

#endif // _LIBCPP___RANDOM_PHILOX_ENGINE_H
