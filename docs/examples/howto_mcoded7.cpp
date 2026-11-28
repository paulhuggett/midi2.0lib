// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <vector>

#include "midi2/mcoded7.hpp"
#include "midi2/utils.hpp"

using namespace midi2::literals;  // for the _b (std::byte) literal

namespace {

void show(char const* const title, std::vector<std::byte> const& bytes) {
  std::cout << title;
  for (auto const b : bytes) {
    std::cout << std::format(" {:02X}", std::to_integer<unsigned>(b));
  }
  std::cout << '\n';
}

}  // end anonymous namespace

int main() {
  // Eight bytes of binary data: the most significant bit of some of them is set.
  std::vector const original{0x81_b, 0x02_b, 0xFF_b, 0x04_b, 0x05_b, 0x06_b, 0x87_b, 0xF0_b};
  show("original:", original);

  // Encode: every group of seven bytes becomes eight. flush() writes the final, short, group.
  midi2::mcoded7::encoder encoder;
  std::vector<std::byte> encoded;
  auto out = std::back_inserter(encoded);
  for (auto const b : original) {
    out = encoder.parse_byte(b, out);
  }
  encoder.flush(out);
  show("encoded: ", encoded);

  // Decode.
  midi2::mcoded7::decoder decoder;
  std::vector<std::byte> decoded;
  auto out2 = std::back_inserter(decoded);
  for (auto const b : encoded) {
    out2 = decoder.parse_byte(b, out2);
  }
  decoder.flush(out2);
  show("decoded: ", decoded);
  std::cout << (decoder.good() ? "valid" : "invalid") << " Mcoded7\n";
}
