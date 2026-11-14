//===-- UMP Dispatcher Backends -----------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ump_dispatcher_backend.hpp
/// \brief Contains concepts and types for the UMP dispatcher

#ifndef MIDI2_UMP_DISPATCHER_BACKEND_HPP
#define MIDI2_UMP_DISPATCHER_BACKEND_HPP

#include <concepts>
#include <cstdint>
#include <functional>
#include <span>

#include "midi2/ump/ump_types.hpp"
#include "midi2/utils.hpp"

/// \brief Concepts and types for the UMP dispatcher
namespace midi2::ump::dispatcher_backend {

// clang-format off
/// \brief Satisfied by types that can handle Utility messages.
///
/// The type must provide the member functions noop(), jr_clock(), jr_timestamp(), delta_clockstamp_tpqn(),
/// delta_clockstamp() and unknown(). Each takes the context value as its first argument and (except for noop()) the
/// message as its second, and returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept utility = requires(T v, Context context) {
  // 7.2.1 NOOP
  { v.noop(context) } -> std::same_as<void>;
  // 7.2.2.1 JR Clock Message
  { v.jr_clock(context, utility::jr_clock{}) } -> std::same_as<void>;
  // 7.2.2.2 JR Timestamp Message
  { v.jr_timestamp(context, utility::jr_timestamp{}) } -> std::same_as<void>;
  // 7.2.3.1 Delta Clockstamp Ticks Per Quarter Note (TPQN)
  { v.delta_clockstamp_tpqn(context, utility::delta_clockstamp_tpqn{}) } -> std::same_as<void>;
  // 7.2.3.2 Delta Clockstamp (DC): Ticks Since Last Event
  { v.delta_clockstamp(context, utility::delta_clockstamp{}) } -> std::same_as<void>;

  { v.unknown(context, std::span<std::uint32_t>{}) } -> std::same_as<void>;
};
/// \brief Satisfied by types that can handle System Common and System Real Time messages.
///
/// The type must provide the member functions midi_time_code(), song_position_pointer(), song_select(), tune_request(),
/// timing_clock(), seq_start(), seq_continue(), seq_stop(), active_sensing() and reset(). Each takes the context value
/// as its first argument and the message as its second, and returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept system = requires(T v, Context context) {
  // 7.6 System Common and System Real Time Messages
  { v.midi_time_code(context, system::midi_time_code{}) } -> std::same_as<void>;
  { v.song_position_pointer(context, system::song_position_pointer{}) } -> std::same_as<void>;
  { v.song_select(context, system::song_select{}) } -> std::same_as<void>;
  { v.tune_request(context, system::tune_request{}) } -> std::same_as<void>;
  { v.timing_clock(context, system::timing_clock{}) } -> std::same_as<void>;
  { v.seq_start(context, system::sequence_start{}) } -> std::same_as<void>;
  { v.seq_continue(context, system::sequence_continue{}) } -> std::same_as<void>;
  { v.seq_stop(context, system::sequence_stop{}) } -> std::same_as<void>;
  { v.active_sensing(context, system::active_sensing{}) } -> std::same_as<void>;
  { v.reset(context, system::reset{}) } -> std::same_as<void>;
};
/// \brief Satisfied by types that can handle MIDI 1.0 Channel Voice messages.
///
/// The type must provide the member functions note_off(), note_on(), poly_pressure(), control_change(),
/// program_change(), channel_pressure() and pitch_bend(). Each takes the context value as its first argument and the
/// message as its second, and returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template<typename T, typename Context>
concept m1cvm = requires(T v, Context context) {
  { v.note_off(context, m1cvm::note_off{}) } -> std::same_as<void>;
  { v.note_on(context, m1cvm::note_on{}) } -> std::same_as<void>;
  { v.poly_pressure(context, m1cvm::poly_pressure{}) } -> std::same_as<void>;
  { v.control_change(context, m1cvm::control_change{}) } -> std::same_as<void>;
  { v.program_change(context, m1cvm::program_change{}) } -> std::same_as<void>;
  { v.channel_pressure(context, m1cvm::channel_pressure{}) } -> std::same_as<void>;
  { v.pitch_bend(context, m1cvm::pitch_bend{}) } -> std::same_as<void>;
};
/// \brief Satisfied by types that can handle Data 64-bit (System Exclusive 7-bit) messages.
///
/// The type must provide the member functions sysex7_in_1(), sysex7_start(), sysex7_continue() and sysex7_end(). Each
/// takes the context value as its first argument and the message as its second, and returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept data64 = requires(T v, Context context) {
  { v.sysex7_in_1(context, data64::sysex7_in_1{}) } -> std::same_as<void>;
  { v.sysex7_start(context, data64::sysex7_start{}) } -> std::same_as<void>;
  { v.sysex7_continue(context, data64::sysex7_continue{}) } -> std::same_as<void>;
  { v.sysex7_end(context, data64::sysex7_end{}) } -> std::same_as<void>;
};
/// \brief Satisfied by types that can handle MIDI 2.0 Channel Voice messages.
///
/// The type must provide the member functions note_off(), note_on(), poly_pressure(), rpn_per_note_controller(),
/// nrpn_per_note_controller(), per_note_management(), control_change(), rpn_controller(), nrpn_controller(),
/// rpn_relative_controller(), nrpn_relative_controller(), program_change(), channel_pressure(), pitch_bend() and
/// per_note_pitch_bend(). Each takes the context value as its first argument and the message as its second, and returns
/// void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept m2cvm = requires(T v, Context context) {
  // 7.4.1 MIDI 2.0 Note Off Message (status=0x8)
  { v.note_off(context, m2cvm::note_off{}) } -> std::same_as<void>;
  // 7.4.2 MIDI 2.0 Note On Message (status=0x9)
  { v.note_on(context, m2cvm::note_on{}) } -> std::same_as<void>;
  // 7.4.3 MIDI 2.0 Poly Pressure Message (status=0xA)
  { v.poly_pressure(context, m2cvm::poly_pressure{}) } -> std::same_as<void>;

  // 7.4.4 MIDI 2.0 Registered Per-Note Controller Message (status=0x0)
  { v.rpn_per_note_controller(context, m2cvm::rpn_per_note_controller{}) } -> std::same_as<void>;
  // 7.4.4 MIDI 2.0 Assignable Per-Note Controller Message (status=0x1)
  { v.nrpn_per_note_controller(context, m2cvm::nrpn_per_note_controller{}) } -> std::same_as<void>;
  // 7.4.7 MIDI 2.0 Registered Controller (RPN) Message (status=0x2)
  { v.rpn_controller(context, m2cvm::rpn_controller{}) } -> std::same_as<void>;
  // 7.4.7 MIDI 2.0 Assignable Controller (NRPN) Message (status=0x3)
  { v.nrpn_controller(context, m2cvm::nrpn_controller{}) } -> std::same_as<void>;
  // 7.4.8 MIDI 2.0 Relative Registered Controller (RPN) Message (status=0x4)
  { v.rpn_relative_controller(context, m2cvm::rpn_relative_controller{}) } -> std::same_as<void>;
  // 7.4.8 MIDI 2.0 Relative Assignable Controller (NRPN) Message (status=0x5)
  { v.nrpn_relative_controller(context, m2cvm::nrpn_relative_controller{}) } -> std::same_as<void>;

  // 7.4.9 MIDI 2.0 Program Change Message (status=0xC)
  { v.program_change(context, m2cvm::program_change{}) } -> std::same_as<void>;
  // 7.4.10 MIDI 2.0 Channel Pressure Message (status=0xD)
  { v.channel_pressure(context, m2cvm::channel_pressure{}) } -> std::same_as<void>;

  { v.per_note_management(context, m2cvm::per_note_management{}) } -> std::same_as<void>;
  { v.control_change(context, m2cvm::control_change{}) } -> std::same_as<void>;

  { v.pitch_bend(context, m2cvm::pitch_bend{}) } -> std::same_as<void>;
  { v.per_note_pitch_bend(context, m2cvm::per_note_pitch_bend{}) } -> std::same_as<void>;
};
/// \brief Satisfied by types that can handle Data 128-bit (System Exclusive 8-bit and Mixed Data Set) messages.
///
/// The type must provide the member functions sysex8_in_1(), sysex8_start(), sysex8_continue(), sysex8_end(),
/// mds_header() and mds_payload(). Each takes the context value as its first argument and the message as its second,
/// and returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept data128 = requires(T v, Context context) {
  // 7.8 System Exclusive 8 (8-Bit) Messages
  { v.sysex8_in_1(context, data128::sysex8_in_1{}) } -> std::same_as<void>;
  { v.sysex8_start(context, data128::sysex8_start{}) } -> std::same_as<void>;
  { v.sysex8_continue(context, data128::sysex8_continue{}) } -> std::same_as<void>;
  { v.sysex8_end(context, data128::sysex8_end{}) } -> std::same_as<void>;
  // 7.9 Mixed Data Set Message
  { v.mds_header(context, data128::mds_header{}) } -> std::same_as<void>;
  { v.mds_payload(context, data128::mds_payload{}) } -> std::same_as<void>;
};
/// \brief Satisfied by types that can handle UMP Stream messages.
///
/// The type must provide the member functions endpoint_discovery(), endpoint_info_notification(),
/// device_identity_notification(), endpoint_name_notification(), product_instance_id_notification(),
/// jr_configuration_request(), jr_configuration_notification(), function_block_discovery(),
/// function_block_info_notification(), function_block_name_notification(), start_of_clip() and end_of_clip(). Each
/// takes the context value as its first argument and the message as its second, and returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept stream = requires(T v, Context context) {
  { v.endpoint_discovery(context, stream::endpoint_discovery{}) } -> std::same_as<void>;
  { v.endpoint_info_notification(context, stream::endpoint_info_notification{}) } -> std::same_as<void>;
  { v.device_identity_notification(context, stream::device_identity_notification{}) } -> std::same_as<void>;
  { v.endpoint_name_notification(context, stream::endpoint_name_notification{}) } -> std::same_as<void>;
  { v.product_instance_id_notification(context, stream::product_instance_id_notification{}) } -> std::same_as<void>;
  { v.jr_configuration_request(context, stream::jr_configuration_request{}) } -> std::same_as<void>;
  { v.jr_configuration_notification(context, stream::jr_configuration_notification{}) } -> std::same_as<void>;
  { v.function_block_discovery(context, stream::function_block_discovery{}) } -> std::same_as<void>;
  { v.function_block_info_notification(context, stream::function_block_info_notification{}) } -> std::same_as<void>;
  { v.function_block_name_notification(context, stream::function_block_name_notification{}) } -> std::same_as<void>;
  { v.start_of_clip(context, stream::start_of_clip{}) } -> std::same_as<void>;
  { v.end_of_clip(context, stream::end_of_clip{}) } -> std::same_as<void>;
};
/// \brief Satisfied by types that can handle Flex Data messages.
///
/// The type must provide the member functions set_tempo(), set_time_signature(), set_metronome(), set_key_signature(),
/// set_chord_name() and text(). Each takes the context value as its first argument and the message as its second, and
/// returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept flex_data = requires(T v, Context context) {
  { v.set_tempo(context, flex_data::set_tempo{}) } -> std::same_as<void>;
  { v.set_time_signature(context, flex_data::set_time_signature{}) } -> std::same_as<void>;
  { v.set_metronome(context, flex_data::set_metronome{}) } -> std::same_as<void>;
  { v.set_key_signature(context, flex_data::set_key_signature{}) } -> std::same_as<void>;
  { v.set_chord_name(context, flex_data::set_chord_name{}) } -> std::same_as<void>;
  { v.text(context, flex_data::text_common{}) } -> std::same_as<void>;
};
// clang-format on

// clang-format off
/// \brief A \ref utility backend whose handlers ignore every Utility message.
///
/// The handlers are static constexpr functions which do nothing, so the dispatcher can eliminate the calls
/// entirely. This is the default backend for these messages.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct utility_null {
  /// \brief Ignores a NOOP message (M2-104 section 7.2.1).
  constexpr static void noop(Context) noexcept { /* do nothing */ }
  /// \brief Ignores a JR Clock message (M2-104 section 7.2.2.1).
  constexpr static void jr_clock(Context, utility::jr_clock const &) noexcept { /* do nothing */ }
  /// \brief Ignores a JR Timestamp message (M2-104 section 7.2.2.2).
  constexpr static void jr_timestamp(Context, utility::jr_timestamp const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message (M2-104 section 7.2.3.1).
  constexpr static void delta_clockstamp_tpqn(Context, utility::delta_clockstamp_tpqn const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Delta Clockstamp (DC) message (M2-104 section 7.2.3.2).
  constexpr static void delta_clockstamp(Context, utility::delta_clockstamp const &) noexcept { /* do nothing */ }

  /// \brief Ignores a message whose message type or status is reserved or not recognized.
  constexpr static void unknown(Context, std::span<std::uint32_t>) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(utility<utility_null<int>, int>, "utility_null must implement the utility concept");

// clang-format off
/// \brief A \ref system backend whose handlers ignore every System Common and System Real Time message.
///
/// The handlers are static constexpr functions which do nothing, so the dispatcher can eliminate the calls
/// entirely. This is the default backend for these messages.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct system_null {
  /// \brief Ignores a MIDI Time Code message (M2-104 section 7.6).
  constexpr static void midi_time_code(Context, system::midi_time_code const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Song Position Pointer message (M2-104 section 7.6).
  constexpr static void song_position_pointer(Context, system::song_position_pointer const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Song Select message (M2-104 section 7.6).
  constexpr static void song_select(Context, system::song_select const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Tune Request message (M2-104 section 7.6).
  constexpr static void tune_request(Context, system::tune_request const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Timing Clock message (M2-104 section 7.6).
  constexpr static void timing_clock(Context, system::timing_clock const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Start message (M2-104 section 7.6).
  constexpr static void seq_start(Context, system::sequence_start const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Continue message (M2-104 section 7.6).
  constexpr static void seq_continue(Context, system::sequence_continue const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Stop message (M2-104 section 7.6).
  constexpr static void seq_stop(Context, system::sequence_stop const &) noexcept { /* do nothing */ }
  /// \brief Ignores an Active Sensing message (M2-104 section 7.6).
  constexpr static void active_sensing(Context, system::active_sensing const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Reset message (M2-104 section 7.6).
  constexpr static void reset(Context, system::reset const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(system<system_null<int>, int>, "system_null must implement the system concept");

// clang-format off
/// \brief A \ref m1cvm backend whose handlers ignore every MIDI 1.0 Channel Voice message.
///
/// The handlers are static constexpr functions which do nothing, so the dispatcher can eliminate the calls
/// entirely. This is the default backend for these messages.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct m1cvm_null {
  /// \brief Ignores a MIDI 1.0 Note Off message (M2-104 section 7.3.1).
  constexpr static void note_off(Context, m1cvm::note_off const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Note On message (M2-104 section 7.3.2).
  constexpr static void note_on(Context, m1cvm::note_on const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Poly Pressure message (M2-104 section 7.3.3).
  constexpr static void poly_pressure(Context, m1cvm::poly_pressure const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Control Change message (M2-104 section 7.3.4).
  constexpr static void control_change(Context, m1cvm::control_change const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Program Change message (M2-104 section 7.3.5).
  constexpr static void program_change(Context, m1cvm::program_change const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Channel Pressure message (M2-104 section 7.3.6).
  constexpr static void channel_pressure(Context, m1cvm::channel_pressure const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Pitch Bend message (M2-104 section 7.3.7).
  constexpr static void pitch_bend(Context, m1cvm::pitch_bend const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(m1cvm<m1cvm_null<int>, int>, "m1cvm_null must implement the m1cvm concept");

// clang-format off
/// \brief A \ref data64 backend whose handlers ignore every Data 64-bit (System Exclusive 7-bit) message.
///
/// The handlers are static constexpr functions which do nothing, so the dispatcher can eliminate the calls
/// entirely. This is the default backend for these messages.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct data64_null {
  /// \brief Ignores a System Exclusive (7-bit) Complete in One UMP message (M2-104 section 7.7).
  constexpr static void sysex7_in_1(Context, data64::sysex7_in_1 const &) noexcept { /* do nothing */ }
  /// \brief Ignores a System Exclusive (7-bit) Start message (M2-104 section 7.7).
  constexpr static void sysex7_start(Context, data64::sysex7_start const &) noexcept { /* do nothing */ }
  /// \brief Ignores a System Exclusive (7-bit) Continue message (M2-104 section 7.7).
  constexpr static void sysex7_continue(Context, data64::sysex7_continue const &) noexcept { /* do nothing */ }
  /// \brief Ignores a System Exclusive (7-bit) End message (M2-104 section 7.7).
  constexpr static void sysex7_end(Context, data64::sysex7_end const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(data64<data64_null<int>, int>, "data64_null must implement the data64 concept");

// clang-format off
/// \brief A \ref m2cvm backend whose handlers ignore every MIDI 2.0 Channel Voice message.
///
/// The handlers are static constexpr functions which do nothing, so the dispatcher can eliminate the calls
/// entirely. This is the default backend for these messages.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct m2cvm_null {
  /// \brief Ignores a MIDI 2.0 Note Off message (M2-104 section 7.4.1).
  constexpr static void note_off(Context, m2cvm::note_off const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Note On message (M2-104 section 7.4.2).
  constexpr static void note_on(Context, m2cvm::note_on const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Poly Pressure message (M2-104 section 7.4.3).
  constexpr static void poly_pressure(Context, m2cvm::poly_pressure const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Program Change message (M2-104 section 7.4.9).
  constexpr static void program_change(Context, m2cvm::program_change const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Channel Pressure message (M2-104 section 7.4.10).
  constexpr static void channel_pressure(Context, m2cvm::channel_pressure const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Registered Per-Note Controller message (M2-104 section 7.4.4).
  constexpr static void rpn_per_note_controller(Context, m2cvm::rpn_per_note_controller const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Assignable Per-Note Controller message (M2-104 section 7.4.4).
  constexpr static void nrpn_per_note_controller(Context, m2cvm::nrpn_per_note_controller const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Registered Controller (RPN) message (M2-104 section 7.4.7).
  constexpr static void rpn_controller(Context, m2cvm::rpn_controller const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Assignable Controller (NRPN) message (M2-104 section 7.4.7).
  constexpr static void nrpn_controller(Context, m2cvm::nrpn_controller const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Relative Registered Controller (RPN) message (M2-104 section 7.4.8).
  constexpr static void rpn_relative_controller(Context, m2cvm::rpn_relative_controller const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Relative Assignable Controller (NRPN) message (M2-104 section 7.4.8).
  constexpr static void nrpn_relative_controller(Context, m2cvm::nrpn_relative_controller const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Per-Note Management message (M2-104 section 7.4.5).
  constexpr static void per_note_management(Context, m2cvm::per_note_management const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Control Change message (M2-104 section 7.4.6).
  constexpr static void control_change(Context, m2cvm::control_change const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Pitch Bend message (M2-104 section 7.4.11).
  constexpr static void pitch_bend(Context, m2cvm::pitch_bend const &) noexcept { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Per-Note Pitch Bend message (M2-104 section 7.4.12).
  constexpr static void per_note_pitch_bend(Context, m2cvm::per_note_pitch_bend const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(m2cvm<m2cvm_null<int>, int>, "m2cvm_null must implement the m2cvm concept");

// clang-format off
/// \brief A \ref data128 backend whose handlers ignore every Data 128-bit (System Exclusive 8-bit and Mixed Data Set)
/// message.
///
/// The handlers are static constexpr functions which do nothing, so the dispatcher can eliminate the calls
/// entirely. This is the default backend for these messages.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct data128_null {
  /// \brief Ignores a System Exclusive 8 (8-bit) Complete in One UMP message (M2-104 section 7.8).
  constexpr static void sysex8_in_1(Context, data128::sysex8_in_1 const &) noexcept { /* do nothing */ }
  /// \brief Ignores a System Exclusive 8 (8-bit) Start message (M2-104 section 7.8).
  constexpr static void sysex8_start(Context, data128::sysex8_start const &) noexcept { /* do nothing */ }
  /// \brief Ignores a System Exclusive 8 (8-bit) Continue message (M2-104 section 7.8).
  constexpr static void sysex8_continue(Context, data128::sysex8_continue const &) noexcept { /* do nothing */ }
  /// \brief Ignores a System Exclusive 8 (8-bit) End message (M2-104 section 7.8).
  constexpr static void sysex8_end(Context, data128::sysex8_end const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Mixed Data Set Header message (M2-104 section 7.9).
  constexpr static void mds_header(Context, data128::mds_header const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Mixed Data Set Payload message (M2-104 section 7.9).
  constexpr static void mds_payload(Context, data128::mds_payload const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(data128<data128_null<int>, int>, "data128_null must implement the data128 concept");

// clang-format off
/// \brief A \ref stream backend whose handlers ignore every UMP Stream message.
///
/// The handlers are static constexpr functions which do nothing, so the dispatcher can eliminate the calls
/// entirely. This is the default backend for these messages.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct stream_null {
  /// \brief Ignores an Endpoint Discovery message (M2-104 section 7.1.1).
  constexpr static void endpoint_discovery(Context, stream::endpoint_discovery const &) noexcept { /* do nothing */ }
  /// \brief Ignores an Endpoint Info Notification message (M2-104 section 7.1.2).
  constexpr static void endpoint_info_notification(Context, stream::endpoint_info_notification const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Device Identity Notification message (M2-104 section 7.1.3).
  constexpr static void device_identity_notification(Context, stream::device_identity_notification const &) noexcept { /* do nothing */ }
  /// \brief Ignores an Endpoint Name Notification message (M2-104 section 7.1.4).
  constexpr static void endpoint_name_notification(Context, stream::endpoint_name_notification const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Product Instance Id Notification message (M2-104 section 7.1.5).
  constexpr static void product_instance_id_notification(Context, stream::product_instance_id_notification const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Stream Configuration Request message (M2-104 section 7.1.6.2).
  constexpr static void jr_configuration_request(Context, stream::jr_configuration_request const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Stream Configuration Notification message (M2-104 section 7.1.6.3).
  constexpr static void jr_configuration_notification(Context, stream::jr_configuration_notification const &) noexcept { /* do nothing */ }

  /// \brief Ignores a Function Block Discovery message (M2-104 section 7.1.7).
  constexpr static void function_block_discovery(Context, stream::function_block_discovery const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Function Block Info Notification message (M2-104 section 7.1.8).
  constexpr static void function_block_info_notification(Context, stream::function_block_info_notification const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Function Block Name Notification message (M2-104 section 7.1.9).
  constexpr static void function_block_name_notification(Context, stream::function_block_name_notification const &) noexcept { /* do nothing */ }

  /// \brief Ignores a Start of Clip message (M2-104 section 7.1.10).
  constexpr static void start_of_clip(Context, stream::start_of_clip const &) noexcept { /* do nothing */ }
  /// \brief Ignores an End of Clip message (M2-104 section 7.1.11).
  constexpr static void end_of_clip(Context, stream::end_of_clip const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(stream<stream_null<int>, int>, "stream_null must implement the stream concept");

// clang-format off
/// \brief A \ref flex_data backend whose handlers ignore every Flex Data message.
///
/// The handlers are static constexpr functions which do nothing, so the dispatcher can eliminate the calls
/// entirely. This is the default backend for these messages.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct flex_data_null {
  /// \brief Ignores a Set Tempo message (M2-104 section 7.5.3).
  constexpr static void set_tempo(Context, flex_data::set_tempo const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Set Time Signature message (M2-104 section 7.5.4).
  constexpr static void set_time_signature(Context, flex_data::set_time_signature const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Set Metronome message (M2-104 section 7.5.5).
  constexpr static void set_metronome(Context, flex_data::set_metronome const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Set Key Signature message (M2-104 section 7.5.7).
  constexpr static void set_key_signature(Context, flex_data::set_key_signature const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Set Chord Name message (M2-104 section 7.5.8).
  constexpr static void set_chord_name(Context, flex_data::set_chord_name const &) noexcept { /* do nothing */ }
  /// \brief Ignores a Flex Data text (Text Messages Common Format) message (M2-104 section 7.5.9).
  constexpr static void text(Context, flex_data::text_common const &) noexcept { /* do nothing */ }
};
// clang-format on

static_assert(flex_data<flex_data_null<int>, int>, "flex_data_null must implement the flex_data concept");

/// \brief An abstract base class for Utility message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// utility_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct utility_pure {
  /// \brief Default constructor.
  constexpr utility_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr utility_pure(utility_pure const&) = default;
  /// \brief Move constructor.
  constexpr utility_pure(utility_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~utility_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr utility_pure& operator=(utility_pure const&) = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr utility_pure& operator=(utility_pure&&) noexcept = default;

  /// \brief Called for each NOOP message (M2-104 section 7.2.1).
  ///
  /// The argument is the context value.
  virtual void noop(Context) = 0;
  /// \brief Called for each JR Clock message (M2-104 section 7.2.2.1).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void jr_clock(Context, utility::jr_clock const&) = 0;
  /// \brief Called for each JR Timestamp message (M2-104 section 7.2.2.2).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void jr_timestamp(Context, utility::jr_timestamp const&) = 0;
  /// \brief Called for each Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message (M2-104 section 7.2.3.1).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void delta_clockstamp_tpqn(Context, utility::delta_clockstamp_tpqn const&) = 0;
  /// \brief Called for each Delta Clockstamp (DC) message (M2-104 section 7.2.3.2).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void delta_clockstamp(Context, utility::delta_clockstamp const&) = 0;

  /// \brief Called for a message whose message type or status is reserved or not recognized.
  ///
  /// The first argument is the context value; the second holds the 32-bit words of the message.
  virtual void unknown(Context, std::span<std::uint32_t>) = 0;
};

static_assert(utility<utility_pure<int>, int>, "utility_pure must implement the utility concept");

/// \brief An abstract base class for System Common and System Real Time message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// system_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct system_pure {
  /// \brief Default constructor.
  constexpr system_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr system_pure(system_pure const&) = default;
  /// \brief Move constructor.
  constexpr system_pure(system_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~system_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr system_pure& operator=(system_pure const&) = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr system_pure& operator=(system_pure&&) noexcept = default;

  /// \brief Called for each MIDI Time Code message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void midi_time_code(Context, system::midi_time_code const&) = 0;
  /// \brief Called for each Song Position Pointer message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void song_position_pointer(Context, system::song_position_pointer const&) = 0;
  /// \brief Called for each Song Select message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void song_select(Context, system::song_select const&) = 0;
  /// \brief Called for each Tune Request message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void tune_request(Context, system::tune_request const&) = 0;
  /// \brief Called for each Timing Clock message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void timing_clock(Context, system::timing_clock const&) = 0;
  /// \brief Called for each Start message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void seq_start(Context, system::sequence_start const&) = 0;
  /// \brief Called for each Continue message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void seq_continue(Context, system::sequence_continue const&) = 0;
  /// \brief Called for each Stop message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void seq_stop(Context, system::sequence_stop const&) = 0;
  /// \brief Called for each Active Sensing message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void active_sensing(Context, system::active_sensing const&) = 0;
  /// \brief Called for each Reset message (M2-104 section 7.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void reset(Context, system::reset const&) = 0;
};

static_assert(system<system_pure<int>, int>, "system_pure must implement the system concept");

/// \brief An abstract base class for MIDI 1.0 Channel Voice message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// m1cvm_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct m1cvm_pure {
  /// \brief Default constructor.
  constexpr m1cvm_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr m1cvm_pure(m1cvm_pure const&) = default;
  /// \brief Move constructor.
  constexpr m1cvm_pure(m1cvm_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~m1cvm_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr m1cvm_pure& operator=(m1cvm_pure const&) = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr m1cvm_pure& operator=(m1cvm_pure&&) noexcept = default;

  /// \brief Called for each MIDI 1.0 Note Off message (M2-104 section 7.3.1).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void note_off(Context, m1cvm::note_off const&) = 0;
  /// \brief Called for each MIDI 1.0 Note On message (M2-104 section 7.3.2).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void note_on(Context, m1cvm::note_on const&) = 0;
  /// \brief Called for each MIDI 1.0 Poly Pressure message (M2-104 section 7.3.3).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void poly_pressure(Context, m1cvm::poly_pressure const&) = 0;
  /// \brief Called for each MIDI 1.0 Control Change message (M2-104 section 7.3.4).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void control_change(Context, m1cvm::control_change const&) = 0;
  /// \brief Called for each MIDI 1.0 Program Change message (M2-104 section 7.3.5).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void program_change(Context, m1cvm::program_change const&) = 0;
  /// \brief Called for each MIDI 1.0 Channel Pressure message (M2-104 section 7.3.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void channel_pressure(Context, m1cvm::channel_pressure const&) = 0;
  /// \brief Called for each MIDI 1.0 Pitch Bend message (M2-104 section 7.3.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void pitch_bend(Context, m1cvm::pitch_bend const&) = 0;
};

static_assert(m1cvm<m1cvm_pure<int>, int>, "m1cvm_pure must implement the m1cvm concept");

/// \brief An abstract base class for Data 64-bit (System Exclusive 7-bit) message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// data64_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct data64_pure {
  /// \brief Default constructor.
  constexpr data64_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr data64_pure(data64_pure const&) noexcept = default;
  /// \brief Move constructor.
  constexpr data64_pure(data64_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~data64_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr data64_pure& operator=(data64_pure const&) noexcept = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr data64_pure& operator=(data64_pure&&) noexcept = default;

  /// \brief Called for each System Exclusive (7-bit) Complete in One UMP message (M2-104 section 7.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void sysex7_in_1(Context, data64::sysex7_in_1 const&) = 0;
  /// \brief Called for each System Exclusive (7-bit) Start message (M2-104 section 7.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void sysex7_start(Context, data64::sysex7_start const&) = 0;
  /// \brief Called for each System Exclusive (7-bit) Continue message (M2-104 section 7.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void sysex7_continue(Context, data64::sysex7_continue const&) = 0;
  /// \brief Called for each System Exclusive (7-bit) End message (M2-104 section 7.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void sysex7_end(Context, data64::sysex7_end const&) = 0;
};

static_assert(data64<data64_pure<int>, int>, "data64_pure must implement the data64 concept");

/// \brief An abstract base class for MIDI 2.0 Channel Voice message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// m2cvm_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct m2cvm_pure {
  /// \brief Default constructor.
  constexpr m2cvm_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr m2cvm_pure(m2cvm_pure const&) noexcept = default;
  /// \brief Move constructor.
  constexpr m2cvm_pure(m2cvm_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~m2cvm_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr m2cvm_pure& operator=(m2cvm_pure const&) noexcept = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr m2cvm_pure& operator=(m2cvm_pure&&) noexcept = default;

  /// \brief Called for each MIDI 2.0 Note Off message (M2-104 section 7.4.1).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void note_off(Context, m2cvm::note_off const&) = 0;
  /// \brief Called for each MIDI 2.0 Note On message (M2-104 section 7.4.2).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void note_on(Context, m2cvm::note_on const&) = 0;
  /// \brief Called for each MIDI 2.0 Poly Pressure message (M2-104 section 7.4.3).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void poly_pressure(Context, m2cvm::poly_pressure const&) = 0;
  /// \brief Called for each MIDI 2.0 Program Change message (M2-104 section 7.4.9).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void program_change(Context, m2cvm::program_change const&) = 0;
  /// \brief Called for each MIDI 2.0 Channel Pressure message (M2-104 section 7.4.10).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void channel_pressure(Context, m2cvm::channel_pressure const&) = 0;

  /// \brief Called for each MIDI 2.0 Registered Per-Note Controller message (M2-104 section 7.4.4).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void rpn_per_note_controller(Context, m2cvm::rpn_per_note_controller const&) = 0;
  /// \brief Called for each MIDI 2.0 Assignable Per-Note Controller message (M2-104 section 7.4.4).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void nrpn_per_note_controller(Context, m2cvm::nrpn_per_note_controller const&) = 0;
  /// \brief Called for each MIDI 2.0 Registered Controller (RPN) message (M2-104 section 7.4.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void rpn_controller(Context, m2cvm::rpn_controller const&) = 0;
  /// \brief Called for each MIDI 2.0 Assignable Controller (NRPN) message (M2-104 section 7.4.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void nrpn_controller(Context, m2cvm::nrpn_controller const&) = 0;
  /// \brief Called for each MIDI 2.0 Relative Registered Controller (RPN) message (M2-104 section 7.4.8).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void rpn_relative_controller(Context, m2cvm::rpn_relative_controller const&) = 0;
  /// \brief Called for each MIDI 2.0 Relative Assignable Controller (NRPN) message (M2-104 section 7.4.8).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void nrpn_relative_controller(Context, m2cvm::nrpn_relative_controller const&) = 0;

  /// \brief Called for each MIDI 2.0 Per-Note Management message (M2-104 section 7.4.5).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void per_note_management(Context, m2cvm::per_note_management const&) = 0;
  /// \brief Called for each MIDI 2.0 Control Change message (M2-104 section 7.4.6).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void control_change(Context, m2cvm::control_change const&) = 0;
  /// \brief Called for each MIDI 2.0 Pitch Bend message (M2-104 section 7.4.11).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void pitch_bend(Context, m2cvm::pitch_bend const&) = 0;
  /// \brief Called for each MIDI 2.0 Per-Note Pitch Bend message (M2-104 section 7.4.12).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void per_note_pitch_bend(Context, m2cvm::per_note_pitch_bend const&) = 0;
};

static_assert(m2cvm<m2cvm_pure<int>, int>, "m2cvm_pure must implement the m2cvm concept");

/// \brief An abstract base class for Data 128-bit (System Exclusive 8-bit and Mixed Data Set) message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// data128_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct data128_pure {
  /// \brief Default constructor.
  constexpr data128_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr data128_pure(data128_pure const&) noexcept = default;
  /// \brief Move constructor.
  constexpr data128_pure(data128_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~data128_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr data128_pure& operator=(data128_pure const&) noexcept = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr data128_pure& operator=(data128_pure&&) noexcept = default;

  /// \brief Called for each System Exclusive 8 (8-bit) Complete in One UMP message (M2-104 section 7.8).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void sysex8_in_1(Context, data128::sysex8_in_1 const&) = 0;
  /// \brief Called for each System Exclusive 8 (8-bit) Start message (M2-104 section 7.8).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void sysex8_start(Context, data128::sysex8_start const&) = 0;
  /// \brief Called for each System Exclusive 8 (8-bit) Continue message (M2-104 section 7.8).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void sysex8_continue(Context, data128::sysex8_continue const&) = 0;
  /// \brief Called for each System Exclusive 8 (8-bit) End message (M2-104 section 7.8).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void sysex8_end(Context, data128::sysex8_end const&) = 0;
  /// \brief Called for each Mixed Data Set Header message (M2-104 section 7.9).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void mds_header(Context, data128::mds_header const&) = 0;
  /// \brief Called for each Mixed Data Set Payload message (M2-104 section 7.9).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void mds_payload(Context, data128::mds_payload const&) = 0;
};

static_assert(data128<data128_pure<int>, int>, "data128_pure must implement the data128 concept");

/// \brief An abstract base class for UMP Stream message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// stream_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct stream_pure {
  /// \brief Default constructor.
  constexpr stream_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr stream_pure(stream_pure const&) = default;
  /// \brief Move constructor.
  constexpr stream_pure(stream_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~stream_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr stream_pure& operator=(stream_pure const&) = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr stream_pure& operator=(stream_pure&&) noexcept = default;

  /// \brief Called for each Endpoint Discovery message (M2-104 section 7.1.1).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void endpoint_discovery(Context, stream::endpoint_discovery const&) = 0;
  /// \brief Called for each Endpoint Info Notification message (M2-104 section 7.1.2).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void endpoint_info_notification(Context, stream::endpoint_info_notification const&) = 0;
  /// \brief Called for each Device Identity Notification message (M2-104 section 7.1.3).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void device_identity_notification(Context, stream::device_identity_notification const&) = 0;
  /// \brief Called for each Endpoint Name Notification message (M2-104 section 7.1.4).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void endpoint_name_notification(Context, stream::endpoint_name_notification const&) = 0;
  /// \brief Called for each Product Instance Id Notification message (M2-104 section 7.1.5).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void product_instance_id_notification(Context, stream::product_instance_id_notification const&) = 0;
  /// \brief Called for each Stream Configuration Request message (M2-104 section 7.1.6.2).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void jr_configuration_request(Context, stream::jr_configuration_request const&) = 0;
  /// \brief Called for each Stream Configuration Notification message (M2-104 section 7.1.6.3).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void jr_configuration_notification(Context, stream::jr_configuration_notification const&) = 0;

  /// \brief Called for each Function Block Discovery message (M2-104 section 7.1.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void function_block_discovery(Context, stream::function_block_discovery const&) = 0;
  /// \brief Called for each Function Block Info Notification message (M2-104 section 7.1.8).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void function_block_info_notification(Context, stream::function_block_info_notification const&) = 0;
  /// \brief Called for each Function Block Name Notification message (M2-104 section 7.1.9).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void function_block_name_notification(Context, stream::function_block_name_notification const&) = 0;

  /// \brief Called for each Start of Clip message (M2-104 section 7.1.10).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void start_of_clip(Context, stream::start_of_clip const&) = 0;
  /// \brief Called for each End of Clip message (M2-104 section 7.1.11).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void end_of_clip(Context, stream::end_of_clip const&) = 0;
};

static_assert(stream<stream_pure<int>, int>, "stream_pure must implement the stream concept");

/// \brief An abstract base class for Flex Data message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// flex_data_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct flex_data_pure {
  /// \brief Default constructor.
  constexpr flex_data_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr flex_data_pure(flex_data_pure const&) = default;
  /// \brief Move constructor.
  constexpr flex_data_pure(flex_data_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~flex_data_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr flex_data_pure& operator=(flex_data_pure const&) = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr flex_data_pure& operator=(flex_data_pure&&) noexcept = default;

  /// \brief Called for each Set Tempo message (M2-104 section 7.5.3).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void set_tempo(Context, flex_data::set_tempo const&) = 0;
  /// \brief Called for each Set Time Signature message (M2-104 section 7.5.4).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void set_time_signature(Context, flex_data::set_time_signature const&) = 0;
  /// \brief Called for each Set Metronome message (M2-104 section 7.5.5).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void set_metronome(Context, flex_data::set_metronome const&) = 0;
  /// \brief Called for each Set Key Signature message (M2-104 section 7.5.7).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void set_key_signature(Context, flex_data::set_key_signature const&) = 0;
  /// \brief Called for each Set Chord Name message (M2-104 section 7.5.8).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void set_chord_name(Context, flex_data::set_chord_name const&) = 0;
  /// \brief Called for each Flex Data text (Text Messages Common Format) message (M2-104 section 7.5.9).
  ///
  /// The first argument is the context value; the second is the message.
  virtual void text(Context, flex_data::text_common const&) = 0;
};

static_assert(flex_data<flex_data_pure<int>, int>, "flex_data_pure must implement the flex_data concept");

// clang-format off
/// \brief A base class which implements each Utility message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct utility_base : utility_pure<Context> {
  /// \brief Ignores a NOOP message (M2-104 section 7.2.1).
  void noop(Context) override { /* do nothing */ }
  /// \brief Ignores a JR Clock message (M2-104 section 7.2.2.1).
  void jr_clock(Context, utility::jr_clock const &) override { /* do nothing */ }
  /// \brief Ignores a JR Timestamp message (M2-104 section 7.2.2.2).
  void jr_timestamp(Context, utility::jr_timestamp const &) override { /* do nothing */ }
  /// \brief Ignores a Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message (M2-104 section 7.2.3.1).
  void delta_clockstamp_tpqn(Context, utility::delta_clockstamp_tpqn const &) override { /* do nothing */ }
  /// \brief Ignores a Delta Clockstamp (DC) message (M2-104 section 7.2.3.2).
  void delta_clockstamp(Context, utility::delta_clockstamp const &) override { /* do nothing */ }

  /// \brief Ignores a message whose message type or status is reserved or not recognized.
  void unknown(Context, std::span<std::uint32_t>) override { /* do nothing */ }
};
/// \brief A base class which implements each System Common and System Real Time message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct system_base : system_pure<Context> {
  /// \brief Ignores a MIDI Time Code message (M2-104 section 7.6).
  void midi_time_code(Context, system::midi_time_code const &) override { /* do nothing */ }
  /// \brief Ignores a Song Position Pointer message (M2-104 section 7.6).
  void song_position_pointer(Context, system::song_position_pointer const &) override { /* do nothing */ }
  /// \brief Ignores a Song Select message (M2-104 section 7.6).
  void song_select(Context, system::song_select const &) override { /* do nothing */ }
  /// \brief Ignores a Tune Request message (M2-104 section 7.6).
  void tune_request(Context, system::tune_request const &) override { /* do nothing */ }
  /// \brief Ignores a Timing Clock message (M2-104 section 7.6).
  void timing_clock(Context, system::timing_clock const &) override { /* do nothing */ }
  /// \brief Ignores a Start message (M2-104 section 7.6).
  void seq_start(Context, system::sequence_start const &) override { /* do nothing */ }
  /// \brief Ignores a Continue message (M2-104 section 7.6).
  void seq_continue(Context, system::sequence_continue const &) override { /* do nothing */ }
  /// \brief Ignores a Stop message (M2-104 section 7.6).
  void seq_stop(Context, system::sequence_stop const &) override { /* do nothing */ }
  /// \brief Ignores an Active Sensing message (M2-104 section 7.6).
  void active_sensing(Context, system::active_sensing const &) override { /* do nothing */ }
  /// \brief Ignores a Reset message (M2-104 section 7.6).
  void reset(Context, system::reset const &) override { /* do nothing */ }
};
/// \brief A base class which implements each MIDI 1.0 Channel Voice message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct m1cvm_base : m1cvm_pure<Context> {
  /// \brief Ignores a MIDI 1.0 Note Off message (M2-104 section 7.3.1).
  void note_off(Context, m1cvm::note_off const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Note On message (M2-104 section 7.3.2).
  void note_on(Context, m1cvm::note_on const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Poly Pressure message (M2-104 section 7.3.3).
  void poly_pressure(Context, m1cvm::poly_pressure const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Control Change message (M2-104 section 7.3.4).
  void control_change(Context, m1cvm::control_change const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Program Change message (M2-104 section 7.3.5).
  void program_change(Context, m1cvm::program_change const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Channel Pressure message (M2-104 section 7.3.6).
  void channel_pressure(Context, m1cvm::channel_pressure const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 1.0 Pitch Bend message (M2-104 section 7.3.7).
  void pitch_bend(Context, m1cvm::pitch_bend const &) override { /* do nothing */ }
};
/// \brief A base class which implements each Data 64-bit (System Exclusive 7-bit) message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct data64_base : data64_pure<Context> {
  /// \brief Ignores a System Exclusive (7-bit) Complete in One UMP message (M2-104 section 7.7).
  void sysex7_in_1(Context, data64::sysex7_in_1 const &) override { /* do nothing */ }
  /// \brief Ignores a System Exclusive (7-bit) Start message (M2-104 section 7.7).
  void sysex7_start(Context, data64::sysex7_start const &) override { /* do nothing */ }
  /// \brief Ignores a System Exclusive (7-bit) Continue message (M2-104 section 7.7).
  void sysex7_continue(Context, data64::sysex7_continue const &) override { /* do nothing */ }
  /// \brief Ignores a System Exclusive (7-bit) End message (M2-104 section 7.7).
  void sysex7_end(Context, data64::sysex7_end const &) override { /* do nothing */ }
};
/// \brief A base class which implements each MIDI 2.0 Channel Voice message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct m2cvm_base : m2cvm_pure<Context> {
  /// \brief Ignores a MIDI 2.0 Note Off message (M2-104 section 7.4.1).
  void note_off(Context, m2cvm::note_off const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Note On message (M2-104 section 7.4.2).
  void note_on(Context, m2cvm::note_on const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Poly Pressure message (M2-104 section 7.4.3).
  void poly_pressure(Context, m2cvm::poly_pressure const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Program Change message (M2-104 section 7.4.9).
  void program_change(Context, m2cvm::program_change const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Channel Pressure message (M2-104 section 7.4.10).
  void channel_pressure(Context, m2cvm::channel_pressure const &) override { /* do nothing */ }

  /// \brief Ignores a MIDI 2.0 Registered Per-Note Controller message (M2-104 section 7.4.4).
  void rpn_per_note_controller(Context, m2cvm::rpn_per_note_controller const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Assignable Per-Note Controller message (M2-104 section 7.4.4).
  void nrpn_per_note_controller(Context, m2cvm::nrpn_per_note_controller const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Registered Controller (RPN) message (M2-104 section 7.4.7).
  void rpn_controller(Context, m2cvm::rpn_controller const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Assignable Controller (NRPN) message (M2-104 section 7.4.7).
  void nrpn_controller(Context, m2cvm::nrpn_controller const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Relative Registered Controller (RPN) message (M2-104 section 7.4.8).
  void rpn_relative_controller(Context, m2cvm::rpn_relative_controller const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Relative Assignable Controller (NRPN) message (M2-104 section 7.4.8).
  void nrpn_relative_controller(Context, m2cvm::nrpn_relative_controller const &) override { /* do nothing */ }

  /// \brief Ignores a MIDI 2.0 Per-Note Management message (M2-104 section 7.4.5).
  void per_note_management(Context, m2cvm::per_note_management const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Control Change message (M2-104 section 7.4.6).
  void control_change(Context, m2cvm::control_change const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Pitch Bend message (M2-104 section 7.4.11).
  void pitch_bend(Context, m2cvm::pitch_bend const &) override { /* do nothing */ }
  /// \brief Ignores a MIDI 2.0 Per-Note Pitch Bend message (M2-104 section 7.4.12).
  void per_note_pitch_bend(Context, m2cvm::per_note_pitch_bend const &) override { /* do nothing */ }
};
/// \brief A base class which implements each Data 128-bit (System Exclusive 8-bit and Mixed Data Set) message handler
/// by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct data128_base : data128_pure<Context> {
  /// \brief Ignores a System Exclusive 8 (8-bit) Complete in One UMP message (M2-104 section 7.8).
  void sysex8_in_1(Context, data128::sysex8_in_1 const &) override { /* do nothing */ }
  /// \brief Ignores a System Exclusive 8 (8-bit) Start message (M2-104 section 7.8).
  void sysex8_start(Context, data128::sysex8_start const &) override { /* do nothing */ }
  /// \brief Ignores a System Exclusive 8 (8-bit) Continue message (M2-104 section 7.8).
  void sysex8_continue(Context, data128::sysex8_continue const &) override { /* do nothing */ }
  /// \brief Ignores a System Exclusive 8 (8-bit) End message (M2-104 section 7.8).
  void sysex8_end(Context, data128::sysex8_end const &) override { /* do nothing */ }
  /// \brief Ignores a Mixed Data Set Header message (M2-104 section 7.9).
  void mds_header(Context, data128::mds_header const &) override { /* do nothing */ }
  /// \brief Ignores a Mixed Data Set Payload message (M2-104 section 7.9).
  void mds_payload(Context, data128::mds_payload const &) override { /* do nothing */ }
};
/// \brief A base class which implements each UMP Stream message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct stream_base : stream_pure<Context> {
  /// \brief Ignores an Endpoint Discovery message (M2-104 section 7.1.1).
  void endpoint_discovery(Context, stream::endpoint_discovery const &) override { /* do nothing */ }
  /// \brief Ignores an Endpoint Info Notification message (M2-104 section 7.1.2).
  void endpoint_info_notification(Context, stream::endpoint_info_notification const &) override { /* do nothing */ }
  /// \brief Ignores a Device Identity Notification message (M2-104 section 7.1.3).
  void device_identity_notification(Context, stream::device_identity_notification const &) override { /* do nothing */ }
  /// \brief Ignores an Endpoint Name Notification message (M2-104 section 7.1.4).
  void endpoint_name_notification(Context, stream::endpoint_name_notification const &) override { /* do nothing */ }
  /// \brief Ignores a Product Instance Id Notification message (M2-104 section 7.1.5).
  void product_instance_id_notification(Context, stream::product_instance_id_notification const &) override { /* do nothing */ }
  /// \brief Ignores a Stream Configuration Request message (M2-104 section 7.1.6.2).
  void jr_configuration_request(Context, stream::jr_configuration_request const &) override { /* do nothing */ }
  /// \brief Ignores a Stream Configuration Notification message (M2-104 section 7.1.6.3).
  void jr_configuration_notification(Context, stream::jr_configuration_notification const &) override { /* do nothing */ }

  /// \brief Ignores a Function Block Discovery message (M2-104 section 7.1.7).
  void function_block_discovery(Context, stream::function_block_discovery const &) override { /* do nothing */ }
  /// \brief Ignores a Function Block Info Notification message (M2-104 section 7.1.8).
  void function_block_info_notification(Context, stream::function_block_info_notification const &) override { /* do nothing */ }
  /// \brief Ignores a Function Block Name Notification message (M2-104 section 7.1.9).
  void function_block_name_notification(Context, stream::function_block_name_notification const &) override { /* do nothing */ }

  /// \brief Ignores a Start of Clip message (M2-104 section 7.1.10).
  void start_of_clip(Context, stream::start_of_clip const &) override { /* do nothing */ }
  /// \brief Ignores an End of Clip message (M2-104 section 7.1.11).
  void end_of_clip(Context, stream::end_of_clip const &) override { /* do nothing */ }
};
/// \brief A base class which implements each Flex Data message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct flex_data_base : flex_data_pure<Context> {
  /// \brief Ignores a Set Tempo message (M2-104 section 7.5.3).
  void set_tempo(Context, flex_data::set_tempo const &) override { /* do nothing */ }
  /// \brief Ignores a Set Time Signature message (M2-104 section 7.5.4).
  void set_time_signature(Context, flex_data::set_time_signature const &) override { /* do nothing */ }
  /// \brief Ignores a Set Metronome message (M2-104 section 7.5.5).
  void set_metronome(Context, flex_data::set_metronome const &) override { /* do nothing */ }
  /// \brief Ignores a Set Key Signature message (M2-104 section 7.5.7).
  void set_key_signature(Context, flex_data::set_key_signature const &) override { /* do nothing */ }
  /// \brief Ignores a Set Chord Name message (M2-104 section 7.5.8).
  void set_chord_name(Context, flex_data::set_chord_name const &) override { /* do nothing */ }
  /// \brief Ignores a Flex Data text (Text Messages Common Format) message (M2-104 section 7.5.9).
  void text(Context, flex_data::text_common const &) override { /* do nothing */ }
};
// clang-format on

/// \brief A \ref utility backend which forwards each Utility message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class utility_function {
public:
  /// \brief The type of the function called for each NOOP message (M2-104 section 7.2.1).
  using noop_fn = std::function<void(Context)>;
  /// \brief The type of the function called for each JR Clock message (M2-104 section 7.2.2.1).
  using jr_clock_fn = std::function<void(Context, utility::jr_clock const&)>;
  /// \brief The type of the function called for each JR Timestamp message (M2-104 section 7.2.2.2).
  using jr_timestamp_fn = std::function<void(Context, utility::jr_timestamp const&)>;
  /// \brief The type of the function called for each Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message (M2-104
  /// section 7.2.3.1).
  using delta_clockstamp_tpqn_fn = std::function<void(Context, utility::delta_clockstamp_tpqn const&)>;
  /// \brief The type of the function called for each Delta Clockstamp (DC) message (M2-104 section 7.2.3.2).
  using delta_clockstamp_fn = std::function<void(Context, utility::delta_clockstamp const&)>;
  /// \brief The type of the function called for a message whose message type or status is reserved or not recognized.
  using unknown_fn = std::function<void(Context, std::span<std::uint32_t>)>;

  // clang-format off
  /// \brief Sets the function to be called for each NOOP message (M2-104 section 7.2.1).
  /// \param noop  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr utility_function &on_noop(noop_fn noop) noexcept { noop_ = std::move(noop); return *this; }
  /// \brief Sets the function to be called for each JR Clock message (M2-104 section 7.2.2.1).
  /// \param jr_clock  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr utility_function &on_jr_clock(jr_clock_fn jr_clock) noexcept { jr_clock_ = std::move(jr_clock); return *this; }
  /// \brief Sets the function to be called for each JR Timestamp message (M2-104 section 7.2.2.2).
  /// \param jr_timestamp  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr utility_function &on_jr_timestamp(jr_timestamp_fn jr_timestamp) noexcept { jr_timestamp_ = std::move(jr_timestamp); return *this; }
  /// \brief Sets the function to be called for each Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message (M2-104
  /// section 7.2.3.1).
  /// \param delta_clockstamp  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr utility_function &on_delta_clockstamp_tpqn(delta_clockstamp_tpqn_fn delta_clockstamp) noexcept { delta_clockstamp_tpqn_ = std::move(delta_clockstamp); return *this; }
  /// \brief Sets the function to be called for each Delta Clockstamp (DC) message (M2-104 section 7.2.3.2).
  /// \param delta_clockstamp  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr utility_function &on_delta_clockstamp(delta_clockstamp_fn delta_clockstamp) noexcept { delta_clockstamp_ = std::move(delta_clockstamp); return *this; }
  /// \brief Sets the function to be called for a message whose message type or status is reserved or not recognized.
  /// \param unknown  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr utility_function &on_unknown(unknown_fn unknown) noexcept { unknown_ = std::move(unknown); return *this; }

  /// \brief Calls the function registered by on_noop(), if any.
  /// \param c  The context value to be passed to the function.
  void noop(Context c) const { call(noop_, c); }
  /// \brief Calls the function registered by on_jr_clock(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param clock  The JR Clock message (M2-104 section 7.2.2.1).
  void jr_clock(Context c, utility::jr_clock const &clock) const { call(jr_clock_, c, clock); }
  /// \brief Calls the function registered by on_jr_timestamp(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param ts  The JR Timestamp message (M2-104 section 7.2.2.2).
  void jr_timestamp(Context c, utility::jr_timestamp const &ts) const { call(jr_timestamp_, c, ts); }
  /// \brief Calls the function registered by on_delta_clockstamp_tpqn(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param time  The Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message (M2-104 section 7.2.3.1).
  void delta_clockstamp_tpqn(Context c, utility::delta_clockstamp_tpqn const &time) const { call(delta_clockstamp_tpqn_, c, time); }
  /// \brief Calls the function registered by on_delta_clockstamp(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param time  The Delta Clockstamp (DC) message (M2-104 section 7.2.3.2).
  void delta_clockstamp(Context c, utility::delta_clockstamp const &time) const { call(delta_clockstamp_, c, time); }
  /// \brief Calls the function registered by on_unknown(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param data  The 32-bit words of the message.
  void unknown(Context c, std::span<std::uint32_t> data) const { call(unknown_, c, data); }
  // clang-format on

private:
  noop_fn noop_;
  jr_clock_fn jr_clock_;
  jr_timestamp_fn jr_timestamp_;
  delta_clockstamp_tpqn_fn delta_clockstamp_tpqn_;
  delta_clockstamp_fn delta_clockstamp_;
  unknown_fn unknown_;
};

static_assert(utility<utility_function<int>, int>, "utility_function must implement the utility concept");

/// \brief A \ref system backend which forwards each System Common and System Real Time message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class system_function {
public:
  /// \brief The type of the function called for each MIDI Time Code message (M2-104 section 7.6).
  using midi_time_code_fn = std::function<void(Context, system::midi_time_code const&)>;
  /// \brief The type of the function called for each Song Position Pointer message (M2-104 section 7.6).
  using song_position_pointer_fn = std::function<void(Context, system::song_position_pointer const&)>;
  /// \brief The type of the function called for each Song Select message (M2-104 section 7.6).
  using song_select_fn = std::function<void(Context, system::song_select const&)>;
  /// \brief The type of the function called for each Tune Request message (M2-104 section 7.6).
  using tune_request_fn = std::function<void(Context, system::tune_request const&)>;
  /// \brief The type of the function called for each Timing Clock message (M2-104 section 7.6).
  using timing_clock_fn = std::function<void(Context, system::timing_clock const&)>;
  /// \brief The type of the function called for each Start message (M2-104 section 7.6).
  using seq_start_fn = std::function<void(Context, system::sequence_start const&)>;
  /// \brief The type of the function called for each Continue message (M2-104 section 7.6).
  using seq_continue_fn = std::function<void(Context, system::sequence_continue const&)>;
  /// \brief The type of the function called for each Stop message (M2-104 section 7.6).
  using seq_stop_fn = std::function<void(Context, system::sequence_stop const&)>;
  /// \brief The type of the function called for each Active Sensing message (M2-104 section 7.6).
  using active_sensing_fn = std::function<void(Context, system::active_sensing const&)>;
  /// \brief The type of the function called for each Reset message (M2-104 section 7.6).
  using reset_fn = std::function<void(Context, system::reset const&)>;

  // clang-format off
  /// \brief Sets the function to be called for each MIDI Time Code message (M2-104 section 7.6).
  /// \param midi_time_code  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_midi_time_code(midi_time_code_fn midi_time_code) { midi_time_code_ = std::move(midi_time_code); return *this; }
  /// \brief Sets the function to be called for each Song Position Pointer message (M2-104 section 7.6).
  /// \param spp  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_song_position_pointer(song_position_pointer_fn spp) { song_position_pointer_ = std::move(spp); return *this; }
  /// \brief Sets the function to be called for each Song Select message (M2-104 section 7.6).
  /// \param song_select  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_song_select(song_select_fn song_select) { song_select_ = std::move(song_select); return *this; }
  /// \brief Sets the function to be called for each Tune Request message (M2-104 section 7.6).
  /// \param tune_request  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_tune_request(tune_request_fn tune_request) { tune_request_ = std::move(tune_request); return *this; }
  /// \brief Sets the function to be called for each Timing Clock message (M2-104 section 7.6).
  /// \param timing_clock  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_timing_clock(timing_clock_fn timing_clock) { timing_clock_ = std::move(timing_clock); return *this; }
  /// \brief Sets the function to be called for each Start message (M2-104 section 7.6).
  /// \param seq_start  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_seq_start(seq_start_fn seq_start) { seq_start_ = std::move(seq_start); return *this; }
  /// \brief Sets the function to be called for each Continue message (M2-104 section 7.6).
  /// \param seq_continue  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_seq_continue(seq_continue_fn seq_continue) { seq_continue_ = std::move(seq_continue); return *this; }
  /// \brief Sets the function to be called for each Stop message (M2-104 section 7.6).
  /// \param seq_stop  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_seq_stop(seq_stop_fn seq_stop) { seq_stop_ = std::move(seq_stop); return *this; }
  /// \brief Sets the function to be called for each Active Sensing message (M2-104 section 7.6).
  /// \param active_sensing  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_active_sensing(active_sensing_fn active_sensing) { active_sensing_ = std::move(active_sensing); return *this; }
  /// \brief Sets the function to be called for each Reset message (M2-104 section 7.6).
  /// \param reset  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr system_function &on_reset(reset_fn reset) { reset_ = std::move(reset); return *this; }

  /// \brief Calls the function registered by on_midi_time_code(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param mtc  The MIDI Time Code message (M2-104 section 7.6).
  void midi_time_code(Context c, system::midi_time_code const &mtc) const { call(midi_time_code_, c, mtc); }
  /// \brief Calls the function registered by on_song_position_pointer(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param spp  The Song Position Pointer message (M2-104 section 7.6).
  void song_position_pointer(Context c, system::song_position_pointer const &spp) const { call(song_position_pointer_, c, spp); }
  /// \brief Calls the function registered by on_song_select(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param song  The Song Select message (M2-104 section 7.6).
  void song_select(Context c, system::song_select const &song) const { call(song_select_, c, song); }
  /// \brief Calls the function registered by on_tune_request(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param request  The Tune Request message (M2-104 section 7.6).
  void tune_request(Context c, system::tune_request const &request) const { call(tune_request_, c, request); }
  /// \brief Calls the function registered by on_timing_clock(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param clock  The Timing Clock message (M2-104 section 7.6).
  void timing_clock(Context c, system::timing_clock const &clock) const { call(timing_clock_, c, clock); }
  /// \brief Calls the function registered by on_seq_start(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param ss  The Start message (M2-104 section 7.6).
  void seq_start(Context c, system::sequence_start const &ss) const { call(seq_start_, c, ss); }
  /// \brief Calls the function registered by on_seq_continue(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sc  The Continue message (M2-104 section 7.6).
  void seq_continue(Context c, system::sequence_continue const &sc) const { call(seq_continue_, c, sc); }
  /// \brief Calls the function registered by on_seq_stop(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param ss  The Stop message (M2-104 section 7.6).
  void seq_stop(Context c, system::sequence_stop const &ss) const { call(seq_stop_, c, ss); }
  /// \brief Calls the function registered by on_active_sensing(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param as  The Active Sensing message (M2-104 section 7.6).
  void active_sensing(Context c, system::active_sensing const &as) const { call(active_sensing_, c, as); }
  /// \brief Calls the function registered by on_reset(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param r  The Reset message (M2-104 section 7.6).
  void reset(Context c, system::reset const &r) const { call(reset_, c, r); }
  // clang-format on

private:
  midi_time_code_fn midi_time_code_;
  song_position_pointer_fn song_position_pointer_;
  song_select_fn song_select_;
  tune_request_fn tune_request_;
  timing_clock_fn timing_clock_;
  seq_start_fn seq_start_;
  seq_continue_fn seq_continue_;
  seq_stop_fn seq_stop_;
  active_sensing_fn active_sensing_;
  reset_fn reset_;
};

static_assert(system<system_function<int>, int>, "system_function must implement the system concept");

/// \brief A \ref m1cvm backend which forwards each MIDI 1.0 Channel Voice message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class m1cvm_function {
public:
  /// \brief The type of the function called for each MIDI 1.0 Note Off message (M2-104 section 7.3.1).
  using note_off_fn = std::function<void(Context, m1cvm::note_off const&)>;
  /// \brief The type of the function called for each MIDI 1.0 Note On message (M2-104 section 7.3.2).
  using note_on_fn = std::function<void(Context, m1cvm::note_on const&)>;
  /// \brief The type of the function called for each MIDI 1.0 Poly Pressure message (M2-104 section 7.3.3).
  using poly_pressure_fn = std::function<void(Context, m1cvm::poly_pressure const&)>;
  /// \brief The type of the function called for each MIDI 1.0 Control Change message (M2-104 section 7.3.4).
  using control_change_fn = std::function<void(Context, m1cvm::control_change const&)>;
  /// \brief The type of the function called for each MIDI 1.0 Program Change message (M2-104 section 7.3.5).
  using program_change_fn = std::function<void(Context, m1cvm::program_change const&)>;
  /// \brief The type of the function called for each MIDI 1.0 Channel Pressure message (M2-104 section 7.3.6).
  using channel_pressure_fn = std::function<void(Context, m1cvm::channel_pressure const&)>;
  /// \brief The type of the function called for each MIDI 1.0 Pitch Bend message (M2-104 section 7.3.7).
  using pitch_bend_fn = std::function<void(Context, m1cvm::pitch_bend const&)>;

  // clang-format off
  /// \brief Sets the function to be called for each MIDI 1.0 Note Off message (M2-104 section 7.3.1).
  /// \param note_off  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m1cvm_function &on_note_off(note_off_fn note_off) noexcept { note_off_ = std::move(note_off); return *this; }
  /// \brief Sets the function to be called for each MIDI 1.0 Note On message (M2-104 section 7.3.2).
  /// \param note_on  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m1cvm_function &on_note_on(note_on_fn note_on) noexcept { note_on_ = std::move(note_on); return *this; }
  /// \brief Sets the function to be called for each MIDI 1.0 Poly Pressure message (M2-104 section 7.3.3).
  /// \param poly_pressure  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m1cvm_function &on_poly_pressure(poly_pressure_fn poly_pressure) noexcept { poly_pressure_ = std::move(poly_pressure); return *this; }
  /// \brief Sets the function to be called for each MIDI 1.0 Control Change message (M2-104 section 7.3.4).
  /// \param control_change  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m1cvm_function &on_control_change(control_change_fn control_change) noexcept { control_change_ = std::move(control_change); return *this; }
  /// \brief Sets the function to be called for each MIDI 1.0 Program Change message (M2-104 section 7.3.5).
  /// \param program_change  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m1cvm_function &on_program_change(program_change_fn program_change) noexcept { program_change_ = std::move(program_change); return *this; }
  /// \brief Sets the function to be called for each MIDI 1.0 Channel Pressure message (M2-104 section 7.3.6).
  /// \param channel_pressure  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m1cvm_function &on_channel_pressure(channel_pressure_fn channel_pressure) noexcept { channel_pressure_ = std::move(channel_pressure); return *this; }
  /// \brief Sets the function to be called for each MIDI 1.0 Pitch Bend message (M2-104 section 7.3.7).
  /// \param pitch_bend  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m1cvm_function &on_pitch_bend(pitch_bend_fn pitch_bend) noexcept { pitch_bend_ = std::move(pitch_bend); return *this; }

  /// \brief Calls the function registered by on_note_off(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param off  The MIDI 1.0 Note Off message (M2-104 section 7.3.1).
  void note_off(Context c, m1cvm::note_off const &off) const { call(note_off_, c, off); }
  /// \brief Calls the function registered by on_note_on(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param on  The MIDI 1.0 Note On message (M2-104 section 7.3.2).
  void note_on(Context c, m1cvm::note_on const &on) const { call(note_on_, c, on); }
  /// \brief Calls the function registered by on_poly_pressure(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param pressure  The MIDI 1.0 Poly Pressure message (M2-104 section 7.3.3).
  void poly_pressure(Context c, m1cvm::poly_pressure const &pressure) const { call(poly_pressure_, c, pressure); }
  /// \brief Calls the function registered by on_control_change(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param cc  The MIDI 1.0 Control Change message (M2-104 section 7.3.4).
  void control_change(Context c, m1cvm::control_change const &cc) const { call(control_change_, c, cc); }
  /// \brief Calls the function registered by on_program_change(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param program  The MIDI 1.0 Program Change message (M2-104 section 7.3.5).
  void program_change(Context c, m1cvm::program_change const &program) const { call(program_change_, c, program); }
  /// \brief Calls the function registered by on_channel_pressure(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param pressure  The MIDI 1.0 Channel Pressure message (M2-104 section 7.3.6).
  void channel_pressure(Context c, m1cvm::channel_pressure const &pressure) const { call(channel_pressure_, c, pressure); }
  /// \brief Calls the function registered by on_pitch_bend(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param bend  The MIDI 1.0 Pitch Bend message (M2-104 section 7.3.7).
  void pitch_bend(Context c, m1cvm::pitch_bend const &bend) const { call(pitch_bend_, c, bend); }
  // clang-format on

private:
  note_off_fn note_off_;
  note_on_fn note_on_;
  poly_pressure_fn poly_pressure_;
  control_change_fn control_change_;
  program_change_fn program_change_;
  channel_pressure_fn channel_pressure_;
  pitch_bend_fn pitch_bend_;
};

static_assert(m1cvm<m1cvm_function<int>, int>, "m1cvm_function must implement the m1cvm concept");

/// \brief A \ref data64 backend which forwards each Data 64-bit (System Exclusive 7-bit) message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class data64_function {
public:
  /// \brief The type of the function called for each System Exclusive (7-bit) Complete in One UMP message (M2-104
  /// section 7.7).
  using sysex7_in_1_fn = std::function<void(Context, data64::sysex7_in_1 const&)>;
  /// \brief The type of the function called for each System Exclusive (7-bit) Start message (M2-104 section 7.7).
  using sysex7_start_fn = std::function<void(Context, data64::sysex7_start const&)>;
  /// \brief The type of the function called for each System Exclusive (7-bit) Continue message (M2-104 section 7.7).
  using sysex7_continue_fn = std::function<void(Context, data64::sysex7_continue const&)>;
  /// \brief The type of the function called for each System Exclusive (7-bit) End message (M2-104 section 7.7).
  using sysex7_end_fn = std::function<void(Context, data64::sysex7_end const&)>;

  // clang-format off
  /// \brief Sets the function to be called for each System Exclusive (7-bit) Complete in One UMP message (M2-104
  /// section 7.7).
  /// \param sysex7_in_1  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data64_function &on_sysex7_in_1(sysex7_in_1_fn sysex7_in_1) noexcept { sysex7_in_1_ = std::move(sysex7_in_1); return *this; }
  /// \brief Sets the function to be called for each System Exclusive (7-bit) Start message (M2-104 section 7.7).
  /// \param sysex7_start  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data64_function &on_sysex7_start(sysex7_start_fn sysex7_start) noexcept { sysex7_start_ = std::move(sysex7_start); return *this; }
  /// \brief Sets the function to be called for each System Exclusive (7-bit) Continue message (M2-104 section 7.7).
  /// \param sysex7_continue  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data64_function &on_sysex7_continue(sysex7_continue_fn sysex7_continue) noexcept { sysex7_continue_ = std::move(sysex7_continue); return *this; }
  /// \brief Sets the function to be called for each System Exclusive (7-bit) End message (M2-104 section 7.7).
  /// \param sysex7_end  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data64_function &on_sysex7_end(sysex7_end_fn sysex7_end) noexcept { sysex7_end_ = std::move(sysex7_end); return *this; }

  /// \brief Calls the function registered by on_sysex7_in_1(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sx  The System Exclusive (7-bit) Complete in One UMP message (M2-104 section 7.7).
  void sysex7_in_1(Context c, data64::sysex7_in_1 const &sx) const { call(sysex7_in_1_, c, sx); }
  /// \brief Calls the function registered by on_sysex7_start(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sx  The System Exclusive (7-bit) Start message (M2-104 section 7.7).
  void sysex7_start(Context c, data64::sysex7_start const &sx) const { call(sysex7_start_, c, sx); }
  /// \brief Calls the function registered by on_sysex7_continue(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sx  The System Exclusive (7-bit) Continue message (M2-104 section 7.7).
  void sysex7_continue(Context c, data64::sysex7_continue const &sx) const { call(sysex7_continue_, c, sx); }
  /// \brief Calls the function registered by on_sysex7_end(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sx  The System Exclusive (7-bit) End message (M2-104 section 7.7).
  void sysex7_end(Context c, data64::sysex7_end const &sx) const { call(sysex7_end_, c, sx); }
  // clang-format on

private:
  sysex7_in_1_fn sysex7_in_1_;
  sysex7_start_fn sysex7_start_;
  sysex7_continue_fn sysex7_continue_;
  sysex7_end_fn sysex7_end_;
};

static_assert(data64<data64_function<int>, int>, "data64_function must implement the data64 concept");

/// \brief A \ref m2cvm backend which forwards each MIDI 2.0 Channel Voice message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class m2cvm_function {
public:
  /// \brief The type of the function called for each MIDI 2.0 Note Off message (M2-104 section 7.4.1).
  using note_off_fn = std::function<void(Context, m2cvm::note_off const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Note On message (M2-104 section 7.4.2).
  using note_on_fn = std::function<void(Context, m2cvm::note_on const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Poly Pressure message (M2-104 section 7.4.3).
  using poly_pressure_fn = std::function<void(Context, m2cvm::poly_pressure const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Program Change message (M2-104 section 7.4.9).
  using program_change_fn = std::function<void(Context, m2cvm::program_change const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Channel Pressure message (M2-104 section 7.4.10).
  using channel_pressure_fn = std::function<void(Context, m2cvm::channel_pressure const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Registered Per-Note Controller message (M2-104 section
  /// 7.4.4).
  using rpn_per_note_controller_fn = std::function<void(Context, m2cvm::rpn_per_note_controller const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Assignable Per-Note Controller message (M2-104 section
  /// 7.4.4).
  using nrpn_per_note_controller_fn = std::function<void(Context, m2cvm::nrpn_per_note_controller const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Registered Controller (RPN) message (M2-104 section
  /// 7.4.7).
  using rpn_controller_fn = std::function<void(Context, m2cvm::rpn_controller const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Assignable Controller (NRPN) message (M2-104 section
  /// 7.4.7).
  using nrpn_controller_fn = std::function<void(Context, m2cvm::nrpn_controller const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Relative Registered Controller (RPN) message (M2-104
  /// section 7.4.8).
  using rpn_relative_controller_fn = std::function<void(Context, m2cvm::rpn_relative_controller const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Relative Assignable Controller (NRPN) message (M2-104
  /// section 7.4.8).
  using nrpn_relative_controller_fn = std::function<void(Context, m2cvm::nrpn_relative_controller const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Per-Note Management message (M2-104 section 7.4.5).
  using per_note_management_fn = std::function<void(Context, m2cvm::per_note_management const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Control Change message (M2-104 section 7.4.6).
  using control_change_fn = std::function<void(Context, m2cvm::control_change const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Pitch Bend message (M2-104 section 7.4.11).
  using pitch_bend_fn = std::function<void(Context, m2cvm::pitch_bend const&)>;
  /// \brief The type of the function called for each MIDI 2.0 Per-Note Pitch Bend message (M2-104 section 7.4.12).
  using per_note_pitch_bend_fn = std::function<void(Context, m2cvm::per_note_pitch_bend const&)>;

  // clang-format off
  /// \brief Sets the function to be called for each MIDI 2.0 Note Off message (M2-104 section 7.4.1).
  /// \param off  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_note_off(note_off_fn off) noexcept { note_off_ = std::move(off); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Note On message (M2-104 section 7.4.2).
  /// \param on  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_note_on(note_on_fn on) noexcept { note_on_ = std::move(on); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Poly Pressure message (M2-104 section 7.4.3).
  /// \param pressure  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_poly_pressure(poly_pressure_fn pressure) noexcept { poly_pressure_ = std::move(pressure); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Program Change message (M2-104 section 7.4.9).
  /// \param program_change  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_program_change(program_change_fn program_change) noexcept { program_change_ = std::move(program_change); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Channel Pressure message (M2-104 section 7.4.10).
  /// \param pressure  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_channel_pressure(channel_pressure_fn pressure) noexcept { channel_pressure_ = std::move(pressure); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Registered Per-Note Controller message (M2-104 section
  /// 7.4.4).
  /// \param controller  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_rpn_per_note_controller(rpn_per_note_controller_fn controller) noexcept { rpn_per_note_controller_ = std::move(controller); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Assignable Per-Note Controller message (M2-104 section
  /// 7.4.4).
  /// \param controller  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_nrpn_per_note_controller(nrpn_per_note_controller_fn controller) noexcept { nrpn_per_note_controller_ = std::move(controller); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Registered Controller (RPN) message (M2-104 section
  /// 7.4.7).
  /// \param controller  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_rpn_controller(rpn_controller_fn controller) noexcept { rpn_controller_ = std::move(controller); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Assignable Controller (NRPN) message (M2-104 section
  /// 7.4.7).
  /// \param controller  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_nrpn_controller(nrpn_controller_fn controller) noexcept { nrpn_controller_ = std::move(controller); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Relative Registered Controller (RPN) message (M2-104
  /// section 7.4.8).
  /// \param controller  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_rpn_relative_controller(rpn_relative_controller_fn controller) noexcept { rpn_relative_controller_ = std::move(controller); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Relative Assignable Controller (NRPN) message (M2-104
  /// section 7.4.8).
  /// \param controller  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_nrpn_relative_controller(nrpn_relative_controller_fn controller) noexcept { nrpn_relative_controller_ = std::move(controller); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Per-Note Management message (M2-104 section 7.4.5).
  /// \param per_note_management  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_per_note_management(per_note_management_fn per_note_management) noexcept { per_note_management_ = std::move(per_note_management); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Control Change message (M2-104 section 7.4.6).
  /// \param cc  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_control_change(control_change_fn cc) noexcept { control_change_ = std::move(cc); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Pitch Bend message (M2-104 section 7.4.11).
  /// \param pitch_bend  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_pitch_bend(pitch_bend_fn pitch_bend) noexcept { pitch_bend_ = std::move(pitch_bend); return *this; }
  /// \brief Sets the function to be called for each MIDI 2.0 Per-Note Pitch Bend message (M2-104 section 7.4.12).
  /// \param per_note_pitch_bend  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr m2cvm_function &on_per_note_pitch_bend(per_note_pitch_bend_fn per_note_pitch_bend) noexcept { per_note_pitch_bend_ = std::move(per_note_pitch_bend); return *this; }

  /// \brief Calls the function registered by on_note_off(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param off  The MIDI 2.0 Note Off message (M2-104 section 7.4.1).
  void note_off(Context c, m2cvm::note_off const &off) const { call(note_off_, c, off); }
  /// \brief Calls the function registered by on_note_on(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param on  The MIDI 2.0 Note On message (M2-104 section 7.4.2).
  void note_on(Context c, m2cvm::note_on const &on) const { call(note_on_, c, on); }
  /// \brief Calls the function registered by on_poly_pressure(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param pressure  The MIDI 2.0 Poly Pressure message (M2-104 section 7.4.3).
  void poly_pressure(Context c, m2cvm::poly_pressure const &pressure) const { call(poly_pressure_, c, pressure); }
  /// \brief Calls the function registered by on_program_change(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param program  The MIDI 2.0 Program Change message (M2-104 section 7.4.9).
  void program_change(Context c, m2cvm::program_change const &program) const { call(program_change_, c, program); }
  /// \brief Calls the function registered by on_channel_pressure(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param pressure  The MIDI 2.0 Channel Pressure message (M2-104 section 7.4.10).
  void channel_pressure(Context c, m2cvm::channel_pressure const &pressure) const { call(channel_pressure_, c, pressure); }
  /// \brief Calls the function registered by on_rpn_per_note_controller(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param rpn  The MIDI 2.0 Registered Per-Note Controller message (M2-104 section 7.4.4).
  void rpn_per_note_controller(Context c, m2cvm::rpn_per_note_controller const &rpn) const { call(rpn_per_note_controller_, c, rpn); }
  /// \brief Calls the function registered by on_nrpn_per_note_controller(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param nrpn  The MIDI 2.0 Assignable Per-Note Controller message (M2-104 section 7.4.4).
  void nrpn_per_note_controller(Context c, m2cvm::nrpn_per_note_controller const &nrpn) const { call(nrpn_per_note_controller_, c, nrpn); }
  /// \brief Calls the function registered by on_rpn_controller(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param rpn  The MIDI 2.0 Registered Controller (RPN) message (M2-104 section 7.4.7).
  void rpn_controller(Context c, m2cvm::rpn_controller const &rpn) const { call(rpn_controller_, c, rpn); }
  /// \brief Calls the function registered by on_nrpn_controller(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param nrpn  The MIDI 2.0 Assignable Controller (NRPN) message (M2-104 section 7.4.7).
  void nrpn_controller(Context c, m2cvm::nrpn_controller const &nrpn) const { call(nrpn_controller_, c, nrpn); }
  /// \brief Calls the function registered by on_rpn_relative_controller(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param rpn  The MIDI 2.0 Relative Registered Controller (RPN) message (M2-104 section 7.4.8).
  void rpn_relative_controller(Context c, m2cvm::rpn_relative_controller const &rpn) const { call(rpn_relative_controller_, c, rpn); }
  /// \brief Calls the function registered by on_nrpn_relative_controller(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param nrpn  The MIDI 2.0 Relative Assignable Controller (NRPN) message (M2-104 section 7.4.8).
  void nrpn_relative_controller(Context c, m2cvm::nrpn_relative_controller const &nrpn) const { call(nrpn_relative_controller_, c, nrpn); }
  /// \brief Calls the function registered by on_per_note_management(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param pnm  The MIDI 2.0 Per-Note Management message (M2-104 section 7.4.5).
  void per_note_management(Context c, m2cvm::per_note_management const &pnm) const { call(per_note_management_, c, pnm); }
  /// \brief Calls the function registered by on_control_change(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param cc  The MIDI 2.0 Control Change message (M2-104 section 7.4.6).
  void control_change(Context c, m2cvm::control_change const &cc) const { call(control_change_, c, cc); }
  /// \brief Calls the function registered by on_pitch_bend(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param pb  The MIDI 2.0 Pitch Bend message (M2-104 section 7.4.11).
  void pitch_bend(Context c, m2cvm::pitch_bend const &pb) const { call(pitch_bend_, c, pb); }
  /// \brief Calls the function registered by on_per_note_pitch_bend(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param pb  The MIDI 2.0 Per-Note Pitch Bend message (M2-104 section 7.4.12).
  void per_note_pitch_bend(Context c, m2cvm::per_note_pitch_bend const &pb) const { call(per_note_pitch_bend_, c, pb); }
  // clang-format on

private:
  note_off_fn note_off_;
  note_on_fn note_on_;
  poly_pressure_fn poly_pressure_;
  program_change_fn program_change_;
  channel_pressure_fn channel_pressure_;
  rpn_per_note_controller_fn rpn_per_note_controller_;
  nrpn_per_note_controller_fn nrpn_per_note_controller_;
  rpn_controller_fn rpn_controller_;
  nrpn_controller_fn nrpn_controller_;
  rpn_relative_controller_fn rpn_relative_controller_;
  nrpn_relative_controller_fn nrpn_relative_controller_;
  per_note_management_fn per_note_management_;
  control_change_fn control_change_;
  pitch_bend_fn pitch_bend_;
  per_note_pitch_bend_fn per_note_pitch_bend_;
};

static_assert(m2cvm<m2cvm_function<int>, int>, "m2cvm_function must implement the m2cvm concept");

/// \brief A \ref data128 backend which forwards each Data 128-bit (System Exclusive 8-bit and Mixed Data Set) message
/// to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class data128_function {
public:
  /// \brief The type of the function called for each System Exclusive 8 (8-bit) Complete in One UMP message (M2-104
  /// section 7.8).
  using sysex8_in_1_fn = std::function<void(Context, data128::sysex8_in_1 const&)>;
  /// \brief The type of the function called for each System Exclusive 8 (8-bit) Start message (M2-104 section 7.8).
  using sysex8_start_fn = std::function<void(Context, data128::sysex8_start const&)>;
  /// \brief The type of the function called for each System Exclusive 8 (8-bit) Continue message (M2-104 section 7.8).
  using sysex8_continue_fn = std::function<void(Context, data128::sysex8_continue const&)>;
  /// \brief The type of the function called for each System Exclusive 8 (8-bit) End message (M2-104 section 7.8).
  using sysex8_end_fn = std::function<void(Context, data128::sysex8_end const&)>;
  /// \brief The type of the function called for each Mixed Data Set Header message (M2-104 section 7.9).
  using mds_header_fn = std::function<void(Context, data128::mds_header const&)>;
  /// \brief The type of the function called for each Mixed Data Set Payload message (M2-104 section 7.9).
  using mds_payload_fn = std::function<void(Context, data128::mds_payload const&)>;

  // clang-format off
  /// \brief Sets the function to be called for each System Exclusive 8 (8-bit) Complete in One UMP message (M2-104
  /// section 7.8).
  /// \param sysex8_in_1  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data128_function &on_sysex8_in_1(sysex8_in_1_fn sysex8_in_1) noexcept { sysex8_in_1_ = std::move(sysex8_in_1); return *this; }
  /// \brief Sets the function to be called for each System Exclusive 8 (8-bit) Start message (M2-104 section 7.8).
  /// \param sysex8_start  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data128_function &on_sysex8_start(sysex8_start_fn sysex8_start) noexcept { sysex8_start_ = std::move(sysex8_start); return *this; }
  /// \brief Sets the function to be called for each System Exclusive 8 (8-bit) Continue message (M2-104 section 7.8).
  /// \param sysex8_continue  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data128_function &on_sysex8_continue(sysex8_continue_fn sysex8_continue) noexcept { sysex8_continue_ = std::move(sysex8_continue); return *this; }
  /// \brief Sets the function to be called for each System Exclusive 8 (8-bit) End message (M2-104 section 7.8).
  /// \param sysex8_end  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data128_function &on_sysex8_end(sysex8_end_fn sysex8_end) noexcept { sysex8_end_ = std::move(sysex8_end); return *this; }
  /// \brief Sets the function to be called for each Mixed Data Set Header message (M2-104 section 7.9).
  /// \param mds_header  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data128_function &on_mds_header(mds_header_fn mds_header) noexcept { mds_header_ = std::move(mds_header); return *this; }
  /// \brief Sets the function to be called for each Mixed Data Set Payload message (M2-104 section 7.9).
  /// \param mds_payload  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr data128_function &on_mds_payload(mds_payload_fn mds_payload) noexcept { mds_payload_ = std::move(mds_payload); return *this; }

  /// \brief Calls the function registered by on_sysex8_in_1(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sysex  The System Exclusive 8 (8-bit) Complete in One UMP message (M2-104 section 7.8).
  void sysex8_in_1(Context c, data128::sysex8_in_1 const &sysex) const { call(sysex8_in_1_, c, sysex); }
  /// \brief Calls the function registered by on_sysex8_start(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sysex  The System Exclusive 8 (8-bit) Start message (M2-104 section 7.8).
  void sysex8_start(Context c, data128::sysex8_start const &sysex) const { call(sysex8_start_, c, sysex); }
  /// \brief Calls the function registered by on_sysex8_continue(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sysex  The System Exclusive 8 (8-bit) Continue message (M2-104 section 7.8).
  void sysex8_continue(Context c, data128::sysex8_continue const &sysex) const { call(sysex8_continue_, c, sysex); }
  /// \brief Calls the function registered by on_sysex8_end(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param sysex  The System Exclusive 8 (8-bit) End message (M2-104 section 7.8).
  void sysex8_end(Context c, data128::sysex8_end const &sysex) const { call(sysex8_end_, c, sysex); }
  /// \brief Calls the function registered by on_mds_header(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param mds  The Mixed Data Set Header message (M2-104 section 7.9).
  void mds_header(Context c, data128::mds_header const &mds) const { call(mds_header_, c, mds); }
  /// \brief Calls the function registered by on_mds_payload(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param mds  The Mixed Data Set Payload message (M2-104 section 7.9).
  void mds_payload(Context c, data128::mds_payload const &mds) const { call(mds_payload_, c, mds); }
  // clang-format on

private:
  sysex8_in_1_fn sysex8_in_1_;
  sysex8_start_fn sysex8_start_;
  sysex8_continue_fn sysex8_continue_;
  sysex8_end_fn sysex8_end_;
  mds_header_fn mds_header_;
  mds_payload_fn mds_payload_;
};

static_assert(data128<data128_function<int>, int>, "data128_function must implement the data128 concept");

/// \brief A \ref stream backend which forwards each UMP Stream message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class stream_function {
public:
  /// \brief The type of the function called for each Endpoint Discovery message (M2-104 section 7.1.1).
  using endpoint_discovery_fn = std::function<void(Context, stream::endpoint_discovery const&)>;
  /// \brief The type of the function called for each Endpoint Info Notification message (M2-104 section 7.1.2).
  using endpoint_info_notification_fn = std::function<void(Context, stream::endpoint_info_notification const&)>;
  /// \brief The type of the function called for each Device Identity Notification message (M2-104 section 7.1.3).
  using device_identity_notification_fn = std::function<void(Context, stream::device_identity_notification const&)>;
  /// \brief The type of the function called for each Endpoint Name Notification message (M2-104 section 7.1.4).
  using endpoint_name_notification_fn = std::function<void(Context, stream::endpoint_name_notification const&)>;
  /// \brief The type of the function called for each Product Instance Id Notification message (M2-104 section 7.1.5).
  using product_instance_id_notification_fn =
      std::function<void(Context, stream::product_instance_id_notification const&)>;
  /// \brief The type of the function called for each Stream Configuration Request message (M2-104 section 7.1.6.2).
  using jr_configuration_request_fn = std::function<void(Context, stream::jr_configuration_request const&)>;
  /// \brief The type of the function called for each Stream Configuration Notification message (M2-104 section
  /// 7.1.6.3).
  using jr_configuration_notification_fn = std::function<void(Context, stream::jr_configuration_notification const&)>;
  /// \brief The type of the function called for each Function Block Discovery message (M2-104 section 7.1.7).
  using function_block_discovery_fn = std::function<void(Context, stream::function_block_discovery const&)>;
  /// \brief The type of the function called for each Function Block Info Notification message (M2-104 section 7.1.8).
  using function_block_info_notification_fn =
      std::function<void(Context, stream::function_block_info_notification const&)>;
  /// \brief The type of the function called for each Function Block Name Notification message (M2-104 section 7.1.9).
  using function_block_name_notification_fn =
      std::function<void(Context, stream::function_block_name_notification const&)>;
  /// \brief The type of the function called for each Start of Clip message (M2-104 section 7.1.10).
  using start_of_clip_fn = std::function<void(Context, stream::start_of_clip const&)>;
  /// \brief The type of the function called for each End of Clip message (M2-104 section 7.1.11).
  using end_of_clip_fn = std::function<void(Context, stream::end_of_clip const&)>;

  // clang-format off
  /// \brief Sets the function to be called for each Endpoint Discovery message (M2-104 section 7.1.1).
  /// \param discovery  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_endpoint_discovery(endpoint_discovery_fn discovery) noexcept { endpoint_discovery_ = std::move(discovery); return *this; }
  /// \brief Sets the function to be called for each Endpoint Info Notification message (M2-104 section 7.1.2).
  /// \param notification  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_endpoint_info_notification(endpoint_info_notification_fn notification) noexcept { endpoint_info_notification_ = std::move(notification); return *this; }
  /// \brief Sets the function to be called for each Device Identity Notification message (M2-104 section 7.1.3).
  /// \param notification  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_device_identity_notification(device_identity_notification_fn notification) noexcept { device_identity_notification_ = std::move(notification); return *this; }
  /// \brief Sets the function to be called for each Endpoint Name Notification message (M2-104 section 7.1.4).
  /// \param notification  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_endpoint_name_notification(endpoint_name_notification_fn notification) noexcept { endpoint_name_notification_ = std::move(notification); return *this; }
  /// \brief Sets the function to be called for each Product Instance Id Notification message (M2-104 section 7.1.5).
  /// \param notification  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_product_instance_id_notification(product_instance_id_notification_fn notification) noexcept { product_instance_id_notification_ = std::move(notification); return *this; }
  /// \brief Sets the function to be called for each Stream Configuration Request message (M2-104 section 7.1.6.2).
  /// \param request  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_jr_configuration_request(jr_configuration_request_fn request) noexcept { jr_configuration_request_ = std::move(request); return *this; }
  /// \brief Sets the function to be called for each Stream Configuration Notification message (M2-104 section 7.1.6.3).
  /// \param notification  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_jr_configuration_notification(jr_configuration_notification_fn notification) noexcept { jr_configuration_notification_ = std::move(notification); return *this; }
  /// \brief Sets the function to be called for each Function Block Discovery message (M2-104 section 7.1.7).
  /// \param discovery  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_function_block_discovery(function_block_discovery_fn discovery) noexcept { function_block_discovery_ = std::move(discovery); return *this; }
  /// \brief Sets the function to be called for each Function Block Info Notification message (M2-104 section 7.1.8).
  /// \param notification  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_function_block_info_notification(function_block_info_notification_fn notification) noexcept { function_block_info_notification_ = std::move(notification); return *this; }
  /// \brief Sets the function to be called for each Function Block Name Notification message (M2-104 section 7.1.9).
  /// \param notification  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_function_block_name_notification(function_block_name_notification_fn notification) noexcept { function_block_name_notification_ = std::move(notification); return *this; }
  /// \brief Sets the function to be called for each Start of Clip message (M2-104 section 7.1.10).
  /// \param clip  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_start_of_clip(start_of_clip_fn clip) noexcept { start_of_clip_ = std::move(clip); return *this; }
  /// \brief Sets the function to be called for each End of Clip message (M2-104 section 7.1.11).
  /// \param clip  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr stream_function &on_end_of_clip(end_of_clip_fn clip) noexcept { end_of_clip_ = std::move(clip); return *this; }

  /// \brief Calls the function registered by on_endpoint_discovery(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param discovery  The Endpoint Discovery message (M2-104 section 7.1.1).
  void endpoint_discovery(Context c, stream::endpoint_discovery const &discovery) const { call(endpoint_discovery_, c, discovery); }
  /// \brief Calls the function registered by on_endpoint_info_notification(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param notification  The Endpoint Info Notification message (M2-104 section 7.1.2).
  void endpoint_info_notification(Context c, stream::endpoint_info_notification const &notification) const { call(endpoint_info_notification_, c, notification); }
  /// \brief Calls the function registered by on_device_identity_notification(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param notification  The Device Identity Notification message (M2-104 section 7.1.3).
  void device_identity_notification(Context c, stream::device_identity_notification const &notification) const { call(device_identity_notification_, c, notification); }
  /// \brief Calls the function registered by on_endpoint_name_notification(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param notification  The Endpoint Name Notification message (M2-104 section 7.1.4).
  void endpoint_name_notification(Context c, stream::endpoint_name_notification const &notification) const { call(endpoint_name_notification_, c, notification); }
  /// \brief Calls the function registered by on_product_instance_id_notification(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param notification  The Product Instance Id Notification message (M2-104 section 7.1.5).
  void product_instance_id_notification(Context c, stream::product_instance_id_notification const &notification) const { call(product_instance_id_notification_, c, notification); }
  /// \brief Calls the function registered by on_jr_configuration_request(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param request  The Stream Configuration Request message (M2-104 section 7.1.6.2).
  void jr_configuration_request(Context c, stream::jr_configuration_request const &request) const { call(jr_configuration_request_, c, request); }
  /// \brief Calls the function registered by on_jr_configuration_notification(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param notification  The Stream Configuration Notification message (M2-104 section 7.1.6.3).
  void jr_configuration_notification(Context c, stream::jr_configuration_notification const &notification) const { call(jr_configuration_notification_, c, notification); }
  /// \brief Calls the function registered by on_function_block_discovery(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param discovery  The Function Block Discovery message (M2-104 section 7.1.7).
  void function_block_discovery(Context c, stream::function_block_discovery const &discovery) const { call(function_block_discovery_, c, discovery); }
  /// \brief Calls the function registered by on_function_block_info_notification(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param notification  The Function Block Info Notification message (M2-104 section 7.1.8).
  void function_block_info_notification(Context c, stream::function_block_info_notification const &notification) const { call(function_block_info_notification_, c, notification); }
  /// \brief Calls the function registered by on_function_block_name_notification(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param notification  The Function Block Name Notification message (M2-104 section 7.1.9).
  void function_block_name_notification(Context c, stream::function_block_name_notification const &notification) const { call(function_block_name_notification_, c, notification); }
  /// \brief Calls the function registered by on_start_of_clip(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param clip  The Start of Clip message (M2-104 section 7.1.10).
  void start_of_clip(Context c, stream::start_of_clip const &clip) const { call(start_of_clip_, c, clip); }
  /// \brief Calls the function registered by on_end_of_clip(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param clip  The End of Clip message (M2-104 section 7.1.11).
  void end_of_clip(Context c, stream::end_of_clip const &clip) const { call(end_of_clip_, c, clip); }
  // clang-format on

private:
  endpoint_discovery_fn endpoint_discovery_;
  endpoint_info_notification_fn endpoint_info_notification_;
  device_identity_notification_fn device_identity_notification_;
  endpoint_name_notification_fn endpoint_name_notification_;
  product_instance_id_notification_fn product_instance_id_notification_;
  jr_configuration_request_fn jr_configuration_request_;
  jr_configuration_notification_fn jr_configuration_notification_;

  function_block_discovery_fn function_block_discovery_;
  function_block_info_notification_fn function_block_info_notification_;
  function_block_name_notification_fn function_block_name_notification_;

  start_of_clip_fn start_of_clip_;
  end_of_clip_fn end_of_clip_;
};

static_assert(stream<stream_function<int>, int>, "stream_function must implement the stream concept");

/// \brief A \ref flex_data backend which forwards each Flex Data message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class flex_data_function {
public:
  /// \brief The type of the function called for each Set Tempo message (M2-104 section 7.5.3).
  using set_tempo_fn = std::function<void(Context, flex_data::set_tempo const&)>;
  /// \brief The type of the function called for each Set Time Signature message (M2-104 section 7.5.4).
  using set_time_signature_fn = std::function<void(Context, flex_data::set_time_signature const&)>;
  /// \brief The type of the function called for each Set Metronome message (M2-104 section 7.5.5).
  using set_metronome_fn = std::function<void(Context, flex_data::set_metronome const&)>;
  /// \brief The type of the function called for each Set Key Signature message (M2-104 section 7.5.7).
  using set_key_signature_fn = std::function<void(Context, flex_data::set_key_signature const&)>;
  /// \brief The type of the function called for each Set Chord Name message (M2-104 section 7.5.8).
  using set_chord_name_fn = std::function<void(Context, flex_data::set_chord_name const&)>;
  /// \brief The type of the function called for each Flex Data text (Text Messages Common Format) message (M2-104
  /// section 7.5.9).
  using text_fn = std::function<void(Context, flex_data::text_common const&)>;

  // clang-format off
  /// \brief Sets the function to be called for each Set Tempo message (M2-104 section 7.5.3).
  /// \param set_tempo  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr flex_data_function &on_set_tempo(set_tempo_fn set_tempo) noexcept { set_tempo_ = std::move(set_tempo); return *this; }
  /// \brief Sets the function to be called for each Set Time Signature message (M2-104 section 7.5.4).
  /// \param set_time_signature  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr flex_data_function &on_set_time_signature(set_time_signature_fn set_time_signature) noexcept { set_time_signature_ = std::move(set_time_signature); return *this; }
  /// \brief Sets the function to be called for each Set Metronome message (M2-104 section 7.5.5).
  /// \param set_metronome  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr flex_data_function &on_set_metronome(set_metronome_fn set_metronome) noexcept { set_metronome_ = std::move(set_metronome); return *this; }
  /// \brief Sets the function to be called for each Set Key Signature message (M2-104 section 7.5.7).
  /// \param set_key_signature  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr flex_data_function &on_set_key_signature(set_key_signature_fn set_key_signature) noexcept { set_key_signature_ = std::move(set_key_signature); return *this; }
  /// \brief Sets the function to be called for each Set Chord Name message (M2-104 section 7.5.8).
  /// \param set_chord_name  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr flex_data_function &on_set_chord_name(set_chord_name_fn set_chord_name) noexcept { set_chord_name_ = std::move(set_chord_name); return *this; }
  /// \brief Sets the function to be called for each Flex Data text (Text Messages Common Format) message (M2-104
  /// section 7.5.9).
  /// \param text  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr flex_data_function &on_text(text_fn text) noexcept { text_ = std::move(text); return *this; }

  /// \brief Calls the function registered by on_set_tempo(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param tempo  The Set Tempo message (M2-104 section 7.5.3).
  void set_tempo(Context c, flex_data::set_tempo const &tempo) const { call(set_tempo_, c, tempo); }
  /// \brief Calls the function registered by on_set_time_signature(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param ts  The Set Time Signature message (M2-104 section 7.5.4).
  void set_time_signature(Context c, flex_data::set_time_signature const &ts) const { call(set_time_signature_, c, ts); }
  /// \brief Calls the function registered by on_set_metronome(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param metronome  The Set Metronome message (M2-104 section 7.5.5).
  void set_metronome(Context c, flex_data::set_metronome const &metronome) const { call(set_metronome_, c, metronome); }
  /// \brief Calls the function registered by on_set_key_signature(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param ks  The Set Key Signature message (M2-104 section 7.5.7).
  void set_key_signature(Context c, flex_data::set_key_signature const &ks) const { call(set_key_signature_, c, ks); }
  /// \brief Calls the function registered by on_set_chord_name(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param chord  The Set Chord Name message (M2-104 section 7.5.8).
  void set_chord_name(Context c, flex_data::set_chord_name const &chord) const { call(set_chord_name_, c, chord); }
  /// \brief Calls the function registered by on_text(), if any.
  /// \param c  The context value to be passed to the function.
  /// \param t  The Flex Data text (Text Messages Common Format) message (M2-104 section 7.5.9).
  void text(Context c, flex_data::text_common const &t) const { call(text_, c, t); }
  // clang-format on

private:
  set_tempo_fn set_tempo_;
  set_time_signature_fn set_time_signature_;
  set_metronome_fn set_metronome_;
  set_key_signature_fn set_key_signature_;
  set_chord_name_fn set_chord_name_;
  text_fn text_;
};

static_assert(flex_data<flex_data_function<int>, int>, "flex_data_function must implement the flex_data concept");

}  // end namespace midi2::ump::dispatcher_backend

#endif  // MIDI2_UMP_DISPATCHER_BACKEND_HPP
