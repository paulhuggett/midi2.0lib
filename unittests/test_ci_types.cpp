//===-- ci types --------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include <gtest/gtest.h>

#include <utility>

#include "midi2/ci/ci_types.hpp"

namespace {

using midi2::ci::b14;
using midi2::ci::b28;
using midi2::ci::b7;
using midi2::ci::byte_array;
using midi2::ci::details::from_le7;

constexpr auto zero = std::byte{0U};
constexpr auto one = std::byte{1};
constexpr auto max = std::byte{0x7F};

TEST(LE7, TwoByte) {
  EXPECT_EQ(from_le7(byte_array<2>{zero, zero}), b14{0U});
  EXPECT_EQ(from_le7(byte_array<2>{one, zero}), b14{1U});
  EXPECT_EQ(from_le7(byte_array<2>{std::byte{2}, zero}), b14{2U});
  EXPECT_EQ(from_le7(byte_array<2>{max, zero}), b14{0x7FU});
  EXPECT_EQ(from_le7(byte_array<2>{zero, one}), b14{0x80U});
  EXPECT_EQ(from_le7(byte_array<2>{zero, max}), b14{0x3F80U});
  EXPECT_EQ(from_le7(byte_array<2>{max, max}), b14{0x3FFFU});
}

TEST(LE7, FourByte) {
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, zero, zero}), b28{0U});
  EXPECT_EQ(from_le7(byte_array<4>{one, zero, zero, zero}), b28{1U});
  EXPECT_EQ(from_le7(byte_array<4>{std::byte{2}, zero, zero, zero}), b28{2U});
  EXPECT_EQ(from_le7(byte_array<4>{max, zero, zero, zero}), b28{0x7FU});
  EXPECT_EQ(from_le7(byte_array<4>{zero, one, zero, zero}), b28{0x80U});
  EXPECT_EQ(from_le7(byte_array<4>{zero, max, zero, zero}), b28{0x3F80U});
  EXPECT_EQ(from_le7(byte_array<4>{max, max, zero, zero}), b28{0x3FFFU});
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, one, zero}), b28{0x4000U});
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, max, zero}), b28{0x1FC000U});
  EXPECT_EQ(from_le7(byte_array<4>{max, max, max, zero}), b28{0x1FFFFFU});
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, zero, one}), b28{0x200000U});
  EXPECT_EQ(from_le7(byte_array<4>{zero, zero, zero, max}), b28{0xFE00000U});
  EXPECT_EQ(from_le7(byte_array<4>{max, max, max, max}), b28{0xFFFFFFFU});
}

// M2-103-UM Common Rules for Property Exchange v1.2, Table 15 Reply Status Codes.
TEST(PEStatus, ValuesMatchSpecification) {
  using midi2::ci::pe_status;
  EXPECT_EQ(std::to_underlying(pe_status::ok), 200);
  EXPECT_EQ(std::to_underlying(pe_status::accepted), 201);
  EXPECT_EQ(std::to_underlying(pe_status::resource_unavailable), 341);
  EXPECT_EQ(std::to_underlying(pe_status::bad_data), 342);
  EXPECT_EQ(std::to_underlying(pe_status::too_many_reqs), 343);
  EXPECT_EQ(std::to_underlying(pe_status::bad_req), 400);
  EXPECT_EQ(std::to_underlying(pe_status::req_unauthorized), 403);
  EXPECT_EQ(std::to_underlying(pe_status::resource_unsupported), 404);
  EXPECT_EQ(std::to_underlying(pe_status::resource_not_allowed), 405);
  EXPECT_EQ(std::to_underlying(pe_status::flow_control_not_supported), 406);
  EXPECT_EQ(std::to_underlying(pe_status::flow_control_required), 407);
  EXPECT_EQ(std::to_underlying(pe_status::payload_too_large), 413);
  EXPECT_EQ(std::to_underlying(pe_status::unsupported_media_type), 415);
  EXPECT_EQ(std::to_underlying(pe_status::invalid_data_version), 445);
  EXPECT_EQ(std::to_underlying(pe_status::internal_device_error), 500);
}

}  // end anonymous namespace
