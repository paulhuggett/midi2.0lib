//===-- CI Types --------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ci_types.hpp
/// \brief Types and constants for MIDI CI

#ifndef MIDI2_CI_TYPES_HPP
#define MIDI2_CI_TYPES_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

#include "midi2/adt/bitfield.hpp"
#include "midi2/adt/uinteger.hpp"
#include "midi2/bytestream/bytestream_types.hpp"

namespace midi2 {
namespace ci::details {

/// \brief Represents an unsigned integer with a specific number of bits.
///
/// \tparam Bits  The number of bits in the value.
/// \tparam Underlying  The unsigned integer type used to store the value.
template <unsigned Bits, std::unsigned_integral Underlying = midi2::adt::uinteger_t<Bits>>
  requires(adt::max_value<Underlying, Bits>() <= std::numeric_limits<Underlying>::max())
class bn {
public:
  /// \brief The unsigned integer type used to store the value.
  using underlying_type = Underlying;
  /// \brief The number of bits in the value.
  static constexpr auto bits = Bits;

  /// \brief Constructs a value of zero.
  constexpr bn() noexcept = default;
  /// \brief Copy constructor.
  /// \param rhs  The value to be copied.
  constexpr bn(bn const& rhs) noexcept = default;
  /// \brief Move constructor.
  /// \param rhs  The value to be moved.
  constexpr bn(bn&& rhs) noexcept = default;
  /// \brief Constructs from an unsigned integer value.
  /// \tparam UInt  An unsigned integral type.
  /// \param v  The initial value. It must be representable in \p Bits bits (this is checked by assertion).
  template <std::unsigned_integral UInt>
  constexpr explicit bn(UInt const v) noexcept : value_{static_cast<Underlying>(v)} {
    assert((v <= adt::max_value<Underlying, Bits>()) && "Value is too large");
  }
  /// \brief Destructor.
  constexpr ~bn() noexcept = default;

  /// \brief Converts to the underlying integer type.
  /// \returns The value.
  constexpr explicit operator Underlying() const noexcept { return value_; }

  /// \brief Three-way comparison of two values.
  /// \param lhs  The first value to be compared.
  /// \param rhs  The second value to be compared.
  /// \returns The ordering of \p lhs relative to \p rhs.
  constexpr friend std::strong_ordering operator<=>(bn const& lhs, bn const& rhs) noexcept = default;

  /// \brief Copy assignment operator.
  /// \param rhs  The value to be copied.
  /// \returns *this
  constexpr bn& operator=(bn const& rhs) noexcept = default;
  /// \brief Move assignment operator.
  /// \param rhs  The value to be moved.
  /// \returns *this
  constexpr bn& operator=(bn&& rhs) noexcept = default;
  /// \brief Assigns an integer value.
  /// \param rhs  The new value. It must be representable in \p Bits bits (this is checked by assertion).
  /// \returns *this
  constexpr bn& operator=(Underlying const rhs) noexcept {
    assert((rhs <= adt::max_value<Underlying, Bits>()) && "Value is too large");
    value_ = rhs;
    return *this;
  }

  /// \brief Returns the value.
  /// \returns The value as the underlying integer type.
  [[nodiscard]] constexpr Underlying get() const noexcept { return value_; }

private:
  Underlying value_ = 0;
};

}  // end namespace ci::details

namespace ci {

/// A seven bit unsigned integer type.
using b7 = ci::details::bn<7>;
/// A fourteen bit unsigned integer type.
using b14 = ci::details::bn<14>;
/// A twenty eight bit unsigned integer type.
using b28 = ci::details::bn<28>;

}  // namespace ci

/// \brief User-defined literals for the MIDI-CI integer types.
namespace literals {

/// \brief A user-defined literal which creates a midi2::ci::b7 value from a character.
/// \param arg  The character. Its value must be less than 0x80.
/// \returns The b7 value.
[[nodiscard]] consteval ci::b7 operator""_b7(char const arg) noexcept {
  assert(arg < (1 << 7));
  return ci::b7{static_cast<std::uint8_t>(arg)};
}
/// \brief A user-defined literal which creates a midi2::ci::b7 value.
/// \param arg  The value. It must be less than 2^7.
/// \returns The b7 value.
[[nodiscard]] consteval ci::b7 operator""_b7(unsigned long long const arg) noexcept {
  assert(arg < (1 << 7));
  return ci::b7{arg};
}
/// \brief A user-defined literal which creates a midi2::ci::b14 value.
/// \param arg  The value. It must be less than 2^14.
/// \returns The b14 value.
[[nodiscard]] consteval ci::b14 operator""_b14(unsigned long long const arg) noexcept {
  assert(arg < (1 << 14));
  return ci::b14{arg};
}
/// \brief A user-defined literal which creates a midi2::ci::b28 value.
/// \param arg  The value. It must be less than 2^28.
/// \returns The b28 value.
[[nodiscard]] consteval ci::b28 operator""_b28(unsigned long long const arg) noexcept {
  assert(arg < (1 << 28));
  return ci::b28{arg};
}

}  // end namespace literals

}  // end namespace midi2

/// \brief A std::formatter specialization which formats a midi2::ci::details::bn<> value as its underlying integer.
/// \tparam Bits  The number of bits in the value.
/// \tparam Underlying  The type used to store the value.
/// \tparam CharT  The character type.
template <unsigned Bits, std::unsigned_integral Underlying, typename CharT>
struct std::formatter<midi2::ci::details::bn<Bits, Underlying>, CharT> : std::formatter<Underlying, CharT> {
  /// \brief Formats a value.
  /// \param value  The value to be formatted.
  /// \param format_ctx  The formatting context.
  /// \returns An iterator past the end of the output.
  auto format(midi2::ci::details::bn<Bits, Underlying> value, auto& format_ctx) const {
    return std::formatter<Underlying, CharT>::format(Underlying{value}, format_ctx);
  }
};

namespace midi2::ci {

/// \brief The MIDI-CI message types: the values of the Universal System Exclusive Sub-ID#2 field (M2-101 section 5.1).
enum class message : std::uint8_t {
  /// Initiate Protocol Negotiation. Deprecated as of MIDI-CI version 1.2.
  protocol_negotiation = 0x10,
  /// Reply to Initiate Protocol Negotiation. Deprecated as of MIDI-CI version 1.2.
  protocol_negotiation_reply = 0x11,
  /// Set New Selected Protocol. Deprecated as of MIDI-CI version 1.2.
  protocol_set = 0x12,
  /// Test New Protocol Initiator to Responder. Deprecated as of MIDI-CI version 1.2.
  protocol_test = 0x13,
  /// Test New Protocol Responder to Initiator. Deprecated as of MIDI-CI version 1.2.
  protocol_test_responder = 0x14,
  /// Confirmation New Protocol Established. Deprecated as of MIDI-CI version 1.2.
  protocol_confirm = 0x15,

  /// Profile Inquiry.
  profile_inquiry = 0x20,
  /// Reply to Profile Inquiry.
  profile_inquiry_reply = 0x21,
  /// Set Profile On.
  profile_set_on = 0x22,
  /// Set Profile Off.
  profile_set_off = 0x23,
  /// Profile Enabled Report.
  profile_enabled = 0x24,
  /// Profile Disabled Report.
  profile_disabled = 0x25,
  /// Profile Added Report.
  profile_added = 0x26,
  /// Profile Removed Report.
  profile_removed = 0x27,
  /// Profile Details Inquiry.
  profile_details = 0x28,
  /// Reply to Profile Details Inquiry.
  profile_details_reply = 0x29,
  /// Profile Specific Data.
  profile_specific_data = 0x2F,

  /// Inquiry: Property Exchange Capabilities.
  pe_capability = 0x30,
  /// Reply to Property Exchange Capabilities.
  pe_capability_reply = 0x31,
  /// Inquiry: Get Property Data.
  pe_get = 0x34,
  /// Reply to Get Property Data.
  pe_get_reply = 0x35,
  /// Inquiry: Set Property Data.
  pe_set = 0x36,
  /// Reply to Set Property Data.
  pe_set_reply = 0x37,
  /// Subscription.
  pe_sub = 0x38,
  /// Reply to Subscription.
  pe_sub_reply = 0x39,
  /// Notify.
  pe_notify = 0x3F,

  /// Inquiry: Process Inquiry Capabilities.
  pi_capability = 0x40,
  /// Reply to Process Inquiry Capabilities.
  pi_capability_reply = 0x41,
  /// Inquiry: MIDI Message Report.
  pi_mm_report = 0x42,
  /// Reply to MIDI Message Report.
  pi_mm_report_reply = 0x43,
  /// End of MIDI Message Report.
  pi_mm_report_end = 0x44,

  /// Discovery.
  discovery = 0x70,
  /// Reply to Discovery.
  discovery_reply = 0x71,
  /// Inquiry: Endpoint.
  endpoint = 0x72,
  /// Reply to Endpoint.
  endpoint_reply = 0x73,
  /// MIDI-CI ACK.
  ack = 0x7D,
  /// Invalidate MUID.
  invalidate_muid = 0x7E,
  /// MIDI-CI NAK.
  nak = 0x7F,
};

/// \brief Property Exchange reply status codes (M2-103 Common Rules for Property Exchange, Table 15).
enum class pe_status {
  /// Success/OK.
  ok = 200,
  /// Accepted: an Inquiry: Set Property Data message was accepted but the device cannot guarantee the result.
  accepted = 202,
  /// Resource currently unavailable or an error occurred.
  resource_unavailable = 341,
  /// Bad data or unexpected end of data set.
  bad_data = 342,
  /// Too many requests: the device is unable to support this additional request at this time.
  too_many_reqs = 343,
  /// Bad request: data was received but it is not correct.
  bad_req = 400,
  /// The request was received but a reply is not available based on authorization.
  req_unauthorized = 403,
  /// Resource not supported or not found.
  resource_unsupported = 404,
  /// Resource not allowed: the resource is not applicable at this time.
  resource_not_allowed = 405,
  /// Payload too large.
  payload_too_large = 413,
  /// Unsupported media type or encoding.
  unsupported_media_type = 415,
  /// Invalid version of data.
  invalid_data_version = 445,
  /// Internal device error.
  internal_device_error = 500,
};

/// \brief Values of the Property Exchange "command" header property, used by Subscription messages.
enum class pe_command : std::uint8_t {
  /// Start a subscription.
  start = 1,
  /// End a subscription.
  end = 2,
  /// Notify the subscriber of a partial change to the subscribed resource.
  partial = 3,
  /// Notify the subscriber of a full replacement of the subscribed resource.
  full = 4,
  /// Notify the subscriber that the resource has changed so that it can request the data.
  notify = 5,
};

/// \brief Property Exchange actions on a resource.
enum class pe_action : std::uint8_t {
  /// Copy a resource.
  copy = 1,
  /// Move a resource.
  move = 2,
  /// Delete a resource.
  del = 3,
  /// Create a directory.
  create_dir = 4,
};

/// \brief Property Exchange data encodings (the "mutualEncoding" header property).
enum class pe_encoding : std::uint8_t {
  /// ASCII text (7-bit data).
  ascii = 1,
  /// Mcoded7 8-bit to 7-bit encoding.
  mcoded7 = 2,
  /// zlib compressed and then encoded using Mcoded7.
  mcoded7zlib = 3,
};

/// \brief An array of bytes.
/// \tparam Size  The number of elements in the array.
template <std::size_t Size> using byte_array = std::array<std::byte, Size>;
/// \brief An array of 7-bit values.
/// \tparam Size  The number of elements in the array.
template <std::size_t Size> using b7_array = std::array<b7, Size>;

/// \brief A MIDI-CI MUID: a 28-bit number used to uniquely identify a MIDI-CI device.
using muid = b28;

/// MUID is a 28-bit random number generated by a MIDI-CI Device used to uniquely identify MIDI-CI messages.
/// This constant holds the largest available MUID that is neither reserved nor used as a broadcast MUID.
constexpr auto max_user_muid = muid{0x0FFFFF00U - 1U};
/// A special MUID value that is reserved for messages that are directed to all listening MIDI-CI devices.
constexpr auto broadcast_muid = muid{0x0FFFFFFFU};

/// \brief Implementation details for MIDI-CI types.
namespace details {

/// \brief A mask which selects the low seven bits of a byte.
constexpr auto mask7b = std::byte{(1 << 7) - 1};

/// \brief Converts four 7-bit bytes (least-significant first) to a 28-bit value.
/// \param v  The bytes to be converted. The most-significant bit of each must be clear.
/// \returns The 28-bit value.
[[nodiscard]] constexpr b28 from_le7(byte_array<4> const& v) noexcept {
  assert(((v[0] | v[1] | v[2] | v[3]) & std::byte{0x80}) == std::byte{0U});
  using ut = b28::underlying_type;
  return b28{(static_cast<ut>(to_underlying(v[0] & mask7b)) << (7 * 0)) |
             (static_cast<ut>(to_underlying(v[1] & mask7b)) << (7 * 1)) |
             (static_cast<ut>(to_underlying(v[2] & mask7b)) << (7 * 2)) |
             (static_cast<ut>(to_underlying(v[3] & mask7b)) << (7 * 3))};
}
/// \brief Converts two 7-bit bytes (least-significant first) to a 14-bit value.
/// \param v  The bytes to be converted. The most-significant bit of each must be clear.
/// \returns The 14-bit value.
[[nodiscard]] constexpr b14 from_le7(byte_array<2> const& v) noexcept {
  assert(((v[0] | v[1]) & std::byte{0x80}) == std::byte{0U});
  using ut = b14::underlying_type;
  return b14{static_cast<ut>((static_cast<ut>(to_underlying(v[0] & mask7b)) << (7 * 0)) |
                             (static_cast<ut>(to_underlying(v[1] & mask7b)) << (7 * 1)))};
}
/// \brief Converts a 7-bit byte to a b7 value.
/// \param v  The byte to be converted. Its most-significant bit must be clear.
/// \returns The 7-bit value.
[[nodiscard]] constexpr b7 from_le7(std::byte const v) noexcept {
  assert((v & std::byte{0x80}) == std::byte{0U});
  return b7{to_underlying(v)};
}
/// \brief Converts an array of five 7-bit bytes to an array of b7 values.
/// \param v  The bytes to be converted. The most-significant bit of each must be clear.
/// \returns The array of 7-bit values.
[[nodiscard]] constexpr b7_array<5> from_le7(byte_array<5> const& v) noexcept {
  b7_array<5> result;
  std::ranges::transform(v, std::begin(result), [](std::byte const b) constexpr { return from_le7(b); });
  return result;
}

/// \brief Converts a 28-bit value to four 7-bit bytes (least-significant first).
/// \param v28  The value to be converted.
/// \returns The four bytes.
[[nodiscard]] constexpr byte_array<4> to_le7(b28 const v28) noexcept {
  auto const v = v28.get();
  assert(v < (std::uint32_t{1} << 28));
  return {static_cast<std::byte>(v >> (7 * 0)) & mask7b, static_cast<std::byte>(v >> (7 * 1)) & mask7b,
          static_cast<std::byte>(v >> (7 * 2)) & mask7b, static_cast<std::byte>(v >> (7 * 3)) & mask7b};
}
/// \brief Converts a 14-bit value to two 7-bit bytes (least-significant first).
/// \param v14  The value to be converted.
/// \returns The two bytes.
[[nodiscard]] constexpr byte_array<2> to_le7(b14 const v14) noexcept {
  auto const v = v14.get();
  assert(v < (std::uint16_t{1} << 14));
  return {static_cast<std::byte>(v >> (7 * 0)) & mask7b, static_cast<std::byte>(v >> (7 * 1)) & mask7b};
}
/// \brief Converts a 7-bit value to a byte.
/// \param v  The value to be converted.
/// \returns The byte.
[[nodiscard]] constexpr std::byte to_le7(b7 const v) noexcept {
  assert(v.get() < (std::uint8_t{1} << 7));
  return static_cast<std::byte>(v.get());
}
/// \brief Converts an array of five 7-bit values to an array of bytes.
/// \param v  The values to be converted.
/// \returns The array of bytes.
[[nodiscard]] constexpr byte_array<5> to_le7(b7_array<5> const& v) noexcept {
  byte_array<5> result;
  std::ranges::transform(v, std::begin(result), [](b7 const b) constexpr { return to_le7(b); });
  return result;
}

/// \brief Converts an array of bytes to an array of 7-bit values.
/// \tparam Size  The number of elements in the array.
/// \param other  The bytes to be converted. The most-significant bit of each must be clear.
/// \returns The array of 7-bit values.
template <std::size_t Size>
[[nodiscard]] constexpr b7_array<Size> from_byte_array(byte_array<Size> const& other) noexcept {
  b7_array<Size> result{};
  std::ranges::transform(other, std::begin(result), [](std::byte const v) constexpr { return from_le7(v); });
  return result;
}

/// \brief Converts an array of 7-bit values to an array of bytes.
/// \tparam Size  The number of elements in the array.
/// \param other  The values to be converted.
/// \returns The array of bytes.
template <std::size_t Size>
[[nodiscard]] constexpr byte_array<Size> to_byte_array(b7_array<Size> const& other) noexcept {
  byte_array<Size> result{};
  std::ranges::transform(other, std::begin(result), [](b7 const v) constexpr { return std::byte{v.get()}; });
  return result;
}

}  // end namespace details

/// \brief Packed (on-the-wire) representations of MIDI-CI messages.
namespace packed {

/// \brief The packed (on-the-wire) layout of the fields common to all MIDI-CI messages (M2-101 section 5.2.1).
struct header {
  std::byte sysex;  ///< Universal System Exclusive (0x7E, Non-Real Time)
  /// Device ID: Source or Destination (depending on type of message):
  /// - 00–0F: To/from MIDI Channels 1-16
  /// - 10–7D: Reserved
  /// - 7E: To/from Group
  /// - 7F: To/from Function Block
  std::byte source;
  std::byte sub_id_1;  ///< Universal System Exclusive Sub-ID#1: MIDI-CI (0x0D)
  /// The MIDI-CI message. Will be one of the values from the midi2::ci::message enum.
  std::byte sub_id_2;
  /// MIDI-CI Message Version/Format
  std::byte version;
  /// Source MUID (LSB first)
  byte_array<4> source_muid;
  /// Destination MUID (LSB first)
  byte_array<4> destination_muid;
};

static_assert(offsetof(header, sysex) == 0);
static_assert(offsetof(header, source) == 1);
static_assert(offsetof(header, sub_id_1) == 2);
static_assert(offsetof(header, sub_id_2) == 3);
static_assert(offsetof(header, version) == 4);
static_assert(offsetof(header, source_muid) == 5);
static_assert(offsetof(header, destination_muid) == 9);
static_assert(sizeof(header) == 13);
static_assert(alignof(header) == 1);
static_assert(std::is_trivially_copyable_v<header>);

}  // end namespace packed

/// \brief The fields common to all MIDI-CI messages (M2-101 section 5.2.1).
struct header {
  /// \brief Compares two header values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(header const&, header const&) noexcept = default;
  /// \brief Converts to the packed (on-the-wire) header representation.
  /// \note The Sub-ID#2 (message type) field of the result is set to zero: the caller must fill it in.
  /// \returns The packed representation.
  explicit constexpr operator packed::header() const noexcept;

  /// \brief Device ID: Source or Destination (depending on type of message):
  /// - 00–0F: To/from MIDI Channels 1-16
  /// - 10–7D: Reserved
  /// - 7E: To/from Group
  /// - 7F: To/from Function Block
  b7 device_id = b7{0U};
  /// MIDI-CI Message Version/Format
  b7 version = b7{1U};
  /// \brief The Source MUID: the MUID of the device which sent the message.
  muid remote_muid;
  /// \brief The Destination MUID: the MUID of the device to which the message is addressed.
  muid local_muid;
};

constexpr header::operator packed::header() const noexcept {
  return packed::header{.sysex = bytestream::s7_universal_nrt,
                        .source = static_cast<std::byte>(device_id.get()),
                        .sub_id_1 = bytestream::s7_midi_ci,
                        .sub_id_2 = std::byte{0U},  // message type
                        .version = static_cast<std::byte>(version.get()),
                        .source_muid = ci::details::to_le7(remote_muid),
                        .destination_muid = ci::details::to_le7(local_muid)};
}

//*     _ _                              *
//*  __| (_)___ __ _____ _____ _ _ _  _  *
//* / _` | (_-</ _/ _ \ V / -_) '_| || | *
//* \__,_|_/__/\__\___/\_/\___|_|  \_, | *
//*                                |__/  *
namespace packed {

/// \brief Version 1 of the MIDI-CI Discovery message.
struct discovery_v1 {
  /// \brief Device Manufacturer (System Exclusive ID Number).
  byte_array<3> manufacturer;
  /// \brief Device Family (LSB first).
  byte_array<2> family;
  /// \brief Device Family Model Number (LSB first).
  byte_array<2> model;
  /// \brief Software Revision Level (format is device specific).
  byte_array<4> version;
  /// \brief Capability Inquiry Category Supported (bitmap).
  std::byte capability;
  /// \brief Receivable Maximum SysEx Message Size (LSB first).
  byte_array<4> max_sysex_size;
};
static_assert(offsetof(discovery_v1, manufacturer) == 0);
static_assert(offsetof(discovery_v1, family) == 3);
static_assert(offsetof(discovery_v1, model) == 5);
static_assert(offsetof(discovery_v1, version) == 7);
static_assert(offsetof(discovery_v1, capability) == 11);
static_assert(offsetof(discovery_v1, max_sysex_size) == 12);
static_assert(sizeof(discovery_v1) == 16);
static_assert(alignof(discovery_v1) == 1);
static_assert(std::is_trivially_copyable_v<discovery_v1>);

/// \brief Version 2 of the MIDI-CI Discovery message.
struct discovery_v2 {
  /// \brief The fields from version 1 of the message.
  discovery_v1 v1;
  /// \brief Initiator's Output Path ID.
  std::byte output_path_id;
};
static_assert(offsetof(discovery_v2, v1) == 0);
static_assert(offsetof(discovery_v2, output_path_id) == 16);
static_assert(sizeof(discovery_v2) == 17);
static_assert(alignof(discovery_v2) == 1);
static_assert(std::is_trivially_copyable_v<discovery_v2>);

static_assert(sizeof(discovery_v2) > sizeof(discovery_v1));

}  // end namespace packed

/// \brief The fields of a MIDI-CI Discovery message (M2-101 section 5.5).
///
/// An Initiator shall establish connections to MIDI-CI Responders by sending a Discovery message.
struct discovery {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Discovery message representation.
  /// \param v1  The packed version 1 message.
  /// \param output_path_id  The Initiator's Output Path ID. Version 1 messages do not include this field.
  /// \returns The new instance.
  [[nodiscard]] static constexpr discovery make(packed::discovery_v1 const& v1, b7 output_path_id = b7{}) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Discovery message representation.
  /// \param v2  The packed version 2 message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr discovery make(packed::discovery_v2 const& v2) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Discovery message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::discovery_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Discovery message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::discovery_v2() const noexcept;
  /// \brief Compares two discovery values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(discovery const&, discovery const&) noexcept = default;

  /// Device Manufacturer (System Exclusive ID Number)
  b7_array<3> manufacturer{};
  /// Device Family
  b14 family;
  /// Device Family Model Number
  b14 model;
  /// Software Revision Level (Format is Device specific)
  b7_array<4> version{};
  /// Capability Inquiry Category Supported (bitmap). D1 (0x02): Protocol Negotiation (deprecated); D2 (0x04): Profile
  /// Configuration; D3 (0x08): Property Exchange; D4 (0x10): Process Inquiry.
  b7 capability;
  /// Receivable Maximum SysEx Message Size
  b28 max_sysex_size;
  /// Initiator's Output Path ID
  b7 output_path_id;
};

constexpr discovery discovery::make(packed::discovery_v1 const& v1, b7 output_path_id) noexcept {
  return discovery{.manufacturer = details::from_byte_array(v1.manufacturer),
                   .family = details::from_le7(v1.family),
                   .model = details::from_le7(v1.model),
                   .version = details::from_byte_array(v1.version),
                   .capability = details::from_le7(v1.capability),
                   .max_sysex_size = details::from_le7(v1.max_sysex_size),
                   .output_path_id = output_path_id};
}
constexpr discovery discovery::make(packed::discovery_v2 const& v2) noexcept {
  return make(v2.v1, details::from_le7(v2.output_path_id));
}
constexpr discovery::operator packed::discovery_v1() const noexcept {
  return {.manufacturer = details::to_byte_array(manufacturer),
          .family = details::to_le7(family),
          .model = details::to_le7(model),
          .version = details::to_byte_array(version),
          .capability = static_cast<std::byte>(capability.get()),
          .max_sysex_size = details::to_le7(max_sysex_size)};
}
constexpr discovery::operator packed::discovery_v2() const noexcept {
  return {.v1 = static_cast<packed::discovery_v1>(*this), .output_path_id = details::to_le7(output_path_id)};
}

//*     _ _                                            _       *
//*  __| (_)___ __ _____ _____ _ _ _  _   _ _ ___ _ __| |_  _  *
//* / _` | (_-</ _/ _ \ V / -_) '_| || | | '_/ -_) '_ \ | || | *
//* \__,_|_/__/\__\___/\_/\___|_|  \_, | |_| \___| .__/_|\_, | *
//*                                |__/          |_|     |__/  *
namespace packed {

/// \brief Version 1 of the MIDI-CI Reply to Discovery message.
struct discovery_reply_v1 {
  /// \brief Device Manufacturer (System Exclusive ID Number).
  byte_array<3> manufacturer;
  /// \brief Device Family (LSB first).
  byte_array<2> family;
  /// \brief Device Family Model Number (LSB first).
  byte_array<2> model;
  /// \brief Software Revision Level (format is device specific).
  byte_array<4> version;
  /// \brief Capability Inquiry Category Supported (bitmap).
  std::byte capability;
  /// \brief Receivable Maximum SysEx Message Size (LSB first).
  byte_array<4> max_sysex_size;
};
static_assert(offsetof(discovery_reply_v1, manufacturer) == 0);
static_assert(offsetof(discovery_reply_v1, family) == 3);
static_assert(offsetof(discovery_reply_v1, model) == 5);
static_assert(offsetof(discovery_reply_v1, version) == 7);
static_assert(offsetof(discovery_reply_v1, capability) == 11);
static_assert(offsetof(discovery_reply_v1, max_sysex_size) == 12);
static_assert(sizeof(discovery_reply_v1) == 16);
static_assert(alignof(discovery_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<discovery_reply_v1>);

/// \brief Version 2 of the MIDI-CI Reply to Discovery message.
struct discovery_reply_v2 {
  /// \brief The fields from version 1 of the message.
  discovery_reply_v1 v1;
  /// \brief Initiator's Output Path ID (from the Discovery message received).
  std::byte output_path_id;
  /// \brief Function Block.
  std::byte function_block;
};
static_assert(offsetof(discovery_reply_v2, v1) == 0);
static_assert(offsetof(discovery_reply_v2, output_path_id) == 16);
static_assert(offsetof(discovery_reply_v2, function_block) == 17);
static_assert(sizeof(discovery_reply_v2) == 18);
static_assert(alignof(discovery_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<discovery_reply_v2>);

static_assert(sizeof(discovery_reply_v1) <= sizeof(discovery_reply_v2));

}  // end namespace packed

/// \brief Reply to Discovery Message (M2-101 section 5.6).
///
/// When a MIDI-CI Device receives a Discovery message it shall become a Responder and send this
/// Reply to Discovery message. This message declares the MUID of the Responder.
struct discovery_reply {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Reply to Discovery message representation.
  /// \param v1  The packed version 1 message.
  /// \param output_path_id  The Initiator's Output Path ID. Version 1 messages do not include this field.
  /// \param function_block  The Function Block. Version 1 messages do not include this field.
  /// \returns The new instance.
  [[nodiscard]] static constexpr discovery_reply make(packed::discovery_reply_v1 const& v1, b7 output_path_id = b7{},
                                                      b7 function_block = b7{}) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Reply to Discovery message representation.
  /// \param v2  The packed version 2 message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr discovery_reply make(packed::discovery_reply_v2 const& v2) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Reply to Discovery message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::discovery_reply_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Reply to Discovery message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::discovery_reply_v2() const noexcept;
  /// \brief Compares two discovery_reply values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(discovery_reply const&, discovery_reply const&) noexcept = default;

  /// Device Manufacturer (System Exclusive ID Number)
  b7_array<3> manufacturer{};
  /// Device Family
  b14 family;
  /// Device Family Model Number
  b14 model;
  /// Software Revision Level (Format is Device specific)
  b7_array<4> version{};
  /// Capability Inquiry Category Supported (bitmap). D1 (0x02): Protocol Negotiation (deprecated); D2 (0x04): Profile
  /// Configuration; D3 (0x08): Property Exchange; D4 (0x10): Process Inquiry.
  b7 capability;
  /// Receivable Maximum SysEx Message Size
  b28 max_sysex_size;
  /// Initiator's Output Path Instance ID (from the Discovery message received)
  b7 output_path_id;
  /// Function Block
  b7 function_block;
};
constexpr discovery_reply discovery_reply::make(packed::discovery_reply_v1 const& v1, b7 output_path_id,
                                                b7 function_block) noexcept {
  return {.manufacturer = details::from_byte_array(v1.manufacturer),
          .family = details::from_le7(v1.family),
          .model = details::from_le7(v1.model),
          .version = details::from_byte_array(v1.version),
          .capability = details::from_le7(v1.capability),
          .max_sysex_size = details::from_le7(v1.max_sysex_size),
          .output_path_id = output_path_id,
          .function_block = function_block};
}
constexpr discovery_reply discovery_reply::make(packed::discovery_reply_v2 const& v2) noexcept {
  return make(v2.v1, details::from_le7(v2.output_path_id), details::from_le7(v2.function_block));
}
constexpr discovery_reply::operator packed::discovery_reply_v1() const noexcept {
  return packed::discovery_reply_v1{.manufacturer = details::to_byte_array(manufacturer),
                                    .family = details::to_le7(family),
                                    .model = details::to_le7(model),
                                    .version = details::to_byte_array(version),
                                    .capability = details::to_le7(capability),
                                    .max_sysex_size = details::to_le7(max_sysex_size)};
}
constexpr discovery_reply::operator packed::discovery_reply_v2() const noexcept {
  return {.v1 = static_cast<packed::discovery_reply_v1>(*this),
          .output_path_id = details::to_le7(output_path_id),
          .function_block = details::to_le7(function_block)};
}

//*              _           _     _    *
//*  ___ _ _  __| |_ __  ___(_)_ _| |_  *
//* / -_) ' \/ _` | '_ \/ _ \ | ' \  _| *
//* \___|_||_\__,_| .__/\___/_|_||_\__| *
//*               |_|                   *
namespace packed {

/// \brief Version 1 of the MIDI-CI Inquiry: Endpoint message.
struct endpoint_v1 {
  /// \brief The Status field which selects the information to be retrieved.
  std::byte status;
};
static_assert(offsetof(endpoint_v1, status) == 0);
static_assert(sizeof(endpoint_v1) == 1);
static_assert(alignof(endpoint_v1) == 1);
static_assert(std::is_trivially_copyable_v<endpoint_v1>);

}  // end namespace packed

/// \brief Inquiry: Endpoint Message (M2-101 section 5.7).
///
/// An Initiator may send the Inquiry: Endpoint Message to a Function Block in a Responder to get
/// information about the UMP Endpoint which has the Function Block. A Status field selects the
/// target data.
struct endpoint {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Inquiry: Endpoint message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr endpoint make(packed::endpoint_v1 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Inquiry: Endpoint message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::endpoint_v1() const noexcept;
  /// \brief Compares two endpoint values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(endpoint const&, endpoint const&) noexcept = default;

  /// The Status field defines which information to retrieve from the Responder. 0x00 requests the Product Instance ID.
  b7 status;
};

constexpr endpoint endpoint::make(packed::endpoint_v1 const& other) noexcept {
  return {.status = b7{to_underlying(other.status)}};
}
constexpr endpoint::operator packed::endpoint_v1() const noexcept {
  return {.status = static_cast<std::byte>(status.get())};
}

//*              _           _     _                  _       *
//*  ___ _ _  __| |_ __  ___(_)_ _| |_   _ _ ___ _ __| |_  _  *
//* / -_) ' \/ _` | '_ \/ _ \ | ' \  _| | '_/ -_) '_ \ | || | *
//* \___|_||_\__,_| .__/\___/_|_||_\__| |_| \___| .__/_|\_, | *
//*               |_|                           |_|     |__/  *
namespace packed {

/// \brief Version 1 of the MIDI-CI Reply to Endpoint message.
///
/// The structure is followed by the remainder of the information data.
struct endpoint_reply_v1 {
  /// \brief The Status value from the Inquiry: Endpoint message.
  std::byte status;
  /// \brief Length of the following information data (LSB first).
  byte_array<2> data_length;
  // Don't be tempted to change to std::array<>. MSVC's iterator checks will trip.
  std::byte data[1];  ///< Information data: an array of size given by data_length
};
static_assert(offsetof(endpoint_reply_v1, status) == 0);
static_assert(offsetof(endpoint_reply_v1, data_length) == 1);
static_assert(offsetof(endpoint_reply_v1, data) == 3);
static_assert(sizeof(endpoint_reply_v1) == 4);
static_assert(alignof(endpoint_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<endpoint_reply_v1>);

}  // end namespace packed

/// \brief Reply to Endpoint Message (M2-101 section 5.8).
struct endpoint_reply {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Reply to Endpoint message representation.
  /// \note The resulting object refers to the information data in \p other, which must remain valid for as long as the
  /// object is in use.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr endpoint_reply make(packed::endpoint_reply_v1 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Reply to Endpoint message representation.
  /// \note Only the fixed-size part of the message is produced. The information data must be written separately,
  /// immediately after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::endpoint_reply_v1() const noexcept;
  /// \brief Compares two endpoint_reply values for equality.
  /// \param lhs  The first value to be compared.
  /// \param rhs  The second value to be compared.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(endpoint_reply const& lhs, endpoint_reply const& rhs) noexcept {
    return lhs.status == rhs.status && std::ranges::equal(lhs.information, rhs.information);
  }

  /// \brief The Status value from the Inquiry: Endpoint message.
  b7 status;
  /// \brief The information data.
  std::span<b7 const> information;
};

constexpr endpoint_reply endpoint_reply::make(packed::endpoint_reply_v1 const& other) noexcept {
  return {.status = details::from_le7(other.status),
          .information{std::bit_cast<b7 const*>(&other.data[0]), details::from_le7(other.data_length).get()}};
}
constexpr endpoint_reply::operator packed::endpoint_reply_v1() const noexcept {
  return {.status = details::to_le7(status),
          .data_length = details::to_le7(static_cast<b14>(information.size())),
          .data = {std::byte{0U}}};
}

//*  _              _ _    _      _         __  __ _   _ ___ ___   *
//* (_)_ ___ ____ _| (_)__| |__ _| |_ ___  |  \/  | | | |_ _|   \  *
//* | | ' \ V / _` | | / _` / _` |  _/ -_) | |\/| | |_| || || |) | *
//* |_|_||_\_/\__,_|_|_\__,_\__,_|\__\___| |_|  |_|\___/|___|___/  *
//*                                                                *
namespace packed {

/// \brief Version 1 of the MIDI-CI Invalidate MUID message.
struct invalidate_muid_v1 {
  /// \brief Target MUID: the MUID to be invalidated (LSB first).
  byte_array<4> target_muid;
};
static_assert(offsetof(invalidate_muid_v1, target_muid) == 0);
static_assert(sizeof(invalidate_muid_v1) == 4);
static_assert(alignof(invalidate_muid_v1) == 1);
static_assert(std::is_trivially_copyable_v<invalidate_muid_v1>);

}  // end namespace packed

/// \brief Invalidate MUID Message (M2-101 section 5.9).
struct invalidate_muid {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Invalidate MUID message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr invalidate_muid make(packed::invalidate_muid_v1 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Invalidate MUID message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::invalidate_muid_v1() const noexcept;
  /// \brief Compares two invalidate_muid values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(invalidate_muid const&, invalidate_muid const&) noexcept = default;

  /// \brief Target MUID: the MUID to be invalidated.
  muid target_muid;
};

constexpr invalidate_muid invalidate_muid::make(packed::invalidate_muid_v1 const& other) noexcept {
  return {.target_muid = details::from_le7(other.target_muid)};
}
constexpr invalidate_muid::operator packed::invalidate_muid_v1() const noexcept {
  return {.target_muid = details::to_le7(target_muid)};
}

//*          _    *
//*  __ _ __| |__ *
//* / _` / _| / / *
//* \__,_\__|_\_\ *
//*               *
namespace packed {

/// \brief Version 1 of the CI Ack message
struct ack_v1 {
  std::byte original_id;         ///< Original Transaction Sub-ID#2 Classification
  std::byte status_code;         ///< ACK Status Code
  std::byte status_data;         ///< ACK Status Data
  byte_array<5> details;         ///< ACK details for each SubID Classification
  byte_array<2> message_length;  ///< Message Length (LSB first)
  b7 message[1];                 ///< Message text (array of size given by message_length)
};
static_assert(offsetof(ack_v1, original_id) == 0);
static_assert(offsetof(ack_v1, status_code) == 1);
static_assert(offsetof(ack_v1, status_data) == 2);
static_assert(offsetof(ack_v1, details) == 3);
static_assert(offsetof(ack_v1, message_length) == 8);
static_assert(offsetof(ack_v1, message) == 10);
static_assert(sizeof(ack_v1) == 11);
static_assert(alignof(ack_v1) == 1);

}  // end namespace packed

/// \brief MIDI-CI ACK Message (M2-101 section 5.10).
///
/// The MIDI-CI ACK Message is a message for dealing with positive acknowledgement of an action, or
/// to provide a notice of ongoing activity, such as timeout wait messages.
struct ack {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 ACK message representation.
  /// \note The resulting object refers to the message text in \p v1, which must remain valid for as long as the object
  /// is in use.
  /// \param v1  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr ack make(packed::ack_v1 const& v1) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 ACK message representation.
  /// \note Only the fixed-size part of the message is produced. The message text must be written separately,
  /// immediately after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::ack_v1() const noexcept;
  /// \brief Compares two ack values for equality.
  /// \param lhs  The first value to be compared.
  /// \param rhs  The second value to be compared.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(ack const& lhs, ack const& rhs) noexcept {
    return lhs.original_id == rhs.original_id && lhs.status_code == rhs.status_code &&
           lhs.status_data == rhs.status_data && lhs.details == rhs.details &&
           std::ranges::equal(lhs.message, rhs.message);
  }

  /// Original Transaction Sub-ID#2 Classification
  b7 original_id;
  /// ACK Status Code
  b7 status_code;
  /// ACK Status Data
  b7 status_data;
  /// ACK details for each SubID Classification
  b7_array<5> details{};
  /// Message text
  std::span<b7 const> message;
};

constexpr ack ack::make(packed::ack_v1 const& v1) noexcept {
  return {.original_id = details::from_le7(v1.original_id),
          .status_code = details::from_le7(v1.status_code),
          .status_data = details::from_le7(v1.status_data),
          .details = details::from_le7(v1.details),
          .message = std::span<b7 const>{std::begin(v1.message), details::from_le7(v1.message_length).get()}};
}
constexpr ack::operator packed::ack_v1() const noexcept {
  return {.original_id = details::to_le7(original_id),
          .status_code = details::to_le7(status_code),
          .status_data = details::to_le7(status_data),
          .details = details::to_le7(details),
          .message_length = details::to_le7(static_cast<b14>(message.size())),
          .message{b7{}}};
}

//*            _    *
//*  _ _  __ _| |__ *
//* | ' \/ _` | / / *
//* |_||_\__,_|_\_\ *
//*                 *
namespace packed {

/// \brief Version 1 of the CI Nak message
struct nak_v1 {};
/// \brief Version 2 of the CI Nak message
struct nak_v2 {
  std::byte original_id;         ///< Original transaction sub-ID#2 classification
  std::byte status_code;         ///< NAK Status Code
  std::byte status_data;         ///< NAK Status Data
  byte_array<5> details;         ///< NAK details for each SubID Classification
  byte_array<2> message_length;  ///< Message Length (LSB first)
  b7 message[1];                 ///< Message text (length given by message_length)
};
static_assert(offsetof(nak_v2, original_id) == 0);
static_assert(offsetof(nak_v2, status_code) == 1);
static_assert(offsetof(nak_v2, status_data) == 2);
static_assert(offsetof(nak_v2, details) == 3);
static_assert(offsetof(nak_v2, message_length) == 8);
static_assert(offsetof(nak_v2, message) == 10);
static_assert(sizeof(nak_v2) == 11);
static_assert(alignof(nak_v2) == 1);
static_assert(std::is_trivially_copyable_v<nak_v2>);

static_assert(sizeof(nak_v1) <= sizeof(nak_v2));

}  // end namespace packed

/// \brief MIDI-CI NAK Message (M2-101 section 5.11).
///
/// Version 1 of the message has no body. Version 2 added the fields below.
struct nak {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 NAK message. Version 1 messages have no body,
  /// so all of the fields are zero.
  /// \returns The new instance.
  [[nodiscard]] static constexpr nak make(packed::nak_v1 const&) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 NAK message representation.
  /// \note The resulting object refers to the message text in \p v2, which must remain valid for as long as the object
  /// is in use.
  /// \param v2  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr nak make(packed::nak_v2 const& v2) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 NAK message (which has no body) representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::nak_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 NAK message representation.
  /// \note Only the fixed-size part of the message is produced. The message text must be written separately,
  /// immediately after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::nak_v2() const noexcept;
  /// \brief Compares two nak values for equality.
  /// \param lhs  The first value to be compared.
  /// \param rhs  The second value to be compared.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(nak const& lhs, nak const& rhs) noexcept {
    return lhs.original_id == rhs.original_id && lhs.status_code == rhs.status_code &&
           lhs.status_data == rhs.status_data && lhs.details == rhs.details &&
           std::ranges::equal(lhs.message, rhs.message);
  }

  b7 original_id;         ///< Original transaction sub-ID#2 classification
  b7 status_code;         ///< NAK Status Code
  b7 status_data;         ///< NAK Status Data
  b7_array<5> details{};  ///< NAK details for each SubID Classification
  /// \brief Message text.
  std::span<b7 const> message;
};

constexpr nak nak::make(packed::nak_v1 const&) noexcept {
  return {};
}
constexpr nak nak::make(packed::nak_v2 const& v2) noexcept {
  return nak{.original_id = b7{to_underlying(v2.original_id)},
             .status_code = b7{to_underlying(v2.status_code)},
             .status_data = b7{to_underlying(v2.status_data)},
             .details = details::from_le7(v2.details),
             .message = std::span<b7 const>{std::begin(v2.message), details::from_le7(v2.message_length).get()}};
}
constexpr nak::operator packed::nak_v1() const noexcept {
  return {};
}
constexpr nak::operator packed::nak_v2() const noexcept {
  return {.original_id = details::to_le7(original_id),
          .status_code = details::to_le7(status_code),
          .status_data = details::to_le7(status_data),
          .details = details::to_le7(details),
          .message_length = details::to_le7(static_cast<b14>(message.size())),
          .message = {b7{0U}}};
}

/// \brief Types for MIDI CI Profile Configuration Messages.
namespace profile_configuration {

/// \brief A five byte Profile ID.
using profile = b7_array<5>;

//*                __ _ _       _                _           *
//*  _ __ _ _ ___ / _(_) |___  (_)_ _  __ _ _  _(_)_ _ _  _  *
//* | '_ \ '_/ _ \  _| | / -_) | | ' \/ _` | || | | '_| || | *
//* | .__/_| \___/_| |_|_\___| |_|_||_\__, |\_,_|_|_|  \_, | *
//* |_|                                  |_|           |__/  *
/// \brief Profile Inquiry message (M2-101 section 7.2). This message has no body.
struct inquiry {};

//*                __ _ _       _                _                         _       *
//*  _ __ _ _ ___ / _(_) |___  (_)_ _  __ _ _  _(_)_ _ _  _   _ _ ___ _ __| |_  _  *
//* | '_ \ '_/ _ \  _| | / -_) | | ' \/ _` | || | | '_| || | | '_/ -_) '_ \ | || | *
//* | .__/_| \___/_| |_|_\___| |_|_||_\__, |\_,_|_|_|  \_, | |_| \___| .__/_|\_, | *
//* |_|                                  |_|           |__/          |_|     |__/  *

namespace packed {

/// \brief Part 1 of version 1 of the CI Inquiry Reply message
struct inquiry_reply_v1_pt1 {
  byte_array<2> num_enabled{};  ///< Number of currently enabled profiles
  profile ids[1];               ///< Profile ID of currently enabled profiles (array length given by num_enabled)
};

static_assert(offsetof(inquiry_reply_v1_pt1, num_enabled) == 0);
static_assert(offsetof(inquiry_reply_v1_pt1, ids) == 2);
static_assert(sizeof(inquiry_reply_v1_pt1) == 7);
static_assert(alignof(inquiry_reply_v1_pt1) == 1);
static_assert(std::is_trivially_copyable_v<inquiry_reply_v1_pt1>);

/// \brief Part 2 of version 1 of the CI Inquiry Reply message
struct inquiry_reply_v1_pt2 {
  byte_array<2> num_disabled{};  ///< Number of currently disabled profiles
  profile ids[1];                ///< Profile ID of currently disabled profiles (array length given by num_disabled)
};

static_assert(offsetof(inquiry_reply_v1_pt2, num_disabled) == 0);
static_assert(offsetof(inquiry_reply_v1_pt2, ids) == 2);
static_assert(sizeof(inquiry_reply_v1_pt2) == 7);
static_assert(alignof(inquiry_reply_v1_pt2) == 1);
static_assert(std::is_trivially_copyable_v<inquiry_reply_v1_pt2>);

}  // end namespace packed

/// \brief Reply to Profile Inquiry message (M2-101 section 7.3).
struct inquiry_reply {
  /// \brief Creates an instance from the two parts of the packed (on-the-wire) version 1 Reply to Profile Inquiry
  /// message.
  /// \param v1_pt1  The packed first part of the message: the list of enabled profiles.
  /// \param v1_pt2  The packed second part of the message: the list of disabled profiles.
  /// \note The resulting object refers to the profile IDs in \p v1_pt1 and \p v1_pt2, which must remain valid for as
  /// long as the object is in use.
  /// \returns The new instance.
  [[nodiscard]] static constexpr inquiry_reply make(packed::inquiry_reply_v1_pt1 const& v1_pt1,
                                                    packed::inquiry_reply_v1_pt2 const& v1_pt2) noexcept;
  /// \brief Converts to the packed (on-the-wire) first part of the version 1 Reply to Profile Inquiry message
  /// representation.
  /// \note Only the fixed-size part of the message is produced. The enabled profile IDs must be written separately,
  /// immediately after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::inquiry_reply_v1_pt1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) second part of the version 1 Reply to Profile Inquiry message
  /// representation.
  /// \note Only the fixed-size part of the message is produced. The disabled profile IDs must be written separately,
  /// immediately after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::inquiry_reply_v1_pt2() const noexcept;
  /// \brief Compares two inquiry_reply values for equality.
  /// \param lhs  The first value to be compared.
  /// \param rhs  The second value to be compared.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(inquiry_reply const& lhs, inquiry_reply const& rhs) noexcept {
    return std::ranges::equal(lhs.enabled, rhs.enabled) && std::ranges::equal(lhs.disabled, rhs.disabled);
  }

  /// \brief The IDs of the currently enabled profiles.
  std::span<profile const> enabled;
  /// \brief The IDs of the currently disabled profiles.
  std::span<profile const> disabled;
};

constexpr inquiry_reply inquiry_reply::make(packed::inquiry_reply_v1_pt1 const& v1_pt1,
                                            packed::inquiry_reply_v1_pt2 const& v1_pt2) noexcept {
  return {
      .enabled = std::span<profile const>{std::begin(v1_pt1.ids), details::from_le7(v1_pt1.num_enabled).get()},
      .disabled = std::span<profile const>{std::begin(v1_pt2.ids), details::from_le7(v1_pt2.num_disabled).get()},
  };
}

constexpr inquiry_reply::operator packed::inquiry_reply_v1_pt1() const noexcept {
  return {.num_enabled = details::to_le7(static_cast<b14>(enabled.size())), .ids = {profile{}}};
}
constexpr inquiry_reply::operator packed::inquiry_reply_v1_pt2() const noexcept {
  return {.num_disabled = details::to_le7(static_cast<b14>(disabled.size())), .ids = {profile{}}};
}

//*                __ _ _               _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   __ _ __| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / _` / _` / _` / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \__,_\__,_\__,_\___\__,_| *
//* |_|                                                  *
namespace packed {

/// \brief Version 1 of the CI Profile Added message
struct added_v1 {
  profile pid{};  ///< Profile ID of profile being added
};
static_assert(offsetof(added_v1, pid) == 0);
static_assert(sizeof(added_v1) == 5);
static_assert(alignof(added_v1) == 1);
static_assert(std::is_trivially_copyable_v<added_v1>);

}  // end namespace packed

/// \brief Profile Added Report message (M2-101 section 7.4).
struct added {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Profile Added Report message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr added make(packed::added_v1 const& other) noexcept { return {.pid = other.pid}; }
  /// \brief Converts to the packed (on-the-wire) version 1 Profile Added Report message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::added_v1() const noexcept { return {.pid = pid}; }
  /// \brief Compares two added values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(added const&, added const&) noexcept = default;

  profile pid{};  ///< Profile ID of profile being added
};

//*                __ _ _                                    _  *
//*  _ __ _ _ ___ / _(_) |___   _ _ ___ _ __  _____ _____ __| | *
//* | '_ \ '_/ _ \  _| | / -_) | '_/ -_) '  \/ _ \ V / -_) _` | *
//* | .__/_| \___/_| |_|_\___| |_| \___|_|_|_\___/\_/\___\__,_| *
//* |_|                                                         *
namespace packed {

/// \brief Version 1 of the CI Profile Removed message
struct removed_v1 {
  profile pid{};  ///< Profile ID of profile being removed
};
static_assert(offsetof(removed_v1, pid) == 0);
static_assert(sizeof(removed_v1) == 5);
static_assert(alignof(removed_v1) == 1);
static_assert(std::is_trivially_copyable_v<removed_v1>);

}  // end namespace packed

/// \brief Profile Removed Report message (M2-101 section 7.5).
struct removed {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Profile Removed Report message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr removed make(packed::removed_v1 const& other) noexcept { return {.pid = other.pid}; }
  /// \brief Converts to the packed (on-the-wire) version 1 Profile Removed Report message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::removed_v1() const noexcept { return {.pid = pid}; }
  /// \brief Compares two removed values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(removed const&, removed const&) noexcept = default;

  profile pid{};  ///< Profile ID of profile being removed
};

//*                __      _     _        _ _      _                _           *
//*  _ __ _ _ ___ / _|  __| |___| |_ __ _(_) |___ (_)_ _  __ _ _  _(_)_ _ _  _  *
//* | '_ \ '_/ _ \  _| / _` / -_)  _/ _` | | (_-< | | ' \/ _` | || | | '_| || | *
//* | .__/_| \___/_|   \__,_\___|\__\__,_|_|_/__/ |_|_||_\__, |\_,_|_|_|  \_, | *
//* |_|                                                     |_|           |__/  *
namespace packed {

/// \brief Version 1 of the CI Profile Details Inquiry message
struct details_v1 {
  profile pid{};  ///< Profile ID of profile
  /// \brief Inquiry Target: the type of data being requested.
  std::byte target{};
};
static_assert(offsetof(details_v1, pid) == 0);
static_assert(offsetof(details_v1, target) == 5);
static_assert(sizeof(details_v1) == 6);
static_assert(alignof(details_v1) == 1);
static_assert(std::is_trivially_copyable_v<details_v1>);

}  // end namespace packed

/// \brief Profile Details Inquiry message (M2-101 section 7.6).
struct details {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Profile Details Inquiry message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr details make(packed::details_v1 const& other) noexcept {
    return {.pid = other.pid, .target = b7{to_underlying(other.target)}};
  }
  /// \brief Converts to the packed (on-the-wire) version 1 Profile Details Inquiry message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::details_v1() const noexcept {
    return {.pid = pid, .target = ci::details::to_le7(target)};
  }
  /// \brief Compares two details values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(details const&, details const&) noexcept = default;

  /// \brief Profile ID of the profile.
  profile pid{};
  /// \brief Inquiry Target: the type of data being requested. Values 0x00-0x3F are registered target data; 0x40-0x7F
  /// are profile specific.
  b7 target;
};

namespace packed {

/// \brief Version 1 of the CI Profile Details Reply message
struct details_reply_v1 {
  profile pid{};                ///< Profile ID of profile
  std::byte target{};           ///< Inquiry target
  byte_array<2> data_length{};  ///< Inquiry target data length (LSB first)
  b7 data[1]{};                 ///< Array length given by data_length
};

static_assert(offsetof(details_reply_v1, pid) == 0);
static_assert(offsetof(details_reply_v1, target) == 5);
static_assert(offsetof(details_reply_v1, data_length) == 6);
static_assert(offsetof(details_reply_v1, data) == 8);
static_assert(sizeof(details_reply_v1) == 9);
static_assert(alignof(details_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<details_reply_v1>);

}  // end namespace packed

/// \brief Reply to Profile Details Inquiry message (M2-101 section 7.7).
struct details_reply {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Reply to Profile Details Inquiry message
  /// representation.
  /// \note The resulting object refers to the target data in \p other, which must remain valid for as long as the
  /// object is in use.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr details_reply make(packed::details_reply_v1 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Reply to Profile Details Inquiry message representation.
  /// \note Only the fixed-size part of the message is produced. The inquiry target data must be written separately,
  /// immediately after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::details_reply_v1() const noexcept;
  /// \brief Compares two details_reply values for equality.
  /// \param lhs  The first value to be compared.
  /// \param rhs  The second value to be compared.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(details_reply const& lhs, details_reply const& rhs) noexcept {
    return lhs.pid == rhs.pid && lhs.target == rhs.target && std::ranges::equal(lhs.data, rhs.data);
  }

  profile pid{};  ///< Profile ID of profile
  b7 target{};    ///< Inquiry target
  /// \brief Inquiry Target Data.
  std::span<b7 const> data;
};

constexpr details_reply details_reply::make(packed::details_reply_v1 const& other) noexcept {
  return {.pid = other.pid,
          .target = b7{to_underlying(other.target)},
          .data = std::span<b7 const>{std::begin(other.data), ci::details::from_le7(other.data_length).get()}};
}
constexpr details_reply::operator packed::details_reply_v1() const noexcept {
  return {
      .pid = pid,
      .target = static_cast<std::byte>(target.get()),
      .data_length =
          ci::details::to_le7(static_cast<b14>(data.size_bytes())),  // Inquiry target data length (LSB first)
      .data = {b7{0U}},
  };
}

//*                __ _ _                 *
//*  _ __ _ _ ___ / _(_) |___   ___ _ _   *
//* | '_ \ '_/ _ \  _| | / -_) / _ \ ' \  *
//* | .__/_| \___/_| |_|_\___| \___/_||_| *
//* |_|                                   *
namespace packed {

/// \brief Version 1 of the CI Profile On message
struct on_v1 {
  profile pid{};  ///< Profile ID of profile to be set to on (to be enabled)
};
static_assert(offsetof(on_v1, pid) == 0);
static_assert(sizeof(on_v1) == 5);
static_assert(alignof(on_v1) == 1);
static_assert(std::is_trivially_copyable_v<on_v1>);

/// \brief Version 2 of the CI Profile On message
struct on_v2 {
  /// \brief The fields from version 1 of the message.
  on_v1 v1;
  /// Number of channels requested (LSB First) to assign to this profile when it is enabled
  byte_array<2> num_channels{};
};
static_assert(offsetof(on_v2, v1) == 0);
static_assert(offsetof(on_v2, num_channels) == 5);
static_assert(sizeof(on_v2) == 7);
static_assert(alignof(on_v2) == 1);
static_assert(std::is_trivially_copyable_v<on_v2>);

static_assert(sizeof(on_v1) <= sizeof(on_v2));

}  // end namespace packed

/// \brief Set Profile On message (M2-101 section 7.8).
struct on {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Set Profile On message representation.
  /// \param other  The packed message.
  /// \param num_channels  The number of channels. Version 1 messages do not include this field.
  /// \returns The new instance.
  [[nodiscard]] static constexpr on make(packed::on_v1 const& other, b14 num_channels = b14{}) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Set Profile On message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr on make(packed::on_v2 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Set Profile On message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::on_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Set Profile On message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::on_v2() const noexcept;
  /// \brief Compares two on values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(on const&, on const&) noexcept = default;

  /// \brief Profile ID of the profile to be set to on (to be enabled).
  profile pid{};
  /// \brief Number of channels requested to assign to this profile when it is enabled.
  b14 num_channels;
};

constexpr on on::make(packed::on_v1 const& other, b14 num_channels) noexcept {
  return {.pid = other.pid, .num_channels = num_channels};
}
constexpr on on::make(packed::on_v2 const& other) noexcept {
  return make(other.v1, ci::details::from_le7(other.num_channels));
}
constexpr on::operator packed::on_v1() const noexcept {
  return {.pid = pid};
}
constexpr on::operator packed::on_v2() const noexcept {
  return packed::on_v2{.v1 = static_cast<packed::on_v1>(*this), .num_channels = ci::details::to_le7(num_channels)};
}

//*                __ _ _            __  __  *
//*  _ __ _ _ ___ / _(_) |___   ___ / _|/ _| *
//* | '_ \ '_/ _ \  _| | / -_) / _ \  _|  _| *
//* | .__/_| \___/_| |_|_\___| \___/_| |_|   *
//* |_|                                      *
namespace packed {

/// \brief Version 1 of the CI Profile Off message
struct off_v1 {
  profile pid{};  ///< Profile ID of Profile to be Set to Off (to be disabled)
};
static_assert(offsetof(off_v1, pid) == 0);
static_assert(sizeof(off_v1) == 5);
static_assert(alignof(off_v1) == 1);
static_assert(std::is_trivially_copyable_v<off_v1>);

/// \brief Version 2 of the CI Profile Off message
struct off_v2 {
  /// \brief The fields from version 1 of the message.
  off_v1 v1;
  /// \brief Reserved.
  byte_array<2> reserved{};
};
static_assert(offsetof(off_v2, v1) == 0);
static_assert(offsetof(off_v2, reserved) == 5);
static_assert(sizeof(off_v2) == 7);
static_assert(alignof(off_v2) == 1);
static_assert(std::is_trivially_copyable_v<off_v2>);

static_assert(sizeof(off_v1) <= sizeof(off_v2));

}  // end namespace packed

/// \brief Set Profile Off message (M2-101 section 7.9).
struct off {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Set Profile Off message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr off make(packed::off_v1 const& other) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Set Profile Off message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr off make(packed::off_v2 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Set Profile Off message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::off_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Set Profile Off message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::off_v2() const noexcept;
  /// \brief Compares two off values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(off const&, off const&) noexcept = default;

  /// \brief Profile ID of the profile to be set to off (to be disabled).
  profile pid{};
  // There's a 14 bit field in the specification that's "reserved"
};

constexpr off off::make(packed::off_v1 const& other) noexcept {
  return {.pid = other.pid};
}
constexpr off off::make(packed::off_v2 const& other) noexcept {
  return make(static_cast<packed::off_v1>(other.v1));
}
constexpr off::operator packed::off_v1() const noexcept {
  return {.pid = pid};
}
constexpr off::operator packed::off_v2() const noexcept {
  return {.v1 = static_cast<packed::off_v1>(*this), .reserved = byte_array<2>{}};
}

//*                __ _ _                     _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   ___ _ _  __ _| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / -_) ' \/ _` | '_ \ / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \___|_||_\__,_|_.__/_\___\__,_| *
//* |_|                                                        *
namespace packed {

/// \brief Version 1 of the CI Profile Enabled message
struct enabled_v1 {
  profile pid{};  ///< Profile ID of Profile that is now enabled
};
static_assert(offsetof(enabled_v1, pid) == 0);
static_assert(sizeof(enabled_v1) == 5);
static_assert(alignof(enabled_v1) == 1);
static_assert(std::is_trivially_copyable_v<enabled_v1>);

/// \brief Version 2 of the CI Profile Enabled message
struct enabled_v2 {
  /// \brief The fields from version 1 of the message.
  enabled_v1 v1;
  /// \brief Number of channels enabled (LSB first).
  byte_array<2> num_channels{};
};
static_assert(offsetof(enabled_v2, v1) == 0);
static_assert(offsetof(enabled_v2, num_channels) == 5);
static_assert(sizeof(enabled_v2) == 7);
static_assert(alignof(enabled_v2) == 1);
static_assert(std::is_trivially_copyable_v<enabled_v2>);

static_assert(sizeof(enabled_v1) <= sizeof(enabled_v2));

}  // end namespace packed

/// \brief Profile Enabled Report message (M2-101 section 7.10).
struct enabled {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Profile Enabled Report message representation.
  /// \param other  The packed message.
  /// \param num_channels  The number of channels. Version 1 messages do not include this field.
  /// \returns The new instance.
  [[nodiscard]] static constexpr enabled make(packed::enabled_v1 const& other, b14 num_channels = b14{}) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Profile Enabled Report message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr enabled make(packed::enabled_v2 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Profile Enabled Report message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::enabled_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Profile Enabled Report message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::enabled_v2() const noexcept;
  /// \brief Compares two enabled values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(enabled const&, enabled const&) noexcept = default;

  /// \brief Profile ID of the profile that is now enabled.
  profile pid{};
  /// \brief Number of channels enabled.
  b14 num_channels;
};

constexpr enabled enabled::make(packed::enabled_v1 const& other, b14 num_channels) noexcept {
  return {.pid = other.pid, .num_channels = num_channels};
}
constexpr enabled enabled::make(packed::enabled_v2 const& other) noexcept {
  return make(other.v1, ci::details::from_le7(other.num_channels));
}
constexpr enabled::operator packed::enabled_v1() const noexcept {
  return {.pid = pid};
}
constexpr enabled::operator packed::enabled_v2() const noexcept {
  return {
      .v1 = static_cast<packed::enabled_v1>(*this),
      .num_channels = ci::details::to_le7(num_channels),
  };
}

//*                __ _ _          _ _          _    _        _  *
//*  _ __ _ _ ___ / _(_) |___   __| (_)___ __ _| |__| |___ __| | *
//* | '_ \ '_/ _ \  _| | / -_) / _` | (_-</ _` | '_ \ / -_) _` | *
//* | .__/_| \___/_| |_|_\___| \__,_|_/__/\__,_|_.__/_\___\__,_| *
//* |_|                                                          *
namespace packed {

/// \brief Version 1 of the CI Profile Disabled message
struct disabled_v1 {
  profile pid;  ///< Profile ID of Profile that is now disabled
};
static_assert(offsetof(disabled_v1, pid) == 0);
static_assert(sizeof(disabled_v1) == 5);
static_assert(alignof(disabled_v1) == 1);
static_assert(std::is_trivially_copyable_v<disabled_v1>);

/// \brief Version 2 of the CI Profile Disabled message
struct disabled_v2 {
  /// \brief The fields from version 1 of the message.
  disabled_v1 v1;
  /// \brief Number of channels disabled (LSB first).
  byte_array<2> num_channels{};
};
static_assert(offsetof(disabled_v2, v1) == 0);
static_assert(offsetof(disabled_v2, num_channels) == 5);
static_assert(sizeof(disabled_v2) == 7);
static_assert(alignof(disabled_v2) == 1);
static_assert(std::is_trivially_copyable_v<disabled_v2>);

static_assert(sizeof(disabled_v1) <= sizeof(disabled_v2));

}  // end namespace packed

/// \brief Profile Disabled Report message (M2-101 section 7.11).
struct disabled {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Profile Disabled Report message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr disabled make(packed::disabled_v1 const& other) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Profile Disabled Report message representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr disabled make(packed::disabled_v2 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Profile Disabled Report message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::disabled_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Profile Disabled Report message representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::disabled_v2() const noexcept;

  /// \brief Compares two disabled values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(disabled const&, disabled const&) noexcept = default;

  /// \brief Profile ID of the profile that is now disabled.
  profile pid{};
  /// \brief Number of channels disabled.
  b14 num_channels;
};

constexpr disabled disabled::make(packed::disabled_v1 const& other) noexcept {
  return {.pid = other.pid, .num_channels = b14{}};
}
constexpr disabled disabled::make(packed::disabled_v2 const& other) noexcept {
  return {.pid = other.v1.pid, .num_channels = ci::details::from_le7(other.num_channels)};
}
constexpr disabled::operator packed::disabled_v1() const noexcept {
  return {.pid = pid};
}
constexpr disabled::operator packed::disabled_v2() const noexcept {
  return {.v1 = static_cast<packed::disabled_v1>(*this), .num_channels = ci::details::to_le7(num_channels)};
}

//*                __ _ _                       _  __ _         _      _         *
//*  _ __ _ _ ___ / _(_) |___   ____ __  ___ __(_)/ _(_)__   __| |__ _| |_ __ _  *
//* | '_ \ '_/ _ \  _| | / -_) (_-< '_ \/ -_) _| |  _| / _| / _` / _` |  _/ _` | *
//* | .__/_| \___/_| |_|_\___| /__/ .__/\___\__|_|_| |_\__| \__,_\__,_|\__\__,_| *
//* |_|                           |_|                                            *
namespace packed {

/// \brief Version 1 of the CI Profile Specific Data message
struct specific_data_v1 {
  profile pid;                ///< Profile ID
  byte_array<2> data_length;  ///< Length of following profile specific data (LSB first)
  b7 data[1];                 ///< Profile specific data (array length given by data_length)
};
static_assert(offsetof(specific_data_v1, pid) == 0);
static_assert(offsetof(specific_data_v1, data_length) == 5);
static_assert(offsetof(specific_data_v1, data) == 7);
static_assert(sizeof(specific_data_v1) == 8);
static_assert(alignof(specific_data_v1) == 1);
static_assert(std::is_trivially_copyable_v<specific_data_v1>);

}  // end namespace packed

/// \brief Profile Specific Data message (M2-101 section 7.12).
struct specific_data {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Profile Specific Data message representation.
  /// \note The resulting object refers to the profile specific data in \p other, which must remain valid for as long as
  /// the object is in use.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr specific_data make(packed::specific_data_v1 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Profile Specific Data message representation.
  /// \note Only the fixed-size part of the message is produced. The profile specific data must be written separately,
  /// immediately after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::specific_data_v1() const noexcept;
  /// \brief Compares two specific_data values for equality.
  /// \param lhs  The first value to be compared.
  /// \param rhs  The second value to be compared.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(specific_data const& lhs, specific_data const& rhs) noexcept {
    return lhs.pid == rhs.pid && std::ranges::equal(lhs.data, rhs.data);
  }

  profile pid{};             ///< Profile ID
  std::span<b7 const> data;  ///< Profile specific data
};

constexpr specific_data specific_data::make(packed::specific_data_v1 const& other) noexcept {
  return {
      .pid = other.pid,
      .data = std::span<b7 const>{std::begin(other.data), ci::details::from_le7(other.data_length).get()},
  };
}
constexpr specific_data::operator packed::specific_data_v1() const noexcept {
  return {
      .pid = pid,
      .data_length = ci::details::to_le7(static_cast<b14>(data.size_bytes())),
      .data = {b7{0U}},
  };
}

}  // namespace profile_configuration

/// \brief Types for MIDI-CI Property Exchange Messages.
namespace property_exchange {

//*                                 _    _ _ _ _   _         *
//*  _ __  ___   __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___ *
//* | '_ \/ -_) / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< *
//* | .__/\___| \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ *
//* |_|                 |_|                                  *
namespace packed {

/// \brief Version 1 of the CI PE Capabilities message
struct capabilities_v1 {
  /// \brief Number of Simultaneous Property Exchange Requests Supported.
  std::byte num_simultaneous;
};
static_assert(offsetof(capabilities_v1, num_simultaneous) == 0);
static_assert(sizeof(capabilities_v1) == 1);
static_assert(alignof(capabilities_v1) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_v1>);

/// \brief Version 2 of the CI PE Capabilities message
struct capabilities_v2 {
  /// \brief The fields from version 1 of the message.
  capabilities_v1 v1;
  /// \brief Property Exchange Major Version.
  std::byte major_version;
  /// \brief Property Exchange Minor Version.
  std::byte minor_version;
};
static_assert(offsetof(capabilities_v2, v1) == 0);
static_assert(offsetof(capabilities_v2, major_version) == 1);
static_assert(offsetof(capabilities_v2, minor_version) == 2);
static_assert(sizeof(capabilities_v2) == 3);
static_assert(alignof(capabilities_v2) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_v2>);

}  // end namespace packed

/// \brief Inquiry: Property Exchange Capabilities message (M2-101 section 8.5).
struct capabilities {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Inquiry: Property Exchange Capabilities message
  /// representation.
  /// Version 1 messages do not include the version fields, so these are set to zero.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr capabilities make(packed::capabilities_v1 const& other) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Inquiry: Property Exchange Capabilities message
  /// representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr capabilities make(packed::capabilities_v2 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Inquiry: Property Exchange Capabilities message
  /// representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::capabilities_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Inquiry: Property Exchange Capabilities message
  /// representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::capabilities_v2() const noexcept;
  /// \brief Compares two capabilities values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(capabilities const&, capabilities const&) noexcept = default;

  /// \brief Number of Simultaneous Property Exchange Requests Supported.
  b7 num_simultaneous;
  /// \brief Property Exchange Major Version.
  b7 major_version;
  /// \brief Property Exchange Minor Version.
  b7 minor_version;
};

constexpr capabilities capabilities::make(packed::capabilities_v1 const& other) noexcept {
  return {
      .num_simultaneous = b7{to_underlying(other.num_simultaneous)},
      .major_version = b7{},
      .minor_version = b7{},
  };
}
constexpr capabilities capabilities::make(packed::capabilities_v2 const& other) noexcept {
  return {
      .num_simultaneous = b7{to_underlying(other.v1.num_simultaneous)},
      .major_version = b7{to_underlying(other.major_version)},
      .minor_version = b7{to_underlying(other.minor_version)},
  };
}
constexpr capabilities::operator packed::capabilities_v1() const noexcept {
  return {.num_simultaneous = static_cast<std::byte>(num_simultaneous.get())};
}
constexpr capabilities::operator packed::capabilities_v2() const noexcept {
  return {
      .v1 = static_cast<packed::capabilities_v1>(*this),
      .major_version = static_cast<std::byte>(major_version.get()),
      .minor_version = static_cast<std::byte>(minor_version.get()),
  };
}

//*                                 _    _ _ _ _   _                       _       *
//*  _ __  ___   __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___  _ _ ___ _ __| |_  _  *
//* | '_ \/ -_) / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< | '_/ -_) '_ \ | || | *
//* | .__/\___| \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ |_| \___| .__/_|\_, | *
//* |_|                 |_|                                          |_|     |__/  *
namespace packed {

/// \brief Version 1 of the CI PE Capabilities Reply message
struct capabilities_reply_v1 {
  /// \brief Number of Simultaneous Property Exchange Requests Supported.
  std::byte num_simultaneous;
};
static_assert(offsetof(capabilities_reply_v1, num_simultaneous) == 0);
static_assert(sizeof(capabilities_reply_v1) == 1);
static_assert(alignof(capabilities_reply_v1) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_reply_v1>);

/// \brief Version 2 of the CI PE Capabilities Reply message
struct capabilities_reply_v2 {
  /// \brief The fields from version 1 of the message.
  capabilities_reply_v1 v1;
  /// \brief Property Exchange Major Version.
  std::byte major_version;
  /// \brief Property Exchange Minor Version.
  std::byte minor_version;
};
static_assert(offsetof(capabilities_reply_v2, v1) == 0);
static_assert(offsetof(capabilities_reply_v2, major_version) == 1);
static_assert(offsetof(capabilities_reply_v2, minor_version) == 2);
static_assert(sizeof(capabilities_reply_v2) == 3);
static_assert(alignof(capabilities_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_reply_v2>);

}  // end namespace packed

/// \brief Reply to Property Exchange Capabilities message (M2-101 section 8.6).
struct capabilities_reply {
  /// \brief Creates an instance from the packed (on-the-wire) version 1 Reply to Property Exchange Capabilities message
  /// representation.
  /// Version 1 messages do not include the version fields, so these are set to zero.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr capabilities_reply make(packed::capabilities_reply_v1 const& other) noexcept;
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Reply to Property Exchange Capabilities message
  /// representation.
  /// \param other  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr capabilities_reply make(packed::capabilities_reply_v2 const& other) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 1 Reply to Property Exchange Capabilities message
  /// representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::capabilities_reply_v1() const noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Reply to Property Exchange Capabilities message
  /// representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::capabilities_reply_v2() const noexcept;
  /// \brief Compares two capabilities_reply values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(capabilities_reply const&, capabilities_reply const&) noexcept = default;

  /// \brief Number of Simultaneous Property Exchange Requests Supported.
  b7 num_simultaneous;
  /// \brief Property Exchange Major Version.
  b7 major_version;
  /// \brief Property Exchange Minor Version.
  b7 minor_version;
};

constexpr capabilities_reply capabilities_reply::make(packed::capabilities_reply_v1 const& other) noexcept {
  return {
      .num_simultaneous = b7{to_underlying(other.num_simultaneous)},
      .major_version = b7{},
      .minor_version = b7{},
  };
}
constexpr capabilities_reply capabilities_reply::make(packed::capabilities_reply_v2 const& other) noexcept {
  return {
      .num_simultaneous = b7{to_underlying(other.v1.num_simultaneous)},
      .major_version = b7{to_underlying(other.major_version)},
      .minor_version = b7{to_underlying(other.minor_version)},
  };
}
constexpr capabilities_reply::operator packed::capabilities_reply_v1() const noexcept {
  return {
      .num_simultaneous = static_cast<std::byte>(num_simultaneous.get()),
  };
}
constexpr capabilities_reply::operator packed::capabilities_reply_v2() const noexcept {
  return {
      .v1 = static_cast<packed::capabilities_reply_v1>(*this),
      .major_version = static_cast<std::byte>(major_version.get()),
      .minor_version = static_cast<std::byte>(minor_version.get()),
  };
}

//*                             _                     _                        *
//*  _ __ _ _ ___ _ __  ___ _ _| |_ _  _   _____ ____| |_  __ _ _ _  __ _ ___  *
//* | '_ \ '_/ _ \ '_ \/ -_) '_|  _| || | / -_) \ / _| ' \/ _` | ' \/ _` / -_) *
//* | .__/_| \___/ .__/\___|_|  \__|\_, | \___/_\_\__|_||_\__,_|_||_\__, \___| *
//* |_|          |_|                |__/                            |___/      *
namespace packed {

/// \brief Part 1 of the CI Property Exchange message: the request ID and header data.
///
/// The structure is followed by the remainder of the header data.
struct property_exchange_pt1 {
  /// \brief Request ID.
  std::byte request_id;
  /// \brief Length of the following header data (LSB first).
  byte_array<2> header_length;
  /// \brief Header data (array length given by header_length).
  char header[1];
};
static_assert(offsetof(property_exchange_pt1, request_id) == 0);
static_assert(offsetof(property_exchange_pt1, header_length) == 1);
static_assert(offsetof(property_exchange_pt1, header) == 3);
static_assert(alignof(property_exchange_pt1) == 1);
static_assert(sizeof(property_exchange_pt1) == 4);
static_assert(std::is_trivially_copyable_v<property_exchange_pt1>);

/// \brief Part 2 of the CI Property Exchange message: the chunk information and property data.
///
/// The structure is followed by the remainder of the property data.
struct property_exchange_pt2 {
  /// \brief Number of chunks in the data set (LSB first). Zero indicates that the number is unknown.
  byte_array<2> number_of_chunks;
  /// \brief Number of this chunk (LSB first). Chunks are numbered from 1.
  byte_array<2> chunk_number;
  /// \brief Length of the following property data (LSB first).
  byte_array<2> data_length;
  /// \brief Property data (array length given by data_length).
  char data[1];
};
static_assert(offsetof(property_exchange_pt2, number_of_chunks) == 0);
static_assert(offsetof(property_exchange_pt2, chunk_number) == 2);
static_assert(offsetof(property_exchange_pt2, data_length) == 4);
static_assert(offsetof(property_exchange_pt2, data) == 6);
static_assert(alignof(property_exchange_pt2) == 1);
static_assert(sizeof(property_exchange_pt2) == 7);
static_assert(std::is_trivially_copyable_v<property_exchange_pt2>);

}  // end namespace packed

/// \brief Identifies a chunk of a Property Exchange message (M2-101 section 8.3).
struct chunk_info {
  /// \brief Compares two chunk_info values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(chunk_info const&, chunk_info const&) noexcept = default;
  /// \brief Number of chunks in the data set. Zero indicates that the number is unknown.
  b14 number_of_chunks;
  /// \brief Number of this chunk. Chunks are numbered from 1.
  b14 chunk_number;
};

/// \brief The Property Exchange messages which share a common format.
enum class property_exchange_type {
  get,                 ///< Inquiry: Get Property Data
  get_reply,           ///< Reply to Get Property Data
  set,                 ///< Inquiry: Set Property Data
  set_reply,           ///< Reply to Set Property Data
  subscription,        ///< Subscription
  subscription_reply,  ///< Reply to Subscription
  notify               ///< Notify
};
/// \brief The fields of a Property Exchange message which carries header and property data (M2-101 sections 8.7 to
/// 8.13).
///
/// The header and data spans refer to memory which is not owned by this object.
///
/// \tparam Pet  The type of the Property Exchange message.
template <property_exchange_type Pet> class property_exchange {
public:
  /// \brief Creates a Property Exchange message.
  /// \param chunk  The chunk information.
  /// \param request  The Request ID.
  /// \param header  The header data.
  /// \param data  The property data.
  /// \returns The new instance.
  [[nodiscard]] static constexpr property_exchange make(chunk_info const& chunk, b7 const request,
                                                        std::span<char const> const header,
                                                        std::span<char const> const data = {}) noexcept {
    return {.chunk = chunk, .request = request, .header = header, .data = data};
  }
  /// \brief Converts to the packed (on-the-wire) first part (request ID and header) of the message representation.
  /// \note Only the fixed-size part of the message is produced. The header data must be written separately, immediately
  /// after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::property_exchange_pt1() const noexcept {
    return {
        .request_id = static_cast<std::byte>(request.get()),
        .header_length = details::to_le7(static_cast<b14>(header.size())),
        .header = {'\0'},
    };
  }
  /// \brief Converts to the packed (on-the-wire) second part (chunk information and property data) of the message
  /// representation.
  /// \note Only the fixed-size part of the message is produced. The property data must be written separately,
  /// immediately after the fixed-size fields.
  /// \returns The packed representation.
  explicit constexpr operator packed::property_exchange_pt2() const noexcept {
    return {
        .number_of_chunks = details::to_le7(chunk.number_of_chunks),
        .chunk_number = details::to_le7(chunk.chunk_number),
        .data_length = details::to_le7(static_cast<b14>(data.size_bytes())),
        .data = {'\0'},
    };
  }
  /// \brief Compares two property_exchange values for equality.
  /// \param lhs  The first value to be compared.
  /// \param rhs  The second value to be compared.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(property_exchange const& lhs, property_exchange const& rhs) noexcept {
    return lhs.chunk == rhs.chunk && lhs.request == rhs.request && std::ranges::equal(lhs.header, rhs.header) &&
           std::ranges::equal(lhs.data, rhs.data);
  }
  /// \brief The chunk information.
  chunk_info chunk;
  /// \brief The Request ID.
  b7 request;
  /// \brief The header data.
  std::span<char const> header;
  /// \brief The property data.
  std::span<char const> data;
};

/// \brief Inquiry: Get Property Data (M2-101 section 8.7).
using get = property_exchange<property_exchange_type::get>;
/// \brief Reply to Get Property Data (M2-101 section 8.8).
using get_reply = property_exchange<property_exchange_type::get_reply>;
/// \brief Inquiry: Set Property Data (M2-101 section 8.9).
using set = property_exchange<property_exchange_type::set>;
/// \brief Reply to Set Property Data (M2-101 section 8.10).
using set_reply = property_exchange<property_exchange_type::set_reply>;
/// \brief Subscription (M2-101 section 8.11).
using subscription = property_exchange<property_exchange_type::subscription>;
/// \brief Reply to Subscription (M2-101 section 8.12).
using subscription_reply = property_exchange<property_exchange_type::subscription_reply>;
/// \brief Notify (M2-101 section 8.13).
using notify = property_exchange<property_exchange_type::notify>;

}  // end namespace property_exchange

/// \brief Types for MIDI CI Process Inquiry messages.
namespace process_inquiry {

//*                     _    _ _ _ _   _         *
//*  __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___ *
//* / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< *
//* \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ *
//*         |_|                                  *
/// \brief Inquiry: Process Inquiry Capabilities message (M2-101 section 9.2). This message has no body.
struct capabilities {};

//*                     _    _ _ _ _   _                       _       *
//*  __ __ _ _ __  __ _| |__(_) (_) |_(_)___ ___  _ _ ___ _ __| |_  _  *
//* / _/ _` | '_ \/ _` | '_ \ | | |  _| / -_|_-< | '_/ -_) '_ \ | || | *
//* \__\__,_| .__/\__,_|_.__/_|_|_|\__|_\___/__/ |_| \___| .__/_|\_, | *
//*         |_|                                          |_|     |__/  *
namespace packed {

/// \brief Version 2 of the Reply to Process Inquiry Capabilities message.
struct capabilities_reply_v2 {
  /// \brief Process Inquiry Supported Features (bitmap).
  std::byte features;
};

static_assert(offsetof(capabilities_reply_v2, features) == 0);
static_assert(alignof(capabilities_reply_v2) == 1);
static_assert(sizeof(capabilities_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<capabilities_reply_v2>);

}  // end namespace packed

/// \brief Reply to Process Inquiry Capabilities message (M2-101 section 9.3).
struct capabilities_reply {
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Reply to Process Inquiry Capabilities message
  /// representation.
  /// \param v2  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr capabilities_reply make(packed::capabilities_reply_v2 const& v2) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Reply to Process Inquiry Capabilities message
  /// representation.
  /// \returns The packed representation.
  explicit constexpr operator packed::capabilities_reply_v2() const noexcept;
  /// \brief Compares two capabilities_reply values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(capabilities_reply const&, capabilities_reply const&) noexcept = default;

  /// \brief Process Inquiry Supported Features (bitmap). D0: MIDI Message Report.
  b7 features;
};

constexpr capabilities_reply capabilities_reply::make(packed::capabilities_reply_v2 const& v2) noexcept {
  return {.features = b7{to_underlying(v2.features)}};
}
constexpr capabilities_reply::operator packed::capabilities_reply_v2() const noexcept {
  return {.features = std::byte{features.get()}};
}

//*        _    _ _                                                          _    *
//*  _ __ (_)__| (_)  _ __  ___ ______ __ _ __ _ ___   _ _ ___ _ __  ___ _ _| |_  *
//* | '  \| / _` | | | '  \/ -_|_-<_-</ _` / _` / -_) | '_/ -_) '_ \/ _ \ '_|  _| *
//* |_|_|_|_\__,_|_| |_|_|_\___/__/__/\__,_\__, \___| |_| \___| .__/\___/_|  \__| *
//*                                        |___/              |_|                 *
namespace packed {

/// \brief Version 2 of the Inquiry: MIDI Message Report message.
struct midi_message_report_v2 {
  /// \brief The bitmap of requested System Messages.
  class system_message : private adt::bit_field<std::uint8_t> {
    using song_select_bits = adt::bit_range<2, 1>;
    using song_position_bits = adt::bit_range<1, 1>;
    using mtc_quarter_frame_bits = adt::bit_range<0, 1>;

  public:
    /// \brief Returns the Song Select bit(s).
    /// \returns The Song Select bit(s).
    [[nodiscard]] constexpr auto song_select() const noexcept { return get<song_select_bits>(); }
    /// \brief Returns the Song Position bit(s).
    /// \returns The Song Position bit(s).
    [[nodiscard]] constexpr auto song_position() const noexcept { return get<song_position_bits>(); }
    /// \brief Returns the MTC Quarter Frame bit(s).
    /// \returns The MTC Quarter Frame bit(s).
    [[nodiscard]] constexpr auto mtc_quarter_frame() const noexcept { return get<mtc_quarter_frame_bits>(); }

    /// \brief Sets the Song Select bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& song_select(adt::uinteger_t<song_select_bits::bits::value> const v) noexcept {
      set<song_select_bits>(v);
      return *this;
    }
    /// \brief Sets the Song Position bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& song_position(adt::uinteger_t<song_position_bits::bits::value> const v) noexcept {
      set<song_position_bits>(v);
      return *this;
    }
    /// \brief Sets the MTC Quarter Frame bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& mtc_quarter_frame(adt::uinteger_t<mtc_quarter_frame_bits::bits::value> const v) noexcept {
      set<mtc_quarter_frame_bits>(v);
      return *this;
    }
  };
  /// \brief The bitmap of requested Channel Controller Messages.
  class channel_controller : private adt::bit_field<std::uint8_t> {
    using reserved0_bits = adt::bit_range<6, 2>;
    using channel_pressure_bits = adt::bit_range<5, 1>;
    using program_change_bits = adt::bit_range<4, 1>;
    using nrpn_assignable_controller_bits = adt::bit_range<3, 1>;
    using rpn_registered_controller_bits = adt::bit_range<2, 1>;
    using control_change_bits = adt::bit_range<1, 1>;
    using pitchbend_bits = adt::bit_range<0, 1>;

  public:
    /// \brief Returns the Reserved bit(s).
    /// \returns The Reserved bit(s).
    [[nodiscard]] constexpr auto reserved0() const noexcept { return get<reserved0_bits>(); }
    /// \brief Returns the Channel Pressure bit(s).
    /// \returns The Channel Pressure bit(s).
    [[nodiscard]] constexpr auto channel_pressure() const noexcept { return get<channel_pressure_bits>(); }
    /// \brief Returns the Program Change bit(s).
    /// \returns The Program Change bit(s).
    [[nodiscard]] constexpr auto program_change() const noexcept { return get<program_change_bits>(); }
    /// \brief Returns the NRPN / Assignable Controller bit(s).
    /// \returns The NRPN / Assignable Controller bit(s).
    [[nodiscard]] constexpr auto nrpn_assignable_controller() const noexcept {
      return get<nrpn_assignable_controller_bits>();
    }
    /// \brief Returns the RPN / Registered Controller bit(s).
    /// \returns The RPN / Registered Controller bit(s).
    [[nodiscard]] constexpr auto rpn_registered_controller() const noexcept {
      return get<rpn_registered_controller_bits>();
    }
    /// \brief Returns the Control Change bit(s).
    /// \returns The Control Change bit(s).
    [[nodiscard]] constexpr auto control_change() const noexcept { return get<control_change_bits>(); }
    /// \brief Returns the Pitchbend bit(s).
    /// \returns The Pitchbend bit(s).
    [[nodiscard]] constexpr auto pitchbend() const noexcept { return get<pitchbend_bits>(); }

    /// \brief Sets the Reserved bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& reserved0(reserved0_bits::uinteger const v) noexcept {
      set<reserved0_bits>(v);
      return *this;
    }
    /// \brief Sets the Channel Pressure bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& channel_pressure(channel_pressure_bits::uinteger const v) noexcept {
      set<channel_pressure_bits>(v);
      return *this;
    }
    /// \brief Sets the Program Change bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& program_change(program_change_bits::uinteger const v) noexcept {
      set<program_change_bits>(v);
      return *this;
    }
    /// \brief Sets the NRPN / Assignable Controller bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& nrpn_assignable_controller(nrpn_assignable_controller_bits::uinteger const v) noexcept {
      set<nrpn_assignable_controller_bits>(v);
      return *this;
    }
    /// \brief Sets the RPN / Registered Controller bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& rpn_registered_controller(rpn_registered_controller_bits::uinteger const v) noexcept {
      set<rpn_registered_controller_bits>(v);
      return *this;
    }
    /// \brief Sets the Control Change bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& control_change(control_change_bits::uinteger const v) noexcept {
      set<control_change_bits>(v);
      return *this;
    }
    /// \brief Sets the Pitchbend bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& pitchbend(pitchbend_bits::uinteger const v) noexcept {
      set<pitchbend_bits>(v);
      return *this;
    }
  };
  /// \brief The bitmap of requested Note Data Messages.
  class note_data_messages : private adt::bit_field<std::uint8_t> {
    using reserved0_bits = adt::bit_range<5, 3>;
    using assignable_per_note_controller_bits = adt::bit_range<4, 1>;
    using registered_per_note_controller_bits = adt::bit_range<3, 1>;
    using per_note_pitchbend_bits = adt::bit_range<2, 1>;
    using poly_pressure_bits = adt::bit_range<1, 1>;
    using notes_bits = adt::bit_range<0, 1>;

  public:
    /// \brief Returns the Reserved bit(s).
    /// \returns The Reserved bit(s).
    [[nodiscard]] constexpr auto reserved0() const noexcept { return get<reserved0_bits>(); }
    /// \brief Returns the Assignable Per-Note Controller bit(s).
    /// \returns The Assignable Per-Note Controller bit(s).
    [[nodiscard]] constexpr auto assignable_per_note_controller() const noexcept {
      return get<assignable_per_note_controller_bits>();
    }
    /// \brief Returns the Registered Per-Note Controller bit(s).
    /// \returns The Registered Per-Note Controller bit(s).
    [[nodiscard]] constexpr auto registered_per_note_controller() const noexcept {
      return get<registered_per_note_controller_bits>();
    }
    /// \brief Returns the Per-Note Pitchbend bit(s).
    /// \returns The Per-Note Pitchbend bit(s).
    [[nodiscard]] constexpr auto per_note_pitchbend() const noexcept { return get<per_note_pitchbend_bits>(); }
    /// \brief Returns the Poly Pressure bit(s).
    /// \returns The Poly Pressure bit(s).
    [[nodiscard]] constexpr auto poly_pressure() const noexcept { return get<poly_pressure_bits>(); }
    /// \brief Returns the Notes bit(s).
    /// \returns The Notes bit(s).
    [[nodiscard]] constexpr auto notes() const noexcept { return get<notes_bits>(); }

    /// \brief Sets the Reserved bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& reserved0(reserved0_bits::uinteger const v) noexcept {
      set<reserved0_bits>(v);
      return *this;
    }
    /// \brief Sets the Assignable Per-Note Controller bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& assignable_per_note_controller(assignable_per_note_controller_bits::uinteger const v) noexcept {
      set<assignable_per_note_controller_bits>(v);
      return *this;
    }
    /// \brief Sets the Registered Per-Note Controller bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& registered_per_note_controller(registered_per_note_controller_bits::uinteger const v) noexcept {
      set<registered_per_note_controller_bits>(v);
      return *this;
    }
    /// \brief Sets the Per-Note Pitchbend bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& per_note_pitchbend(per_note_pitchbend_bits::uinteger const v) noexcept {
      set<per_note_pitchbend_bits>(v);
      return *this;
    }
    /// \brief Sets the Poly Pressure bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& poly_pressure(poly_pressure_bits::uinteger const v) noexcept {
      set<poly_pressure_bits>(v);
      return *this;
    }
    /// \brief Sets the Notes bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& notes(notes_bits::uinteger const v) noexcept {
      set<notes_bits>(v);
      return *this;
    }
  };

  /// \brief Message Data Control.
  std::byte message_data_control{0U};
  /// \brief Bitmap of requested System Messages.
  system_message system_message;
  /// \brief Reserved for other System Messages. Set to 0.
  std::byte reserved{0U};
  /// \brief Bitmap of requested Channel Controller Messages.
  channel_controller channel_controller;
  /// \brief Bitmap of requested Note Data Messages.
  note_data_messages note_data_messages;
};

static_assert(offsetof(midi_message_report_v2, message_data_control) == 0);
static_assert(offsetof(midi_message_report_v2, system_message) == 1);
static_assert(offsetof(midi_message_report_v2, reserved) == 2);
static_assert(offsetof(midi_message_report_v2, channel_controller) == 3);
static_assert(offsetof(midi_message_report_v2, note_data_messages) == 4);
static_assert(sizeof(midi_message_report_v2) == 5);
static_assert(alignof(midi_message_report_v2) == 1);
static_assert(std::is_trivially_copyable_v<midi_message_report_v2>);

}  // end namespace packed

/// \brief Inquiry: MIDI Message Report message (M2-101 section 9.5).
struct midi_message_report {
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Inquiry: MIDI Message Report message
  /// representation.
  /// \param v2  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr midi_message_report make(packed::midi_message_report_v2 const& v2) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Inquiry: MIDI Message Report message representation.
  /// \returns The packed representation.
  constexpr explicit operator packed::midi_message_report_v2() const noexcept;
  /// \brief Compares two midi_message_report values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(midi_message_report const&, midi_message_report const&) noexcept = default;

  /// \brief Message Data Control values (M2-101 section 9.5.1).
  enum class control : std::uint8_t {
    /// Do not report any data: send Begin and End replies only.
    no_data = 0x00,
    /// Report only controllers with non-default values, active notes, and per-note controllers with non-default values.
    only_non_default = 0x01,
    /// Full report of all requested and supported controllers, notes, and per-note controllers.
    full = 0x7F,
  };

  /// \brief Message Data Control: the amount of data that the Responder is to return.
  control message_data_control = control::no_data;
  // system messages
  /// \brief True if MTC Quarter Frame messages are requested.
  unsigned mtc_quarter_frame : 1 = 0;
  /// \brief True if Song Position messages are requested.
  unsigned song_position : 1 = 0;
  /// \brief True if Song Select messages are requested.
  unsigned song_select : 1 = 0;
  // channel controller messages
  /// \brief True if Pitchbend messages are requested.
  unsigned pitchbend : 1 = 0;
  /// \brief True if Control Change messages are requested.
  unsigned control_change : 1 = 0;
  /// \brief True if RPN / Registered Controller messages are requested.
  unsigned rpn_registered_controller : 1 = 0;
  /// \brief True if NRPN / Assignable Controller messages are requested.
  unsigned nrpn_assignable_controller : 1 = 0;
  /// \brief True if Program Change messages are requested.
  unsigned program_change : 1 = 0;
  /// \brief True if Channel Pressure messages are requested.
  unsigned channel_pressure : 1 = 0;
  // note data messages
  /// \brief True if Notes messages are requested.
  unsigned notes : 1 = 0;
  /// \brief True if Poly Pressure messages are requested.
  unsigned poly_pressure : 1 = 0;
  /// \brief True if Per-Note Pitchbend messages are requested.
  unsigned per_note_pitchbend : 1 = 0;
  /// \brief True if Registered Per-Note Controller messages are requested.
  unsigned registered_per_note_controller : 1 = 0;
  /// \brief True if Assignable Per-Note Controller messages are requested.
  unsigned assignable_per_note_controller : 1 = 0;
};

constexpr midi_message_report midi_message_report::make(packed::midi_message_report_v2 const& v2) noexcept {
  return {.message_data_control = static_cast<enum control>(v2.message_data_control),

          .mtc_quarter_frame = v2.system_message.mtc_quarter_frame(),
          .song_position = v2.system_message.song_position(),
          .song_select = v2.system_message.song_select(),

          .pitchbend = v2.channel_controller.pitchbend(),
          .control_change = v2.channel_controller.control_change(),
          .rpn_registered_controller = v2.channel_controller.rpn_registered_controller(),
          .nrpn_assignable_controller = v2.channel_controller.nrpn_assignable_controller(),
          .program_change = v2.channel_controller.program_change(),
          .channel_pressure = v2.channel_controller.channel_pressure(),

          .notes = v2.note_data_messages.notes(),
          .poly_pressure = v2.note_data_messages.poly_pressure(),
          .per_note_pitchbend = v2.note_data_messages.per_note_pitchbend(),
          .registered_per_note_controller = v2.note_data_messages.registered_per_note_controller(),
          .assignable_per_note_controller = v2.note_data_messages.assignable_per_note_controller()};
}

constexpr midi_message_report::operator packed::midi_message_report_v2() const noexcept {
  using sm_type = class packed::midi_message_report_v2::system_message;
  auto const sm = sm_type{}.mtc_quarter_frame(mtc_quarter_frame).song_position(song_position).song_select(song_select);

  using cc_type = class packed::midi_message_report_v2::channel_controller;
  auto const cc = cc_type{}
                      .pitchbend(pitchbend)
                      .control_change(control_change)
                      .rpn_registered_controller(rpn_registered_controller)
                      .nrpn_assignable_controller(nrpn_assignable_controller)
                      .program_change(program_change)
                      .channel_pressure(channel_pressure);

  using ndm_type = class packed::midi_message_report_v2::note_data_messages;
  auto const ndm = ndm_type{}
                       .notes(notes)
                       .poly_pressure(poly_pressure)
                       .per_note_pitchbend(per_note_pitchbend)
                       .registered_per_note_controller(registered_per_note_controller)
                       .assignable_per_note_controller(assignable_per_note_controller);

  return {.message_data_control = static_cast<std::byte>(message_data_control),
          .system_message = sm,
          .reserved = std::byte{0U},
          .channel_controller = cc,
          .note_data_messages = ndm};
}

namespace packed {

/// \brief Version 2 of the Reply to MIDI Message Report message.
struct midi_message_report_reply_v2 {
  /// \brief The bitmap of supported System Messages.
  class system_message : private adt::bit_field<std::uint8_t> {
    using reserved0_bits = adt::bit_range<3, 5>;
    using song_select_bits = adt::bit_range<2, 1>;
    using song_position_bits = adt::bit_range<1, 1>;
    using mtc_quarter_frame_bits = adt::bit_range<0, 1>;

  public:
    /// \brief Returns the Reserved bit(s).
    /// \returns The Reserved bit(s).
    [[nodiscard]] constexpr auto reserved0() const noexcept { return get<reserved0_bits>(); }
    /// \brief Returns the Song Select bit(s).
    /// \returns The Song Select bit(s).
    [[nodiscard]] constexpr auto song_select() const noexcept { return get<song_select_bits>(); }
    /// \brief Returns the Song Position bit(s).
    /// \returns The Song Position bit(s).
    [[nodiscard]] constexpr auto song_position() const noexcept { return get<song_position_bits>(); }
    /// \brief Returns the MTC Quarter Frame bit(s).
    /// \returns The MTC Quarter Frame bit(s).
    [[nodiscard]] constexpr auto mtc_quarter_frame() const noexcept { return get<mtc_quarter_frame_bits>(); }

    /// \brief Sets the Reserved bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& reserved0(reserved0_bits::uinteger const v) noexcept {
      set<reserved0_bits>(v);
      return *this;
    }
    /// \brief Sets the Song Select bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& song_select(song_select_bits::uinteger const v) noexcept {
      set<song_select_bits>(v);
      return *this;
    }
    /// \brief Sets the Song Position bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& song_position(song_position_bits::uinteger const v) noexcept {
      set<song_position_bits>(v);
      return *this;
    }
    /// \brief Sets the MTC Quarter Frame bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& mtc_quarter_frame(mtc_quarter_frame_bits::uinteger const v) noexcept {
      set<mtc_quarter_frame_bits>(v);
      return *this;
    }
  };
  /// \brief The bitmap of supported Channel Controller Messages.
  class channel_controller : private adt::bit_field<std::uint8_t> {
    using reserved0_bits = adt::bit_range<6, 2>;
    using channel_pressure_bits = adt::bit_range<5, 1>;
    using program_change_bits = adt::bit_range<4, 1>;
    using nrpn_assignable_controller_bits = adt::bit_range<3, 1>;
    using rpn_registered_controller_bits = adt::bit_range<2, 1>;
    using control_change_bits = adt::bit_range<1, 1>;
    using pitchbend_bits = adt::bit_range<0, 1>;

  public:
    /// \brief Returns the Reserved bit(s).
    /// \returns The Reserved bit(s).
    [[nodiscard]] constexpr auto reserved0() const noexcept { return get<reserved0_bits>(); }
    /// \brief Returns the Channel Pressure bit(s).
    /// \returns The Channel Pressure bit(s).
    [[nodiscard]] constexpr auto channel_pressure() const noexcept { return get<channel_pressure_bits>(); }
    /// \brief Returns the Program Change bit(s).
    /// \returns The Program Change bit(s).
    [[nodiscard]] constexpr auto program_change() const noexcept { return get<program_change_bits>(); }
    /// \brief Returns the NRPN / Assignable Controller bit(s).
    /// \returns The NRPN / Assignable Controller bit(s).
    [[nodiscard]] constexpr auto nrpn_assignable_controller() const noexcept {
      return get<nrpn_assignable_controller_bits>();
    }
    /// \brief Returns the RPN / Registered Controller bit(s).
    /// \returns The RPN / Registered Controller bit(s).
    [[nodiscard]] constexpr auto rpn_registered_controller() const noexcept {
      return get<rpn_registered_controller_bits>();
    }
    /// \brief Returns the Control Change bit(s).
    /// \returns The Control Change bit(s).
    [[nodiscard]] constexpr auto control_change() const noexcept { return get<control_change_bits>(); }
    /// \brief Returns the Pitchbend bit(s).
    /// \returns The Pitchbend bit(s).
    [[nodiscard]] constexpr auto pitchbend() const noexcept { return get<pitchbend_bits>(); }

    /// \brief Sets the Reserved bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& reserved0(reserved0_bits::uinteger const v) noexcept {
      set<reserved0_bits>(v);
      return *this;
    }
    /// \brief Sets the Channel Pressure bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& channel_pressure(channel_pressure_bits::uinteger const v) noexcept {
      set<channel_pressure_bits>(v);
      return *this;
    }
    /// \brief Sets the Program Change bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& program_change(program_change_bits::uinteger const v) noexcept {
      set<program_change_bits>(v);
      return *this;
    }
    /// \brief Sets the NRPN / Assignable Controller bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& nrpn_assignable_controller(nrpn_assignable_controller_bits::uinteger const v) noexcept {
      set<nrpn_assignable_controller_bits>(v);
      return *this;
    }
    /// \brief Sets the RPN / Registered Controller bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& rpn_registered_controller(rpn_registered_controller_bits::uinteger const v) noexcept {
      set<rpn_registered_controller_bits>(v);
      return *this;
    }
    /// \brief Sets the Control Change bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& control_change(control_change_bits::uinteger const v) noexcept {
      set<control_change_bits>(v);
      return *this;
    }
    /// \brief Sets the Pitchbend bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& pitchbend(pitchbend_bits::uinteger const v) noexcept {
      set<pitchbend_bits>(v);
      return *this;
    }
  };
  /// \brief The bitmap of supported Note Data Messages.
  class note_data_messages : private adt::bit_field<std::uint8_t> {
    using reserved0_bits = adt::bit_range<5, 3>;
    using assignable_per_note_controller_bits = adt::bit_range<4, 1>;
    using registered_per_note_controller_bits = adt::bit_range<3, 1>;
    using per_note_pitchbend_bits = adt::bit_range<2, 1>;
    using poly_pressure_bits = adt::bit_range<1, 1>;
    using notes_bits = adt::bit_range<0, 1>;

  public:
    /// \brief Returns the Reserved bit(s).
    /// \returns The Reserved bit(s).
    [[nodiscard]] constexpr auto reserved0() const noexcept { return get<reserved0_bits>(); }
    /// \brief Returns the Assignable Per-Note Controller bit(s).
    /// \returns The Assignable Per-Note Controller bit(s).
    [[nodiscard]] constexpr auto assignable_per_note_controller() const noexcept {
      return get<assignable_per_note_controller_bits>();
    }
    /// \brief Returns the Registered Per-Note Controller bit(s).
    /// \returns The Registered Per-Note Controller bit(s).
    [[nodiscard]] constexpr auto registered_per_note_controller() const noexcept {
      return get<registered_per_note_controller_bits>();
    }
    /// \brief Returns the Per-Note Pitchbend bit(s).
    /// \returns The Per-Note Pitchbend bit(s).
    [[nodiscard]] constexpr auto per_note_pitchbend() const noexcept { return get<per_note_pitchbend_bits>(); }
    /// \brief Returns the Poly Pressure bit(s).
    /// \returns The Poly Pressure bit(s).
    [[nodiscard]] constexpr auto poly_pressure() const noexcept { return get<poly_pressure_bits>(); }
    /// \brief Returns the Notes bit(s).
    /// \returns The Notes bit(s).
    [[nodiscard]] constexpr auto notes() const noexcept { return get<notes_bits>(); }

    /// \brief Sets the Reserved bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& reserved0(reserved0_bits::uinteger const v) noexcept {
      set<reserved0_bits>(v);
      return *this;
    }
    /// \brief Sets the Assignable Per-Note Controller bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& assignable_per_note_controller(assignable_per_note_controller_bits::uinteger const v) noexcept {
      set<assignable_per_note_controller_bits>(v);
      return *this;
    }
    /// \brief Sets the Registered Per-Note Controller bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& registered_per_note_controller(registered_per_note_controller_bits::uinteger const v) noexcept {
      set<registered_per_note_controller_bits>(v);
      return *this;
    }
    /// \brief Sets the Per-Note Pitchbend bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& per_note_pitchbend(per_note_pitchbend_bits::uinteger const v) noexcept {
      set<per_note_pitchbend_bits>(v);
      return *this;
    }
    /// \brief Sets the Poly Pressure bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& poly_pressure(poly_pressure_bits::uinteger const v) noexcept {
      set<poly_pressure_bits>(v);
      return *this;
    }
    /// \brief Sets the Notes bit(s).
    /// \param v  The new value.
    /// \returns *this
    constexpr auto& notes(notes_bits::uinteger const v) noexcept {
      set<notes_bits>(v);
      return *this;
    }
  };

  /// \brief Bitmap of System Messages that will be reported.
  class system_message system_message;
  /// \brief Reserved for other System Messages. Set to 0.
  std::byte reserved{0U};
  /// \brief Bitmap of Channel Controller Messages that will be reported.
  class channel_controller channel_controller;
  /// \brief Bitmap of Note Data Messages that will be reported.
  class note_data_messages note_data_messages;
};

static_assert(offsetof(midi_message_report_reply_v2, system_message) == 0);
static_assert(offsetof(midi_message_report_reply_v2, reserved) == 1);
static_assert(offsetof(midi_message_report_reply_v2, channel_controller) == 2);
static_assert(offsetof(midi_message_report_reply_v2, note_data_messages) == 3);
static_assert(sizeof(midi_message_report_reply_v2) == 4);
static_assert(alignof(midi_message_report_reply_v2) == 1);
static_assert(std::is_trivially_copyable_v<midi_message_report_reply_v2>);

}  // end namespace packed

/// \brief Reply to MIDI Message Report message (M2-101 section 9.6).
struct midi_message_report_reply {
  /// \brief Creates an instance from the packed (on-the-wire) version 2 Reply to MIDI Message Report message
  /// representation.
  /// \param v2  The packed message.
  /// \returns The new instance.
  [[nodiscard]] static constexpr midi_message_report_reply make(
      packed::midi_message_report_reply_v2 const& v2) noexcept;
  /// \brief Converts to the packed (on-the-wire) version 2 Reply to MIDI Message Report message representation.
  /// \returns The packed representation.
  constexpr explicit operator packed::midi_message_report_reply_v2() const noexcept;
  /// \brief Compares two midi_message_report_reply values for equality.
  /// \returns True if the two values are equal.
  constexpr friend bool operator==(midi_message_report_reply const&,
                                   midi_message_report_reply const&) noexcept = default;

  // system messages
  /// \brief True if MTC Quarter Frame messages will be reported.
  unsigned mtc_quarter_frame : 1 = 0;
  /// \brief True if Song Position messages will be reported.
  unsigned song_position : 1 = 0;
  /// \brief True if Song Select messages will be reported.
  unsigned song_select : 1 = 0;
  // channel controller messages
  /// \brief True if Pitchbend messages will be reported.
  unsigned pitchbend : 1 = 0;
  /// \brief True if Control Change messages will be reported.
  unsigned control_change : 1 = 0;
  /// \brief True if RPN / Registered Controller messages will be reported.
  unsigned rpn_registered_controller : 1 = 0;
  /// \brief True if NRPN / Assignable Controller messages will be reported.
  unsigned nrpn_assignable_controller : 1 = 0;
  /// \brief True if Program Change messages will be reported.
  unsigned program_change : 1 = 0;
  /// \brief True if Channel Pressure messages will be reported.
  unsigned channel_pressure : 1 = 0;
  // note data messages
  /// \brief True if Notes messages will be reported.
  unsigned notes : 1 = 0;
  /// \brief True if Poly Pressure messages will be reported.
  unsigned poly_pressure : 1 = 0;
  /// \brief True if Per-Note Pitchbend messages will be reported.
  unsigned per_note_pitchbend : 1 = 0;
  /// \brief True if Registered Per-Note Controller messages will be reported.
  unsigned registered_per_note_controller : 1 = 0;
  /// \brief True if Assignable Per-Note Controller messages will be reported.
  unsigned assignable_per_note_controller : 1 = 0;
};

constexpr midi_message_report_reply midi_message_report_reply::make(
    packed::midi_message_report_reply_v2 const& v2) noexcept {
  return {
      .mtc_quarter_frame = v2.system_message.mtc_quarter_frame(),
      .song_position = v2.system_message.song_position(),
      .song_select = v2.system_message.song_select(),

      .pitchbend = v2.channel_controller.pitchbend(),
      .control_change = v2.channel_controller.control_change(),
      .rpn_registered_controller = v2.channel_controller.rpn_registered_controller(),
      .nrpn_assignable_controller = v2.channel_controller.nrpn_assignable_controller(),
      .program_change = v2.channel_controller.program_change(),
      .channel_pressure = v2.channel_controller.channel_pressure(),

      .notes = v2.note_data_messages.notes(),
      .poly_pressure = v2.note_data_messages.poly_pressure(),
      .per_note_pitchbend = v2.note_data_messages.per_note_pitchbend(),
      .registered_per_note_controller = v2.note_data_messages.registered_per_note_controller(),
      .assignable_per_note_controller = v2.note_data_messages.assignable_per_note_controller(),
  };
}
constexpr midi_message_report_reply::operator packed::midi_message_report_reply_v2() const noexcept {
  using sm_type = class packed::midi_message_report_reply_v2::system_message;
  using cc_type = class packed::midi_message_report_reply_v2::channel_controller;
  using ndm_type = class packed::midi_message_report_reply_v2::note_data_messages;
  return {.system_message =
              sm_type{}.mtc_quarter_frame(mtc_quarter_frame).song_position(song_position).song_select(song_select),
          .reserved = std::byte{0U},
          .channel_controller = cc_type{}
                                    .pitchbend(pitchbend)
                                    .control_change(control_change)
                                    .rpn_registered_controller(rpn_registered_controller)
                                    .nrpn_assignable_controller(nrpn_assignable_controller)
                                    .program_change(program_change)
                                    .channel_pressure(channel_pressure),
          .note_data_messages = ndm_type{}
                                    .notes(notes)
                                    .poly_pressure(poly_pressure)
                                    .per_note_pitchbend(per_note_pitchbend)
                                    .registered_per_note_controller(registered_per_note_controller)
                                    .assignable_per_note_controller(assignable_per_note_controller)};
}

//*                                      _                 _  *
//*  _ __  _ __    _ _ ___ _ __  ___ _ _| |_   ___ _ _  __| | *
//* | '  \| '  \  | '_/ -_) '_ \/ _ \ '_|  _| / -_) ' \/ _` | *
//* |_|_|_|_|_|_| |_| \___| .__/\___/_|  \__| \___|_||_\__,_| *
//*                       |_|                                 *
/// \brief End of MIDI Message Report message (M2-101 section 9.8). This message has no body.
struct midi_message_report_end {};

}  // end namespace process_inquiry
}  // end namespace midi2::ci

#endif  // MIDI2_CI_TYPES_HPP
