//===-- UMP Types -------------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ump_types.hpp
/// \brief Defines UMP message types

#ifndef MIDI2_UMP_TYPES_HPP
#define MIDI2_UMP_TYPES_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <ranges>
#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
#include <string>
#endif

#include "midi2/adt/bitfield.hpp"
#include "midi2/adt/uinteger.hpp"
#include "midi2/ump/ump_as_array.hpp"
#include "midi2/ump/ump_utils.hpp"

namespace midi2::ump {

/// Convert a fixed-size span<> of T to a tuple<> with the same number of elements in the same order. Each
/// member of the tuple must be constructible from T.
///
///     std::array<T1, 2> in = {...};
///     std::tuple<T2, T3> out = span_to_tuple(in);
///
/// (where instances of T2 and T3 can be constructed from T1).
///
/// \tparam T  The type of the span elements.
/// \tparam N  The number of elements in the span.
/// \param s  The span to be converted.
/// \returns A tuple of N elements, each initialized from the corresponding element of \p s.
template <typename T, std::size_t N> constexpr auto span_to_tuple(std::span<T, N> s) {
  auto const impl = []<std::size_t... Is>(std::span<T, N> sp, std::index_sequence<Is...>) {
    return std::tuple{sp[Is]...};
  };
  return impl(s, std::make_index_sequence<N>{});
}

/// The values of the UMP "mt" (message type) field. The message type is held in the four most significant bits of
/// the first word of every UMP and determines the size of the UMP and the group of messages to which it belongs.
enum class message_type : std::uint8_t {
  utility = 0x00,         ///< Utility messages
  system = 0x01,          ///< System real time and system common messages (except system exclusive)
  m1cvm = 0x02,           ///< MIDI 1.0 channel voice messages
  data64 = 0x03,          ///< 64-bit data messages (including system exclusive)
  m2cvm = 0x04,           ///< MIDI 2.0 channel voice messages
  data128 = 0x05,         ///< 128-bit data messages
  reserved32_06 = 0x06,   ///< Reserved for future definition
  reserved32_07 = 0x07,   ///< Reserved for future definition
  reserved64_08 = 0x08,   ///< Reserved for future definition
  reserved64_09 = 0x09,   ///< Reserved for future definition
  reserved64_0a = 0x0A,   ///< Reserved for future definition
  reserved96_0b = 0x0B,   ///< Reserved for future definition
  reserved96_0c = 0x0C,   ///< Reserved for future definition
  flex_data = 0x0D,       ///< Flex data messages
  reserved128_0e = 0x0E,  ///< Reserved for future definition
  stream = 0x0F,          ///< UMP stream messages
};

/// Collects the enumerations that define the values of the status field for each UMP message type. Each
/// enumeration corresponds to one of the values of midi2::ump::message_type.
namespace mt {

/// The status values for the System Common and System Real-time messages
/// \note Here, CRT stands for "Common and Real-Time".
enum class system_crt : std::uint8_t {
  // System Common messages
  timing_code = 0xF1,   ///< MIDI Time Code
  spp = 0xF2,           ///< Song Position Pointer
  song_select = 0xF3,   ///< Song Select
  tune_request = 0xF6,  ///< Tune Request
  // System Real Time messages
  timing_clock = 0xF8,       ///< Timing Clock
  sequence_start = 0xFA,     ///< Start the current sequence playing
  sequence_continue = 0xFB,  ///< Continue at the point the sequence was stopped
  sequence_stop = 0xFC,      ///< Stop the current sequence
  active_sensing = 0xFE,     ///< Active Sensing
  system_reset = 0xFF,       ///< Reset
};

/// The status values for the MIDI 1.0 Channel Voice messages
enum class m1cvm : std::uint8_t {
  note_off = 0x8,          ///< Note Off
  note_on = 0x9,           ///< Note On
  poly_pressure = 0xA,     ///< Polyphonic Key Pressure (Aftertouch).
  cc = 0xB,                ///< Control Change
  program_change = 0xC,    ///< Program Change
  channel_pressure = 0xD,  ///< Channel Pressure (Aftertouch).
  pitch_bend = 0xE,        ///< Pitch Bend
};

/// The status values for the MIDI 2.0 Channel Voice messages
enum class m2cvm : std::uint8_t {
  rpn_per_note = 0x0,         ///< Registered per-note controller
  nrpn_per_note = 0x1,        ///< Assignable per-note controller
  rpn = 0x2,                  ///< Registered controller (RPN)
  nrpn = 0x3,                 ///< Assignable controller (NRPN)
  rpn_relative = 0x4,         ///< Relative registered controller (RPN)
  nrpn_relative = 0x5,        ///< Relative assignable controller (NRPN)
  pitch_bend_per_note = 0x6,  ///< Per-note pitch bend
  note_off = 0x8,             ///< Note off
  note_on = 0x9,              ///< Note on
  poly_pressure = 0xA,        ///< Polyphonic aftertouch
  cc = 0xB,                   ///< Control change
  program_change = 0xC,       ///< Program change
  channel_pressure = 0xD,     ///< Channel pressure (aftertouch)
  pitch_bend = 0xE,           ///< Pitch bend
  per_note_manage = 0xF,      ///< Per-note management
};

/// The status values for the Data 64-bit (7-bit system exclusive) messages
enum class data64 : std::uint8_t {
  /// Complete system exclusive message in one UMP.
  sysex7_in_1 = 0x00,
  /// System exclusive start UMP. Terminate with a System Exclusive End UMP.
  sysex7_start = 0x01,
  /// System exclusive continue UMP.
  sysex7_continue = 0x02,
  /// System exclusive end UMP.
  sysex7_end = 0x03,
};

/// The status values for the Utility messages
enum class utility : std::uint8_t {
  noop = 0b0000,               ///< A "no operation" message
  jr_clock = 0b0001,           ///< Jitter reduction clock
  jr_ts = 0b0010,              ///< Jitter reduction time-stamp
  delta_clock_tick = 0b0011,   ///< Delta Clockstamp: Ticks Per Quarter Note
  delta_clock_since = 0b0100,  ///< Delta Clockstamp: Ticks Since Last Event
};

/// The status values for the Flex Data messages in status bank 0 (setup and performance events)
enum class flex_data : std::uint8_t {
  set_tempo = 0x00,           ///< Status code for the ump::flex_data::set_tempo message
  set_time_signature = 0x01,  ///< Status code for the ump::flex_data::set_time_signature message
  set_metronome = 0x02,       ///< Status code for the ump::flex_data::set_metronome message
  set_key_signature = 0x05,   ///< Status code for the ump::flex_data::set_key_signature message
  set_chord_name = 0x06,      ///< Status code for the ump::flex_data::set_chord_name message
};

/// The status values for the UMP Stream messages
enum class stream : std::uint16_t {
  endpoint_discovery = 0x00,            ///< Status code for the ump::stream::endpoint_discovery message
  endpoint_info_notification = 0x01,    ///< Status code for the ump::stream::endpoint_info_notification message
  device_identity_notification = 0x02,  ///< Status code for the ump::stream::device_identity_notification message
  endpoint_name_notification = 0x03,    ///< Status code for the ump::stream::endpoint_name_notification message
  /// Status code for the ump::stream::product_instance_id_notification message
  product_instance_id_notification = 0x04,
  /// Status code for the ump::stream::jr_configuration_request (Stream Configuration Request) message
  jr_configuration_request = 0x05,
  /// Status code for the ump::stream::jr_configuration_notification (Stream Configuration Notification) message
  jr_configuration_notification = 0x06,
  function_block_discovery = 0x10,  ///< Status code for the ump::stream::function_block_discovery message
  /// Status code for the ump::stream::function_block_info_notification message
  function_block_info_notification = 0x11,
  /// Status code for the ump::stream::function_block_name_notification message
  function_block_name_notification = 0x12,
  start_of_clip = 0x20,  ///< Status code for the ump::stream::start_of_clip message
  end_of_clip = 0x21,    ///< Status code for the ump::stream::end_of_clip message
};

/// The status values for the Data 128-bit (system exclusive 8 and mixed data set) messages
enum class data128 : std::uint8_t {
  /// Status code for the midi2::ump::data128::sysex8_in_1 message.
  sysex8_in_1 = 0x00,
  /// Status code for the midi2::ump::data128::sysex8_start message.
  sysex8_start = 0x01,
  /// Status code for the midi2::ump::data128::sysex8_continue message.
  sysex8_continue = 0x02,
  /// Status code for the midi2::ump::data128::sysex8_end message.
  sysex8_end = 0x03,
  /// Status code for the midi2::ump::data128::mds_header message.
  mixed_data_set_header = 0x08,
  /// Status code for the midi2::ump::data128::mds_payload message.
  mixed_data_set_payload = 0x09,
};

}  // end namespace mt

/// \brief Private implementation details of the UMP types and functions
namespace details {

/// \brief Converts one of the status enumerations to the corresponding message_type enumeration value.
///
/// This value is used by the "mt" field in UMP messages. Each specialization provides a static constexpr member
/// `value` which holds the message_type value for the status enumeration \p T.
///
/// \tparam T  One of the status enumerations defined in midi2::ump::mt.
template <typename T>
  requires std::is_enum_v<T>
struct status_to_message_type;
/// \brief Maps the mt::system_crt status enumeration to message_type::system.
template <> struct status_to_message_type<mt::system_crt> {
  /// The message type for messages whose status is given by mt::system_crt.
  static constexpr auto value = message_type::system;
};
/// \brief Maps the mt::utility status enumeration to message_type::utility.
template <> struct status_to_message_type<mt::utility> {
  /// The message type for messages whose status is given by mt::utility.
  static constexpr auto value = message_type::utility;
};
/// \brief Maps the mt::m1cvm status enumeration to message_type::m1cvm.
template <> struct status_to_message_type<mt::m1cvm> {
  /// The message type for messages whose status is given by mt::m1cvm.
  static constexpr auto value = message_type::m1cvm;
};
/// \brief Maps the mt::data64 status enumeration to message_type::data64.
template <> struct status_to_message_type<mt::data64> {
  /// The message type for messages whose status is given by mt::data64.
  static constexpr auto value = message_type::data64;
};
/// \brief Maps the mt::m2cvm status enumeration to message_type::m2cvm.
template <> struct status_to_message_type<mt::m2cvm> {
  /// The message type for messages whose status is given by mt::m2cvm.
  static constexpr auto value = message_type::m2cvm;
};
/// \brief Maps the mt::data128 status enumeration to message_type::data128.
template <> struct status_to_message_type<mt::data128> {
  /// The message type for messages whose status is given by mt::data128.
  static constexpr auto value = message_type::data128;
};
/// \brief Maps the mt::flex_data status enumeration to message_type::flex_data.
template <> struct status_to_message_type<mt::flex_data> {
  /// The message type for messages whose status is given by mt::flex_data.
  static constexpr auto value = message_type::flex_data;
};
/// \brief Maps the mt::stream status enumeration to message_type::stream.
template <> struct status_to_message_type<mt::stream> {
  /// The message type for messages whose status is given by mt::stream.
  static constexpr auto value = message_type::stream;
};

/// A helper variable template for status_to_message_type<>. Holds the message_type value which corresponds to
/// the status enumeration \p T.
/// \tparam T  One of the status enumerations defined in midi2::ump::mt.
template <typename T>
  requires std::is_enum_v<T>
constexpr message_type status_to_message_type_v = status_to_message_type<T>::value;

/// \brief The base class for each of the 32-bit words that make up a UMP message.
///
/// A word is a 32-bit bit_field. The classes derived from word_base declare the fields of the word as nested
/// adt::bit_range<> types.
class word_base : public adt::bit_field<std::uint32_t> {
public:
  using bit_field::bit_field;

  /// Compares two words for equality.
  /// \param a  The first word to be compared.
  /// \param b  The second word to be compared.
  /// \returns True if the two words hold the same value, false otherwise.
  friend constexpr bool operator==(word_base const& a, word_base const& b) noexcept = default;

  /// Sets the field \p Field to the value of the enumeration \p v. If the enumeration's underlying type is
  /// signed, the value is stored in two's complement form.
  ///
  /// \tparam Field  The adt::bit_range<> which describes the field to be set.
  /// \tparam Enumeration  The type of the enumeration.
  /// \param v  The value to be stored.
  /// \returns *this
  template <adt::bit_range_type Field, typename Enumeration>
    requires(std::is_enum_v<Enumeration>)
  constexpr auto& set_enum_field(Enumeration const v) noexcept {
    if constexpr (std::is_unsigned_v<std::underlying_type_t<Enumeration>>) {
      this->template set<Field>(std::to_underlying(v));
    } else {
      this->template set_signed<Field>(std::to_underlying(v));
    }
    return *this;
  }
  /// Returns the raw value of the field \p Field. The value is sign-extended if the enumeration's underlying
  /// type is signed. The value is not checked against the members of the enumeration.
  ///
  /// \tparam Field  The adt::bit_range<> which describes the field to be read.
  /// \tparam Enumeration  The type of the enumeration.
  /// \returns The raw value of the field.
  template <adt::bit_range_type Field, typename Enumeration>
    requires(std::is_enum_v<Enumeration>)
  [[nodiscard]] constexpr auto get_enum_field_raw() const noexcept {
    if constexpr (std::is_unsigned_v<std::underlying_type_t<Enumeration>>) {
      return this->template get<Field>();
    } else {
      return this->template get_signed<Field>();
    }
  }
  /// Returns the value of the field \p Field cast to the enumeration type \p Enumeration.
  ///
  /// \tparam Field  The adt::bit_range<> which describes the field to be read.
  /// \tparam Enumeration  The type of the enumeration.
  /// \returns The value of the field as an Enumeration.
  template <adt::bit_range_type Field, typename Enumeration>
    requires(std::is_enum_v<Enumeration>)
  [[nodiscard]] constexpr Enumeration get_enum_field() const noexcept {
    return static_cast<Enumeration>(this->get_enum_field_raw<Field, Enumeration>());
  }

protected:
  /// Initializes the message type and status fields of the word.
  ///
  /// \tparam MtField  The adt::bit_range<> which describes the mt (message type) field.
  /// \tparam StatusField  The adt::bit_range<> which describes the status field.
  /// \tparam StatusType  One of the status enumerations defined in midi2::ump::mt.
  /// \param status  The status value to be stored. The message type is derived from its type.
  template <adt::bit_range_type MtField, adt::bit_range_type StatusField, typename StatusType>
    requires std::is_enum_v<StatusType>
  constexpr void init(StatusType const status) noexcept {
    this->set<MtField>(std::to_underlying(status_to_message_type_v<StatusType>));
    this->set<StatusField>(std::to_underlying(status));
  }

  /// \tparam StatusType  One of the status enumerations defined in midi2::ump::mt.
  /// \returns The integer value of the message type which corresponds to the status enumeration type.
  template <typename StatusType>
    requires std::is_enum_v<StatusType>
  static constexpr auto underlying_mt(StatusType const /*s*/) {
    return std::to_underlying(ump::details::status_to_message_type_v<std::remove_const_t<StatusType>>);
  }

  /// Checks that the message type and status fields of the word match \p status.
  ///
  /// \tparam MtField  The adt::bit_range<> which describes the mt (message type) field.
  /// \tparam StatusField  The adt::bit_range<> which describes the status field.
  /// \tparam StatusType  One of the status enumerations defined in midi2::ump::mt.
  /// \param status  The expected status value.
  /// \returns True if the mt and status fields hold the expected values, false otherwise.
  template <adt::bit_range_type MtField, adt::bit_range_type StatusField, typename StatusType>
    requires std::is_enum_v<StatusType>
  [[nodiscard]] constexpr bool check(StatusType const status) const noexcept {
    return this->get<MtField>() == underlying_mt(status) && this->get<StatusField>() == std::to_underlying(status);
  }
};

}  // end namespace details

/// Calls the supplied function for each of the words in a UMP message of type \p T.
///
/// Words are processed in order, starting from index 0. If the function returns a value which converts to
/// boolean true, processing stops and that value is returned. Otherwise, processing continues until all words
/// have been processed.
///
/// \tparam T  A message type. One of the types defined in the midi2::ump namespace.
/// \tparam Function  A function accepting a std::uint32_t message word. Returns a value which can be cast to
///   bool.
/// \tparam Index  The index at which processing should start.
/// \param message   An instance of one of the UMP message types.
/// \param function  The function to be called for each member of the UMP message type.
/// \returns  The result of the calling \p function which was converted to boolean true, or the
///   last result received.
template <typename T, typename Function, std::size_t Index = 0>
  requires(Index < std::tuple_size_v<T> &&
           std::derived_from<std::remove_cvref_t<decltype(get<Index>(T{}))>, details::word_base> &&
           std::is_constructible_v<bool, std::invoke_result_t<Function, std::uint32_t>>)
constexpr std::invoke_result_t<Function, std::uint32_t> apply(T const& message, Function function) {
  auto result = function(static_cast<std::uint32_t>(get<Index>(message)));
  if (!bool{result}) {
    if constexpr (Index + 1 < std::tuple_size_v<T>) {
      result = apply<T, Function, Index + 1>(message, std::move(function));
    }
  }
  return result;
}

/// Calls the check() method of each word of a UMP message of type \p T.
///
/// This can be used to validate the internal consistency of a complete UMP message.
///
/// \tparam T  A message type. One of the types defined in the midi2::ump namespace.
/// \tparam Index  The index at which processing should start.
/// \param message   An instance of one of the UMP message types.
/// \returns  True if each of the check() functions returns true, false otherwise.
template <typename T, std::size_t Index = 0>
  requires(Index < std::tuple_size_v<T> &&
           std::derived_from<std::remove_cvref_t<decltype(get<0>(T{}))>, details::word_base>)
constexpr bool check(T const& message) {
  if (!get<Index>(message).check()) {
    return false;
  }
  if constexpr (Index + 1 >= std::tuple_size_v<T>) {
    return true;
  } else {
    return check<T, Index + 1>(message);
  }
}

}  // end namespace midi2::ump

/// Creates a "getter" member function for a field of one of the words of a UMP message class. The function has
/// the same name as the field.
/// \param word  The name of the word class (for example, word0) in which the field is declared.
/// \param field  The name of the field.
#define MIDI2_UMP_GETTER(word, field)                                   \
  constexpr auto field() const noexcept {                               \
    return std::get<word>(words_).template get<typename word::field>(); \
  }
/// Creates two "getter" member functions for an enumerated field of one of the words of a UMP message class. The
/// first, field_raw(), returns the raw integer value of the field; the second, field(), returns the value cast to
/// the enumeration type.
/// \param word  The name of the word class (for example, word0) in which the field is declared.
/// \param field  The name of the field.
/// \param enumeration  The enumeration type of the field.
#define MIDI2_UMP_GETTER_ENUM(word, field, enumeration)                                             \
  constexpr auto field##_raw() const noexcept {                                                     \
    return std::get<word>(words_).template get_enum_field_raw<typename word::field, enumeration>(); \
  }                                                                                                 \
  constexpr enumeration field() const noexcept {                                                    \
    return std::get<word>(words_).template get_enum_field<typename word::field, enumeration>();     \
  }

/// Creates a "setter" member function for a field of one of the words of a UMP message class. The function has
/// the same name as the field and returns a reference to the message object.
/// \param word  The name of the word class (for example, word0) in which the field is declared.
/// \param field  The name of the field.
/// \param result_type  The type of the message class.
#define MIDI2_UMP_SETTER(word, field, result_type)                                           \
  constexpr result_type& field(adt::uinteger_t<word::field::bits::value> const v) noexcept { \
    std::get<word>(words_).template set<typename word::field>(v);                            \
    return *this;                                                                            \
  }
/// Creates a "setter" member function for an enumerated field of one of the words of a UMP message class. The
/// function has the same name as the field and returns a reference to the message object.
/// \param word  The name of the word class (for example, word0) in which the field is declared.
/// \param field  The name of the field.
/// \param enumeration  The enumeration type of the field.
/// \param result_type  The type of the message class.
#define MIDI2_UMP_SETTER_ENUM(word, field, enumeration, result_type)                      \
  constexpr result_type& field(enum enumeration const v) noexcept {                       \
    std::get<word>(words_).template set_enum_field<typename word::field, enumeration>(v); \
    return *this;                                                                         \
  }

/// Creates both "getter" and "setter" member functions for a field of one of the words of a UMP message class.
/// \param word  The name of the word class (for example, word0) in which the field is declared.
/// \param field  The name of the field.
/// \param result_type  The type of the message class.
#define MIDI2_UMP_GETTER_SETTER(word, field, result_type) \
  MIDI2_UMP_GETTER(word, field)                           \
  MIDI2_UMP_SETTER(word, field, result_type)
/// Creates both "getter" and "setter" member functions for an enumerated field of one of the words of a UMP
/// message class.
/// \param word  The name of the word class (for example, word0) in which the field is declared.
/// \param field  The name of the field.
/// \param enumeration  The enumeration type of the field.
/// \param result_type  The type of the message class.
#define MIDI2_UMP_GETTER_SETTER_ENUM(word, field, enumeration, result_type) \
  MIDI2_UMP_GETTER_ENUM(word, field, enumeration)                           \
  MIDI2_UMP_SETTER_ENUM(word, field, enumeration, result_type)

/// This macro is used to generate boilerplate definitions of three items for the class specified by the \p group
/// and \p message parameters:
/// 1. A specialization of `std::tuple_size<>`
/// 2. A specialization of `std::tuple_element<>`
/// 3. A static assertion that the tuple size for the class matches that of the group as a whole
#define MIDI2_UMP_TUPLE(group, message)                                                                              \
  template <>                                                                                                        \
  struct std::tuple_size<::midi2::ump::group::message> /* NOLINT(cert-dcl58-cpp] */                                  \
      : std::integral_constant<std::size_t, std::tuple_size_v<decltype(::midi2::ump::group::message::words_)>> {};   \
  template <std::size_t I> struct std::tuple_element<I, ::midi2::ump::group::message> { /* NOLINT(cert-dcl58-cpp] */ \
    using type = std::tuple_element_t<I, decltype(::midi2::ump::group::message::words_)>;                            \
  };                                                                                                                 \
  static_assert(std::tuple_size_v<::midi2::ump::group::message> ==                                                   \
                ::midi2::ump::message_size<::midi2::ump::message_type::group>::value);

/// Defines a constructor for the class specified by the \p group and \p message parameters which takes a span of
/// std::uint32_t with a size equal to the number of words in the message. The constructor initialises the message from
/// the span and, in a debug build, asserts that the first word of the message is valid (that is, the message type
/// and status fields are correct).
#define MIDI2_SPAN_CTOR(group, message)                                            \
  constexpr ::midi2::ump::group::message::message(                                 \
      ::std::span<::std::uint32_t, ::std::tuple_size_v<message>> const m) noexcept \
      : words_{span_to_tuple(m)} {                                                 \
    assert(get<0>(words_).check());                                                \
  }

namespace midi2::ump {

/// \brief An integral constant which holds the number of 32-bit words in a UMP message of a specific message
///   type.
template <message_type> struct message_size;  // not defined

}  // end namespace midi2::ump

//*       _   _ _ _ _         *
//*  _  _| |_(_) (_) |_ _  _  *
//* | || |  _| | | |  _| || | *
//*  \_,_|\__|_|_|_|\__|\_, | *
//*                     |__/  *
/// \brief An integral constant which holds the number of 32-bit words in a UMP utility message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::utility> : std::integral_constant<unsigned, 1> {};

/// Defines the C++ types that represent Utility type messages
namespace midi2::ump::utility {

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return std::get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return std::get<I>(t.words_);
}

class noop;
class jr_clock;
class jr_timestamp;
class delta_clockstamp_tpqn;
class delta_clockstamp;

}  // end namespace midi2::ump::utility

// F.1.1 Message Type 0x0: Utility
// Table 26 4-Byte UMP Formats for Message Type 0x0: Utility

/// \brief The NOOP message (section 7.2.1)
class midi2::ump::utility::noop {
public:
  /// The word of a NOOP message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::utility::noop;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept {
      return this->word_base::check<mt, status>(message) && this->get<reserved>() == 0;
    }

    /// Defines the bit position of the mt (message-type) field. Always 0x0 (message_type::utility).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the status field. Always 0x0 (mt::utility::noop).
    using status = adt::bit_range<20, 4>;

    /// Defines a group of reserved bits.
    using reserved = adt::bit_range<0, 20>;
  };

  /// Default constructs a NOOP message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr noop() noexcept = default;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(noop const&, noop const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x0 (message_type::utility).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \brief Returns the value of the word0::status field. Always 0x0 (mt::utility::noop).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<noop>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::utility::noop>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::utility::noop
//
/// \class std::tuple_element<I, ::midi2::ump::utility::noop>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::utility::noop
MIDI2_UMP_TUPLE(utility, noop)  // Define tuple_size and tuple_element for noop

template <> struct std::tuple_size<::midi2::ump::utility::jr_clock>;

// 7.2.2.1 JR Clock Message
/// \brief The JR Clock message (section 7.2.2.1)
class midi2::ump::utility::jr_clock {
public:
  /// The first word of a Jitter Reduction Clock message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::utility::jr_clock;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x0 (message_type::utility).
    using mt = adt::bit_range<28, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x1 (mt::utility::jr_clock).
    using status = adt::bit_range<20, 4>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<16, 4>;
    /// Defines the bit position of the sender clock time field. A 16-bit time value in clock ticks of 1/31250 of one
    /// second (32 µsec, clock frequency of 1 MHz / 32).
    using sender_clock_time = adt::bit_range<0, 16>;
  };

  /// Default constructs a JR Clock message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr jr_clock() noexcept = default;
  /// Constructs a JR Clock message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit jr_clock(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(jr_clock const&, jr_clock const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x0 (message_type::utility).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \brief Returns the value of the word0::status field. Always 0x1 (mt::utility::jr_clock).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::utility::jr_clock::sender_clock_time() const noexcept
  /// \brief Returns the value of the word0::sender_clock_time field.
  ///
  /// The word0::sender_clock_time field contains a 16-bit time in ticks of 1/31250 of a second (32 µsec, clock
  /// frequency of 1 MHz / 32).
  /// \returns The value of the word0::sender_clock_time field.
  //
  /// \fn constexpr ump::utility::jr_clock & midi2::ump::utility::jr_clock::sender_clock_time(adt::uinteger_t<word0::sender_clock_time::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::sender_clock_time field.
  /// \param v  The value to be assigned to the word0::sender_clock_time field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, sender_clock_time, ump::utility::jr_clock)

private:
  friend struct ::std::tuple_size<jr_clock>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::utility::jr_clock>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::utility::jr_clock
//
/// \class std::tuple_element<I, ::midi2::ump::utility::jr_clock>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::utility::jr_clock
MIDI2_UMP_TUPLE(utility, jr_clock)  // Define tuple_size and tuple_element for jr_clock
MIDI2_SPAN_CTOR(utility, jr_clock)  // Define the span constructor for jr_clock

// 7.2.2.2 Jitter-Reduction Timestamp Message
/// \brief The Jitter-Reduction Timestamp message (section 7.2.2.2)
class midi2::ump::utility::jr_timestamp {
public:
  /// The first word of a Jitter-Reduction Timestamp message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::utility::jr_ts;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x0 (message_type::utility).
    using mt = adt::bit_range<28, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x2 (mt::utility::jr_ts).
    using status = adt::bit_range<20, 4>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<16, 4>;
    /// Defines the bit position of the sender clock timestamp field. A 16-bit time value in clock ticks of 1/31250 of
    /// one second (32 µsec, clock frequency of 1 MHz / 32).
    using timestamp = adt::bit_range<0, 16>;
  };

  /// Default constructs a JR Timestamp message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr jr_timestamp() noexcept = default;
  /// Constructs a JR Timestamp message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit jr_timestamp(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(jr_timestamp const&, jr_timestamp const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x0 (message_type::utility).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \brief Returns the value of the word0::status field. Always 0x2 (mt::utility::jr_ts).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

  /// \fn constexpr auto midi2::ump::utility::jr_timestamp::timestamp() const noexcept
  /// \brief Returns the value of the word0::timestamp field.
  ///
  /// The word0::timestamp field contains a 16-bit time in ticks of 1/31250 of a second (32 µsec, clock frequency of 1
  /// MHz / 32).
  /// \returns The value of the word0::timestamp field.
  //
  /// \fn constexpr ump::utility::jr_timestamp & midi2::ump::utility::jr_timestamp::timestamp(adt::uinteger_t<word0::timestamp::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::timestamp field.
  /// \param v  The value to be assigned to the word0::timestamp field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, timestamp, ump::utility::jr_timestamp)

private:
  friend struct ::std::tuple_size<jr_timestamp>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::utility::jr_timestamp>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::utility::jr_timestamp
//
/// \class std::tuple_element<I, ::midi2::ump::utility::jr_timestamp>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::utility::jr_timestamp
MIDI2_UMP_TUPLE(utility, jr_timestamp)  // Define tuple_size and tuple_element for jr_timestamp
MIDI2_SPAN_CTOR(utility, jr_timestamp)  // Define the span constructor for jr_timestamp

// 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (TPQN)
/// \brief The Delta Clockstamp Ticks Per Quarter Note (TPQN) message
class midi2::ump::utility::delta_clockstamp_tpqn {
public:
  /// \brief The first word of a Delta Clockstamp TPQN message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::utility::delta_clock_tick;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x0 (message_type::utility).
    using mt = adt::bit_range<28, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x3 (mt::utility::delta_clock_tick).
    using status = adt::bit_range<20, 4>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<16, 4>;
    /// Defines the bit position of the number of ticks per quarter note field. This declares the unit of measure used
    /// by Delta Clockstamp messages in a MIDI Clip File. It may have a value of 1-65535 (0 is reserved).
    using ticks_pqn = adt::bit_range<0, 16>;
  };

  /// Default constructs a Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message. The fields which identify the
  /// message are initialized; all other fields are zero.
  constexpr delta_clockstamp_tpqn() noexcept = default;
  /// Constructs a Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit delta_clockstamp_tpqn(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(delta_clockstamp_tpqn const&, delta_clockstamp_tpqn const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x0 (message_type::utility).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \brief Returns the value of the word0::status field. Always 0x3 (mt::utility::delta_clock_tick).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::utility::delta_clockstamp_tpqn::ticks_pqn() const noexcept
  /// \brief Returns the value of the word0::ticks_pqn field.
  ///
  /// The word0::ticks_pqn field declares the unit of measure used by Delta Clockstamp messages in a MIDI Clip File. It
  /// may have a value of 1 – 65,535 (0 = Reserved).
  /// \returns The value of the word0::ticks_pqn field.
  //
  /// \fn constexpr ump::utility::delta_clockstamp_tpqn & midi2::ump::utility::delta_clockstamp_tpqn::ticks_pqn(adt::uinteger_t<word0::ticks_pqn::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::ticks_pqn field.
  /// \param v  The value to be assigned to the word0::ticks_pqn field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, ticks_pqn, ump::utility::delta_clockstamp_tpqn)

private:
  friend struct ::std::tuple_size<delta_clockstamp_tpqn>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::utility::delta_clockstamp_tpqn>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::utility::delta_clockstamp_tpqn
//
/// \class std::tuple_element<I, ::midi2::ump::utility::delta_clockstamp_tpqn>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::utility::delta_clockstamp_tpqn
MIDI2_UMP_TUPLE(utility, delta_clockstamp_tpqn)  // Define tuple_size and tuple_element for delta_clockstamp_tpqn
MIDI2_SPAN_CTOR(utility, delta_clockstamp_tpqn)  // Define the span constructor for delta_clockstamp_tpqn

/// \brief The Delta Clockstamp (DC): Ticks Since Last Event message
///
/// The Delta Clockstamp message declares the time of all following messages which occur before the next Delta
/// Clockstamp message.
class midi2::ump::utility::delta_clockstamp {
public:
  /// \brief The first word of a Delta Clockstamp Since Last Event message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::utility::delta_clock_since;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x0 (message_type::utility).
    using mt = adt::bit_range<28, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x4 (mt::utility::delta_clock_since).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the number of ticks since the last event field. The unit of measure is set by the
    /// most recent Delta Clockstamp Ticks Per Quarter Note message.
    using num_ticks = adt::bit_range<0, 20>;
  };

  /// Default constructs a Delta Clockstamp (DC): Ticks Since Last Event message. The fields which identify the message
  /// are initialized; all other fields are zero.
  constexpr delta_clockstamp() noexcept = default;
  /// Constructs a Delta Clockstamp (DC): Ticks Since Last Event message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit delta_clockstamp(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(delta_clockstamp const&, delta_clockstamp const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x0 (message_type::utility).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \brief Returns the value of the word0::status field. Always 0x4 (mt::utility::delta_clock_since).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::utility::delta_clockstamp::num_ticks() const noexcept
  /// \brief Returns the value of the word0::num_ticks field.
  /// \returns The value of the word0::num_ticks field.
  //
  /// \fn constexpr ump::utility::delta_clockstamp & midi2::ump::utility::delta_clockstamp::num_ticks(adt::uinteger_t<word0::num_ticks::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::num_ticks field.
  /// \param v  The value to be assigned to the word0::num_ticks field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, num_ticks, ump::utility::delta_clockstamp)

private:
  friend struct ::std::tuple_size<delta_clockstamp>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::utility::delta_clockstamp>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::utility::delta_clockstamp
//
/// \class std::tuple_element<I, ::midi2::ump::utility::delta_clockstamp>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::utility::delta_clockstamp
MIDI2_UMP_TUPLE(utility, delta_clockstamp)  // Define tuple_size and tuple_element for delta_clockstamp
MIDI2_SPAN_CTOR(utility, delta_clockstamp)  // Define the span constructor for delta_clockstamp

//*             _              *
//*  ____  _ __| |_ ___ _ __   *
//* (_-< || (_-<  _/ -_) '  \  *
//* /__/\_, /__/\__\___|_|_|_| *
//*     |__/                   *
// 7.6 System Common and System Real Time Messages

/// \brief An integral constant which holds the number of 32-bit words in a UMP system message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::system> : std::integral_constant<unsigned, 1> {};

/// Defines the C++ types that represent System type messages
namespace midi2::ump::system {

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return get<I>(t.words_);
}

class midi_time_code;
class song_position_pointer;
class song_select;
class tune_request;
class timing_clock;
class sequence_start;
class sequence_continue;
class sequence_stop;
class active_sensing;
class reset;

}  // namespace midi2::ump::system

/// \brief  The contents of the MIDI time code message
class midi2::ump::system::midi_time_code {
public:
  /// \brief The first and only word of the midi_time_code message
  class word0 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = mt::system_crt::timing_code;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(ump::mt::system_crt::timing_code); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28U, 4U>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24U, 4U>;
    /// Defines the bit position of the status field. Always 0xF1 (mt::system_crt::timing_code).
    using status = adt::bit_range<16U, 8U>;
    /// Defines a reserved bit
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the time code field. This is the 7-bit data byte of a MIDI 1.0 quarter-frame
    /// message: a 3-bit message type (bits 4-6) and a 4-bit value (bits 0-3).
    using time_code = adt::bit_range<8, 7>;
    /// Defines a group of reserved bits
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines a group of reserved bits
    using reserved2 = adt::bit_range<0, 7>;
  };

  /// Default constructs a MIDI Time Code message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr midi_time_code() noexcept = default;
  /// Constructs a MIDI Time Code message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit midi_time_code(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(midi_time_code const&, midi_time_code const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)

  /// \fn constexpr auto midi2::ump::system::midi_time_code::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::midi_time_code & midi2::ump::system::midi_time_code::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::midi_time_code)
  /// \brief Returns the value of the word0::status field. Always 0xF1 (mt::system_crt::timing_code).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::system::midi_time_code::time_code() const noexcept
  /// \brief Returns the value of the word0::time_code field.
  /// \returns The value of the word0::time_code field.
  //
  /// \fn constexpr ump::system::midi_time_code & midi2::ump::system::midi_time_code::time_code(adt::uinteger_t<word0::time_code::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::time_code field.
  /// \param v  The value to be assigned to the word0::time_code field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, time_code, ump::system::midi_time_code)

private:
  friend struct ::std::tuple_size<midi_time_code>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::midi_time_code>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::midi_time_code
//
/// \class std::tuple_element<I, ::midi2::ump::system::midi_time_code>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::midi_time_code
MIDI2_UMP_TUPLE(system, midi_time_code)  // Define tuple_size and tuple_element for midi_time_code
MIDI2_SPAN_CTOR(system, midi_time_code)  // Define the span constructor for midi_time_code

/// \brief The contents of the MIDI song position pointer message
class midi2::ump::system::song_position_pointer {
public:
  /// \brief The first word of the Song Position Pointer message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = mt::system_crt::spp;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(ump::mt::system_crt::spp); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xF2 (mt::system_crt::spp).
    using status = adt::bit_range<16, 8>;
    /// Defines a reserved bit
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the least significant 7 bits of the 14-bit song position (the number of MIDI beats,
    /// each of six MIDI clocks, since the start of the song).
    using position_lsb = adt::bit_range<8, 7>;
    /// Defines a reserved bit
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the most significant 7 bits of the 14-bit song position (the number of MIDI beats,
    /// each of six MIDI clocks, since the start of the song).
    using position_msb = adt::bit_range<0, 7>;
  };

  /// Default constructs a Song Position Pointer message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr song_position_pointer() noexcept = default;
  /// Constructs a Song Position Pointer message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit song_position_pointer(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(song_position_pointer const&, song_position_pointer const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::song_position_pointer::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::song_position_pointer & midi2::ump::system::song_position_pointer::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::song_position_pointer)
  /// \brief Returns the value of the word0::status field. Always 0xF2 (mt::system_crt::spp).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::system::song_position_pointer::position_lsb() const noexcept
  /// \brief Returns the value of the word0::position_lsb field.
  /// \returns The value of the word0::position_lsb field.
  //
  /// \fn constexpr ump::system::song_position_pointer & midi2::ump::system::song_position_pointer::position_lsb(adt::uinteger_t<word0::position_lsb::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::position_lsb field.
  /// \param v  The value to be assigned to the word0::position_lsb field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, position_lsb, ump::system::song_position_pointer)
  /// \fn constexpr auto midi2::ump::system::song_position_pointer::position_msb() const noexcept
  /// \brief Returns the value of the word0::position_msb field.
  /// \returns The value of the word0::position_msb field.
  //
  /// \fn constexpr ump::system::song_position_pointer & midi2::ump::system::song_position_pointer::position_msb(adt::uinteger_t<word0::position_msb::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::position_msb field.
  /// \param v  The value to be assigned to the word0::position_msb field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, position_msb, ump::system::song_position_pointer)

private:
  friend struct ::std::tuple_size<song_position_pointer>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::song_position_pointer>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::song_position_pointer
//
/// \class std::tuple_element<I, ::midi2::ump::system::song_position_pointer>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::song_position_pointer
MIDI2_UMP_TUPLE(system, song_position_pointer)  // Define tuple_size and tuple_element for song_position_pointer
MIDI2_SPAN_CTOR(system, song_position_pointer)  // Define the span constructor for song_position_pointer

/// \brief The contents of the MIDI song select message
class midi2::ump::system::song_select {
public:
  /// \brief The first and only word of the MIDI song select message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::system_crt::song_select;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xF3 (mt::system_crt::song_select).
    using status = adt::bit_range<16, 8>;
    /// Defines a reserved bit
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit song number field.
    using song = adt::bit_range<8, 7>;
    /// Defines a reserved bit
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines a group of reserved bits
    using reserved2 = adt::bit_range<0, 7>;
  };

  /// Default constructs a Song Select message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr song_select() noexcept = default;
  /// Constructs a Song Select message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit song_select(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(song_select const&, song_select const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::song_select::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::song_select & midi2::ump::system::song_select::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::song_select)
  /// \brief Returns the value of the word0::status field. Always 0xF3 (mt::system_crt::song_select).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::system::song_select::song() const noexcept
  /// \brief Returns the value of the word0::song field.
  /// \returns The value of the word0::song field.
  //
  /// \fn constexpr ump::system::song_select & midi2::ump::system::song_select::song(adt::uinteger_t<word0::song::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::song field.
  /// \param v  The value to be assigned to the word0::song field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, song, ump::system::song_select)

private:
  friend struct ::std::tuple_size<song_select>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::song_select>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::song_select
//
/// \class std::tuple_element<I, ::midi2::ump::system::song_select>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::song_select
MIDI2_UMP_TUPLE(system, song_select)  // Define tuple_size and tuple_element for song_select
MIDI2_SPAN_CTOR(system, song_select)  // Define the span constructor for song_select

/// \brief The contents of the MIDI tune-request message
class midi2::ump::system::tune_request {
public:
  /// \brief The contents of the first and only word of a MIDI tune-request message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = mt::system_crt::tune_request;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xF6 (mt::system_crt::tune_request).
    using status = adt::bit_range<16, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Tune Request message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr tune_request() noexcept = default;
  /// Constructs a Tune Request message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit tune_request(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(tune_request const&, tune_request const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::tune_request::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::tune_request & midi2::ump::system::tune_request::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::tune_request)
  /// \brief Returns the value of the word0::status field. Always 0xF6 (mt::system_crt::tune_request).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<tune_request>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::tune_request>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::tune_request
//
/// \class std::tuple_element<I, ::midi2::ump::system::tune_request>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::tune_request
MIDI2_UMP_TUPLE(system, tune_request)  // Define tuple_size and tuple_element for tune_request
MIDI2_SPAN_CTOR(system, tune_request)  // Define the span constructor for tune_request

//
/// \brief The Timing Clock message (section 7.6).
class midi2::ump::system::timing_clock {
public:
  /// \brief The first and only word of the MIDI timing-clock message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = mt::system_crt::timing_clock;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xF8 (mt::system_crt::timing_clock).
    using status = adt::bit_range<16, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Timing Clock message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr timing_clock() noexcept = default;
  /// Constructs a Timing Clock message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit timing_clock(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(timing_clock const&, timing_clock const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::timing_clock::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::timing_clock & midi2::ump::system::timing_clock::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::timing_clock)
  /// \brief Returns the value of the word0::status field. Always 0xF8 (mt::system_crt::timing_clock).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<timing_clock>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::timing_clock>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::timing_clock
//
/// \class std::tuple_element<I, ::midi2::ump::system::timing_clock>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::timing_clock
MIDI2_UMP_TUPLE(system, timing_clock)  // Define tuple_size and tuple_element for timing_clock
MIDI2_SPAN_CTOR(system, timing_clock)  // Define the span constructor for timing_clock

/// \brief Defines the MIDI sequence-start message
class midi2::ump::system::sequence_start {
public:
  /// \brief The contents of the first and only word of a MIDI sequence-start message
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = mt::system_crt::sequence_start;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xFA (mt::system_crt::sequence_start).
    using status = adt::bit_range<16, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Start message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr sequence_start() noexcept = default;
  /// Constructs a Start message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit sequence_start(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(sequence_start const&, sequence_start const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::sequence_start::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::sequence_start & midi2::ump::system::sequence_start::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::sequence_start)
  /// \brief Returns the value of the word0::status field. Always 0xFA (mt::system_crt::sequence_start).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<sequence_start>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::sequence_start>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::sequence_start
//
/// \class std::tuple_element<I, ::midi2::ump::system::sequence_start>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::sequence_start
MIDI2_UMP_TUPLE(system, sequence_start)  // Define tuple_size and tuple_element for sequence_start
MIDI2_SPAN_CTOR(system, sequence_start)  // Define the span constructor for sequence_start

/// \brief Defines the MIDI sequence-continue message
class midi2::ump::system::sequence_continue {
public:
  /// \brief The contents of the first and only word of a MIDI sequence-continue message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::system_crt::sequence_continue;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xFB (mt::system_crt::sequence_continue).
    using status = adt::bit_range<16, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Continue message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr sequence_continue() noexcept = default;
  /// Constructs a Continue message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit sequence_continue(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(sequence_continue const&, sequence_continue const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::sequence_continue::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::sequence_continue & midi2::ump::system::sequence_continue::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::sequence_continue)
  /// \brief Returns the value of the word0::status field. Always 0xFB (mt::system_crt::sequence_continue).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<sequence_continue>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::sequence_continue>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::sequence_continue
//
/// \class std::tuple_element<I, ::midi2::ump::system::sequence_continue>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::sequence_continue
MIDI2_UMP_TUPLE(system, sequence_continue)  // Define tuple_size and tuple_element for sequence_continue
MIDI2_SPAN_CTOR(system, sequence_continue)  // Define the span constructor for sequence_continue

/// \brief Defines the MIDI sequence-stop message
class midi2::ump::system::sequence_stop {
public:
  /// \brief The first word of the Stop message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::system_crt::sequence_stop;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xFC (mt::system_crt::sequence_stop).
    using status = adt::bit_range<16, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Stop message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr sequence_stop() noexcept = default;
  /// Constructs a Stop message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit sequence_stop(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(sequence_stop const&, sequence_stop const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::sequence_stop::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::sequence_stop & midi2::ump::system::sequence_stop::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::sequence_stop)
  /// \brief Returns the value of the word0::status field. Always 0xFC (mt::system_crt::sequence_stop).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<sequence_stop>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::sequence_stop>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::sequence_stop
//
/// \class std::tuple_element<I, ::midi2::ump::system::sequence_stop>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::sequence_stop
MIDI2_UMP_TUPLE(system, sequence_stop)  // Define tuple_size and tuple_element for sequence_stop
MIDI2_SPAN_CTOR(system, sequence_stop)  // Define the span constructor for sequence_stop

/// \brief Defines the MIDI active-sensing message
class midi2::ump::system::active_sensing {
public:
  /// \brief The first word of the Active Sensing message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::system_crt::active_sensing;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xFE (mt::system_crt::active_sensing).
    using status = adt::bit_range<16, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Active Sensing message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr active_sensing() noexcept = default;
  /// Constructs a Active Sensing message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit active_sensing(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(active_sensing const&, active_sensing const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::active_sensing::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::active_sensing & midi2::ump::system::active_sensing::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::active_sensing)
  /// \brief Returns the value of the word0::status field. Always 0xFE (mt::system_crt::active_sensing).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<active_sensing>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::active_sensing>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::active_sensing
//
/// \class std::tuple_element<I, ::midi2::ump::system::active_sensing>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::active_sensing
MIDI2_UMP_TUPLE(system, active_sensing)  // Define tuple_size and tuple_element for active_sensing
MIDI2_SPAN_CTOR(system, active_sensing)  // Define the span constructor for active_sensing

/// \brief Defines the MIDI reset message
class midi2::ump::system::reset {
public:
  /// \brief The first word of the System Reset message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::system_crt::system_reset;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x1 (message_type::system).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xFF (mt::system_crt::system_reset).
    using status = adt::bit_range<16, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a System Reset message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr reset() noexcept = default;
  /// Constructs a System Reset message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit reset(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(reset const&, reset const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x1 (message_type::system).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::system::reset::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::system::reset & midi2::ump::system::reset::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::system::reset)
  /// \brief Returns the value of the word0::status field. Always 0xFF (mt::system_crt::system_reset).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)

private:
  friend struct ::std::tuple_size<reset>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::system::reset>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::system::reset
//
/// \class std::tuple_element<I, ::midi2::ump::system::reset>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::system::reset
MIDI2_UMP_TUPLE(system, reset)  // Define tuple_size and tuple_element for system/reset
MIDI2_SPAN_CTOR(system, reset)  // Define the span constructor for system/reset

//*        _                 *
//*  _ __ / |  ____ ___ __   *
//* | '  \| | / _\ V / '  \  *
//* |_|_|_|_| \__|\_/|_|_|_| *
//*                          *
// F.1.3 Mess Type 0x2: MIDI 1.0 Channel Voice Messages
// Table 28 4-Byte UMP Formats for Message Type 0x2: MIDI 1.0 Channel Voice
// Messages

/// \brief An integral constant which holds the number of 32-bit words in a UMP m1cvm message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::m1cvm> : std::integral_constant<unsigned, 1> {};

/// Defines the C++ types that represent MIDI 1.0 Channel Voice type messages
namespace midi2::ump::m1cvm {

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return get<I>(t.words_);
}

class note_on;
class note_off;
class poly_pressure;
class control_change;
class program_change;
class channel_pressure;
class pitch_bend;

}  // end namespace midi2::ump::m1cvm

/// Defines the MIDI 1.0 Note On message.
class midi2::ump::m1cvm::note_on {
public:
  /// \brief The first and only word of a note_on message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::m1cvm::note_on;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (message_type::m1cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x9 (mt::m1cvm::note_on).
    using status = adt::bit_range<20, 4>;
    /// \brief Defines the bit position of the channel field.
    using channel = adt::bit_range<16, 4>;
    /// \brief Defines the bit position of the first reserved field.
    using reserved0 = adt::bit_range<15, 1>;
    /// \brief Defines the bit position of the note number field.
    using note = adt::bit_range<8, 7>;
    /// \brief Defines the bit position of the second reserved field.
    using reserved1 = adt::bit_range<7, 1>;
    /// \brief Defines the bit position of the velocity field.
    using velocity = adt::bit_range<0, 7>;
  };

  /// Default constructs a MIDI 1.0 Note On message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr note_on() noexcept = default;
  /// Constructs a MIDI 1.0 Note On message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit note_on(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(note_on const&, note_on const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x2 (message_type::m1cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m1cvm::note_on::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m1cvm::note_on & midi2::ump::m1cvm::note_on::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m1cvm::note_on)
  /// \brief Returns the value of the word0::status field. Always 0x9 (mt::m1cvm::note_on).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m1cvm::note_on::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m1cvm::note_on & midi2::ump::m1cvm::note_on::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m1cvm::note_on)
  /// \fn constexpr auto midi2::ump::m1cvm::note_on::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m1cvm::note_on & midi2::ump::m1cvm::note_on::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m1cvm::note_on)
  /// \fn constexpr auto midi2::ump::m1cvm::note_on::velocity() const noexcept
  /// \brief Returns the value of the word0::velocity field.
  /// \returns The value of the word0::velocity field.
  //
  /// \fn constexpr ump::m1cvm::note_on & midi2::ump::m1cvm::note_on::velocity(adt::uinteger_t<word0::velocity::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::velocity field.
  /// \param v  The value to be assigned to the word0::velocity field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, velocity, ump::m1cvm::note_on)

private:
  friend struct ::std::tuple_size<note_on>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::m1cvm::note_on>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m1cvm::note_on
//
/// \class std::tuple_element<I, ::midi2::ump::m1cvm::note_on>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m1cvm::note_on
MIDI2_UMP_TUPLE(m1cvm, note_on)  // Define tuple_size and tuple_element for m1cvm/note_on
MIDI2_SPAN_CTOR(m1cvm, note_on)  // Define the span constructor for m1cvm/note_on

// 7.3.1 MIDI 1.0 Note Off Message
/// \brief The MIDI 1.0 Note Off message (section 7.3.1).
class midi2::ump::m1cvm::note_off {
public:
  /// \brief The first and only word of a note_off message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::m1cvm::note_off;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (message_type::m1cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x8 (mt::m1cvm::note_off).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the 7-bit (release) velocity field.
    using velocity = adt::bit_range<0, 7>;
  };

  /// Default constructs a note-off message with all fields set to zero.
  constexpr note_off() noexcept = default;
  /// Constructs a MIDI 1.0 Note Off message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit note_off(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(note_off const&, note_off const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x2 (message_type::m1cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m1cvm::note_off::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m1cvm::note_off & midi2::ump::m1cvm::note_off::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m1cvm::note_off)
  /// \brief Returns the value of the word0::status field. Always 0x8 (mt::m1cvm::note_off).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m1cvm::note_off::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m1cvm::note_off & midi2::ump::m1cvm::note_off::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m1cvm::note_off)
  /// \fn constexpr auto midi2::ump::m1cvm::note_off::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m1cvm::note_off & midi2::ump::m1cvm::note_off::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m1cvm::note_off)
  /// \fn constexpr auto midi2::ump::m1cvm::note_off::velocity() const noexcept
  /// \brief Returns the value of the word0::velocity field.
  /// \returns The value of the word0::velocity field.
  //
  /// \fn constexpr ump::m1cvm::note_off & midi2::ump::m1cvm::note_off::velocity(adt::uinteger_t<word0::velocity::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::velocity field.
  /// \param v  The value to be assigned to the word0::velocity field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, velocity, ump::m1cvm::note_off)

private:
  friend struct ::std::tuple_size<note_off>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::m1cvm::note_off>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m1cvm::note_off
//
/// \class std::tuple_element<I, ::midi2::ump::m1cvm::note_off>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m1cvm::note_off
MIDI2_UMP_TUPLE(m1cvm, note_off)  // Define tuple_size and tuple_element for m1cvm/note_off
MIDI2_SPAN_CTOR(m1cvm, note_off)  // Define the span constructor for m1cvm/note_off

// 7.3.3 MIDI 1.0 Poly Pressure Message
/// \brief The MIDI 1.0 Poly Pressure message (section 7.3.3).
class midi2::ump::m1cvm::poly_pressure {
public:
  /// \brief The first and only word of a poly_pressure message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::m1cvm::poly_pressure;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (message_type::m1cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xA (mt::m1cvm::poly_pressure).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the 7-bit pressure (aftertouch) field.
    using pressure = adt::bit_range<0, 7>;
  };

  /// Default constructs a MIDI 1.0 Poly Pressure message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr poly_pressure() noexcept = default;
  /// Constructs a MIDI 1.0 Poly Pressure message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit poly_pressure(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(poly_pressure const&, poly_pressure const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x2 (message_type::m1cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \brief Returns the value of the word0::status field. Always 0xA (mt::m1cvm::poly_pressure).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m1cvm::poly_pressure::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m1cvm::poly_pressure & midi2::ump::m1cvm::poly_pressure::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m1cvm::poly_pressure)
  /// \fn constexpr auto midi2::ump::m1cvm::poly_pressure::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m1cvm::poly_pressure & midi2::ump::m1cvm::poly_pressure::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m1cvm::poly_pressure)
  /// \fn constexpr auto midi2::ump::m1cvm::poly_pressure::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m1cvm::poly_pressure & midi2::ump::m1cvm::poly_pressure::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m1cvm::poly_pressure)
  /// \fn constexpr auto midi2::ump::m1cvm::poly_pressure::pressure() const noexcept
  /// \brief Returns the value of the word0::pressure field.
  /// \returns The value of the word0::pressure field.
  //
  /// \fn constexpr ump::m1cvm::poly_pressure & midi2::ump::m1cvm::poly_pressure::pressure(adt::uinteger_t<word0::pressure::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::pressure field.
  /// \param v  The value to be assigned to the word0::pressure field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, pressure, ump::m1cvm::poly_pressure)

private:
  friend struct ::std::tuple_size<poly_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::m1cvm::poly_pressure>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m1cvm::poly_pressure
//
/// \class std::tuple_element<I, ::midi2::ump::m1cvm::poly_pressure>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m1cvm::poly_pressure
MIDI2_UMP_TUPLE(m1cvm, poly_pressure)  // Define tuple_size and tuple_element for m1cvm/poly_pressure
MIDI2_SPAN_CTOR(m1cvm, poly_pressure)  // Define the span constructor for m1cvm/poly_pressure

// 7.3.4 MIDI 1.0 Control Change Message
/// \brief The MIDI 1.0 Control Change message (section 7.3.4).
class midi2::ump::m1cvm::control_change {
public:
  /// \brief The first and only word of a control_change message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::m1cvm::cc;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (message_type::m1cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xB (mt::m1cvm::cc).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit controller number (index) field.
    using controller = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the 7-bit controller value field.
    using value = adt::bit_range<0, 7>;
  };

  /// Default constructs a MIDI 1.0 Control Change message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr control_change() noexcept = default;
  /// Constructs a MIDI 1.0 Control Change message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit control_change(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(control_change const&, control_change const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x2 (message_type::m1cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m1cvm::control_change::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m1cvm::control_change & midi2::ump::m1cvm::control_change::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m1cvm::control_change)
  /// \brief Returns the value of the word0::status field. Always 0xB (mt::m1cvm::cc).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m1cvm::control_change::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m1cvm::control_change & midi2::ump::m1cvm::control_change::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m1cvm::control_change)
  /// \fn constexpr auto midi2::ump::m1cvm::control_change::controller() const noexcept
  /// \brief Returns the value of the word0::controller field.
  /// \returns The value of the word0::controller field.
  //
  /// \fn constexpr ump::m1cvm::control_change & midi2::ump::m1cvm::control_change::controller(adt::uinteger_t<word0::controller::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::controller field.
  /// \param v  The value to be assigned to the word0::controller field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, controller, ump::m1cvm::control_change)
  /// \fn constexpr auto midi2::ump::m1cvm::control_change::value() const noexcept
  /// \brief Returns the value of the word0::value field.
  /// \returns The value of the word0::value field.
  //
  /// \fn constexpr ump::m1cvm::control_change & midi2::ump::m1cvm::control_change::value(adt::uinteger_t<word0::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::value field.
  /// \param v  The value to be assigned to the word0::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, value, ump::m1cvm::control_change)

  /// Sets the value of the word0::controller field from a member of the ump::control enumeration.
  /// \param c  The controller number.
  /// \returns *this
  constexpr auto& controller(ump::control const c) noexcept { return this->controller(std::to_underlying(c)); }

private:
  friend struct ::std::tuple_size<control_change>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::m1cvm::control_change>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m1cvm::control_change
//
/// \class std::tuple_element<I, ::midi2::ump::m1cvm::control_change>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m1cvm::control_change
MIDI2_UMP_TUPLE(m1cvm, control_change)  // Define tuple_size and tuple_element for m1cvm/control_change
MIDI2_SPAN_CTOR(m1cvm, control_change)  // Define the span constructor for m1cvm/control_change

// 7.3.5 MIDI 1.0 Program Change Message
/// \brief The MIDI 1.0 Program Change message (section 7.3.5).
class midi2::ump::m1cvm::program_change {
public:
  /// \brief The first and only word of a program_change message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::m1cvm::program_change;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (message_type::m1cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xC (mt::m1cvm::program_change).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit program number field.
    using program = adt::bit_range<8, 7>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a MIDI 1.0 Program Change message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr program_change() noexcept = default;
  /// Constructs a MIDI 1.0 Program Change message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit program_change(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(program_change const&, program_change const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x2 (message_type::m1cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m1cvm::program_change::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m1cvm::program_change & midi2::ump::m1cvm::program_change::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m1cvm::program_change)
  /// \brief Returns the value of the word0::status field. Always 0xC (mt::m1cvm::program_change).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m1cvm::program_change::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m1cvm::program_change & midi2::ump::m1cvm::program_change::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m1cvm::program_change)
  /// \fn constexpr auto midi2::ump::m1cvm::program_change::program() const noexcept
  /// \brief Returns the value of the word0::program field.
  /// \returns The value of the word0::program field.
  //
  /// \fn constexpr ump::m1cvm::program_change & midi2::ump::m1cvm::program_change::program(adt::uinteger_t<word0::program::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::program field.
  /// \param v  The value to be assigned to the word0::program field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, program, ump::m1cvm::program_change)

private:
  friend struct ::std::tuple_size<program_change>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::m1cvm::program_change>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m1cvm::program_change
//
/// \class std::tuple_element<I, ::midi2::ump::m1cvm::program_change>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m1cvm::program_change
MIDI2_UMP_TUPLE(m1cvm, program_change)  // Define tuple_size and tuple_element for m1cvm/program_change
MIDI2_SPAN_CTOR(m1cvm, program_change)  // Define the span constructor for m1cvm/program_change

/// \fn constexpr auto midi2::ump::m1cvm::channel_pressure::status() const noexcept
/// \brief Returns the value of the word0::status field. Always mt::m1cvm::channel_pressure.
/// \note This is a read-only field.
/// \returns The value of the word0::status field. Always mt::m1cvm::channel_pressure.

/// \fn constexpr auto midi2::ump::m1cvm::channel_pressure::mt() const noexcept
/// \brief Returns the value of the word0::status field. Always message_type::m1cvm.
/// \note This is a read-only field.
/// \returns The value of the word0::status field. Always message_type::m1cvm.

/// \brief The MIDI 1.0 Channel Pressure message
class midi2::ump::m1cvm::channel_pressure {
public:
  /// \brief The first and only word of a channel_pressure message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::m1cvm::channel_pressure;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (message_type::m1cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xD (mt::m1cvm::channel_pressure).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit pressure (aftertouch) field.
    using data = adt::bit_range<8, 7>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };

  /// Default constructs a MIDI 1.0 Channel Pressure message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr channel_pressure() noexcept = default;
  /// Constructs a MIDI 1.0 Channel Pressure message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit channel_pressure(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(channel_pressure const&, channel_pressure const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x2 (message_type::m1cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m1cvm::channel_pressure::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m1cvm::channel_pressure & midi2::ump::m1cvm::channel_pressure::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m1cvm::channel_pressure)
  /// \brief Returns the value of the word0::status field. Always 0xD (mt::m1cvm::channel_pressure).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m1cvm::channel_pressure::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m1cvm::channel_pressure & midi2::ump::m1cvm::channel_pressure::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m1cvm::channel_pressure)
  /// \fn constexpr auto midi2::ump::m1cvm::channel_pressure::data() const noexcept
  /// \brief Returns the value of the word0::data field.
  /// \returns The value of the word0::data field.
  //
  /// \fn constexpr ump::m1cvm::channel_pressure & midi2::ump::m1cvm::channel_pressure::data(adt::uinteger_t<word0::data::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::data field.
  /// \param v  The value to be assigned to the word0::data field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, data, ump::m1cvm::channel_pressure)

private:
  friend struct ::std::tuple_size<channel_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::m1cvm::channel_pressure>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m1cvm::channel_pressure
//
/// \class std::tuple_element<I, ::midi2::ump::m1cvm::channel_pressure>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m1cvm::channel_pressure
MIDI2_UMP_TUPLE(m1cvm, channel_pressure)  // Define tuple_size and tuple_element for m1cvm/channel_pressure
MIDI2_SPAN_CTOR(m1cvm, channel_pressure)  // Define the span constructor for m1cvm/channel_pressure

// 7.3.7 MIDI 1.0 Pitch Bend Message
/// \brief The MIDI 1.0 Pitch Bend message (section 7.3.7).
class midi2::ump::m1cvm::pitch_bend {
public:
  /// \brief The first and only word of a pitch_bend message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::m1cvm::pitch_bend;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x2 (message_type::m1cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xE (mt::m1cvm::pitch_bend).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the least significant 7 bits of the 14-bit pitch bend value. The value is centered
    /// at 0x2000.
    using lsb_data = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the most significant 7 bits of the 14-bit pitch bend value. The value is centered at
    /// 0x2000.
    using msb_data = adt::bit_range<0, 7>;
  };
  /// Default constructs a MIDI 1.0 Pitch Bend message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr pitch_bend() noexcept = default;
  /// Constructs a MIDI 1.0 Pitch Bend message from 1 raw 32-bit word.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The word of the message.
  constexpr explicit pitch_bend(std::span<std::uint32_t, 1> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(pitch_bend const&, pitch_bend const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x2 (message_type::m1cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m1cvm::pitch_bend::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m1cvm::pitch_bend & midi2::ump::m1cvm::pitch_bend::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m1cvm::pitch_bend)
  /// \brief Returns the value of the word0::status field. Always 0xE (mt::m1cvm::pitch_bend).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m1cvm::pitch_bend::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m1cvm::pitch_bend & midi2::ump::m1cvm::pitch_bend::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m1cvm::pitch_bend)
  /// \fn constexpr auto midi2::ump::m1cvm::pitch_bend::lsb_data() const noexcept
  /// \brief Returns the value of the word0::lsb_data field.
  /// \returns The value of the word0::lsb_data field.
  //
  /// \fn constexpr ump::m1cvm::pitch_bend & midi2::ump::m1cvm::pitch_bend::lsb_data(adt::uinteger_t<word0::lsb_data::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::lsb_data field.
  /// \param v  The value to be assigned to the word0::lsb_data field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, lsb_data, ump::m1cvm::pitch_bend)
  /// \fn constexpr auto midi2::ump::m1cvm::pitch_bend::msb_data() const noexcept
  /// \brief Returns the value of the word0::msb_data field.
  /// \returns The value of the word0::msb_data field.
  //
  /// \fn constexpr ump::m1cvm::pitch_bend & midi2::ump::m1cvm::pitch_bend::msb_data(adt::uinteger_t<word0::msb_data::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::msb_data field.
  /// \param v  The value to be assigned to the word0::msb_data field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, msb_data, ump::m1cvm::pitch_bend)

private:
  friend struct ::std::tuple_size<pitch_bend>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0> words_;
};
/// \class std::tuple_size<::midi2::ump::m1cvm::pitch_bend>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m1cvm::pitch_bend
//
/// \class std::tuple_element<I, ::midi2::ump::m1cvm::pitch_bend>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m1cvm::pitch_bend
MIDI2_UMP_TUPLE(m1cvm, pitch_bend)  // Define tuple_size and tuple_element for m1cvm/pitch_bend
MIDI2_SPAN_CTOR(m1cvm, pitch_bend)  // Define the span constructor for m1cvm/pitch_bend

//*     _      _         __ _ _   *
//*  __| |__ _| |_ __ _ / /| | |  *
//* / _` / _` |  _/ _` / _ \_  _| *
//* \__,_\__,_|\__\__,_\___/ |_|  *
//*                               *

/// \brief An integral constant which holds the number of 32-bit words in a UMP data64 message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::data64> : std::integral_constant<unsigned, 2> {};

namespace midi2::ump::data64::details {

// 7.7 System Exclusive (7-Bit) Messages
template <midi2::ump::mt::data64 Status> class sysex7;

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return get<I>(t.words_);
}

}  // end namespace midi2::ump::data64::details

/// \brief The System Exclusive (7-Bit) message (section 7.7).
///
/// \tparam Status  The status value which distinguishes the complete, start, continue, and end
///   messages.
template <midi2::ump::mt::data64 Status> class midi2::ump::data64::details::sysex7 {
public:
  /// \brief The first word of the System Exclusive (7-Bit) message.
  class word0 : public ump::details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = Status;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x3 (message_type::data64).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always the Status template argument.
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the number of bytes field. This is the number of valid data bytes (0-6) in this UMP.
    using number_of_bytes = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of data byte 0.
    using data0 = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of data byte 1.
    using data1 = adt::bit_range<0, 7>;
  };
  /// \brief The second word of the System Exclusive (7-Bit) message.
  class word1 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<31, 1>;
    /// Defines the bit position of data byte 2.
    using data2 = adt::bit_range<24, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<23, 1>;
    /// Defines the bit position of data byte 3.
    using data3 = adt::bit_range<16, 7>;
    /// Defines a reserved bit.
    using reserved2 = adt::bit_range<15, 1>;
    /// Defines the bit position of data byte 4.
    using data4 = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved3 = adt::bit_range<7, 1>;
    /// Defines the bit position of data byte 5.
    using data5 = adt::bit_range<0, 7>;
  };

  /// Default constructs a System Exclusive (7-Bit) message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr sysex7() noexcept = default;
  /// Constructs a System Exclusive (7-Bit) message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit sysex7(std::span<std::uint32_t, 2> const m) noexcept : words_{span_to_tuple(m)} {
    assert(get<0>(words_).check());
  }
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(sysex7 const&, sysex7 const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x3 (message_type::data64).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::data64::details::sysex7::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  ///
  /// The group field is a 4-bit value to address a UMP MIDI Message to one of 16 Groups.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::data64::details::sysex7<Status> & midi2::ump::data64::details::sysex7::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::data64::details::sysex7<Status>)
  /// \brief Returns the value of the word0::status field. Always the Status template argument.
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::data64::details::sysex7::number_of_bytes() const noexcept
  /// \brief Returns the value of the word0::number_of_bytes field.
  /// \returns The value of the word0::number_of_bytes field.
  //
  /// \fn constexpr ump::data64::details::sysex7<Status> & midi2::ump::data64::details::sysex7::number_of_bytes(adt::uinteger_t<word0::number_of_bytes::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::number_of_bytes field.
  /// \param v  The value to be assigned to the word0::number_of_bytes field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, number_of_bytes, ump::data64::details::sysex7<Status>)
  /// \fn constexpr auto midi2::ump::data64::details::sysex7::data0() const noexcept
  /// \brief Returns the value of the word0::data0 field.
  /// \returns The value of the word0::data0 field.
  //
  /// \fn constexpr ump::data64::details::sysex7<Status> & midi2::ump::data64::details::sysex7::data0(adt::uinteger_t<word0::data0::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::data0 field.
  /// \param v  The value to be assigned to the word0::data0 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, data0, ump::data64::details::sysex7<Status>)
  /// \fn constexpr auto midi2::ump::data64::details::sysex7::data1() const noexcept
  /// \brief Returns the value of the word0::data1 field.
  /// \returns The value of the word0::data1 field.
  //
  /// \fn constexpr ump::data64::details::sysex7<Status> & midi2::ump::data64::details::sysex7::data1(adt::uinteger_t<word0::data1::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::data1 field.
  /// \param v  The value to be assigned to the word0::data1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, data1, ump::data64::details::sysex7<Status>)
  /// \fn constexpr auto midi2::ump::data64::details::sysex7::data2() const noexcept
  /// \brief Returns the value of the word1::data2 field.
  /// \returns The value of the word1::data2 field.
  //
  /// \fn constexpr ump::data64::details::sysex7<Status> & midi2::ump::data64::details::sysex7::data2(adt::uinteger_t<word1::data2::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data2 field.
  /// \param v  The value to be assigned to the word1::data2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data2, ump::data64::details::sysex7<Status>)
  /// \fn constexpr auto midi2::ump::data64::details::sysex7::data3() const noexcept
  /// \brief Returns the value of the word1::data3 field.
  /// \returns The value of the word1::data3 field.
  //
  /// \fn constexpr ump::data64::details::sysex7<Status> & midi2::ump::data64::details::sysex7::data3(adt::uinteger_t<word1::data3::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data3 field.
  /// \param v  The value to be assigned to the word1::data3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data3, ump::data64::details::sysex7<Status>)
  /// \fn constexpr auto midi2::ump::data64::details::sysex7::data4() const noexcept
  /// \brief Returns the value of the word1::data4 field.
  /// \returns The value of the word1::data4 field.
  //
  /// \fn constexpr ump::data64::details::sysex7<Status> & midi2::ump::data64::details::sysex7::data4(adt::uinteger_t<word1::data4::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data4 field.
  /// \param v  The value to be assigned to the word1::data4 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data4, ump::data64::details::sysex7<Status>)
  /// \fn constexpr auto midi2::ump::data64::details::sysex7::data5() const noexcept
  /// \brief Returns the value of the word1::data5 field.
  /// \returns The value of the word1::data5 field.
  //
  /// \fn constexpr ump::data64::details::sysex7<Status> & midi2::ump::data64::details::sysex7::data5(adt::uinteger_t<word1::data5::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data5 field.
  /// \param v  The value to be assigned to the word1::data5 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data5, ump::data64::details::sysex7<Status>)

  /// The type of the data bytes held by the message.
  using value_type = adt::uinteger_t<7>;
  /// An unsigned integer type used to represent sizes and indices.
  using size_type = std::size_t;
  /// A signed integer type used to represent the distance between two iterators.
  using difference_type = std::ptrdiff_t;
  /// A random-access iterator over the data bytes of the message.
  using iterator = ::midi2::ump::details::iterator_base<sysex7>;
  /// A random-access iterator over the data bytes of a const message.
  using const_iterator = ::midi2::ump::details::iterator_base<sysex7 const>;

  /// \returns An iterator referring to the first data byte.
  [[nodiscard]] constexpr auto begin() noexcept { return iterator{*this, 0}; }
  /// \returns A const iterator referring to the first data byte.
  [[nodiscard]] constexpr auto begin() const noexcept { return const_iterator{*this, 0}; }
  /// \returns A const iterator referring to the first data byte.
  [[nodiscard]] constexpr auto cbegin() noexcept { return const_iterator{*this, 0}; }

  /// \returns An iterator referring to the position after the last data byte.
  [[nodiscard]] constexpr auto end() noexcept { return iterator{*this, this->number_of_bytes()}; }
  /// \returns A const iterator referring to the position after the last data byte.
  [[nodiscard]] constexpr auto end() const noexcept { return const_iterator{*this, this->number_of_bytes()}; }
  /// \returns A const iterator referring to the position after the last data byte.
  [[nodiscard]] constexpr auto cend() noexcept { return const_iterator{*this, this->number_of_bytes()}; }

  /// \returns The maximum number of data bytes that the message can hold.
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[nodiscard]] constexpr size_type max_size() const noexcept { return 6; }
  /// \returns The number of data bytes in the message.
  [[nodiscard]] constexpr size_type size() const noexcept { return this->number_of_bytes(); }
  /// \returns True if the message contains no data bytes, false otherwise.
  [[nodiscard]] constexpr bool empty() const noexcept { return this->size() == 0; }

#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
  /// Provides access to the data byte at position \p idx.
  /// \param self  The message object.
  /// \param idx  The index of the data byte to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read or assign the data byte.
  constexpr decltype(auto) operator[](this auto& self, std::size_t idx) noexcept {
    return midi2::ump::details::array_subscript_proxy<std::remove_reference_t<decltype(self)>>{self, idx};
  }
#else
  /// Provides access to the data byte at position \p idx.
  /// \param idx  The index of the data byte to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read or assign the data byte.
  constexpr decltype(auto) operator[](std::size_t idx) noexcept {
    return midi2::ump::details::array_subscript_proxy<sysex7>{*this, idx};
  }
  /// Provides read-only access to the data byte at position \p idx.
  /// \param idx  The index of the data byte to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read the data byte.
  constexpr decltype(auto) operator[](std::size_t idx) const noexcept {
    return midi2::ump::details::array_subscript_proxy<sysex7 const>{*this, idx};
  }
#endif

  /// Sets the data byte at position \p index.
  /// \param index  The index of the data byte to be set. Must be less than max_size().
  /// \param v  The value to be stored.
  /// \returns *this
  constexpr sysex7& data(std::size_t const index, adt::uinteger_t<7> const v) noexcept {
    switch (index) {
    case 0: this->data0(v); break;
    case 1: this->data1(v); break;
    case 2: this->data2(v); break;
    case 3: this->data3(v); break;
    case 4: this->data4(v); break;
    case 5: this->data5(v); break;
    default: assert(false && "Index out of range"); break;
    }
    return *this;
  }
  /// Returns the data byte at position \p index.
  /// \param index  The index of the data byte to be returned. Must be less than max_size().
  /// \returns The data byte at position \p index.
  [[nodiscard]] constexpr adt::uinteger_t<7> data(std::size_t const index) const noexcept {
    switch (index) {
    case 0: return this->data0();
    case 1: return this->data1();
    case 2: return this->data2();
    case 3: return this->data3();
    case 4: return this->data4();
    case 5: return this->data5();
    default: assert(false && "Index out of range"); return 0;
    }
  }

  /// Copies data bytes from an initializer list into the message. The number_of_bytes field is set to the number of
  ///   values copied.
  /// \tparam T  An integral type.
  /// \param vs  The data bytes to be copied. At most max_size() values are copied.
  /// \returns *this
  template <std::integral T> constexpr sysex7& data(std::initializer_list<T> vs) {
    auto const index = ::midi2::ump::details::data_helper<sysex7, 7U>::data(*this, vs);
    return this->number_of_bytes(static_cast<word0::number_of_bytes::uinteger>(index));
  }

  /// Copies data bytes from a range into the message. The number_of_bytes field is set to the number of values
  ///   copied.
  /// \tparam Range  An input range whose elements are integral values.
  /// \tparam Proj  The type of the projection applied to each element of the range.
  /// \param range  The data bytes to be copied. At most max_size() values are copied.
  /// \param proj  A projection applied to each element of the range.
  /// \returns *this
  template <std::ranges::input_range Range, typename Proj = std::identity>
    requires std::is_integral_v<std::remove_cvref_t<std::ranges::range_value_t<Range>>>
  constexpr sysex7& data(Range&& range, Proj proj = {}) {
    auto const index = ::midi2::ump::details::data_helper<sysex7, 7U>::data(*this, std::forward<Range>(range), proj);
    return this->number_of_bytes(static_cast<word0::number_of_bytes::uinteger>(index));
  }

  /// Copies data bytes from the iterator range [\p first, \p last) into the message. The number_of_bytes field is set
  ///   to the number of values copied.
  /// \tparam I  An input iterator type whose value type is integral.
  /// \tparam S  A sentinel type for \p I.
  /// \param first  The beginning of the range of data bytes to be copied.
  /// \param last  The end of the range of data bytes to be copied.
  /// \returns *this
  template <std::input_iterator I, std::sentinel_for<I> S>
    requires std::is_integral_v<std::remove_cvref_t<typename std::iterator_traits<I>::value_type>>
  constexpr sysex7& data(I first, S last) {
    auto const index = ::midi2::ump::details::data_helper<sysex7, 7U>::data(*this, first, last);
    return this->number_of_bytes(static_cast<word0::number_of_bytes::uinteger>(index));
  }

private:
  friend struct ::std::tuple_size<sysex7>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_{};
};

/// \brief Specialization of `std::tuple_size<>` for midi2::ump::data64::details::sysex7<>.
/// \tparam Status  The sysex7 message status.
template <midi2::ump::mt::data64 Status>
struct std::tuple_size<midi2::ump::data64::details::sysex7<Status>> /* NOLINT(cert-dcl58-cpp]*/
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::ump::data64::details::sysex7<Status>::words_)>> {};

/// \brief Specialization of `std::tuple_element<>` for midi2::ump::data64::details::sysex7<>.
/// \tparam I  The index of the word.
/// \tparam Status  The sysex7 message status.
template <std::size_t I, midi2::ump::mt::data64 Status>
struct std::tuple_element<I, midi2::ump::data64::details::sysex7<Status>> { /* NOLINT(cert-dcl58-cpp] */
  /// The type of the Ith word of the tuple-like type midi2::ump::data64::details::sysex7<>.
  using type = std::tuple_element_t<I, decltype(midi2::ump::data64::details::sysex7<Status>::words_)>;
};

/// Defines the C++ types that represent Data 64 Bit messages
namespace midi2::ump::data64 {

/// The message type used for 7-bit wide system exclusive data packed into a single UMP message.
using sysex7_in_1 = details::sysex7<mt::data64::sysex7_in_1>;
/// \brief The start of a block of 7-bit system exclusive data that is split over zero or more of
///   sysex7_continue messages and completed by a sysex7_end message.
using sysex7_start = details::sysex7<mt::data64::sysex7_start>;
/// \brief A block of 7-bit system exclusive data. They can be sent as many times as necessary
///   but must be preceded by a sysex7_start message.
using sysex7_continue = details::sysex7<mt::data64::sysex7_continue>;
/// A message which signals the end of a series of sysex7_start and sysex7_continue messages.
using sysex7_end = details::sysex7<mt::data64::sysex7_end>;

}  // namespace midi2::ump::data64

static_assert(std::tuple_size_v<midi2::ump::data64::sysex7_in_1> ==
              midi2::ump::message_size<midi2::ump::message_type::data64>::value);
static_assert(std::tuple_size_v<midi2::ump::data64::sysex7_start> ==
              midi2::ump::message_size<midi2::ump::message_type::data64>::value);
static_assert(std::tuple_size_v<midi2::ump::data64::sysex7_continue> ==
              midi2::ump::message_size<midi2::ump::message_type::data64>::value);
static_assert(std::tuple_size_v<midi2::ump::data64::sysex7_end> ==
              midi2::ump::message_size<midi2::ump::message_type::data64>::value);

static_assert(std::random_access_iterator<midi2::ump::data64::sysex7_in_1::iterator>,
              "sysex7_in_1::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::data64::sysex7_in_1::const_iterator>,
              "sysex7_in_1::const_iterator should meet the requirements of std::random_access_iterator");

static_assert(std::random_access_iterator<midi2::ump::data64::sysex7_start::iterator>,
              "sysex7_start::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::data64::sysex7_start::const_iterator>,
              "sysex7_start::const_iterator should meet the requirements of std::random_access_iterator");

static_assert(std::random_access_iterator<midi2::ump::data64::sysex7_continue::iterator>,
              "sysex7_continue::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::data64::sysex7_continue::const_iterator>,
              "sysex7_continue::const_iterator should meet the requirements of std::random_access_iterator");

static_assert(std::random_access_iterator<midi2::ump::data64::sysex7_end::iterator>,
              "sysex7_end::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::data64::sysex7_end::const_iterator>,
              "sysex7_end::const_iterator should meet the requirements of std::random_access_iterator");

//*        ___               *
//*  _ __ |_  )____ ___ __   *
//* | '  \ / // _\ V / '  \  *
//* |_|_|_/___\__|\_/|_|_|_| *
//*                          *
// F.2.2 Message Type 0x4: MIDI 2.0 Channel Voice Messages
// Table 30 8-Byte UMP Formats for Message Type 0x4: MIDI 2.0 Channel Voice Messages

/// \brief An integral constant which holds the number of 32-bit words in a UMP m2cvm message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::m2cvm> : std::integral_constant<unsigned, 2> {};

/// Defines the C++ types that represent MIDI 2.0 Channel Voice messages
namespace midi2::ump::m2cvm {

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return get<I>(t.words_);
}

class note_off;
class note_on;
class poly_pressure;
class rpn_per_note_controller;
class nrpn_per_note_controller;
class rpn_controller;
class nrpn_controller;
class rpn_relative_controller;
class nrpn_relative_controller;
class per_note_management;
class control_change;
class program_change;
class channel_pressure;
class pitch_bend;
class per_note_pitch_bend;

}  // end namespace midi2::ump::m2cvm

/// \brief Defines the MIDI 2.0 Note Off Message
class midi2::ump::m2cvm::note_off {
public:
  /// \brief The first word of the MIDI 2.0 Note Off Message
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::note_off;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit range of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x8 (mt::m2cvm::note_off).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines the bit position of the attribute type field. This declares the type of the data in the attribute field:
    /// 0x00 no attribute data, 0x01 manufacturer specific, 0x02 profile specific, 0x03 pitch 7.9. Other values are
    /// reserved.
    using attribute_type = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Note Off Message
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 16-bit (release) velocity field.
    using velocity = adt::bit_range<16, 16>;
    /// Defines the bit position of the 16-bit attribute data field. Its meaning is given by the attribute_type field.
    using attribute = adt::bit_range<0, 16>;
  };

  /// Default constructs a MIDI 2.0 Note Off message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr note_off() noexcept = default;
  /// Constructs a MIDI 2.0 Note Off message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit note_off(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(note_off const& a, note_off const& b) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::note_off::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::note_off & midi2::ump::m2cvm::note_off::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::note_off)
  /// \brief Returns the value of the word0::status field. Always 0x8 (mt::m2cvm::note_off).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::note_off::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::note_off & midi2::ump::m2cvm::note_off::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::note_off)
  /// \fn constexpr auto midi2::ump::m2cvm::note_off::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m2cvm::note_off & midi2::ump::m2cvm::note_off::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m2cvm::note_off)
  /// \fn constexpr auto midi2::ump::m2cvm::note_off::attribute_type() const noexcept
  /// \brief Returns the value of the word0::attribute_type field.
  /// \returns The value of the word0::attribute_type field.
  //
  /// \fn constexpr ump::m2cvm::note_off & midi2::ump::m2cvm::note_off::attribute_type(adt::uinteger_t<word0::attribute_type::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::attribute_type field.
  /// \param v  The value to be assigned to the word0::attribute_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, attribute_type, ump::m2cvm::note_off)
  /// \fn constexpr auto midi2::ump::m2cvm::note_off::velocity() const noexcept
  /// \brief Returns the value of the word1::velocity field.
  /// \returns The value of the word1::velocity field.
  //
  /// \fn constexpr ump::m2cvm::note_off & midi2::ump::m2cvm::note_off::velocity(adt::uinteger_t<word1::velocity::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::velocity field.
  /// \param v  The value to be assigned to the word1::velocity field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, velocity, ump::m2cvm::note_off)
  /// \fn constexpr auto midi2::ump::m2cvm::note_off::attribute() const noexcept
  /// \brief Returns the value of the word1::attribute field.
  /// \returns The value of the word1::attribute field.
  //
  /// \fn constexpr ump::m2cvm::note_off & midi2::ump::m2cvm::note_off::attribute(adt::uinteger_t<word1::attribute::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::attribute field.
  /// \param v  The value to be assigned to the word1::attribute field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, attribute, ump::m2cvm::note_off)

private:
  friend struct ::std::tuple_size<note_off>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::note_off>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::note_off
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::note_off>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::note_off
MIDI2_UMP_TUPLE(m2cvm, note_off)  // Define tuple_size and tuple_element for m2cvm/note_off
MIDI2_SPAN_CTOR(m2cvm, note_off)  // Define the span constructor for m2cvm/note_off

// 7.4.2 MIDI 2.0 Note On Message
/// \brief The MIDI 2.0 Note On message (section 7.4.2).
class midi2::ump::m2cvm::note_on {
public:
  /// \brief The first word of the MIDI 2.0 Note On message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::note_on;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }
    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x9 (mt::m2cvm::note_on).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines the bit position of the attribute type field. This declares the type of the data in the attribute field:
    /// 0x00 no attribute data, 0x01 manufacturer specific, 0x02 profile specific, 0x03 pitch 7.9. Other values are
    /// reserved.
    using attribute_type = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Note On message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 16-bit velocity field. Unlike MIDI 1.0, a velocity of zero does not function as
    /// a Note Off.
    using velocity = adt::bit_range<16, 16>;
    /// Defines the bit position of the 16-bit attribute data field. Its meaning is given by the attribute_type field.
    using attribute = adt::bit_range<0, 16>;
  };

  /// Default constructs a MIDI 2.0 Note On message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr note_on() noexcept = default;
  /// Constructs a MIDI 2.0 Note On message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit note_on(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(note_on const& a, note_on const& b) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::note_on::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::note_on & midi2::ump::m2cvm::note_on::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::note_on)
  /// \brief Returns the value of the word0::status field. Always 0x9 (mt::m2cvm::note_on).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::note_on::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::note_on & midi2::ump::m2cvm::note_on::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::note_on)
  /// \fn constexpr auto midi2::ump::m2cvm::note_on::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m2cvm::note_on & midi2::ump::m2cvm::note_on::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m2cvm::note_on)
  /// \fn constexpr auto midi2::ump::m2cvm::note_on::attribute_type() const noexcept
  /// \brief Returns the value of the word0::attribute_type field.
  /// \returns The value of the word0::attribute_type field.
  //
  /// \fn constexpr ump::m2cvm::note_on & midi2::ump::m2cvm::note_on::attribute_type(adt::uinteger_t<word0::attribute_type::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::attribute_type field.
  /// \param v  The value to be assigned to the word0::attribute_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, attribute_type, ump::m2cvm::note_on)
  /// \fn constexpr auto midi2::ump::m2cvm::note_on::velocity() const noexcept
  /// \brief Returns the value of the word1::velocity field.
  /// \returns The value of the word1::velocity field.
  //
  /// \fn constexpr ump::m2cvm::note_on & midi2::ump::m2cvm::note_on::velocity(adt::uinteger_t<word1::velocity::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::velocity field.
  /// \param v  The value to be assigned to the word1::velocity field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, velocity, ump::m2cvm::note_on)
  /// \fn constexpr auto midi2::ump::m2cvm::note_on::attribute() const noexcept
  /// \brief Returns the value of the word1::attribute field.
  /// \returns The value of the word1::attribute field.
  //
  /// \fn constexpr ump::m2cvm::note_on & midi2::ump::m2cvm::note_on::attribute(adt::uinteger_t<word1::attribute::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::attribute field.
  /// \param v  The value to be assigned to the word1::attribute field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, attribute, ump::m2cvm::note_on)

private:
  friend struct ::std::tuple_size<note_on>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::note_on>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::note_on
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::note_on>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::note_on
MIDI2_UMP_TUPLE(m2cvm, note_on)  // Define tuple_size and tuple_element for m2cvm/note_on
MIDI2_SPAN_CTOR(m2cvm, note_on)  // Define the span constructor for m2cvm/note_on

// 7.4.3 MIDI 2.0 Poly Pressure Message
/// \brief The MIDI 2.0 Poly Pressure message (section 7.4.3).
class midi2::ump::m2cvm::poly_pressure {
public:
  /// \brief The first word of the MIDI 2.0 Poly Pressure message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::poly_pressure;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xA (mt::m2cvm::poly_pressure).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Poly Pressure message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit pressure (aftertouch) field.
    using pressure = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Poly Pressure message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr poly_pressure() noexcept = default;
  /// Constructs a MIDI 2.0 Poly Pressure message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit poly_pressure(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(poly_pressure const& a, poly_pressure const& b) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::poly_pressure::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::poly_pressure & midi2::ump::m2cvm::poly_pressure::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::poly_pressure)
  /// \brief Returns the value of the word0::status field. Always 0xA (mt::m2cvm::poly_pressure).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::poly_pressure::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::poly_pressure & midi2::ump::m2cvm::poly_pressure::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::poly_pressure)
  /// \fn constexpr auto midi2::ump::m2cvm::poly_pressure::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m2cvm::poly_pressure & midi2::ump::m2cvm::poly_pressure::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m2cvm::poly_pressure)
  /// \fn constexpr auto midi2::ump::m2cvm::poly_pressure::pressure() const noexcept
  /// \brief Returns the value of the word1::pressure field.
  /// \returns The value of the word1::pressure field.
  //
  /// \fn constexpr ump::m2cvm::poly_pressure & midi2::ump::m2cvm::poly_pressure::pressure(adt::uinteger_t<word1::pressure::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::pressure field.
  /// \param v  The value to be assigned to the word1::pressure field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, pressure, ump::m2cvm::poly_pressure)

private:
  friend struct ::std::tuple_size<poly_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::poly_pressure>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::poly_pressure
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::poly_pressure>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::poly_pressure
MIDI2_UMP_TUPLE(m2cvm, poly_pressure)  // Define tuple_size and tuple_element for m2cvm/poly_pressure
MIDI2_SPAN_CTOR(m2cvm, poly_pressure)  // Define the span constructor for m2cvm/poly_pressure

// 7.4.4 MIDI 2.0 Registered Per-Note Controller Messages
/// \brief The MIDI 2.0 Registered Per-Note Controller message (section 7.4.4).
class midi2::ump::m2cvm::rpn_per_note_controller {
public:
  /// \brief The first word of the MIDI 2.0 Registered Per-Note Controller message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::rpn_per_note;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x0 (mt::m2cvm::rpn_per_note).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines the bit position of the registered per-note controller number (0-255).
    using index = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Registered Per-Note Controller message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit controller value field.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Registered Per-Note Controller message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr rpn_per_note_controller() noexcept = default;
  /// Constructs a MIDI 2.0 Registered Per-Note Controller message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit rpn_per_note_controller(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(rpn_per_note_controller const&, rpn_per_note_controller const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_per_note_controller::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::rpn_per_note_controller & midi2::ump::m2cvm::rpn_per_note_controller::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::rpn_per_note_controller)
  /// \brief Returns the value of the word0::status field. Always 0x0 (mt::m2cvm::rpn_per_note).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_per_note_controller::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::rpn_per_note_controller & midi2::ump::m2cvm::rpn_per_note_controller::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::rpn_per_note_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_per_note_controller::reserved() const noexcept
  /// \brief Returns the value of the word0::reserved field.
  /// \returns The value of the word0::reserved field.
  //
  /// \fn constexpr ump::m2cvm::rpn_per_note_controller & midi2::ump::m2cvm::rpn_per_note_controller::reserved(adt::uinteger_t<word0::reserved::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::reserved field.
  /// \param v  The value to be assigned to the word0::reserved field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, reserved, ump::m2cvm::rpn_per_note_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_per_note_controller::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m2cvm::rpn_per_note_controller & midi2::ump::m2cvm::rpn_per_note_controller::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m2cvm::rpn_per_note_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_per_note_controller::index() const noexcept
  /// \brief Returns the value of the word0::index field.
  /// \returns The value of the word0::index field.
  //
  /// \fn constexpr ump::m2cvm::rpn_per_note_controller & midi2::ump::m2cvm::rpn_per_note_controller::index(adt::uinteger_t<word0::index::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::index field.
  /// \param v  The value to be assigned to the word0::index field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, index, ump::m2cvm::rpn_per_note_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_per_note_controller::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::rpn_per_note_controller & midi2::ump::m2cvm::rpn_per_note_controller::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::rpn_per_note_controller)

private:
  friend struct ::std::tuple_size<rpn_per_note_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::rpn_per_note_controller>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::rpn_per_note_controller
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::rpn_per_note_controller>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::rpn_per_note_controller
MIDI2_UMP_TUPLE(m2cvm,
                rpn_per_note_controller)  // Define tuple_size and tuple_element for m2cvm/rpn_per_note_controller
MIDI2_SPAN_CTOR(m2cvm, rpn_per_note_controller)  // Define the span constructor for m2cvm/rpn_per_note_controller

// 7.4.4 MIDI 2.0 Assignable Per-Note Controller Messages
/// \brief The MIDI 2.0 Assignable Per-Note Controller message (section 7.4.4).
class midi2::ump::m2cvm::nrpn_per_note_controller {
public:
  /// \brief The first word of the MIDI 2.0 Assignable Per-Note Controller message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::nrpn_per_note;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x1 (mt::m2cvm::nrpn_per_note).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines the bit position of the assignable per-note controller number (0-255).
    using index = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Assignable Per-Note Controller message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit controller value field.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Assignable Per-Note Controller message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr nrpn_per_note_controller() noexcept = default;
  /// Constructs a MIDI 2.0 Assignable Per-Note Controller message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit nrpn_per_note_controller(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(nrpn_per_note_controller const&, nrpn_per_note_controller const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_per_note_controller::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_per_note_controller & midi2::ump::m2cvm::nrpn_per_note_controller::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::nrpn_per_note_controller)
  /// \brief Returns the value of the word0::status field. Always 0x1 (mt::m2cvm::nrpn_per_note).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_per_note_controller::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_per_note_controller & midi2::ump::m2cvm::nrpn_per_note_controller::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::nrpn_per_note_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_per_note_controller::reserved() const noexcept
  /// \brief Returns the value of the word0::reserved field.
  /// \returns The value of the word0::reserved field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_per_note_controller & midi2::ump::m2cvm::nrpn_per_note_controller::reserved(adt::uinteger_t<word0::reserved::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::reserved field.
  /// \param v  The value to be assigned to the word0::reserved field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, reserved, ump::m2cvm::nrpn_per_note_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_per_note_controller::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_per_note_controller & midi2::ump::m2cvm::nrpn_per_note_controller::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m2cvm::nrpn_per_note_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_per_note_controller::index() const noexcept
  /// \brief Returns the value of the word0::index field.
  /// \returns The value of the word0::index field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_per_note_controller & midi2::ump::m2cvm::nrpn_per_note_controller::index(adt::uinteger_t<word0::index::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::index field.
  /// \param v  The value to be assigned to the word0::index field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, index, ump::m2cvm::nrpn_per_note_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_per_note_controller::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_per_note_controller & midi2::ump::m2cvm::nrpn_per_note_controller::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::nrpn_per_note_controller)

private:
  friend struct ::std::tuple_size<nrpn_per_note_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::nrpn_per_note_controller>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::nrpn_per_note_controller
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::nrpn_per_note_controller>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::nrpn_per_note_controller
MIDI2_UMP_TUPLE(m2cvm,
                nrpn_per_note_controller)  // Define tuple_size and tuple_element for m2cvm/nrpn_per_note_controller
MIDI2_SPAN_CTOR(m2cvm, nrpn_per_note_controller)  // Define the span constructor for m2cvm/nrpn_per_note_controller

// 7.4.7 MIDI 2.0 Registered Controller (RPN) Message
/// "Registered Controllers have specific functions defined by MMA/AMEI specifications. Registered Controllers
/// map and translate directly to MIDI 1.0 Registered Parameter Numbers and use the same
/// definitions as MMA/AMEI approved RPN messages. Registered Controllers are organized in 128 Banks
/// (corresponds to RPN MSB), with 128 controllers per Bank (corresponds to RPN LSB)."
class midi2::ump::m2cvm::rpn_controller {
public:
  /// \brief The first word of the MIDI 2.0 Registered Controller (RPN) message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::rpn;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x2 (mt::m2cvm::rpn).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit bank field (corresponds to the RPN MSB).
    using bank = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the 7-bit index field (corresponds to the RPN LSB).
    using index = adt::bit_range<0, 7>;
  };
  /// \brief The second word of the MIDI 2.0 Registered Controller (RPN) message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit controller value field.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Registered Controller (RPN) message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr rpn_controller() noexcept = default;
  /// Constructs a MIDI 2.0 Registered Controller (RPN) message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit rpn_controller(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(rpn_controller const&, rpn_controller const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \brief Returns the value of the word0::status field. Always 0x2 (mt::m2cvm::rpn).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_controller::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::rpn_controller & midi2::ump::m2cvm::rpn_controller::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::rpn_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_controller::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::rpn_controller & midi2::ump::m2cvm::rpn_controller::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::rpn_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_controller::bank() const noexcept
  /// \brief Returns the value of the word0::bank field.
  /// \returns The value of the word0::bank field.
  //
  /// \fn constexpr ump::m2cvm::rpn_controller & midi2::ump::m2cvm::rpn_controller::bank(adt::uinteger_t<word0::bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::bank field.
  /// \param v  The value to be assigned to the word0::bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, bank, ump::m2cvm::rpn_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_controller::index() const noexcept
  /// \brief Returns the value of the word0::index field.
  /// \returns The value of the word0::index field.
  //
  /// \fn constexpr ump::m2cvm::rpn_controller & midi2::ump::m2cvm::rpn_controller::index(adt::uinteger_t<word0::index::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::index field.
  /// \param v  The value to be assigned to the word0::index field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, index, ump::m2cvm::rpn_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_controller::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::rpn_controller & midi2::ump::m2cvm::rpn_controller::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::rpn_controller)

private:
  friend struct ::std::tuple_size<rpn_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::rpn_controller>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::rpn_controller
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::rpn_controller>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::rpn_controller
MIDI2_UMP_TUPLE(m2cvm, rpn_controller)  // Define tuple_size and tuple_element for m2cvm/rpn_controller
MIDI2_SPAN_CTOR(m2cvm, rpn_controller)  // Define the span constructor for m2cvm/rpn_controller

// 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message
/// \brief The MIDI 2.0 Assignable Controller (NRPN) message (section 7.4.7).
class midi2::ump::m2cvm::nrpn_controller {
public:
  /// \brief The first word of the MIDI 2.0 Assignable Controller (NRPN) message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::nrpn;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x3 (mt::m2cvm::nrpn).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit bank field (corresponds to the NRPN MSB).
    using bank = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the 7-bit index field (corresponds to the NRPN LSB).
    using index = adt::bit_range<0, 7>;
  };
  /// \brief The second word of the MIDI 2.0 Assignable Controller (NRPN) message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit controller value field.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Assignable Controller (NRPN) message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr nrpn_controller() noexcept = default;
  /// Constructs a MIDI 2.0 Assignable Controller (NRPN) message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit nrpn_controller(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(nrpn_controller const&, nrpn_controller const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_controller::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_controller & midi2::ump::m2cvm::nrpn_controller::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::nrpn_controller)
  /// \brief Returns the value of the word0::status field. Always 0x3 (mt::m2cvm::nrpn).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_controller::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_controller & midi2::ump::m2cvm::nrpn_controller::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::nrpn_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_controller::bank() const noexcept
  /// \brief Returns the value of the word0::bank field.
  /// \returns The value of the word0::bank field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_controller & midi2::ump::m2cvm::nrpn_controller::bank(adt::uinteger_t<word0::bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::bank field.
  /// \param v  The value to be assigned to the word0::bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, bank, ump::m2cvm::nrpn_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_controller::index() const noexcept
  /// \brief Returns the value of the word0::index field.
  /// \returns The value of the word0::index field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_controller & midi2::ump::m2cvm::nrpn_controller::index(adt::uinteger_t<word0::index::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::index field.
  /// \param v  The value to be assigned to the word0::index field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, index, ump::m2cvm::nrpn_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_controller::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_controller & midi2::ump::m2cvm::nrpn_controller::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::nrpn_controller)

private:
  friend struct ::std::tuple_size<nrpn_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::nrpn_controller>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::nrpn_controller
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::nrpn_controller>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::nrpn_controller
MIDI2_UMP_TUPLE(m2cvm, nrpn_controller)  // Define tuple_size and tuple_element for m2cvm/nrpn_controller
MIDI2_SPAN_CTOR(m2cvm, nrpn_controller)  // Define the span constructor for m2cvm/nrpn_controller

// 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message
/// \brief The MIDI 2.0 Relative Registered Controller message (section 7.4.8).
class midi2::ump::m2cvm::rpn_relative_controller {
public:
  /// \brief The first word of the MIDI 2.0 Relative Registered Controller message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::rpn_relative;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x4 (mt::m2cvm::rpn_relative).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit bank field (corresponds to the RPN MSB).
    using bank = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the 7-bit index field (corresponds to the RPN LSB).
    using index = adt::bit_range<0, 7>;
  };
  /// \brief The second word of the MIDI 2.0 Relative Registered Controller message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit value field. This is a two's complement value giving a relative increase
    /// or decrease to the controller's current value.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Relative Registered Controller message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr rpn_relative_controller() noexcept = default;
  /// Constructs a MIDI 2.0 Relative Registered Controller message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit rpn_relative_controller(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(rpn_relative_controller const&, rpn_relative_controller const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_relative_controller::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::rpn_relative_controller & midi2::ump::m2cvm::rpn_relative_controller::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::rpn_relative_controller)
  /// \brief Returns the value of the word0::status field. Always 0x4 (mt::m2cvm::rpn_relative).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_relative_controller::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::rpn_relative_controller & midi2::ump::m2cvm::rpn_relative_controller::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::rpn_relative_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_relative_controller::bank() const noexcept
  /// \brief Returns the value of the word0::bank field.
  /// \returns The value of the word0::bank field.
  //
  /// \fn constexpr ump::m2cvm::rpn_relative_controller & midi2::ump::m2cvm::rpn_relative_controller::bank(adt::uinteger_t<word0::bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::bank field.
  /// \param v  The value to be assigned to the word0::bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, bank, ump::m2cvm::rpn_relative_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_relative_controller::index() const noexcept
  /// \brief Returns the value of the word0::index field.
  /// \returns The value of the word0::index field.
  //
  /// \fn constexpr ump::m2cvm::rpn_relative_controller & midi2::ump::m2cvm::rpn_relative_controller::index(adt::uinteger_t<word0::index::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::index field.
  /// \param v  The value to be assigned to the word0::index field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, index, ump::m2cvm::rpn_relative_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::rpn_relative_controller::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::rpn_relative_controller & midi2::ump::m2cvm::rpn_relative_controller::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::rpn_relative_controller)

private:
  friend struct ::std::tuple_size<rpn_relative_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::rpn_relative_controller>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::rpn_relative_controller
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::rpn_relative_controller>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::rpn_relative_controller
MIDI2_UMP_TUPLE(m2cvm,
                rpn_relative_controller)  // Define tuple_size and tuple_element for m2cvm/rpn_relative_controller
MIDI2_SPAN_CTOR(m2cvm, rpn_relative_controller)  // Define the span constructor for m2cvm/rpn_relative_controller

// 7.4.8 MIDI 2.0 Assignable Controller (NRPN) Message
/// \brief The MIDI 2.0 Relative Assignable Controller message (section 7.4.8).
class midi2::ump::m2cvm::nrpn_relative_controller {
public:
  /// \brief The first word of the MIDI 2.0 Relative Assignable Controller message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::nrpn_relative;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x5 (mt::m2cvm::nrpn_relative).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit bank field (corresponds to the NRPN MSB).
    using bank = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<7, 1>;
    /// Defines the bit position of the 7-bit index field (corresponds to the NRPN LSB).
    using index = adt::bit_range<0, 7>;
  };
  /// \brief The second word of the MIDI 2.0 Relative Assignable Controller message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit value field. This is a two's complement value giving a relative increase
    /// or decrease to the controller's current value.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Relative Assignable Controller message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr nrpn_relative_controller() noexcept = default;
  /// Constructs a MIDI 2.0 Relative Assignable Controller message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit nrpn_relative_controller(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(nrpn_relative_controller const&, nrpn_relative_controller const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_relative_controller::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_relative_controller & midi2::ump::m2cvm::nrpn_relative_controller::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::nrpn_relative_controller)
  /// \brief Returns the value of the word0::status field. Always 0x5 (mt::m2cvm::nrpn_relative).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_relative_controller::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_relative_controller & midi2::ump::m2cvm::nrpn_relative_controller::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::nrpn_relative_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_relative_controller::reserved0() const noexcept
  /// \brief Returns the value of the word0::reserved0 field.
  /// \returns The value of the word0::reserved0 field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_relative_controller & midi2::ump::m2cvm::nrpn_relative_controller::reserved0(adt::uinteger_t<word0::reserved0::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::reserved0 field.
  /// \param v  The value to be assigned to the word0::reserved0 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, reserved0, ump::m2cvm::nrpn_relative_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_relative_controller::bank() const noexcept
  /// \brief Returns the value of the word0::bank field.
  /// \returns The value of the word0::bank field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_relative_controller & midi2::ump::m2cvm::nrpn_relative_controller::bank(adt::uinteger_t<word0::bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::bank field.
  /// \param v  The value to be assigned to the word0::bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, bank, ump::m2cvm::nrpn_relative_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_relative_controller::reserved1() const noexcept
  /// \brief Returns the value of the word0::reserved1 field.
  /// \returns The value of the word0::reserved1 field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_relative_controller & midi2::ump::m2cvm::nrpn_relative_controller::reserved1(adt::uinteger_t<word0::reserved1::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::reserved1 field.
  /// \param v  The value to be assigned to the word0::reserved1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, reserved1, ump::m2cvm::nrpn_relative_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_relative_controller::index() const noexcept
  /// \brief Returns the value of the word0::index field.
  /// \returns The value of the word0::index field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_relative_controller & midi2::ump::m2cvm::nrpn_relative_controller::index(adt::uinteger_t<word0::index::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::index field.
  /// \param v  The value to be assigned to the word0::index field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, index, ump::m2cvm::nrpn_relative_controller)
  /// \fn constexpr auto midi2::ump::m2cvm::nrpn_relative_controller::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::nrpn_relative_controller & midi2::ump::m2cvm::nrpn_relative_controller::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::nrpn_relative_controller)

private:
  friend struct ::std::tuple_size<nrpn_relative_controller>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::nrpn_relative_controller>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::nrpn_relative_controller
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::nrpn_relative_controller>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::nrpn_relative_controller
MIDI2_UMP_TUPLE(m2cvm,
                nrpn_relative_controller)  // Define tuple_size and tuple_element for m2cvm/nrpn_relative_controller
MIDI2_SPAN_CTOR(m2cvm, nrpn_relative_controller)  // Define the span constructor for m2cvm/nrpn_relative_controller

// 7.4.5 MIDI 2.0 Per-Note Management Message
/// \brief The MIDI 2.0 Per-Note Management message (section 7.4.5).
class midi2::ump::m2cvm::per_note_management {
public:
  /// \brief The first word of the MIDI 2.0 Per-Note Management message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::per_note_manage;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xF (mt::m2cvm::per_note_manage).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines the bit position of the option flags field. The individual flags are also available as the detach and
    /// set_to_default fields.
    using option_flags = adt::bit_range<0, 1>;
    /// Defines the bit position of the detach (D) option flag. When set, per-note controllers are detached from
    /// previously received notes on this note number.
    using detach = adt::bit_range<1, 1>;
    /// Defines the bit position of the reset (S) option flag. When set, per-note controllers on this note number are
    /// reset (set) to their default values.
    using set_to_default = adt::bit_range<0, 1>;
  };
  /// \brief The second word of the MIDI 2.0 Per-Note Management message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the second word of the message. This is reserved and should be zero.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Per-Note Management message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr per_note_management() noexcept = default;
  /// Constructs a MIDI 2.0 Per-Note Management message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit per_note_management(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(per_note_management const&, per_note_management const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_management::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::per_note_management & midi2::ump::m2cvm::per_note_management::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::per_note_management)
  /// \brief Returns the value of the word0::status field. Always 0xF (mt::m2cvm::per_note_manage).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_management::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::per_note_management & midi2::ump::m2cvm::per_note_management::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::per_note_management)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_management::reserved() const noexcept
  /// \brief Returns the value of the word0::reserved field.
  /// \returns The value of the word0::reserved field.
  //
  /// \fn constexpr ump::m2cvm::per_note_management & midi2::ump::m2cvm::per_note_management::reserved(adt::uinteger_t<word0::reserved::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::reserved field.
  /// \param v  The value to be assigned to the word0::reserved field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, reserved, ump::m2cvm::per_note_management)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_management::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m2cvm::per_note_management & midi2::ump::m2cvm::per_note_management::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m2cvm::per_note_management)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_management::option_flags() const noexcept
  /// \brief Returns the value of the word0::option_flags field.
  /// \returns The value of the word0::option_flags field.
  //
  /// \fn constexpr ump::m2cvm::per_note_management & midi2::ump::m2cvm::per_note_management::option_flags(adt::uinteger_t<word0::option_flags::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::option_flags field.
  /// \param v  The value to be assigned to the word0::option_flags field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, option_flags, ump::m2cvm::per_note_management)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_management::detach() const noexcept
  /// \brief Returns the value of the word0::detach field.
  /// \returns The value of the word0::detach field.
  //
  /// \fn constexpr ump::m2cvm::per_note_management & midi2::ump::m2cvm::per_note_management::detach(adt::uinteger_t<word0::detach::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::detach field.
  /// \param v  The value to be assigned to the word0::detach field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, detach, ump::m2cvm::per_note_management)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_management::set_to_default() const noexcept
  /// \brief Returns the value of the word0::set_to_default field.
  /// \returns The value of the word0::set_to_default field.
  //
  /// \fn constexpr ump::m2cvm::per_note_management & midi2::ump::m2cvm::per_note_management::set_to_default(adt::uinteger_t<word0::set_to_default::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::set_to_default field.
  /// \param v  The value to be assigned to the word0::set_to_default field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, set_to_default, ump::m2cvm::per_note_management)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_management::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::per_note_management & midi2::ump::m2cvm::per_note_management::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::per_note_management)

private:
  friend struct ::std::tuple_size<per_note_management>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::per_note_management>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::per_note_management
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::per_note_management>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::per_note_management
MIDI2_UMP_TUPLE(m2cvm, per_note_management)  // Define tuple_size and tuple_element for m2cvm/per_note_management
MIDI2_SPAN_CTOR(m2cvm, per_note_management)  // Define the span constructor for m2cvm/per_note_management

// 7.4.6 MIDI 2.0 Control Change Message
/// \brief The MIDI 2.0 Control Change message (section 7.4.6).
class midi2::ump::m2cvm::control_change {
public:
  /// \brief The first word of the MIDI 2.0 Control Change message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::cc;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xB (mt::m2cvm::cc).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit controller number (index) field.
    using controller = adt::bit_range<8, 7>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Control Change message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit controller value field.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Control Change message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr control_change() noexcept = default;
  /// Constructs a MIDI 2.0 Control Change message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit control_change(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(control_change const&, control_change const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::control_change::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::control_change & midi2::ump::m2cvm::control_change::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::control_change)
  /// \brief Returns the value of the word0::status field. Always 0xB (mt::m2cvm::cc).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::control_change::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::control_change & midi2::ump::m2cvm::control_change::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::control_change)
  /// \fn constexpr auto midi2::ump::m2cvm::control_change::controller() const noexcept
  /// \brief Returns the value of the word0::controller field.
  /// \returns The value of the word0::controller field.
  //
  /// \fn constexpr ump::m2cvm::control_change & midi2::ump::m2cvm::control_change::controller(adt::uinteger_t<word0::controller::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::controller field.
  /// \param v  The value to be assigned to the word0::controller field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, controller, ump::m2cvm::control_change)
  /// \fn constexpr auto midi2::ump::m2cvm::control_change::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::control_change & midi2::ump::m2cvm::control_change::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::control_change)

  /// Sets the value of the word0::controller field from a member of the ump::control enumeration.
  /// \param c  The controller number.
  /// \returns *this
  constexpr auto& controller(control const c) noexcept { return this->controller(std::to_underlying(c)); }

private:
  friend struct ::std::tuple_size<control_change>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::control_change>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::control_change
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::control_change>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::control_change
MIDI2_UMP_TUPLE(m2cvm, control_change)  // Define tuple_size and tuple_element for m2cvm/control_change
MIDI2_SPAN_CTOR(m2cvm, control_change)  // Define the span constructor for m2cvm/control_change

// 7.4.9 MIDI 2.0 Program Change Message
/// \brief The MIDI 2.0 Program Change message (section 7.4.9).
class midi2::ump::m2cvm::program_change {
public:
  /// \brief The first word of the MIDI 2.0 Program Change message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::program_change;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xC (mt::m2cvm::program_change).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a group of reserved bits.
    using reserved = adt::bit_range<8, 8>;
    /// Defines the bit position of the option flags that are reserved by the specification. They must be zero.
    using option_flags = adt::bit_range<1, 7>;
    /// Defines the bit position of the bank valid (B) flag. If set, the receiver selects the bank given by bank_msb and
    /// bank_lsb before changing the program. If clear, the bank is unchanged and bank_msb and bank_lsb should be zero.
    using bank_valid = adt::bit_range<0, 1>;
  };
  /// \brief The second word of the MIDI 2.0 Program Change message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of the program number field.
    using program = adt::bit_range<24, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<16, 8>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<15, 1>;
    /// Defines the bit position of the bank select MSB field. This is used only if the bank_valid flag is set.
    using bank_msb = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved2 = adt::bit_range<7, 1>;
    /// Defines the bit position of the bank select LSB field. This is used only if the bank_valid flag is set.
    using bank_lsb = adt::bit_range<0, 7>;
  };

  /// Default constructs a MIDI 2.0 Program Change message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr program_change() noexcept = default;
  /// Constructs a MIDI 2.0 Program Change message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit program_change(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(program_change const&, program_change const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::program_change::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::program_change & midi2::ump::m2cvm::program_change::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::program_change)
  /// \brief Returns the value of the word0::status field. Always 0xC (mt::m2cvm::program_change).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::program_change::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::program_change & midi2::ump::m2cvm::program_change::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::program_change)
  /// \fn constexpr auto midi2::ump::m2cvm::program_change::option_flags() const noexcept
  /// \brief Returns the value of the word0::option_flags field.
  /// \returns The value of the word0::option_flags field.
  //
  /// \fn constexpr ump::m2cvm::program_change & midi2::ump::m2cvm::program_change::option_flags(adt::uinteger_t<word0::option_flags::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::option_flags field.
  /// \param v  The value to be assigned to the word0::option_flags field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, option_flags, ump::m2cvm::program_change)
  /// \fn constexpr auto midi2::ump::m2cvm::program_change::bank_valid() const noexcept
  /// \brief Returns the value of the word0::bank_valid field.
  /// \returns The value of the word0::bank_valid field.
  //
  /// \fn constexpr ump::m2cvm::program_change & midi2::ump::m2cvm::program_change::bank_valid(adt::uinteger_t<word0::bank_valid::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::bank_valid field.
  /// \param v  The value to be assigned to the word0::bank_valid field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, bank_valid, ump::m2cvm::program_change)
  /// \fn constexpr auto midi2::ump::m2cvm::program_change::program() const noexcept
  /// \brief Returns the value of the word1::program field.
  /// \returns The value of the word1::program field.
  //
  /// \fn constexpr ump::m2cvm::program_change & midi2::ump::m2cvm::program_change::program(adt::uinteger_t<word1::program::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::program field.
  /// \param v  The value to be assigned to the word1::program field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, program, ump::m2cvm::program_change)
  /// \fn constexpr auto midi2::ump::m2cvm::program_change::bank_msb() const noexcept
  /// \brief Returns the value of the word1::bank_msb field.
  /// \returns The value of the word1::bank_msb field.
  //
  /// \fn constexpr ump::m2cvm::program_change & midi2::ump::m2cvm::program_change::bank_msb(adt::uinteger_t<word1::bank_msb::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::bank_msb field.
  /// \param v  The value to be assigned to the word1::bank_msb field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, bank_msb, ump::m2cvm::program_change)
  /// \fn constexpr auto midi2::ump::m2cvm::program_change::bank_lsb() const noexcept
  /// \brief Returns the value of the word1::bank_lsb field.
  /// \returns The value of the word1::bank_lsb field.
  //
  /// \fn constexpr ump::m2cvm::program_change & midi2::ump::m2cvm::program_change::bank_lsb(adt::uinteger_t<word1::bank_lsb::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::bank_lsb field.
  /// \param v  The value to be assigned to the word1::bank_lsb field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, bank_lsb, ump::m2cvm::program_change)

private:
  friend struct ::std::tuple_size<program_change>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::program_change>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::program_change
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::program_change>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::program_change
MIDI2_UMP_TUPLE(m2cvm, program_change)  // Define tuple_size and tuple_element for m2cvm/program_change
MIDI2_SPAN_CTOR(m2cvm, program_change)  // Define the span constructor for m2cvm/program_change

// 7.4.10 MIDI 2.0 Channel Pressure Message
/// \brief The MIDI 2.0 Channel Pressure message (section 7.4.10).
class midi2::ump::m2cvm::channel_pressure {
public:
  /// \brief The first word of the MIDI 2.0 Channel Pressure message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::channel_pressure;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xD (mt::m2cvm::channel_pressure).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Channel Pressure message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit pressure (aftertouch) field.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Channel Pressure message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr channel_pressure() noexcept = default;
  /// Constructs a MIDI 2.0 Channel Pressure message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit channel_pressure(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(channel_pressure const&, channel_pressure const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::channel_pressure::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::channel_pressure & midi2::ump::m2cvm::channel_pressure::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::channel_pressure)
  /// \brief Returns the value of the word0::status field. Always 0xD (mt::m2cvm::channel_pressure).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::channel_pressure::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::channel_pressure & midi2::ump::m2cvm::channel_pressure::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::channel_pressure)
  /// \fn constexpr auto midi2::ump::m2cvm::channel_pressure::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::channel_pressure & midi2::ump::m2cvm::channel_pressure::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::channel_pressure)

private:
  friend struct ::std::tuple_size<channel_pressure>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::channel_pressure>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::channel_pressure
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::channel_pressure>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::channel_pressure
MIDI2_UMP_TUPLE(m2cvm, channel_pressure)  // Define tuple_size and tuple_element for m2cvm/channel_pressure
MIDI2_SPAN_CTOR(m2cvm, channel_pressure)  // Define the span constructor for m2cvm/channel_pressure

// 7.4.11 MIDI 2.0 Pitch Bend Message
/// \brief The MIDI 2.0 Pitch Bend message (section 7.4.11).
class midi2::ump::m2cvm::pitch_bend {
public:
  /// \brief The first word of the MIDI 2.0 Pitch Bend message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::pitch_bend;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0xE (mt::m2cvm::pitch_bend).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Pitch Bend message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit pitch bend field. This is an unsigned bipolar value centered at
    /// 0x80000000.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Pitch Bend message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr pitch_bend() noexcept = default;
  /// Constructs a MIDI 2.0 Pitch Bend message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit pitch_bend(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(pitch_bend const& a, pitch_bend const& b) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::pitch_bend::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::pitch_bend & midi2::ump::m2cvm::pitch_bend::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::pitch_bend)
  /// \brief Returns the value of the word0::status field. Always 0xE (mt::m2cvm::pitch_bend).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::pitch_bend::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::pitch_bend & midi2::ump::m2cvm::pitch_bend::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::pitch_bend)
  /// \fn constexpr auto midi2::ump::m2cvm::pitch_bend::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::pitch_bend & midi2::ump::m2cvm::pitch_bend::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::pitch_bend)

private:
  friend struct ::std::tuple_size<pitch_bend>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::pitch_bend>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::pitch_bend
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::pitch_bend>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::pitch_bend
MIDI2_UMP_TUPLE(m2cvm, pitch_bend)  // Define tuple_size and tuple_element for m2cvm/pitch_bend
MIDI2_SPAN_CTOR(m2cvm, pitch_bend)  // Define the span constructor for m2cvm/channel_pressure

// 7.4.12 MIDI 2.0 Per-Note Pitch Bend Message
/// \brief The MIDI 2.0 Per-Note Pitch Bend message (section 7.4.12).
class midi2::ump::m2cvm::per_note_pitch_bend {
public:
  /// \brief The first word of the MIDI 2.0 Per-Note Pitch Bend message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::m2cvm::pitch_bend_per_note;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x4 (message_type::m2cvm).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x6 (mt::m2cvm::pitch_bend_per_note).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is addressed.
    using channel = adt::bit_range<16, 4>;
    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<15, 1>;
    /// Defines the bit position of the 7-bit note number field.
    using note = adt::bit_range<8, 7>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the MIDI 2.0 Per-Note Pitch Bend message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the 32-bit pitch bend field. This is an unsigned bipolar value centered at
    /// 0x80000000.
    using value = adt::bit_range<0, 32>;
  };

  /// Default constructs a MIDI 2.0 Per-Note Pitch Bend message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr per_note_pitch_bend() noexcept = default;
  /// Constructs a MIDI 2.0 Per-Note Pitch Bend message from 2 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit per_note_pitch_bend(std::span<std::uint32_t, 2> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(per_note_pitch_bend const&, per_note_pitch_bend const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x4 (message_type::m2cvm).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_pitch_bend::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::m2cvm::per_note_pitch_bend & midi2::ump::m2cvm::per_note_pitch_bend::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::m2cvm::per_note_pitch_bend)
  /// \brief Returns the value of the word0::status field. Always 0x6 (mt::m2cvm::pitch_bend_per_note).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_pitch_bend::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::m2cvm::per_note_pitch_bend & midi2::ump::m2cvm::per_note_pitch_bend::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::m2cvm::per_note_pitch_bend)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_pitch_bend::note() const noexcept
  /// \brief Returns the value of the word0::note field.
  /// \returns The value of the word0::note field.
  //
  /// \fn constexpr ump::m2cvm::per_note_pitch_bend & midi2::ump::m2cvm::per_note_pitch_bend::note(adt::uinteger_t<word0::note::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::note field.
  /// \param v  The value to be assigned to the word0::note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, note, ump::m2cvm::per_note_pitch_bend)
  /// \fn constexpr auto midi2::ump::m2cvm::per_note_pitch_bend::value() const noexcept
  /// \brief Returns the value of the word1::value field.
  /// \returns The value of the word1::value field.
  //
  /// \fn constexpr ump::m2cvm::per_note_pitch_bend & midi2::ump::m2cvm::per_note_pitch_bend::value(adt::uinteger_t<word1::value::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value field.
  /// \param v  The value to be assigned to the word1::value field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value, ump::m2cvm::per_note_pitch_bend)

private:
  friend struct ::std::tuple_size<per_note_pitch_bend>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1> words_;
};
/// \class std::tuple_size<::midi2::ump::m2cvm::per_note_pitch_bend>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::m2cvm::per_note_pitch_bend
//
/// \class std::tuple_element<I, ::midi2::ump::m2cvm::per_note_pitch_bend>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::m2cvm::per_note_pitch_bend
MIDI2_UMP_TUPLE(m2cvm, per_note_pitch_bend)  // Define tuple_size and tuple_element for m2cvm/per_note_pitch_bend
MIDI2_SPAN_CTOR(m2cvm, per_note_pitch_bend)  // Define the span constructor for m2cvm/per_note_pitch_bend

//*                       _                       *
//*  _  _ _ __  _ __   __| |_ _ _ ___ __ _ _ __   *
//* | || | '  \| '_ \ (_-<  _| '_/ -_) _` | '  \  *
//*  \_,_|_|_|_| .__/ /__/\__|_| \___\__,_|_|_|_| *
//*            |_|                                *
/// \brief An integral constant which holds the number of 32-bit words in a UMP stream message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::stream> : std::integral_constant<unsigned, 4> {};

/// Defines the C++ types that represent UMP Stream messages
namespace midi2::ump::stream {

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return get<I>(t.words_);
}

class endpoint_discovery;
class endpoint_info_notification;
class device_identity_notification;
class endpoint_name_notification;
class product_instance_id_notification;
class jr_configuration_request;
class jr_configuration_notification;
class function_block_discovery;
class function_block_info_notification;
class function_block_name_notification;
class start_of_clip;
class end_of_clip;

}  // namespace midi2::ump::stream

// 7.1.1 Endpoint Discovery Message
/// \brief The Endpoint Discovery message (section 7.1.1).
class midi2::ump::stream::endpoint_discovery {
public:
  /// \brief The first word of the Endpoint Discovery message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::endpoint_discovery;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x00 (mt::stream::endpoint_discovery).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of the UMP version major field. This is 0x01 for version 1.1 of the UMP specification.
    using version_major = adt::bit_range<8, 8>;
    /// Defines the bit position of the UMP version minor field. This is 0x01 for version 1.1 of the UMP specification.
    using version_minor = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Endpoint Discovery message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines a group of reserved bits.
    using reserved = adt::bit_range<8, 24>;
    /// Defines the bit position of the filter bitmap field. Each bit requests one reply: bit 0 (e) an Endpoint Info
    /// Notification, bit 1 (d) a Device Identity Notification, bit 2 (n) an Endpoint Name Notification, bit 3 (i) a
    /// Product Instance Id Notification, bit 4 (s) a Stream Configuration Notification.
    using filter = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the Endpoint Discovery message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Endpoint Discovery message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Endpoint Discovery message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr endpoint_discovery() noexcept = default;
  /// Constructs a Endpoint Discovery message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit endpoint_discovery(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(endpoint_discovery const&, endpoint_discovery const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::endpoint_discovery::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::endpoint_discovery & midi2::ump::stream::endpoint_discovery::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::endpoint_discovery)
  /// \brief Returns the value of the word0::status field. Always 0x00 (mt::stream::endpoint_discovery).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::endpoint_discovery::version_major() const noexcept
  /// \brief Returns the value of the word0::version_major field.
  /// \returns The value of the word0::version_major field.
  //
  /// \fn constexpr ump::stream::endpoint_discovery & midi2::ump::stream::endpoint_discovery::version_major(adt::uinteger_t<word0::version_major::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::version_major field.
  /// \param v  The value to be assigned to the word0::version_major field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, version_major, ump::stream::endpoint_discovery)
  /// \fn constexpr auto midi2::ump::stream::endpoint_discovery::version_minor() const noexcept
  /// \brief Returns the value of the word0::version_minor field.
  /// \returns The value of the word0::version_minor field.
  //
  /// \fn constexpr ump::stream::endpoint_discovery & midi2::ump::stream::endpoint_discovery::version_minor(adt::uinteger_t<word0::version_minor::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::version_minor field.
  /// \param v  The value to be assigned to the word0::version_minor field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, version_minor, ump::stream::endpoint_discovery)
  /// \fn constexpr auto midi2::ump::stream::endpoint_discovery::filter() const noexcept
  /// \brief Returns the value of the word1::filter field.
  /// \returns The value of the word1::filter field.
  //
  /// \fn constexpr ump::stream::endpoint_discovery & midi2::ump::stream::endpoint_discovery::filter(adt::uinteger_t<word1::filter::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::filter field.
  /// \param v  The value to be assigned to the word1::filter field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, filter, ump::stream::endpoint_discovery)
  /// \fn constexpr auto midi2::ump::stream::endpoint_discovery::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::stream::endpoint_discovery & midi2::ump::stream::endpoint_discovery::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::stream::endpoint_discovery)
  /// \fn constexpr auto midi2::ump::stream::endpoint_discovery::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::stream::endpoint_discovery & midi2::ump::stream::endpoint_discovery::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::stream::endpoint_discovery)

private:
  friend struct ::std::tuple_size<endpoint_discovery>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::stream::endpoint_discovery>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::endpoint_discovery
//
/// \class std::tuple_element<I, ::midi2::ump::stream::endpoint_discovery>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::endpoint_discovery
MIDI2_UMP_TUPLE(stream, endpoint_discovery)  // Define tuple_size and tuple_element for stream/endpoint_discovery
MIDI2_SPAN_CTOR(stream, endpoint_discovery)  // Define the span constructor for stream/endpoint_discovery

// 7.1.2 Endpoint Info Notification Message
/// \brief The Endpoint Info Notification message (section 7.1.2).
class midi2::ump::stream::endpoint_info_notification {
public:
  /// \brief The first word of the Endpoint Info Notification message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::endpoint_info_notification;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x01 (mt::stream::endpoint_info_notification).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of the UMP version major field. This is 0x01 for version 1.1 of the UMP specification.
    using version_major = adt::bit_range<8, 8>;
    /// Defines the bit position of the UMP version minor field. This is 0x01 for version 1.1 of the UMP specification.
    using version_minor = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Endpoint Info Notification message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the static function blocks (S) flag. If set, the endpoint will not change any of the
    /// properties of its function blocks after initial discovery.
    using static_function_blocks = adt::bit_range<31, 1>;
    /// Defines the bit position of the number of function blocks field (0-32).
    using number_function_blocks = adt::bit_range<24, 7>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<10, 14>;
    /// Defines the bit position of the MIDI 2.0 protocol capability (M2) flag. If set, the endpoint supports receiving
    /// and sending the MIDI 2.0 protocol.
    using midi2_protocol_capability = adt::bit_range<9, 1>;
    /// Defines the bit position of the MIDI 1.0 protocol capability (M1) flag. If set, the endpoint supports receiving
    /// and sending the MIDI 1.0 protocol.
    using midi1_protocol_capability = adt::bit_range<8, 1>;
    /// Defines a group of reserved bits.
    using reserved1 = adt::bit_range<2, 6>;
    /// Defines the bit position of the receive JR timestamp capability (RXJR) flag. If set, the endpoint supports
    /// receiving JR timestamps.
    using receive_jr_timestamp_capability = adt::bit_range<1, 1>;
    /// Defines the bit position of the transmit JR timestamp capability (TXJR) flag. If set, the endpoint supports
    /// sending JR timestamps.
    using transmit_jr_timestamp_capability = adt::bit_range<0, 1>;
  };
  /// \brief The third word of the Endpoint Info Notification message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Endpoint Info Notification message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Endpoint Info Notification message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr endpoint_info_notification() noexcept = default;
  /// Constructs a Endpoint Info Notification message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit endpoint_info_notification(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(endpoint_info_notification const&,
                                   endpoint_info_notification const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::endpoint_info_notification)
  /// \brief Returns the value of the word0::status field. Always 0x01 (mt::stream::endpoint_info_notification).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::version_major() const noexcept
  /// \brief Returns the value of the word0::version_major field.
  /// \returns The value of the word0::version_major field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::version_major(adt::uinteger_t<word0::version_major::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::version_major field.
  /// \param v  The value to be assigned to the word0::version_major field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, version_major, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::version_minor() const noexcept
  /// \brief Returns the value of the word0::version_minor field.
  /// \returns The value of the word0::version_minor field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::version_minor(adt::uinteger_t<word0::version_minor::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::version_minor field.
  /// \param v  The value to be assigned to the word0::version_minor field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, version_minor, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::static_function_blocks() const noexcept
  /// \brief Returns the value of the word1::static_function_blocks field.
  /// \returns The value of the word1::static_function_blocks field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::static_function_blocks(adt::uinteger_t<word1::static_function_blocks::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::static_function_blocks field.
  /// \param v  The value to be assigned to the word1::static_function_blocks field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, static_function_blocks, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::number_function_blocks() const noexcept
  /// \brief Returns the value of the word1::number_function_blocks field.
  /// \returns The value of the word1::number_function_blocks field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::number_function_blocks(adt::uinteger_t<word1::number_function_blocks::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::number_function_blocks field.
  /// \param v  The value to be assigned to the word1::number_function_blocks field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, number_function_blocks, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::midi2_protocol_capability() const noexcept
  /// \brief Returns the value of the word1::midi2_protocol_capability field.
  /// \returns The value of the word1::midi2_protocol_capability field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::midi2_protocol_capability(adt::uinteger_t<word1::midi2_protocol_capability::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::midi2_protocol_capability field.
  /// \param v  The value to be assigned to the word1::midi2_protocol_capability field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, midi2_protocol_capability, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::midi1_protocol_capability() const noexcept
  /// \brief Returns the value of the word1::midi1_protocol_capability field.
  /// \returns The value of the word1::midi1_protocol_capability field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::midi1_protocol_capability(adt::uinteger_t<word1::midi1_protocol_capability::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::midi1_protocol_capability field.
  /// \param v  The value to be assigned to the word1::midi1_protocol_capability field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, midi1_protocol_capability, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::receive_jr_timestamp_capability() const noexcept
  /// \brief Returns the value of the word1::receive_jr_timestamp_capability field.
  /// \returns The value of the word1::receive_jr_timestamp_capability field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::receive_jr_timestamp_capability(adt::uinteger_t<word1::receive_jr_timestamp_capability::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::receive_jr_timestamp_capability field.
  /// \param v  The value to be assigned to the word1::receive_jr_timestamp_capability field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, receive_jr_timestamp_capability, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::transmit_jr_timestamp_capability() const noexcept
  /// \brief Returns the value of the word1::transmit_jr_timestamp_capability field.
  /// \returns The value of the word1::transmit_jr_timestamp_capability field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::transmit_jr_timestamp_capability(adt::uinteger_t<word1::transmit_jr_timestamp_capability::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::transmit_jr_timestamp_capability field.
  /// \param v  The value to be assigned to the word1::transmit_jr_timestamp_capability field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, transmit_jr_timestamp_capability, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::stream::endpoint_info_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_info_notification::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::stream::endpoint_info_notification & midi2::ump::stream::endpoint_info_notification::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::stream::endpoint_info_notification)

private:
  friend struct ::std::tuple_size<endpoint_info_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/endpoint_info_notification
/// \class std::tuple_size<::midi2::ump::stream::endpoint_info_notification>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::endpoint_info_notification
//
/// \class std::tuple_element<I, ::midi2::ump::stream::endpoint_info_notification>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::endpoint_info_notification
MIDI2_UMP_TUPLE(stream, endpoint_info_notification)
// Define the span constructor for stream/endpoint_info_notification
MIDI2_SPAN_CTOR(stream, endpoint_info_notification)

// 7.1.3 Device Identity Notification Message
/// \brief The Device Identity Notification message (section 7.1.3).
class midi2::ump::stream::device_identity_notification {
public:
  /// \brief The first word of the Device Identity Notification message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::device_identity_notification;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x02 (mt::stream::device_identity_notification).
    using status = adt::bit_range<16, 10>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<0, 16>;
  };
  /// \brief The second word of the Device Identity Notification message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<24, 8>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<23, 1>;
    /// Defines the bit position of byte 1 of the device manufacturer's system exclusive ID. For a one-byte ID, this
    /// holds the ID and the remaining two bytes are zero.
    using dev_manuf_sysex_id_1 = adt::bit_range<16, 7>;
    /// Defines a reserved bit.
    using reserved2 = adt::bit_range<15, 1>;
    /// Defines the bit position of byte 2 of the device manufacturer's system exclusive ID.
    using dev_manuf_sysex_id_2 = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved3 = adt::bit_range<7, 1>;
    /// Defines the bit position of byte 3 of the device manufacturer's system exclusive ID.
    using dev_manuf_sysex_id_3 = adt::bit_range<0, 7>;
  };
  /// \brief The third word of the Device Identity Notification message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<31, 1>;
    /// Defines the bit position of the least significant 7 bits of the device family.
    using device_family_lsb = adt::bit_range<24, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<23, 1>;
    /// Defines the bit position of the most significant 7 bits of the device family.
    using device_family_msb = adt::bit_range<16, 7>;
    /// Defines a reserved bit.
    using reserved2 = adt::bit_range<15, 1>;
    /// Defines the bit position of the least significant 7 bits of the device family model number.
    using device_family_model_lsb = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved3 = adt::bit_range<7, 1>;
    /// Defines the bit position of the most significant 7 bits of the device family model number.
    using device_family_model_msb = adt::bit_range<0, 7>;
  };
  /// \brief The fourth word of the Device Identity Notification message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines a reserved bit.
    using reserved0 = adt::bit_range<31, 1>;
    /// Defines the bit position of byte 1 of the software revision level.
    using sw_revision_1 = adt::bit_range<24, 7>;
    /// Defines a reserved bit.
    using reserved1 = adt::bit_range<23, 1>;
    /// Defines the bit position of byte 2 of the software revision level.
    using sw_revision_2 = adt::bit_range<16, 7>;
    /// Defines a reserved bit.
    using reserved2 = adt::bit_range<15, 1>;
    /// Defines the bit position of byte 3 of the software revision level.
    using sw_revision_3 = adt::bit_range<8, 7>;
    /// Defines a reserved bit.
    using reserved3 = adt::bit_range<7, 1>;
    /// Defines the bit position of byte 4 of the software revision level.
    using sw_revision_4 = adt::bit_range<0, 7>;
  };

  /// Default constructs a Device Identity Notification message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr device_identity_notification() noexcept = default;
  /// Constructs a Device Identity Notification message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit device_identity_notification(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(device_identity_notification const&,
                                   device_identity_notification const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::device_identity_notification)
  /// \brief Returns the value of the word0::status field. Always 0x02 (mt::stream::device_identity_notification).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::dev_manuf_sysex_id_1() const noexcept
  /// \brief Returns the value of the word1::dev_manuf_sysex_id_1 field.
  /// \returns The value of the word1::dev_manuf_sysex_id_1 field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::dev_manuf_sysex_id_1(adt::uinteger_t<word1::dev_manuf_sysex_id_1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::dev_manuf_sysex_id_1 field.
  /// \param v  The value to be assigned to the word1::dev_manuf_sysex_id_1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_1, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::dev_manuf_sysex_id_2() const noexcept
  /// \brief Returns the value of the word1::dev_manuf_sysex_id_2 field.
  /// \returns The value of the word1::dev_manuf_sysex_id_2 field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::dev_manuf_sysex_id_2(adt::uinteger_t<word1::dev_manuf_sysex_id_2::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::dev_manuf_sysex_id_2 field.
  /// \param v  The value to be assigned to the word1::dev_manuf_sysex_id_2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_2, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::dev_manuf_sysex_id_3() const noexcept
  /// \brief Returns the value of the word1::dev_manuf_sysex_id_3 field.
  /// \returns The value of the word1::dev_manuf_sysex_id_3 field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::dev_manuf_sysex_id_3(adt::uinteger_t<word1::dev_manuf_sysex_id_3::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::dev_manuf_sysex_id_3 field.
  /// \param v  The value to be assigned to the word1::dev_manuf_sysex_id_3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, dev_manuf_sysex_id_3, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::device_family_lsb() const noexcept
  /// \brief Returns the value of the word2::device_family_lsb field.
  /// \returns The value of the word2::device_family_lsb field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::device_family_lsb(adt::uinteger_t<word2::device_family_lsb::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::device_family_lsb field.
  /// \param v  The value to be assigned to the word2::device_family_lsb field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, device_family_lsb, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::device_family_msb() const noexcept
  /// \brief Returns the value of the word2::device_family_msb field.
  /// \returns The value of the word2::device_family_msb field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::device_family_msb(adt::uinteger_t<word2::device_family_msb::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::device_family_msb field.
  /// \param v  The value to be assigned to the word2::device_family_msb field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, device_family_msb, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::device_family_model_lsb() const noexcept
  /// \brief Returns the value of the word2::device_family_model_lsb field.
  /// \returns The value of the word2::device_family_model_lsb field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::device_family_model_lsb(adt::uinteger_t<word2::device_family_model_lsb::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::device_family_model_lsb field.
  /// \param v  The value to be assigned to the word2::device_family_model_lsb field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, device_family_model_lsb, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::device_family_model_msb() const noexcept
  /// \brief Returns the value of the word2::device_family_model_msb field.
  /// \returns The value of the word2::device_family_model_msb field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::device_family_model_msb(adt::uinteger_t<word2::device_family_model_msb::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::device_family_model_msb field.
  /// \param v  The value to be assigned to the word2::device_family_model_msb field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, device_family_model_msb, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::sw_revision_1() const noexcept
  /// \brief Returns the value of the word3::sw_revision_1 field.
  /// \returns The value of the word3::sw_revision_1 field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::sw_revision_1(adt::uinteger_t<word3::sw_revision_1::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::sw_revision_1 field.
  /// \param v  The value to be assigned to the word3::sw_revision_1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, sw_revision_1, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::sw_revision_2() const noexcept
  /// \brief Returns the value of the word3::sw_revision_2 field.
  /// \returns The value of the word3::sw_revision_2 field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::sw_revision_2(adt::uinteger_t<word3::sw_revision_2::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::sw_revision_2 field.
  /// \param v  The value to be assigned to the word3::sw_revision_2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, sw_revision_2, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::sw_revision_3() const noexcept
  /// \brief Returns the value of the word3::sw_revision_3 field.
  /// \returns The value of the word3::sw_revision_3 field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::sw_revision_3(adt::uinteger_t<word3::sw_revision_3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::sw_revision_3 field.
  /// \param v  The value to be assigned to the word3::sw_revision_3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, sw_revision_3, ump::stream::device_identity_notification)
  /// \fn constexpr auto midi2::ump::stream::device_identity_notification::sw_revision_4() const noexcept
  /// \brief Returns the value of the word3::sw_revision_4 field.
  /// \returns The value of the word3::sw_revision_4 field.
  //
  /// \fn constexpr ump::stream::device_identity_notification & midi2::ump::stream::device_identity_notification::sw_revision_4(adt::uinteger_t<word3::sw_revision_4::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::sw_revision_4 field.
  /// \param v  The value to be assigned to the word3::sw_revision_4 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, sw_revision_4, ump::stream::device_identity_notification)

private:
  friend struct ::std::tuple_size<device_identity_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/device_identity_notification
/// \class std::tuple_size<::midi2::ump::stream::device_identity_notification>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::device_identity_notification
//
/// \class std::tuple_element<I, ::midi2::ump::stream::device_identity_notification>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::device_identity_notification
MIDI2_UMP_TUPLE(stream, device_identity_notification)
// Define the span constructor for stream/device_identity_notification
MIDI2_SPAN_CTOR(stream, device_identity_notification)

// 7.1.4 Endpoint Name Notification
/// \brief The Endpoint Name Notification message (section 7.1.4).
class midi2::ump::stream::endpoint_name_notification {
public:
  /// \brief The first word of the Endpoint Name Notification message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::endpoint_name_notification;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x03 (mt::stream::endpoint_name_notification).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of byte 1 of the UTF-8 name.
    using name1 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 2 of the UTF-8 name.
    using name2 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Endpoint Name Notification message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 3 of the UTF-8 name.
    using name3 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 4 of the UTF-8 name.
    using name4 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 5 of the UTF-8 name.
    using name5 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 6 of the UTF-8 name.
    using name6 = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the Endpoint Name Notification message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 7 of the UTF-8 name.
    using name7 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 8 of the UTF-8 name.
    using name8 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 9 of the UTF-8 name.
    using name9 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 10 of the UTF-8 name.
    using name10 = adt::bit_range<0, 8>;
  };
  /// \brief The fourth word of the Endpoint Name Notification message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 11 of the UTF-8 name.
    using name11 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 12 of the UTF-8 name.
    using name12 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 13 of the UTF-8 name.
    using name13 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 14 of the UTF-8 name.
    using name14 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Endpoint Name Notification message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr endpoint_name_notification() noexcept = default;
  /// Constructs a Endpoint Name Notification message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit endpoint_name_notification(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(endpoint_name_notification const&,
                                   endpoint_name_notification const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::endpoint_name_notification)
  /// \brief Returns the value of the word0::status field. Always 0x03 (mt::stream::endpoint_name_notification).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name1() const noexcept
  /// \brief Returns the value of the word0::name1 field.
  /// \returns The value of the word0::name1 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name1(adt::uinteger_t<word0::name1::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::name1 field.
  /// \param v  The value to be assigned to the word0::name1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, name1, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name2() const noexcept
  /// \brief Returns the value of the word0::name2 field.
  /// \returns The value of the word0::name2 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name2(adt::uinteger_t<word0::name2::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::name2 field.
  /// \param v  The value to be assigned to the word0::name2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, name2, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name3() const noexcept
  /// \brief Returns the value of the word1::name3 field.
  /// \returns The value of the word1::name3 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name3(adt::uinteger_t<word1::name3::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::name3 field.
  /// \param v  The value to be assigned to the word1::name3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, name3, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name4() const noexcept
  /// \brief Returns the value of the word1::name4 field.
  /// \returns The value of the word1::name4 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name4(adt::uinteger_t<word1::name4::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::name4 field.
  /// \param v  The value to be assigned to the word1::name4 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, name4, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name5() const noexcept
  /// \brief Returns the value of the word1::name5 field.
  /// \returns The value of the word1::name5 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name5(adt::uinteger_t<word1::name5::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::name5 field.
  /// \param v  The value to be assigned to the word1::name5 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, name5, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name6() const noexcept
  /// \brief Returns the value of the word1::name6 field.
  /// \returns The value of the word1::name6 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name6(adt::uinteger_t<word1::name6::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::name6 field.
  /// \param v  The value to be assigned to the word1::name6 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, name6, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name7() const noexcept
  /// \brief Returns the value of the word2::name7 field.
  /// \returns The value of the word2::name7 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name7(adt::uinteger_t<word2::name7::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::name7 field.
  /// \param v  The value to be assigned to the word2::name7 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, name7, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name8() const noexcept
  /// \brief Returns the value of the word2::name8 field.
  /// \returns The value of the word2::name8 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name8(adt::uinteger_t<word2::name8::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::name8 field.
  /// \param v  The value to be assigned to the word2::name8 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, name8, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name9() const noexcept
  /// \brief Returns the value of the word2::name9 field.
  /// \returns The value of the word2::name9 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name9(adt::uinteger_t<word2::name9::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::name9 field.
  /// \param v  The value to be assigned to the word2::name9 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, name9, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name10() const noexcept
  /// \brief Returns the value of the word2::name10 field.
  /// \returns The value of the word2::name10 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name10(adt::uinteger_t<word2::name10::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::name10 field.
  /// \param v  The value to be assigned to the word2::name10 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, name10, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name11() const noexcept
  /// \brief Returns the value of the word3::name11 field.
  /// \returns The value of the word3::name11 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name11(adt::uinteger_t<word3::name11::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::name11 field.
  /// \param v  The value to be assigned to the word3::name11 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, name11, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name12() const noexcept
  /// \brief Returns the value of the word3::name12 field.
  /// \returns The value of the word3::name12 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name12(adt::uinteger_t<word3::name12::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::name12 field.
  /// \param v  The value to be assigned to the word3::name12 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, name12, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name13() const noexcept
  /// \brief Returns the value of the word3::name13 field.
  /// \returns The value of the word3::name13 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name13(adt::uinteger_t<word3::name13::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::name13 field.
  /// \param v  The value to be assigned to the word3::name13 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, name13, ump::stream::endpoint_name_notification)
  /// \fn constexpr auto midi2::ump::stream::endpoint_name_notification::name14() const noexcept
  /// \brief Returns the value of the word3::name14 field.
  /// \returns The value of the word3::name14 field.
  //
  /// \fn constexpr ump::stream::endpoint_name_notification & midi2::ump::stream::endpoint_name_notification::name14(adt::uinteger_t<word3::name14::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::name14 field.
  /// \param v  The value to be assigned to the word3::name14 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, name14, ump::stream::endpoint_name_notification)

private:
  friend struct ::std::tuple_size<endpoint_name_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/endpoint_name_notification
/// \class std::tuple_size<::midi2::ump::stream::endpoint_name_notification>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::endpoint_name_notification
//
/// \class std::tuple_element<I, ::midi2::ump::stream::endpoint_name_notification>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::endpoint_name_notification
MIDI2_UMP_TUPLE(stream, endpoint_name_notification)
// Define the span constructor for stream/endpoint_name_notification
MIDI2_SPAN_CTOR(stream, endpoint_name_notification)

// 7.1.5 Product Instance ID Notification Message
/// \brief The Product Instance Id Notification message (section 7.1.5).
class midi2::ump::stream::product_instance_id_notification {
public:
  /// \brief The first word of the Product Instance Id Notification message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::product_instance_id_notification;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x04 (mt::stream::product_instance_id_notification).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of byte 1 of the product instance ID.
    using pid1 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 2 of the product instance ID.
    using pid2 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Product Instance Id Notification message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 3 of the product instance ID.
    using pid3 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 4 of the product instance ID.
    using pid4 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 5 of the product instance ID.
    using pid5 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 6 of the product instance ID.
    using pid6 = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the Product Instance Id Notification message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 7 of the product instance ID.
    using pid7 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 8 of the product instance ID.
    using pid8 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 9 of the product instance ID.
    using pid9 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 10 of the product instance ID.
    using pid10 = adt::bit_range<0, 8>;
  };
  /// \brief The fourth word of the Product Instance Id Notification message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 11 of the product instance ID.
    using pid11 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 12 of the product instance ID.
    using pid12 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 13 of the product instance ID.
    using pid13 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 14 of the product instance ID.
    using pid14 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Product Instance Id Notification message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr product_instance_id_notification() noexcept = default;
  /// Constructs a Product Instance Id Notification message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit product_instance_id_notification(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(product_instance_id_notification const&,
                                   product_instance_id_notification const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::product_instance_id_notification)
  /// \brief Returns the value of the word0::status field. Always 0x04 (mt::stream::product_instance_id_notification).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid1() const noexcept
  /// \brief Returns the value of the word0::pid1 field.
  /// \returns The value of the word0::pid1 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid1(adt::uinteger_t<word0::pid1::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::pid1 field.
  /// \param v  The value to be assigned to the word0::pid1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, pid1, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid2() const noexcept
  /// \brief Returns the value of the word0::pid2 field.
  /// \returns The value of the word0::pid2 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid2(adt::uinteger_t<word0::pid2::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::pid2 field.
  /// \param v  The value to be assigned to the word0::pid2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, pid2, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid3() const noexcept
  /// \brief Returns the value of the word1::pid3 field.
  /// \returns The value of the word1::pid3 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid3(adt::uinteger_t<word1::pid3::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::pid3 field.
  /// \param v  The value to be assigned to the word1::pid3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, pid3, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid4() const noexcept
  /// \brief Returns the value of the word1::pid4 field.
  /// \returns The value of the word1::pid4 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid4(adt::uinteger_t<word1::pid4::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::pid4 field.
  /// \param v  The value to be assigned to the word1::pid4 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, pid4, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid5() const noexcept
  /// \brief Returns the value of the word1::pid5 field.
  /// \returns The value of the word1::pid5 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid5(adt::uinteger_t<word1::pid5::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::pid5 field.
  /// \param v  The value to be assigned to the word1::pid5 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, pid5, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid6() const noexcept
  /// \brief Returns the value of the word1::pid6 field.
  /// \returns The value of the word1::pid6 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid6(adt::uinteger_t<word1::pid6::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::pid6 field.
  /// \param v  The value to be assigned to the word1::pid6 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, pid6, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid7() const noexcept
  /// \brief Returns the value of the word2::pid7 field.
  /// \returns The value of the word2::pid7 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid7(adt::uinteger_t<word2::pid7::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::pid7 field.
  /// \param v  The value to be assigned to the word2::pid7 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, pid7, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid8() const noexcept
  /// \brief Returns the value of the word2::pid8 field.
  /// \returns The value of the word2::pid8 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid8(adt::uinteger_t<word2::pid8::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::pid8 field.
  /// \param v  The value to be assigned to the word2::pid8 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, pid8, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid9() const noexcept
  /// \brief Returns the value of the word2::pid9 field.
  /// \returns The value of the word2::pid9 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid9(adt::uinteger_t<word2::pid9::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::pid9 field.
  /// \param v  The value to be assigned to the word2::pid9 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, pid9, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid10() const noexcept
  /// \brief Returns the value of the word2::pid10 field.
  /// \returns The value of the word2::pid10 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid10(adt::uinteger_t<word2::pid10::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::pid10 field.
  /// \param v  The value to be assigned to the word2::pid10 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, pid10, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid11() const noexcept
  /// \brief Returns the value of the word3::pid11 field.
  /// \returns The value of the word3::pid11 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid11(adt::uinteger_t<word3::pid11::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::pid11 field.
  /// \param v  The value to be assigned to the word3::pid11 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, pid11, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid12() const noexcept
  /// \brief Returns the value of the word3::pid12 field.
  /// \returns The value of the word3::pid12 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid12(adt::uinteger_t<word3::pid12::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::pid12 field.
  /// \param v  The value to be assigned to the word3::pid12 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, pid12, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid13() const noexcept
  /// \brief Returns the value of the word3::pid13 field.
  /// \returns The value of the word3::pid13 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid13(adt::uinteger_t<word3::pid13::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::pid13 field.
  /// \param v  The value to be assigned to the word3::pid13 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, pid13, ump::stream::product_instance_id_notification)
  /// \fn constexpr auto midi2::ump::stream::product_instance_id_notification::pid14() const noexcept
  /// \brief Returns the value of the word3::pid14 field.
  /// \returns The value of the word3::pid14 field.
  //
  /// \fn constexpr ump::stream::product_instance_id_notification & midi2::ump::stream::product_instance_id_notification::pid14(adt::uinteger_t<word3::pid14::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::pid14 field.
  /// \param v  The value to be assigned to the word3::pid14 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, pid14, ump::stream::product_instance_id_notification)

private:
  friend struct ::std::tuple_size<product_instance_id_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/product_instance_id_notification
/// \class std::tuple_size<::midi2::ump::stream::product_instance_id_notification>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::product_instance_id_notification
//
/// \class std::tuple_element<I, ::midi2::ump::stream::product_instance_id_notification>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::product_instance_id_notification
MIDI2_UMP_TUPLE(stream, product_instance_id_notification)
// Define the span constructor for stream/product_instance_id_notification
MIDI2_SPAN_CTOR(stream, product_instance_id_notification)

// 7.1.6 Selecting a MIDI Protocol and Jitter Reduction Timestamps for a UMP Stream
// 7.1.6.1 Steps to Select Protocol and Jitter Reduction Timestamps
// 7.1.6.2 JR Stream Configuration Request
/// \brief The Stream Configuration Request message (section 7.1.6.2).
class midi2::ump::stream::jr_configuration_request {
public:
  /// \brief The first word of the Stream Configuration Request message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::jr_configuration_request;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x05 (mt::stream::jr_configuration_request).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of the protocol field: 0x01 MIDI 1.0 protocol, 0x02 MIDI 2.0 protocol. Other values are
    /// reserved.
    using protocol = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<2, 6>;
    /// Defines the bit position of the receive JR timestamp (RXJR) flag. If set, the receiver of this message can
    /// expect incoming messages to be prefixed with JR timestamps.
    using rxjr = adt::bit_range<1, 1>;
    /// Defines the bit position of the transmit JR timestamp (TXJR) flag. If set, the receiver of this message is asked
    /// to prefix all messages with JR timestamps.
    using txjr = adt::bit_range<0, 1>;
  };
  /// \brief The second word of the Stream Configuration Request message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word1::value1 field. This word is reserved and should be zero.
    using value1 = adt::bit_range<0, 32>;
  };
  /// \brief The third word of the Stream Configuration Request message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Stream Configuration Request message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Stream Configuration Request message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr jr_configuration_request() noexcept = default;
  /// Constructs a Stream Configuration Request message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit jr_configuration_request(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(jr_configuration_request const&, jr_configuration_request const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_request::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::jr_configuration_request & midi2::ump::stream::jr_configuration_request::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::jr_configuration_request)
  /// \brief Returns the value of the word0::status field. Always 0x05 (mt::stream::jr_configuration_request).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_request::protocol() const noexcept
  /// \brief Returns the value of the word0::protocol field.
  /// \returns The value of the word0::protocol field.
  //
  /// \fn constexpr ump::stream::jr_configuration_request & midi2::ump::stream::jr_configuration_request::protocol(adt::uinteger_t<word0::protocol::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::protocol field.
  /// \param v  The value to be assigned to the word0::protocol field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, protocol, ump::stream::jr_configuration_request)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_request::rxjr() const noexcept
  /// \brief Returns the value of the word0::rxjr field.
  /// \returns The value of the word0::rxjr field.
  //
  /// \fn constexpr ump::stream::jr_configuration_request & midi2::ump::stream::jr_configuration_request::rxjr(adt::uinteger_t<word0::rxjr::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::rxjr field.
  /// \param v  The value to be assigned to the word0::rxjr field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, rxjr, ump::stream::jr_configuration_request)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_request::txjr() const noexcept
  /// \brief Returns the value of the word0::txjr field.
  /// \returns The value of the word0::txjr field.
  //
  /// \fn constexpr ump::stream::jr_configuration_request & midi2::ump::stream::jr_configuration_request::txjr(adt::uinteger_t<word0::txjr::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::txjr field.
  /// \param v  The value to be assigned to the word0::txjr field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, txjr, ump::stream::jr_configuration_request)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_request::value1() const noexcept
  /// \brief Returns the value of the word1::value1 field.
  /// \returns The value of the word1::value1 field.
  //
  /// \fn constexpr ump::stream::jr_configuration_request & midi2::ump::stream::jr_configuration_request::value1(adt::uinteger_t<word1::value1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value1 field.
  /// \param v  The value to be assigned to the word1::value1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value1, ump::stream::jr_configuration_request)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_request::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::stream::jr_configuration_request & midi2::ump::stream::jr_configuration_request::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::stream::jr_configuration_request)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_request::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::stream::jr_configuration_request & midi2::ump::stream::jr_configuration_request::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::stream::jr_configuration_request)

private:
  friend struct ::std::tuple_size<jr_configuration_request>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/jr_configuration_request
/// \class std::tuple_size<::midi2::ump::stream::jr_configuration_request>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::jr_configuration_request
//
/// \class std::tuple_element<I, ::midi2::ump::stream::jr_configuration_request>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::jr_configuration_request
MIDI2_UMP_TUPLE(stream, jr_configuration_request)
// Define the span constructor for stream/jr_configuration_request
MIDI2_SPAN_CTOR(stream, jr_configuration_request)

// 7.1.6.3 JR Stream Configuration Notification Message
/// \brief The Stream Configuration Notification message (section 7.1.6.3).
class midi2::ump::stream::jr_configuration_notification {
public:
  /// \brief The first word of the Stream Configuration Notification message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::jr_configuration_notification;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x06 (mt::stream::jr_configuration_notification).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of the protocol field: 0x01 MIDI 1.0 protocol, 0x02 MIDI 2.0 protocol. Other values are
    /// reserved.
    using protocol = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<2, 6>;
    /// Defines the bit position of the receive JR timestamp (RXJR) flag. If set, the sender of this message expects
    /// messages sent to it to include JR timestamps.
    using rxjr = adt::bit_range<1, 1>;
    /// Defines the bit position of the transmit JR timestamp (TXJR) flag. If set, the sender of this message will send
    /// all messages with JR timestamps.
    using txjr = adt::bit_range<0, 1>;
  };
  /// \brief The second word of the Stream Configuration Notification message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word1::value1 field. This word is reserved and should be zero.
    using value1 = adt::bit_range<0, 32>;
  };
  /// \brief The third word of the Stream Configuration Notification message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Stream Configuration Notification message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Stream Configuration Notification message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr jr_configuration_notification() noexcept = default;
  /// Constructs a Stream Configuration Notification message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit jr_configuration_notification(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(jr_configuration_notification const&,
                                   jr_configuration_notification const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_notification::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::jr_configuration_notification & midi2::ump::stream::jr_configuration_notification::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::jr_configuration_notification)
  /// \brief Returns the value of the word0::status field. Always 0x06 (mt::stream::jr_configuration_notification).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_notification::protocol() const noexcept
  /// \brief Returns the value of the word0::protocol field.
  /// \returns The value of the word0::protocol field.
  //
  /// \fn constexpr ump::stream::jr_configuration_notification & midi2::ump::stream::jr_configuration_notification::protocol(adt::uinteger_t<word0::protocol::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::protocol field.
  /// \param v  The value to be assigned to the word0::protocol field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, protocol, ump::stream::jr_configuration_notification)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_notification::rxjr() const noexcept
  /// \brief Returns the value of the word0::rxjr field.
  /// \returns The value of the word0::rxjr field.
  //
  /// \fn constexpr ump::stream::jr_configuration_notification & midi2::ump::stream::jr_configuration_notification::rxjr(adt::uinteger_t<word0::rxjr::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::rxjr field.
  /// \param v  The value to be assigned to the word0::rxjr field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, rxjr, ump::stream::jr_configuration_notification)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_notification::txjr() const noexcept
  /// \brief Returns the value of the word0::txjr field.
  /// \returns The value of the word0::txjr field.
  //
  /// \fn constexpr ump::stream::jr_configuration_notification & midi2::ump::stream::jr_configuration_notification::txjr(adt::uinteger_t<word0::txjr::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::txjr field.
  /// \param v  The value to be assigned to the word0::txjr field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, txjr, ump::stream::jr_configuration_notification)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_notification::value1() const noexcept
  /// \brief Returns the value of the word1::value1 field.
  /// \returns The value of the word1::value1 field.
  //
  /// \fn constexpr ump::stream::jr_configuration_notification & midi2::ump::stream::jr_configuration_notification::value1(adt::uinteger_t<word1::value1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value1 field.
  /// \param v  The value to be assigned to the word1::value1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value1, ump::stream::jr_configuration_notification)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_notification::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::stream::jr_configuration_notification & midi2::ump::stream::jr_configuration_notification::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::stream::jr_configuration_notification)
  /// \fn constexpr auto midi2::ump::stream::jr_configuration_notification::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::stream::jr_configuration_notification & midi2::ump::stream::jr_configuration_notification::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::stream::jr_configuration_notification)

private:
  friend struct ::std::tuple_size<jr_configuration_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/jr_configuration_notification
/// \class std::tuple_size<::midi2::ump::stream::jr_configuration_notification>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::jr_configuration_notification
//
/// \class std::tuple_element<I, ::midi2::ump::stream::jr_configuration_notification>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::jr_configuration_notification
MIDI2_UMP_TUPLE(stream, jr_configuration_notification)
// Define the span constructor for stream/jr_configuration_notification
MIDI2_SPAN_CTOR(stream, jr_configuration_notification)

// 7.1.7 Function Block Discovery Message
/// \brief The Function Block Discovery message (section 7.1.7).
class midi2::ump::stream::function_block_discovery {
public:
  /// \brief The first word of the Function Block Discovery message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::function_block_discovery;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x10 (mt::stream::function_block_discovery).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of the function block number field (0-31). 0xFF requests information about all function
    /// blocks.
    using block_num = adt::bit_range<8, 8>;
    /// Defines the bit position of the filter bitmap field. Each bit requests one reply: bit 0 (i) a Function Block
    /// Info Notification, bit 1 (n) a Function Block Name Notification.
    using filter = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Function Block Discovery message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word1::value1 field. This word is reserved and should be zero.
    using value1 = adt::bit_range<0, 32>;
  };
  /// \brief The third word of the Function Block Discovery message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Function Block Discovery message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Function Block Discovery message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr function_block_discovery() noexcept = default;
  /// Constructs a Function Block Discovery message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit function_block_discovery(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(function_block_discovery const&, function_block_discovery const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::function_block_discovery::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::function_block_discovery & midi2::ump::stream::function_block_discovery::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::function_block_discovery)
  /// \brief Returns the value of the word0::status field. Always 0x10 (mt::stream::function_block_discovery).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::function_block_discovery::block_num() const noexcept
  /// \brief Returns the value of the word0::block_num field.
  /// \returns The value of the word0::block_num field.
  //
  /// \fn constexpr ump::stream::function_block_discovery & midi2::ump::stream::function_block_discovery::block_num(adt::uinteger_t<word0::block_num::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::block_num field.
  /// \param v  The value to be assigned to the word0::block_num field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, block_num, ump::stream::function_block_discovery)
  /// \fn constexpr auto midi2::ump::stream::function_block_discovery::filter() const noexcept
  /// \brief Returns the value of the word0::filter field.
  /// \returns The value of the word0::filter field.
  //
  /// \fn constexpr ump::stream::function_block_discovery & midi2::ump::stream::function_block_discovery::filter(adt::uinteger_t<word0::filter::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::filter field.
  /// \param v  The value to be assigned to the word0::filter field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, filter, ump::stream::function_block_discovery)
  /// \fn constexpr auto midi2::ump::stream::function_block_discovery::value1() const noexcept
  /// \brief Returns the value of the word1::value1 field.
  /// \returns The value of the word1::value1 field.
  //
  /// \fn constexpr ump::stream::function_block_discovery & midi2::ump::stream::function_block_discovery::value1(adt::uinteger_t<word1::value1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value1 field.
  /// \param v  The value to be assigned to the word1::value1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value1, ump::stream::function_block_discovery)
  /// \fn constexpr auto midi2::ump::stream::function_block_discovery::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::stream::function_block_discovery & midi2::ump::stream::function_block_discovery::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::stream::function_block_discovery)
  /// \fn constexpr auto midi2::ump::stream::function_block_discovery::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::stream::function_block_discovery & midi2::ump::stream::function_block_discovery::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::stream::function_block_discovery)

private:
  friend struct ::std::tuple_size<function_block_discovery>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/function_block_discovery
/// \class std::tuple_size<::midi2::ump::stream::function_block_discovery>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::function_block_discovery
//
/// \class std::tuple_element<I, ::midi2::ump::stream::function_block_discovery>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::function_block_discovery
MIDI2_UMP_TUPLE(stream, function_block_discovery)
// Define the span constructor for stream/function_block_discovery
MIDI2_SPAN_CTOR(stream, function_block_discovery)

// 7.1.8 Function Block Info Notification
/// \brief The Function Block Info Notification message (section 7.1.8).
class midi2::ump::stream::function_block_info_notification {
public:
  /// \brief The first word of the Function Block Info Notification message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::function_block_info_notification;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x11 (mt::stream::function_block_info_notification).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of the function block active (a) flag. If clear, the function block is inactive and can
    /// be ignored.
    using block_active = adt::bit_range<15, 1>;
    /// Defines the bit position of the function block number field (0-31).
    using block_num = adt::bit_range<8, 7>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<6, 2>;
    /// Defines the bit position of the user interface hint field: 0b00 unknown or undeclared; 0b01 primarily a receiver
    /// (destination); 0b10 primarily a sender (source); 0b11 both a sender and a receiver.
    using ui_hint = adt::bit_range<4, 2>;
    /// Defines the bit position of the MIDI 1.0 field: 0x0 not MIDI 1.0; 0x1 MIDI 1.0, don't restrict bandwidth; 0x2
    /// MIDI 1.0, restrict bandwidth to 31.25Kbps; 0x3 reserved.
    using midi1 = adt::bit_range<2, 2>;
    /// Defines the bit position of the direction field: 0b00 reserved; 0b01 input (receives MIDI messages only); 0b10
    /// output (transmits MIDI messages only); 0b11 bidirectional.
    using direction = adt::bit_range<0, 2>;
  };
  /// \brief The second word of the Function Block Info Notification message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the first group field: the lowest numbered group (0x0-0xF) which is a member of this
    /// function block.
    using first_group = adt::bit_range<24, 8>;
    /// Defines the bit position of the number of groups spanned field (1-16).
    using num_spanned = adt::bit_range<16, 8>;
    /// Defines the bit position of the MIDI-CI message version/format field. 0x00 means none or unknown.
    using ci_message_version = adt::bit_range<8, 8>;
    /// Defines the bit position of the maximum number of SysEx8 streams field: the number of simultaneous System
    /// Exclusive 8 stream IDs supported (0 means that SysEx8 is not supported).
    using max_sys8_streams = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the Function Block Info Notification message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Function Block Info Notification message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Function Block Info Notification message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr function_block_info_notification() noexcept = default;
  /// Constructs a Function Block Info Notification message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit function_block_info_notification(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(function_block_info_notification const&,
                                   function_block_info_notification const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::function_block_info_notification)
  /// \brief Returns the value of the word0::status field. Always 0x11 (mt::stream::function_block_info_notification).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::block_active() const noexcept
  /// \brief Returns the value of the word0::block_active field.
  /// \returns The value of the word0::block_active field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::block_active(adt::uinteger_t<word0::block_active::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::block_active field.
  /// \param v  The value to be assigned to the word0::block_active field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, block_active, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::block_num() const noexcept
  /// \brief Returns the value of the word0::block_num field.
  /// \returns The value of the word0::block_num field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::block_num(adt::uinteger_t<word0::block_num::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::block_num field.
  /// \param v  The value to be assigned to the word0::block_num field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, block_num, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::ui_hint() const noexcept
  /// \brief Returns the value of the word0::ui_hint field.
  /// \returns The value of the word0::ui_hint field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::ui_hint(adt::uinteger_t<word0::ui_hint::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::ui_hint field.
  /// \param v  The value to be assigned to the word0::ui_hint field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, ui_hint, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::midi1() const noexcept
  /// \brief Returns the value of the word0::midi1 field.
  /// \returns The value of the word0::midi1 field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::midi1(adt::uinteger_t<word0::midi1::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::midi1 field.
  /// \param v  The value to be assigned to the word0::midi1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, midi1, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::direction() const noexcept
  /// \brief Returns the value of the word0::direction field.
  /// \returns The value of the word0::direction field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::direction(adt::uinteger_t<word0::direction::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::direction field.
  /// \param v  The value to be assigned to the word0::direction field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, direction, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::first_group() const noexcept
  /// \brief Returns the value of the word1::first_group field.
  /// \returns The value of the word1::first_group field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::first_group(adt::uinteger_t<word1::first_group::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::first_group field.
  /// \param v  The value to be assigned to the word1::first_group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, first_group, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::num_spanned() const noexcept
  /// \brief Returns the value of the word1::num_spanned field.
  /// \returns The value of the word1::num_spanned field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::num_spanned(adt::uinteger_t<word1::num_spanned::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::num_spanned field.
  /// \param v  The value to be assigned to the word1::num_spanned field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, num_spanned, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::ci_message_version() const noexcept
  /// \brief Returns the value of the word1::ci_message_version field.
  /// \returns The value of the word1::ci_message_version field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::ci_message_version(adt::uinteger_t<word1::ci_message_version::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::ci_message_version field.
  /// \param v  The value to be assigned to the word1::ci_message_version field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, ci_message_version, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::max_sys8_streams() const noexcept
  /// \brief Returns the value of the word1::max_sys8_streams field.
  /// \returns The value of the word1::max_sys8_streams field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::max_sys8_streams(adt::uinteger_t<word1::max_sys8_streams::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::max_sys8_streams field.
  /// \param v  The value to be assigned to the word1::max_sys8_streams field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, max_sys8_streams, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::stream::function_block_info_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_info_notification::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::stream::function_block_info_notification & midi2::ump::stream::function_block_info_notification::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::stream::function_block_info_notification)

private:
  friend struct ::std::tuple_size<function_block_info_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/function_block_info_notification
/// \class std::tuple_size<::midi2::ump::stream::function_block_info_notification>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::function_block_info_notification
//
/// \class std::tuple_element<I, ::midi2::ump::stream::function_block_info_notification>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::function_block_info_notification
MIDI2_UMP_TUPLE(stream, function_block_info_notification)
// Define the span constructor for stream/function_block_info_notification
MIDI2_SPAN_CTOR(stream, function_block_info_notification)

// 7.1.9 Function Block Name Notification
/// \brief The Function Block Name Notification message (section 7.1.9).
class midi2::ump::stream::function_block_name_notification {
public:
  /// \brief The first word of the Function Block Name Notification message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::function_block_name_notification;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x12 (mt::stream::function_block_name_notification).
    using status = adt::bit_range<16, 10>;
    /// Defines the bit position of the function block number field (0-31).
    using block_num = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 0 of the UTF-8 name.
    using name0 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Function Block Name Notification message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 1 of the UTF-8 name.
    using name1 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 2 of the UTF-8 name.
    using name2 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 3 of the UTF-8 name.
    using name3 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 4 of the UTF-8 name.
    using name4 = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the Function Block Name Notification message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 5 of the UTF-8 name.
    using name5 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 6 of the UTF-8 name.
    using name6 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 7 of the UTF-8 name.
    using name7 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 8 of the UTF-8 name.
    using name8 = adt::bit_range<0, 8>;
  };
  /// \brief The fourth word of the Function Block Name Notification message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 9 of the UTF-8 name.
    using name9 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 10 of the UTF-8 name.
    using name10 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 11 of the UTF-8 name.
    using name11 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 12 of the UTF-8 name.
    using name12 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Function Block Name Notification message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr function_block_name_notification() noexcept = default;
  /// Constructs a Function Block Name Notification message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit function_block_name_notification(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(function_block_name_notification const&,
                                   function_block_name_notification const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::function_block_name_notification)
  /// \brief Returns the value of the word0::status field. Always 0x12 (mt::stream::function_block_name_notification).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::block_num() const noexcept
  /// \brief Returns the value of the word0::block_num field.
  /// \returns The value of the word0::block_num field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::block_num(adt::uinteger_t<word0::block_num::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::block_num field.
  /// \param v  The value to be assigned to the word0::block_num field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, block_num, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name0() const noexcept
  /// \brief Returns the value of the word0::name0 field.
  /// \returns The value of the word0::name0 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name0(adt::uinteger_t<word0::name0::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::name0 field.
  /// \param v  The value to be assigned to the word0::name0 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, name0, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name1() const noexcept
  /// \brief Returns the value of the word1::name1 field.
  /// \returns The value of the word1::name1 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name1(adt::uinteger_t<word1::name1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::name1 field.
  /// \param v  The value to be assigned to the word1::name1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, name1, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name2() const noexcept
  /// \brief Returns the value of the word1::name2 field.
  /// \returns The value of the word1::name2 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name2(adt::uinteger_t<word1::name2::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::name2 field.
  /// \param v  The value to be assigned to the word1::name2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, name2, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name3() const noexcept
  /// \brief Returns the value of the word1::name3 field.
  /// \returns The value of the word1::name3 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name3(adt::uinteger_t<word1::name3::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::name3 field.
  /// \param v  The value to be assigned to the word1::name3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, name3, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name4() const noexcept
  /// \brief Returns the value of the word1::name4 field.
  /// \returns The value of the word1::name4 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name4(adt::uinteger_t<word1::name4::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::name4 field.
  /// \param v  The value to be assigned to the word1::name4 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, name4, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name5() const noexcept
  /// \brief Returns the value of the word2::name5 field.
  /// \returns The value of the word2::name5 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name5(adt::uinteger_t<word2::name5::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::name5 field.
  /// \param v  The value to be assigned to the word2::name5 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, name5, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name6() const noexcept
  /// \brief Returns the value of the word2::name6 field.
  /// \returns The value of the word2::name6 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name6(adt::uinteger_t<word2::name6::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::name6 field.
  /// \param v  The value to be assigned to the word2::name6 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, name6, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name7() const noexcept
  /// \brief Returns the value of the word2::name7 field.
  /// \returns The value of the word2::name7 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name7(adt::uinteger_t<word2::name7::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::name7 field.
  /// \param v  The value to be assigned to the word2::name7 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, name7, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name8() const noexcept
  /// \brief Returns the value of the word2::name8 field.
  /// \returns The value of the word2::name8 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name8(adt::uinteger_t<word2::name8::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::name8 field.
  /// \param v  The value to be assigned to the word2::name8 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, name8, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name9() const noexcept
  /// \brief Returns the value of the word3::name9 field.
  /// \returns The value of the word3::name9 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name9(adt::uinteger_t<word3::name9::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::name9 field.
  /// \param v  The value to be assigned to the word3::name9 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, name9, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name10() const noexcept
  /// \brief Returns the value of the word3::name10 field.
  /// \returns The value of the word3::name10 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name10(adt::uinteger_t<word3::name10::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::name10 field.
  /// \param v  The value to be assigned to the word3::name10 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, name10, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name11() const noexcept
  /// \brief Returns the value of the word3::name11 field.
  /// \returns The value of the word3::name11 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name11(adt::uinteger_t<word3::name11::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::name11 field.
  /// \param v  The value to be assigned to the word3::name11 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, name11, ump::stream::function_block_name_notification)
  /// \fn constexpr auto midi2::ump::stream::function_block_name_notification::name12() const noexcept
  /// \brief Returns the value of the word3::name12 field.
  /// \returns The value of the word3::name12 field.
  //
  /// \fn constexpr ump::stream::function_block_name_notification & midi2::ump::stream::function_block_name_notification::name12(adt::uinteger_t<word3::name12::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::name12 field.
  /// \param v  The value to be assigned to the word3::name12 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, name12, ump::stream::function_block_name_notification)

private:
  friend struct ::std::tuple_size<function_block_name_notification>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
// Define tuple_size and tuple_element for stream/function_block_name_notification
/// \class std::tuple_size<::midi2::ump::stream::function_block_name_notification>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::function_block_name_notification
//
/// \class std::tuple_element<I, ::midi2::ump::stream::function_block_name_notification>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::function_block_name_notification
MIDI2_UMP_TUPLE(stream, function_block_name_notification)
// Define the span constructor for stream/function_block_name_notification
MIDI2_SPAN_CTOR(stream, function_block_name_notification)

// 7.1.10 Start of Clip Message
/// \brief The Start of Clip message (section 7.1.10).
class midi2::ump::stream::start_of_clip {
public:
  /// \brief The first word of the Start of Clip message.
  class word0 : public details::word_base {
  public:
    /// The status value which identifies this message.
    static constexpr auto message = mt::stream::start_of_clip;
    using word_base::word_base;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x20 (mt::stream::start_of_clip).
    using status = adt::bit_range<16, 10>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<0, 16>;
  };
  /// \brief The second word of the Start of Clip message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word1::value1 field. This word is reserved and should be zero.
    using value1 = adt::bit_range<0, 32>;
  };
  /// \brief The third word of the Start of Clip message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Start of Clip message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Start of Clip message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr start_of_clip() noexcept = default;
  /// Constructs a Start of Clip message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit start_of_clip(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(start_of_clip const&, start_of_clip const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::start_of_clip::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::start_of_clip & midi2::ump::stream::start_of_clip::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::start_of_clip)
  /// \brief Returns the value of the word0::status field. Always 0x20 (mt::stream::start_of_clip).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::start_of_clip::value1() const noexcept
  /// \brief Returns the value of the word1::value1 field.
  /// \returns The value of the word1::value1 field.
  //
  /// \fn constexpr ump::stream::start_of_clip & midi2::ump::stream::start_of_clip::value1(adt::uinteger_t<word1::value1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value1 field.
  /// \param v  The value to be assigned to the word1::value1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value1, ump::stream::start_of_clip)
  /// \fn constexpr auto midi2::ump::stream::start_of_clip::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::stream::start_of_clip & midi2::ump::stream::start_of_clip::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::stream::start_of_clip)
  /// \fn constexpr auto midi2::ump::stream::start_of_clip::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::stream::start_of_clip & midi2::ump::stream::start_of_clip::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::stream::start_of_clip)

private:
  friend struct ::std::tuple_size<start_of_clip>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::stream::start_of_clip>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::start_of_clip
//
/// \class std::tuple_element<I, ::midi2::ump::stream::start_of_clip>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::start_of_clip
MIDI2_UMP_TUPLE(stream, start_of_clip)  // Define tuple_size and tuple_element for stream/start_of_clip
MIDI2_SPAN_CTOR(stream, start_of_clip)  // Define the span constructor for stream/start_of_clip

// 7.1.11 End of Clip Message
/// \brief The End of Clip message (section 7.1.11).
class midi2::ump::stream::end_of_clip {
public:
  /// \brief The first word of the End of Clip message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::stream::end_of_clip;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xF (message_type::stream).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the format field: 0x0 a complete message in one UMP; 0x1 the start of a message
    /// which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans two or
    /// more UMPs.
    using format = adt::bit_range<26, 2>;
    /// Defines the bit position of the status field. Always 0x21 (mt::stream::end_of_clip).
    using status = adt::bit_range<16, 10>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<0, 16>;
  };
  /// \brief The second word of the End of Clip message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word1::value1 field. This word is reserved and should be zero.
    using value1 = adt::bit_range<0, 32>;
  };
  /// \brief The third word of the End of Clip message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the End of Clip message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a End of Clip message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr end_of_clip() noexcept = default;
  /// Constructs a End of Clip message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit end_of_clip(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(end_of_clip const&, end_of_clip const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xF (message_type::stream).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::stream::end_of_clip::format() const noexcept
  /// \brief Returns the value of the word0::format field.
  /// \returns The value of the word0::format field.
  //
  /// \fn constexpr ump::stream::end_of_clip & midi2::ump::stream::end_of_clip::format(adt::uinteger_t<word0::format::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::format field.
  /// \param v  The value to be assigned to the word0::format field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, format, ump::stream::end_of_clip)
  /// \brief Returns the value of the word0::status field. Always 0x21 (mt::stream::end_of_clip).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::stream::end_of_clip::value1() const noexcept
  /// \brief Returns the value of the word1::value1 field.
  /// \returns The value of the word1::value1 field.
  //
  /// \fn constexpr ump::stream::end_of_clip & midi2::ump::stream::end_of_clip::value1(adt::uinteger_t<word1::value1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value1 field.
  /// \param v  The value to be assigned to the word1::value1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value1, ump::stream::end_of_clip)
  /// \fn constexpr auto midi2::ump::stream::end_of_clip::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::stream::end_of_clip & midi2::ump::stream::end_of_clip::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::stream::end_of_clip)
  /// \fn constexpr auto midi2::ump::stream::end_of_clip::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::stream::end_of_clip & midi2::ump::stream::end_of_clip::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::stream::end_of_clip)

private:
  friend struct ::std::tuple_size<end_of_clip>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::stream::end_of_clip>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::stream::end_of_clip
//
/// \class std::tuple_element<I, ::midi2::ump::stream::end_of_clip>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::stream::end_of_clip
MIDI2_UMP_TUPLE(stream, end_of_clip)  // Define tuple_size and tuple_element for stream/end_of_clip
MIDI2_SPAN_CTOR(stream, end_of_clip)  // Define the span constructor for stream/end_of_clip

//*   __ _              _      _         *
//*  / _| |_____ __  __| |__ _| |_ __ _  *
//* |  _| / -_) \ / / _` / _` |  _/ _` | *
//* |_| |_\___/_\_\ \__,_\__,_|\__\__,_| *
//*                                      *

/// \brief An integral constant which holds the number of 32-bit words in a UMP flex_data message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::flex_data> : std::integral_constant<unsigned, 4> {};

/// Defines the C++ types that represent Flex Data messages
namespace midi2::ump::flex_data {

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return get<I>(t.words_);
}

class set_tempo;
class set_time_signature;
class set_metronome;
class set_key_signature;
class set_chord_name;
class text_common;

}  // namespace midi2::ump::flex_data

// 7.5.3 Set Tempo Message
/// \brief The Set Tempo message (section 7.5.3).
class midi2::ump::flex_data::set_tempo {
public:
  /// \brief The first word of the Set Tempo message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::flex_data::set_tempo;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD (message_type::flex_data).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the format (form) field: 0x0 a complete message in one UMP; 0x1 the start of a
    /// message which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans
    /// two or more UMPs.
    using form = adt::bit_range<22, 2>;
    /// Defines the bit position of the address field: 0x0 the message is sent to the channel in the channel field; 0x1
    /// the message is sent to the group (the channel field is ignored). Other values are reserved.
    using address = adt::bit_range<20, 2>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is sent if the
    /// address field is 0. Otherwise, it is reserved and should be 0.
    using channel = adt::bit_range<16, 4>;
    /// Defines the bit position of the status bank field. 0x00 for setup and performance events, 0x01 for metadata
    /// text, 0x02 for performance text events.
    using status_bank = adt::bit_range<8, 8>;
    /// Defines the bit position of the status field. Always 0x00 (mt::flex_data::set_tempo).
    using status = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Set Tempo message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the tempo field: the number of 10 nanosecond units per quarter note.
    using value1 = adt::bit_range<0, 32>;
  };
  /// \brief The third word of the Set Tempo message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Set Tempo message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Set Tempo message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr set_tempo() noexcept = default;
  /// Constructs a Set Tempo message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit set_tempo(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(set_tempo const&, set_tempo const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xD (message_type::flex_data).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::flex_data::set_tempo::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::flex_data::set_tempo & midi2::ump::flex_data::set_tempo::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::flex_data::set_tempo)
  /// \fn constexpr auto midi2::ump::flex_data::set_tempo::form() const noexcept
  /// \brief Returns the value of the word0::form field.
  /// \returns The value of the word0::form field.
  //
  /// \fn constexpr ump::flex_data::set_tempo & midi2::ump::flex_data::set_tempo::form(adt::uinteger_t<word0::form::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::form field.
  /// \param v  The value to be assigned to the word0::form field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, form, ump::flex_data::set_tempo)
  /// \fn constexpr auto midi2::ump::flex_data::set_tempo::address() const noexcept
  /// \brief Returns the value of the word0::address field.
  /// \returns The value of the word0::address field.
  //
  /// \fn constexpr ump::flex_data::set_tempo & midi2::ump::flex_data::set_tempo::address(adt::uinteger_t<word0::address::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::address field.
  /// \param v  The value to be assigned to the word0::address field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, address, ump::flex_data::set_tempo)
  /// \fn constexpr auto midi2::ump::flex_data::set_tempo::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::flex_data::set_tempo & midi2::ump::flex_data::set_tempo::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::flex_data::set_tempo)
  /// \fn constexpr auto midi2::ump::flex_data::set_tempo::status_bank() const noexcept
  /// \brief Returns the value of the word0::status_bank field.
  /// \returns The value of the word0::status_bank field.
  //
  /// \fn constexpr ump::flex_data::set_tempo & midi2::ump::flex_data::set_tempo::status_bank(adt::uinteger_t<word0::status_bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::status_bank field.
  /// \param v  The value to be assigned to the word0::status_bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, status_bank, ump::flex_data::set_tempo)
  /// \brief Returns the value of the word0::status field. Always 0x00 (mt::flex_data::set_tempo).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::flex_data::set_tempo::value1() const noexcept
  /// \brief Returns the value of the word1::value1 field.
  /// \returns The value of the word1::value1 field.
  //
  /// \fn constexpr ump::flex_data::set_tempo & midi2::ump::flex_data::set_tempo::value1(adt::uinteger_t<word1::value1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value1 field.
  /// \param v  The value to be assigned to the word1::value1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value1, ump::flex_data::set_tempo)
  /// \fn constexpr auto midi2::ump::flex_data::set_tempo::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::flex_data::set_tempo & midi2::ump::flex_data::set_tempo::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::flex_data::set_tempo)
  /// \fn constexpr auto midi2::ump::flex_data::set_tempo::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::flex_data::set_tempo & midi2::ump::flex_data::set_tempo::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::flex_data::set_tempo)

private:
  friend struct ::std::tuple_size<set_tempo>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::flex_data::set_tempo>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::flex_data::set_tempo
//
/// \class std::tuple_element<I, ::midi2::ump::flex_data::set_tempo>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::flex_data::set_tempo
MIDI2_UMP_TUPLE(flex_data, set_tempo)  // Define tuple_size and tuple_element for flex_data/set_tempo
MIDI2_SPAN_CTOR(flex_data, set_tempo)  // Define the span constructor for flex_data/set_tempo

// 7.5.4 Set Time Signature Message
/// \brief The Set Time Signature message (section 7.5.4).
class midi2::ump::flex_data::set_time_signature {
public:
  /// \brief The first word of the Set Time Signature message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::flex_data::set_time_signature;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD (message_type::flex_data).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the format (form) field: 0x0 a complete message in one UMP; 0x1 the start of a
    /// message which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans
    /// two or more UMPs.
    using form = adt::bit_range<22, 2>;
    /// Defines the bit position of the address field: 0x0 the message is sent to the channel in the channel field; 0x1
    /// the message is sent to the group (the channel field is ignored). Other values are reserved.
    using address = adt::bit_range<20, 2>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is sent if the
    /// address field is 0. Otherwise, it is reserved and should be 0.
    using channel = adt::bit_range<16, 4>;
    /// Defines the bit position of the status bank field. 0x00 for setup and performance events, 0x01 for metadata
    /// text, 0x02 for performance text events.
    using status_bank = adt::bit_range<8, 8>;
    /// Defines the bit position of the status field. Always 0x01 (mt::flex_data::set_time_signature).
    using status = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Set Time Signature message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the numerator field: the number of beats in a bar.
    using numerator = adt::bit_range<24, 8>;
    /// Defines the bit position of the denominator field, expressed as a negative power of 2 (2 represents a quarter
    /// note, 3 an eighth note, and so on). Zero indicates a non-standard denominator.
    using denominator = adt::bit_range<16, 8>;
    /// Defines the bit position of the field giving the number of 1/32 notes in 24 MIDI clocks.
    using number_of_32_notes = adt::bit_range<8, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the Set Time Signature message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Set Time Signature message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Set Time Signature message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr set_time_signature() noexcept = default;
  /// Constructs a Set Time Signature message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit set_time_signature(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(set_time_signature const&, set_time_signature const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xD (message_type::flex_data).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::flex_data::set_time_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::form() const noexcept
  /// \brief Returns the value of the word0::form field.
  /// \returns The value of the word0::form field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::form(adt::uinteger_t<word0::form::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::form field.
  /// \param v  The value to be assigned to the word0::form field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, form, ump::flex_data::set_time_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::address() const noexcept
  /// \brief Returns the value of the word0::address field.
  /// \returns The value of the word0::address field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::address(adt::uinteger_t<word0::address::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::address field.
  /// \param v  The value to be assigned to the word0::address field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, address, ump::flex_data::set_time_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::flex_data::set_time_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::status_bank() const noexcept
  /// \brief Returns the value of the word0::status_bank field.
  /// \returns The value of the word0::status_bank field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::status_bank(adt::uinteger_t<word0::status_bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::status_bank field.
  /// \param v  The value to be assigned to the word0::status_bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, status_bank, ump::flex_data::set_time_signature)
  /// \brief Returns the value of the word0::status field. Always 0x01 (mt::flex_data::set_time_signature).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::numerator() const noexcept
  /// \brief Returns the value of the word1::numerator field.
  /// \returns The value of the word1::numerator field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::numerator(adt::uinteger_t<word1::numerator::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::numerator field.
  /// \param v  The value to be assigned to the word1::numerator field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, numerator, ump::flex_data::set_time_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::denominator() const noexcept
  /// \brief Returns the value of the word1::denominator field.
  /// \returns The value of the word1::denominator field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::denominator(adt::uinteger_t<word1::denominator::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::denominator field.
  /// \param v  The value to be assigned to the word1::denominator field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, denominator, ump::flex_data::set_time_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::number_of_32_notes() const noexcept
  /// \brief Returns the value of the word1::number_of_32_notes field.
  /// \returns The value of the word1::number_of_32_notes field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::number_of_32_notes(adt::uinteger_t<word1::number_of_32_notes::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::number_of_32_notes field.
  /// \param v  The value to be assigned to the word1::number_of_32_notes field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, number_of_32_notes, ump::flex_data::set_time_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::flex_data::set_time_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_time_signature::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::flex_data::set_time_signature & midi2::ump::flex_data::set_time_signature::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::flex_data::set_time_signature)

private:
  friend struct ::std::tuple_size<set_time_signature>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::flex_data::set_time_signature>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::flex_data::set_time_signature
//
/// \class std::tuple_element<I, ::midi2::ump::flex_data::set_time_signature>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::flex_data::set_time_signature
MIDI2_UMP_TUPLE(flex_data, set_time_signature)  // Define tuple_size and tuple_element for flex_data/set_time_signature
MIDI2_SPAN_CTOR(flex_data, set_time_signature)  // Define the span constructor for flex_data/set_time_signature

// 7.5.5 Set Metronome Message
/// \class std::tuple_size<::midi2::ump::flex_data::set_metronome>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::flex_data::set_metronome
//
/// \class std::tuple_element<I, ::midi2::ump::flex_data::set_metronome>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::flex_data::set_metronome
//
/// \brief The Set Metronome message (section 7.5.5).
class midi2::ump::flex_data::set_metronome {
public:
  /// \brief The first word of the Set Metronome message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::flex_data::set_metronome;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD (message_type::flex_data).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the format (form) field: 0x0 a complete message in one UMP; 0x1 the start of a
    /// message which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans
    /// two or more UMPs.
    using form = adt::bit_range<22, 2>;
    /// Defines the bit position of the address field: 0x0 the message is sent to the channel in the channel field; 0x1
    /// the message is sent to the group (the channel field is ignored). Other values are reserved.
    using address = adt::bit_range<20, 2>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is sent if the
    /// address field is 0. Otherwise, it is reserved and should be 0.
    using channel = adt::bit_range<16, 4>;
    /// Defines the bit position of the status bank field. 0x00 for setup and performance events, 0x01 for metadata
    /// text, 0x02 for performance text events.
    using status_bank = adt::bit_range<8, 8>;
    /// Defines the bit position of the status field. Always 0x02 (mt::flex_data::set_metronome).
    using status = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Set Metronome message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the number of MIDI clocks per primary click field.
    using num_clocks_per_primary_click = adt::bit_range<24, 8>;
    /// Defines the bit position of the bar accent part 1 field. If the bar is not divided, this is the number of beats
    /// in the bar.
    using bar_accent_part_1 = adt::bit_range<16, 8>;
    /// Defines the bit position of the bar accent part 2 field. Zero if the bar is not divided.
    using bar_accent_part_2 = adt::bit_range<8, 8>;
    /// Defines the bit position of the bar accent part 3 field. Zero if the bar is not divided.
    using bar_accent_part_3 = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the Set Metronome message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the first number of subdivision clicks field: the number of clicks which sound
    /// within the period of a primary click. Zero disables subdivision clicks.
    using num_subdivision_clicks_1 = adt::bit_range<24, 8>;
    /// Defines the bit position of the second number of subdivision clicks field. This allows two overlapping
    /// subdivisions to sound.
    using num_subdivision_clicks_2 = adt::bit_range<16, 8>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<0, 16>;
  };
  /// \brief The fourth word of the Set Metronome message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Set Metronome message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr set_metronome() noexcept = default;
  /// Constructs a Set Metronome message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit set_metronome(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(set_metronome const&, set_metronome const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xD (message_type::flex_data).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::form() const noexcept
  /// \brief Returns the value of the word0::form field.
  /// \returns The value of the word0::form field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::form(adt::uinteger_t<word0::form::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::form field.
  /// \param v  The value to be assigned to the word0::form field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, form, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::address() const noexcept
  /// \brief Returns the value of the word0::address field.
  /// \returns The value of the word0::address field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::address(adt::uinteger_t<word0::address::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::address field.
  /// \param v  The value to be assigned to the word0::address field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, address, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::status_bank() const noexcept
  /// \brief Returns the value of the word0::status_bank field.
  /// \returns The value of the word0::status_bank field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::status_bank(adt::uinteger_t<word0::status_bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::status_bank field.
  /// \param v  The value to be assigned to the word0::status_bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, status_bank, ump::flex_data::set_metronome)
  /// \brief Returns the value of the word0::status field. Always 0x02 (mt::flex_data::set_metronome).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::num_clocks_per_primary_click() const noexcept
  /// \brief Returns the value of the word1::num_clocks_per_primary_click field.
  /// \returns The value of the word1::num_clocks_per_primary_click field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::num_clocks_per_primary_click(adt::uinteger_t<word1::num_clocks_per_primary_click::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::num_clocks_per_primary_click field.
  /// \param v  The value to be assigned to the word1::num_clocks_per_primary_click field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, num_clocks_per_primary_click, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::bar_accent_part_1() const noexcept
  /// \brief Returns the value of the word1::bar_accent_part_1 field.
  /// \returns The value of the word1::bar_accent_part_1 field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::bar_accent_part_1(adt::uinteger_t<word1::bar_accent_part_1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::bar_accent_part_1 field.
  /// \param v  The value to be assigned to the word1::bar_accent_part_1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, bar_accent_part_1, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::bar_accent_part_2() const noexcept
  /// \brief Returns the value of the word1::bar_accent_part_2 field.
  /// \returns The value of the word1::bar_accent_part_2 field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::bar_accent_part_2(adt::uinteger_t<word1::bar_accent_part_2::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::bar_accent_part_2 field.
  /// \param v  The value to be assigned to the word1::bar_accent_part_2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, bar_accent_part_2, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::bar_accent_part_3() const noexcept
  /// \brief Returns the value of the word1::bar_accent_part_3 field.
  /// \returns The value of the word1::bar_accent_part_3 field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::bar_accent_part_3(adt::uinteger_t<word1::bar_accent_part_3::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::bar_accent_part_3 field.
  /// \param v  The value to be assigned to the word1::bar_accent_part_3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, bar_accent_part_3, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::num_subdivision_clicks_1() const noexcept
  /// \brief Returns the value of the word2::num_subdivision_clicks_1 field.
  /// \returns The value of the word2::num_subdivision_clicks_1 field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::num_subdivision_clicks_1(adt::uinteger_t<word2::num_subdivision_clicks_1::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::num_subdivision_clicks_1 field.
  /// \param v  The value to be assigned to the word2::num_subdivision_clicks_1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, num_subdivision_clicks_1, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::num_subdivision_clicks_2() const noexcept
  /// \brief Returns the value of the word2::num_subdivision_clicks_2 field.
  /// \returns The value of the word2::num_subdivision_clicks_2 field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::num_subdivision_clicks_2(adt::uinteger_t<word2::num_subdivision_clicks_2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::num_subdivision_clicks_2 field.
  /// \param v  The value to be assigned to the word2::num_subdivision_clicks_2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, num_subdivision_clicks_2, ump::flex_data::set_metronome)
  /// \fn constexpr auto midi2::ump::flex_data::set_metronome::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::flex_data::set_metronome & midi2::ump::flex_data::set_metronome::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::flex_data::set_metronome)

private:
  friend struct ::std::tuple_size<set_metronome>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
MIDI2_UMP_TUPLE(flex_data, set_metronome)  // Define tuple_size and tuple_element for flex_data/set_metronome
MIDI2_SPAN_CTOR(flex_data, set_metronome)  // Define the span constructor for flex_data/set_metronome

namespace midi2::ump::flex_data {

/// An X macro which lists the members of the midi2::ump::flex_data::sharps_flats enumeration and their values.
#define MIDI2_SHARPS_FLATS \
  MIDI2_X(double_sharp, 2) \
  MIDI2_X(sharp, 1)        \
  MIDI2_X(natural, 0)      \
  MIDI2_X(flat, -1)        \
  MIDI2_X(double_flat, -2) \
  MIDI2_X(chord_tonic, -8)

/// \brief  An X macro for defining the sharps_flats enumeration.
#define MIDI2_X(a, b) a = (b),
/// \brief The values of the 4-bit two's complement sharps/flats fields of the set_key_signature and
///   set_chord_name messages.
///
/// In a key signature, positive values declare the number of sharps and negative values the number of flats (from
/// -7 to 7). In a chord name, the value is the number of sharps or flats applied to the note (-2 to 2). A value of
/// -8 indicates an unknown or non-standard key signature or, for a bass note, that the bass note is the same as the
/// chord tonic.
enum class sharps_flats : std::int8_t { MIDI2_SHARPS_FLATS };
#undef MIDI2_X
/// \var midi2::ump::flex_data::sharps_flats midi2::ump::flex_data::sharps_flats::double_sharp
/// Double sharp (2).
/// \var midi2::ump::flex_data::sharps_flats midi2::ump::flex_data::sharps_flats::sharp
/// Sharp (1).
/// \var midi2::ump::flex_data::sharps_flats midi2::ump::flex_data::sharps_flats::natural
/// Natural (0).
/// \var midi2::ump::flex_data::sharps_flats midi2::ump::flex_data::sharps_flats::flat
/// Flat (-1).
/// \var midi2::ump::flex_data::sharps_flats midi2::ump::flex_data::sharps_flats::double_flat
/// Double flat (-2).
/// \var midi2::ump::flex_data::sharps_flats midi2::ump::flex_data::sharps_flats::chord_tonic
/// -8. In the bass sharps/flats field of a set_chord_name message, the bass note is the same as the chord tonic
/// note. In a set_key_signature message, the key signature is unknown or non-standard.

/// \brief Returns true if the argument can be safely cast to midi2::ump::flex_data::sharps_flats.
/// \param v  A value to be checked for compatibility with the sharps_flats enumeration.
/// \returns True if the argument can be safely cast to sharps_flats, false otherwise.
[[nodiscard]] constexpr bool valid_sharps_flats(std::underlying_type_t<sharps_flats> const v) noexcept {
#define MIDI2_X(a, b) \
  case b: return true;
  switch (v) {
    MIDI2_SHARPS_FLATS
  default: return false;
  }
#undef MIDI2_X
}

/// An X macro which lists the members of the midi2::ump::flex_data::note enumeration and their values.
#define MIDI2_NOTE      \
  MIDI2_X(unknown, 0x0) \
  MIDI2_X(a, 0x1)       \
  MIDI2_X(b, 0x2)       \
  MIDI2_X(c, 0x3)       \
  MIDI2_X(d, 0x4)       \
  MIDI2_X(e, 0x5)       \
  MIDI2_X(f, 0x6)       \
  MIDI2_X(g, 0x7)

/// \brief  An X macro for defining the note enumeration.
#define MIDI2_X(a, b) a = (b),
/// Defines the note values used to describe a chord, key signatures, and so on.
enum class note : std::int8_t { MIDI2_NOTE };
#undef MIDI2_X
/// \var midi2::ump::flex_data::note midi2::ump::flex_data::note::unknown
/// Unknown or non-standard. In a chord name, this means "no chord" (chord tonic) or "same as the chord
/// tonic note" (bass note).
/// \var midi2::ump::flex_data::note midi2::ump::flex_data::note::a
/// A
/// \var midi2::ump::flex_data::note midi2::ump::flex_data::note::b
/// B
/// \var midi2::ump::flex_data::note midi2::ump::flex_data::note::c
/// C
/// \var midi2::ump::flex_data::note midi2::ump::flex_data::note::d
/// D
/// \var midi2::ump::flex_data::note midi2::ump::flex_data::note::e
/// E
/// \var midi2::ump::flex_data::note midi2::ump::flex_data::note::f
/// F
/// \var midi2::ump::flex_data::note midi2::ump::flex_data::note::g
/// G

/// \brief Returns true if the argument can be safely cast to midi2::ump::flex_data::note.
/// \param v  A value to be checked for compatibility with the note enumeration.
/// \returns True if the argument can be safely cast to note, false otherwise.
[[nodiscard]] constexpr bool valid_note(std::underlying_type_t<note> const v) noexcept {
#define MIDI2_X(a, b) \
  case b: return true;
  switch (v) {
    MIDI2_NOTE
  default: return false;
  }
#undef MIDI2_X
}

/// An X macro which lists the members of the midi2::ump::flex_data::chord_type enumeration and their values.
#define MIDI2_CHORD_TYPE            \
  MIDI2_X(no_chord, 0x00)           \
  MIDI2_X(major, 0x01)              \
  MIDI2_X(major_6th, 0x02)          \
  MIDI2_X(major_7th, 0x03)          \
  MIDI2_X(major_9th, 0x04)          \
  MIDI2_X(major_11th, 0x05)         \
  MIDI2_X(major_13th, 0x06)         \
  MIDI2_X(minor, 0x07)              \
  MIDI2_X(minor_6th, 0x08)          \
  MIDI2_X(minor_7th, 0x09)          \
  MIDI2_X(minor_9th, 0x0A)          \
  MIDI2_X(minor_11th, 0x0B)         \
  MIDI2_X(minor_13th, 0x0C)         \
  MIDI2_X(dominant, 0x0D)           \
  MIDI2_X(dominant_ninth, 0x0E)     \
  MIDI2_X(dominant_11th, 0x0F)      \
  MIDI2_X(dominant_13th, 0x10)      \
  MIDI2_X(augmented, 0x11)          \
  MIDI2_X(augmented_seventh, 0x12)  \
  MIDI2_X(diminished, 0x13)         \
  MIDI2_X(diminished_seventh, 0x14) \
  MIDI2_X(half_diminished, 0x15)    \
  MIDI2_X(major_minor, 0x16)        \
  MIDI2_X(pedal, 0x17)              \
  MIDI2_X(power, 0x18)              \
  MIDI2_X(suspended_2nd, 0x19)      \
  MIDI2_X(suspended_4th, 0x1A)      \
  MIDI2_X(seven_suspended_4th, 0x1B)

/// \brief  An X macro for defining the chord_type enumeration.
#define MIDI2_X(a, b) a = (b),
/// The values of the chord type and bass chord type fields of the set_chord_name message.
enum class chord_type : std::uint8_t { MIDI2_CHORD_TYPE };
#undef MIDI2_X
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::no_chord
/// Clear chord: no chord.
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::major
/// Major
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::major_6th
/// Major 6th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::major_7th
/// Major 7th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::major_9th
/// Major 9th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::major_11th
/// Major 11th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::major_13th
/// Major 13th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::minor
/// Minor
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::minor_6th
/// Minor 6th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::minor_7th
/// Minor 7th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::minor_9th
/// Minor 9th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::minor_11th
/// Minor 11th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::minor_13th
/// Minor 13th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::dominant
/// Dominant
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::dominant_ninth
/// Dominant ninth
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::dominant_11th
/// Dominant 11th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::dominant_13th
/// Dominant 13th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::augmented
/// Augmented
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::augmented_seventh
/// Augmented seventh
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::diminished
/// Diminished
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::diminished_seventh
/// Diminished seventh
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::half_diminished
/// Half diminished (diminished triad, minor seventh)
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::major_minor
/// Major-minor or minor-major (minor triad, major seventh)
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::pedal
/// Pedal (e.g. XF 1+8)
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::power
/// Power (e.g. XF 1+5)
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::suspended_2nd
/// Suspended 2nd (e.g. XF 1+2+5)
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::suspended_4th
/// Suspended 4th
/// \var midi2::ump::flex_data::chord_type midi2::ump::flex_data::chord_type::seven_suspended_4th
/// 7 suspended 4th

/// \brief Returns true if the argument can be safely cast to midi2::ump::flex_data::chord_type.
/// \param v  A value to be checked for compatibility with the chord_type enumeration.
/// \returns True if the argument can be safely cast to chord_type, false otherwise.
[[nodiscard]] constexpr bool valid_chord_type(std::underlying_type_t<chord_type> const v) noexcept {
#define MIDI2_X(a, b) \
  case b: return true;
  switch (v) {
    MIDI2_CHORD_TYPE
  default: return false;
  }
#undef MIDI2_X
}

}  // end namespace midi2::ump::flex_data

// 7.5.7 Set Key Signature Message
/// \brief The Set Key Signature message (section 7.5.7).
class midi2::ump::flex_data::set_key_signature {
public:
  /// \brief The first word of the Set Key Signature message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::flex_data::set_key_signature;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD (message_type::flex_data).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the format (form) field: 0x0 a complete message in one UMP; 0x1 the start of a
    /// message which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans
    /// two or more UMPs.
    using form = adt::bit_range<22, 2>;
    /// Defines the bit position of the address field: 0x0 the message is sent to the channel in the channel field; 0x1
    /// the message is sent to the group (the channel field is ignored). Other values are reserved.
    using address = adt::bit_range<20, 2>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is sent if the
    /// address field is 0. Otherwise, it is reserved and should be 0.
    using channel = adt::bit_range<16, 4>;
    /// Defines the bit position of the status bank field. 0x00 for setup and performance events, 0x01 for metadata
    /// text, 0x02 for performance text events.
    using status_bank = adt::bit_range<8, 8>;
    /// Defines the bit position of the status field. Always 0x05 (mt::flex_data::set_key_signature).
    using status = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Set Key Signature message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept {
      return valid_sharps_flats(get_signed<sharps_flats>()) && valid_note(get_signed<tonic_note>());
    }
    /// Defines the bit position of the sharps/flats field. This is a 4-bit two's complement value: positive values give
    /// the number of sharps, negative values the number of flats. -8 means unknown or non-standard.
    using sharps_flats = adt::bit_range<28, 4>;
    /// Defines the bit position of the tonic note field. See midi2::ump::flex_data::note.
    using tonic_note = adt::bit_range<24, 4>;
    /// Defines a group of reserved bits.
    using reserved0 = adt::bit_range<0, 24>;
  };
  /// \brief The third word of the Set Key Signature message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word2::value2 field. This word is reserved and should be zero.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth word of the Set Key Signature message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the word3::value3 field. This word is reserved and should be zero.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Set Key Signature message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr set_key_signature() noexcept = default;
  /// Constructs a Set Key Signature message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit set_key_signature(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(set_key_signature const&, set_key_signature const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xD (message_type::flex_data).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::flex_data::set_key_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::form() const noexcept
  /// \brief Returns the value of the word0::form field.
  /// \returns The value of the word0::form field.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::form(adt::uinteger_t<word0::form::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::form field.
  /// \param v  The value to be assigned to the word0::form field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, form, ump::flex_data::set_key_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::address() const noexcept
  /// \brief Returns the value of the word0::address field.
  /// \returns The value of the word0::address field.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::address(adt::uinteger_t<word0::address::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::address field.
  /// \param v  The value to be assigned to the word0::address field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, address, ump::flex_data::set_key_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::flex_data::set_key_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::status_bank() const noexcept
  /// \brief Returns the value of the word0::status_bank field.
  /// \returns The value of the word0::status_bank field.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::status_bank(adt::uinteger_t<word0::status_bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::status_bank field.
  /// \param v  The value to be assigned to the word0::status_bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, status_bank, ump::flex_data::set_key_signature)
  /// \brief Returns the value of the word0::status field. Always 0x05 (mt::flex_data::set_key_signature).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::sharps_flats_raw() const noexcept
  /// \brief Returns the raw (integer) value of the word1::sharps_flats field.
  /// \returns The raw value of the word1::sharps_flats field.
  //
  /// \fn constexpr ump::flex_data::sharps_flats midi2::ump::flex_data::set_key_signature::sharps_flats() const noexcept
  /// \brief Returns the value of the word1::sharps_flats field as a sharps_flats enumeration.
  /// \returns The value of the word1::sharps_flats field as a sharps_flats enumeration.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::sharps_flats(enum ump::flex_data::sharps_flats const v) noexcept
  /// \brief Sets the value of the word1::sharps_flats field.
  /// \param v  The value to be assigned to the word1::sharps_flats field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, sharps_flats, ump::flex_data::sharps_flats, ump::flex_data::set_key_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::tonic_note_raw() const noexcept
  /// \brief Returns the raw (integer) value of the word1::tonic_note field.
  /// \returns The raw value of the word1::tonic_note field.
  //
  /// \fn constexpr ump::flex_data::note midi2::ump::flex_data::set_key_signature::tonic_note() const noexcept
  /// \brief Returns the value of the word1::tonic_note field as a note enumeration.
  /// \returns The value of the word1::tonic_note field as a note enumeration.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::tonic_note(enum ump::flex_data::note const v) noexcept
  /// \brief Sets the value of the word1::tonic_note field.
  /// \param v  The value to be assigned to the word1::tonic_note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, tonic_note, ump::flex_data::note, ump::flex_data::set_key_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::flex_data::set_key_signature)
  /// \fn constexpr auto midi2::ump::flex_data::set_key_signature::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::flex_data::set_key_signature & midi2::ump::flex_data::set_key_signature::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::flex_data::set_key_signature)

private:
  friend struct ::std::tuple_size<set_key_signature>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::flex_data::set_key_signature>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::flex_data::set_key_signature
//
/// \class std::tuple_element<I, ::midi2::ump::flex_data::set_key_signature>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::flex_data::set_key_signature
MIDI2_UMP_TUPLE(flex_data, set_key_signature)  // Define tuple_size and tuple_element for flex_data/set_key_signature
MIDI2_SPAN_CTOR(flex_data, set_key_signature)  // Define the span constructor for flex_data/set_key_signature

/// \class midi2::ump::flex_data::set_chord_name
/// \brief This message declares the name of a chord.
class midi2::ump::flex_data::set_chord_name {
public:
  /// Defines the fields of the first word of the set_chord_name message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::flex_data::set_chord_name;
    /// Default constructing word0 initializes the word0::mt and word0::status fields to define the
    /// message type. Other fields are initialized to 0.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this message word are valid.
    /// \returns True if the message word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0xD (message_type::flex_data).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field. The group addresses the message to one of 16 groups (0x0-0xF).
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the format (form) field: 0x0 a complete message in one UMP; 0x1 the start of a
    /// message which spans two or more UMPs; 0x2 a continuation of such a message; 0x3 the end of a message which spans
    /// two or more UMPs.
    using form = adt::bit_range<22, 2>;
    /// Defines the bit position of the address field: 0x0 the message is sent to the channel in the channel field; 0x1
    /// the message is sent to the group (the channel field is ignored). Other values are reserved.
    using address = adt::bit_range<20, 2>;
    /// Defines the bit position of the channel field. The MIDI channel (0x0-0xF) to which the message is sent if the
    /// address field is 0. Otherwise, it is reserved and should be 0.
    using channel = adt::bit_range<16, 4>;
    /// Defines the bit position of the status bank field. 0x00 for setup and performance events, 0x01 for metadata
    /// text, 0x02 for performance text events.
    using status_bank = adt::bit_range<8, 8>;
    /// Defines the bit position of the status field. Always 0x06 (mt::flex_data::set_chord_name).
    using status = adt::bit_range<0, 8>;
  };
  /// Defines the fields of the second word of the set_chord_name message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this message word are valid.
    /// \returns True if the message word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept {
      return valid_sharps_flats(get_signed<tonic_sharps_flats>()) && valid_note(get_signed<chord_tonic>()) &&
             valid_chord_type(get<chord_type>());
    }

    /// Defines the bit position of the tonic sharps/flats field. This is a 4-bit two's complement value: 2 double
    /// sharp, 1 sharp, 0 natural, -1 flat, -2 double flat. See midi2::ump::flex_data::sharps_flats.
    using tonic_sharps_flats = adt::bit_range<28, 4>;
    /// Defines the bit position of the chord tonic note field. See midi2::ump::flex_data::note. note::unknown is used
    /// for "no chord".
    using chord_tonic = adt::bit_range<24, 4>;
    /// Defines the bit position of the chord type field. See midi2::ump::flex_data::chord_type.
    using chord_type = adt::bit_range<16, 8>;
    /// Defines the bit position of the alteration 1 type field: 0 no alteration; 1 add degree; 2 subtract degree; 3
    /// raise degree, adding if needed; 4 lower degree, adding if needed. Other values are reserved.
    using alter_1_type = adt::bit_range<12, 4>;
    /// Defines the bit position of the alteration 1 degree field: the degree of the chord (1 for the root, 3 for the
    /// third, and so on) which is altered.
    using alter_1_degree = adt::bit_range<8, 4>;
    /// Defines the bit position of the alteration 2 type field: 0 no alteration; 1 add degree; 2 subtract degree; 3
    /// raise degree, adding if needed; 4 lower degree, adding if needed. Other values are reserved.
    using alter_2_type = adt::bit_range<4, 4>;
    /// Defines the bit position of the alteration 2 degree field: the degree of the chord (1 for the root, 3 for the
    /// third, and so on) which is altered.
    using alter_2_degree = adt::bit_range<0, 4>;
  };
  /// Defines the fields of the third word of the set_chord_name message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this message word are valid.
    /// \returns True if the message word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of the alteration 3 type field: 0 no alteration; 1 add degree; 2 subtract degree; 3
    /// raise degree, adding if needed; 4 lower degree, adding if needed. Other values are reserved.
    using alter_3_type = adt::bit_range<28, 4>;
    /// Defines the bit position of the alteration 3 degree field: the degree of the chord (1 for the root, 3 for the
    /// third, and so on) which is altered.
    using alter_3_degree = adt::bit_range<24, 4>;
    /// Defines the bit position of the alteration 4 type field: 0 no alteration; 1 add degree; 2 subtract degree; 3
    /// raise degree, adding if needed; 4 lower degree, adding if needed. Other values are reserved.
    using alter_4_type = adt::bit_range<20, 4>;
    /// Defines the bit position of the alteration 4 degree field: the degree of the chord (1 for the root, 3 for the
    /// third, and so on) which is altered.
    using alter_4_degree = adt::bit_range<16, 4>;
    /// Defines a group of reserved bits.
    using reserved = adt::bit_range<0, 16>;
  };
  /// Defines the fields of the fourth word of the set_chord_name message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this message word are valid.
    /// \returns True if the message word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept {
      return valid_sharps_flats(get_signed<bass_sharps_flats>()) && valid_note(get_signed<bass_note>()) &&
             valid_chord_type(get<bass_chord_type>());
    }
    /// Defines the bit position of the bass sharps/flats field. This is a 4-bit two's complement value: 2 double sharp,
    /// 1 sharp, 0 natural, -1 flat, -2 double flat. -8 means that the bass note is the same as the chord tonic note.
    /// See midi2::ump::flex_data::sharps_flats.
    using bass_sharps_flats = adt::bit_range<28, 4>;
    /// Defines the bit position of the bass note field. See midi2::ump::flex_data::note. note::unknown means that the
    /// bass note is the same as the chord tonic note.
    using bass_note = adt::bit_range<24, 4>;
    /// Defines the bit position of the bass chord type field. See midi2::ump::flex_data::chord_type.
    /// chord_type::no_chord means that there is no bass chord.
    using bass_chord_type = adt::bit_range<16, 8>;
    /// Defines the bit position of the bass chord alteration 1 type field: 0 no alteration; 1 add degree; 2 subtract
    /// degree; 3 raise degree, adding if needed; 4 lower degree, adding if needed. Other values are reserved.
    using bass_alter_1_type = adt::bit_range<12, 4>;
    /// Defines the bit position of the bass chord alteration 1 degree field: the degree of the chord (1 for the root, 3
    /// for the third, and so on) which is altered.
    using bass_alter_1_degree = adt::bit_range<8, 4>;
    /// Defines the bit position of the bass chord alteration 2 type field: 0 no alteration; 1 add degree; 2 subtract
    /// degree; 3 raise degree, adding if needed; 4 lower degree, adding if needed. Other values are reserved.
    using bass_alter_2_type = adt::bit_range<4, 4>;
    /// Defines the bit position of the bass chord alteration 2 degree field: the degree of the chord (1 for the root, 3
    /// for the third, and so on) which is altered.
    using bass_alter_2_degree = adt::bit_range<0, 4>;
  };

  /// Defaults constructs a set_chord_name message with all fields set to zero.
  constexpr set_chord_name() noexcept = default;
  /// Constructs a Set Chord Name message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit set_chord_name(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(set_chord_name const&, set_chord_name const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xD (message_type::flex_data).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::flex_data::set_chord_name)
  /// \brief Returns the value of the word0::form field.
  /// \note This is a read-only field.
  /// \returns The value of the word0::form field.
  MIDI2_UMP_GETTER(word0, form)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::address() const noexcept
  /// \brief Returns the value of the word0::address field.
  /// \returns The value of the word0::address field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::address(adt::uinteger_t<word0::address::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::address field.
  /// \param v  The value to be assigned to the word0::address field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, address, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::flex_data::set_chord_name)
  /// \brief Returns the value of the word0::status_bank field.
  /// \note This is a read-only field.
  /// \returns The value of the word0::status_bank field.
  MIDI2_UMP_GETTER(word0, status_bank)
  /// \brief Returns the value of the word0::status field. Always 0x06 (mt::flex_data::set_chord_name).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::tonic_sharps_flats_raw() const noexcept
  /// \brief Returns the raw (integer) value of the word1::tonic_sharps_flats field.
  /// \returns The raw value of the word1::tonic_sharps_flats field.
  //
  /// \fn constexpr midi2::ump::flex_data::sharps_flats midi2::ump::flex_data::set_chord_name::tonic_sharps_flats() const noexcept
  /// \brief Returns the value of the word1::tonic_sharps_flats field as a sharps_flats enumeration.
  /// \returns The value of the word1::tonic_sharps_flats field as a sharps_flats enumeration.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::tonic_sharps_flats(enum midi2::ump::flex_data::sharps_flats const v) noexcept
  /// \brief Sets the value of the word1::tonic_sharps_flats field.
  /// \param v  The value to be assigned to the word1::tonic_sharps_flats field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, tonic_sharps_flats, midi2::ump::flex_data::sharps_flats,
                               ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::chord_tonic_raw() const noexcept
  /// \brief Returns the raw (integer) value of the word1::chord_tonic field.
  /// \returns The raw value of the word1::chord_tonic field.
  //
  /// \fn constexpr midi2::ump::flex_data::note midi2::ump::flex_data::set_chord_name::chord_tonic() const noexcept
  /// \brief Returns the value of the word1::chord_tonic field as a note enumeration.
  /// \returns The value of the word1::chord_tonic field as a note enumeration.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::chord_tonic(enum midi2::ump::flex_data::note const v) noexcept
  /// \brief Sets the value of the word1::chord_tonic field.
  /// \param v  The value to be assigned to the word1::chord_tonic field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, chord_tonic, midi2::ump::flex_data::note, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::chord_type_raw() const noexcept
  /// \brief Returns the raw (integer) value of the word1::chord_type field.
  /// \returns The raw value of the word1::chord_type field.
  //
  /// \fn constexpr midi2::ump::flex_data::chord_type midi2::ump::flex_data::set_chord_name::chord_type() const noexcept
  /// \brief Returns the value of the word1::chord_type field as a chord_type enumeration.
  /// \returns The value of the word1::chord_type field as a chord_type enumeration.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::chord_type(enum midi2::ump::flex_data::chord_type const v) noexcept
  /// \brief Sets the value of the word1::chord_type field.
  /// \param v  The value to be assigned to the word1::chord_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER_ENUM(word1, chord_type, midi2::ump::flex_data::chord_type, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::alter_1_type() const noexcept
  /// \brief Returns the value of the word1::alter_1_type field.
  /// \returns The value of the word1::alter_1_type field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::alter_1_type(adt::uinteger_t<word1::alter_1_type::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::alter_1_type field.
  /// \param v  The value to be assigned to the word1::alter_1_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, alter_1_type, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::alter_1_degree() const noexcept
  /// \brief Returns the value of the word1::alter_1_degree field.
  /// \returns The value of the word1::alter_1_degree field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::alter_1_degree(adt::uinteger_t<word1::alter_1_degree::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::alter_1_degree field.
  /// \param v  The value to be assigned to the word1::alter_1_degree field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, alter_1_degree, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::alter_2_type() const noexcept
  /// \brief Returns the value of the word1::alter_2_type field.
  /// \returns The value of the word1::alter_2_type field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::alter_2_type(adt::uinteger_t<word1::alter_2_type::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::alter_2_type field.
  /// \param v  The value to be assigned to the word1::alter_2_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, alter_2_type, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::alter_2_degree() const noexcept
  /// \brief Returns the value of the word1::alter_2_degree field.
  /// \returns The value of the word1::alter_2_degree field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::alter_2_degree(adt::uinteger_t<word1::alter_2_degree::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::alter_2_degree field.
  /// \param v  The value to be assigned to the word1::alter_2_degree field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, alter_2_degree, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::alter_3_type() const noexcept
  /// \brief Returns the value of the word2::alter_3_type field.
  /// \returns The value of the word2::alter_3_type field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::alter_3_type(adt::uinteger_t<word2::alter_3_type::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::alter_3_type field.
  /// \param v  The value to be assigned to the word2::alter_3_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, alter_3_type, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::alter_3_degree() const noexcept
  /// \brief Returns the value of the word2::alter_3_degree field.
  /// \returns The value of the word2::alter_3_degree field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::alter_3_degree(adt::uinteger_t<word2::alter_3_degree::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::alter_3_degree field.
  /// \param v  The value to be assigned to the word2::alter_3_degree field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, alter_3_degree, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::alter_4_type() const noexcept
  /// \brief Returns the value of the word2::alter_4_type field.
  /// \returns The value of the word2::alter_4_type field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::alter_4_type(adt::uinteger_t<word2::alter_4_type::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::alter_4_type field.
  /// \param v  The value to be assigned to the word2::alter_4_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, alter_4_type, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::alter_4_degree() const noexcept
  /// \brief Returns the value of the word2::alter_4_degree field.
  /// \returns The value of the word2::alter_4_degree field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::alter_4_degree(adt::uinteger_t<word2::alter_4_degree::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::alter_4_degree field.
  /// \param v  The value to be assigned to the word2::alter_4_degree field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, alter_4_degree, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::bass_sharps_flats_raw() const noexcept
  /// \brief Returns the raw (integer) value of the word3::bass_sharps_flats field.
  /// \returns The raw value of the word3::bass_sharps_flats field.
  //
  /// \fn constexpr midi2::ump::flex_data::sharps_flats midi2::ump::flex_data::set_chord_name::bass_sharps_flats() const noexcept
  /// \brief Returns the value of the word3::bass_sharps_flats field as a sharps_flats enumeration.
  /// \returns The value of the word3::bass_sharps_flats field as a sharps_flats enumeration.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::bass_sharps_flats(enum midi2::ump::flex_data::sharps_flats const v) noexcept
  /// \brief Sets the value of the word3::bass_sharps_flats field.
  /// \param v  The value to be assigned to the word3::bass_sharps_flats field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER_ENUM(word3, bass_sharps_flats, midi2::ump::flex_data::sharps_flats,
                               ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::bass_note_raw() const noexcept
  /// \brief Returns the raw (integer) value of the word3::bass_note field.
  /// \returns The raw value of the word3::bass_note field.
  //
  /// \fn constexpr midi2::ump::flex_data::note midi2::ump::flex_data::set_chord_name::bass_note() const noexcept
  /// \brief Returns the value of the word3::bass_note field as a note enumeration.
  /// \returns The value of the word3::bass_note field as a note enumeration.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::bass_note(enum midi2::ump::flex_data::note const v) noexcept
  /// \brief Sets the value of the word3::bass_note field.
  /// \param v  The value to be assigned to the word3::bass_note field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER_ENUM(word3, bass_note, midi2::ump::flex_data::note, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::bass_chord_type_raw() const noexcept
  /// \brief Returns the raw (integer) value of the word3::bass_chord_type field.
  /// \returns The raw value of the word3::bass_chord_type field.
  //
  /// \fn constexpr midi2::ump::flex_data::chord_type midi2::ump::flex_data::set_chord_name::bass_chord_type() const noexcept
  /// \brief Returns the value of the word3::bass_chord_type field as a chord_type enumeration.
  /// \returns The value of the word3::bass_chord_type field as a chord_type enumeration.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::bass_chord_type(enum midi2::ump::flex_data::chord_type const v) noexcept
  /// \brief Sets the value of the word3::bass_chord_type field.
  /// \param v  The value to be assigned to the word3::bass_chord_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER_ENUM(word3, bass_chord_type, midi2::ump::flex_data::chord_type,
                               ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::bass_alter_1_type() const noexcept
  /// \brief Returns the value of the word3::bass_alter_1_type field.
  /// \returns The value of the word3::bass_alter_1_type field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::bass_alter_1_type(adt::uinteger_t<word3::bass_alter_1_type::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::bass_alter_1_type field.
  /// \param v  The value to be assigned to the word3::bass_alter_1_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, bass_alter_1_type, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::bass_alter_1_degree() const noexcept
  /// \brief Returns the value of the word3::bass_alter_1_degree field.
  /// \returns The value of the word3::bass_alter_1_degree field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::bass_alter_1_degree(adt::uinteger_t<word3::bass_alter_1_degree::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::bass_alter_1_degree field.
  /// \param v  The value to be assigned to the word3::bass_alter_1_degree field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, bass_alter_1_degree, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::bass_alter_2_type() const noexcept
  /// \brief Returns the value of the word3::bass_alter_2_type field.
  /// \returns The value of the word3::bass_alter_2_type field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::bass_alter_2_type(adt::uinteger_t<word3::bass_alter_2_type::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::bass_alter_2_type field.
  /// \param v  The value to be assigned to the word3::bass_alter_2_type field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, bass_alter_2_type, ump::flex_data::set_chord_name)
  /// \fn constexpr auto midi2::ump::flex_data::set_chord_name::bass_alter_2_degree() const noexcept
  /// \brief Returns the value of the word3::bass_alter_2_degree field.
  /// \returns The value of the word3::bass_alter_2_degree field.
  //
  /// \fn constexpr ump::flex_data::set_chord_name & midi2::ump::flex_data::set_chord_name::bass_alter_2_degree(adt::uinteger_t<word3::bass_alter_2_degree::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::bass_alter_2_degree field.
  /// \param v  The value to be assigned to the word3::bass_alter_2_degree field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, bass_alter_2_degree, ump::flex_data::set_chord_name)

private:
  friend struct ::std::tuple_size<set_chord_name>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::flex_data::set_chord_name>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::flex_data::set_chord_name
//
/// \class std::tuple_element<I, ::midi2::ump::flex_data::set_chord_name>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::flex_data::set_chord_name
MIDI2_UMP_TUPLE(flex_data, set_chord_name)  // Define tuple_size and tuple_element for flex_data/set_chord_name
MIDI2_SPAN_CTOR(flex_data, set_chord_name)  // Define the span constructor for flex_data/set_chord_name

// 7.5.9 Text Messages Common Format
/// \brief The Flex Data Text Common Format message (section 7.5.9).
class midi2::ump::flex_data::text_common {
public:
  /// \brief The first word of the Flex Data Text Common Format message.
  class word0 : public details::word_base {
  public:
    using word_base::word_base;
    /// Default constructs the word with the mt field set to the message type. All other fields are
    /// zero.
    constexpr word0() noexcept { this->set<mt>(std::to_underlying(ump::message_type::flex_data)); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept {
      return this->get<mt>() == std::to_underlying(ump::message_type::flex_data);
    }

    /// Defines the bit position of the mt (message-type) field. Always 0xD (message_type::flex_data).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field.
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the form field.
    using form = adt::bit_range<22, 2>;
    /// Defines the bit position of the address field.
    using address = adt::bit_range<20, 2>;
    /// Defines the bit position of the channel field.
    using channel = adt::bit_range<16, 4>;
    /// Defines the bit position of the status_bank field.
    using status_bank = adt::bit_range<8, 8>;
    /// Defines the bit position of the status field.
    using status = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the Flex Data Text Common Format message.
  class word1 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 0 of the UTF-8 text.
    using data0 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 1 of the UTF-8 text.
    using data1 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 2 of the UTF-8 text.
    using data2 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 3 of the UTF-8 text.
    using data3 = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the Flex Data Text Common Format message.
  class word2 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 4 of the UTF-8 text.
    using data4 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 5 of the UTF-8 text.
    using data5 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 6 of the UTF-8 text.
    using data6 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 7 of the UTF-8 text.
    using data7 = adt::bit_range<0, 8>;
  };
  /// \brief The fourth word of the Flex Data Text Common Format message.
  class word3 : public details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of byte 8 of the UTF-8 text.
    using data8 = adt::bit_range<24, 8>;
    /// Defines the bit position of byte 9 of the UTF-8 text.
    using data9 = adt::bit_range<16, 8>;
    /// Defines the bit position of byte 10 of the UTF-8 text.
    using data10 = adt::bit_range<8, 8>;
    /// Defines the bit position of byte 11 of the UTF-8 text.
    using data11 = adt::bit_range<0, 8>;
  };

  /// Default constructs a Flex Data Text Common Format message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr text_common() noexcept = default;
  /// Constructs a Flex Data Text Common Format message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit text_common(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(text_common const&, text_common const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0xD (message_type::flex_data).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::form() const noexcept
  /// \brief Returns the value of the word0::form field.
  /// \returns The value of the word0::form field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::form(adt::uinteger_t<word0::form::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::form field.
  /// \param v  The value to be assigned to the word0::form field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, form, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::address() const noexcept
  /// \brief Returns the value of the word0::address field.
  /// \returns The value of the word0::address field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::address(adt::uinteger_t<word0::address::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::address field.
  /// \param v  The value to be assigned to the word0::address field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, address, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::channel() const noexcept
  /// \brief Returns the value of the word0::channel field.
  /// \returns The value of the word0::channel field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::channel(adt::uinteger_t<word0::channel::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::channel field.
  /// \param v  The value to be assigned to the word0::channel field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, channel, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::status_bank() const noexcept
  /// \brief Returns the value of the word0::status_bank field.
  /// \returns The value of the word0::status_bank field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::status_bank(adt::uinteger_t<word0::status_bank::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::status_bank field.
  /// \param v  The value to be assigned to the word0::status_bank field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, status_bank, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::status() const noexcept
  /// \brief Returns the value of the word0::status field.
  /// \returns The value of the word0::status field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::status(adt::uinteger_t<word0::status::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::status field.
  /// \param v  The value to be assigned to the word0::status field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, status, ump::flex_data::text_common)

  /// \fn constexpr auto midi2::ump::flex_data::text_common::data0() const noexcept
  /// \brief Returns the value of the word1::data0 field.
  /// \returns The value of the word1::data0 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data0(adt::uinteger_t<word1::data0::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data0 field.
  /// \param v  The value to be assigned to the word1::data0 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data0, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data1() const noexcept
  /// \brief Returns the value of the word1::data1 field.
  /// \returns The value of the word1::data1 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data1(adt::uinteger_t<word1::data1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data1 field.
  /// \param v  The value to be assigned to the word1::data1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data1, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data2() const noexcept
  /// \brief Returns the value of the word1::data2 field.
  /// \returns The value of the word1::data2 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data2(adt::uinteger_t<word1::data2::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data2 field.
  /// \param v  The value to be assigned to the word1::data2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data2, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data3() const noexcept
  /// \brief Returns the value of the word1::data3 field.
  /// \returns The value of the word1::data3 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data3(adt::uinteger_t<word1::data3::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data3 field.
  /// \param v  The value to be assigned to the word1::data3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data3, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data4() const noexcept
  /// \brief Returns the value of the word2::data4 field.
  /// \returns The value of the word2::data4 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data4(adt::uinteger_t<word2::data4::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::data4 field.
  /// \param v  The value to be assigned to the word2::data4 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, data4, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data5() const noexcept
  /// \brief Returns the value of the word2::data5 field.
  /// \returns The value of the word2::data5 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data5(adt::uinteger_t<word2::data5::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::data5 field.
  /// \param v  The value to be assigned to the word2::data5 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, data5, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data6() const noexcept
  /// \brief Returns the value of the word2::data6 field.
  /// \returns The value of the word2::data6 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data6(adt::uinteger_t<word2::data6::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::data6 field.
  /// \param v  The value to be assigned to the word2::data6 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, data6, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data7() const noexcept
  /// \brief Returns the value of the word2::data7 field.
  /// \returns The value of the word2::data7 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data7(adt::uinteger_t<word2::data7::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::data7 field.
  /// \param v  The value to be assigned to the word2::data7 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, data7, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data8() const noexcept
  /// \brief Returns the value of the word3::data8 field.
  /// \returns The value of the word3::data8 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data8(adt::uinteger_t<word3::data8::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::data8 field.
  /// \param v  The value to be assigned to the word3::data8 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, data8, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data9() const noexcept
  /// \brief Returns the value of the word3::data9 field.
  /// \returns The value of the word3::data9 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data9(adt::uinteger_t<word3::data9::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::data9 field.
  /// \param v  The value to be assigned to the word3::data9 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, data9, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data10() const noexcept
  /// \brief Returns the value of the word3::data10 field.
  /// \returns The value of the word3::data10 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data10(adt::uinteger_t<word3::data10::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::data10 field.
  /// \param v  The value to be assigned to the word3::data10 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, data10, ump::flex_data::text_common)
  /// \fn constexpr auto midi2::ump::flex_data::text_common::data11() const noexcept
  /// \brief Returns the value of the word3::data11 field.
  /// \returns The value of the word3::data11 field.
  //
  /// \fn constexpr ump::flex_data::text_common & midi2::ump::flex_data::text_common::data11(adt::uinteger_t<word3::data11::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::data11 field.
  /// \param v  The value to be assigned to the word3::data11 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, data11, ump::flex_data::text_common)

  /// The type of the characters held by the message.
  using value_type = char8_t;
  /// An unsigned integer type used to represent sizes and indices.
  using size_type = std::size_t;
  /// A signed integer type used to represent the distance between two iterators.
  using difference_type = std::ptrdiff_t;
  /// A random-access iterator over the characters of the message.
  using iterator = details::iterator_base<text_common>;
  /// A random-access iterator over the characters of a const message.
  using const_iterator = details::iterator_base<text_common const>;

  /// \returns An iterator referring to the first character.
  [[nodiscard]] constexpr auto begin() noexcept { return iterator{*this, 0}; }
  /// \returns A const iterator referring to the first character.
  [[nodiscard]] constexpr auto begin() const noexcept { return const_iterator{*this, 0}; }
  /// \returns A const iterator referring to the first character.
  [[nodiscard]] constexpr auto cbegin() const noexcept { return const_iterator{*this, 0}; }

  /// \returns An iterator referring to the position after the last character.
  [[nodiscard]] constexpr auto end() noexcept { return iterator{*this, this->number_of_bytes()}; }
  /// \returns A const iterator referring to the position after the last character.
  [[nodiscard]] constexpr auto end() const noexcept { return const_iterator{*this, this->number_of_bytes()}; }
  /// \returns A const iterator referring to the position after the last character.
  [[nodiscard]] constexpr auto cend() const noexcept { return const_iterator{*this, this->number_of_bytes()}; }

  /// \returns The maximum number of characters that the message can hold.
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[nodiscard]] constexpr size_type max_size() const noexcept { return 12U; }
  /// \returns The number of characters in the message.
  [[nodiscard]] constexpr size_type size() const noexcept { return this->number_of_bytes(); }
  /// \returns True if the message contains no characters, false otherwise.
  [[nodiscard]] constexpr bool empty() const noexcept { return this->size() == 0; }

#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
  /// Provides access to the character at position \p idx.
  /// \param self  The message object.
  /// \param idx  The index of the character to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read or assign the character.
  constexpr decltype(auto) operator[](this auto& self, std::size_t idx) noexcept {
    return midi2::ump::details::array_subscript_proxy<std::remove_reference_t<decltype(self)>>{self, idx};
  }
#else
  /// Provides access to the character at position \p idx.
  /// \param idx  The index of the character to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read or assign the character.
  constexpr decltype(auto) operator[](std::size_t idx) noexcept {
    return midi2::ump::details::array_subscript_proxy<text_common>{*this, idx};
  }
  /// Provides read-only access to the character at position \p idx.
  /// \param idx  The index of the character to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read the character.
  constexpr decltype(auto) operator[](std::size_t idx) const noexcept {
    return midi2::ump::details::array_subscript_proxy<text_common const>{*this, idx};
  }
#endif

  /// Sets the character at position \p index.
  /// \param index  The index of the character to be set. Must be less than max_size().
  /// \param v  The value to be stored.
  /// \returns *this
  constexpr text_common& data(std::size_t const index, value_type const v) noexcept {
    switch (index) {
    case 0: this->data0(v); break;
    case 1: this->data1(v); break;
    case 2: this->data2(v); break;
    case 3: this->data3(v); break;
    case 4: this->data4(v); break;
    case 5: this->data5(v); break;
    case 6: this->data6(v); break;
    case 7: this->data7(v); break;
    case 8: this->data8(v); break;
    case 9: this->data9(v); break;
    case 10: this->data10(v); break;
    case 11: this->data11(v); break;
    default: assert(false && "Index out of range"); break;
    }
    return *this;
  }
  /// Returns the character at position \p index.
  /// \param index  The index of the character to be returned. Must be less than max_size().
  /// \returns The character at position \p index.
  [[nodiscard]] constexpr value_type data(std::size_t const index) const noexcept {
    switch (index) {
    case 0: return this->data0();
    case 1: return this->data1();
    case 2: return this->data2();
    case 3: return this->data3();
    case 4: return this->data4();
    case 5: return this->data5();
    case 6: return this->data6();
    case 7: return this->data7();
    case 8: return this->data8();
    case 9: return this->data9();
    case 10: return this->data10();
    case 11: return this->data11();
    default: assert(false && "Index out of range"); return 0;
    }
  }

  /// Copies characters from an initializer list into the message. Any unused bytes are set to zero.
  /// \tparam T  An integral type.
  /// \param vs  The characters to be copied. At most max_size() values are copied.
  /// \returns *this
  template <std::integral T>
    requires std::is_convertible_v<T, char8_t>
  constexpr text_common& data(std::initializer_list<T> vs) {
    auto const index = details::data_helper<text_common, 8U>::data(*this, vs);
    auto const begin = this->begin();
    std::fill(begin + index, begin + this->max_size(), '\0');
    return *this;
  }

  /// Copies characters from a range into the message. Any unused bytes are set to zero.
  /// \tparam Range  An input range whose elements are integral values.
  /// \tparam Proj  The type of the projection applied to each element of the range.
  /// \param range  The characters to be copied. At most max_size() values are copied.
  /// \param proj  A projection applied to each element of the range.
  /// \returns *this
  template <std::ranges::input_range Range, typename Proj = std::identity>
    requires std::is_convertible_v<std::remove_cvref_t<std::ranges::range_value_t<Range>>, char8_t>
  constexpr text_common& data(Range&& range, Proj proj = {}) {
    auto const index = details::data_helper<text_common, 8U>::data(*this, std::forward<Range>(range), proj);
    auto const begin = this->begin();
    std::fill(begin + index, begin + this->max_size(), '\0');
    return *this;
  }

  /// Copies characters from the iterator range [\p first, \p last) into the message. Any unused bytes are set to
  ///   zero.
  /// \tparam I  An input iterator type whose value type is integral.
  /// \tparam S  A sentinel type for \p I.
  /// \param first  The beginning of the range of characters to be copied.
  /// \param last  The end of the range of characters to be copied.
  /// \returns *this
  template <std::input_iterator I, std::sentinel_for<I> S>
    requires std::is_convertible_v<std::remove_cvref_t<typename std::iterator_traits<I>::value_type>, char8_t>
  constexpr text_common& data(I first, S last) {
    auto const index = details::data_helper<text_common, 8U>::data(*this, first, last);
    auto const begin = this->begin();
    std::fill(begin + index, begin + this->max_size(), '\0');
    return *this;
  }

  /// Copies the UTF-8 string \p str into the message. At most max_size() code units are copied; any unused bytes are
  ///   set to zero.
  /// \param str  The string to be copied.
  /// \returns *this
  constexpr text_common& data(std::u8string_view const& str) {
    // NOLINTNEXTLINE(llvm-qualified-auto,readability-qualified-auto)
    auto const first = std::begin(str);
    return this->data(first, first + std::min(this->max_size(), str.size()) + 1);
  }

#if defined(MIDI2_TEXT_COMMON_STRING) && MIDI2_TEXT_COMMON_STRING
  /// \returns The text held by this message as a UTF-8 string.
  [[nodiscard]] constexpr std::u8string str() const { return {begin(), end()}; }
#endif
  /// Counts the number of bytes of text in the message. The text ends at the first zero byte, or after max_size()
  ///   bytes if there is no zero byte.
  /// \returns The number of bytes of text held by the message.
  [[nodiscard]] constexpr std::size_t number_of_bytes() const {
    std::size_t index = 0;
    // TODO: check whole words for 0 first. Search backwards.
    auto const limit = this->max_size();
    for (; index < limit; ++index) {
      if (this->data(index) == '\0') {
        break;
      }
    }
    return index;
  }

private:
  friend struct ::std::tuple_size<text_common>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::flex_data::text_common>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::flex_data::text_common
//
/// \class std::tuple_element<I, ::midi2::ump::flex_data::text_common>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::flex_data::text_common
MIDI2_UMP_TUPLE(flex_data, text_common)  // Define tuple_size and tuple_element for flex_data/text_common
MIDI2_SPAN_CTOR(flex_data, text_common)  // Define the span constructor for flex_data/text_common

static_assert(std::random_access_iterator<midi2::ump::flex_data::text_common::iterator>,
              "text_common::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::flex_data::text_common::const_iterator>,
              "text_common::const_iterator should meet the requirements of std::random_access_iterator");

//*     _      _          _ ___ ___  *
//*  __| |__ _| |_ __ _  / |_  | _ ) *
//* / _` / _` |  _/ _` | | |/ // _ \ *
//* \__,_\__,_|\__\__,_| |_/___\___/ *
//*                                  *

/// \brief An integral constant which holds the number of 32-bit words in a UMP data128 message.
template <> struct midi2::ump::message_size<midi2::ump::message_type::data128> : std::integral_constant<unsigned, 4> {};

/// Defines the C++ types that represent Data 128 messages
namespace midi2::ump::data128 {

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return get<I>(t.words_);
}

namespace details {

// 7.8 System Exclusive 8 (8-Bit) Messages
// SysEx8 in 1 UMP (word 1)
// SysEx8 Start (word 1)
// SysEx8 Continue (word 1)
// SysEx8 End (word 1)
template <ump::mt::data128 Status> class sysex8;

/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A const reference to word \p I of \p t.
template <std::size_t I, typename T> auto const& get(T const& t) noexcept {
  return get<I>(t.words_);
}
/// Returns a reference to word \p I of the UMP message \p t.
/// \tparam I  The index of the word to be returned.
/// \tparam T  The type of the UMP message.
/// \param t  The UMP message.
/// \returns A reference to word \p I of \p t.
template <std::size_t I, typename T> auto& get(T& t) noexcept {
  return get<I>(t.words_);
}

}  // end namespace details

class mds_header;
class mds_payload;

}  // namespace midi2::ump::data128

/// \brief The System Exclusive 8 (8-Bit) message (section 7.8).
///
/// \tparam Status  The status value which distinguishes the complete, start, continue, and end
///   messages.
template <midi2::ump::mt::data128 Status> class midi2::ump::data128::details::sysex8 {
public:
  /// \brief The first word of the System Exclusive 8 (8-Bit) message.
  class word0 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = Status;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }
    /// Defines the bit position of the mt (message-type) field. Always 0x5 (message_type::data128).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always the Status template argument.
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the number of bytes field. This is the number of valid bytes (1-14) in this UMP,
    /// including the stream ID. In an end message, 0xF aborts the message.
    using number_of_bytes = adt::bit_range<16, 4>;
    /// Defines the bit position of the stream ID field. This allows multiple System Exclusive 8 messages to be
    /// interleaved.
    using stream_id = adt::bit_range<8, 8>;
    /// Defines the bit position of data byte 0.
    using data0 = adt::bit_range<0, 8>;
  };
  /// \brief The second word of the System Exclusive 8 (8-Bit) message.
  class word1 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of data byte 1.
    using data1 = adt::bit_range<24, 8>;
    /// Defines the bit position of data byte 2.
    using data2 = adt::bit_range<16, 8>;
    /// Defines the bit position of data byte 3.
    using data3 = adt::bit_range<8, 8>;
    /// Defines the bit position of data byte 4.
    using data4 = adt::bit_range<0, 8>;
  };
  /// \brief The third word of the System Exclusive 8 (8-Bit) message.
  class word2 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of data byte 5.
    using data5 = adt::bit_range<24, 8>;
    /// Defines the bit position of data byte 6.
    using data6 = adt::bit_range<16, 8>;
    /// Defines the bit position of data byte 7.
    using data7 = adt::bit_range<8, 8>;
    /// Defines the bit position of data byte 8.
    using data8 = adt::bit_range<0, 8>;
  };
  /// \brief The fourth word of the System Exclusive 8 (8-Bit) message.
  class word3 : public midi2::ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of data byte 9.
    using data9 = adt::bit_range<24, 8>;
    /// Defines the bit position of data byte 10.
    using data10 = adt::bit_range<16, 8>;
    /// Defines the bit position of data byte 11.
    using data11 = adt::bit_range<8, 8>;
    /// Defines the bit position of data byte 12.
    using data12 = adt::bit_range<0, 8>;
  };

  /// Default constructs a System Exclusive 8 (8-Bit) message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr sysex8() noexcept = default;
  /// Constructs a System Exclusive 8 (8-Bit) message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit sysex8(std::span<std::uint32_t, 4> const m) noexcept : words_{span_to_tuple(m)} {
    assert(get<0>(words_).check());
  }
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(sysex8 const&, sysex8 const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x5 (message_type::data128).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \brief Returns the value of the word0::status field. Always the Status template argument.
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::number_of_bytes() const noexcept
  /// \brief Returns the value of the word0::number_of_bytes field.
  /// \returns The value of the word0::number_of_bytes field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::number_of_bytes(adt::uinteger_t<word0::number_of_bytes::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::number_of_bytes field.
  /// \param v  The value to be assigned to the word0::number_of_bytes field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, number_of_bytes, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::stream_id() const noexcept
  /// \brief Returns the value of the word0::stream_id field.
  /// \returns The value of the word0::stream_id field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::stream_id(adt::uinteger_t<word0::stream_id::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::stream_id field.
  /// \param v  The value to be assigned to the word0::stream_id field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, stream_id, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data0() const noexcept
  /// \brief Returns the value of the word0::data0 field.
  /// \returns The value of the word0::data0 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data0(adt::uinteger_t<word0::data0::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::data0 field.
  /// \param v  The value to be assigned to the word0::data0 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, data0, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data1() const noexcept
  /// \brief Returns the value of the word1::data1 field.
  /// \returns The value of the word1::data1 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data1(adt::uinteger_t<word1::data1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data1 field.
  /// \param v  The value to be assigned to the word1::data1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data1, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data2() const noexcept
  /// \brief Returns the value of the word1::data2 field.
  /// \returns The value of the word1::data2 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data2(adt::uinteger_t<word1::data2::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data2 field.
  /// \param v  The value to be assigned to the word1::data2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data2, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data3() const noexcept
  /// \brief Returns the value of the word1::data3 field.
  /// \returns The value of the word1::data3 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data3(adt::uinteger_t<word1::data3::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data3 field.
  /// \param v  The value to be assigned to the word1::data3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data3, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data4() const noexcept
  /// \brief Returns the value of the word1::data4 field.
  /// \returns The value of the word1::data4 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data4(adt::uinteger_t<word1::data4::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::data4 field.
  /// \param v  The value to be assigned to the word1::data4 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, data4, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data5() const noexcept
  /// \brief Returns the value of the word2::data5 field.
  /// \returns The value of the word2::data5 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data5(adt::uinteger_t<word2::data5::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::data5 field.
  /// \param v  The value to be assigned to the word2::data5 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, data5, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data6() const noexcept
  /// \brief Returns the value of the word2::data6 field.
  /// \returns The value of the word2::data6 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data6(adt::uinteger_t<word2::data6::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::data6 field.
  /// \param v  The value to be assigned to the word2::data6 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, data6, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data7() const noexcept
  /// \brief Returns the value of the word2::data7 field.
  /// \returns The value of the word2::data7 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data7(adt::uinteger_t<word2::data7::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::data7 field.
  /// \param v  The value to be assigned to the word2::data7 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, data7, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data8() const noexcept
  /// \brief Returns the value of the word2::data8 field.
  /// \returns The value of the word2::data8 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data8(adt::uinteger_t<word2::data8::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::data8 field.
  /// \param v  The value to be assigned to the word2::data8 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, data8, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data9() const noexcept
  /// \brief Returns the value of the word3::data9 field.
  /// \returns The value of the word3::data9 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data9(adt::uinteger_t<word3::data9::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::data9 field.
  /// \param v  The value to be assigned to the word3::data9 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, data9, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data10() const noexcept
  /// \brief Returns the value of the word3::data10 field.
  /// \returns The value of the word3::data10 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data10(adt::uinteger_t<word3::data10::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::data10 field.
  /// \param v  The value to be assigned to the word3::data10 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, data10, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data11() const noexcept
  /// \brief Returns the value of the word3::data11 field.
  /// \returns The value of the word3::data11 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data11(adt::uinteger_t<word3::data11::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::data11 field.
  /// \param v  The value to be assigned to the word3::data11 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, data11, ump::data128::details::sysex8<Status>)
  /// \fn constexpr auto midi2::ump::data128::details::sysex8::data12() const noexcept
  /// \brief Returns the value of the word3::data12 field.
  /// \returns The value of the word3::data12 field.
  //
  /// \fn constexpr ump::data128::details::sysex8<Status> & midi2::ump::data128::details::sysex8::data12(adt::uinteger_t<word3::data12::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::data12 field.
  /// \param v  The value to be assigned to the word3::data12 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, data12, ump::data128::details::sysex8<Status>)

  /// The type of the data bytes held by the message.
  using value_type = adt::uinteger_t<8>;
  /// An unsigned integer type used to represent sizes and indices.
  using size_type = std::size_t;
  /// A signed integer type used to represent the distance between two iterators.
  using difference_type = std::ptrdiff_t;
  /// A random-access iterator over the data bytes of the message.
  using iterator = ::midi2::ump::details::iterator_base<sysex8>;
  /// A random-access iterator over the data bytes of a const message.
  using const_iterator = ::midi2::ump::details::iterator_base<sysex8 const>;

  /// \returns An iterator referring to the first data byte.
  [[nodiscard]] constexpr auto begin() noexcept { return iterator{*this, 0}; }
  /// \returns A const iterator referring to the first data byte.
  [[nodiscard]] constexpr auto begin() const noexcept { return const_iterator{*this, 0}; }
  /// \returns A const iterator referring to the first data byte.
  [[nodiscard]] constexpr auto cbegin() noexcept { return const_iterator{*this, 0}; }

  /// \returns An iterator referring to the position after the last data byte.
  [[nodiscard]] constexpr auto end() noexcept { return iterator{*this, this->number_of_bytes()}; }
  /// \returns A const iterator referring to the position after the last data byte.
  [[nodiscard]] constexpr auto end() const noexcept { return const_iterator{*this, this->number_of_bytes()}; }
  /// \returns A const iterator referring to the position after the last data byte.
  [[nodiscard]] constexpr auto cend() noexcept { return const_iterator{*this, this->number_of_bytes()}; }

  /// \returns The maximum number of data bytes that the message can hold.
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  [[nodiscard]] constexpr size_type max_size() const noexcept { return 13U; }
  /// \returns The number of data bytes in the message.
  [[nodiscard]] constexpr size_type size() const noexcept { return this->number_of_bytes(); }
  /// \returns True if the message contains no data bytes, false otherwise.
  [[nodiscard]] constexpr bool empty() const noexcept { return this->size() == 0; }

#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
  /// Provides access to the data byte at position \p idx.
  /// \param self  The message object.
  /// \param idx  The index of the data byte to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read or assign the data byte.
  constexpr decltype(auto) operator[](this auto& self, std::size_t idx) noexcept {
    return midi2::ump::details::array_subscript_proxy<std::remove_reference_t<decltype(self)>>{self, idx};
  }
#else
  /// Provides access to the data byte at position \p idx.
  /// \param idx  The index of the data byte to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read or assign the data byte.
  constexpr decltype(auto) operator[](std::size_t idx) noexcept {
    return midi2::ump::details::array_subscript_proxy<sysex8>{*this, idx};
  }
  /// Provides read-only access to the data byte at position \p idx.
  /// \param idx  The index of the data byte to be accessed. Must be less than max_size().
  /// \returns A proxy object which can be used to read the data byte.
  constexpr decltype(auto) operator[](std::size_t idx) const noexcept {
    return midi2::ump::details::array_subscript_proxy<sysex8 const>{*this, idx};
  }
#endif

  /// Sets the data byte at position \p index.
  /// \param index  The index of the data byte to be set. Must be less than max_size().
  /// \param v  The value to be stored.
  /// \returns *this
  constexpr sysex8& data(std::size_t const index, adt::uinteger_t<8> const v) noexcept {
    switch (index) {
    case 0: this->data0(v); break;
    case 1: this->data1(v); break;
    case 2: this->data2(v); break;
    case 3: this->data3(v); break;
    case 4: this->data4(v); break;
    case 5: this->data5(v); break;
    case 6: this->data6(v); break;
    case 7: this->data7(v); break;
    case 8: this->data8(v); break;
    case 9: this->data9(v); break;
    case 10: this->data10(v); break;
    case 11: this->data11(v); break;
    case 12: this->data12(v); break;
    default: assert(false && "Index out of range"); break;
    }
    return *this;
  }
  /// Returns the data byte at position \p index.
  /// \param index  The index of the data byte to be returned. Must be less than max_size().
  /// \returns The data byte at position \p index.
  [[nodiscard]] constexpr adt::uinteger_t<8> data(std::size_t const index) const noexcept {
    switch (index) {
    case 0: return this->data0();
    case 1: return this->data1();
    case 2: return this->data2();
    case 3: return this->data3();
    case 4: return this->data4();
    case 5: return this->data5();
    case 6: return this->data6();
    case 7: return this->data7();
    case 8: return this->data8();
    case 9: return this->data9();
    case 10: return this->data10();
    case 11: return this->data11();
    case 12: return this->data12();
    default: assert(false && "Index out of range"); return 0;
    }
  }

  /// Copies data bytes from an initializer list into the message. The number_of_bytes field is set to the number of
  ///   values copied.
  /// \tparam T  An integral type.
  /// \param vs  The data bytes to be copied. At most max_size() values are copied.
  /// \returns *this
  template <std::integral T>
    requires std::is_convertible_v<T, char8_t>
  constexpr sysex8& data(std::initializer_list<T> vs) {
    auto const index = ::midi2::ump::details::data_helper<sysex8, 8U>::data(*this, vs);
    return this->number_of_bytes(static_cast<word0::number_of_bytes::uinteger>(index));
  }

  /// Copies data bytes from a range into the message. The number_of_bytes field is set to the number of values
  ///   copied.
  /// \tparam Range  An input range whose elements are integral values.
  /// \tparam Proj  The type of the projection applied to each element of the range.
  /// \param range  The data bytes to be copied. At most max_size() values are copied.
  /// \param proj  A projection applied to each element of the range.
  /// \returns *this
  template <std::ranges::input_range Range, typename Proj = std::identity>
    requires std::is_integral_v<std::remove_cvref_t<std::ranges::range_value_t<Range>>>
  constexpr sysex8& data(Range&& range, Proj proj = {}) {
    auto const index = ::midi2::ump::details::data_helper<sysex8, 8U>::data(*this, std::forward<Range>(range), proj);
    return this->number_of_bytes(static_cast<word0::number_of_bytes::uinteger>(index));
  }

  /// Copies data bytes from the iterator range [\p first, \p last) into the message. The number_of_bytes field is set
  ///   to the number of values copied.
  /// \tparam I  An input iterator type whose value type is integral.
  /// \tparam S  A sentinel type for \p I.
  /// \param first  The beginning of the range of data bytes to be copied.
  /// \param last  The end of the range of data bytes to be copied.
  /// \returns *this
  template <std::input_iterator I, std::sentinel_for<I> S>
    requires std::is_integral_v<std::remove_cvref_t<typename std::iterator_traits<I>::value_type>>
  constexpr sysex8& data(I first, S last) {
    auto const index = ::midi2::ump::details::data_helper<sysex8, 8U>::data(*this, first, last);
    return this->number_of_bytes(static_cast<word0::number_of_bytes::uinteger>(index));
  }

private:
  friend struct ::std::tuple_size<sysex8>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_{};
};

/// \brief Specialization of `std::tuple_size<>` for midi2::ump::data128::details::sysex8<>.
/// \tparam Status  The sysex8 message status.
template <midi2::ump::mt::data128 Status>
struct std::tuple_size<midi2::ump::data128::details::sysex8<Status>> /* NOLINT(cert-dcl58-cpp]*/
    : std::integral_constant<std::size_t,
                             std::tuple_size_v<decltype(midi2::ump::data128::details::sysex8<Status>::words_)>> {};

/// \brief Specialization of `std::tuple_element<>` for midi2::ump::data128::details::sysex8<>.
/// \tparam I  The index of the word.
/// \tparam Status  The sysex8 message status.
template <std::size_t I, midi2::ump::mt::data128 Status>
struct std::tuple_element<I, midi2::ump::data128::details::sysex8<Status>> { /* NOLINT(cert-dcl58-cpp] */
  /// The type of the Ith word of the tuple-like type midi2::ump::data128::details::sysex8<>.
  using type = std::tuple_element_t<I, decltype(midi2::ump::data128::details::sysex8<Status>::words_)>;
};

namespace midi2::ump::data128 {

/// The message type used for 8-bit wide system exclusive data packed into a single UMP message.
using sysex8_in_1 = details::sysex8<midi2::ump::mt::data128::sysex8_in_1>;
/// \brief The start of a block of 8-bit system exclusive data that is split over zero or more of
///   sysex8_continue messages and completed by a sysex8_end message.
using sysex8_start = details::sysex8<midi2::ump::mt::data128::sysex8_start>;
/// \brief A block of 128 bits of system exclusive data. They can be sent as many times as necessary
///   but must be preceded by a sysex8_start message.
using sysex8_continue = details::sysex8<midi2::ump::mt::data128::sysex8_continue>;
/// A message which signals the end of a series of sysex8_start and sysex8_continue messages.
using sysex8_end = details::sysex8<midi2::ump::mt::data128::sysex8_end>;

}  // end namespace midi2::ump::data128

static_assert(std::random_access_iterator<midi2::ump::data128::sysex8_in_1::iterator>,
              "sysex8_in_1::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::data128::sysex8_in_1::const_iterator>,
              "sysex8_in_1::const_iterator should meet the requirements of std::random_access_iterator");

static_assert(std::random_access_iterator<midi2::ump::data128::sysex8_start::iterator>,
              "sysex8_start::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::data128::sysex8_start::const_iterator>,
              "sysex8_start::const_iterator should meet the requirements of std::random_access_iterator");

static_assert(std::random_access_iterator<midi2::ump::data128::sysex8_continue::iterator>,
              "sysex8_continue::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::data128::sysex8_continue::const_iterator>,
              "sysex8_continue::const_iterator should meet the requirements of std::random_access_iterator");

static_assert(std::random_access_iterator<midi2::ump::data128::sysex8_end::iterator>,
              "sysex8_end::iterator should meet the requirements of std::random_access_iterator");
static_assert(std::random_access_iterator<midi2::ump::data128::sysex8_end::const_iterator>,
              "sysex8_end::const_iterator should meet the requirements of std::random_access_iterator");

/// \brief The header message for a Mixed Data Set sequence.
///
/// Mixed Data Set messages can carry any data payload, without the 7-bit restriction of the MIDI 1.0
/// Protocol. This mechanism is targeted primarily for use with large data sets, including non-MIDI
/// data.
class midi2::ump::data128::mds_header {
public:
  /// \brief The first word of the Mixed Data Set Header message.
  class word0 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = ump::mt::data128::mixed_data_set_header;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x5 (message_type::data128).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x8 (mt::data128::mixed_data_set_header).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the MDS ID field. This ties together the chunks of a mixed data set and
    /// distinguishes up to 16 simultaneous mixed data sets within one group.
    using mds_id = adt::bit_range<16, 4>;
    /// Defines the bit position of the number of valid bytes in this message chunk field (including the header).
    using bytes_in_chunk = adt::bit_range<0, 16>;
  };
  /// \brief The second word of the Mixed Data Set Header message.
  class word1 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of the number of chunks in the mixed data set field. 0 means that the number of chunks
    /// is unknown.
    using chunks_in_mds = adt::bit_range<16, 16>;
    /// Defines the bit position of the number of this chunk field. Chunks are numbered from 1.
    using chunk_num = adt::bit_range<0, 16>;
  };
  /// \brief The third word of the Mixed Data Set Header message.
  class word2 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of the 16-bit manufacturer ID field.
    using manufacturer_id = adt::bit_range<16, 16>;
    /// Defines the bit position of the device ID field. 0xFFFF addresses all devices ("all call").
    using device_id = adt::bit_range<0, 16>;
  };
  /// \brief The fourth word of the Mixed Data Set Header message.
  class word3 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of the sub ID #1 field.
    using sub_id_1 = adt::bit_range<16, 16>;
    /// Defines the bit position of the sub ID #2 field.
    using sub_id_2 = adt::bit_range<0, 16>;
  };

  /// Default constructs a Mixed Data Set Header message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr mds_header() noexcept = default;
  /// Constructs a Mixed Data Set Header message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit mds_header(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(mds_header const&, mds_header const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x5 (message_type::data128).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::data128::mds_header::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::data128::mds_header)
  /// \brief Returns the value of the word0::status field. Always 0x8 (mt::data128::mixed_data_set_header).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::data128::mds_header::mds_id() const noexcept
  /// \brief Returns the value of the word0::mds_id field.
  /// \returns The value of the word0::mds_id field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::mds_id(adt::uinteger_t<word0::mds_id::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::mds_id field.
  /// \param v  The value to be assigned to the word0::mds_id field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, mds_id, ump::data128::mds_header)
  /// \fn constexpr auto midi2::ump::data128::mds_header::bytes_in_chunk() const noexcept
  /// \brief Returns the value of the word0::bytes_in_chunk field.
  /// \returns The value of the word0::bytes_in_chunk field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::bytes_in_chunk(adt::uinteger_t<word0::bytes_in_chunk::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::bytes_in_chunk field.
  /// \param v  The value to be assigned to the word0::bytes_in_chunk field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, bytes_in_chunk, ump::data128::mds_header)
  /// \fn constexpr auto midi2::ump::data128::mds_header::chunks_in_mds() const noexcept
  /// \brief Returns the value of the word1::chunks_in_mds field.
  /// \returns The value of the word1::chunks_in_mds field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::chunks_in_mds(adt::uinteger_t<word1::chunks_in_mds::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::chunks_in_mds field.
  /// \param v  The value to be assigned to the word1::chunks_in_mds field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, chunks_in_mds, ump::data128::mds_header)
  /// \fn constexpr auto midi2::ump::data128::mds_header::chunk_num() const noexcept
  /// \brief Returns the value of the word1::chunk_num field.
  /// \returns The value of the word1::chunk_num field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::chunk_num(adt::uinteger_t<word1::chunk_num::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::chunk_num field.
  /// \param v  The value to be assigned to the word1::chunk_num field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, chunk_num, ump::data128::mds_header)
  /// \fn constexpr auto midi2::ump::data128::mds_header::manufacturer_id() const noexcept
  /// \brief Returns the value of the word2::manufacturer_id field.
  /// \returns The value of the word2::manufacturer_id field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::manufacturer_id(adt::uinteger_t<word2::manufacturer_id::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::manufacturer_id field.
  /// \param v  The value to be assigned to the word2::manufacturer_id field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, manufacturer_id, ump::data128::mds_header)
  /// \fn constexpr auto midi2::ump::data128::mds_header::device_id() const noexcept
  /// \brief Returns the value of the word2::device_id field.
  /// \returns The value of the word2::device_id field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::device_id(adt::uinteger_t<word2::device_id::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::device_id field.
  /// \param v  The value to be assigned to the word2::device_id field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, device_id, ump::data128::mds_header)
  /// \fn constexpr auto midi2::ump::data128::mds_header::sub_id_1() const noexcept
  /// \brief Returns the value of the word3::sub_id_1 field.
  /// \returns The value of the word3::sub_id_1 field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::sub_id_1(adt::uinteger_t<word3::sub_id_1::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::sub_id_1 field.
  /// \param v  The value to be assigned to the word3::sub_id_1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, sub_id_1, ump::data128::mds_header)
  /// \fn constexpr auto midi2::ump::data128::mds_header::sub_id_2() const noexcept
  /// \brief Returns the value of the word3::sub_id_2 field.
  /// \returns The value of the word3::sub_id_2 field.
  //
  /// \fn constexpr ump::data128::mds_header & midi2::ump::data128::mds_header::sub_id_2(adt::uinteger_t<word3::sub_id_2::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::sub_id_2 field.
  /// \param v  The value to be assigned to the word3::sub_id_2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, sub_id_2, ump::data128::mds_header)

private:
  friend struct ::std::tuple_size<mds_header>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::data128::mds_header>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::data128::mds_header
//
/// \class std::tuple_element<I, ::midi2::ump::data128::mds_header>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::data128::mds_header
MIDI2_UMP_TUPLE(data128, mds_header)  // Define tuple_size and tuple_element for data128/mds_header
MIDI2_SPAN_CTOR(data128, mds_header)  // Define the span constructor for data128/mds_header

/// \brief The payload message for a Mixed Data Set (MDS) sequence.
///
/// Mixed Data Set messages can carry any data payload, without the 7-bit restriction of the MIDI 1.0
/// Protocol.
class midi2::ump::data128::mds_payload {
public:
  /// \brief The first 32-bit word of an MDS payload message
  class word0 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// The status value which identifies this message.
    static constexpr auto message = midi2::ump::mt::data128::mixed_data_set_payload;
    /// Default constructs the word with the mt and status fields set to the values which identify
    /// this message. All other fields are zero.
    constexpr word0() noexcept { this->init<mt, status>(message); }
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    [[nodiscard]] constexpr bool check() const noexcept { return this->word_base::check<mt, status>(message); }

    /// Defines the bit position of the mt (message-type) field. Always 0x5 (message_type::data128).
    using mt = adt::bit_range<28, 4>;
    /// Defines the bit position of the group field
    using group = adt::bit_range<24, 4>;
    /// Defines the bit position of the status field. Always 0x9 (mt::data128::mixed_data_set_payload).
    using status = adt::bit_range<20, 4>;
    /// Defines the bit position of the MDS ID field. This ties together the chunks of a mixed data set and
    /// distinguishes up to 16 simultaneous mixed data sets within one group.
    using mds_id = adt::bit_range<16, 4>;
    /// Defines the bit position of the first 16 bits of payload data.
    using value0 = adt::bit_range<0, 16>;
  };
  /// \brief The second 32-bit word of an MDS payload message
  class word1 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of 32 bits of payload data.
    using value1 = adt::bit_range<0, 32>;
  };
  /// \brief The third 32-bit word of an MDS payload message
  class word2 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }

    /// Defines the bit position of 32 bits of payload data.
    using value2 = adt::bit_range<0, 32>;
  };
  /// \brief The fourth 32-bit word of an MDS payload message
  class word3 : public ump::details::word_base {
  public:
    using word_base::word_base;
    /// Checks that the fields of this word hold valid values for this message.
    /// \returns True if the word is valid, false otherwise.
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] constexpr bool check() const noexcept { return true; }
    /// Defines the bit position of 32 bits of payload data.
    using value3 = adt::bit_range<0, 32>;
  };

  /// Default constructs a Mixed Data Set Payload message. The fields which identify the message are
  /// initialized; all other fields are zero.
  constexpr mds_payload() noexcept = default;
  /// Constructs a Mixed Data Set Payload message from 4 raw 32-bit words.
  /// In a debug build, checks that the class invariants hold.
  /// \param m  The words of the message.
  constexpr explicit mds_payload(std::span<std::uint32_t, 4> m) noexcept;
  /// Compares two messages for equality.
  /// \returns True if the two messages are equal, false otherwise.
  friend constexpr bool operator==(mds_payload const&, mds_payload const&) noexcept = default;

  /// \brief Returns the value of the word0::mt field. Always 0x5 (message_type::data128).
  /// \note This is a read-only field.
  /// \returns The value of the word0::mt field.
  MIDI2_UMP_GETTER(word0, mt)
  /// \fn constexpr auto midi2::ump::data128::mds_payload::group() const noexcept
  /// \brief Returns the value of the word0::group field.
  /// \returns The value of the word0::group field.
  //
  /// \fn constexpr ump::data128::mds_payload & midi2::ump::data128::mds_payload::group(adt::uinteger_t<word0::group::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::group field.
  /// \param v  The value to be assigned to the word0::group field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, group, ump::data128::mds_payload)
  /// \brief Returns the value of the word0::status field. Always 0x9 (mt::data128::mixed_data_set_payload).
  /// \note This is a read-only field.
  /// \returns The value of the word0::status field.
  MIDI2_UMP_GETTER(word0, status)
  /// \fn constexpr auto midi2::ump::data128::mds_payload::mds_id() const noexcept
  /// \brief Returns the value of the word0::mds_id field.
  /// \returns The value of the word0::mds_id field.
  //
  /// \fn constexpr ump::data128::mds_payload & midi2::ump::data128::mds_payload::mds_id(adt::uinteger_t<word0::mds_id::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::mds_id field.
  /// \param v  The value to be assigned to the word0::mds_id field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, mds_id, ump::data128::mds_payload)
  /// \fn constexpr auto midi2::ump::data128::mds_payload::value0() const noexcept
  /// \brief Returns the value of the word0::value0 field.
  /// \returns The value of the word0::value0 field.
  //
  /// \fn constexpr ump::data128::mds_payload & midi2::ump::data128::mds_payload::value0(adt::uinteger_t<word0::value0::bits::value> const v) noexcept
  /// \brief Sets the value of the word0::value0 field.
  /// \param v  The value to be assigned to the word0::value0 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word0, value0, ump::data128::mds_payload)
  /// \fn constexpr auto midi2::ump::data128::mds_payload::value1() const noexcept
  /// \brief Returns the value of the word1::value1 field.
  /// \returns The value of the word1::value1 field.
  //
  /// \fn constexpr ump::data128::mds_payload & midi2::ump::data128::mds_payload::value1(adt::uinteger_t<word1::value1::bits::value> const v) noexcept
  /// \brief Sets the value of the word1::value1 field.
  /// \param v  The value to be assigned to the word1::value1 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word1, value1, ump::data128::mds_payload)
  /// \fn constexpr auto midi2::ump::data128::mds_payload::value2() const noexcept
  /// \brief Returns the value of the word2::value2 field.
  /// \returns The value of the word2::value2 field.
  //
  /// \fn constexpr ump::data128::mds_payload & midi2::ump::data128::mds_payload::value2(adt::uinteger_t<word2::value2::bits::value> const v) noexcept
  /// \brief Sets the value of the word2::value2 field.
  /// \param v  The value to be assigned to the word2::value2 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word2, value2, ump::data128::mds_payload)
  /// \fn constexpr auto midi2::ump::data128::mds_payload::value3() const noexcept
  /// \brief Returns the value of the word3::value3 field.
  /// \returns The value of the word3::value3 field.
  //
  /// \fn constexpr ump::data128::mds_payload & midi2::ump::data128::mds_payload::value3(adt::uinteger_t<word3::value3::bits::value> const v) noexcept
  /// \brief Sets the value of the word3::value3 field.
  /// \param v  The value to be assigned to the word3::value3 field.
  /// \returns *this
  MIDI2_UMP_GETTER_SETTER(word3, value3, ump::data128::mds_payload)

private:
  friend struct ::std::tuple_size<mds_payload>;
  template <std::size_t I, typename T> friend struct ::std::tuple_element;
  template <std::size_t I, typename T> friend auto const& get(T const&) noexcept;
  template <std::size_t I, typename T> friend auto& get(T&) noexcept;

  std::tuple<word0, word1, word2, word3> words_;
};
/// \class std::tuple_size<::midi2::ump::data128::mds_payload>
/// \brief Specialization of `std::tuple_size<>` for midi2::ump::data128::mds_payload
//
/// \class std::tuple_element<I, ::midi2::ump::data128::mds_payload>
/// \brief Specialization of `std::tuple_element<>` for midi2::ump::data128::mds_payload
MIDI2_UMP_TUPLE(data128, mds_payload)  // Define tuple_size and tuple_element for data128/mds_payload
MIDI2_SPAN_CTOR(data128, mds_payload)  // Define the span constructor for data128/mds_payload

/// \brief An integral constant which holds the number of 32-bit words in a UMP reserved32_06 message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved32_06> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of 32-bit words in a UMP reserved32_07 message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved32_07> : std::integral_constant<unsigned, 1> {};
/// \brief An integral constant which holds the number of 32-bit words in a UMP reserved64_08 message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved64_08> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of 32-bit words in a UMP reserved64_09 message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved64_09> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of 32-bit words in a UMP reserved64_0a message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved64_0a> : std::integral_constant<unsigned, 2> {};
/// \brief An integral constant which holds the number of 32-bit words in a UMP reserved96_0b message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved96_0b> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of 32-bit words in a UMP reserved96_0c message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved96_0c> : std::integral_constant<unsigned, 3> {};
/// \brief An integral constant which holds the number of 32-bit words in a UMP reserved128_0e message.
template <>
struct midi2::ump::message_size<midi2::ump::message_type::reserved128_0e> : std::integral_constant<unsigned, 4> {};

#undef MIDI2_UMP_GETTER
#undef MIDI2_UMP_GETTER_ENUM
#undef MIDI2_UMP_SETTER
#undef MIDI2_UMP_SETTER_ENUM
#undef MIDI2_UMP_GETTER_SETTER
#undef MIDI2_UMP_GETTER_SETTER_ENUM
#undef MIDI2_UMP_TUPLE
#undef MIDI2_SPAN_CTOR
#undef MIDI2_SHARPS_FLATS
#undef MIDI2_NOTE
#undef MIDI2_CHORD_TYPE

#endif  // MIDI2_UMP_TYPES_HPP
