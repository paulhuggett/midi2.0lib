//===-- CI Create Message -----------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ci_create_message.hpp
/// \brief Functions which serialize MIDI-CI messages to a sequence of bytes.

#ifndef MIDI2_CI_CREATE_MESSAGE_HPP
#define MIDI2_CI_CREATE_MESSAGE_HPP

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <span>
#include <type_traits>

#include "midi2/ci/ci_types.hpp"

namespace midi2::ci {

/// \brief Private implementation details of the CI types and functions
namespace details {

/// \brief Maps a MIDI-CI message type to its message ID and to the packed (on-the-wire) types used to send it.
///
/// Each specialization provides a static constant `id` (the message ID) and, for messages with a fixed-size body, the
/// member types `v1` and `v2` which are the packed representations for MIDI-CI version 1 and version 2 (and later)
/// respectively.
///
/// \tparam T  A MIDI-CI message type.
template <typename T> struct type_to_packed;

/// \brief A type used by type_to_packed<> to indicate that a message has no body.
struct empty {};
/// \brief A type used by type_to_packed<> to indicate that a message is not defined for a MIDI-CI version.
struct not_available {};
/// Maps the public "discovery" message structure to the packed types that conform to the MIDI CI standard for sending
/// to other devices.
template <> struct type_to_packed<discovery> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::discovery;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = packed::discovery_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = packed::discovery_v2;
};
/// Maps the public "discovery reply" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<discovery_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::discovery_reply;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = packed::discovery_reply_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = packed::discovery_reply_v2;
};
/// Maps the public "endpoint" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<endpoint> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::endpoint;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = packed::endpoint_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = packed::endpoint_v1;
};
/// Maps the public "endpoint reply" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<endpoint_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::endpoint_reply;
};
/// Maps the public "invalidate MUID" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<invalidate_muid> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::invalidate_muid;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = packed::invalidate_muid_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = packed::invalidate_muid_v1;
};
/// Maps the public "ACK" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<ack> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::ack;
};
/// Maps the public "NAK" message structure to the packed types that conform to the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<nak> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::nak;
  /// \brief The packed type for MIDI-CI version 1: empty because the message has no body.
  using v1 = empty;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = packed::nak_v2;
};

// Profile Configuration Types

/// Maps the public "profile configuration added" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::added> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_added;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::added_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::added_v1;
};
/// Maps the public "profile configuration removed" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::removed> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_removed;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::removed_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::removed_v1;
};
/// Maps the public "profile configuration details" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::details> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_details;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::details_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::details_v1;
};
/// Maps the public "profile configuration details reply" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::details_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_details_reply;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::details_reply_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::details_reply_v1;
};
/// Maps the public "profile configuration Inquiry" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::inquiry> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_inquiry;
  /// \brief The packed type for MIDI-CI version 1: empty because the message has no body.
  using v1 = empty;
  /// \brief The packed type for MIDI-CI version 2 and later: empty because the message has no body.
  using v2 = empty;
};
/// Maps the public "profile configuration Inquiry Reply" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::inquiry_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_inquiry_reply;
};
/// Maps the public "profile configuration on" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::on> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_set_on;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::on_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::on_v2;
};
/// Maps the public "profile configuration off" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::off> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_set_off;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::off_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::off_v2;
};
/// Maps the public "profile configuration enabled" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::enabled> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_enabled;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::enabled_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::enabled_v2;
};
/// Maps the public "profile configuration disabled" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::disabled> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_disabled;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::disabled_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::disabled_v2;
};
/// Maps the public "profile configuration specific data" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<profile_configuration::specific_data> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::profile_specific_data;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = profile_configuration::packed::specific_data_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = profile_configuration::packed::specific_data_v1;
};

// Property Exchange Types

/// Maps the public "property exchange capabilities" message structure to the packed types that conform to the MIDI CI
/// standard for sending to other devices.
template <> struct type_to_packed<property_exchange::capabilities> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_capability;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = property_exchange::packed::capabilities_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = property_exchange::packed::capabilities_v2;
};
/// Maps the public "property exchange capabilities reply" message structure to the packed types that conform to the
/// MIDI CI standard for sending to other devices.
template <> struct type_to_packed<property_exchange::capabilities_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_capability_reply;
  /// \brief The packed type for MIDI-CI version 1.
  using v1 = property_exchange::packed::capabilities_reply_v1;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = property_exchange::packed::capabilities_reply_v2;
};
/// Maps the public "property_exchange::get" message structure to the packed types that conform to the MIDI CI standard
/// for
/// sending to other devices.
template <> struct type_to_packed<property_exchange::get> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_get;
};
/// Maps the public "property_exchange::get_reply" message structure to the packed types that conform to the MIDI CI
/// standard for
/// sending to other devices.
template <> struct type_to_packed<property_exchange::get_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_get_reply;
};
/// Maps the public "property_exchange::set" message structure to the packed types that conform to the MIDI CI standard
/// for
/// sending to other devices.
template <> struct type_to_packed<property_exchange::set> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_set;
};
/// Maps the public "property_exchange::set_reply" message structure to the packed types that conform to the MIDI CI
/// standard for
/// sending to other devices.
template <> struct type_to_packed<property_exchange::set_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_set_reply;
};
/// Maps the public "property_exchange::subscription" message structure to the packed types that conform to the MIDI CI
/// standard for
/// sending to other devices.
template <> struct type_to_packed<property_exchange::subscription> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_sub;
};
/// Maps the public "property_exchange::subscription_reply" message structure to the packed types that conform to the
/// MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<property_exchange::subscription_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_sub_reply;
};
/// Maps the public "property_exchange::notify" message structure to the packed types that conform to the MIDI CI
/// standard for
/// sending to other devices.
template <> struct type_to_packed<property_exchange::notify> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pe_notify;
};

// Process Inquiry

/// Maps the public "process_inquiry::capabilities" message structure to the packed types that conform to the MIDI CI
/// standard for
/// sending to other devices.
template <> struct type_to_packed<process_inquiry::capabilities> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pi_capability;
  /// \brief The packed type for MIDI-CI version 1: empty because the message has no body.
  using v1 = empty;
  /// \brief The packed type for MIDI-CI version 2 and later: empty because the message has no body.
  using v2 = empty;
};
/// Maps the public "process_inquiry::capabilities_reply" message structure to the packed types that conform to the MIDI
/// CI standard for
/// sending to other devices.
template <> struct type_to_packed<process_inquiry::capabilities_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pi_capability_reply;
  /// \brief The packed type for MIDI-CI version 1: not_available because the message is not defined
  /// for that version.
  using v1 = not_available;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = process_inquiry::packed::capabilities_reply_v2;
};
/// Maps the public "process_inquiry::midi_message_report" message structure to the packed types that conform to the
/// MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<process_inquiry::midi_message_report> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pi_mm_report;
  /// \brief The packed type for MIDI-CI version 1: not_available because the message is not defined
  /// for that version.
  using v1 = not_available;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = process_inquiry::packed::midi_message_report_v2;
};
/// Maps the public "process_inquiry::midi_message_report_reply" message structure to the packed types that conform to
/// the MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<process_inquiry::midi_message_report_reply> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pi_mm_report_reply;
  /// \brief The packed type for MIDI-CI version 1: not_available because the message is not defined
  /// for that version.
  using v1 = not_available;
  /// \brief The packed type for MIDI-CI version 2 and later.
  using v2 = process_inquiry::packed::midi_message_report_reply_v2;
};
/// Maps the public "process_inquiry::midi_message_report_end" message structure to the packed types that conform to the
/// MIDI CI standard for
/// sending to other devices.
template <> struct type_to_packed<process_inquiry::midi_message_report_end> {
  /// \brief The MIDI-CI message type (the Universal System Exclusive Sub-ID#2 value).
  static constexpr auto id = message::pi_mm_report_end;
  /// \brief The packed type for MIDI-CI version 1: not_available because the message is not defined
  /// for that version.
  using v1 = not_available;
  /// \brief The packed type for MIDI-CI version 2 and later: empty because the message has no body.
  using v2 = empty;
};

/// \brief Copies the bytes of \p t to the byte range specified by \p first and \p last.
///
/// \tparam T  A trivially copyable type with an alignment of 1.
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are copied.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are copied.
/// \param t  The object whose bytes are to be copied.
/// \returns An output iterator past the last element copied if there was sufficient space. If there was not
///   sufficient space, nothing is copied and \p first is advanced as far as \p last.
/// \note The copy is only performed if there is room for more than sizeof(T) bytes: if the range holds exactly
///   sizeof(T) bytes, nothing is copied.
template <typename T, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
  requires(std::is_trivially_copyable_v<T> && alignof(T) == 1)
constexpr O safe_copy(O first, S last, T const& t) {
  auto first2 = first;
  // Is there sufficient room?
  std::ranges::advance(first2, sizeof(T), last);
  if (first2 != last) {
    // Yes: do the copy.
    first2 = std::ranges::copy(std::bit_cast<std::byte const*>(&t), std::bit_cast<std::byte const*>(&t + 1), first).out;
  }
  return first2;
}

/// \brief Writes the fixed-size part of a packed message followed by its variable-length tail.
///
/// \tparam ElementType  The type of the elements of the tail. A trivially copyable type with an alignment of 1.
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param ptr  A pointer to the bytes of the fixed-size part of the message.
/// \param size  The number of bytes of the fixed-size part of the message.
/// \param span  The variable-length tail of the message.
/// \returns An output iterator past the last element written. If there was not sufficient space, nothing is written
///   and \p first is advanced as far as \p last.
/// \note As with safe_copy(), nothing is written if the range holds exactly the number of bytes required.
template <typename ElementType, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
  requires(std::is_trivially_copyable_v<ElementType> && alignof(ElementType) == 1)
constexpr O write_packed_with_tail(O first, S const last, std::byte const* ptr, std::size_t const size,
                                   std::span<ElementType const> const span) {
  auto first2 = first;
  std::ranges::advance(first2, static_cast<std::iter_difference_t<decltype(first)>>(size + span.size_bytes()), last);
  if (first2 != last) {
    first = std::ranges::copy(ptr, ptr + size, first).out;
    first2 = std::ranges::copy(
                 std::span<std::byte const>{std::bit_cast<std::byte const*>(span.data()), span.size_bytes()}, first)
                 .out;
  }
  return first2;
}

/// \brief Writes a packed MIDI-CI message header.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param h  The message header.
/// \param id  The message type (Sub-ID#2).
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O write_header(O first, S const last, struct header const& h, message const id) {
  auto hdr = static_cast<packed::header>(h);
  hdr.sub_id_2 = static_cast<std::byte>(id);
  return details::safe_copy(first, last, hdr);
}

/// \brief Writes a MIDI-CI message header followed by a fixed-size message body.
///
/// \tparam ExternalType  The packed (on-the-wire) type of the message body, or details::empty if there is no body.
/// \tparam InternalType  The type of the message.
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param t  The message.
/// \returns An output iterator past the last element written.
template <typename ExternalType, typename InternalType, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O write_header_body(O first, S const last, header const& hdr, InternalType const& t) {
  first = write_header(first, last, hdr, details::type_to_packed<InternalType>::id);
  if constexpr (!std::is_same_v<ExternalType, details::empty>) {
    first = safe_copy(first, last, static_cast<ExternalType>(t));
  }
  return first;
}

/// \brief Wraps an array of bytes that is large enough to hold an instance of type \p T.
///
/// \p T is a trivially copyable type with alignment of 1 so can be safely copied into the byte array. A data() member
/// enables access to that array.
///
/// \tparam T  A trivially copyable type with an alignment of 1.
template <typename T>
  requires(std::is_trivially_copyable_v<T> && alignof(T) == 1)
class byte_array_wrapper {
public:
  /// \brief Copies the bytes of an object into the array.
  /// \param other  The object to be copied.
  explicit constexpr byte_array_wrapper(T const& other) { std::memcpy(arr_.data(), &other, sizeof(T)); }
  /// \brief Returns a pointer to the array of bytes.
  /// \returns A pointer to the array of bytes.
  [[nodiscard]] constexpr std::byte const* data() const noexcept { return arr_.data(); }

private:
  byte_array<sizeof(T)> arr_;
};

/// \brief Writes a Property Exchange message: the header, the request ID and header data, and the chunk information
/// and property data.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \tparam Pet  The type of the Property Exchange message.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param pe  The Property Exchange message.
/// \param id  The message type (Sub-ID#2).
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S, property_exchange::property_exchange_type Pet>
constexpr O write_pe(O first, S const last, header const& hdr, property_exchange::property_exchange<Pet> const& pe,
                     message const id) {
  first = details::write_header(first, last, hdr, id);

  using property_exchange::packed::property_exchange_pt1;
  using property_exchange::packed::property_exchange_pt2;

  byte_array_wrapper const part1{static_cast<property_exchange_pt1>(pe)};
  first =
      details::write_packed_with_tail(first, last, part1.data(), offsetof(property_exchange_pt1, header), pe.header);
  byte_array_wrapper const part2{static_cast<property_exchange_pt2>(pe)};
  first = details::write_packed_with_tail(first, last, part2.data(), offsetof(property_exchange_pt2, data), pe.data);
  return first;
}

}  // end namespace details

/// \brief A sentinel which never compares equal to an iterator. Use this with create_message() when writing to an
/// unbounded output iterator such as std::back_insert_iterator<>.
struct trivial_sentinel {
  /// Trivial sentinels are never equal to an object of a different type.
  /// \tparam T  The type of the object being compared.
  /// \returns False.
  template <typename T> friend constexpr bool operator==(trivial_sentinel, T) noexcept { return false; }
  /// Trivial sentinels always compare equal.
  /// \returns True.
  friend constexpr bool operator==(trivial_sentinel, trivial_sentinel) noexcept = default;
};

/// \brief Serializes a MIDI-CI message whose body has a fixed size.
///
/// The packed layout used for the body is selected by the version field of \p hdr: version 1 uses the version 1
/// layout; any other version uses the version 2 layout. If the message is not defined for version 1, nothing is
/// written.
///
/// \tparam T  The type of the message.
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param t  The message.
/// \returns An output iterator past the last element written.
/// \note The bytes written do not include the F0 (start) and F7 (end) System Exclusive bytes.
template <typename T, std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, T const& t) {
  if (hdr.version == b7{1U}) {
    using v1_type = details::type_to_packed<T>::v1;
    if constexpr (!std::is_same_v<v1_type, details::not_available>) {
      first = details::write_header_body<v1_type>(first, last, hdr, t);
    }
    return first;
  }
  return details::write_header_body<typename details::type_to_packed<T>::v2>(first, last, hdr, t);
}

/// \brief Serializes a Reply to Endpoint message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param reply  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, endpoint_reply const& reply) {
  first = details::write_header(first, last, hdr, details::type_to_packed<endpoint_reply>::id);
  details::byte_array_wrapper const v1{static_cast<packed::endpoint_reply_v1>(reply)};
  return details::write_packed_with_tail(first, last, v1.data(), offsetof(packed::endpoint_reply_v1, data),
                                         reply.information);
}

/// \brief Serializes a MIDI-CI ACK message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param ack  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, struct ack const& ack) {
  first = details::write_header(first, last, hdr, details::type_to_packed<struct ack>::id);
  details::byte_array_wrapper const v1{static_cast<packed::ack_v1>(ack)};
  return details::write_packed_with_tail(first, last, v1.data(), offsetof(packed::ack_v1, message), ack.message);
}

/// \brief Serializes a MIDI-CI NAK message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param nak  The message.
/// \note If the header version is 1, only the header is written because version 1 NAK messages have no body.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S last, header const& hdr, struct nak const& nak) {
  first = details::write_header(first, last, hdr, details::type_to_packed<struct nak>::id);
  if (hdr.version == b7{1U}) {
    return first;
  }
  details::byte_array_wrapper const v2{static_cast<packed::nak_v2>(nak)};
  return details::write_packed_with_tail(first, last, v2.data(), offsetof(packed::nak_v2, message), nak.message);
}

/// \brief Serializes a Reply to Profile Details Inquiry message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param reply  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr,
                           profile_configuration::details_reply const& reply) {
  using profile_configuration::packed::details_reply_v1;
  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::details_reply>::id);
  details::byte_array_wrapper const v1{static_cast<details_reply_v1>(reply)};
  return details::write_packed_with_tail(first, last, v1.data(), offsetof(details_reply_v1, data), reply.data);
}

/// \brief Serializes a Reply to Profile Inquiry message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param reply  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr,
                           profile_configuration::inquiry_reply const& reply) {
  using profile_configuration::packed::inquiry_reply_v1_pt1;
  using profile_configuration::packed::inquiry_reply_v1_pt2;

  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::inquiry_reply>::id);
  details::byte_array_wrapper const part1{static_cast<inquiry_reply_v1_pt1>(reply)};
  first =
      details::write_packed_with_tail(first, last, part1.data(), offsetof(inquiry_reply_v1_pt1, ids), reply.enabled);
  details::byte_array_wrapper const part2{static_cast<inquiry_reply_v1_pt2>(reply)};
  first =
      details::write_packed_with_tail(first, last, part2.data(), offsetof(inquiry_reply_v1_pt2, ids), reply.disabled);
  return first;
}

/// \brief Serializes a Profile Specific Data message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param sd  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, profile_configuration::specific_data const& sd) {
  using profile_configuration::packed::specific_data_v1;
  first = details::write_header(first, last, hdr, details::type_to_packed<profile_configuration::specific_data>::id);
  details::byte_array_wrapper const v1{static_cast<specific_data_v1>(sd)};
  return details::write_packed_with_tail(first, last, v1.data(), offsetof(specific_data_v1, data), sd.data);
}

/// \brief Serializes an Inquiry: Get Property Data message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param pe  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, property_exchange::get const& pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
/// \brief Serializes a Reply to Get Property Data message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param pe  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, property_exchange::get_reply const& pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
/// \brief Serializes an Inquiry: Set Property Data message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param pe  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, property_exchange::set const& pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
/// \brief Serializes a Reply to Set Property Data message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param pe  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, property_exchange::set_reply const& pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
/// \brief Serializes a Subscription message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param pe  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, property_exchange::subscription const& pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
/// \brief Serializes a Reply to Subscription message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param pe  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, property_exchange::subscription_reply const& pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}
/// \brief Serializes a Notify message.
///
/// \tparam O  An output iterator type which accepts std::byte values.
/// \tparam S  A sentinel type for \p O.
/// \param first  Along with \p last, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param last  Along with \p first, the iterator-sentinel pair defining the range to which the bytes are written.
/// \param hdr  The message header.
/// \param pe  The message.
/// \returns An output iterator past the last element written.
template <std::output_iterator<std::byte> O, std::sentinel_for<O> S>
constexpr O create_message(O first, S const last, header const& hdr, property_exchange::notify const& pe) {
  return details::write_pe(first, last, hdr, pe, details::type_to_packed<std::remove_cvref_t<decltype(pe)>>::id);
}

}  // end namespace midi2::ci

#endif  // MIDI2_CI_CREATE_MESSAGE_HPP
