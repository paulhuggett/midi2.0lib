//===-- USB MIDI1.0 to Bytestream ---------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//
/// \file usbm1_to_bytestream.hpp
///
/// \brief USB-MIDI Event Packets to MIDI 1.0 bytestream (\ref midi2::bytestream::usbm1_to_bytestream).
///
/// This translator converts USB-MIDI event packets as described by the document "Universal Serial Bus Device Class
/// Definition for MIDI Devices" Release 1.0 dated Nov 1, 1999 to a MIDI 1.0 bytestream.

#ifndef MIDI2_BYTESTREAM_USBM1_TO_BYTESTREAM_HPP
#define MIDI2_BYTESTREAM_USBM1_TO_BYTESTREAM_HPP

// Standard library
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>

// Local includes
#include "midi2/adt/fifo.hpp"
#include "midi2/translator.hpp"

namespace midi2::bytestream {

namespace details {

/// \brief Packs a sequence of up to 16 2-bit values into an unsigned 32-bit integer.
///
/// The first value occupies the least significant two bits of the result.
///
/// \tparam Args  The types of the second and subsequent values.
/// \param first  The first 2-bit value.
/// \param rest  The second and subsequent 2-bit values.
/// \returns The packed values.
template <typename... Args>
[[nodiscard]] consteval std::uint32_t pack_to_32_bits(std::uint32_t const first, Args&&... rest) noexcept {
  assert(first <= 0b11);
  if constexpr (sizeof...(Args) == 0U) {
    return first;
  } else {
    static_assert(sizeof...(Args) < 16U, "A maximum of 16 2-bit values");
    return first | (pack_to_32_bits(std::forward<Args>(rest)...) << 2U);
  }
}
/// \brief Extracts a 2-bit value from a 32-bit unsigned value produced by pack_to_32_bits().
/// \param packed  A value produced by pack_to_32_bits().
/// \param index  The index of the value to be extracted (0 to 15).
/// \returns The 2-bit value at position \p index.
[[nodiscard]] constexpr unsigned extract_from_32_bits(std::uint32_t const packed, unsigned const index) noexcept {
  assert(index < 16);
  return ((packed >> (index << 1)) & 0b11);
}

}  // end namespace details

/// \brief Converts USB-MIDI 1.0 Event Packets to a MIDI 1.0 bytestream.
///
/// Only packets addressed to the selected virtual cable are translated; all others are ignored.
class usbm1_to_bytestream {
public:
  /// \brief The type of input: a 32-bit USB-MIDI Event Packet
  using input_type = std::uint32_t;
  /// \brief The type of output to a bytestream
  using output_type = std::byte;

  /// \brief Constructs a translator for virtual cable 0.
  constexpr usbm1_to_bytestream() noexcept = default;

  /// \brief Constructs a translator for the given virtual cable.
  /// \param cable  The virtual cable number whose messages are to be translated.
  explicit constexpr usbm1_to_bytestream(std::uint8_t const cable) noexcept : cable_{cable} {
    assert(cable < 16U && "cable number must be four bits");
  }

  /// \brief Sets the cable number to be translated.
  ///
  /// Any in-flight messages are lost.
  ///
  /// \param cable  The virtual cable number whose messages are to be translated.
  constexpr void set_cable(std::uint8_t const cable) {
    assert(cable < 16U && "cable number must be four bits");
    reset();
    cable_ = cable;
  }
  /// \brief Returns the cable number for messages to be translated.
  /// \returns The virtual cable number.
  [[nodiscard]] constexpr std::uint8_t get_cable() const noexcept { return cable_; }

  /// \brief Checks if the output has no elements
  /// \returns True if there are no bytes waiting to be popped; false otherwise.
  [[nodiscard]] constexpr bool empty() const noexcept { return output_.empty(); }
  /// \brief Pops and returns the next available byte for the bytestream
  /// \return The next available byte
  /// \pre !empty()
  [[nodiscard]] constexpr output_type pop() noexcept {
    assert(!empty());
    return output_.pop_front();
  }

  /// \brief Provides a word of USBM1 input to the translator.
  ///
  /// Messages addressed to a cable number different from the one set in the constructor or by a call to set_cable()
  /// are ignored.
  ///
  /// \param usbm1  The word of input to be translated
  constexpr void push(input_type const usbm1) noexcept {
    if (cable(usbm1) != this->get_cable()) {
      return;
    }
    auto const bytes = midi_x_size(get_cin(usbm1));
    assert(bytes < 4);
    if (bytes > 0) {
      output_.push_back(static_cast<std::byte>((usbm1 >> 16) & 0xFF));
    }
    if (bytes > 1) {
      output_.push_back(static_cast<std::byte>((usbm1 >> 8) & 0xFF));
    }
    if (bytes > 2) {
      output_.push_back(static_cast<std::byte>(usbm1 & 0xFF));
    }
  }

  /// \brief Restore the translator to its original state. Sets the cable number to 0.
  ///
  /// Any in-flight messages are lost.
  constexpr void reset() {
    output_.clear();
    cable_ = 0U;
  }

private:
  std::uint8_t cable_ = 0;
  adt::fifo<std::byte, 4> output_;

  /// \brief Extracts the virtual cable number from a USB-MIDI message.
  /// \param p  A USB_MIDI1 message
  /// \returns The virtual cable number field from message \p p
  [[nodiscard]] static constexpr std::uint8_t cable(std::uint32_t const p) noexcept { return (p >> 28) & 0x0F; }

  /// \brief Extracts the USB MIDI1 Code Index Number from the supplied USB-MIDI message.
  /// \param p  A USB_MIDI1 message
  /// \return The USB MIDI1 Code Index Number field from message \p p
  [[nodiscard]] static constexpr std::uint8_t get_cin(std::uint32_t const p) noexcept { return (p >> 24) & 0x0F; }

  /// \brief Converts the Code Index Number (CIN) from a USB-MIDI packet to the number of bytes contained within the
  /// packet's fields.
  /// \param cin  The Code Index Number.
  /// \returns The number of MIDI bytes carried by a packet with this Code Index Number (0 to 3).
  [[nodiscard]] static constexpr unsigned midi_x_size(std::uint8_t const cin) noexcept {
    assert(cin < 0x10U && "code index number should be four bits");

    // These values come from Table 4-1: "Code Index Number Classifications" in the "Universal
    // Serial Bus Device Class Definition for MIDI Devices" (Release 1.0 Nov 1, 1999)
    static constexpr std::uint32_t packed =
        details::pack_to_32_bits(0U,  // [0] Reserved for future extension
                                 0U,  // [1] Reserved for future extension
                                 2U,  // [2] Two-byte System Common messages
                                 3U,  // [3] Three-byte System Common messages
                                 3U,  // [4] SysEx starts or continues
                                 1U,  // [5] Single-byte System Common/SysEx end Message
                                 2U,  // [6] SysEx ends with following two bytes
                                 3U,  // [7] SysEx ends with following three bytes
                                 3U,  // [8] Note-off
                                 3U,  // [9] Note-on
                                 3U,  // [A] Poly Key Press
                                 3U,  // [B] Control Change
                                 2U,  // [C] Program Change
                                 2U,  // [D] Channel Pressure
                                 3U,  // [E] Pitch-bend Change
                                 1U   // [F] Single byte
        );
    return details::extract_from_32_bits(packed, cin);
  }
};

static_assert(translator<std::uint32_t, std::byte, usbm1_to_bytestream>);

}  // end namespace midi2::bytestream

#endif  // MIDI2_BYTESTREAM_USBM1_TO_BYTESTREAM_HPP
