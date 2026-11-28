// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <format>
#include <iostream>

#include "midi2/midi2.hpp"

using namespace midi2::literals;  // for the _b (std::byte) literal

int main() {
  // Handlers share state through a context. This program doesn't need any.
  struct context {};
  auto dispatcher = midi2::ump::make_ump_function_dispatcher(context{});
  dispatcher.config().m2cvm.on_note_on([](context&, midi2::ump::m2cvm::note_on const& non) {
    std::cout << std::format("note on: note={} velocity={:#06x}\n", non.note(), non.velocity());
  });

  midi2::bytestream::to_ump bytestream_to_ump{0};  // MIDI 1.0 bytes to MIDI 1.0 UMP on group 0
  midi2::ump::ump_to_midi2 midi1_to_midi2{0};      // MIDI 1.0 UMP to MIDI 2.0 UMP

  // Two MIDI 1.0 Note On messages. The second uses running status.
  for (auto const b : {0x90_b, 0x3C_b, 0x7F_b, 0x40_b, 0x40_b}) {
    bytestream_to_ump.push(b);
    while (!bytestream_to_ump.empty()) {
      midi1_to_midi2.push(bytestream_to_ump.pop());
      while (!midi1_to_midi2.empty()) {
        dispatcher.dispatch(midi1_to_midi2.pop());
      }
    }
  }
}
