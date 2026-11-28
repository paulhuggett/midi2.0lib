//===-- Demo MIDI 2.0 Workbench Device ----------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2026 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

// A MIDI 2.0 device, built from the midi2 library, which the MIDI 2.0 Workbench
// (https://github.com/midi2-dev/MIDI2.0Workbench) can connect to and test.
//
// The program creates a CoreMIDI virtual source and destination of the same name which use the MIDI 2.0
// protocol. The Workbench lists such a pair as a UMP endpoint. The device then answers:
//
// - UMP Stream messages: Endpoint Discovery (with the Endpoint Info, Device Identity, Endpoint Name, Product
//   Instance Id and Stream Configuration Notifications), Stream Configuration Request, and Function Block
//   Discovery (with the Function Block Info and Name Notifications).
// - MIDI-CI messages carried in SysEx7 on the function block's group: Discovery, Invalidate MUID, Inquiry: Endpoint,
//   Profile Inquiry, Property Exchange Capabilities, and Property Exchange Get for the ResourceList and DeviceInfo
//   resources. The header of each Get request is parsed by the peejay JSON parser
//   (https://github.com/paulhuggett/peejay).
//
// When its MUID is invalidated, or another device's Discovery uses the same MUID, the device picks a new MUID and
// broadcasts a Discovery message so that other devices can connect to it again. It invalidates its MUID when it quits.
//
// Channel voice messages are logged. Run the program, then start (or refresh) the Workbench. Press Ctrl-C to quit.
//
// Usage: demo_workbench_device [-debug] [endpoint-name]
//
// With -debug, the program also logs each UMP message that it receives or sends as hexadecimal words, and the bytes of
// each System Exclusive message.

// Apple
#include <CoreFoundation/CoreFoundation.h>
#include <CoreMIDI/CoreMIDI.h>
#include <dispatch/dispatch.h>

// midi2 library
#include <midi2/ci/ci_create_message.hpp>
#include <midi2/ci/ci_dispatcher.hpp>
#include <midi2/ci/ci_types.hpp>
#include <midi2/ump/ump_dispatcher.hpp>
#include <midi2/ump/ump_types.hpp>

// peejay JSON parser
#include <peejay/dom.hpp>
#include <peejay/json.hpp>

// Standard library
#include <algorithm>
#include <array>
#include <concepts>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <expected>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <random>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace midi2::literals;
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace std::string_view_literals;

namespace {

// The identity of the device. 0x7D is the manufacturer ID reserved for non-commercial and educational use.
constexpr auto manufacturer_id = std::array{std::uint8_t{0x7D}, std::uint8_t{0x00}, std::uint8_t{0x00}};
constexpr auto software_revision =
    std::array{std::uint8_t{0x00}, std::uint8_t{0x00}, std::uint8_t{0x00}, std::uint8_t{0x01}};
constexpr auto default_endpoint_name = "midi2lib Demo Device"sv;
constexpr auto product_instance_id = "midi2lib-0001"sv;
constexpr auto function_block_name = "midi2lib Block"sv;

// The device has a single bidirectional function block which spans one group.
constexpr auto function_block_number = std::uint8_t{0};
constexpr auto function_block_group = std::uint8_t{0};

// The largest System Exclusive message that the device will receive.
constexpr auto max_sysex_size = std::size_t{512};

// The UMP stream format field values (section 7.1 of the UMP specification).
enum class stream_format : std::uint8_t { complete = 0x0, start = 0x1, cont = 0x2, end = 0x3 };

// The protocol values used by the Stream Configuration messages.
constexpr auto midi2_protocol = std::uint8_t{0x02};

// MIDI-CI Device ID 0x7F addresses the whole function block.
constexpr auto function_block_device_id = 0x7F_b7;

// The Property Exchange resources that the device implements.
constexpr auto resource_list_json = R"([{"resource":"DeviceInfo"}])"sv;
constexpr auto device_info_json =
    R"({"manufacturerId":[125,0,0],"manufacturer":"midi2lib","familyId":[0,0],"family":"Demo",)"
    R"("modelId":[0,0],"model":"Workbench Device","versionId":[0,0,0,1],"version":"0.0.1"})"sv;
constexpr auto status_ok_json = R"({"status":200})"sv;
constexpr auto status_bad_request_json = R"({"status":400})"sv;
constexpr auto status_not_found_json = R"({"status":404})"sv;

/// The text bytes carried by one UMP Stream message.
///
/// \tparam ChunkSize  The number of text bytes that the message carries.
template <std::size_t ChunkSize> using text_chunk = std::array<std::uint8_t, ChunkSize>;

/// Calls a function for each of the pieces that a text string is split into when it is carried by a sequence of UMP
/// Stream messages.
///
/// \tparam ChunkSize  The number of text bytes that each UMP Stream message carries. Must not be zero.
/// \tparam Function  A function which accepts a stream_format value and a text_chunk<ChunkSize>.
/// \param text  The text to be sent. An empty string is sent as a single message.
/// \param function  The function called for each piece of the text.
template <std::size_t ChunkSize, std::invocable<stream_format, text_chunk<ChunkSize> const&> Function>
  requires(ChunkSize > 0)
void for_each_text_chunk(std::string_view text, Function function) {
  auto const chunks = std::max(std::size_t{1}, (text.size() + ChunkSize - 1) / ChunkSize);
  for (auto index = std::size_t{0}; index < chunks; ++index) {
    text_chunk<ChunkSize> bytes{};
    auto const piece = text.substr(std::min(text.size(), index * ChunkSize), ChunkSize);
    std::ranges::transform(piece, bytes.begin(), [](char const c) { return static_cast<std::uint8_t>(c); });
    auto format = stream_format::cont;
    if (chunks == 1) {
      format = stream_format::complete;
    } else if (index == 0) {
      format = stream_format::start;
    } else if (index == chunks - 1) {
      format = stream_format::end;
    }
    function(format, bytes);
  }
}

/// Returns the data bytes carried by a SysEx7 message.
///
/// \tparam Sysex7  One of the midi2::ump::data64 SysEx7 message types.
/// \param message  The SysEx7 message.
/// \param out  Storage for the data bytes. The returned span refers to it.
/// \returns  The data bytes of the message: at most six.
template <midi2::ump::data64::sysex7_message Sysex7>
std::span<std::byte const> sysex7_data(Sysex7 const& message, std::array<std::byte, 6>& out) {
  out = std::array{std::byte{message.data0()}, std::byte{message.data1()}, std::byte{message.data2()},
                   std::byte{message.data3()}, std::byte{message.data4()}, std::byte{message.data5()}};
  return std::span{out}.first(std::min(std::size_t{message.number_of_bytes()}, out.size()));
}

/// Creates a SysEx7 message which carries up to six bytes of data.
///
/// \tparam Sysex7  One of the midi2::ump::data64 SysEx7 message types.
/// \param group  The UMP group on which the message is sent.
/// \param data  The data bytes carried by the message: at most six.
/// \returns  The SysEx7 message.
template <midi2::ump::data64::sysex7_message Sysex7>
Sysex7 make_sysex7(std::uint8_t const group, std::span<std::byte const> const data) {
  return Sysex7{}.group(group).data(
      data | std::views::transform([](std::byte const b) { return std::to_integer<std::uint8_t>(b); }));
}

/// Finds the value of the "resource" member of a Property Exchange request header such as {"resource":"DeviceInfo"}.
/// The header may have other members, such as "resId" or "offset": they are ignored.
///
/// \param header  The header data from a Property Exchange request.
/// \returns  The value of the "resource" member, or a description of the error if the header is not valid JSON or is
///   not an object with a string "resource" member.
std::expected<std::u8string, std::string> resource_name(std::span<char const> const header) {
  peejay::parser<peejay::dom::dom<>> parser;
  auto const root =
      parser.input(header | std::views::transform([](char const c) { return static_cast<char8_t>(c); })).eof();
  if (auto const err = parser.last_error()) {
    return std::unexpected{err.message()};
  }
  if (root) {
    if (auto const resource = root->get_object_element<std::u8string>(u8"resource")) {
      return resource->get();
    }
  }
  return std::unexpected{"the header is not an object with a string \"resource\" member"};
}

/// Formats UMP words as hexadecimal.
///
/// \param words  The words to be formatted.
/// \returns  The words, each as eight hexadecimal digits separated by spaces.
std::string hex_words(std::span<std::uint32_t const> const words) {
  std::string result;
  for (auto const word : words) {
    result += std::format("{}{:08X}", result.empty() ? "" : " ", word);
  }
  return result;
}

/// Formats bytes as hexadecimal, 16 bytes to a line.
///
/// \param bytes  The bytes to be formatted.
/// \param indent  The text which starts each line after the first.
/// \returns  The bytes, each as two hexadecimal digits separated by spaces.
std::string hex_bytes(std::span<std::byte const> const bytes, std::string_view const indent) {
  constexpr auto bytes_per_line = std::size_t{16};
  std::string result;
  for (auto index = std::size_t{0}; index < bytes.size(); ++index) {
    if (index > 0) {
      result += index % bytes_per_line == 0 ? std::format("\n{}", indent) : std::string{" "};
    }
    result += std::format("{:02X}", std::to_integer<unsigned>(bytes[index]));
  }
  return result;
}

/// Returns a random MUID. Values from 0x0FFFFF00 are reserved.
///
/// \param avoid  A MUID that the result must differ from: the MUID being replaced, if any.
/// \returns  A new MUID.
midi2::ci::muid make_muid(std::optional<midi2::ci::muid> const avoid = std::nullopt) {
  std::random_device random;
  std::uniform_int_distribution<std::uint32_t> distribution{0U, 0x0FFFFEFFU};
  for (;;) {
    if (auto const result = midi2::ci::muid{distribution(random)}; result != avoid) {
      return result;
    }
  }
}

/// A MIDI 2.0 device which answers UMP Stream and MIDI-CI messages received from a CoreMIDI destination by sending
/// its replies from a CoreMIDI source.
class device {
public:
  /// \param source  The CoreMIDI source from which the device sends its messages.
  /// \param endpoint_name  The name that the device reports in its Endpoint Name Notification.
  /// \param muid  The device's MIDI-CI MUID.
  /// \param debug  If true, the device logs every UMP message and System Exclusive message that it receives or sends.
  device(MIDIEndpointRef const source, std::string endpoint_name, midi2::ci::muid const muid, bool const debug)
      : source_{source}, endpoint_name_{std::move(endpoint_name)}, muid_{muid}, debug_{debug} {
    product_instance_id_.resize(product_instance_id.size());
    std::ranges::transform(product_instance_id, product_instance_id_.begin(),
                           [](char const c) { return midi2::ci::b7{static_cast<std::uint8_t>(c)}; });
    this->setup_ump();
    this->setup_ci();
  }
  device(device const&) = delete;
  device(device&&) = delete;
  ~device() noexcept = default;
  device& operator=(device const&) = delete;
  device& operator=(device&&) = delete;

  /// Processes a list of UMP packets received by the destination.
  void receive(MIDIEventList const* const list) {
    auto const* packet = &list->packet[0];
    for (auto index = UInt32{0}; index < list->numPackets; ++index) {
      for (auto const word : std::span{packet->words, packet->wordCount}) {
        if (debug_) {
          this->log_received_word(word);
        }
        ump_.dispatch(word);
      }
      packet = MIDIEventPacketNext(packet);
    }
  }

  [[nodiscard]] constexpr midi2::ci::muid muid() const noexcept { return muid_; }

  /// Broadcasts an Invalidate MUID message for the device's own MUID. Call this when the device is shutting down,
  /// after the destination has been disposed so that no messages are being received.
  void invalidate_own_muid() const;

private:
  using ump_config = midi2::ump::function_config<device*>;
  using ci_config = midi2::ci::function_config<device*, max_sysex_size>;

  MIDIEndpointRef source_;
  std::string endpoint_name_;
  midi2::ci::muid muid_;
  bool debug_;
  std::vector<midi2::ci::b7> product_instance_id_;
  // With debug logging, the words of the UMP message being received and the bytes of the System Exclusive message
  // being received.
  std::array<std::uint32_t, 4> rx_words_{};
  std::size_t rx_size_ = 0;
  std::vector<std::byte> rx_sysex_;
  midi2::ump::ump_dispatcher<ump_config> ump_{ump_config{this}};
  midi2::ci::ci_dispatcher<ci_config> ci_{function_block_device_id, function_block_group, ci_config{this}};

  void setup_ump();
  void setup_ci();

  // Debug logging
  void log_received_word(std::uint32_t word);

  // Sending
  void send_words(std::span<std::uint32_t const> words) const;
  /// Sends a UMP message from the source.
  /// \tparam Message  One of the midi2::ump message types.
  /// \param message  The message to be sent.
  template <midi2::ump::ump_message Message> void send(Message const& message) const {
    std::array<std::uint32_t, 4> words{};
    auto size = std::size_t{0};
    midi2::ump::apply(message, [&words, &size](std::uint32_t const word) {
      words.at(size++) = word;
      return false;
    });
    this->send_words(std::span{words}.first(size));
  }
  void send_sysex7(std::uint8_t group, std::span<std::byte const> data) const;
  /// Sends a MIDI-CI reply to the device that sent a request.
  /// \tparam Message  One of the midi2::ci message types.
  /// \param request  The header of the request being answered.
  /// \param device_id  The Device ID of the reply.
  /// \param message  The body of the reply.
  template <midi2::ci::ci_message Message>
  void send_ci(midi2::ci::header const& request, midi2::ci::b7 const device_id, Message const& message) const {
    this->send_ci(
        midi2::ci::header{
            .device_id = device_id,
            .version = request.version == 1_b7 ? 1_b7 : 2_b7,
            .remote_muid = muid_,
            .local_muid = request.remote_muid,
        },
        message);
  }
  /// Sends a MIDI-CI message that is not a reply to every device on the function block's group.
  /// \tparam Message  One of the midi2::ci message types.
  /// \param message  The body of the message.
  template <midi2::ci::ci_message Message> void broadcast_ci(Message const& message) const {
    this->send_ci(
        midi2::ci::header{
            .device_id = function_block_device_id,
            .version = 2_b7,
            .remote_muid = muid_,
            .local_muid = midi2::ci::broadcast_muid,
        },
        message);
  }
  /// Sends a MIDI-CI message.
  /// \tparam Message  One of the midi2::ci message types.
  /// \param header  The header of the message.
  /// \param message  The body of the message.
  template <midi2::ci::ci_message Message> void send_ci(midi2::ci::header const& header, Message const& message) const {
    // create_message() returns the end of the output range both when the message fills it exactly and when the message
    // doesn't fit. The spare byte means that a message of up to max_sysex_size bytes never reaches the end of the
    // buffer, so reaching it means that the message is too large.
    std::array<std::byte, max_sysex_size + 1> buffer{};
    auto const last = midi2::ci::create_message(buffer.begin(), buffer.end(), header, message);
    if (last == buffer.end()) {
      std::cerr << "MIDI-CI reply is too large\n";
      return;
    }
    auto const bytes = std::span<std::byte const>{buffer.begin(), last};
    if (debug_) {
      std::cout << std::format("-> SysEx ({} bytes): {}\n", bytes.size(), hex_bytes(bytes, "   "));
    }
    this->send_sysex7(function_block_group, bytes);
  }

  // UMP Stream
  void endpoint_discovery(midi2::ump::stream::endpoint_discovery const& message) const;
  void stream_configuration_request(midi2::ump::stream::jr_configuration_request const& message) const;
  void function_block_discovery(midi2::ump::stream::function_block_discovery const& message) const;
  /// Sends a text string as a sequence of UMP Stream messages, each of which carries a piece of the text.
  /// \tparam Message  The type of the UMP Stream messages: one which is accessible as an array of text bytes.
  /// \tparam SetText  A function which stores a piece of the text in a message.
  /// \param text  The text to be sent.
  /// \param set_text  The function called to store each piece of the text in its message.
  template <midi2::ump::ump_message Message,
            std::invocable<Message&, text_chunk<midi2::ump::max_size_v<Message>> const&> SetText>
  void send_text(std::string_view const text, SetText set_text) const {
    constexpr auto max_size = midi2::ump::max_size_v<Message>;
    for_each_text_chunk<max_size>(text, [this, &set_text](stream_format const format, text_chunk<max_size> const& c) {
      auto message = Message{}.format(std::to_underlying(format));
      std::invoke(set_text, message, c);
      this->send(message);
    });
  }
  void send_stream_configuration() const;
  void send_function_block_info() const;
  void send_function_block_name() const;

  // SysEx7
  /// Passes the data bytes of a SysEx7 message to the MIDI-CI dispatcher.
  /// \tparam Sysex7  One of the midi2::ump::data64 SysEx7 message types.
  /// \param message  The SysEx7 message.
  /// \param start  True if the message starts a new System Exclusive message.
  /// \param end  True if the message ends a System Exclusive message.
  template <midi2::ump::data64::sysex7_message Sysex7>
  void sysex7(Sysex7 const& message, bool const start, bool const end) {
    if (message.group() != function_block_group) {
      return;
    }
    if (start) {
      ci_.reset();
    }
    std::array<std::byte, 6> buffer{};
    auto const data = sysex7_data(message, buffer);
    if (debug_) {
      // Log the whole System Exclusive message before the final bytes reach the MIDI-CI dispatcher, so that the log
      // shows the request before the reply.
      if (start) {
        rx_sysex_.clear();
      }
      rx_sysex_.insert(rx_sysex_.end(), data.begin(), data.end());
      if (end) {
        std::cout << std::format("<- SysEx ({} bytes): {}\n", rx_sysex_.size(), hex_bytes(rx_sysex_, "   "));
      }
    }
    for (auto const b : data) {
      ci_.dispatch(b);
    }
  }

  // MIDI-CI
  void ci_discovery(midi2::ci::header const& header, midi2::ci::discovery const& discovery);
  void ci_discovery_reply(midi2::ci::header const& header) const;
  void ci_invalidate_muid(midi2::ci::header const& header, midi2::ci::invalidate_muid const& invalidate);
  void change_muid();
  void send_discovery() const;
  void ci_endpoint(midi2::ci::header const& header, midi2::ci::endpoint const& endpoint) const;
  void ci_profile_inquiry(midi2::ci::header const& header) const;
  void ci_pe_capabilities(midi2::ci::header const& header) const;
  void ci_pe_get(midi2::ci::header const& header, midi2::ci::property_exchange::get const& get) const;
};

void device::setup_ump() {
  auto& config = ump_.config();
  namespace stream = midi2::ump::stream;
  namespace d64 = midi2::ump::data64;
  config.stream
      .on_endpoint_discovery(
          [](device const* const self, stream::endpoint_discovery const& m) { self->endpoint_discovery(m); })
      .on_jr_configuration_request([](device const* const self, stream::jr_configuration_request const& m) {
        self->stream_configuration_request(m);
      })
      .on_function_block_discovery([](device const* const self, stream::function_block_discovery const& m) {
        self->function_block_discovery(m);
      });

  config.data64.on_sysex7_in_1([](device* self, d64::sysex7_in_1 const& m) { self->sysex7(m, true, true); })
      .on_sysex7_start([](device* self, d64::sysex7_start const& m) { self->sysex7(m, true, false); })
      .on_sysex7_continue([](device* self, d64::sysex7_continue const& m) { self->sysex7(m, false, false); })
      .on_sysex7_end([](device* self, d64::sysex7_end const& m) { self->sysex7(m, false, true); });

  config.m2cvm
      .on_note_on([](device*, midi2::ump::m2cvm::note_on const& m) {
        std::cout << std::format("Note on:  group={} channel={} note={} velocity={}\n", m.group(), m.channel(),
                                 m.note(), m.velocity());
      })
      .on_note_off([](device*, midi2::ump::m2cvm::note_off const& m) {
        std::cout << std::format("Note off: group={} channel={} note={} velocity={}\n", m.group(), m.channel(),
                                 m.note(), m.velocity());
      });
  config.m1cvm
      .on_note_on([](device*, midi2::ump::m1cvm::note_on const& m) {
        std::cout << std::format("MIDI 1.0 note on:  group={} channel={} note={} velocity={}\n", m.group(), m.channel(),
                                 m.note(), m.velocity());
      })
      .on_note_off([](device*, midi2::ump::m1cvm::note_off const& m) {
        std::cout << std::format("MIDI 1.0 note off: group={} channel={} note={} velocity={}\n", m.group(), m.channel(),
                                 m.note(), m.velocity());
      });
}

void device::setup_ci() {
  auto& config = ci_.config();
  // The broadcast MUID is always accepted. Otherwise, only messages addressed to this device are dispatched.
  config.system.on_check_muid(
      [](device const* const self, std::uint8_t, midi2::ci::muid const m) { return m == self->muid(); });
  config.system.on_buffer_overflow([](device*) { std::cerr << "MIDI-CI message is too large\n"; });
  using midi2::ci::header;
  namespace pe = midi2::ci::property_exchange;
  config.management
      .on_discovery(
          [](device* const self, header const& h, midi2::ci::discovery const& d) { self->ci_discovery(h, d); })
      .on_discovery_reply([](device const* const self, header const& h, midi2::ci::discovery_reply const&) {
        self->ci_discovery_reply(h);
      })
      .on_invalidate_muid(
          [](device* self, header const& h, midi2::ci::invalidate_muid const& i) { self->ci_invalidate_muid(h, i); })
      .on_endpoint(
          [](device const* const self, header const& h, midi2::ci::endpoint const& e) { self->ci_endpoint(h, e); });
  config.profile.on_inquiry([](device const* const self, header const& h) { self->ci_profile_inquiry(h); });
  config.property_exchange
      .on_capabilities(
          [](device const* const self, header const& h, pe::capabilities const&) { self->ci_pe_capabilities(h); })
      .on_get([](device const* const self, header const& h, pe::get const& g) { self->ci_pe_get(h, g); });
}

// Collects the words of each received UMP message and logs the message once it is complete.
void device::log_received_word(std::uint32_t const word) {
  rx_words_.at(rx_size_++) = word;
  auto const type = static_cast<midi2::ump::message_type>(rx_words_.at(0) >> 28U);
  if (rx_size_ >= midi2::ump::ump_message_size(type)) {
    std::cout << std::format("<- UMP: {}\n", hex_words(std::span{rx_words_}.first(rx_size_)));
    rx_size_ = 0;
  }
}

void device::send_words(std::span<std::uint32_t const> const words) const {
  if (debug_) {
    std::cout << std::format("-> UMP: {}\n", hex_words(words));
  }
  MIDIEventList list;
  auto* packet = MIDIEventListInit(&list, kMIDIProtocol_2_0);
  packet = MIDIEventListAdd(&list, sizeof(list), packet, 0, words.size(), words.data());
  if (packet == nullptr) {
    std::cerr << "Could not build a MIDI event list\n";
    return;
  }
  if (OSStatus const err = MIDIReceivedEventList(source_, &list); err != noErr) {
    std::cerr << std::format("MIDIReceivedEventList failed ({})\n", err);
  }
}

void device::send_sysex7(std::uint8_t const group, std::span<std::byte const> data) const {
  namespace d64 = midi2::ump::data64;
  constexpr auto max_bytes = std::size_t{6};
  if (data.size() <= max_bytes) {
    this->send(make_sysex7<d64::sysex7_in_1>(group, data));
    return;
  }
  this->send(make_sysex7<d64::sysex7_start>(group, data.first(max_bytes)));
  data = data.subspan(max_bytes);
  while (data.size() > max_bytes) {
    this->send(make_sysex7<d64::sysex7_continue>(group, data.first(max_bytes)));
    data = data.subspan(max_bytes);
  }
  this->send(make_sysex7<d64::sysex7_end>(group, data));
}

// UMP Stream
// ~~~~~~~~~~

void device::endpoint_discovery(midi2::ump::stream::endpoint_discovery const& message) const {
  std::cout << std::format("Endpoint Discovery: UMP v{}.{} filter=0x{:02X}\n", message.version_major(),
                           message.version_minor(), message.filter());
  using enum midi2::ump::stream::endpoint_discovery::filter_mask;
  if (message.filter_requests(endpoint_info)) {
    this->send(midi2::ump::stream::endpoint_info_notification{}
                   .version_major(1U)
                   .version_minor(1U)
                   .static_function_blocks(1U)
                   .number_function_blocks(1U)
                   .midi2_protocol_capability(1U)
                   .midi1_protocol_capability(0U)
                   .receive_jr_timestamp_capability(0U)
                   .transmit_jr_timestamp_capability(0U));
  }
  if (message.filter_requests(device_identity)) {
    this->send(midi2::ump::stream::device_identity_notification{}
                   .dev_manuf_sysex_id(manufacturer_id)
                   .device_family_lsb(0U)
                   .device_family_msb(0U)
                   .device_family_model_lsb(0U)
                   .device_family_model_msb(0U)
                   .sw_revision(software_revision));
  }
  if (message.filter_requests(endpoint_name)) {
    this->send_text<midi2::ump::stream::endpoint_name_notification>(endpoint_name_,
                                                                    [](auto& m, auto const& chunk) { m.name(chunk); });
  }
  if (message.filter_requests(product_instance)) {
    this->send_text<midi2::ump::stream::product_instance_id_notification>(
        product_instance_id, [](auto& m, auto const& chunk) { m.pid(chunk); });
  }
  if (message.filter_requests(stream_configuration)) {
    this->send_stream_configuration();
  }
}

void device::send_stream_configuration() const {
  this->send(midi2::ump::stream::jr_configuration_notification{}.protocol(midi2_protocol).rxjr(0U).txjr(0U));
}

void device::stream_configuration_request(midi2::ump::stream::jr_configuration_request const& message) const {
  std::cout << std::format("Stream Configuration Request: protocol=0x{:02X} rxjr={} txjr={}\n", message.protocol(),
                           message.rxjr(), message.txjr());
  // The device supports only the MIDI 2.0 protocol without JR timestamps, so it reports that configuration
  // whatever was requested.
  this->send_stream_configuration();
}

void device::function_block_discovery(midi2::ump::stream::function_block_discovery const& message) const {
  constexpr auto all_blocks = std::uint8_t{0xFF};
  auto const block = message.block_num();
  auto const filter = message.filter();
  std::cout << std::format("Function Block Discovery: block=0x{:02X} filter=0x{:02X}\n", block, filter);
  if (block != all_blocks && block != function_block_number) {
    return;
  }
  using enum midi2::ump::stream::function_block_discovery::filter_mask;
  if (message.filter_requests(info)) {
    this->send_function_block_info();
  }
  if (message.filter_requests(name)) {
    this->send_function_block_name();
  }
}

void device::send_function_block_info() const {
  constexpr auto ui_hint_receiver_and_sender = std::uint8_t{0x3};
  constexpr auto not_midi1 = std::uint8_t{0x0};
  constexpr auto bidirectional = std::uint8_t{0x3};
  constexpr auto midi_ci_version_1_2 = std::uint8_t{0x02};
  this->send(midi2::ump::stream::function_block_info_notification{}
                 .block_active(1U)
                 .block_num(function_block_number)
                 .ui_hint(ui_hint_receiver_and_sender)
                 .midi1(not_midi1)
                 .direction(bidirectional)
                 .first_group(function_block_group)
                 .num_spanned(1U)
                 .ci_message_version(midi_ci_version_1_2)
                 .max_sys8_streams(0U));
}

void device::send_function_block_name() const {
  constexpr auto max_size = midi2::ump::max_size_v<midi2::ump::stream::function_block_name_notification>;
  for_each_text_chunk<max_size>(function_block_name, [this](stream_format const format, text_chunk<max_size> const& c) {
    this->send(midi2::ump::stream::function_block_name_notification{}
                   .format(std::to_underlying(format))
                   .block_num(function_block_number)
                   .name(c));
  });
}

// MIDI-CI
// ~~~~~~~

// The fields that the device's Discovery and Reply to Discovery messages share.
constexpr auto ci_manufacturer =
    std::array{midi2::ci::b7{manufacturer_id[0]}, midi2::ci::b7{manufacturer_id[1]}, midi2::ci::b7{manufacturer_id[2]}};
constexpr auto ci_version = std::array{midi2::ci::b7{software_revision[0]}, midi2::ci::b7{software_revision[1]},
                                       midi2::ci::b7{software_revision[2]}, midi2::ci::b7{software_revision[3]}};
// Bits of the capability inquiry category: 0x04 Profile Configuration, 0x08 Property Exchange.
constexpr auto ci_capabilities = 0x0C_b7;

void device::ci_discovery(midi2::ci::header const& header, midi2::ci::discovery const& discovery) {
  std::cout << std::format("MIDI-CI Discovery: from MUID 0x{:07X}, CI version 0x{:02X}\n", header.remote_muid,
                           header.version);
  if (header.remote_muid == muid_) {
    // Another device is using this device's MUID. Resolve the collision as M2-101 section 5.9.1 option B describes:
    // invalidate the duplicated MUID (so that the other device changes its MUID too), then change this one.
    std::cout << "MUID collision: invalidating the duplicated MUID\n";
    this->broadcast_ci(midi2::ci::invalidate_muid{.target_muid = muid_});
    this->change_muid();
    return;
  }
  this->send_ci(header, function_block_device_id,
                midi2::ci::discovery_reply{
                    .manufacturer = ci_manufacturer,
                    .family = 0_b14,
                    .model = 0_b14,
                    .version = ci_version,
                    .capability = ci_capabilities,
                    .max_sysex_size = midi2::ci::b28{max_sysex_size},
                    .output_path_id = discovery.output_path_id,
                    .function_block = midi2::ci::b7{function_block_number},
                });
}

void device::ci_discovery_reply(midi2::ci::header const& header) const {
  std::cout << std::format("MIDI-CI Reply to Discovery: from MUID 0x{:07X}\n", header.remote_muid);
}

void device::ci_invalidate_muid(midi2::ci::header const& header, midi2::ci::invalidate_muid const& invalidate) {
  std::cout << std::format("MIDI-CI Invalidate MUID: from MUID 0x{:07X}, target MUID 0x{:07X}\n", header.remote_muid,
                           invalidate.target_muid);
  // The device keeps no information about other devices and each of its transactions is a single request and reply,
  // so an Invalidate MUID message for another device needs no action. One for this device means that it must change
  // its MUID (M2-101 section 5.9). No reply is sent.
  if (invalidate.target_muid == muid_) {
    this->change_muid();
  }
}

// Replaces the device's MUID with a new one and broadcasts a Discovery message so that other devices can connect to
// the device again (M2-101 section 5.9).
void device::change_muid() {
  muid_ = make_muid(muid_);
  std::cout << std::format("New MUID 0x{:07X}\n", muid_);
  this->send_discovery();
}

void device::send_discovery() const {
  this->broadcast_ci(midi2::ci::discovery{
      .manufacturer = ci_manufacturer,
      .family = 0_b14,
      .model = 0_b14,
      .version = ci_version,
      .capability = ci_capabilities,
      .max_sysex_size = midi2::ci::b28{max_sysex_size},
      .output_path_id = 0_b7,
  });
}

void device::invalidate_own_muid() const {
  std::cout << std::format("Invalidating MUID 0x{:07X}\n", muid_);
  this->broadcast_ci(midi2::ci::invalidate_muid{.target_muid = muid_});
}

void device::ci_endpoint(midi2::ci::header const& header, midi2::ci::endpoint const& endpoint) const {
  std::cout << std::format("MIDI-CI Inquiry: Endpoint: status=0x{:02X}\n", endpoint.status);
  // Status 0x00 asks for the Product Instance Id.
  if (endpoint.status != 0_b7) {
    return;
  }
  this->send_ci(header, function_block_device_id,
                midi2::ci::endpoint_reply{.status = 0_b7, .information = product_instance_id_});
}

void device::ci_profile_inquiry(midi2::ci::header const& header) const {
  std::cout << std::format("MIDI-CI Profile Inquiry: device ID 0x{:02X}\n", header.device_id);
  // The device has no profiles.
  this->send_ci(header, header.device_id, midi2::ci::profile_configuration::inquiry_reply{});
}

void device::ci_pe_capabilities(midi2::ci::header const& header) const {
  std::cout << "MIDI-CI Property Exchange Capabilities\n";
  this->send_ci(header, function_block_device_id,
                midi2::ci::property_exchange::capabilities_reply{
                    .num_simultaneous = 1_b7,
                    .major_version = 0_b7,
                    .minor_version = 0_b7,
                });
}

void device::ci_pe_get(midi2::ci::header const& header, midi2::ci::property_exchange::get const& get) const {
  std::cout << std::format("MIDI-CI Property Exchange Get: {}\n",
                           std::string_view{get.header.data(), get.header.size()});

  auto reply_header = status_ok_json;
  auto data = std::string_view{};
  if (auto const resource = resource_name(get.header); !resource) {
    std::cout << std::format("  Bad request: {}\n", resource.error());
    reply_header = status_bad_request_json;
  } else if (*resource == u8"ResourceList") {
    data = resource_list_json;
  } else if (*resource == u8"DeviceInfo") {
    data = device_info_json;
  } else {
    reply_header = status_not_found_json;
  }
  this->send_ci(header, function_block_device_id,
                midi2::ci::property_exchange::get_reply{
                    .chunk = {.number_of_chunks = 1_b14, .chunk_number = 1_b14},
                    .request = get.request,
                    .header = std::span{reply_header.data(), reply_header.size()},
                    .data = std::as_bytes(std::span{data.data(), data.size()}),
                });
}

// Owns a CoreFoundation object and releases it on destruction.
class cf_string {
public:
  explicit cf_string(std::string const& str)
      : ref_{CFStringCreateWithCString(kCFAllocatorDefault, str.c_str(), kCFStringEncodingUTF8)} {}
  cf_string(cf_string const&) = delete;
  cf_string(cf_string&&) = delete;
  ~cf_string() noexcept {
    if (ref_ != nullptr) {
      CFRelease(ref_);
    }
  }
  cf_string& operator=(cf_string const&) = delete;
  cf_string& operator=(cf_string&&) = delete;
  [[nodiscard]] CFStringRef get() const noexcept { return ref_; }

private:
  CFStringRef ref_;
};

int run(std::string const& endpoint_name, bool const debug) {
  // Show each log line as soon as it is written, even when the output is redirected to a file.
  std::cout << std::unitbuf;

  MIDIClientRef client = 0;
  if (OSStatus const err = MIDIClientCreateWithBlock(CFSTR("midi2lib workbench device"), &client, nullptr);
      err != noErr) {
    std::cerr << std::format("MIDIClientCreate failed ({})\n", err);
    return EXIT_FAILURE;
  }
  cf_string const name{endpoint_name};
  MIDIEndpointRef source = 0;
  if (OSStatus const err = MIDISourceCreateWithProtocol(client, name.get(), kMIDIProtocol_2_0, &source); err != noErr) {
    std::cerr << std::format("MIDISourceCreateWithProtocol failed ({})\n", err);
    MIDIClientDispose(client);
    return EXIT_FAILURE;
  }

  device dev{source, endpoint_name, make_muid(), debug};
  device* const devp = &dev;
  MIDIEndpointRef destination = 0;
  // CoreMIDI declares its receive block to be nonblocking, but this one logs and sends replies. That is fine for a
  // test device; a real-time application would hand the words to another thread instead.
#if defined(__clang__)
#pragma clang diagnostic push
#if __has_warning("-Wfunction-effects")
#pragma clang diagnostic ignored "-Wfunction-effects"
#endif
#endif
  OSStatus const err =
      MIDIDestinationCreateWithProtocol(client, name.get(), kMIDIProtocol_2_0, &destination,
                                        ^(MIDIEventList const* const list, void* /*src_conn_ref_con*/) {
                                          devp->receive(list);
                                        });
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
  if (err != noErr) {
    std::cerr << std::format("MIDIDestinationCreateWithProtocol failed ({})\n", err);
    MIDIEndpointDispose(source);
    MIDIClientDispose(client);
    return EXIT_FAILURE;
  }

  std::cout << std::format("\"{}\" is ready (MUID 0x{:07X}). Press Ctrl-C to quit.\n", endpoint_name, dev.muid());

  // Run until SIGINT. CoreMIDI calls the destination's block on a thread of its own, so all of the device's work
  // happens there; the main thread's run loop just waits for the signal.
  std::signal(SIGINT, SIG_IGN);
  dispatch_source_t sigint = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGINT, 0, dispatch_get_main_queue());
  dispatch_source_set_event_handler(sigint, ^{
    CFRunLoopStop(CFRunLoopGetMain());
  });
  dispatch_resume(sigint);
  CFRunLoopRun();
  dispatch_source_cancel(sigint);

  std::cout << "\nStopping\n";
  // Dispose of the destination first so that nothing is being received while the device invalidates its MUID.
  MIDIEndpointDispose(destination);
  dev.invalidate_own_muid();
  MIDIEndpointDispose(source);
  MIDIClientDispose(client);
  return EXIT_SUCCESS;
}

}  // end anonymous namespace

int main(int argc, char const* argv[]) {
  int exit_code = EXIT_SUCCESS;
  try {
    auto const args = std::span{argv, static_cast<std::size_t>(argc)};
    auto debug = false;
    auto endpoint_name = std::string{default_endpoint_name};
    for (auto const* const arg : args.subspan(1)) {
      if (arg == "-debug"sv) {
        debug = true;
      } else if (arg[0] == '-') {
        std::cerr << std::format("Usage: {} [-debug] [endpoint-name]\n", args[0]);
        return EXIT_FAILURE;
      } else {
        endpoint_name = arg;
      }
    }
    exit_code = run(endpoint_name, debug);
  } catch (std::exception const& ex) {
    // The midi2 library doesn't throw but something else might...
    std::cerr << "Error: " << ex.what() << '\n';
    exit_code = EXIT_FAILURE;
  } catch (...) {
    std::cerr << "An unknown error\n";
    exit_code = EXIT_FAILURE;
  }
  return exit_code;
}
