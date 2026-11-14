//===-- Bytestream Types ------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file bytestream_types.hpp
/// \brief
#ifndef MIDI2_BYTESTREAM_BYTESTREAM_TYPES_HPP
#define MIDI2_BYTESTREAM_BYTESTREAM_TYPES_HPP

#include <cstddef>
#include <cstdint>

/// \brief Functions and types relating to processing of a MIDI 1.0 bytestream
namespace midi2::bytestream {

/// \brief Bytestream status message codes
enum class status : std::uint8_t {
  // Channel voice messages
  note_off = 0x80,          ///< Note Off
  note_on = 0x90,           ///< Note On
  poly_pressure = 0xA0,     ///< Polyphonic Key Pressure (Aftertouch).
  cc = 0xB0,                ///< Continuous Controller
  program_change = 0xC0,    ///< Program Change
  channel_pressure = 0xD0,  ///< Channel Pressure (Aftertouch).
  pitch_bend = 0xE0,        ///< Pitch Bend Change

  // System Common/Real Time/Exclusive Messages
  sysex_start = 0xF0,  ///< Begin system exclusive data
  timing_code = 0xF1,  ///< MTC quarter frame
  spp = 0xF2,          ///< Song Position Pointer
  /// The song or sequence to be played upon receipt of a Start message.
  song_select = 0xF3,
  reserved1 = 0xF4,  ///< Undefined (reserved) System Common status
  reserved2 = 0xF5,  ///< Undefined (reserved) System Common status
  /// Used with analog synthesizers to request that all oscillators be tuned.
  tune_request = 0xF6,
  sysex_stop = 0xF7,         ///< End of system exclusive
  timing_clock = 0xF8,       ///< Synchronize clock-based MIDI systems
  reserved3 = 0xF9,          ///< Undefined (reserved) System Real Time status
  sequence_start = 0xFA,     ///< Start the current sequence playing
  sequence_continue = 0xFB,  ///< Continue at the point the sequence was stopped
  sequence_stop = 0xFC,      ///< Stop the current sequence
  reserved4 = 0xFD,          ///< Undefined (reserved) System Real Time status
  /// Sent at intervals of no more than 300 ms whenever there is no other MIDI data being transmitted
  active_sensing = 0xFE,
  system_reset = 0xFF,  ///< Reset all receivers to their power-up state
};

/// \brief Checks whether a byte is the status byte of a System Real Time message.
///
/// System Real Time messages may appear anywhere in the bytestream (including between the bytes of other messages) and
/// do not affect running status. The reserved System Real Time status bytes (0xF9 and 0xFD) are not included.
///
/// \param midi1_byte  The byte to be checked.
/// \returns True if \p midi1_byte is a System Real Time status byte; false otherwise.
[[nodiscard]] constexpr bool is_system_real_time_message(std::byte const midi1_byte) noexcept {
  using enum status;
  switch (static_cast<status>(midi1_byte)) {
  case timing_clock:
  case sequence_start:
  case sequence_continue:
  case sequence_stop:
  case active_sensing:
  case system_reset: return true;
  default: return false;
  }
}

/// \brief Checks whether a byte is a status byte (that is, its most significant bit is set).
/// \param midi1_byte  The byte to be checked.
/// \returns True if \p midi1_byte is a status byte; false if it is a data byte.
[[nodiscard]] constexpr bool is_status_byte(std::byte const midi1_byte) noexcept {
  return (midi1_byte & std::byte{0x80}) != std::byte{0x00};
}

/// The Universal System Exclusive ID for Non-Real Time messages.
constexpr auto s7_universal_nrt = std::byte{0x7E};
/// The Universal System Exclusive Sub-ID#1 for MIDI Capability Inquiry (MIDI-CI) messages.
constexpr auto s7_midi_ci = std::byte{0x0D};

}  // end namespace midi2::bytestream

#endif  // MIDI2_BYTESTREAM_BYTESTREAM_TYPES_HPP
