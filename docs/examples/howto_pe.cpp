// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "midi2/ci/ci_create_message.hpp"
#include "midi2/ci/ci_dispatcher.hpp"

using namespace midi2::literals;  // for the _b7, _b14, and _b28 literals
using namespace std::string_view_literals;

namespace {

namespace ci = midi2::ci;
namespace pe = midi2::ci::property_exchange;

constexpr auto responder_muid = 0x0123456_b28;
constexpr auto initiator_muid = 0x0567890_b28;

// The responder: builds a Reply to Get Property Data carrying a DeviceInfo resource.
std::vector<std::byte> make_get_reply(ci::b7 const request_id) {
  constexpr auto header = R"({"status":200})"sv;
  constexpr auto data = R"({"manufacturerId":[125,0,0],"familyId":[1,0],"modelId":[2,0],"versionId":[1,0,0,0]})"sv;
  auto const reply = pe::get_reply::make(pe::chunk_info{.number_of_chunks = 1_b14, .chunk_number = 1_b14}, request_id,
                                         header, std::as_bytes(std::span{data}));
  ci::header const hdr{
      .device_id = 0x7F_b7, .version = 2_b7, .remote_muid = responder_muid, .local_muid = initiator_muid};
  std::vector<std::byte> message;
  ci::create_message(std::back_inserter(message), ci::trivial_sentinel{}, hdr, reply);
  return message;
}

struct context {
  std::string text;  // The pieces of the header or property data received so far.
};

}  // end anonymous namespace

int main() {
  // The initiator: a dispatcher whose buffer holds only 16 bytes. Property Exchange messages are passed to the
  // property_exchange_stream handlers in pieces.
  using config = ci::function_stream_config<context, 16>;
  ci::ci_dispatcher<config> dispatcher{0x7F_b7, std::uint8_t{0}, config{}};
  auto& c = dispatcher.config();
  c.system.on_check_muid([](context&, std::uint8_t, ci::muid const m) { return m == initiator_muid; });
  c.system.on_buffer_overflow([](context&) { std::cout << "buffer overflow\n"; });
  c.property_exchange_stream
      .on_start([](context& ctx, ci::header const&, pe::stream_start const& s) {
        std::cout << std::format("start: request {}, {} bytes of header\n", s.request, s.header_length);
        ctx.text.clear();
      })
      .on_header_data([](context& ctx, ci::header const&, std::span<char const> const piece) {
        std::cout << std::format("  header piece of {} bytes\n", piece.size());
        ctx.text.append(piece.begin(), piece.end());
      })
      .on_chunk([](context& ctx, ci::header const&, pe::stream_chunk const& chunk) {
        std::cout << std::format("header: {}\nchunk {} of {}, {} bytes of property data\n", ctx.text,
                                 chunk.chunk.chunk_number, chunk.chunk.number_of_chunks, chunk.data_length);
        ctx.text.clear();
      })
      .on_property_data([](context& ctx, ci::header const&, std::span<std::byte const> const piece) {
        std::cout << std::format("  property data piece of {} bytes\n", piece.size());
        for (auto const b : piece) {
          ctx.text.push_back(static_cast<char>(b));
        }
      })
      .on_end([](context& ctx, ci::header const&) { std::cout << std::format("property data: {}\nend\n", ctx.text); })
      .on_abort([](context&, ci::header const&) { std::cout << "abort\n"; });

  dispatcher.reset();
  for (auto const b : make_get_reply(1_b7)) {
    dispatcher.dispatch(b);
  }
}
