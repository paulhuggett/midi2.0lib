//===-- Demo Compressed Property Exchange Viewer ------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file pe_zlib_viewer.cpp
/// \brief Displays the contents of a MIDI-CI Property Exchange message whose JSON property data is compressed and
///   encoded.
///
/// The property data of a Property Exchange message may be zlib compressed and then Mcoded7 encoded to fit into the
/// 7-bit bytes of System Exclusive (see "Compressed and Encoded JSON Property Data" in the Common Rules for MIDI-CI
/// Property Exchange specification). The header's "mutualEncoding" property is then "zlib+Mcoded7".
///
/// The UMP System Exclusive (7-bit) packets are passed to a ci_dispatcher which streams Property Exchange messages.
/// The header is given directly to the peejay JSON parser. The property data takes the reverse of the sender's path:
/// each piece is decoded from Mcoded7, inflated by zlib, and the resulting JSON passed to the parser. Neither the
/// encoded nor the decompressed property data is ever held in its entirety. Property data which is encoded as Mcoded7
/// without being compressed ("Mcoded7") and which is not encoded at all ("ASCII") is also supported.
///
/// A sender must escape any character which is not ASCII, but Mcoded7 is able to carry bytes of any value. The JSON is
/// therefore checked as it is parsed: well-formed UTF-8 is accepted and anything else is reported as an error.
///
/// Property data need not be JSON if the header's "mediaType" property names another type. We can't display that data,
/// so it is reported as an error and ignored.

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <version>

// Local includes
#include "mcoded7_decoder.hpp"
#include "zlib_decoder.hpp"

// midi2 library
#include <midi2/ci/ci_dispatcher.hpp>
#include <midi2/ump/ump_dispatcher.hpp>

// icubaby Unicode transcoder
#include <icubaby/icubaby.hpp>

// peejay JSON parser
#include <peejay/json.hpp>
#include <peejay/writer.hpp>

using namespace std::string_view_literals;

namespace {

/// A Reply to Get Property Data message for a ProgramList resource, as UMP System Exclusive (7-bit) packets laid out
/// two to a line. The header is {"status":200,"mutualEncoding":"zlib+Mcoded7"}. The property data is the following
/// 654 bytes of JSON, compressed to 224 bytes by zlib and then encoded as 256 bytes of Mcoded7:
///
///     [{"title":"Grand Piano","bankPC":[0,0,0]},{"title":"Bright Piano","bankPC":[0,0,1]},
///      {"title":"Electric Piano","bankPC":[0,0,2]},{"title":"Harpsichord","bankPC":[0,0,3]},
///      {"title":"Celesta","bankPC":[0,0,4]},{"title":"Glockenspiel","bankPC":[0,0,5]},
///      {"title":"Music Box","bankPC":[0,0,6]},{"title":"Vibraphone","bankPC":[0,0,7]},
///      {"title":"Marimba","bankPC":[0,0,8]},{"title":"Xylophone","bankPC":[0,0,9]},
///      {"title":"Tubular Bells","bankPC":[0,0,10]},{"title":"Dulcimer","bankPC":[0,0,11]},
///      {"title":"Drawbar Organ","bankPC":[0,0,12]},{"title":"Church Organ","bankPC":[0,0,13]},
///      {"title":"Reed Organ","bankPC":[0,0,14]},{"title":"Accordion","bankPC":[0,0,15]}]
// clang-format off
constexpr auto packets = std::array<std::uint32_t, 108>{
    0x30167E7F, 0x0D350221, 0x30260615, 0x03566848,
    0x30260001, 0x2E007B22, 0x30267374, 0x61747573,
    0x3026223A, 0x3230302C, 0x3026226D, 0x75747561,
    0x30266C45, 0x6E636F64, 0x3026696E, 0x67223A22,
    0x30267A6C, 0x69622B4D, 0x3026636F, 0x64656437,
    0x3026227D, 0x01000100, 0x30260002, 0x28785A75,
    0x3026514D, 0x4B031F31, 0x30261046, 0x712F3264,
    0x3026703C, 0x077A525F, 0x30267A73, 0x4137522F,
    0x30266211, 0x11215674, 0x30263049, 0x0E4D5034,
    0x30260659, 0x66135444, 0x30266F6E, 0x186E3249,
    0x3026165C, 0x7F3C3309, 0x3026796F, 0x7E542468,
    0x30263D50, 0x2D540641, 0x30260F4D, 0x69164007,
    0x3026552A, 0x0D7E7538, 0x30266D55, 0x6A376873,
    0x30267C59, 0x7F367F30, 0x3026633A, 0x4458382F,
    0x30262F04, 0x7C741F68, 0x30262213, 0x19371742,
    0x3026103E, 0x010F1319, 0x30261B78, 0x39286115,
    0x3026003D, 0x3A1C3622, 0x30261468, 0x5A200D0B,
    0x30266066, 0x087E1A09, 0x30265D29, 0x4917423E,
    0x3026277C, 0x6B537A05, 0x30262F12, 0x5D08764E,
    0x30265A1A, 0x61344163, 0x3026696E, 0x4B651C30,
    0x30261D74, 0x753A2B3B, 0x3026013E, 0x3E5D185D,
    0x3026573A, 0x176C2D69, 0x30266400, 0x5F1B0E1D,
    0x30261B2A, 0x1F163051, 0x30265649, 0x193A2157,
    0x3026024C, 0x1659337C, 0x30266A3C, 0x6E794207,
    0x30267015, 0x1545387A, 0x30261B58, 0x58335436,
    0x30260B79, 0x451C4E40, 0x30265965, 0x6A41181C,
    0x30261842, 0x6D726A15, 0x30367D1F, 0x17744A47,
};
// clang-format on

/// The peejay policies.
struct json_policies : peejay::default_policies {
  /// The maximum length of a string. The header's "mediaType" property may have as many as 75 characters.
  static constexpr std::size_t max_length = 75;
};

/// A peejay::writer which also remembers the values of the header's "mutualEncoding" and "mediaType" properties. Its
/// begin_object(), end_object(), key(), and string_value() members hide those of the writer: the parser calls the
/// backend's members directly, so no virtual functions are needed.
class header_writer : public peejay::writer<std::ostream_iterator<char>, json_policies> {
public:
  using writer::writer;

  /// The encodings of property data which may be named by the "mutualEncoding" property.
  enum class encoding {
    ascii,         ///< Uncompressed and unencoded (must be 7-bit data).
    mcoded7,       ///< Encoded using Mcoded7.
    zlib_mcoded7,  ///< Compressed using zlib and then encoded using Mcoded7.
    unsupported,   ///< Any other encoding.
  };

  /// Writes the start of an object. Only the properties of the outermost object are remembered.
  ///
  /// \returns The error code from the writer.
  std::error_code begin_object() {
    ++depth_;
    return writer::begin_object();
  }

  /// Writes the end of an object.
  ///
  /// \returns The error code from the writer.
  std::error_code end_object() {
    --depth_;
    return writer::end_object();
  }

  /// Writes the key of an object member and, if it is one of the header's properties whose value is to be remembered,
  /// notes that its value is expected next.
  ///
  /// \param k  The key.
  /// \returns The error code from the writer.
  std::error_code key(std::u8string_view const& k) {
    using enum property;
    property_ = other;
    if (depth_ == 1U) {
      if (k == u8"mutualEncoding"sv) {
        property_ = mutual_encoding;
      } else if (k == u8"mediaType"sv) {
        property_ = media_type;
      }
    }
    return writer::key(k);
  }

  /// Writes a string and, if it is the value of the "mutualEncoding" or "mediaType" property, remembers it.
  ///
  /// \param s  The string.
  /// \returns The error code from the writer.
  std::error_code string_value(std::u8string_view const& s) {
    using enum property;
    switch (property_) {
    case mutual_encoding: encoding_ = to_encoding(s); break;
    case media_type: json_ = is_json(s); break;
    case other: break;
    }
    property_ = other;
    return writer::string_value(s);
  }

  /// \returns The value of the "mutualEncoding" property or encoding::ascii if there was none.
  [[nodiscard]] encoding const& get_encoding() const noexcept { return encoding_; }
  /// \returns True if the "mediaType" property named JSON or if there was none: property data is JSON by default.
  [[nodiscard]] bool json_media_type() const noexcept { return json_; }

private:
  /// The header properties whose values are remembered.
  enum class property {
    other,            ///< Any property whose value is not remembered.
    mutual_encoding,  ///< The "mutualEncoding" property.
    media_type,       ///< The "mediaType" property.
  };

  /// The number of objects that are currently open.
  unsigned depth_ = 0U;
  /// The property whose value is expected next.
  property property_ = property::other;
  /// The value of the "mutualEncoding" property.
  encoding encoding_ = encoding::ascii;
  /// False if the "mediaType" property named a type other than JSON.
  bool json_ = true;

  /// Converts the value of the "mutualEncoding" property.
  ///
  /// \param s  The value of the property.
  /// \returns The encoding named by \p s.
  [[nodiscard]] static encoding to_encoding(std::u8string_view const s) noexcept {
    using enum encoding;
    if (s == u8"ASCII"sv) {
      return ascii;
    }
    if (s == u8"Mcoded7"sv) {
      return mcoded7;
    }
    if (s == u8"zlib+Mcoded7"sv) {
      return zlib_mcoded7;
    }
    return unsupported;
  }

  /// Checks the value of the "mediaType" property. Media types are not case-sensitive.
  ///
  /// \param s  The value of the property.
  /// \returns True if \p s is the JSON media type.
  [[nodiscard]] static bool is_json(std::u8string_view const s) {
    auto const lower = [](char8_t const c) {
      return c >= u8'A' && c <= u8'Z' ? static_cast<char8_t>(c - u8'A' + u8'a') : c;
    };
    return std::ranges::equal(s, u8"application/json"sv, std::ranges::equal_to{}, lower);
  }
};

/// Converts a piece of a message to the type of input that peejay expects.
///
/// \tparam T  The element type of the piece: either char or std::byte.
/// \param s  The characters or bytes to be converted.
/// \returns The characters or bytes of \p s as UTF-8 code units.
template <typename T>
  requires(sizeof(T) == sizeof(char8_t))
std::u8string_view utf8(std::span<T const> const s) {
#if defined(__cpp_lib_start_lifetime_as) && __cpp_lib_start_lifetime_as >= 202207L
  auto const* const ptr = std::start_lifetime_as<char8_t const>(s.data());
#else
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto const* const ptr = reinterpret_cast<char8_t const*>(s.data());
#endif
  return std::u8string_view{ptr, s.size()};
}

/// A JSON parser which writes its input to the standard output and remembers the header's properties.
using json_parser = peejay::parser<header_writer>;
/// The number of spaces by which each level of the displayed JSON is indented.
constexpr auto json_indent = std::size_t{2};
/// Tells the parser to report ill-formed UTF-8 as an error rather than replace it with U+FFFD.
constexpr auto reject_ill_formed = true;

/// The state shared by the MIDI-CI message handlers.
struct context {
  /// The header is complete before any of the property data arrives, so one parser handles both: finish()
  /// prepares it for the next JSON document.
  json_parser json{std::ostream_iterator<char>{std::cout}, json_indent};
  /// Checks that the parser's input is well-formed UTF-8. A new instance is needed for each JSON document because it
  /// never forgets that it has seen ill-formed input.
  icubaby::t8_8 transcoder;
  /// Inflates the property data.
  zlib_decoder inflater;
  /// Decodes the property data from Mcoded7.
  mcoded7_decoder mcoded7;
  /// The encoding of the message's property data.
  header_writer::encoding encoding = header_writer::encoding::ascii;
  /// False if the message's property data must be ignored because its encoding or media type is not supported.
  bool supported = true;
  /// False once an error has been reported.
  bool ok = true;
};

/// Prepares for a new JSON document, discarding anything that remains of the previous one.
///
/// \param ctxt  The state shared by the message handlers.
void reset_json(context& ctxt) {
  ctxt.json.reset(std::ostream_iterator<char>{std::cout}, json_indent);
  ctxt.transcoder = icubaby::t8_8{};
}

/// Passes a piece of a JSON document to the parser. Ill-formed UTF-8 stops the parse with an error which is reported by
/// finish().
///
/// \tparam T  The element type of the piece: either char or std::byte.
/// \param ctxt  The state shared by the message handlers.
/// \param piece  The piece of the JSON document.
template <typename T> void parse(context& ctxt, std::span<T const> const piece) {
  ctxt.json.input(ctxt.transcoder, utf8(piece), reject_ill_formed);
}

/// Completes a JSON parse, reporting any error, and prepares for the next JSON document.
///
/// \param ctxt  The state shared by the message handlers.
/// \returns True if the parse succeeded.
bool finish(context& ctxt) {
  ctxt.json.eof(ctxt.transcoder, reject_ill_formed);
  std::cout << '\n';  // The writer does not end the document with a newline.
  auto const err = ctxt.json.last_error();
  if (err) {
    std::cerr << "JSON error: " << err.message() << '\n';
  }
  reset_json(ctxt);
  return !err;
}

/// Called at the start of each Property Exchange message. Discards anything that remains of the previous message.
///
/// \param ctxt  The state shared by the message handlers.
void start(context& ctxt, midi2::ci::header const&, midi2::ci::property_exchange::stream_start const&) {
  reset_json(ctxt);  // The previous message may have been incomplete.
  ctxt.inflater.reset();
  ctxt.mcoded7.reset();
  std::cout << "Header:\n";
}

/// Passes a piece of the header to the JSON parser.
///
/// \param ctxt  The state shared by the message handlers.
/// \param piece  The piece of the header.
void header_data(context& ctxt, midi2::ci::header const&, std::span<char const> const piece) {
  parse(ctxt, piece);
}

/// Called when the header is complete and the property data follows. An unsupported encoding or media type is reported
/// as an error.
///
/// \param ctxt  The state shared by the message handlers.
void chunk(context& ctxt, midi2::ci::header const&, midi2::ci::property_exchange::stream_chunk const&) {
  // The header's "mutualEncoding" property tells us whether the property data must be decoded and decompressed and its
  // "mediaType" whether the result is JSON. Copy them before finish() replaces the parser's backend.
  ctxt.encoding = ctxt.json.backend().get_encoding();
  auto const json = ctxt.json.backend().json_media_type();
  ctxt.supported = json && ctxt.encoding != header_writer::encoding::unsupported;
  ctxt.ok = finish(ctxt) && ctxt.ok;
  if (!ctxt.supported) {
    std::cerr << (json ? "Unsupported mutualEncoding\n" : "Unsupported mediaType\n");
    ctxt.ok = false;
    return;
  }
  std::cout << "Property data:\n";
}

/// Decodes a piece of the property data and passes the resulting JSON to the parser. The data is ignored if chunk()
/// reported that its encoding or media type is not supported.
///
/// \param ctxt  The state shared by the message handlers.
/// \param piece  The piece of property data.
void property_data(context& ctxt, midi2::ci::header const&, std::span<std::byte const> const piece) {
  using enum header_writer::encoding;
  if (!ctxt.supported) {
    return;  // Already reported by chunk().
  }
  auto const to_parser = [&ctxt](std::span<std::byte const> const decoded) {
    parse(ctxt, decoded);
    return true;  // The parser's errors are reported by finish().
  };
  switch (ctxt.encoding) {
  case ascii: parse(ctxt, piece); break;
  case mcoded7: ctxt.ok = ctxt.mcoded7.input(piece, to_parser) && ctxt.ok; break;
  case zlib_mcoded7:
    ctxt.ok = ctxt.mcoded7.input(piece, [&ctxt, &to_parser](std::span<std::byte const> const decoded) {
      return ctxt.inflater.input(decoded, to_parser);
    }) && ctxt.ok;
    break;
  case unsupported: break;  // Excluded by the check of ctxt.supported.
  }
}

/// Called at the end of each Property Exchange message. Reports an incomplete compressed stream or a JSON error.
///
/// \param ctxt  The state shared by the message handlers.
void end(context& ctxt, midi2::ci::header const&) {
  if (!ctxt.supported) {
    return;
  }
  if (ctxt.encoding == header_writer::encoding::zlib_mcoded7) {
    // Bad Mcoded7 data has already been reported and will have left the compressed stream incomplete.
    ctxt.ok = ctxt.mcoded7.good() && ctxt.inflater.end() && ctxt.ok;
  }
  ctxt.ok = finish(ctxt) && ctxt.ok;
}

}  // end anonymous namespace

int main() {
  // At 32 bytes, the dispatcher's buffer is much smaller than the message: the header and data pass through it in
  // pieces.
  using dispatcher = midi2::ci::ci_dispatcher<midi2::ci::function_stream_config<context, 32U>>;
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
  auto const sysex7 = []<midi2::ump::mt::data64 Status>(dispatcher& d,
                                                        midi2::ump::data64::details::sysex7<Status> const& message) {
    for (auto const b : message) {
      d.dispatch(static_cast<std::byte>(b.get()));
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
