//===-- CI Dispatcher ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// DUT
#include "midi2/ci/ci_dispatcher.hpp"

// MIDI2
#include "midi2/ci/ci_create_message.hpp"
#include "midi2/ci/ci_types.hpp"
#include "midi2/utils.hpp"

// Standard library
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// google mock/test/fuzz
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
#include <fuzztest/fuzztest.h>
#endif

namespace {
std::ostream& write_bytes(std::ostream& os, std::span<midi2::ci::b7 const> const& arr) {
  os << '[';
  auto const* separator = "";
  for (auto const v : arr) {
    os << separator << unsigned{v.get()};
    separator = ",";
  }
  os << ']';
  return os;
}
}  // end anonymous namespace

namespace midi2::ci {

std::ostream& operator<<(std::ostream& os, header const& ci);
std::ostream& operator<<(std::ostream& os, header const& ci) {
  return os << "{ device_id=" << static_cast<unsigned>(ci.device_id.get())
            << ", version=" << static_cast<unsigned>(ci.version.get()) << ", remote_muid=" << ci.remote_muid.get()
            << ", local_muid=" << ci.local_muid.get() << " }";
}

std::ostream& operator<<(std::ostream& os, property_exchange::chunk_info const& ci);
std::ostream& operator<<(std::ostream& os, property_exchange::chunk_info const& ci) {
  return os << "{ number_of_chunks=" << ci.number_of_chunks.get() << ", chunk_number=" << ci.chunk_number.get() << " }";
}

std::ostream& operator<<(std::ostream& os, ci::discovery const& d);
std::ostream& operator<<(std::ostream& os, ci::discovery const& d) {
  os << "{ manufacturer=";
  write_bytes(os, std::span{d.manufacturer});
  os << ", family=" << d.family.get() << ", model=" << d.model.get() << ", version=";
  write_bytes(os, d.version);
  os << ", capability=" << static_cast<unsigned>(d.capability.get())
     << ", max_sysex_size=" << std::uint32_t{d.max_sysex_size} << " }";
  return os;
}

std::ostream& operator<<(std::ostream& os, ci::discovery_reply const& d);
std::ostream& operator<<(std::ostream& os, ci::discovery_reply const& d) {
  os << "{ manufacturer=";
  write_bytes(os, d.manufacturer);
  os << ", family=" << d.family.get() << ", model=" << d.model.get() << ", version=";
  write_bytes(os, d.version);
  os << ", capability=" << static_cast<unsigned>(d.capability.get()) << ", max_sysex_size=" << d.max_sysex_size.get()
     << " }";
  return os;
}

std::ostream& operator<<(std::ostream& os, ci::nak const& nak);
std::ostream& operator<<(std::ostream& os, ci::nak const& nak) {
  os << "{ original_id=" << static_cast<unsigned>(nak.original_id.get())
     << ", status_code=" << static_cast<unsigned>(nak.status_code.get())
     << ", status_data=" << static_cast<unsigned>(nak.status_data.get()) << ", details=";
  write_bytes(os, nak.details);
  os << ", message=";
  write_bytes(os, nak.message);
  os << " }";
  return os;
}

std::ostream& operator<<(std::ostream& os, ci::profile_configuration::disabled const& pds);
std::ostream& operator<<(std::ostream& os, ci::profile_configuration::disabled const& pd) {
  os << "{ pid=";
  write_bytes(os, pd.pid);
  os << ", num_channels=" << pd.num_channels.get() << " }";
  return os;
}

}  // end namespace midi2::ci

namespace {

using testing::AllOf;
using testing::ElementsAreArray;
using testing::Eq;
using testing::Field;
using testing::IsEmpty;
using testing::Return;
using testing::StrictMock;

using midi2::ci::byte_array;
using midi2::ci::header;
using midi2::ci::details::from_byte_array;
using midi2::ci::details::from_le7;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace midi2::literals;

consteval std::uint8_t operator""_u8(unsigned long long arg) noexcept {
  assert(arg < 256);
  return static_cast<std::uint8_t>(arg);
}

struct context_type {
  constexpr bool operator==(context_type const&) const noexcept = default;
};

class mock_system_callbacks : public midi2::ci::dispatcher_backend::system_pure<context_type> {
public:
  MOCK_METHOD(bool, check_muid, (context_type&, std::uint8_t group, midi2::ci::muid muid), (override));
  MOCK_METHOD(void, buffer_overflow, (context_type&), (override));
  MOCK_METHOD(void, unknown_midici, (context_type&, header const&), (override));
};

class mock_management_callbacks : public midi2::ci::dispatcher_backend::management_pure<context_type> {
public:
  MOCK_METHOD(void, discovery, (context_type&, header const&, midi2::ci::discovery const&), (override));
  MOCK_METHOD(void, discovery_reply, (context_type&, header const&, midi2::ci::discovery_reply const&), (override));
  MOCK_METHOD(void, endpoint, (context_type&, header const&, midi2::ci::endpoint const&), (override));
  MOCK_METHOD(void, endpoint_reply, (context_type&, header const&, midi2::ci::endpoint_reply const&), (override));
  MOCK_METHOD(void, invalidate_muid, (context_type&, header const&, midi2::ci::invalidate_muid const&), (override));
  MOCK_METHOD(void, ack, (context_type&, header const&, midi2::ci::ack const&), (override));
  MOCK_METHOD(void, nak, (context_type&, header const&, midi2::ci::nak const&), (override));
};

class mock_profile_callbacks : public midi2::ci::dispatcher_backend::profile_pure<context_type> {
public:
  MOCK_METHOD(void, inquiry, (context_type&, header const&), (override));
  MOCK_METHOD(void, inquiry_reply,
              (context_type&, header const&, midi2::ci::profile_configuration::inquiry_reply const&), (override));
  MOCK_METHOD(void, added, (context_type&, header const&, midi2::ci::profile_configuration::added const&), (override));
  MOCK_METHOD(void, removed, (context_type&, header const&, midi2::ci::profile_configuration::removed const&),
              (override));
  MOCK_METHOD(void, details, (context_type&, header const&, midi2::ci::profile_configuration::details const&),
              (override));
  MOCK_METHOD(void, details_reply,
              (context_type&, header const&, midi2::ci::profile_configuration::details_reply const&), (override));
  MOCK_METHOD(void, on, (context_type&, header const&, midi2::ci::profile_configuration::on const&), (override));
  MOCK_METHOD(void, off, (context_type&, header const&, midi2::ci::profile_configuration::off const&), (override));
  MOCK_METHOD(void, enabled, (context_type&, header const&, midi2::ci::profile_configuration::enabled const&),
              (override));
  MOCK_METHOD(void, disabled, (context_type&, header const&, midi2::ci::profile_configuration::disabled const&),
              (override));
  MOCK_METHOD(void, specific_data,
              (context_type&, header const&, midi2::ci::profile_configuration::specific_data const&), (override));
};

class mock_property_exchange_callbacks : public midi2::ci::dispatcher_backend::property_exchange_pure<context_type> {
public:
  MOCK_METHOD(void, capabilities, (context_type&, header const&, midi2::ci::property_exchange::capabilities const&),
              (override));
  MOCK_METHOD(void, capabilities_reply,
              (context_type&, header const&, midi2::ci::property_exchange::capabilities_reply const&), (override));

  MOCK_METHOD(void, get, (context_type&, header const&, midi2::ci::property_exchange::get const&), (override));
  MOCK_METHOD(void, get_reply, (context_type&, header const&, midi2::ci::property_exchange::get_reply const&),
              (override));
  MOCK_METHOD(void, set, (context_type&, header const&, midi2::ci::property_exchange::set const&), (override));
  MOCK_METHOD(void, set_reply, (context_type&, header const&, midi2::ci::property_exchange::set_reply const&),
              (override));
  MOCK_METHOD(void, subscription, (context_type&, header const&, midi2::ci::property_exchange::subscription const&),
              (override));
  MOCK_METHOD(void, subscription_reply,
              (context_type&, header const&, midi2::ci::property_exchange::subscription_reply const&), (override));
  MOCK_METHOD(void, notify, (context_type&, header const&, midi2::ci::property_exchange::notify const&), (override));
};

class mock_process_inquiry_callbacks : public midi2::ci::dispatcher_backend::process_inquiry_pure<context_type> {
public:
  MOCK_METHOD(void, capabilities, (context_type&, header const&), (override));
  MOCK_METHOD(void, capabilities_reply,
              (context_type&, header const&, midi2::ci::process_inquiry::capabilities_reply const&), (override));
  MOCK_METHOD(void, midi_message_report,
              (context_type&, header const&, midi2::ci::process_inquiry::midi_message_report const&), (override));
  MOCK_METHOD(void, midi_message_report_reply,
              (context_type&, header const&, midi2::ci::process_inquiry::midi_message_report_reply const&), (override));
  MOCK_METHOD(void, midi_message_report_end, (context_type&, header const&), (override));
};

constexpr auto broadcast_muid = midi2::ci::broadcast_muid;

class CIDispatcher : public testing::Test {
public:
  CIDispatcher() : processor_{config_} {}

protected:
  struct mocked_config {
    [[no_unique_address]] context_type context;
    static constexpr auto buffer_size = std::size_t{128};
    StrictMock<mock_system_callbacks> system;
    StrictMock<mock_management_callbacks> management;
    StrictMock<mock_profile_callbacks> profile;
    StrictMock<mock_property_exchange_callbacks> property_exchange;
    StrictMock<mock_process_inquiry_callbacks> process_inquiry;
  };
  mocked_config config_;
  midi2::ci::ci_dispatcher<std::reference_wrapper<mocked_config>> processor_;

  static constexpr auto sender_muid_ = midi2::ci::muid{from_le7(std::array{0x7F_b, 0x7E_b, 0x7D_b, 0x7C_b})};
  static constexpr auto destination_muid_ = midi2::ci::muid{from_le7(std::array{0x62_b, 0x16_b, 0x63_b, 0x26_b})};

  template <typename Content> static std::vector<std::byte> make_message(header const& hdr, Content const& content) {
    std::vector<std::byte> message;
    midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, hdr, content);
    message.push_back(0_b);  // a stray extra byte
    return message;
  }

  template <typename Content> void dispatch_ci(std::uint8_t group, header const& hdr, Content const& content) {
    processor_.set_group(group);
    processor_.set_device_id(hdr.device_id);
    auto const message = make_message(hdr, content);
    std::ranges::for_each(message, [this](std::byte const b) { processor_.dispatch(b); });
  }
};
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Empty) {
  processor_.dispatch(0_b);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryV1) {
  constexpr header hdr{.device_id = 0x7F_b7,
                       .version = 1_b7,
                       .remote_muid = midi2::ci::muid{0U},
                       .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = std::array{0x12_b7, 0x23_b7, 0x34_b7},
                                           .family = midi2::ci::b14{(1U << (7 * 2)) - 1U},
                                           .model = midi2::ci::b14{(1U << (7 * 2)) - 2U},
                                           .version = std::array{0x7F_b7, 0x3C_b7, 0x2A_b7, 0x18_b7},
                                           .capability = 0x7F_b7,
                                           .max_sysex_size = midi2::ci::b28{(1U << (7 * 4)) - 1U},
                                           .output_path_id = 0_b7};
  EXPECT_CALL(config_.management, discovery(config_.context, hdr, discovery)).Times(1);
  this->dispatch_ci(0xFF_u8, hdr, discovery);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryV2) {
  constexpr header hdr{.device_id = 0x7F_b7,
                       .version = 2_b7,
                       .remote_muid = midi2::ci::muid{0U},
                       .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery discovery{.manufacturer = std::array{0x12_b7, 0x23_b7, 0x34_b7},
                                           .family = 0x1779_b14,
                                           .model = 0x1B5D_b14,
                                           .version = from_byte_array(std::array{0x4E_b, 0x3C_b, 0x2A_b, 0x18_b}),
                                           .capability = 0x7F_b7,
                                           .max_sysex_size = 0x07654321_b28,
                                           .output_path_id = 0x71_b7};
  EXPECT_CALL(config_.management, discovery(config_.context, hdr, discovery)).Times(1);
  this->dispatch_ci(0xFF_u8, hdr, discovery);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, DiscoveryReplyV2) {
  constexpr auto device_id = 0x7F_b7;
  constexpr auto manufacturer = std::array{0x12_b, 0x23_b, 0x34_b};
  constexpr auto version = std::array{0x7E_b, 0x6C_b, 0x5A_b, 0x48_b};

  constexpr header hdr{.device_id = device_id,
                       .version = 2_b7,
                       .remote_muid = midi2::ci::muid{0U},
                       .local_muid = midi2::ci::broadcast_muid};
  constexpr midi2::ci::discovery_reply reply{.manufacturer = from_byte_array(manufacturer),
                                             .family = 0x33F9_b14,
                                             .model = 0x014D_b14,
                                             .version = from_byte_array(version),
                                             .capability = 0x7F_b7,
                                             .max_sysex_size = 0xFF_b28,
                                             .output_path_id = 0x71_b7,
                                             .function_block = 0x32_b7};
  EXPECT_CALL(config_.management, discovery_reply(config_.context, hdr, reply)).Times(1);
  this->dispatch_ci(0xFF_u8, hdr, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Endpoint) {
  constexpr auto group = 0x01_u8;
  constexpr midi2::ci::muid receiver_muid{0x012345EFU};

  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::endpoint endpoint{.status = 0b0101010_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint(config_.context, hdr, endpoint)).Times(1);

  this->dispatch_ci(group, hdr, endpoint);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, EndpointReply) {
  constexpr auto group = 0x71_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr auto information = std::array{'1'_b7, '2'_b7, '3'_b7, '4'_b7};

  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  midi2::ci::endpoint_reply reply{.status = 0_b7, .information = information};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint_reply(config_.context, hdr, reply)).Times(1);

  this->dispatch_ci(group, hdr, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, InvalidateMuid) {
  constexpr auto group = 0x71_u8;
  constexpr auto device_id = 0x7F_b7;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr auto target_muid = midi2::ci::muid{0x0FE43210U};

  constexpr header hdr{
      .device_id = device_id, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::invalidate_muid invalidate_muid{.target_muid = target_muid};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, invalidate_muid(config_.context, hdr, invalidate_muid)).Times(1);

  this->dispatch_ci(group, hdr, invalidate_muid);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ResetAllowsASecondMessage) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::endpoint first{.status = 0b0101010_b7};
  constexpr midi2::ci::endpoint second{.status = 0b0010101_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  {
    testing::InSequence const seq;
    EXPECT_CALL(config_.management, endpoint(config_.context, hdr, first)).Times(1);
    EXPECT_CALL(config_.management, endpoint(config_.context, hdr, second)).Times(1);
  }
  this->dispatch_ci(group, hdr, first);
  processor_.reset();
  // The group and device ID set by the first dispatch_ci() call must survive the reset: send the raw bytes.
  std::ranges::for_each(make_message(hdr, second), [this](std::byte const b) { processor_.dispatch(b); });
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, WithoutResetASecondMessageIsDiscarded) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::endpoint first{.status = 0b0101010_b7};
  constexpr midi2::ci::endpoint second{.status = 0b0010101_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint(config_.context, hdr, first)).Times(1);
  this->dispatch_ci(group, hdr, first);
  this->dispatch_ci(group, hdr, second);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ResetAbandonsPartialMessage) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr auto information = std::array{'1'_b7, '2'_b7, '3'_b7};
  midi2::ci::endpoint_reply const first{.status = 0_b7, .information = information};
  constexpr midi2::ci::endpoint second{.status = 0b0010101_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, endpoint(config_.context, hdr, second)).Times(1);

  processor_.set_group(group);
  processor_.set_device_id(hdr.device_id);
  auto const partial = make_message(hdr, first);
  // Stop two bytes before the end of the information field.
  std::ranges::for_each(partial | std::views::take(partial.size() - 3U),
                        [this](std::byte const b) { processor_.dispatch(b); });
  processor_.reset();
  std::ranges::for_each(make_message(hdr, second), [this](std::byte const b) { processor_.dispatch(b); });
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ResetAfterOverflow) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  std::vector<midi2::ci::b7> const text{decltype(config_)::buffer_size, 'a'_b7};
  midi2::ci::ack const ack{.original_id = 0x34_b7,
                           .status_code = 0x17_b7,
                           .status_data = 0x7F_b7,
                           .details = std::array{0x01_b7, 0x02_b7, 0x03_b7, 0x04_b7, 0x05_b7},
                           .message = text};
  constexpr midi2::ci::endpoint endpoint{.status = 0b0101010_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.system, buffer_overflow(config_.context)).Times(1);
  EXPECT_CALL(config_.management, endpoint(config_.context, hdr, endpoint)).Times(1);

  this->dispatch_ci(group, hdr, ack);
  processor_.reset();
  this->dispatch_ci(group, hdr, endpoint);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, Ack) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr auto original_id = 0x34_b7;
  constexpr auto ack_status_code = 0x00_b7;
  constexpr auto ack_status_data = 0x7F_b7;
  constexpr auto ack_details = std::array{0x01_b7, 0x02_b7, 0x03_b7, 0x04_b7, 0x05_b7};
  static constexpr auto text = std::array{'H'_b7, 'e'_b7, 'l'_b7, 'l'_b7, 'o'_b7};

  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::ack ack{.original_id = original_id,
                               .status_code = ack_status_code,
                               .status_data = ack_status_data,
                               .details = ack_details,
                               .message = text};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, ack(config_.context, hdr,
                                      AllOf(Field("original_id", &midi2::ci::ack::original_id, Eq(original_id)),
                                            Field("status_code", &midi2::ci::ack::status_code, Eq(ack_status_code)),
                                            Field("status_data", &midi2::ci::ack::status_data, Eq(ack_status_data)),
                                            Field("details", &midi2::ci::ack::details, ElementsAreArray(ack_details)),
                                            Field("message", &midi2::ci::ack::message, ElementsAreArray(text)))))
      .Times(1);

  this->dispatch_ci(group, hdr, ack);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, AckMessageTooLong) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr auto text_length = decltype(config_)::buffer_size;
  std::vector<midi2::ci::b7> text{text_length, 'a'_b7};

  constexpr header hdr{.device_id = 0x7F_b7, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  midi2::ci::ack const ack{.original_id = 0x34_b7,
                           .status_code = 0x17_b7,
                           .status_data = 0x7F_b7,
                           .details = std::array{0x01_b7, 0x02_b7, 0x03_b7, 0x04_b7, 0x05_b7},
                           .message = text};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.system, buffer_overflow(config_.context)).Times(1);

  this->dispatch_ci(group, hdr, ack);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, NakV1) {
  constexpr auto group = 0x01_u8;
  constexpr auto device_id = 0x7F_b7;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr header hdr{
      .device_id = device_id, .version = 1_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::nak nak;

  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, nak(config_.context, hdr, nak)).Times(1);

  this->dispatch_ci(group, hdr, nak);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, NakV2) {
  constexpr auto group = 0x01_u8;
  constexpr auto device_id = 0x7F_b7;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr auto original_id = 0x34_b7;
  constexpr auto nak_status_code = 0x17_b7;
  constexpr auto nak_status_data = 0x7F_b7;
  constexpr auto nak_details = std::array{0x01_b7, 0x02_b7, 0x03_b7, 0x04_b7, 0x05_b7};
  // (The code points a span<> at this object so it must be static in order for the
  // objects to be constexpr.)
  static constexpr auto text = std::array{'H'_b7, 'e'_b7, 'l'_b7, 'l'_b7, 'o'_b7};

  constexpr header hdr{
      .device_id = device_id, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  constexpr midi2::ci::nak nak{.original_id = original_id,
                               .status_code = nak_status_code,
                               .status_data = nak_status_data,
                               .details = nak_details,
                               .message = text};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.management, nak(config_.context, hdr, nak)).Times(1);

  this->dispatch_ci(group, hdr, nak);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileInquiry) {
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};
  constexpr auto hdr =
      header{.device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, inquiry(config_.context, hdr)).Times(1);

  this->dispatch_ci(group, hdr, midi2::ci::profile_configuration::inquiry{});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileInquiryReply) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr auto receiver_muid = midi2::ci::muid{0x012345EFU};

  constexpr auto enabled = std::array{
      profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7},
      profile{0x76_b7, 0x65_b7, 0x54_b7, 0x43_b7, 0x32_b7},
  };
  constexpr auto disabled = std::array<profile, 1>{
      profile{0x71_b7, 0x61_b7, 0x51_b7, 0x41_b7, 0x31_b7},
  };

  constexpr header hdr{.device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = receiver_muid};
  EXPECT_CALL(config_.system, check_muid(config_.context, group, receiver_muid)).WillRepeatedly(Return(true));
  using midi2::ci::profile_configuration::inquiry_reply;
  EXPECT_CALL(config_.profile,
              inquiry_reply(config_.context, hdr,
                            AllOf(Field("enabled", &inquiry_reply::enabled, ElementsAreArray(enabled)),
                                  Field("disabled", &inquiry_reply::disabled, ElementsAreArray(disabled)))))
      .Times(1);

  this->dispatch_ci(group, hdr, inquiry_reply{enabled, disabled});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileAdded) {
  using midi2::ci::profile_configuration::added;
  using midi2::ci::profile_configuration::profile;
  constexpr header hdr{
      .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = broadcast_muid};
  constexpr added added_profile{.pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7}};
  EXPECT_CALL(config_.profile, added(config_.context, hdr, added_profile)).Times(1);

  this->dispatch_ci(0x01_u8, hdr, added_profile);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileRemoved) {
  using midi2::ci::profile_configuration::profile;
  using midi2::ci::profile_configuration::removed;
  constexpr header hdr{
      .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = broadcast_muid};
  constexpr removed removed_profile{.pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7}};
  EXPECT_CALL(config_.profile, removed(config_.context, hdr, removed_profile)).Times(1);

  this->dispatch_ci(0x01_u8, hdr, removed_profile);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDetails) {
  using midi2::ci::profile_configuration::details;
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = destination_muid_};
  constexpr details config_details{.pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7}, .target = 0x23_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, details(config_.context, hdr, config_details)).Times(1);

  this->dispatch_ci(group, hdr, config_details);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDetailsReply) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr auto pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7};
  constexpr auto target = 0x23_b7;
  constexpr auto data = std::array{'H'_b7, 'e'_b7, 'l'_b7, 'l'_b7, 'o'_b7};

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  using midi2::ci::profile_configuration::details_reply;

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, details_reply(config_.context, hdr,
                                             AllOf(Field("pid", &details_reply::pid, Eq(pid)),
                                                   Field("target", &details_reply::target, Eq(target)),
                                                   Field("data", &details_reply::data, ElementsAreArray(data)))))
      .Times(1);

  this->dispatch_ci(group, hdr, details_reply{pid, target, data});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileOn) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::profile_configuration::on on{
      .pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7},
      .num_channels = 23_b14,
  };

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, on(config_.context, hdr, on)).Times(1);

  this->dispatch_ci(group, hdr, on);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileOff) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr auto off =
      midi2::ci::profile_configuration::off{.pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7}};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.profile, off(config_.context, hdr, off)).Times(1);

  this->dispatch_ci(group, hdr, off);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileEnabled) {
  using midi2::ci::profile_configuration::profile;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = broadcast_muid,
  };
  constexpr midi2::ci::profile_configuration::enabled enabled{
      .pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7},
      .num_channels = 0x1122_b14,
  };

  EXPECT_CALL(config_.profile, enabled(config_.context, hdr, enabled)).Times(1);

  this->dispatch_ci(0x01_u8, hdr, enabled);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileDisabled) {
  using midi2::ci::profile_configuration::profile;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = broadcast_muid,
  };
  constexpr midi2::ci::profile_configuration::disabled disabled{
      .pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7},
      .num_channels = 0x123_b14,
  };
  EXPECT_CALL(config_.profile, disabled(config_.context, hdr, disabled)).Times(1);

  this->dispatch_ci(0x01_u8, hdr, disabled);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProfileSpecificData) {
  using midi2::ci::profile_configuration::profile;
  constexpr auto group = 0x01_u8;
  constexpr auto pid = profile{0x12_b7, 0x23_b7, 0x34_b7, 0x45_b7, 0x56_b7};
  constexpr auto data = std::array{'H'_b7, 'e'_b7, 'l'_b7, 'l'_b7, 'o'_b7};

  // clang-format on
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = broadcast_muid,
  };
  using midi2::ci::profile_configuration::specific_data;
  EXPECT_CALL(config_.profile, specific_data(config_.context, hdr,
                                             AllOf(Field("pid", &specific_data::pid, Eq(pid)),
                                                   Field("data", &specific_data::data, ElementsAreArray(data)))))
      .Times(1);

  this->dispatch_ci(group, hdr, specific_data{pid, data});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeCapabilities) {
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::property_exchange::capabilities caps{
      .num_simultaneous = 2_b7,
      .major_version = 3_b7,
      .minor_version = 4_b7,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, capabilities(config_.context, hdr, caps)).Times(1);

  this->dispatch_ci(group, hdr, caps);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeCapabilitiesReply) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::property_exchange::capabilities_reply caps{
      .num_simultaneous = 2_b7,
      .major_version = 3_b7,
      .minor_version = 4_b7,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, capabilities_reply(config_.context, hdr, caps)).Times(1);

  this->dispatch_ci(group, hdr, caps);
}

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace std::string_view_literals;

namespace {

/// \returns The bytes of \p s, as used for Property Exchange property data.
std::span<std::byte const> bytes(std::string_view const s) noexcept {
  return std::as_bytes(std::span{s});
}

}  // end anonymous namespace
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeGetPropertyData) {
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::property_exchange::get g{
      .chunk = midi2::ci::property_exchange::chunk_info{2_b14, 1_b14},
      .request = 1_b7,
      .header = R"({"status":200})"sv,
      .data = {},
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, get(config_.context, hdr, g));

  this->dispatch_ci(group, hdr, g);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeGetPropertyDataReply) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  using get_reply = midi2::ci::property_exchange::get_reply;
  get_reply const gr{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 1_b7,
      .header = R"({"status":200})"sv,
      .data = bytes(R"([{"resource":"DeviceInfo"},{"resource":"ChannelList"},{"resource":"CMList"}])"sv),
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              get_reply(config_.context, hdr,
                        AllOf(Field("chunk", &get_reply::chunk, Eq(gr.chunk)),
                              Field("request", &get_reply::request, Eq(gr.request)),
                              Field("header", &get_reply::header, ElementsAreArray(gr.header)),
                              Field("data", &get_reply::data, ElementsAreArray(gr.data)))));

  this->dispatch_ci(group, hdr, gr);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSetPropertyData) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::set;
  set const spd{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 1_b7,
      .header = R"({"resource":"X-ProgramEdit","resId":"abcd"})"sv,
      .data = bytes(R"({"name":"Violin 2","lfoSpeed":10,"lfoWaveform":"sine"})"sv),
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              set(config_.context, hdr,
                  AllOf(Field("chunk", &set::chunk, Eq(spd.chunk)), Field("request", &set::request, Eq(spd.request)),
                        Field("header", &set::header, ElementsAreArray(spd.header)),
                        Field("data", &set::data, ElementsAreArray(spd.data)))));

  this->dispatch_ci(group, hdr, spd);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSetPropertyDataReply) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::set_reply;
  constexpr set_reply spd_reply{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 2_b7,
      .header = R"({"status":200})"sv,
      .data = {},
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              set_reply(config_.context, hdr,
                        AllOf(Field("chunk", &set_reply::chunk, Eq(spd_reply.chunk)),
                              Field("request", &set_reply::request, Eq(spd_reply.request)),
                              Field("header", &set_reply::header, ElementsAreArray(spd_reply.header)))));

  this->dispatch_ci(group, hdr, spd_reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSubscription) {
  constexpr auto group = 0x01_u8;
  constexpr auto destination = 0x0F_b7;

  constexpr auto header = R"({"command":"full","subscribeId":"sub32847623"})"sv;
  constexpr auto data = "multichannel"sv;

  constexpr struct header hdr {
    .device_id = destination, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::subscription;
  subscription const sub{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 17_b7,
      .header = header,
      .data = bytes(data),
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              subscription(config_.context, hdr,
                           AllOf(Field("chunk", &subscription::chunk, Eq(sub.chunk)),
                                 Field("request", &subscription::request, Eq(sub.request)),
                                 Field("header", &subscription::header, ElementsAreArray(header)),
                                 Field("data", &subscription::data, ElementsAreArray(bytes(data))))));

  this->dispatch_ci(group, hdr, sub);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeSubscriptionReply) {
  constexpr auto group = 0x01_u8;

  constexpr auto header = R"({"status":200})"sv;
  constexpr auto data = "data"sv;

  constexpr struct header hdr {
    .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::subscription_reply;
  subscription_reply const sub_reply{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 17_b7,
      .header = header,
      .data = bytes(data),
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange,
              subscription_reply(config_.context, hdr,
                                 AllOf(Field("chunk", &subscription_reply::chunk, Eq(sub_reply.chunk)),
                                       Field("request", &subscription_reply::request, Eq(sub_reply.request)),
                                       Field("header", &subscription_reply::header, ElementsAreArray(header)),
                                       Field("data", &subscription_reply::data, ElementsAreArray(bytes(data))))));

  this->dispatch_ci(group, hdr, sub_reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, PropertyExchangeNotify) {
  constexpr auto group = 0x01_u8;
  constexpr auto header = R"({"status":144})"sv;
  constexpr auto data = "data"sv;

  constexpr struct header hdr {
    .device_id = 0x0F_b7, .version = 2_b7, .remote_muid = sender_muid_, .local_muid = destination_muid_,
  };
  using midi2::ci::property_exchange::notify;
  notify const note{
      .chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
      .request = 1_b7,
      .header = header,
      .data = bytes(data),
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.property_exchange, notify(config_.context, hdr,
                                                AllOf(Field("chunk", &notify::chunk, Eq(note.chunk)),
                                                      Field("request", &notify::request, Eq(note.request)),
                                                      Field("header", &notify::header, ElementsAreArray(header)),
                                                      Field("data", &notify::data, ElementsAreArray(bytes(data))))));

  this->dispatch_ci(group, hdr, note);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryCapabilities) {
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x7F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, capabilities(config_.context, hdr)).Times(1);

  this->dispatch_ci(group, hdr, midi2::ci::process_inquiry::capabilities{});
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryCapabilitiesReply) {
  constexpr auto group = 0x01_u8;
  constexpr auto destination = 0x7F_b7;

  constexpr header hdr{
      .device_id = destination,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::process_inquiry::capabilities_reply reply{.features = 0b0101010_b7};

  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, capabilities_reply(config_.context, hdr, reply)).Times(1);

  this->dispatch_ci(group, hdr, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReport) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x01_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::process_inquiry::midi_message_report report{
      .message_data_control = decltype(report)::control::full,
      // system messages
      .mtc_quarter_frame = 1,
      .song_position = 0,
      .song_select = 1,
      // channel controller messages
      .pitchbend = 1,
      .control_change = 0,
      .rpn_registered_controller = 1,
      .nrpn_assignable_controller = 0,
      .program_change = 1,
      .channel_pressure = 0,
      // note data messages
      .notes = 1,
      .poly_pressure = 0,
      .per_note_pitchbend = 1,
      .registered_per_note_controller = 0,
      .assignable_per_note_controller = 1,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report(config_.context, hdr, report)).Times(1);

  this->dispatch_ci(group, hdr, report);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReportReply) {
  constexpr auto group = 0x01_u8;

  constexpr header hdr{
      .device_id = 0x01_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  constexpr midi2::ci::process_inquiry::midi_message_report_reply reply{
      // system messages
      .mtc_quarter_frame = 1,
      .song_position = 0,
      .song_select = 1,
      // channel controller messages
      .pitchbend = 1,
      .control_change = 0,
      .rpn_registered_controller = 1,
      .nrpn_assignable_controller = 0,
      .program_change = 1,
      .channel_pressure = 0,
      // note data messages
      .notes = 1,
      .poly_pressure = 0,
      .per_note_pitchbend = 1,
      .registered_per_note_controller = 0,
      .assignable_per_note_controller = 1,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report_reply(config_.context, hdr, reply)).Times(1);

  this->dispatch_ci(group, hdr, reply);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcher, ProcessInquiryMidiMessageReportEnd) {
  constexpr auto group = 0x01_u8;
  constexpr header hdr{
      .device_id = 0x01_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };
  EXPECT_CALL(config_.system, check_muid(config_.context, group, destination_muid_)).WillRepeatedly(Return(true));
  EXPECT_CALL(config_.process_inquiry, midi_message_report_end(config_.context, hdr)).Times(1);

  this->dispatch_ci(group, hdr, midi2::ci::process_inquiry::midi_message_report_end{});
}

// A property_exchange_stream backend which records the calls that it receives.
struct stream_recorder {
  std::vector<std::string> events;
  std::vector<header> headers;  // The MIDI-CI header passed to each call.
  midi2::ci::property_exchange::stream_start start_fields;
  midi2::ci::property_exchange::stream_chunk chunk_fields;
  std::string header_text;
  std::string data_text;
  std::size_t largest_piece = 0;

  void start(context_type&, header const& ci, midi2::ci::property_exchange::stream_start const& s) {
    this->record("start", ci);
    start_fields = s;
  }
  void header_data(context_type&, header const& ci, std::span<char const> const piece) {
    this->record("header_data", ci);
    header_text.append(piece.begin(), piece.end());
    largest_piece = std::max(largest_piece, piece.size());
  }
  void chunk(context_type&, header const& ci, midi2::ci::property_exchange::stream_chunk const& c) {
    this->record("chunk", ci);
    chunk_fields = c;
  }
  void property_data(context_type&, header const& ci, std::span<std::byte const> const piece) {
    this->record("property_data", ci);
    std::ranges::transform(piece, std::back_inserter(data_text),
                           [](std::byte const b) { return static_cast<char>(b); });
    largest_piece = std::max(largest_piece, piece.size());
  }
  void end(context_type&, header const& ci) { this->record("end", ci); }
  void abort(context_type&, header const& ci) { this->record("abort", ci); }

  void clear() { *this = stream_recorder{}; }
  [[nodiscard]] std::string last_event() const { return events.empty() ? std::string{} : events.back(); }

private:
  void record(char const* const event, header const& ci) {
    events.emplace_back(event);
    headers.push_back(ci);
  }
};

// The expected sequence of events for a message whose header and data are sent in pieces of at most piece_size bytes.
std::vector<std::string> expected_events(std::size_t const header_length, std::size_t const data_length,
                                         std::size_t const piece_size) {
  std::vector<std::string> result{"start"};
  result.insert(result.end(), (header_length + piece_size - 1) / piece_size, "header_data");
  result.emplace_back("chunk");
  result.insert(result.end(), (data_length + piece_size - 1) / piece_size, "property_data");
  result.emplace_back("end");
  return result;
}

class CIDispatcherStream : public testing::Test {
public:
  CIDispatcherStream() : processor_{config_} {
    EXPECT_CALL(config_.system, check_muid(config_.context, group_, destination_muid_)).WillRepeatedly(Return(true));
  }

protected:
  struct stream_config {
    [[no_unique_address]] context_type context;
    static constexpr auto buffer_size = std::size_t{16};
    StrictMock<mock_system_callbacks> system;
    StrictMock<mock_management_callbacks> management;
    StrictMock<mock_profile_callbacks> profile;
    // A StrictMock so that a call to the buffered property exchange handlers fails the test.
    StrictMock<mock_property_exchange_callbacks> property_exchange;
    StrictMock<mock_process_inquiry_callbacks> process_inquiry;
    stream_recorder property_exchange_stream;
  };
  static_assert(midi2::ci::ci_dispatcher_stream_config<stream_config>);

  stream_config config_;
  midi2::ci::ci_dispatcher<std::reference_wrapper<stream_config>> processor_;

  static constexpr auto group_ = 0x01_u8;
  static constexpr auto sender_muid_ = midi2::ci::muid{from_le7(std::array{0x7F_b, 0x7E_b, 0x7D_b, 0x7C_b})};
  static constexpr auto destination_muid_ = midi2::ci::muid{from_le7(std::array{0x62_b, 0x16_b, 0x63_b, 0x26_b})};
  static constexpr header hdr_{
      .device_id = 0x0F_b7,
      .version = 2_b7,
      .remote_muid = sender_muid_,
      .local_muid = destination_muid_,
  };

  template <typename Content> static std::vector<std::byte> make_message(Content const& content) {
    std::vector<std::byte> message;
    midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, hdr_, content);
    return message;
  }
  void dispatch(std::span<std::byte const> const message) {
    processor_.set_group(group_);
    processor_.set_device_id(hdr_.device_id);
    std::ranges::for_each(message, [this](std::byte const b) { processor_.dispatch(b); });
  }

  // Returns a string of the given length containing a JSON array of numbers.
  static std::string make_json(std::size_t const length) {
    std::string result = "[";
    auto separator = ""sv;
    for (auto n = 0U; result.size() < length - 1; ++n) {
      result += separator;
      result += std::to_string(n);
      separator = ","sv;
    }
    result.resize(length - 1, ' ');
    result += ']';
    return result;
  }
};

static_assert(!midi2::ci::ci_dispatcher_stream_config<midi2::ci::function_config<context_type, 16>>);
static_assert(midi2::ci::ci_dispatcher_stream_config<midi2::ci::function_stream_config<context_type, 16>>);

// The buffer must be large enough to hold a MIDI-CI message header.
static_assert(sizeof(midi2::ci::packed::header) == 13);
static_assert(!midi2::ci::ci_dispatcher_config<midi2::ci::function_config<context_type, 12>>);
static_assert(midi2::ci::ci_dispatcher_config<midi2::ci::function_config<context_type, 13>>);
static_assert(!midi2::ci::ci_dispatcher_stream_config<midi2::ci::function_stream_config<context_type, 12>>);
static_assert(midi2::ci::ci_dispatcher_stream_config<midi2::ci::function_stream_config<context_type, 13>>);

// NOLINTNEXTLINE
TEST_F(CIDispatcherStream, LargeGetReply) {
  using get_reply = midi2::ci::property_exchange::get_reply;
  auto const header_json = std::string{R"({"status":200,"cacheTime":3600,"mediaType":"application/json"})"};
  auto const data_json = make_json(5000);
  auto const gr = get_reply{.chunk = midi2::ci::property_exchange::chunk_info{1_b14, 1_b14},
                            .request = 0x23_b7,
                            .header = header_json,
                            .data = bytes(data_json)};
  auto const message = make_message(gr);
  ASSERT_GT(message.size(), 10 * stream_config::buffer_size);
  this->dispatch(message);

  auto const& rec = config_.property_exchange_stream;
  EXPECT_THAT(rec.events,
              ElementsAreArray(expected_events(header_json.size(), data_json.size(), stream_config::buffer_size)));
  EXPECT_THAT(rec.headers, testing::Each(Eq(hdr_)));
  EXPECT_EQ(rec.start_fields, (midi2::ci::property_exchange::stream_start{
                                  .type = midi2::ci::property_exchange::property_exchange_type::get_reply,
                                  .request = 0x23_b7,
                                  .header_length = midi2::ci::b14{static_cast<std::uint16_t>(header_json.size())}}));
  EXPECT_EQ(rec.chunk_fields,
            (midi2::ci::property_exchange::stream_chunk{
                .chunk = gr.chunk, .data_length = midi2::ci::b14{static_cast<std::uint16_t>(data_json.size())}}));
  EXPECT_EQ(rec.header_text, header_json);
  EXPECT_EQ(rec.data_text, data_json);
  EXPECT_EQ(rec.largest_piece, stream_config::buffer_size);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherStream, MessageTypes) {
  namespace pe = midi2::ci::property_exchange;
  using pet = pe::property_exchange_type;
  auto const chunk = pe::chunk_info{1_b14, 1_b14};
  auto const header_json = std::string{R"({"resource":"DeviceInfo"})"};
  auto const data_json = std::string{R"({"manufacturer":"Acme"})"};

  auto check = [&, this](auto const& msg, pet const expected) {
    SCOPED_TRACE(static_cast<int>(expected));
    processor_.reset();
    config_.property_exchange_stream.clear();
    this->dispatch(make_message(msg));
    auto const& rec = config_.property_exchange_stream;
    EXPECT_EQ(rec.start_fields.type, expected);
    EXPECT_EQ(rec.header_text, header_json);
    EXPECT_EQ(rec.last_event(), "end");
  };
  check(pe::get::make(chunk, 1_b7, header_json), pet::get);
  check(pe::get_reply::make(chunk, 1_b7, header_json, bytes(data_json)), pet::get_reply);
  check(pe::set::make(chunk, 1_b7, header_json, bytes(data_json)), pet::set);
  check(pe::set_reply::make(chunk, 1_b7, header_json, bytes(data_json)), pet::set_reply);
  check(pe::subscription::make(chunk, 1_b7, header_json, bytes(data_json)), pet::subscription);
  check(pe::subscription_reply::make(chunk, 1_b7, header_json, bytes(data_json)), pet::subscription_reply);
  check(pe::notify::make(chunk, 1_b7, header_json, bytes(data_json)), pet::notify);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherStream, EmptyHeaderAndData) {
  using notify = midi2::ci::property_exchange::notify;
  this->dispatch(make_message(notify::make(midi2::ci::property_exchange::chunk_info{1_b14, 1_b14}, 2_b7, {}, {})));

  auto const& rec = config_.property_exchange_stream;
  EXPECT_THAT(rec.events, ElementsAreArray({"start", "chunk", "end"}));
  EXPECT_EQ(rec.start_fields.header_length, 0_b14);
  EXPECT_EQ(rec.chunk_fields.data_length, 0_b14);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherStream, ExactMultipleOfBufferSize) {
  using set = midi2::ci::property_exchange::set;
  auto const header_json = make_json(2 * stream_config::buffer_size);
  auto const data_json = make_json(3 * stream_config::buffer_size);
  this->dispatch(make_message(
      set::make(midi2::ci::property_exchange::chunk_info{1_b14, 1_b14}, 3_b7, header_json, bytes(data_json))));

  auto const& rec = config_.property_exchange_stream;
  EXPECT_THAT(rec.events, ElementsAreArray({"start", "header_data", "header_data", "chunk", "property_data",
                                            "property_data", "property_data", "end"}));
  EXPECT_EQ(rec.header_text, header_json);
  EXPECT_EQ(rec.data_text, data_json);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherStream, MultipleChunks) {
  using get_reply = midi2::ci::property_exchange::get_reply;
  using chunk_info = midi2::ci::property_exchange::chunk_info;
  auto const header_json = std::string{R"({"status":200})"};
  auto const data_json = make_json(100);
  auto const first = std::string_view{data_json}.substr(0, 60);
  auto const second = std::string_view{data_json}.substr(60);

  std::string received;
  this->dispatch(make_message(get_reply::make(chunk_info{2_b14, 1_b14}, 4_b7, header_json, bytes(first))));
  EXPECT_EQ(config_.property_exchange_stream.chunk_fields.chunk, (chunk_info{2_b14, 1_b14}));
  EXPECT_EQ(config_.property_exchange_stream.last_event(), "end");
  received += config_.property_exchange_stream.data_text;

  processor_.reset();
  config_.property_exchange_stream.clear();
  this->dispatch(make_message(get_reply::make(chunk_info{2_b14, 2_b14}, 4_b7, header_json, bytes(second))));
  EXPECT_EQ(config_.property_exchange_stream.chunk_fields.chunk, (chunk_info{2_b14, 2_b14}));
  EXPECT_EQ(config_.property_exchange_stream.last_event(), "end");
  received += config_.property_exchange_stream.data_text;

  EXPECT_EQ(received, data_json);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherStream, ResetPartWayThroughCallsAbort) {
  using set = midi2::ci::property_exchange::set;
  auto const header_json = std::string{R"({"resource":"X-Big"})"};
  auto const data_json = make_json(200);
  auto const message = make_message(
      set::make(midi2::ci::property_exchange::chunk_info{1_b14, 1_b14}, 5_b7, header_json, bytes(data_json)));

  // Send all but the last 50 bytes.
  this->dispatch(std::span{message}.first(message.size() - 50));
  EXPECT_EQ(config_.property_exchange_stream.last_event(), "property_data");
  processor_.reset();
  EXPECT_EQ(config_.property_exchange_stream.last_event(), "abort");
  EXPECT_THAT(config_.property_exchange_stream.headers, testing::Each(Eq(hdr_)));

  // A second reset must not call abort() again.
  auto const events = config_.property_exchange_stream.events.size();
  processor_.reset();
  EXPECT_EQ(config_.property_exchange_stream.events.size(), events);

  // The next message is received in full.
  config_.property_exchange_stream.clear();
  this->dispatch(message);
  EXPECT_EQ(config_.property_exchange_stream.last_event(), "end");
  EXPECT_EQ(config_.property_exchange_stream.data_text, data_json);
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherStream, ResetAfterEndDoesNotCallAbort) {
  using notify = midi2::ci::property_exchange::notify;
  this->dispatch(make_message(notify::make(midi2::ci::property_exchange::chunk_info{1_b14, 1_b14}, 6_b7,
                                           R"({"command":"notify"})", bytes("{}"))));
  ASSERT_EQ(config_.property_exchange_stream.last_event(), "end");
  processor_.reset();
  EXPECT_EQ(config_.property_exchange_stream.last_event(), "end");
}
// NOLINTNEXTLINE
TEST_F(CIDispatcherStream, CapabilitiesAreNotStreamed) {
  using capabilities = midi2::ci::property_exchange::capabilities;
  constexpr capabilities caps{.num_simultaneous = 2_b7, .major_version = 0_b7, .minor_version = 0_b7};
  EXPECT_CALL(config_.property_exchange, capabilities(config_.context, hdr_, caps));
  this->dispatch(make_message(caps));
  EXPECT_THAT(config_.property_exchange_stream.events, IsEmpty());
}
// NOLINTNEXTLINE
TEST(CIDispatcherFunctionStream, LargeMessage) {
  struct empty {};
  constexpr auto buffer_size = std::size_t{32};
  using chunk_info = midi2::ci::property_exchange::chunk_info;
  using get_reply = midi2::ci::property_exchange::get_reply;

  auto const header_json = std::string(100, ' ');
  auto const data_json = std::string(1000, 'x');
  std::vector<std::byte> message;
  midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{},
                            header{.device_id = 0_b7, .version = 2_b7, .remote_muid = 1_b28, .local_muid = 2_b28},
                            get_reply::make(chunk_info{1_b14, 1_b14}, 7_b7, header_json, bytes(data_json)));

  std::string header_text;
  std::string data_text;
  auto ended = false;
  midi2::ci::ci_dispatcher<midi2::ci::function_stream_config<empty, buffer_size>> dispatcher;
  auto& config = dispatcher.config();
  config.system.on_check_muid([](empty, std::uint8_t, midi2::ci::muid) { return true; }).on_buffer_overflow([](empty) {
    FAIL() << "unexpected buffer overflow";
  });
  config.property_exchange_stream
      .on_header_data(
          [&](empty, header const&, std::span<char const> piece) { header_text.append(piece.begin(), piece.end()); })
      .on_property_data([&](empty, header const&, std::span<std::byte const> piece) {
        std::ranges::transform(piece, std::back_inserter(data_text),
                               [](std::byte const b) { return static_cast<char>(b); });
      })
      .on_end([&](empty, header const&) { ended = true; });
  std::ranges::for_each(message, [&dispatcher](std::byte const b) { dispatcher.dispatch(b); });

  EXPECT_EQ(header_text, header_json);
  EXPECT_EQ(data_text, data_json);
  EXPECT_TRUE(ended);
}
// NOLINTNEXTLINE
TEST(CIDispatcherFunctionStream, BufferedConfigOverflows) {
  struct empty {};
  constexpr auto buffer_size = std::size_t{32};
  using chunk_info = midi2::ci::property_exchange::chunk_info;
  using get_reply = midi2::ci::property_exchange::get_reply;

  std::vector<std::byte> message;
  midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{},
                            header{.device_id = 0_b7, .version = 2_b7, .remote_muid = 1_b28, .local_muid = 2_b28},
                            get_reply::make(chunk_info{1_b14, 1_b14}, 7_b7, "{}", bytes(std::string(100, 'x'))));

  auto overflows = 0;
  auto replies = 0;
  auto dispatcher = midi2::ci::make_function_dispatcher<empty, buffer_size>();
  auto& config = dispatcher.config();
  config.system.on_check_muid([](empty, std::uint8_t, midi2::ci::muid) { return true; })
      .on_buffer_overflow([&overflows](empty) { ++overflows; });
  config.property_exchange.on_get_reply([&replies](empty, header const&, get_reply const&) { ++replies; });
  std::ranges::for_each(message, [&dispatcher](std::byte const b) { dispatcher.dispatch(b); });

  EXPECT_EQ(overflows, 1);
  EXPECT_EQ(replies, 0);
}

// A context type with a constructor that takes several arguments. It counts the copies and moves that were needed to
// produce an instance.
class in_place_context {
public:
  in_place_context(std::string name, std::unique_ptr<int> number) : name_{std::move(name)}, number_{*number} {}
  in_place_context(in_place_context const& other)
      : name_{other.name_}, number_{other.number_}, transfers_{other.transfers_ + 1} {}
  in_place_context(in_place_context&& other) noexcept
      : name_{std::move(other.name_)}, number_{other.number_}, transfers_{other.transfers_ + 1} {}
  ~in_place_context() noexcept = default;
  in_place_context& operator=(in_place_context const&) = delete;
  in_place_context& operator=(in_place_context&&) noexcept = delete;

  [[nodiscard]] std::string const& name() const noexcept { return name_; }
  [[nodiscard]] int number() const noexcept { return number_; }
  [[nodiscard]] int transfers() const noexcept { return transfers_; }

private:
  std::string name_;
  int number_ = 0;
  int transfers_ = 0;
};

template <typename Config> class CIDispatcherInPlace : public testing::Test {
protected:
  using dispatcher_type = midi2::ci::ci_dispatcher<Config>;

  // Results recorded by the message handlers.
  std::string name_;
  int number_ = 0;
  std::uint8_t group_ = 0xFF_u8;
  midi2::ci::b7 device_id_ = 0x7F_b7;

  // Sends a message and records the context, group, and device ID with which the handlers are called.
  void dispatch(dispatcher_type& dispatcher) {
    dispatcher.config().system.on_check_muid([this](in_place_context const &c, std::uint8_t group, midi2::ci::muid) {
      name_ = c.name();
      number_ = c.number();
      group_ = group;
      return true;
    });
    dispatcher.config().process_inquiry.on_capabilities(
        [this](in_place_context const&, header const& hdr) { device_id_ = hdr.device_id; });
    std::vector<std::byte> message;
    midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{},
                              header{.device_id = 0_b7, .version = 2_b7, .remote_muid = 1_b28, .local_muid = 2_b28},
                              midi2::ci::process_inquiry::capabilities{});
    message.push_back(0_b);  // a stray extra byte
    std::ranges::for_each(message, [&dispatcher](std::byte const b) { dispatcher.dispatch(b); });
  }
};

using in_place_configs = testing::Types<midi2::ci::function_config<in_place_context, 32>,
                                        midi2::ci::function_stream_config<in_place_context, 32>>;
TYPED_TEST_SUITE(CIDispatcherInPlace, in_place_configs, );

// NOLINTNEXTLINE
TYPED_TEST(CIDispatcherInPlace, ContextArguments) {
  // The unique_ptr<> argument can only be moved, so this also checks that the arguments are perfectly forwarded.
  typename TestFixture::dispatcher_type dispatcher{std::in_place, "name", std::make_unique<int>(42)};
  EXPECT_EQ(dispatcher.config().context.transfers(), 0) << "The context should have been constructed in place";

  this->dispatch(dispatcher);
  EXPECT_EQ(this->name_, "name");
  EXPECT_EQ(this->number_, 42);
  EXPECT_EQ(this->group_, 0_u8);
  EXPECT_EQ(this->device_id_, 0_b7);
}
// NOLINTNEXTLINE
TYPED_TEST(CIDispatcherInPlace, DeviceIdGroupAndContextArguments) {
  typename TestFixture::dispatcher_type dispatcher{0x12_b7, 3_u8, std::in_place, "name", std::make_unique<int>(42)};
  EXPECT_EQ(dispatcher.config().context.transfers(), 0) << "The context should have been constructed in place";

  this->dispatch(dispatcher);
  EXPECT_EQ(this->name_, "name");
  EXPECT_EQ(this->number_, 42);
  EXPECT_EQ(this->group_, 3_u8);
  EXPECT_EQ(this->device_id_, 0x12_b7);
}
// NOLINTNEXTLINE
TYPED_TEST(CIDispatcherInPlace, Constraints) {
  using dispatcher_type = typename TestFixture::dispatcher_type;
  using midi2::ci::b7;
  using number = std::unique_ptr<int>;
  static_assert(std::is_constructible_v<dispatcher_type, std::in_place_t, char const*, number>);
  static_assert(std::is_constructible_v<dispatcher_type, b7, std::uint8_t, std::in_place_t, char const*, number>);
  // The arguments must be accepted by the context's constructor.
  static_assert(!std::is_constructible_v<dispatcher_type, std::in_place_t>);
  static_assert(!std::is_constructible_v<dispatcher_type, std::in_place_t, char const*>);
  static_assert(!std::is_constructible_v<dispatcher_type, std::in_place_t, char const*, number&>);
  static_assert(!std::is_constructible_v<dispatcher_type, b7, std::uint8_t, std::in_place_t>);
  // The tag is required.
  static_assert(!std::is_constructible_v<dispatcher_type, char const*, number>);
  static_assert(!std::is_constructible_v<dispatcher_type, b7, std::uint8_t, char const*, number>);
  // A reference to a configuration has no context to be constructed.
  static_assert(!std::is_constructible_v<midi2::ci::ci_dispatcher<std::reference_wrapper<TypeParam>>, std::in_place_t,
                                         char const*, number>);
}
// NOLINTNEXTLINE
TEST(CIDispatcherInPlaceContext, NoArguments) {
  using config = midi2::ci::function_config<int, 32>;
  midi2::ci::ci_dispatcher<config> const a{std::in_place};
  EXPECT_EQ(a.config().context, 0);
  midi2::ci::ci_dispatcher<config> const b{0x12_b7, 3_u8, std::in_place};
  EXPECT_EQ(b.config().context, 0);
}
// NOLINTNEXTLINE
TEST(CIDispatcherInPlaceContext, Aggregate) {
  struct aggregate {
    int a;
    int b;
  };
  // This is only a compile-time check because clang's -Wc++20-extensions warns about the parenthesized initialization
  // of an aggregate (even when compiling as C++20 or later).
  using dispatcher_type = midi2::ci::ci_dispatcher<midi2::ci::function_config<aggregate, 32>>;
  static_assert(std::is_constructible_v<dispatcher_type, std::in_place_t, int, int>);
}
// NOLINTNEXTLINE
TEST(CIDispatcherInPlaceContext, ReferenceWrapper) {
  auto context = std::string{"name"};
  using config = midi2::ci::function_stream_config<std::reference_wrapper<std::string>, 32>;
  midi2::ci::ci_dispatcher<config> const dispatcher{std::in_place, context};
  EXPECT_EQ(&dispatcher.config().context.get(), &context);
}

// A context type which can be neither copied nor moved.
struct pinned_context {
  pinned_context() noexcept = default;
  pinned_context(pinned_context const&) = delete;
  pinned_context(pinned_context&&) noexcept = delete;
  ~pinned_context() noexcept = default;
  pinned_context& operator=(pinned_context const&) = delete;
  pinned_context& operator=(pinned_context&&) noexcept = delete;

  int calls = 0;
};

namespace backend = midi2::ci::dispatcher_backend;
static_assert(backend::system<backend::system_null<pinned_context>, pinned_context>);
static_assert(backend::system<backend::system_pure<pinned_context>, pinned_context>);
static_assert(backend::system<backend::system_base<pinned_context>, pinned_context>);
static_assert(backend::system<backend::system_function<pinned_context>, pinned_context>);
static_assert(backend::management<backend::management_null<pinned_context>, pinned_context>);
static_assert(backend::management<backend::management_pure<pinned_context>, pinned_context>);
static_assert(backend::management<backend::management_base<pinned_context>, pinned_context>);
static_assert(backend::management<backend::management_function<pinned_context>, pinned_context>);
static_assert(backend::profile<backend::profile_null<pinned_context>, pinned_context>);
static_assert(backend::profile<backend::profile_pure<pinned_context>, pinned_context>);
static_assert(backend::profile<backend::profile_base<pinned_context>, pinned_context>);
static_assert(backend::profile<backend::profile_function<pinned_context>, pinned_context>);
static_assert(backend::property_exchange<backend::property_exchange_null<pinned_context>, pinned_context>);
static_assert(backend::property_exchange<backend::property_exchange_pure<pinned_context>, pinned_context>);
static_assert(backend::property_exchange<backend::property_exchange_base<pinned_context>, pinned_context>);
static_assert(backend::property_exchange<backend::property_exchange_function<pinned_context>, pinned_context>);
static_assert(
    backend::property_exchange_stream<backend::property_exchange_stream_null<pinned_context>, pinned_context>);
static_assert(
    backend::property_exchange_stream<backend::property_exchange_stream_pure<pinned_context>, pinned_context>);
static_assert(
    backend::property_exchange_stream<backend::property_exchange_stream_base<pinned_context>, pinned_context>);
static_assert(
    backend::property_exchange_stream<backend::property_exchange_stream_function<pinned_context>, pinned_context>);
static_assert(backend::process_inquiry<backend::process_inquiry_null<pinned_context>, pinned_context>);
static_assert(backend::process_inquiry<backend::process_inquiry_pure<pinned_context>, pinned_context>);
static_assert(backend::process_inquiry<backend::process_inquiry_base<pinned_context>, pinned_context>);
static_assert(backend::process_inquiry<backend::process_inquiry_function<pinned_context>, pinned_context>);

// The handlers are passed a reference to the context so it cannot be const unless they accept a const reference.
static_assert(midi2::ci::ci_dispatcher_config<midi2::ci::function_config<int, 32>&>);
static_assert(!midi2::ci::ci_dispatcher_config<midi2::ci::function_config<int, 32> const&>);
static_assert(midi2::ci::ci_dispatcher_config<midi2::ci::function_config<int const, 32> const&>);

template <typename Config> class CIDispatcherPinnedContext : public testing::Test {};
using pinned_configs = testing::Types<midi2::ci::function_config<pinned_context, 32>,
                                      midi2::ci::function_stream_config<pinned_context, 32>>;
TYPED_TEST_SUITE(CIDispatcherPinnedContext, pinned_configs, );

// NOLINTNEXTLINE
TYPED_TEST(CIDispatcherPinnedContext, HandlersReceiveAReference) {
  midi2::ci::ci_dispatcher<TypeParam> dispatcher{std::in_place};
  auto& config = dispatcher.config();
  pinned_context const* check_muid_context = nullptr;
  pinned_context const* capabilities_context = nullptr;
  config.system.on_check_muid([&check_muid_context](pinned_context& c, std::uint8_t, midi2::ci::muid) {
    ++c.calls;
    check_muid_context = &c;
    return true;
  });
  config.process_inquiry.on_capabilities([&capabilities_context](pinned_context& c, header const&) {
    ++c.calls;
    capabilities_context = &c;
  });

  std::vector<std::byte> message;
  midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{},
                            header{.device_id = 0_b7, .version = 2_b7, .remote_muid = 1_b28, .local_muid = 2_b28},
                            midi2::ci::process_inquiry::capabilities{});
  std::ranges::for_each(message, [&dispatcher](std::byte const b) { dispatcher.dispatch(b); });

  EXPECT_EQ(check_muid_context, &config.context);
  EXPECT_EQ(capabilities_context, &config.context);
  EXPECT_EQ(config.context.calls, 2);
}
// NOLINTNEXTLINE
TEST(CIDispatcherContext, MoveOnly) {
  using context = std::unique_ptr<int>;
  auto dispatcher = midi2::ci::make_function_dispatcher<context, 32>(0_b7, 0_u8, std::make_unique<int>(42));
  auto& config = dispatcher.config();
  ASSERT_NE(config.context, nullptr);
  EXPECT_EQ(*config.context, 42);
  config.system.on_check_muid([](context& c, std::uint8_t, midi2::ci::muid) {
    ++*c;
    return false;
  });
  EXPECT_FALSE(config.system.check_muid(config.context, 0_u8, midi2::ci::muid{}));
  EXPECT_EQ(*config.context, 43);
}

// This test simply gets ci_dispatcher to consume a random buffer.
void NeverCrashes(std::vector<std::byte> const& message) {
  // Ensure the top bit of each byte of the incoming message stream is clear.
  std::vector<std::byte> message2;
  message2.reserve(message.size());
  std::ranges::transform(message, std::back_inserter(message2), [](std::byte v) { return v & 0x7F_b; });

  // A context type for the dispatcher
  struct empty {};

  static constexpr auto buffer_size = std::size_t{64};
  auto dispatcher = midi2::ci::make_function_dispatcher<empty, buffer_size>();

  dispatcher.config().system.on_check_muid([](empty, std::uint8_t, midi2::ci::muid) { return true; });
  std::ranges::for_each(message2, std::bind_front(&decltype(dispatcher)::dispatch, &dispatcher));
}

// The same as NeverCrashes() but with a dispatcher which streams property exchange messages.
void NeverCrashesStreaming(std::vector<std::byte> const& message) {
  std::vector<std::byte> message2;
  message2.reserve(message.size());
  std::ranges::transform(message, std::back_inserter(message2), [](std::byte v) { return v & 0x7F_b; });

  struct empty {};
  static constexpr auto buffer_size = std::size_t{16};
  midi2::ci::ci_dispatcher<midi2::ci::function_stream_config<empty, buffer_size>> dispatcher;
  dispatcher.config().system.on_check_muid([](empty, std::uint8_t, midi2::ci::muid) { return true; });
  std::ranges::for_each(message2, std::bind_front(&decltype(dispatcher)::dispatch, &dispatcher));
  dispatcher.reset();
}

#if defined(MIDI2_FUZZTEST) && MIDI2_FUZZTEST
// NOLINTNEXTLINE
FUZZ_TEST(CIProcessorFuzz, NeverCrashes);
// NOLINTNEXTLINE
FUZZ_TEST(CIProcessorFuzz, NeverCrashesStreaming);
#endif
// NOLINTNEXTLINE
TEST(CIProcessorFuzz, Empty) {
  NeverCrashes({});
  NeverCrashesStreaming({});
}

}  // end anonymous namespace
