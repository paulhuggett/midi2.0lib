//===-- Demo Property Exchange Viewer -----------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file pe_viewer.cpp
/// \brief Displays the contents of a MIDI-CI Property Exchange message carried by UMP packets.
///
/// The UMP System Exclusive (7-bit) packets are passed to a ci_dispatcher which streams Property Exchange messages:
/// the header and property data are given to the peejay JSON parser piece by piece as they arrive. The message is
/// much larger than the dispatcher's buffer.

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

// midi2 library
#include <midi2/ci/ci_dispatcher.hpp>
#include <midi2/ump/ump_dispatcher.hpp>

// peejay JSON parser
#include <peejay/json.hpp>
#include <peejay/writer.hpp>

namespace {

/// A Reply to Get Property Data message for the DeviceInfo resource, as UMP System Exclusive (7-bit) packets laid
/// out two to a line. The header is {"status":200} and the property data is:
///
///     {"manufacturerId":[125,0,0],"familyId":[0,0],"modelId":[48,0],"versionId":[0,0,1,0],
///      "manufacturer":"Educational Use","family":"Example","model":"PE Viewer","version":"1.0"}
// clang-format off
constexpr auto packets = std::array<std::uint32_t, 70>{
    0x30167E7F, 0x0D350221, 0x30260615, 0x03566848,
    0x30260001, 0x0E007B22, 0x30267374, 0x61747573,
    0x3026223A, 0x3230307D, 0x30260100, 0x01002C01,
    0x30267B22, 0x6D616E75, 0x30266661, 0x63747572,
    0x30266572, 0x4964223A, 0x30265B31, 0x32352C30,
    0x30262C30, 0x5D2C2266, 0x3026616D, 0x696C7949,
    0x30266422, 0x3A5B302C, 0x3026305D, 0x2C226D6F,
    0x30266465, 0x6C496422, 0x30263A5B, 0x34382C30,
    0x30265D2C, 0x22766572, 0x30267369, 0x6F6E4964,
    0x3026223A, 0x5B302C30, 0x30262C31, 0x2C305D2C,
    0x3026226D, 0x616E7566, 0x30266163, 0x74757265,
    0x30267222, 0x3A224564, 0x30267563, 0x6174696F,
    0x30266E61, 0x6C205573, 0x30266522, 0x2C226661,
    0x30266D69, 0x6C79223A, 0x30262245, 0x78616D70,
    0x30266C65, 0x222C226D, 0x30266F64, 0x656C223A,
    0x30262250, 0x45205669, 0x30266577, 0x6572222C,
    0x30262276, 0x65727369, 0x30266F6E, 0x223A2231,
    0x30342E30, 0x227D0000,
};
// clang-format on

/// Converts a piece of a message to the type of input that peejay expects.
///
/// \tparam T  The element type of the piece: either char or std::byte.
/// \param s  The characters or bytes to be converted.
/// \returns The characters or bytes of \p s as UTF-8 code units.
template <typename T> std::u8string_view utf8(std::span<T const> const s) {
  return std::u8string_view{reinterpret_cast<char8_t const*>(s.data()), s.size()};
}

/// A JSON parser which writes its input to the standard output.
using json_parser = peejay::parser<peejay::writer<std::ostream_iterator<char>>>;
/// The number of spaces by which each level of the displayed JSON is indented.
constexpr auto json_indent = std::size_t{2};

/// Prepares a parser for a new JSON document, discarding anything that remains of the previous one.
///
/// \param parser  The parser to be reset.
void reset(json_parser& parser) {
  parser.reset(std::ostream_iterator<char>{std::cout}, json_indent);
}

/// Completes a JSON parse, reporting any error, and prepares for the next JSON document.
///
/// \param parser  The parser to be completed.
/// \returns True if the parse succeeded.
bool finish(json_parser& parser) {
  parser.eof();
  std::cout << '\n';  // The writer does not end the document with a newline.
  auto const err = parser.last_error();
  if (err) {
    std::cerr << "JSON error: " << err.message() << '\n';
  }
  reset(parser);
  return !err;
}

/// The state shared by the MIDI-CI message handlers. The dispatcher's configuration constructs it in place and passes
/// it to each handler by reference.
struct context {
  /// The header is complete before any of the property data arrives, so one parser handles both: finish()
  /// prepares it for the next JSON document.
  json_parser json{std::ostream_iterator<char>{std::cout}, json_indent};
  /// False once an error has been reported.
  bool ok = true;
};

/// Called at the start of each Property Exchange message. Discards anything that remains of the previous message.
///
/// \param ctxt  The state shared by the message handlers.
void start(context& ctxt, midi2::ci::header const&, midi2::ci::property_exchange::stream_start const&) {
  reset(ctxt.json);  // The previous message may have been incomplete.
  std::cout << "Header:\n";
}

/// Passes a piece of the header to the JSON parser.
///
/// \param ctxt  The state shared by the message handlers.
/// \param piece  The piece of the header.
void header_data(context& ctxt, midi2::ci::header const&, std::span<char const> const piece) {
  ctxt.json.input(utf8(piece));
}

/// Called when the header is complete and the property data follows.
///
/// \param ctxt  The state shared by the message handlers.
void chunk(context& ctxt, midi2::ci::header const&, midi2::ci::property_exchange::stream_chunk const&) {
  ctxt.ok = finish(ctxt.json) && ctxt.ok;
  std::cout << "Property data:\n";
}

/// Passes a piece of the property data to the JSON parser.
///
/// \param ctxt  The state shared by the message handlers.
/// \param piece  The piece of property data.
void property_data(context& ctxt, midi2::ci::header const&, std::span<std::byte const> const piece) {
  ctxt.json.input(utf8(piece));
}

/// Called at the end of each Property Exchange message.
///
/// \param ctxt  The state shared by the message handlers.
void end(context& ctxt, midi2::ci::header const&) {
  ctxt.ok = finish(ctxt.json) && ctxt.ok;
}

}  // end anonymous namespace

int main() {
  // The dispatcher's buffer is much smaller than the message: the header and data pass through it in pieces.
  using dispatcher = midi2::ci::ci_dispatcher<midi2::ci::function_stream_config<context, 32>>;
  dispatcher ci{std::in_place};

  midi2::ci::muid const my_muid{0x123456U};
  // Accept only messages sent to our MUID (and those that are broadcast, which are not passed to this handler).
  ci.config().system.on_check_muid(
      [my_muid](context&, std::uint8_t, midi2::ci::muid const muid) { return muid == my_muid; });
  ci.config()
      .property_exchange_stream.on_start(start)
      .on_header_data(header_data)
      .on_chunk(chunk)
      .on_property_data(property_data)
      .on_end(end);

  // Pass the bytes of each System Exclusive (7-bit) packet to the MIDI-CI dispatcher, which is the UMP dispatcher's
  // context.
  auto ump = midi2::ump::make_ump_function_dispatcher(std::ref(ci));
  auto const sysex7 = [](dispatcher& ci_dispatcher, auto const& message) {
    using value_type = std::remove_cvref_t<decltype(message)>::value_type;
    for (value_type const b : message) {
      ci_dispatcher.dispatch(static_cast<std::byte>(b));
    }
  };
  // The first packet of each System Exclusive message must also prepare the dispatcher for a new MIDI-CI message.
  auto const first_sysex7 = [sysex7](dispatcher& ci_dispatcher, auto const& message) {
    ci_dispatcher.reset();
    sysex7(ci_dispatcher, message);
  };
  ump.config()
      .data64.on_sysex7_in_1(first_sysex7)
      .on_sysex7_start(first_sysex7)
      .on_sysex7_continue(sysex7)
      .on_sysex7_end(sysex7);

  for (auto const word : packets) {
    ump.dispatch(word);
  }
  return ci.config().context.ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
