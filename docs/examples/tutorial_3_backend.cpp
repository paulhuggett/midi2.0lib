// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <bitset>
#include <cstdint>
#include <format>
#include <iostream>

#include "midi2/ump/ump_dispatcher.hpp"

namespace {

namespace backend = midi2::ump::dispatcher_backend;

// Records which notes are sounding on each of the 16 channels of group 0.
struct held_notes {
  std::bitset<128> notes[16];
};

// Handles the MIDI 2.0 Channel Voice messages. It inherits handlers that do nothing for all of the messages, then
// replaces those for Note On and Note Off.
struct note_tracker : backend::m2cvm_null<held_notes> {
  static void note_on(held_notes& h, midi2::ump::m2cvm::note_on const& non) {
    if (non.group() == 0) {
      h.notes[non.channel()].set(non.note());
    }
  }
  static void note_off(held_notes& h, midi2::ump::m2cvm::note_off const& noff) {
    if (noff.group() == 0) {
      h.notes[noff.channel()].reset(noff.note());
    }
  }
};

// The dispatcher's configuration: the context and one backend for each group of messages.
struct config {
  held_notes context;
  [[no_unique_address]] backend::utility_null<held_notes> utility;
  [[no_unique_address]] backend::system_null<held_notes> system;
  [[no_unique_address]] backend::m1cvm_null<held_notes> m1cvm;
  [[no_unique_address]] backend::data64_null<held_notes> data64;
  [[no_unique_address]] note_tracker m2cvm;
  [[no_unique_address]] backend::data128_null<held_notes> data128;
  [[no_unique_address]] backend::stream_null<held_notes> stream;
  [[no_unique_address]] backend::flex_data_null<held_notes> flex;
};

}  // end anonymous namespace

int main() {
  midi2::ump::ump_dispatcher dispatcher{config{}};

  // Note On for notes 60, 64, and 67 on channel 2, then Note Off for note 64.
  for (std::uint32_t const word :
       {0x40923C00U, 0xC0000000U, 0x40924000U, 0xC0000000U, 0x40924300U, 0xC0000000U, 0x40824000U, 0x00000000U}) {
    dispatcher.dispatch(word);
  }

  auto const& held = dispatcher.config().context.notes[2];
  std::cout << std::format("{} notes held on channel 2:", held.count());
  for (auto note = 0U; note < held.size(); ++note) {
    if (held.test(note)) {
      std::cout << ' ' << note;
    }
  }
  std::cout << '\n';
}
