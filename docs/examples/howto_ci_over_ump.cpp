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
#include "midi2/ump/ump_dispatcher.hpp"
#include "midi2/ump/ump_types.hpp"

using namespace midi2::literals;  // for the _b7 and _b28 literals

namespace {

namespace ci = midi2::ci;
namespace d64 = midi2::ump::data64;

constexpr auto ci_group = std::uint8_t{0};  // The group on which this device's function block carries MIDI-CI
constexpr auto my_muid = 0x0123456_b28;
constexpr auto max_sysex_size = std::size_t{128};

// Sends the bytes of a MIDI-CI message (without F0 and F7) as a sequence of SysEx7 packets.
template <typename Send> void send_sysex7(std::span<std::byte const> data, Send send) {
  constexpr auto max = d64::sysex7_in_1{}.max_size();
  auto const to_u8 = [](std::byte const b) { return std::to_integer<std::uint8_t>(b); };
  auto const packet = [&]<typename Packet>(Packet p, std::size_t const size) {
    midi2::ump::apply(p.group(ci_group).data(data.first(size), to_u8), send);
    data = data.subspan(size);
  };
  if (data.size() <= max) {
    return packet(d64::sysex7_in_1{}, data.size());
  }
  packet(d64::sysex7_start{}, max);
  while (data.size() > max) {
    packet(d64::sysex7_continue{}, max);
  }
  packet(d64::sysex7_end{}, data.size());
}

// Writes a MIDI-CI message to a buffer and sends it as SysEx7 packets.
template <typename Message, typename Send> void send_ci(ci::header const& header, Message const& message, Send send) {
  // One byte larger than the largest message, so that reaching the end of the buffer means that the message is too
  // large.
  std::array<std::byte, max_sysex_size + 1> buffer{};
  auto const last = ci::create_message(buffer.begin(), buffer.end(), header, message);
  if (last == buffer.end()) {
    std::cerr << "message is too large\n";
    return;
  }
  send_sysex7(std::span<std::byte const>{buffer.begin(), last}, send);
}

bool print_word(std::uint32_t const word) {
  std::cout << std::format("{:08X} ", word);
  return false;  // Continue with the next word
}

// A MIDI-CI responder that answers Inquiry: Endpoint. It holds a pointer to itself in the MIDI-CI dispatcher's
// context, so it can be neither copied nor moved.
class device {
public:
  device() {
    auto& config = ci_.config();
    config.system.on_check_muid(
        [](device*, std::uint8_t const group, ci::muid const m) { return group == ci_group && m == my_muid; });
    config.management.on_endpoint(
        [](device* const self, ci::header const& h, ci::endpoint const& e) { self->endpoint(h, e); });
  }
  device(device const&) = delete;
  device(device&&) = delete;
  device& operator=(device const&) = delete;
  device& operator=(device&&) = delete;
  ~device() noexcept = default;

  // Passes the data bytes of a SysEx7 packet to the MIDI-CI dispatcher.
  template <typename Packet> void sysex7(Packet const& packet, bool const start) {
    if (packet.group() != ci_group) {
      return;
    }
    if (start) {
      ci_.reset();  // A new System Exclusive message begins
    }
    for (std::uint8_t const b : packet) {
      ci_.dispatch(std::byte{b});
    }
  }

private:
  using ci_config = ci::function_config<device*, max_sysex_size>;
  ci::ci_dispatcher<ci_config> ci_{0x7F_b7, ci_group, ci_config{this}};

  void endpoint(ci::header const& h, ci::endpoint const& e) const {
    std::cout << std::format("Inquiry: Endpoint from MUID 0x{:X}, status {}\n", h.remote_muid, e.status);
    if (e.status != 0_b7) {
      return;  // Only status 0 (Product Instance ID) is supported
    }
    static constexpr std::array id{'M'_b7, 'I'_b7, 'D'_b7, 'I'_b7, '2'_b7, '-'_b7, '1'_b7};
    ci::header const reply_header{
        .device_id = 0x7F_b7, .version = 2_b7, .remote_muid = my_muid, .local_muid = h.remote_muid};
    std::cout << "reply: ";
    send_ci(reply_header, ci::endpoint_reply{.status = 0_b7, .information = id}, print_word);
    std::cout << '\n';
  }
};

}  // end anonymous namespace

int main() {
  device dev;
  // The context is a pointer to dev, which can't be copied into the dispatcher.
  auto ump = midi2::ump::make_ump_function_dispatcher(&dev);
  ump.config()
      .data64.on_sysex7_in_1([](device* const d, d64::sysex7_in_1 const& p) { d->sysex7(p, true); })
      .on_sysex7_start([](device* const d, d64::sysex7_start const& p) { d->sysex7(p, true); })
      .on_sysex7_continue([](device* const d, d64::sysex7_continue const& p) { d->sysex7(p, false); })
      .on_sysex7_end([](device* const d, d64::sysex7_end const& p) { d->sysex7(p, false); });

  // Another device (MUID 0x0567890) sends Inquiry: Endpoint to this one.
  ci::header const inquiry{.device_id = 0x7F_b7, .version = 2_b7, .remote_muid = 0x0567890_b28, .local_muid = my_muid};
  std::vector<std::uint32_t> words;
  send_ci(inquiry, ci::endpoint{.status = 0_b7}, [&words](std::uint32_t const word) {
    words.push_back(word);
    return false;  // Continue with the next word
  });
  std::cout << "inquiry: ";
  for (auto const word : words) {
    print_word(word);
  }
  std::cout << '\n';
  for (auto const word : words) {
    ump.dispatch(word);
  }
}
