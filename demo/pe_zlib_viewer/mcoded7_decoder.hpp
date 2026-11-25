//===-- Mcoded7 Decoder -------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file mcoded7_decoder.hpp
/// \brief Decodes the Mcoded7 property data of a MIDI-CI Property Exchange message.

#ifndef DEMO_PE_ZLIB_VIEWER_MCODED7_DECODER_HPP
#define DEMO_PE_ZLIB_VIEWER_MCODED7_DECODER_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <span>

// midi2 library
#include <midi2/mcoded7.hpp>

/// Decodes Mcoded7 property data and passes the result to the next stage. The data may arrive in pieces of any size.
class mcoded7_decoder {
public:
  /// Prepares to receive the property data of a new message.
  void reset() { decoder_.reset(); }

  /// Decodes a piece of property data. Once an error has been reported, the rest of the message's property data is
  /// ignored.
  ///
  /// \tparam Sink  The type of a function which accepts a span of bytes and returns a boolean.
  /// \param piece  The Mcoded7 data to be decoded.
  /// \param sink  A function which is called with each block of decoded bytes. It returns false to stop decoding.
  /// \returns True if the data was valid and each call to \p sink returned true.
  template <std::predicate<std::span<std::byte const>> Sink> bool input(std::span<std::byte const> piece, Sink sink) {
    if (!this->good()) {
      return false;
    }
    std::array<std::byte, 32> out{};
    while (!piece.empty()) {
      // The decoder produces at most one byte of output for each byte of input.
      auto const block = piece.first(std::min(piece.size(), out.size()));
      auto* last = out.data();
      for (std::byte const b : block) {
        last = decoder_.parse_byte(b, last);
      }
      if (!this->good()) {
        std::cerr << "Property data error: bad Mcoded7 data\n";
        return false;
      }
      if (last != out.data() && !sink(std::span<std::byte const>{out.data(), last})) {
        return false;
      }
      piece = piece.subspan(block.size());
    }
    return true;
  }

  /// \returns True if the property data received so far was valid.
  [[nodiscard]] bool good() const noexcept { return decoder_.good(); }

private:
  /// The library's decoder, which processes a byte at a time.
  midi2::mcoded7::decoder decoder_;
};

#endif  // DEMO_PE_ZLIB_VIEWER_MCODED7_DECODER_HPP
