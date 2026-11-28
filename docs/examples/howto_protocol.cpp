// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstdint>
#include <format>
#include <iostream>

#include "midi2/ump/ump_to_midi1.hpp"
#include "midi2/ump/ump_to_midi2.hpp"
#include "midi2/ump/ump_types.hpp"

namespace {

template <typename Translator> void drain(Translator& t) {
  while (!t.empty()) {
    std::cout << std::format(" {:08X}", t.pop());
  }
}

template <typename Translator, typename Message> void push(Translator& t, Message const& message) {
  midi2::ump::apply(message, [&t](std::uint32_t const word) {
    t.push(word);
    drain(t);
    return false;  // Continue with the next word
  });
}

}  // end anonymous namespace

int main() {
  namespace m1 = midi2::ump::m1cvm;
  namespace m2 = midi2::ump::m2cvm;

  std::cout << "MIDI 1.0 to MIDI 2.0\n";
  midi2::ump::ump_to_midi2 to_midi2{0};
  auto cc = m1::control_change{}.group(0).channel(0);

  std::cout << "  bank select and program change:";
  push(to_midi2, cc.controller(0).value(1));   // Bank Select MSB
  push(to_midi2, cc.controller(32).value(2));  // Bank Select LSB
  push(to_midi2, m1::program_change{}.group(0).channel(0).program(3));
  std::cout << '\n';

  std::cout << "  RPN 0 (pitch bend sensitivity):";
  push(to_midi2, cc.controller(101).value(0));  // RPN MSB
  push(to_midi2, cc.controller(100).value(0));  // RPN LSB
  push(to_midi2, cc.controller(6).value(12));   // Data Entry MSB: the value is held, waiting for an LSB
  std::cout << " (held) then flush:";
  to_midi2.flush(0, 0);
  drain(to_midi2);
  std::cout << '\n';

  std::cout << "MIDI 2.0 to MIDI 1.0\n";
  midi2::ump::to_midi1 to_midi1;
  std::cout << "  note on:";
  push(to_midi1, m2::note_on{}.group(0).channel(0).note(60).velocity(0xFFFF));
  std::cout << "\n  registered controller:";
  push(to_midi1, m2::rpn_controller{}.group(0).channel(0).bank(0).index(0).value(0x18000000));
  std::cout << '\n';
}
