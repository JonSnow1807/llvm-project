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

// using result_type = UIntType;

#include <cstdint>
#include <random>
#include <type_traits>

#include "philox_engines.h"
#include "test_macros.h"

static_assert(std::is_same_v<std::philox4x32::result_type, std::uint_fast32_t>);
static_assert(std::is_same_v<std::philox4x64::result_type, std::uint_fast64_t>);
static_assert(std::is_same_v<philox4x48::result_type, std::uint64_t>);
static_assert(std::is_same_v<philox2x16::result_type, unsigned short>);

int main(int, char**) { return 0; }
