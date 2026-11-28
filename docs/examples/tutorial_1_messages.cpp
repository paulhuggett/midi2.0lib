// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstdint>
#include <format>
#include <iostream>
#include <system_error>
#include <tuple>

#include "midi2/ump/ump_types.hpp"

namespace {

// Stands in for a real transport, such as USB or a network connection.
std::error_code transmit(std::uint32_t const word) {
  std::cout << std::format("{:08X} ", word);
  return {};
}

}  // end anonymous namespace

int main() {
  // A MIDI 2.0 Note On: group 0, channel 2, middle C, velocity 0xC000.
  auto non = midi2::ump::m2cvm::note_on{}.group(0).channel(2).note(60).velocity(0xC000);
  std::cout << std::format("note={} velocity={:#x}\n", non.note(), non.velocity());

  // The message is two 32-bit words. apply() passes each one to transmit().
  std::cout << std::format("{} words: ", std::tuple_size_v<decltype(non)>);
  if (std::error_code const err = midi2::ump::apply(non, transmit)) {
    std::cerr << "transmit failed: " << err.message() << '\n';
    return 1;
  }
  std::cout << '\n';

  // Change one field and send the message again.
  non.note(64);
  std::cout << "2 words: ";
  midi2::ump::apply(non, transmit);
  std::cout << '\n';

  // The words can also be reached as the elements of a tuple.
  auto const [w0, w1] = non;
  std::cout << std::format("w0={:08X} w1={:08X}\n", static_cast<std::uint32_t>(w0), static_cast<std::uint32_t>(w1));

  // A MIDI 1.0 Channel Voice message is a single word.
  auto const pc = midi2::ump::m1cvm::program_change{}.group(0).channel(2).program(5);
  std::cout << "1 word: ";
  midi2::ump::apply(pc, transmit);
  std::cout << '\n';
}
