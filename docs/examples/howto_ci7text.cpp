// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

#include "midi2/ci/ci7text.hpp"

using namespace std::string_view_literals;

int main() {
  // UTF-8 to 7-bit MIDI-CI text.
  auto const original = u8"Café \\ Piano 🎹"sv;
  midi2::transcoder<char8_t, char> encode;
  std::string ci_text;
  auto out = std::back_inserter(ci_text);
  for (auto const cu : original) {
    out = encode(cu, out);
  }
  encode.end_cp(out);
  std::cout << "MIDI-CI text: " << ci_text << '\n';

  // And back to UTF-8.
  midi2::transcoder<char, char8_t> decode;
  std::u8string utf8;
  auto out8 = std::back_inserter(utf8);
  for (auto const c : ci_text) {
    out8 = decode(c, out8);
  }
  decode.end_cp(out8);
  std::cout << "round trip " << (utf8 == original && decode.well_formed() ? "succeeded" : "failed") << '\n';
}
