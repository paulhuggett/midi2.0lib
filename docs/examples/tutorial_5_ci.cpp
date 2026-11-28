// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <span>
#include <vector>

#include "midi2/ci/ci_create_message.hpp"
#include "midi2/ci/ci_dispatcher.hpp"

using namespace midi2::literals;  // for the _b, _b7, _b14, and _b28 literals

namespace {

constexpr auto my_muid = 0x0A1B2C3_b28;  // A real device chooses its MUID at random.

// Writes the bytes of a System Exclusive message (without F0 and F7) as hexadecimal.
void show(std::span<std::byte const> const bytes) {
  for (auto const b : bytes) {
    std::cout << std::format("{:02X} ", std::to_integer<unsigned>(b));
  }
  std::cout << '\n';
}

// Builds a Reply to Discovery addressed to the device that sent the Discovery message.
std::vector<std::byte> discovery_reply(midi2::ci::header const& discovery_header) {
  midi2::ci::header const header{
      .device_id = 0x7F_b7,  // To/from Function Block
      .version = 2_b7,
      .remote_muid = my_muid,                      // The source: this device
      .local_muid = discovery_header.remote_muid,  // The destination: the device that sent the Discovery
  };
  midi2::ci::discovery_reply const reply{
      .manufacturer = {0x7D_b7, 0_b7, 0_b7},  // 7D: the manufacturer ID for prototyping and private use
      .family = 0x0001_b14,
      .model = 0x0002_b14,
      .version = {1_b7, 0_b7, 0_b7, 0_b7},
      .capability = 0x0C_b7,  // Profile Configuration and Property Exchange
      .max_sysex_size = 512_b28,
      .output_path_id = 0_b7,
      .function_block = 0_b7,
  };
  std::vector<std::byte> message;
  midi2::ci::create_message(std::back_inserter(message), midi2::ci::trivial_sentinel{}, header, reply);
  return message;
}

struct context {
  std::vector<std::byte> reply;  // The message to be sent in reply, if any.
};

}  // end anonymous namespace

int main() {
  // A dispatcher whose handlers are std::function<>s, with a 256-byte buffer for each message.
  auto dispatcher = midi2::ci::make_function_dispatcher<context, 256>(0x7F_b7, std::uint8_t{0});
  auto& config = dispatcher.config();
  // Accept messages addressed to this device's MUID. Messages sent to the broadcast MUID are always accepted.
  config.system.on_check_muid([](context&, std::uint8_t /*group*/, midi2::ci::muid const m) { return m == my_muid; });
  config.management.on_discovery([](context& ctx, midi2::ci::header const& h, midi2::ci::discovery const& d) {
    std::cout << std::format("Discovery from MUID 0x{:X}: manufacturer {:02X} {:02X} {:02X}, max SysEx size {}\n",
                             h.remote_muid, d.manufacturer[0], d.manufacturer[1], d.manufacturer[2], d.max_sysex_size);
    ctx.reply = discovery_reply(h);
  });

  // A Discovery message from the device whose MUID is 0x0567890, sent to the broadcast MUID.
  constexpr std::array discovery{0x7E_b, 0x7F_b, 0x0D_b, 0x70_b, 0x02_b, 0x10_b, 0x71_b, 0x59_b, 0x02_b, 0x7F_b,
                                 0x7F_b, 0x7F_b, 0x7F_b, 0x12_b, 0x23_b, 0x34_b, 0x79_b, 0x2E_b, 0x5D_b, 0x56_b,
                                 0x01_b, 0x00_b, 0x00_b, 0x00_b, 0x7F_b, 0x00_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b};
  dispatcher.reset();  // Prepare for a new message
  for (auto const b : discovery) {
    dispatcher.dispatch(b);
  }
  std::cout << "Reply: ";
  show(config.context.reply);
}
