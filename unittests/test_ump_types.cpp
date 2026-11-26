//===-- UMP -------------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// Force the UMP text-common message to include the str() method.
#define MIDI2_TEXT_COMMON_STRING (1)

#include "midi2/ump/ump_types.hpp"

// Standard library
#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <tuple>

// Google test
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace {

template <typename T> class UMPCheckFixture : public testing::Test {
public:
  T value_;
};
using ump_types = ::testing::Types<
    midi2::ump::utility::noop, midi2::ump::utility::jr_clock, midi2::ump::utility::jr_timestamp,
    midi2::ump::utility::delta_clockstamp_tpqn, midi2::ump::utility::delta_clockstamp,

    midi2::ump::system::midi_time_code, midi2::ump::system::song_position_pointer, midi2::ump::system::song_select,
    midi2::ump::system::tune_request, midi2::ump::system::timing_clock, midi2::ump::system::sequence_start,
    midi2::ump::system::sequence_continue, midi2::ump::system::sequence_stop, midi2::ump::system::active_sensing,
    midi2::ump::system::reset,

    midi2::ump::m1cvm::note_on, midi2::ump::m1cvm::note_off, midi2::ump::m1cvm::poly_pressure,
    midi2::ump::m1cvm::control_change, midi2::ump::m1cvm::program_change, midi2::ump::m1cvm::channel_pressure,
    midi2::ump::m1cvm::pitch_bend,

    midi2::ump::m2cvm::note_off, midi2::ump::m2cvm::note_on, midi2::ump::m2cvm::poly_pressure,
    midi2::ump::m2cvm::rpn_per_note_controller, midi2::ump::m2cvm::nrpn_per_note_controller,
    midi2::ump::m2cvm::rpn_controller, midi2::ump::m2cvm::nrpn_controller, midi2::ump::m2cvm::rpn_relative_controller,
    midi2::ump::m2cvm::nrpn_relative_controller, midi2::ump::m2cvm::per_note_management,
    midi2::ump::m2cvm::control_change, midi2::ump::m2cvm::program_change, midi2::ump::m2cvm::channel_pressure,
    midi2::ump::m2cvm::pitch_bend, midi2::ump::m2cvm::per_note_pitch_bend,

    midi2::ump::stream::endpoint_discovery, midi2::ump::stream::endpoint_info_notification,
    midi2::ump::stream::device_identity_notification, midi2::ump::stream::endpoint_name_notification,
    midi2::ump::stream::product_instance_id_notification, midi2::ump::stream::jr_configuration_request,
    midi2::ump::stream::jr_configuration_notification, midi2::ump::stream::function_block_discovery,
    midi2::ump::stream::function_block_info_notification, midi2::ump::stream::function_block_name_notification,
    midi2::ump::stream::start_of_clip, midi2::ump::stream::end_of_clip,

    midi2::ump::flex_data::set_tempo, midi2::ump::flex_data::set_time_signature, midi2::ump::flex_data::set_metronome,
    midi2::ump::flex_data::set_key_signature, midi2::ump::flex_data::set_chord_name, midi2::ump::flex_data::text_common,

    midi2::ump::data128::sysex8_in_1, midi2::ump::data128::sysex8_start, midi2::ump::data128::sysex8_continue,
    midi2::ump::data128::sysex8_end, midi2::ump::data128::mds_header, midi2::ump::data128::mds_payload>;
TYPED_TEST_SUITE(UMPCheckFixture, ump_types);
TYPED_TEST(UMPCheckFixture, Check) {
  ASSERT_TRUE(midi2::ump::check(this->value_));
}

// M2-104-UM Figure 64: word 1 is r (bit 31), program (bits 30-24), reserved (23-16), r (15), bank msb (14-8),
// r (7), bank lsb (6-0).
TEST(UMPTypesM2CVMProgramChange, ProgramFieldLayout) {
  constexpr auto message = midi2::ump::m2cvm::program_change{}.program(0x7F);
  EXPECT_EQ(std::uint32_t{get<1>(message)}, 0x7F000000U);
  EXPECT_EQ(message.program(), 0x7F);
}
TEST(UMPTypesM2CVMProgramChange, ReservedBitIsNotPartOfProgram) {
  std::array<std::uint32_t, 2> words{std::uint32_t{get<0>(midi2::ump::m2cvm::program_change{})}, 0x85000000U};
  auto const message = midi2::ump::m2cvm::program_change{std::span{words}};
  EXPECT_EQ(message.program(), 0x05);
}
TEST(UMPTypesSysex7, AssignIndividually) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4);
  message2[0] = 0x7E;
  message2[1] = 0x7F;
  message2[2] = 0x07;
  message2[3] = 0x0D;
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, InitializerList) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, Range) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr std::array values{0x7E, 0x7F, 0x07, 0x0D};
  constexpr auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data(values);
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, RangeWithProjection) {
  // The elements of the range are std::byte, which is not integral: the projection converts them.
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr std::array values{std::byte{0x7E}, std::byte{0x7F}, std::byte{0x07}, std::byte{0x0D}};
  constexpr auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data(
      values, [](std::byte const b) { return std::to_integer<std::uint8_t>(b); });
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, TransformedRange) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr std::array values{std::byte{0x7E}, std::byte{0x7F}, std::byte{0x07}, std::byte{0x0D}};
  constexpr auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data(
      values | std::views::transform([](std::byte const b) { return std::to_integer<std::uint8_t>(b); }));
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, IteratorAssignData) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  constexpr std::array values{0x7E, 0x7F, 0x07, 0x0D};
  constexpr auto message2 = midi2::ump::data64::sysex7_in_1{}.group(0).data(values.begin(), values.end());
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex7, IteratorRead) {
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  auto first = message.begin();
  auto last = message.end();
  EXPECT_EQ(std::distance(first, last), 4U);
  EXPECT_EQ(*first, 0x7E);
  ++first;
  EXPECT_EQ(*first, 0x7F);
  ++first;
  EXPECT_EQ(*first, 0x07);
  ++first;
  EXPECT_EQ(*first, 0x0D);
  ++first;
  EXPECT_EQ(first, last);
}
TEST(UMPTypesSysex7, ConstIteratorRead) {
  constexpr auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  auto first = message.begin();
  auto last = message.end();
  EXPECT_EQ(std::distance(first, last), 4U);
  EXPECT_EQ(*first, 0x7E);
  ++first;
  EXPECT_EQ(*first, 0x7F);
  ++first;
  EXPECT_EQ(*first, 0x07);
  ++first;
  EXPECT_EQ(*first, 0x0D);
  ++first;
  EXPECT_EQ(first, last);
}
TEST(UMPTypesSysex7, IteratorWrite) {
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0);
  EXPECT_TRUE(message.empty());
  EXPECT_EQ(message.size(), 0U);
  message.data({1, 2, 3});
  EXPECT_FALSE(message.empty());
  EXPECT_EQ(message.size(), 3U);
  auto first = message.begin();
  auto last = message.end();
  *first = 4;
  EXPECT_EQ(*first, 4);
  ++first;
  EXPECT_EQ(*first, 2);
  ++first;
  EXPECT_EQ(*first, 3);
  ++first;
  EXPECT_EQ(first, last);
}
TEST(UMPTypesSysex7, IteratorWriteUsingAlgorithm) {
  using testing::ElementsAreArray;

  auto message = midi2::ump::data64::sysex7_in_1{}.group(0);
  constexpr std::array src{0x1, 0x3, 0x5, 0x7};
  std::ranges::copy(src, message.begin());
  using count_type = midi2::ump::data64::sysex7_in_1::word0::number_of_bytes::uinteger;
  message.number_of_bytes(static_cast<count_type>(src.size()));
  EXPECT_THAT(message, ElementsAreArray(src));
}
TEST(UMPTypesSysex7, IteratorSubtract) {
  using testing::ElementsAreArray;

  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  auto first = std::begin(message);
  auto last = std::end(message);
  EXPECT_EQ(last - first, 4U);
}
TEST(UMPTypesSysex7, IteratorSubscript) {
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  auto const first = message.begin();
  EXPECT_EQ(first[0], 0x1);
  EXPECT_EQ(first[3], 0x7);
  auto const last = message.end();
  EXPECT_EQ(last[-1], 0x7);
  EXPECT_EQ(last[-4], 0x1);
  first[1] = 0x9;
  EXPECT_EQ(message[1], 0x9);
}
TEST(UMPTypesSysex7, ConstIteratorSubscript) {
  constexpr auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  auto const first = message.begin();
  EXPECT_EQ(first[0], 0x1);
  EXPECT_EQ(first[2], 0x5);
}
TEST(UMPTypesSysex7, IteratorPostfix) {
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  auto it = message.begin();
  auto const prev = it++;
  EXPECT_EQ(prev, message.begin());
  EXPECT_EQ(*prev, 0x1);
  EXPECT_EQ(*it, 0x3);
  auto const prev2 = it--;
  EXPECT_EQ(*prev2, 0x3);
  EXPECT_EQ(it, message.begin());
}
TEST(UMPTypesSysex7, ProxyPostfix) {
  using proxy = midi2::ump::details::array_subscript_proxy<midi2::ump::data64::sysex7_in_1>;
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  auto p = proxy{message, 1};
  auto const prev = p++;
  EXPECT_EQ(prev.index(), 1U);
  EXPECT_EQ(p.index(), 2U);
  auto const prev2 = p--;
  EXPECT_EQ(prev2.index(), 2U);
  EXPECT_EQ(p.index(), 1U);
}
TEST(UMPTypesSysex7, IteratorConstAndNonConstCompare) {
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  auto const& cmessage = message;
  midi2::ump::data64::sysex7_in_1::const_iterator cit = cmessage.begin();
  EXPECT_TRUE(cit == message.begin());
  EXPECT_TRUE(cit < message.end());
  cit = message.begin() + 2;  // Assign a non-const iterator to a const iterator.
  EXPECT_EQ(*cit, 0x5);
}
TEST(UMPTypesSysex7, CopyBetweenMessages) {
  using testing::ElementsAre;
  auto const src = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  auto dest = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x0, 0x0, 0x0, 0x0});
  std::ranges::copy(src, dest.begin());
  EXPECT_THAT(dest, ElementsAre(0x1, 0x3, 0x5, 0x7));
}
TEST(UMPTypesSysex7, AssignProxyToProxy) {
  auto message = midi2::ump::data64::sysex7_in_1{}.group(0).data({0x1, 0x3, 0x5, 0x7});
  // Assigning one element to another must copy the value, not change which element the left-hand side refers to.
  *message.begin() = *(message.begin() + 3);
  EXPECT_EQ(message[0], 0x7);
  EXPECT_EQ(message[3], 0x7);
  message[1] = message[2];
  EXPECT_EQ(message[1], 0x5);
}
TEST(UMPTypesSysex7, ReadArray) {
  constexpr auto message =
      midi2::ump::data64::sysex7_in_1{}.group(0).number_of_bytes(4).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x00);
  EXPECT_EQ(message[5], 0x00);
}

// M2-104-UM section 7.5.7: a key signature has up to 7 sharps or 7 flats; -8 means unknown or non-standard.
TEST(UMPTypesFlexDataSetKeySignature, AllSharpsFlatsValuesAreValid) {
  using midi2::ump::flex_data::note;
  using midi2::ump::flex_data::set_key_signature;
  using midi2::ump::flex_data::sharps_flats;
  for (auto v = -8; v <= 7; ++v) {
    auto const sf = static_cast<sharps_flats>(v);
    auto const message = set_key_signature{}.group(0).sharps_flats(sf).tonic_note(note::c);
    EXPECT_TRUE(midi2::ump::check(message)) << "sharps/flats value " << v;
    EXPECT_EQ(message.sharps_flats(), sf);
    EXPECT_EQ(message.sharps_flats_raw(), v);
  }
}

TEST(UMPTypesFlexDataSetChordName, TonicSharpsFlatsOutsideEnumerationIsInvalid) {
  using midi2::ump::flex_data::set_chord_name;
  using midi2::ump::flex_data::sharps_flats;
  auto const message = set_chord_name{}.group(0).tonic_sharps_flats(static_cast<sharps_flats>(3));
  EXPECT_FALSE(midi2::ump::check(message));
}

// M2-104-UM Figure 51: the low byte of word 0 is option flags (bits 7-2), D (bit 1), S (bit 0).
TEST(UMPTypesM2CVMPerNoteManagement, FlagsAreIndependent) {
  using per_note_management = midi2::ump::m2cvm::per_note_management;
  constexpr per_note_management empty;
  auto const base = std::uint32_t{get<0>(empty)};

  auto const s = per_note_management{}.set_to_default(1);
  EXPECT_EQ(std::uint32_t{get<0>(s)}, base | 0x01U);
  EXPECT_EQ(s.option_flags(), 0U);
  EXPECT_EQ(s.detach(), 0U);

  auto const d = per_note_management{}.detach(1);
  EXPECT_EQ(std::uint32_t{get<0>(d)}, base | 0x02U);
  EXPECT_EQ(d.option_flags(), 0U);
  EXPECT_EQ(d.set_to_default(), 0U);

  auto const o = per_note_management{}.option_flags(0x3F);
  EXPECT_EQ(std::uint32_t{get<0>(o)}, base | 0xFCU);
  EXPECT_EQ(o.detach(), 0U);
  EXPECT_EQ(o.set_to_default(), 0U);
}

TEST(UMPTypesSysex8, AssignIndividually) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}.group(0).number_of_bytes(5).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  auto message2 = sysex8_in_1{}.group(0).number_of_bytes(5);
  message2[0] = 0x7E;
  message2[1] = 0x7F;
  message2[2] = 0x07;
  message2[3] = 0x0D;
  EXPECT_EQ(message, message2);
}
TEST(UMPTypesSysex8, AllElements) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}
                               .group(0)
                               .number_of_bytes(14)
                               .data0(0x7E)
                               .data1(0x7F)
                               .data2(0x07)
                               .data3(0x0D)
                               .data4(0x01)
                               .data5(0x02)
                               .data6(0x03)
                               .data7(0x04)
                               .data8(0x05)
                               .data9(0x06)
                               .data10(0x07)
                               .data11(0x08)
                               .data12(0x09);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x01);
  EXPECT_EQ(message[5], 0x02);
  EXPECT_EQ(message[6], 0x03);
  EXPECT_EQ(message[7], 0x04);
  EXPECT_EQ(message[8], 0x05);
  EXPECT_EQ(message[9], 0x06);
  EXPECT_EQ(message[10], 0x07);
  EXPECT_EQ(message[11], 0x08);
  EXPECT_EQ(message[12], 0x09);
}
TEST(UMPTypesSysex8, InitializerList) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D});
  ASSERT_EQ(message.number_of_bytes(), 5U);
  ASSERT_EQ(message.size(), 4U);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
}
TEST(UMPTypesSysex8, InitializerListMaxLength) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message =
      sysex8_in_1{}.group(0).data({0x7E, 0x7F, 0x07, 0x0D, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09});
  ASSERT_EQ(message.number_of_bytes(), 14U);
  ASSERT_EQ(message.size(), 13U);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x01);
  EXPECT_EQ(message[5], 0x02);
  EXPECT_EQ(message[6], 0x03);
  EXPECT_EQ(message[7], 0x04);
  EXPECT_EQ(message[8], 0x05);
  EXPECT_EQ(message[9], 0x06);
  EXPECT_EQ(message[10], 0x07);
  EXPECT_EQ(message[11], 0x08);
  EXPECT_EQ(message[12], 0x09);
}
TEST(UMPTypesSysex8, IteratorWriteUsingAlgorithm) {
  using testing::ElementsAreArray;
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  auto message = sysex8_in_1{}.group(0);
  constexpr std::array src{0x1, 0x3, 0x5, 0x7};
  std::ranges::copy(src, message.begin());
  using count_type = sysex8_in_1::word0::number_of_bytes::uinteger;
  message.number_of_bytes(static_cast<count_type>(src.size() + 1U));  // +1 for the stream ID
  EXPECT_THAT(message, ElementsAreArray(src));
}
TEST(UMPTypesSysex8, ReadArray) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}.group(0).number_of_bytes(5).data0(0x7E).data1(0x7F).data2(0x07).data3(0x0D);
  EXPECT_EQ(message[0], 0x7E);
  EXPECT_EQ(message[1], 0x7F);
  EXPECT_EQ(message[2], 0x07);
  EXPECT_EQ(message[3], 0x0D);
  EXPECT_EQ(message[4], 0x00);
  EXPECT_EQ(message[5], 0x00);
  EXPECT_EQ(message[6], 0x00);
  EXPECT_EQ(message[7], 0x00);
  EXPECT_EQ(message[8], 0x00);
  EXPECT_EQ(message[9], 0x00);
  EXPECT_EQ(message[10], 0x00);
  EXPECT_EQ(message[11], 0x00);
  EXPECT_EQ(message[12], 0x00);
}
// M2-104-UM section 7.8: "# of bytes" counts the stream ID as well as the data bytes (1-14). 0 is invalid and 0xF
// aborts the message.
TEST(UMPTypesSysex8, NumberOfBytesIncludesStreamId) {
  using testing::ElementsAre;
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr auto message = sysex8_in_1{}.group(0).stream_id(3).number_of_bytes(3).data0(0x11).data1(0x22).data2(0x33);
  EXPECT_EQ(message.size(), 2U);
  EXPECT_FALSE(message.empty());
  EXPECT_THAT(message, ElementsAre(0x11, 0x22));
}
TEST(UMPTypesSysex8, StreamIdOnlyIsEmpty) {
  using sysex8_end = midi2::ump::data128::sysex8_end;
  constexpr auto message = sysex8_end{}.group(0).number_of_bytes(1);
  EXPECT_EQ(message.size(), 0U);
  EXPECT_TRUE(message.empty());
  EXPECT_EQ(message.begin(), message.end());
}
TEST(UMPTypesSysex8, InvalidAndAbortCountsAreEmpty) {
  using sysex8_end = midi2::ump::data128::sysex8_end;
  EXPECT_TRUE(sysex8_end{}.group(0).number_of_bytes(0).empty());
  EXPECT_TRUE(sysex8_end{}.group(0).number_of_bytes(0xF).empty());
}
TEST(UMPTypesSysex8, DataFromRangeSetsNumberOfBytes) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr std::array src{0x01, 0x02, 0x03};
  auto const message = sysex8_in_1{}.group(0).data(src);
  EXPECT_EQ(message.number_of_bytes(), 4U);
  EXPECT_EQ(message.size(), 3U);
  auto const message2 = sysex8_in_1{}.group(0).data(src.begin(), src.end());
  EXPECT_EQ(message2.number_of_bytes(), 4U);
  EXPECT_EQ(message2.size(), 3U);
}
TEST(UMPTypesSysex8, DataFromRangeWithProjection) {
  using sysex8_in_1 = midi2::ump::data128::sysex8_in_1;
  constexpr std::array src{std::byte{0x01}, std::byte{0x02}, std::byte{0xFF}};
  auto const message =
      sysex8_in_1{}.group(0).data(src, [](std::byte const b) { return std::to_integer<std::uint8_t>(b); });
  EXPECT_EQ(message.number_of_bytes(), 4U);
  ASSERT_EQ(message.size(), 3U);
  EXPECT_EQ(message[0], 0x01U);
  EXPECT_EQ(message[1], 0x02U);
  EXPECT_EQ(message[2], 0xFFU);
}

TEST(UMPTypesTextCommon, InitializerList) {
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto message = text_common{}.group(0).data({'a', 'b', 'c', 'd'});
  ASSERT_EQ(message.number_of_bytes(), 4U);
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), u8"abcd");
#endif
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], '\0');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
TEST(UMPTypesTextCommon, InitializerListMaxLength) {
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto message = text_common{}.group(0).data({'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'});
  EXPECT_EQ(message.number_of_bytes(), 12U);
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), u8"abcdefghijkl");
#endif
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], 'h');
  EXPECT_EQ(message[8], 'i');
  EXPECT_EQ(message[9], 'j');
  EXPECT_EQ(message[10], 'k');
  EXPECT_EQ(message[11], 'l');
}
// data(u8string_view) must copy only the characters in the view: a string_view isn't null-terminated.
TEST(UMPTypesTextCommon, DataFromStringViewStopsAtEndOfView) {
  // NOLINTNEXTLINE(google-build-using-namespace)
  using namespace std::literals::string_view_literals;
  using text_common = midi2::ump::flex_data::text_common;
  // A const lvalue selects the data(std::u8string_view const&) overload rather than the range overload.
  constexpr auto view = u8"abcdef"sv.substr(0, 2);
  auto const message = text_common{}.group(0).data(view);
  EXPECT_EQ(message.size(), 2U);
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], '\0');
}
TEST(UMPTypesTextCommon, DataFromRangeWithProjection) {
  using text_common = midi2::ump::flex_data::text_common;
  constexpr std::array src{std::byte{'a'}, std::byte{'b'}};
  auto const message = text_common{}.group(0).data(src, [](std::byte const b) { return static_cast<char8_t>(b); });
  EXPECT_EQ(message.size(), 2U);
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], '\0');
}
TEST(UMPTypesTextCommon, DataFromStringViewOfUnterminatedArray) {
  using text_common = midi2::ump::flex_data::text_common;
  // Evaluating in a constant expression means that a read past the end of the array fails to compile.
  static constexpr std::array<char8_t, 3> chars{u8'x', u8'y', u8'z'};
  static constexpr std::u8string_view view{chars.data(), chars.size()};
  auto const message = text_common{}.group(0).data(view);
  EXPECT_EQ(message.size(), 3U);
  EXPECT_EQ(message[2], 'z');
}
TEST(UMPTypesTextCommon, DataFromStringViewTruncatesAtMaxSize) {
  // NOLINTNEXTLINE(google-build-using-namespace)
  using namespace std::literals::string_view_literals;
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto view = u8"abcdefghijklmn"sv;
  auto const message = text_common{}.group(0).data(view);
  EXPECT_EQ(message.size(), 12U);
  EXPECT_EQ(message[11], 'l');
}

TEST(UMPTypesTextCommon, DataSetByIteratorSentinel) {
  // NOLINTNEXTLINE(google-build-using-namespace)
  using namespace std::literals::string_view_literals;
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto str = u8"hello"sv;
  constexpr auto message = text_common{}.group(0).data(std::begin(str), std::end(str));
  ASSERT_EQ(message.number_of_bytes(), 5U);
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), str);
#endif
  EXPECT_EQ(message[0], 'h');
  EXPECT_EQ(message[1], 'e');
  EXPECT_EQ(message[2], 'l');
  EXPECT_EQ(message[3], 'l');
  EXPECT_EQ(message[4], 'o');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
TEST(UMPTypesTextCommon, IteratorWriteUsingAlgorithm) {
  using testing::ElementsAreArray;
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0);
  constexpr std::array src{'a', 'b', 'c', 'd'};
  std::ranges::copy(src, message.begin());
  EXPECT_THAT(message, ElementsAreArray(src));
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), u8"abcd");
#endif
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], '\0');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
TEST(UMPTypesTextCommon, IteratorWriteUsingAlgorithmMaxLength) {
  using testing::ElementsAreArray;
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0);
  constexpr std::array src{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'};
  std::ranges::copy(src, message.begin());
  EXPECT_THAT(message, ElementsAreArray(src));
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  EXPECT_EQ(message.str(), u8"abcdefghijkl");
#endif
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], 'h');
  EXPECT_EQ(message[8], 'i');
  EXPECT_EQ(message[9], 'j');
  EXPECT_EQ(message[10], 'k');
  EXPECT_EQ(message[11], 'l');
}
TEST(UMPTypesTextCommon, MaxSize) {
  EXPECT_EQ(midi2::ump::flex_data::text_common{}.max_size(), 12U);
}
TEST(UMPTypesTextCommon, ReadArray) {
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0).data({'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l'});
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], 'h');
  EXPECT_EQ(message[8], 'i');
  EXPECT_EQ(message[9], 'j');
  EXPECT_EQ(message[10], 'k');
  EXPECT_EQ(message[11], 'l');
  message.data({'a', 'b'});
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], '\0');
  EXPECT_EQ(message[3], '\0');
  EXPECT_EQ(message[4], '\0');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
TEST(UMPTypesTextCommon, String) {
  using text_common = midi2::ump::flex_data::text_common;
  constexpr auto message = text_common{}.group(0).data({'a', 'b', 'c', 'd', 'e', 'f', 'g'});
  EXPECT_EQ(message.str(), u8"abcdefg");
}
#endif  // MIDI2_TEXT_COMMON_STRING
TEST(UMPTypesTextCommon, FromStringView) {
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0).data("abcdefg");
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
  message.data("ab");
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], '\0');
  EXPECT_EQ(message[3], '\0');
  EXPECT_EQ(message[4], '\0');
  EXPECT_EQ(message[5], '\0');
  EXPECT_EQ(message[6], '\0');
  EXPECT_EQ(message[7], '\0');
  EXPECT_EQ(message[8], '\0');
  EXPECT_EQ(message[9], '\0');
  EXPECT_EQ(message[10], '\0');
  EXPECT_EQ(message[11], '\0');
}
TEST(UMPTypesTextCommon, FromStringTooLong) {
  using text_common = midi2::ump::flex_data::text_common;
  auto message = text_common{}.group(0).data("abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(message[0], 'a');
  EXPECT_EQ(message[1], 'b');
  EXPECT_EQ(message[2], 'c');
  EXPECT_EQ(message[3], 'd');
  EXPECT_EQ(message[4], 'e');
  EXPECT_EQ(message[5], 'f');
  EXPECT_EQ(message[6], 'g');
  EXPECT_EQ(message[7], 'h');
  EXPECT_EQ(message[8], 'i');
  EXPECT_EQ(message[9], 'j');
  EXPECT_EQ(message[10], 'k');
  EXPECT_EQ(message[11], 'l');
}

// ump_message
static_assert(midi2::ump::ump_message<midi2::ump::utility::noop>);
static_assert(midi2::ump::ump_message<midi2::ump::m1cvm::note_on>);
static_assert(midi2::ump::ump_message<midi2::ump::m2cvm::note_on>);
static_assert(midi2::ump::ump_message<midi2::ump::data64::sysex7_in_1>);
static_assert(midi2::ump::ump_message<midi2::ump::stream::endpoint_name_notification>);
static_assert(midi2::ump::ump_message<midi2::ump::flex_data::set_tempo>);
static_assert(!midi2::ump::ump_message<int>, "Not tuple-like");
static_assert(!midi2::ump::ump_message<std::uint32_t>, "Not tuple-like");
static_assert(!midi2::ump::ump_message<std::tuple<std::uint32_t, std::uint32_t>>, "Tuple-like but not UMP words");
static_assert(!midi2::ump::ump_message<std::array<std::uint32_t, 2>>, "Tuple-like but not UMP words");
static_assert(!midi2::ump::ump_message<std::tuple<midi2::ump::m2cvm::note_on::word0, std::uint32_t>>,
              "Only the first element is a UMP word");
static_assert(!midi2::ump::ump_message<std::tuple<>>, "Too few words");
static_assert(!midi2::ump::ump_message<std::tuple<midi2::ump::m2cvm::note_on::word0, midi2::ump::m2cvm::note_on::word0,
                                                  midi2::ump::m2cvm::note_on::word0, midi2::ump::m2cvm::note_on::word0,
                                                  midi2::ump::m2cvm::note_on::word0>>,
              "Too many words");

// The constraints on apply() and check().
/// A function for apply() which returns a value that converts to bool.
struct apply_bool {
  /// \returns False, so that apply() goes on to the next word.
  constexpr bool operator()(std::uint32_t) const noexcept { return false; }
};
/// A function for apply() whose result does not convert to bool.
struct apply_void {
  constexpr void operator()(std::uint32_t) const noexcept {}
};
template <typename T, typename Function>
concept can_apply = requires(T const& t, Function f) { midi2::ump::apply(t, f); };
template <typename T, typename Function, std::size_t Index>
concept can_apply_from = requires(T const& t, Function f) { midi2::ump::apply<T, Function, Index>(t, f); };
template <typename T>
concept can_check = requires(T const& t) { midi2::ump::check(t); };

static_assert(can_apply<midi2::ump::m2cvm::note_on, apply_bool>);
static_assert(!can_apply<int, apply_bool>, "Not a UMP message");
static_assert(!can_apply<std::array<std::uint32_t, 2>, apply_bool>, "Not a UMP message");
static_assert(!can_apply<midi2::ump::m2cvm::note_on, apply_void>, "The result doesn't convert to bool");
static_assert(!can_apply<midi2::ump::m2cvm::note_on, int>, "Not a function");
static_assert(can_apply_from<midi2::ump::m2cvm::note_on, apply_bool, 1>);
static_assert(!can_apply_from<midi2::ump::m2cvm::note_on, apply_bool, 2>, "Index is past the last word");
static_assert(can_check<midi2::ump::m2cvm::note_on>);
static_assert(can_check<midi2::ump::utility::noop>);
static_assert(!can_check<int>, "Not a UMP message");
static_assert(!can_check<std::tuple<std::uint32_t>>, "Not a UMP message");

// The data() member functions check the elements of a range after the projection has been applied.
template <typename Message, typename Range, typename... Proj>
concept can_set_data = requires(Message m, Range const& r, Proj... p) { m.data(r, p...); };
/// A projection which converts std::byte to an integer.
struct byte_to_uint8 {
  /// \param b  The byte to be converted.
  /// \returns The value of \p b.
  constexpr std::uint8_t operator()(std::byte const b) const noexcept { return std::to_integer<std::uint8_t>(b); }
};
/// A projection whose result is neither integral nor convertible to char8_t.
struct to_void {
  constexpr void operator()(std::byte) const noexcept {}
};
using byte_array = std::array<std::byte, 2>;
static_assert(can_set_data<midi2::ump::data64::sysex7_in_1, byte_array, byte_to_uint8>);
static_assert(!can_set_data<midi2::ump::data64::sysex7_in_1, byte_array>, "std::byte is not integral");
static_assert(!can_set_data<midi2::ump::data64::sysex7_in_1, byte_array, to_void>, "The projection gives void");
static_assert(can_set_data<midi2::ump::data128::sysex8_in_1, byte_array, byte_to_uint8>);
static_assert(!can_set_data<midi2::ump::data128::sysex8_in_1, byte_array>, "std::byte is not integral");
static_assert(can_set_data<midi2::ump::flex_data::text_common, byte_array, byte_to_uint8>);
static_assert(!can_set_data<midi2::ump::flex_data::text_common, byte_array>, "std::byte doesn't convert to char8_t");
static_assert(!can_set_data<midi2::ump::flex_data::text_common, byte_array, to_void>, "The projection gives void");

// sysex7_message
static_assert(midi2::ump::data64::sysex7_message<midi2::ump::data64::sysex7_in_1>);
static_assert(midi2::ump::data64::sysex7_message<midi2::ump::data64::sysex7_start>);
static_assert(midi2::ump::data64::sysex7_message<midi2::ump::data64::sysex7_continue>);
static_assert(midi2::ump::data64::sysex7_message<midi2::ump::data64::sysex7_end>);
static_assert(!midi2::ump::data64::sysex7_message<midi2::ump::m2cvm::note_on>);
static_assert(!midi2::ump::data64::sysex7_message<int>);

}  // end anonymous namespace
