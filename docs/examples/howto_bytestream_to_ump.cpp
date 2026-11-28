// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <format>
#include <iostream>

#include "midi2/bytestream/bytestream_to_ump.hpp"

using namespace midi2::literals;  // for the _b (std::byte) literal

int main() {
  // Messages arriving on the second MIDI 1.0 port are put on UMP group 1.
  midi2::bytestream::to_ump bytestream_to_ump{1};

  // An Identity Request System Exclusive message with a Timing Clock (F8) in the middle of it, followed by a Note On.
  for (auto const b : {0xF0_b, 0x7E_b, 0x7F_b, 0xF8_b, 0x06_b, 0x01_b, 0xF7_b, 0x92_b, 0x3C_b, 0x40_b}) {
    bytestream_to_ump.push(b);
    while (!bytestream_to_ump.empty()) {
      std::cout << std::format("{:08X}\n", bytestream_to_ump.pop());
    }
  }
}
