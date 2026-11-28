// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstdint>
#include <format>
#include <iostream>

#include "midi2/bytestream/ump_to_bytestream.hpp"

int main() {
  midi2::bytestream::to_bytestream ump_to_bytestream;
  // Translate only the messages on group 0. Bit n of the bitmap selects group n.
  ump_to_bytestream.group_filter(0b0000'0000'0000'0001);

  // Three MIDI 1.0 Note On messages, a Timing Clock, and a Note Off. The third Note On is on group 1.
  for (std::uint32_t const word : {0x20903C40U, 0x20904040U, 0x21904340U, 0x10F80000U, 0x20803C00U}) {
    ump_to_bytestream.push(word);
    while (!ump_to_bytestream.empty()) {
      std::cout << std::format("{:02X} ", std::to_integer<unsigned>(ump_to_bytestream.pop()));
    }
  }
  std::cout << '\n';
}
