// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstdint>
#include <format>
#include <iostream>
#include <span>

#include "midi2/ump/ump_dispatcher.hpp"

namespace {

// State shared by the message handlers.
struct context {
  unsigned messages = 0;
};

}  // end anonymous namespace

int main() {
  auto dispatcher = midi2::ump::make_ump_function_dispatcher(context{});
  dispatcher.config()
      .m2cvm
      .on_note_on([](context& ctx, midi2::ump::m2cvm::note_on const& non) {
        ++ctx.messages;
        std::cout << std::format("note on:  channel={} note={} velocity={:#x}\n", non.channel(), non.note(),
                                 non.velocity());
      })
      .on_note_off([](context& ctx, midi2::ump::m2cvm::note_off const& noff) {
        ++ctx.messages;
        std::cout << std::format("note off: channel={} note={}\n", noff.channel(), noff.note());
      });
  dispatcher.config().m1cvm.on_program_change([](context& ctx, midi2::ump::m1cvm::program_change const& pc) {
    ++ctx.messages;
    std::cout << std::format("program change: channel={} program={}\n", pc.channel(), pc.program());
  });
  // Called for messages with a reserved message type or status.
  dispatcher.config().utility.on_unknown([](context&, std::span<std::uint32_t> const words) {
    std::cout << std::format("unknown message: {:08X}\n", words[0]);
  });

  // The words written by the first program, then a UMP Stream message (which has no handler, so is ignored), a Note
  // Off, and a message with a reserved message type (0x6).
  for (std::uint32_t const word : {0x40923C00U, 0xC0000000U, 0x40924000U, 0xC0000000U, 0x20C20500U, 0xF0000000U,
                                   0x00000000U, 0x00000000U, 0x00000000U, 0x40824000U, 0x00000000U, 0x60000000U}) {
    dispatcher.dispatch(word);
  }
  std::cout << std::format("{} messages handled\n", dispatcher.config().context.messages);
}
