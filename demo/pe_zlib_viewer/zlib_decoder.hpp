//===-- zlib Decoder ----------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file zlib_decoder.hpp
/// \brief Decompresses the property data of a MIDI-CI Property Exchange message.

#ifndef DEMO_PE_ZLIB_VIEWER_ZLIB_DECODER_HPP
#define DEMO_PE_ZLIB_VIEWER_ZLIB_DECODER_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <iostream>
#include <span>
#include <string_view>

// zlib
#include <zlib.h>

/// Inflates property data that was compressed by zlib and passes the result to the next stage. The data must have
/// already been decoded from Mcoded7 and may arrive in pieces of any size.
///
/// zlib's internal state holds a pointer to the z_stream, so instances can be neither copied nor moved.
class zlib_decoder {
public:
  /// Initializes zlib. A failure is reported by the first call to input().
  zlib_decoder() {
    // Leaving next_in, zalloc, zfree, and opaque as null selects zlib's default memory allocation.
    init_ = inflateInit(&z_);
  }
  zlib_decoder(zlib_decoder const&) = delete;
  zlib_decoder(zlib_decoder&&) noexcept = delete;
  /// Releases the memory allocated by zlib.
  ~zlib_decoder() noexcept {
    if (init_ == Z_OK) {
      inflateEnd(&z_);
    }
  }
  zlib_decoder& operator=(zlib_decoder const&) = delete;
  zlib_decoder& operator=(zlib_decoder&&) noexcept = delete;

  /// Prepares to receive the property data of a new message.
  void reset() {
    if (init_ == Z_OK) {
      inflateReset(&z_);
    }
    done_ = false;
    failed_ = false;
  }

  /// Inflates a piece of property data. Once an error has been reported, the rest of the message's property data is
  /// ignored.
  ///
  /// \tparam Sink  The type of a function which accepts a span of bytes and returns a boolean.
  /// \param piece  The compressed data to be inflated.
  /// \param sink  A function which is called with each block of inflated bytes. It returns false to stop inflating: the
  ///   rest of the message's property data is then ignored.
  /// \returns True if the data was valid and each call to \p sink returned true.
  template <std::predicate<std::span<std::byte const>> Sink>
  bool input(std::span<std::byte const> const piece, Sink sink) {
    if (failed_) {
      return false;
    }
    if (init_ != Z_OK) {
      return this->error("zlib initialization failed");
    }
    if (piece.empty()) {
      return true;
    }
    if (done_) {
      return this->error("unexpected data after the compressed stream");
    }
    // The cmake project defines ZLIB_CONST, so this shouldn't require a const-cast.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    z_.next_in = reinterpret_cast<Bytef const*>(piece.data());
    z_.avail_in = static_cast<uInt>(piece.size());
    // Run inflate() until it has consumed all of the input and has no more output to deliver: a full output buffer
    // means that there may be more to come.
    std::array<std::byte, 64> out{};
    do {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      z_.next_out = reinterpret_cast<Bytef*>(out.data());
      z_.avail_out = static_cast<uInt>(out.size());
      switch (inflate(&z_, Z_NO_FLUSH)) {
      case Z_STREAM_END: done_ = true; break;
      case Z_OK:
      case Z_BUF_ERROR:  // No progress was possible: inflate() needs more input.
        break;
      default: return this->error(z_.msg != nullptr ? z_.msg : "zlib error");
      }
      if (auto const size = out.size() - z_.avail_out;
          size > 0 && !sink(std::span<std::byte const>{out.data(), size})) {
        failed_ = true;
        return false;
      }
    } while (z_.avail_out == 0 && !done_);

    if (z_.avail_in > 0) {
      return this->error("unexpected data after the compressed stream");
    }
    return true;
  }

  /// Checks that the complete compressed stream has been received.
  /// \returns True if the zlib stream was complete.
  bool end() {
    if (failed_) {
      return false;
    }
    return done_ || this->error("the compressed data was incomplete");
  }

private:
  /// The zlib stream.
  z_stream z_{};
  /// The result of initializing zlib.
  int init_ = Z_OK;
  /// True once zlib has seen the end of the compressed stream.
  bool done_ = false;
  /// True once an error has been reported or the sink has stopped the process.
  bool failed_ = false;

  /// Reports an error. The rest of the message's property data will be ignored.
  ///
  /// \param message  The error message.
  /// \returns False.
  bool error(std::string_view const message) {
    failed_ = true;
    std::cerr << "Property data error: " << message << '\n';
    return false;
  }
};

#endif  // DEMO_PE_ZLIB_VIEWER_ZLIB_DECODER_HPP
