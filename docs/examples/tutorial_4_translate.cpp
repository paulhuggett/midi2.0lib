// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <vector>

#include "midi2/bytestream/bytestream_to_ump.hpp"
#include "midi2/bytestream/ump_to_bytestream.hpp"
#include "midi2/translator.hpp"
#include "midi2/ump/ump_to_midi1.hpp"
#include "midi2/ump/ump_to_midi2.hpp"

using namespace midi2::literals;  // for the _b (std::byte) literal

namespace {

// Passes each value of the input to a translator and collects the translator's output.
template <typename T>
  requires midi2::translator<typename T::input_type, typename T::output_type, T>
std::vector<typename T::output_type> translate(T& translator, std::vector<typename T::input_type> const& input) {
  std::vector<typename T::output_type> output;
  for (auto const value : input) {
    translator.push(value);
    while (!translator.empty()) {
      output.push_back(translator.pop());
    }
  }
  return output;
}

void show(char const* title, std::vector<std::uint32_t> const& words) {
  std::cout << title;
  for (auto const w : words) {
    std::cout << std::format(" {:08X}", w);
  }
  std::cout << '\n';
}
void show(char const* title, std::vector<std::byte> const& bytes) {
  std::cout << title;
  for (auto const b : bytes) {
    std::cout << std::format(" {:02X}", std::to_integer<unsigned>(b));
  }
  std::cout << '\n';
}

}  // end anonymous namespace

int main() {
  // Note On (with running status for the second note) and Control Change 7 (volume) on channel 1.
  std::vector const midi1{0x90_b, 0x3C_b, 0x7F_b, 0x40_b, 0x40_b, 0xB0_b, 0x07_b, 0x64_b};
  show("MIDI 1.0 bytestream: ", midi1);

  midi2::bytestream::to_ump bytestream_to_ump{0};
  auto const ump1 = translate(bytestream_to_ump, midi1);
  show("MIDI 1.0 UMP:        ", ump1);

  midi2::ump::ump_to_midi2 midi1_to_midi2{0};
  auto const ump2 = translate(midi1_to_midi2, ump1);
  show("MIDI 2.0 UMP:        ", ump2);

  // And back again.
  midi2::ump::to_midi1 midi2_to_midi1;
  auto const back1 = translate(midi2_to_midi1, ump2);
  show("MIDI 1.0 UMP:        ", back1);

  midi2::bytestream::to_bytestream ump_to_bytestream;
  show("MIDI 1.0 bytestream: ", translate(ump_to_bytestream, back1));
}
