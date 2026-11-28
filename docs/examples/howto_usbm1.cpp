// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstdint>
#include <format>
#include <iostream>

#include "midi2/bytestream/bytestream_to_ump.hpp"
#include "midi2/bytestream/usbm1_to_bytestream.hpp"

int main() {
  // Translate the packets for virtual cable 0, then convert the resulting bytestream to UMP on group 0.
  midi2::bytestream::usbm1_to_bytestream usb_to_bytestream{0};
  midi2::bytestream::to_ump bytestream_to_ump{0};

  // USB-MIDI 1.0 Event Packets: Note On (CIN 9) on cable 0, Note On on cable 1, and Note Off (CIN 8) on cable 0.
  for (std::uint32_t const packet : {0x09903C40U, 0x19904040U, 0x08803C00U}) {
    usb_to_bytestream.push(packet);
    while (!usb_to_bytestream.empty()) {
      bytestream_to_ump.push(usb_to_bytestream.pop());
      while (!bytestream_to_ump.empty()) {
        std::cout << std::format("{:08X}\n", bytestream_to_ump.pop());
      }
    }
  }
}
