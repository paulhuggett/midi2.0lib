// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstdint>
#include <format>
#include <iostream>
#include <string_view>

#include "midi2/ump/ump_dispatcher.hpp"

using namespace std::string_view_literals;

namespace {

namespace backend = midi2::ump::dispatcher_backend;

struct state {};

// Two sets of handlers for MIDI 2.0 Channel Voice messages. Each derives from m2cvm_base<>, which ignores every
// message, and overrides only the functions it needs.
struct logger final : backend::m2cvm_base<state> {
  void note_on(state&, midi2::ump::m2cvm::note_on const& non) override {
    std::cout << std::format("note on {}\n", non.note());
  }
};
struct counter final : backend::m2cvm_base<state> {
  unsigned notes = 0;
  void note_on(state&, midi2::ump::m2cvm::note_on const&) override { ++notes; }
  void note_off(state&, midi2::ump::m2cvm::note_off const&) override { --notes; }
};

// The configuration refers to the MIDI 2.0 Channel Voice handlers through a reference to their common base class, so
// the handlers can be chosen at run time.
struct config {
  explicit config(backend::m2cvm_pure<state>& handlers) : m2cvm{handlers} {}

  [[no_unique_address]] state context;
  [[no_unique_address]] backend::utility_null<state> utility;
  [[no_unique_address]] backend::system_null<state> system;
  [[no_unique_address]] backend::m1cvm_null<state> m1cvm;
  [[no_unique_address]] backend::data64_null<state> data64;
  backend::m2cvm_pure<state>& m2cvm;
  [[no_unique_address]] backend::data128_null<state> data128;
  [[no_unique_address]] backend::stream_null<state> stream;
  [[no_unique_address]] backend::flex_data_null<state> flex;
};

void play(backend::m2cvm_pure<state>& handlers) {
  midi2::ump::ump_dispatcher dispatcher{config{handlers}};
  // Note On for notes 60 and 64, then Note Off for note 60.
  for (std::uint32_t const word : {0x40903C00U, 0x80000000U, 0x40904000U, 0x80000000U, 0x40803C00U, 0x00000000U}) {
    dispatcher.dispatch(word);
  }
}

}  // end anonymous namespace

int main(int argc, char const* argv[]) {
  logger log;
  counter count;
  // Use the logger unless the program's first argument is "count".
  bool const counting = argc > 1 && argv[1] == "count"sv;
  play(counting ? static_cast<backend::m2cvm_pure<state>&>(count) : log);
  if (counting) {
    std::cout << std::format("{} note held\n", count.notes);
  }
}
