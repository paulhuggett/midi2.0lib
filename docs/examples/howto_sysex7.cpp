// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <algorithm>
#include <cstdint>
#include <format>
#include <iostream>
#include <span>
#include <vector>

#include "midi2/ump/ump_dispatcher.hpp"
#include "midi2/ump/ump_types.hpp"

namespace {

namespace d64 = midi2::ump::data64;

// Splits the bytes of a System Exclusive message (without F0 and F7) into SysEx7 packets and calls send() for each
// 32-bit word.
template <typename Send> void send_sysex7(std::uint8_t const group, std::span<std::uint8_t const> data, Send send) {
  constexpr auto max = d64::sysex7_in_1{}.max_size();  // 6 bytes per packet
  auto const send_packet = [&]<typename Packet>(Packet packet, std::size_t const size) {
    packet.group(group).data(data.first(size));  // Copies the bytes and sets number_of_bytes
    midi2::ump::apply(packet, send);
    data = data.subspan(size);
  };
  if (data.size() <= max) {
    send_packet(d64::sysex7_in_1{}, data.size());
    return;
  }
  send_packet(d64::sysex7_start{}, max);
  while (data.size() > max) {
    send_packet(d64::sysex7_continue{}, max);
  }
  send_packet(d64::sysex7_end{}, data.size());
}

// Collects the bytes of the SysEx7 packets that make up a message.
struct receiver {
  std::vector<std::uint8_t> message;

  template <typename Packet> void append(Packet const& packet) {
    // A SysEx7 packet can be used as a range of its data bytes.
    std::ranges::copy(packet, std::back_inserter(message));
  }
  void complete() {
    std::cout << std::format("\nreceived {} bytes:", message.size());
    for (auto const b : message) {
      std::cout << std::format(" {:02X}", b);
    }
    std::cout << '\n';
    message.clear();
  }
};

}  // end anonymous namespace

int main() {
  auto dispatcher = midi2::ump::make_ump_function_dispatcher(receiver{});
  dispatcher.config()
      .data64
      .on_sysex7_in_1([](receiver& r, d64::sysex7_in_1 const& p) {
        r.message.clear();
        r.append(p);
        r.complete();
      })
      .on_sysex7_start([](receiver& r, d64::sysex7_start const& p) {
        r.message.clear();
        r.append(p);
      })
      .on_sysex7_continue([](receiver& r, d64::sysex7_continue const& p) { r.append(p); })
      .on_sysex7_end([](receiver& r, d64::sysex7_end const& p) {
        r.append(p);
        r.complete();
      });

  // A Universal Non-Real Time Identity Reply: 14 bytes, which need three packets.
  std::vector<std::uint8_t> const identity{0x7E, 0x7F, 0x06, 0x02, 0x7D, 0x01, 0x00,
                                           0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
  send_sysex7(0, identity, [&dispatcher](std::uint32_t const word) {
    std::cout << std::format("{:08X} ", word);
    dispatcher.dispatch(word);
    return false;  // Continue with the next word
  });
}
