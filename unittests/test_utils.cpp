//===-- utils -----------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/utils.hpp"

// Standard library
#include <cstdint>

namespace {

// one_of<>
static_assert(midi2::one_of<int, int>);
static_assert(midi2::one_of<int, char, int, long>);
static_assert(midi2::one_of<std::uint8_t, unsigned char>);
static_assert(!midi2::one_of<int>, "An empty pack matches nothing");
static_assert(!midi2::one_of<int, char, long>);
static_assert(!midi2::one_of<int const, int>, "cv-qualification must match");
static_assert(!midi2::one_of<int&, int>, "A reference is not the referenced type");

}  // end anonymous namespace
