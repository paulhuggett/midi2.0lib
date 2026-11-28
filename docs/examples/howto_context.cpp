// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <atomic>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <utility>

#include "midi2/ump/ump_dispatcher.hpp"

namespace {

namespace backend = midi2::ump::dispatcher_backend;

// Statistics that another thread might read while messages are dispatched. std::atomic<> can be neither copied nor
// moved, so neither can this struct.
struct statistics {
  explicit statistics(unsigned const first) : notes{first} {}
  std::atomic<unsigned> notes;
};

struct note_counter : backend::m2cvm_null<statistics> {
  static void note_on(statistics& s, midi2::ump::m2cvm::note_on const&) { ++s.notes; }
};

struct config {
  statistics context{0};
  [[no_unique_address]] backend::utility_null<statistics> utility;
  [[no_unique_address]] backend::system_null<statistics> system;
  [[no_unique_address]] backend::m1cvm_null<statistics> m1cvm;
  [[no_unique_address]] backend::data64_null<statistics> data64;
  [[no_unique_address]] note_counter m2cvm;
  [[no_unique_address]] backend::data128_null<statistics> data128;
  [[no_unique_address]] backend::stream_null<statistics> stream;
  [[no_unique_address]] backend::flex_data_null<statistics> flex;
};

constexpr std::uint32_t note_on[] = {0x40903C00U, 0x80000000U};

}  // end anonymous namespace

int main() {
  // 1. Construct the context in place. The arguments after std::in_place are passed to the constructor of
  // statistics. The function_config<> handlers are std::function<>s.
  midi2::ump::ump_dispatcher<midi2::ump::function_config<statistics>> d1{std::in_place, 10U};
  d1.config().m2cvm.on_note_on([](statistics& s, midi2::ump::m2cvm::note_on const&) { ++s.notes; });
  for (auto const word : note_on) {
    d1.dispatch(word);
  }
  std::cout << std::format("in place: notes={}\n", d1.config().context.notes.load());

  // 2. Keep the configuration outside the dispatcher and give the dispatcher a reference to it.
  config cfg;
  midi2::ump::ump_dispatcher d2{std::ref(cfg)};
  for (auto const word : note_on) {
    d2.dispatch(word);
  }
  std::cout << std::format("by reference: notes={}\n", cfg.context.notes.load());
}
