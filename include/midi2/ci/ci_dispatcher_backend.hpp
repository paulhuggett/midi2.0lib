//===-- CI Dispatcher Backend -------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ci_dispatcher_backend.hpp
/// \brief  Concepts and types for MIDI Capability Inquiry dispatcher backends

#ifndef MIDI2_CI_DISPATCHER_BACKEND_HPP
#define MIDI2_CI_DISPATCHER_BACKEND_HPP

#include <concepts>
#include <cstdint>
#include <functional>

#include "midi2/ci/ci_types.hpp"
#include "midi2/utils.hpp"

namespace midi2::ci::dispatcher_backend {

// clang-format off
/// \brief The system messages concept gathers MIDI CI dispatcher callback functions that relate to the system as a
/// whole rather than to a specific group of MIDI messages.
///
/// The functions required are:
///
/// - check_muid: Checks whether the message is addressed to this receiver. If this function returns true, the message
///   is dispatched; otherwise it is dropped.
/// - unknown_midici: Called when an unrecognized message is received.
/// - buffer_overflow: Called when the space allocated to the internal buffer is exceeded.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept system = requires(T v, Context context) {
  { v.check_muid(context, std::uint8_t{} /*group*/, muid{}) } -> std::convertible_to<bool>;
  { v.unknown_midici(context, header{}) } -> std::same_as<void>;
  { v.buffer_overflow(context) } -> std::same_as<void>;
};

/// \brief The management concept gathers MIDI CI dispatcher callback functions for the management category of messages.
///
/// The functions required are:
///
/// - discovery: Called when a Discovery message is received. Discovery is used by an initiator to find MIDI-CI
///   devices and establish connections.
/// - discovery_reply: Called when a Reply to Discovery message is received. A responder sends this reply in
///   response to a Discovery message.
/// - endpoint: Called when an Inquiry: Endpoint message (a request for information about an endpoint) is received.
/// - endpoint_reply: Called when a response to an endpoint message is received.
/// - invalidate_muid: Called when an Invalidate MUID message is received.
/// - ack: Called when a MIDI-CI ACK message is received.
/// - nak: Called when a MIDI-CI NAK message is received.
///
/// Each takes the context value, the MIDI-CI header, and the message body. Each returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept management = requires(T v, Context context) {
  { v.discovery(context, header{}, discovery{}) } -> std::same_as<void>;
  { v.discovery_reply(context, header{}, discovery_reply{}) } -> std::same_as<void>;
  { v.endpoint(context, header{}, endpoint{}) } -> std::same_as<void>;
  { v.endpoint_reply(context, header{}, endpoint_reply{}) } -> std::same_as<void>;
  { v.invalidate_muid(context, header{}, invalidate_muid{}) } -> std::same_as<void>;
  { v.ack(context, header{}, ack{}) } -> std::same_as<void>;
  { v.nak(context, header{}, nak{}) } -> std::same_as<void>;
};

/// \brief The profile concept gathers the MIDI-CI dispatcher callback functions for MIDI-CI Profile Configuration
/// messages.
///
/// The type must provide the member functions inquiry(), inquiry_reply(), added(), removed(), details(),
/// details_reply(), on(), off(), enabled(), disabled() and specific_data(). Each takes the context value, the MIDI-CI
/// header, and (except for messages which consist only of the header) the message body. Each returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept profile = requires(T v, Context context) {
  { v.inquiry(context, header{}) } -> std::same_as<void>;
  { v.inquiry_reply(context, header{}, profile_configuration::inquiry_reply{}) } -> std::same_as<void>;
  { v.added(context, header{}, profile_configuration::added{}) } -> std::same_as<void>;
  { v.removed(context, header{}, profile_configuration::removed{}) } -> std::same_as<void>;
  { v.details(context, header{}, profile_configuration::details{}) } -> std::same_as<void>;
  { v.details_reply(context, header{}, profile_configuration::details_reply{}) } -> std::same_as<void>;
  { v.on(context, header{}, profile_configuration::on{}) } -> std::same_as<void>;
  { v.off(context, header{}, profile_configuration::off{}) } -> std::same_as<void>;
  { v.enabled(context, header{}, profile_configuration::enabled{}) } -> std::same_as<void>;
  { v.disabled(context, header{}, profile_configuration::disabled{}) } -> std::same_as<void>;
  { v.specific_data(context, header{}, profile_configuration::specific_data{}) } -> std::same_as<void>;
};

/// \brief The property_exchange concept gathers the MIDI-CI dispatcher callback functions for MIDI-CI Property Exchange
/// messages.
///
/// The type must provide the member functions capabilities(), capabilities_reply(), get(), get_reply(), set(),
/// set_reply(), subscription(), subscription_reply() and notify(). Each takes the context value, the MIDI-CI header,
/// and (except for messages which consist only of the header) the message body. Each returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept property_exchange = requires(T v, Context context) {
  { v.capabilities(context, header{}, property_exchange::capabilities{}) } -> std::same_as<void>;
  { v.capabilities_reply(context, header{}, property_exchange::capabilities_reply{}) } -> std::same_as<void>;

  { v.get(context, header{}, property_exchange::get{}) } -> std::same_as<void>;
  { v.get_reply(context, header{}, property_exchange::get_reply{}) } -> std::same_as<void>;
  { v.set(context, header{}, property_exchange::set{}) } -> std::same_as<void>;
  { v.set_reply(context, header{}, property_exchange::set_reply{}) } -> std::same_as<void>;

  { v.subscription(context, header{}, property_exchange::subscription{}) } -> std::same_as<void>;
  { v.subscription_reply(context, header{}, property_exchange::subscription_reply{}) } -> std::same_as<void>;
  { v.notify(context, header{}, property_exchange::notify{}) } -> std::same_as<void>;
};

/// \brief The process_inquiry concept gathers the MIDI-CI dispatcher callback functions for MIDI-CI Process Inquiry
/// messages.
///
/// The type must provide the member functions capabilities(), capabilities_reply(), midi_message_report(),
/// midi_message_report_reply() and midi_message_report_end(). Each takes the context value, the MIDI-CI header, and
/// (except for messages which consist only of the header) the message body. Each returns void.
///
/// \p Context is the type of the context value that is passed to each handler.
template <typename T, typename Context>
concept process_inquiry = requires(T v, Context context) {
  { v.capabilities(context, header{}) } -> std::same_as<void>;
  { v.capabilities_reply(context, header{}, process_inquiry::capabilities_reply{}) } -> std::same_as<void>;
  { v.midi_message_report(context, header{}, process_inquiry::midi_message_report{}) } -> std::same_as<void>;
  { v.midi_message_report_reply(context, header{}, process_inquiry::midi_message_report_reply{}) } -> std::same_as<void>;
  { v.midi_message_report_end(context, header{}) } -> std::same_as<void>;
};
// clang-format on

// clang-format off
/// An implementation of the MIDI CI system message handlers which each discard the incoming message.
template <typename Context> struct system_null {
  /// Checks whether the message is addressed to this receiver. If this function returns true, the message is
  /// dispatched; otherwise it is dropped.
  ///
  /// \param c  The context object passed to all message handlers.
  /// \param group  The MIDI group to which the message was addressed.
  /// \param id  The MUID to which the message was addressed.
  /// \returns True if the message is to be dispatched, false if it is to be ignored.
  constexpr static bool check_muid(Context c, std::uint8_t const group, muid const id) { (void)c; (void)group; (void)id; return false; }
  /// This function is called when an unrecognized message is received.
  ///
  /// \param c  The context object passed to all message handlers.
  /// \param h  The header of the unrecognized message.
  constexpr static void unknown_midici(Context c, header const &h) { (void)c; (void)h; /* do nothing */ }
  /// Called when a message has overflowed the capacity of the internal buffer.
  ///
  /// \param c  The context object passed to all message handlers.
  constexpr static void buffer_overflow(Context c) { (void)c; /* do nothing */ }
};
/// An implementation of the MIDI CI management message handlers which each discard the incoming message.
template <typename Context> struct management_null {
  /// \param c  The context object passed to all message handlers.
  /// \param h  An instance of midi2::ci::header which contains fields from the standard MIDI CI header.
  /// \param m  The discovery message fields.
  constexpr static void discovery(Context c, header const &h, discovery const &m) { (void)c; (void)h; (void)m; /* do nothing */ }
  /// \param c  The context object passed to all message handlers.
  /// \param h  An instance of midi2::ci::header which contains fields from the standard MIDI CI header.
  /// \param m  The discovery-reply message fields.
  constexpr static void discovery_reply(Context c, header const &h, discovery_reply const &m) { (void)c; (void)h; (void)m; /* do nothing */ }
  /// \param c  The context object passed to all message handlers.
  /// \param h  An instance of midi2::ci::header which contains fields from the standard MIDI CI header.
  /// \param m  The endpoint message fields.
  constexpr static void endpoint(Context c, header const &h, endpoint const &m) { (void)c; (void)h; (void)m; /* do nothing */ }
  /// \param c  The context object passed to all message handlers.
  /// \param h  An instance of midi2::ci::header which contains fields from the standard MIDI CI header.
  /// \param m  The endpoint-reply message fields.
  constexpr static void endpoint_reply(Context c, header const &h, endpoint_reply const &m) { (void)c; (void)h; (void)m; /* do nothing */ }
  /// \param c  The context object passed to all message handlers.
  /// \param h  An instance of midi2::ci::header which contains fields from the standard MIDI CI header.
  /// \param m  The invalidate-MUID message fields.
  constexpr static void invalidate_muid(Context c, header const &h, invalidate_muid const &m) { (void)c; (void)h; (void)m; /* do nothing */ }
  /// \param c  The context object passed to all message handlers.
  /// \param h  An instance of midi2::ci::header which contains fields from the standard MIDI CI header.
  /// \param m  The Ack message fields.
  constexpr static void ack(Context c, header const &h, ack const &m) { (void)c; (void)h; (void)m; /* do nothing */ }
  /// \param c  The context object passed to all message handlers.
  /// \param h  An instance of midi2::ci::header which contains fields from the standard MIDI CI header.
  /// \param m  The Nak message fields.
  constexpr static void nak(Context c, header const &h, nak const &m) { (void)c; (void)h; (void)m; /* do nothing */ }
};
/// \brief An implementation of the MIDI-CI Profile Configuration message handlers which each discard the incoming
/// message.
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct profile_null {
  /// \brief Ignores a "Profile Inquiry" message (M2-101 section 7.2).
  constexpr static void inquiry(Context, header const &) { /* do nothing */ }
  /// \brief Ignores a "Reply to Profile Inquiry" message (M2-101 section 7.3).
  constexpr static void inquiry_reply(Context, header const &, profile_configuration::inquiry_reply const &) { /* do nothing */ }
  /// \brief Ignores a "Profile Added Report" message (M2-101 section 7.4).
  constexpr static void added(Context, header const &, profile_configuration::added const &) { /* do nothing */ }
  /// \brief Ignores a "Profile Removed Report" message (M2-101 section 7.5).
  constexpr static void removed(Context, header const &, profile_configuration::removed const &) { /* do nothing */ }
  /// \brief Ignores a "Profile Details Inquiry" message (M2-101 section 7.6).
  constexpr static void details(Context, header const &, profile_configuration::details const &) { /* do nothing */ }
  /// \brief Ignores a "Reply to Profile Details Inquiry" message (M2-101 section 7.7).
  constexpr static void details_reply(Context, header const &, profile_configuration::details_reply const &) { /* do nothing */ }
  /// \brief Ignores a "Set Profile On" message (M2-101 section 7.8).
  constexpr static void on(Context, header const &, profile_configuration::on const &) { /* do nothing */ }
  /// \brief Ignores a "Set Profile Off" message (M2-101 section 7.9).
  constexpr static void off(Context, header const &, profile_configuration::off const &) { /* do nothing */ }
  /// \brief Ignores a "Profile Enabled Report" message (M2-101 section 7.10).
  constexpr static void enabled(Context, header const &, profile_configuration::enabled const &) { /* do nothing */ }
  /// \brief Ignores a "Profile Disabled Report" message (M2-101 section 7.11).
  constexpr static void disabled(Context, header const &, profile_configuration::disabled const &) { /* do nothing */ }
  /// \brief Ignores a "Profile Specific Data" message (M2-101 section 7.12).
  constexpr static void specific_data(Context, header const &, profile_configuration::specific_data const &) { /* do nothing */ }
};
/// \brief An implementation of the MIDI-CI Property Exchange message handlers which each discard the incoming message.
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct property_exchange_null {
  /// \brief Ignores a "Inquiry: Property Exchange Capabilities" message (M2-101 section 8.5).
  constexpr static void capabilities(Context, header const &, property_exchange::capabilities const &) { /* do nothing */ }
  /// \brief Ignores a "Reply to Property Exchange Capabilities" message (M2-101 section 8.6).
  constexpr static void capabilities_reply(Context, header const &, property_exchange::capabilities_reply const &) { /* do nothing */ }

  /// \brief Ignores a "Inquiry: Get Property Data" message (M2-101 section 8.7).
  constexpr static void get(Context, header const &, property_exchange::get const &) { /* do nothing */ }
  /// \brief Ignores a "Reply to Get Property Data" message (M2-101 section 8.8).
  constexpr static void get_reply(Context, header const &, property_exchange::get_reply const &) { /* do nothing */ }
  /// \brief Ignores a "Inquiry: Set Property Data" message (M2-101 section 8.9).
  constexpr static void set(Context, header const &, property_exchange::set const &) { /* do nothing */ }
  /// \brief Ignores a "Reply to Set Property Data" message (M2-101 section 8.10).
  constexpr static void set_reply(Context, header const &, property_exchange::set_reply const &) { /* do nothing */ }

  /// \brief Ignores a "Subscription" message (M2-101 section 8.11).
  constexpr static void subscription(Context, header const &, property_exchange::subscription const &) { /* do nothing */ }
  /// \brief Ignores a "Reply to Subscription" message (M2-101 section 8.12).
  constexpr static void subscription_reply(Context, header const &, property_exchange::subscription_reply const &) { /* do nothing */ }
  /// \brief Ignores a "Notify" message (M2-101 section 8.13).
  constexpr static void notify(Context, header const &, property_exchange::notify const &) { /* do nothing */ }
};
/// \brief An implementation of the MIDI-CI Process Inquiry message handlers which each discard the incoming message.
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct process_inquiry_null {
  /// \brief Ignores a "Inquiry: Process Inquiry Capabilities" message (M2-101 section 9.2).
  constexpr static void capabilities(Context, header const &) { /* do nothing */ }
  /// \brief Ignores a "Reply to Process Inquiry Capabilities" message (M2-101 section 9.3).
  constexpr static void capabilities_reply(Context, header const &, process_inquiry::capabilities_reply const &) { /* do nothing */ }
  /// \brief Ignores a "Inquiry: MIDI Message Report" message (M2-101 section 9.5).
  constexpr static void midi_message_report(Context, header const &, process_inquiry::midi_message_report const &) { /* do nothing */ }
  /// \brief Ignores a "Reply to MIDI Message Report" message (M2-101 section 9.6).
  constexpr static void midi_message_report_reply(Context, header const &, process_inquiry::midi_message_report_reply const &) { /* do nothing */ }
  /// \brief Ignores a "End of MIDI Message Report" message (M2-101 section 9.8).
  constexpr static void midi_message_report_end(Context, header const &) { /* do nothing */ }
};
// clang-format on

static_assert(system<system_null<int>, int>);
static_assert(management<management_null<int>, int>);
static_assert(profile<profile_null<int>, int>);
static_assert(property_exchange<property_exchange_null<int>, int>);
static_assert(process_inquiry<process_inquiry_null<int>, int>);

/// \brief An abstract base class for MIDI-CI system message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// system_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct system_pure {
  /// \brief Default constructor.
  constexpr system_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr system_pure(system_pure const&) = default;
  /// \brief Move constructor.
  constexpr system_pure(system_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~system_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr system_pure& operator=(system_pure const&) noexcept = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr system_pure& operator=(system_pure&&) noexcept = default;

  /// Checks whether the message is addressed to this receiver. If this function returns true, the message is
  /// dispatched; otherwise it is dropped.
  ///
  /// \param c  The context object passed to all message handlers.
  /// \param group  The MIDI group to which the message was addressed.
  /// \param id  The MUID to which the message was addressed.
  /// \returns True if the message is to be dispatched, false if it is to be ignored.
  virtual bool check_muid(Context c, std::uint8_t group, muid id) = 0;
  /// This function is called when an unrecognized message is received.
  ///
  /// \param c  The context object passed to all message handlers.
  /// \param h  The header of the unrecognized message.
  virtual void unknown_midici(Context c, header const& h) = 0;
  /// Called when a message has overflowed the capacity of the internal buffer.
  ///
  /// \param c  The context object passed to all message handlers.
  virtual void buffer_overflow(Context c) = 0;
};
/// \brief An abstract base class for MIDI-CI Management message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// management_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct management_pure {
  /// \brief Default constructor.
  constexpr management_pure() noexcept = default;
  /// \brief Copy constructor.
  constexpr management_pure(management_pure const&) = default;
  /// \brief Move constructor.
  constexpr management_pure(management_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~management_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr management_pure& operator=(management_pure const&) noexcept = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr management_pure& operator=(management_pure&&) noexcept = default;

  /// \brief Called for each "Discovery" message (M2-101 section 5.5).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void discovery(Context, header const&, discovery const&) = 0;
  /// \brief Called for each "Reply to Discovery" message (M2-101 section 5.6).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void discovery_reply(Context, header const&, discovery_reply const&) = 0;
  /// \brief Called for each "Inquiry: Endpoint" message (M2-101 section 5.7).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void endpoint(Context, header const&, endpoint const&) = 0;
  /// \brief Called for each "Reply to Endpoint" message (M2-101 section 5.8).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void endpoint_reply(Context, header const&, endpoint_reply const&) = 0;
  /// \brief Called for each "Invalidate MUID" message (M2-101 section 5.9).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void invalidate_muid(Context, header const&, invalidate_muid const&) = 0;
  /// \brief Called for each "MIDI-CI ACK" message (M2-101 section 5.10).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void ack(Context, header const&, ack const&) = 0;
  /// \brief Called for each "MIDI-CI NAK" message (M2-101 section 5.11).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void nak(Context, header const&, nak const&) = 0;
};
/// \brief An abstract base class for MIDI-CI Profile Configuration message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// profile_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct profile_pure {
  /// \brief Default constructor.
  constexpr profile_pure() = default;
  /// \brief Copy constructor.
  constexpr profile_pure(profile_pure const&) = default;
  /// \brief Move constructor.
  constexpr profile_pure(profile_pure&&) noexcept = default;
  /// \brief Destructor.
  constexpr virtual ~profile_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr profile_pure& operator=(profile_pure const&) = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr profile_pure& operator=(profile_pure&&) noexcept = default;

  /// \brief Called for each "Profile Inquiry" message (M2-101 section 7.2).
  ///
  /// The arguments are the context value and the MIDI-CI header.
  virtual void inquiry(Context, header const&) = 0;
  /// \brief Called for each "Reply to Profile Inquiry" message (M2-101 section 7.3).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void inquiry_reply(Context, header const&, profile_configuration::inquiry_reply const&) = 0;
  /// \brief Called for each "Profile Added Report" message (M2-101 section 7.4).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void added(Context, header const&, profile_configuration::added const&) = 0;
  /// \brief Called for each "Profile Removed Report" message (M2-101 section 7.5).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void removed(Context, header const&, profile_configuration::removed const&) = 0;
  /// \brief Called for each "Profile Details Inquiry" message (M2-101 section 7.6).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void details(Context, header const&, profile_configuration::details const&) = 0;
  /// \brief Called for each "Reply to Profile Details Inquiry" message (M2-101 section 7.7).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void details_reply(Context, header const&, profile_configuration::details_reply const&) = 0;
  /// \brief Called for each "Set Profile On" message (M2-101 section 7.8).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void on(Context, header const&, profile_configuration::on const&) = 0;
  /// \brief Called for each "Set Profile Off" message (M2-101 section 7.9).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void off(Context, header const&, profile_configuration::off const&) = 0;
  /// \brief Called for each "Profile Enabled Report" message (M2-101 section 7.10).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void enabled(Context, header const&, profile_configuration::enabled const&) = 0;
  /// \brief Called for each "Profile Disabled Report" message (M2-101 section 7.11).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void disabled(Context, header const&, profile_configuration::disabled const&) = 0;
  /// \brief Called for each "Profile Specific Data" message (M2-101 section 7.12).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void specific_data(Context, header const&, profile_configuration::specific_data const&) = 0;
};
/// \brief An abstract base class for MIDI-CI Property Exchange message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// property_exchange_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct property_exchange_pure {
  /// \brief Default constructor.
  constexpr property_exchange_pure() = default;
  /// \brief Copy constructor.
  constexpr property_exchange_pure(property_exchange_pure const&) = default;
  /// \brief Move constructor.
  constexpr property_exchange_pure(property_exchange_pure&&) noexcept = default;
  /// \brief Destructor.
  constexpr virtual ~property_exchange_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  constexpr property_exchange_pure& operator=(property_exchange_pure const&) = default;
  /// \brief Move assignment operator.
  /// \returns *this
  constexpr property_exchange_pure& operator=(property_exchange_pure&&) noexcept = default;

  /// \brief Called for each "Inquiry: Property Exchange Capabilities" message (M2-101 section 8.5).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void capabilities(Context, header const&, property_exchange::capabilities const&) = 0;
  /// \brief Called for each "Reply to Property Exchange Capabilities" message (M2-101 section 8.6).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void capabilities_reply(Context, header const&, property_exchange::capabilities_reply const&) = 0;

  /// \brief Called for each "Inquiry: Get Property Data" message (M2-101 section 8.7).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void get(Context, header const&, property_exchange::get const&) = 0;
  /// \brief Called for each "Reply to Get Property Data" message (M2-101 section 8.8).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void get_reply(Context, header const&, property_exchange::get_reply const&) = 0;
  /// \brief Called for each "Inquiry: Set Property Data" message (M2-101 section 8.9).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void set(Context, header const&, property_exchange::set const&) = 0;
  /// \brief Called for each "Reply to Set Property Data" message (M2-101 section 8.10).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void set_reply(Context, header const&, property_exchange::set_reply const&) = 0;

  /// \brief Called for each "Subscription" message (M2-101 section 8.11).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void subscription(Context, header const&, property_exchange::subscription const&) = 0;
  /// \brief Called for each "Reply to Subscription" message (M2-101 section 8.12).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void subscription_reply(Context, header const&, property_exchange::subscription_reply const&) = 0;
  /// \brief Called for each "Notify" message (M2-101 section 8.13).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void notify(Context, header const&, property_exchange::notify const&) = 0;
};
/// \brief An abstract base class for MIDI-CI Process Inquiry message handlers.
///
/// Each handler is a pure virtual function, so a derived class must override all of them. Derive from
/// process_inquiry_base<> instead to override only some of them.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct process_inquiry_pure {
  /// \brief Default constructor.
  process_inquiry_pure() = default;
  /// \brief Copy constructor.
  process_inquiry_pure(process_inquiry_pure const&) = default;
  /// \brief Move constructor.
  process_inquiry_pure(process_inquiry_pure&&) noexcept = default;
  /// \brief Destructor.
  virtual ~process_inquiry_pure() noexcept = default;

  /// \brief Copy assignment operator.
  /// \returns *this
  process_inquiry_pure& operator=(process_inquiry_pure const&) = default;
  /// \brief Move assignment operator.
  /// \returns *this
  process_inquiry_pure& operator=(process_inquiry_pure&&) noexcept = default;

  /// \brief Called for each "Inquiry: Process Inquiry Capabilities" message (M2-101 section 9.2).
  ///
  /// The arguments are the context value and the MIDI-CI header.
  virtual void capabilities(Context, header const&) = 0;
  /// \brief Called for each "Reply to Process Inquiry Capabilities" message (M2-101 section 9.3).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void capabilities_reply(Context, header const&, process_inquiry::capabilities_reply const&) = 0;
  /// \brief Called for each "Inquiry: MIDI Message Report" message (M2-101 section 9.5).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void midi_message_report(Context, header const&, process_inquiry::midi_message_report const&) = 0;
  /// \brief Called for each "Reply to MIDI Message Report" message (M2-101 section 9.6).
  ///
  /// The arguments are the context value, the MIDI-CI header, and the message body.
  virtual void midi_message_report_reply(Context, header const &,
                                         process_inquiry::midi_message_report_reply const &) = 0;
  /// \brief Called for each "End of MIDI Message Report" message (M2-101 section 9.8).
  ///
  /// The arguments are the context value and the MIDI-CI header.
  virtual void midi_message_report_end(Context, header const&) = 0;
};

static_assert(management<management_pure<int>, int>);
static_assert(profile<profile_pure<int>, int>);
static_assert(property_exchange<property_exchange_pure<int>, int>);
static_assert(process_inquiry<process_inquiry_pure<int>, int>);

// clang-format off
/// \brief A base class which implements each MIDI-CI system message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct system_base : system_pure<Context> {
  /// Checks whether the message is addressed to this receiver. If this function returns true, the message is
  /// dispatched; otherwise it is dropped.
  ///
  /// \param c  The context object passed to all message handlers.
  /// \param group  The MIDI group to which the message was addressed.
  /// \param id  The MUID to which the message was addressed.
  /// \returns True if the message is to be dispatched, false if it is to be ignored.
  bool check_muid(Context c, std::uint8_t const group, muid const id) override { (void)c; (void)group; (void)id; return false; }
  /// This function is called when an unrecognized message is received.
  ///
  /// \param c  The context object passed to all message handlers.
  /// \param h  The header of the unrecognized message.
  void unknown_midici(Context c, header const &h) override { (void)c; (void)h; /* do nothing */ }
  /// Called when a message has overflowed the capacity of the internal buffer.
  ///
  /// \param c  The context object passed to all message handlers.
  void buffer_overflow(Context c) override { (void)c; }
};
/// \brief A base class which implements each MIDI-CI Management message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct management_base : management_pure<Context> {
  /// \brief Ignores a "Discovery" message (M2-101 section 5.5).
  void discovery(Context, header const &, discovery const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to Discovery" message (M2-101 section 5.6).
  void discovery_reply(Context, header const &, discovery_reply const &) override { /* do nothing */ }
  /// \brief Ignores a "Inquiry: Endpoint" message (M2-101 section 5.7).
  void endpoint(Context, header const &, endpoint const &) override { /* do nothing*/ }
  /// \brief Ignores a "Reply to Endpoint" message (M2-101 section 5.8).
  void endpoint_reply(Context, header const &, endpoint_reply const &) override { /* do nothing */ }
  /// \brief Ignores a "Invalidate MUID" message (M2-101 section 5.9).
  void invalidate_muid(Context, header const &, invalidate_muid const &) override { /* do nothing */ }
  /// \brief Ignores a "MIDI-CI ACK" message (M2-101 section 5.10).
  void ack(Context, header const &, ack const &) override { /* do nothing */ }
  /// \brief Ignores a "MIDI-CI NAK" message (M2-101 section 5.11).
  void nak(Context, header const &, nak const &) override { /* do nothing */ }
};
/// \brief A base class which implements each MIDI-CI Profile Configuration message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct profile_base : profile_pure<Context> {
  /// \brief Ignores a "Profile Inquiry" message (M2-101 section 7.2).
  void inquiry(Context, header const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to Profile Inquiry" message (M2-101 section 7.3).
  void inquiry_reply(Context, header const &, profile_configuration::inquiry_reply const &) override { /* do nothing */ }
  /// \brief Ignores a "Profile Added Report" message (M2-101 section 7.4).
  void added(Context, header const &, profile_configuration::added const &) override { /* do nothing */ }
  /// \brief Ignores a "Profile Removed Report" message (M2-101 section 7.5).
  void removed(Context, header const &, profile_configuration::removed const &) override { /* do nothing */ }
  /// \brief Ignores a "Profile Details Inquiry" message (M2-101 section 7.6).
  void details(Context, header const &, profile_configuration::details const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to Profile Details Inquiry" message (M2-101 section 7.7).
  void details_reply(Context, header const &, profile_configuration::details_reply const &) override { /* do nothing */ }
  /// \brief Ignores a "Set Profile On" message (M2-101 section 7.8).
  void on(Context, header const &, profile_configuration::on const &) override { /* do nothing */ }
  /// \brief Ignores a "Set Profile Off" message (M2-101 section 7.9).
  void off(Context, header const &, profile_configuration::off const &) override { /* do nothing */ }
  /// \brief Ignores a "Profile Enabled Report" message (M2-101 section 7.10).
  void enabled(Context, header const &, profile_configuration::enabled const &) override { /* do nothing */ }
  /// \brief Ignores a "Profile Disabled Report" message (M2-101 section 7.11).
  void disabled(Context, header const &, profile_configuration::disabled const &) override { /* do nothing */ }
  /// \brief Ignores a "Profile Specific Data" message (M2-101 section 7.12).
  void specific_data(Context, header const &, profile_configuration::specific_data const &) override { /* do nothing */ }
};
/// \brief A base class which implements each MIDI-CI Property Exchange message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct property_exchange_base : property_exchange_pure<Context> {
  /// \brief Ignores a "Inquiry: Property Exchange Capabilities" message (M2-101 section 8.5).
  void capabilities(Context, header const &, property_exchange::capabilities const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to Property Exchange Capabilities" message (M2-101 section 8.6).
  void capabilities_reply(Context, header const &, property_exchange::capabilities_reply const &) override { /* do nothing */ }

  /// \brief Ignores a "Inquiry: Get Property Data" message (M2-101 section 8.7).
  void get(Context, header const &, property_exchange::get const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to Get Property Data" message (M2-101 section 8.8).
  void get_reply(Context, header const &, property_exchange::get_reply const &) override { /* do nothing */ }
  /// \brief Ignores a "Inquiry: Set Property Data" message (M2-101 section 8.9).
  void set(Context, header const &, property_exchange::set const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to Set Property Data" message (M2-101 section 8.10).
  void set_reply(Context, header const &, property_exchange::set_reply const &) override { /* do nothing */ }

  /// \brief Ignores a "Subscription" message (M2-101 section 8.11).
  void subscription(Context, header const &, property_exchange::subscription const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to Subscription" message (M2-101 section 8.12).
  void subscription_reply(Context, header const &, property_exchange::subscription_reply const &) override { /* do nothing */ }
  /// \brief Ignores a "Notify" message (M2-101 section 8.13).
  void notify(Context, header const &, property_exchange::notify const &) override { /* do nothing */ }
};
/// \brief A base class which implements each MIDI-CI Process Inquiry message handler by doing nothing.
///
/// Derive from this class and override only the handlers for the messages that you want to receive.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> struct process_inquiry_base : process_inquiry_pure<Context> {
  /// \brief Ignores a "Inquiry: Process Inquiry Capabilities" message (M2-101 section 9.2).
  void capabilities(Context, header const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to Process Inquiry Capabilities" message (M2-101 section 9.3).
  void capabilities_reply(Context, header const &, process_inquiry::capabilities_reply const &) override { /* do nothing */ }
  /// \brief Ignores a "Inquiry: MIDI Message Report" message (M2-101 section 9.5).
  void midi_message_report(Context, header const &, process_inquiry::midi_message_report const &) override { /* do nothing */ }
  /// \brief Ignores a "Reply to MIDI Message Report" message (M2-101 section 9.6).
  void midi_message_report_reply(Context, header const &, process_inquiry::midi_message_report_reply const &) override { /* do nothing */ }
  /// \brief Ignores a "End of MIDI Message Report" message (M2-101 section 9.8).
  void midi_message_report_end(Context, header const &) override { /* do nothing */ }
};
// clang-format on

/// \brief A \ref system backend which forwards each MIDI-CI system message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class system_function {
public:
  /// \brief The type of the function called to check whether a message is addressed to this receiver. Its arguments
  /// are the context value, the group, and the destination MUID. It returns true if the message is to be dispatched.
  using check_muid_fn = std::function<bool(Context, std::uint8_t /*group*/, muid)>;
  /// \brief The type of the function called when an unrecognized message is received. Its arguments are the context
  /// value and the message header.
  using unknown_fn = std::function<void(Context, header const&)>;
  /// \brief The type of the function called when a message overflows the capacity of the internal buffer. Its
  /// argument is the context value.
  using buffer_overflow_fn = std::function<void(Context)>;

  /// Sets the function that is to be called when the library needs to check whether the message is addressed to
  /// this receiver.
  ///
  /// \param check_muid  The function to be called to check whether a message should be handled.
  /// \returns *this
  constexpr system_function& on_check_muid(check_muid_fn check_muid) {
    check_muid_ = std::move(check_muid);
    return *this;
  }
  /// Sets the function that will be called when the library receives an unrecognized message.
  ///
  /// \param unknown  The function to be called when an unrecognized message is received.
  /// \returns *this
  constexpr system_function& on_unknown(unknown_fn unknown) {
    unknown_ = std::move(unknown);
    return *this;
  }
  /// Sets the function that will be called when a message has overflowed the capacity of the internal buffer.
  ///
  /// \param overflow  The function to be called when the internal buffer overflows.
  /// \returns *this
  constexpr system_function& on_buffer_overflow(buffer_overflow_fn overflow) {
    overflow_ = std::move(overflow);
    return *this;
  }

  /// \brief Dispatches a call to system::check_muid to the function that was installed by an earlier call to
  /// on_check_muid().
  ///
  /// \param context  The context value to be passed to the function.
  /// \param group  The MIDI group to which the message was addressed.
  /// \param m  The MUID to which the message was addressed.
  /// \returns The value returned by the installed function, or false (the message is dropped) if no function has been
  ///   installed.
  [[nodiscard]] bool check_muid(Context context, std::uint8_t group, muid m) const {
    return check_muid_ ? check_muid_(std::move(context), group, m) : false;
  }
  /// \brief Dispatches a call to system::unknown_midici to the function that was installed previously by a call to
  /// on_unknown(). If no function has been installed, the message is dropped.
  ///
  /// \param context  The context value to be passed to the function.
  /// \param ci  The header of the unrecognized message.
  void unknown_midici(Context context, header const& ci) const { call(unknown_, std::move(context), ci); }
  /// \brief Dispatches a call to system::buffer_overflow to the function that was installed previously by a call to
  /// on_buffer_overflow(). If no function has been installed, the message is dropped.
  ///
  /// \param context  The context value to be passed to the function.
  void buffer_overflow(Context context) const { call(overflow_, std::move(context)); }

private:
  check_muid_fn check_muid_;
  unknown_fn unknown_;
  buffer_overflow_fn overflow_;
};

/// \brief A \ref management backend which forwards each MIDI-CI Management message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class management_function {
public:
  /// \brief The type of the function called for each "Discovery" message (M2-101 section 5.5).
  using discovery_fn = std::function<void(Context, header const&, ci::discovery const&)>;
  /// \brief The type of the function called for each "Reply to Discovery" message (M2-101 section 5.6).
  using discovery_reply_fn = std::function<void(Context, header const&, ci::discovery_reply const&)>;
  /// \brief The type of the function called for each "Inquiry: Endpoint" message (M2-101 section 5.7).
  using endpoint_fn = std::function<void(Context, header const&, ci::endpoint const&)>;
  /// \brief The type of the function called for each "Reply to Endpoint" message (M2-101 section 5.8).
  using endpoint_reply_fn = std::function<void(Context, header const&, ci::endpoint_reply const&)>;
  /// \brief The type of the function called for each "Invalidate MUID" message (M2-101 section 5.9).
  using invalidate_muid_fn = std::function<void(Context, header const&, ci::invalidate_muid const&)>;
  /// \brief The type of the function called for each "MIDI-CI ACK" message (M2-101 section 5.10).
  using ack_fn = std::function<void(Context, header const&, ci::ack const&)>;
  /// \brief The type of the function called for each "MIDI-CI NAK" message (M2-101 section 5.11).
  using nak_fn = std::function<void(Context, header const&, ci::nak const&)>;

  /// \brief Sets the function to be called for each "Discovery" message (M2-101 section 5.5).
  /// \param discovery  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr management_function& on_discovery(discovery_fn discovery) {
    discovery_ = std::move(discovery);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Discovery" message (M2-101 section 5.6).
  /// \param discovery_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr management_function& on_discovery_reply(discovery_reply_fn discovery_reply) {
    discovery_reply_ = std::move(discovery_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Inquiry: Endpoint" message (M2-101 section 5.7).
  /// \param endpoint  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr management_function& on_endpoint(endpoint_fn endpoint) {
    endpoint_ = std::move(endpoint);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Endpoint" message (M2-101 section 5.8).
  /// \param endpoint_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr management_function& on_endpoint_reply(endpoint_reply_fn endpoint_reply) {
    endpoint_reply_ = std::move(endpoint_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Invalidate MUID" message (M2-101 section 5.9).
  /// \param invalidate_muid  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr management_function& on_invalidate_muid(invalidate_muid_fn invalidate_muid) {
    invalidate_muid_ = std::move(invalidate_muid);
    return *this;
  }
  /// \brief Sets the function to be called for each "MIDI-CI ACK" message (M2-101 section 5.10).
  /// \param ack  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr management_function& on_ack(ack_fn ack) {
    ack_ = std::move(ack);
    return *this;
  }
  /// \brief Sets the function to be called for each "MIDI-CI NAK" message (M2-101 section 5.11).
  /// \param nak  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr management_function& on_nak(nak_fn nak) {
    nak_ = std::move(nak);
    return *this;
  }

  /// \brief Calls the function registered by on_discovery(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param d  The body of the "Discovery" message (M2-101 section 5.5).
  void discovery(Context context, header const& ci, ci::discovery const& d) const {
    call(discovery_, std::move(context), ci, d);
  }
  /// \brief Calls the function registered by on_discovery_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param dr  The body of the "Reply to Discovery" message (M2-101 section 5.6).
  void discovery_reply(Context context, header const& ci, ci::discovery_reply const& dr) const {
    call(discovery_reply_, std::move(context), ci, dr);
  }
  /// \brief Calls the function registered by on_endpoint(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param epi  The body of the "Inquiry: Endpoint" message (M2-101 section 5.7).
  void endpoint(Context context, header const& ci, ci::endpoint const& epi) const {
    call(endpoint_, std::move(context), ci, epi);
  }
  /// \brief Calls the function registered by on_endpoint_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to Endpoint" message (M2-101 section 5.8).
  void endpoint_reply(Context context, header const& ci, ci::endpoint_reply const& reply) const {
    call(endpoint_reply_, std::move(context), ci, reply);
  }
  /// \brief Calls the function registered by on_invalidate_muid(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param im  The body of the "Invalidate MUID" message (M2-101 section 5.9).
  void invalidate_muid(Context context, header const& ci, ci::invalidate_muid const& im) const {
    call(invalidate_muid_, std::move(context), ci, im);
  }
  /// \brief Calls the function registered by on_ack(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param a  The body of the "MIDI-CI ACK" message (M2-101 section 5.10).
  void ack(Context context, header const& ci, ci::ack const& a) const { call(ack_, std::move(context), ci, a); }
  /// \brief Calls the function registered by on_nak(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param n  The body of the "MIDI-CI NAK" message (M2-101 section 5.11).
  void nak(Context context, header const& ci, ci::nak const& n) const { call(nak_, std::move(context), ci, n); }

private:
  discovery_fn discovery_;
  discovery_reply_fn discovery_reply_;
  endpoint_fn endpoint_;
  endpoint_reply_fn endpoint_reply_;
  invalidate_muid_fn invalidate_muid_;
  ack_fn ack_;
  nak_fn nak_;
};

static_assert(management<management_function<int>, int>);

/// \brief A \ref profile backend which forwards each MIDI-CI Profile Configuration message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class profile_function {
public:
  /// \brief The type of the function called for each "Profile Inquiry" message (M2-101 section 7.2).
  using inquiry_fn = std::function<void(Context, header const&)>;
  /// \brief The type of the function called for each "Reply to Profile Inquiry" message (M2-101 section 7.3).
  using inquiry_reply_fn =
      std::function<void(Context, header const &, ci::profile_configuration::inquiry_reply const &)>;
  /// \brief The type of the function called for each "Profile Added Report" message (M2-101 section 7.4).
  using added_fn = std::function<void(Context, header const&, ci::profile_configuration::added const&)>;
  /// \brief The type of the function called for each "Profile Removed Report" message (M2-101 section 7.5).
  using removed_fn = std::function<void(Context, header const&, ci::profile_configuration::removed const&)>;
  /// \brief The type of the function called for each "Profile Details Inquiry" message (M2-101 section 7.6).
  using details_fn = std::function<void(Context, header const&, ci::profile_configuration::details const&)>;
  /// \brief The type of the function called for each "Reply to Profile Details Inquiry" message (M2-101 section 7.7).
  using details_reply_fn =
      std::function<void(Context, header const &, ci::profile_configuration::details_reply const &)>;
  /// \brief The type of the function called for each "Set Profile On" message (M2-101 section 7.8).
  using on_fn = std::function<void(Context, header const&, ci::profile_configuration::on const&)>;
  /// \brief The type of the function called for each "Set Profile Off" message (M2-101 section 7.9).
  using off_fn = std::function<void(Context, header const&, ci::profile_configuration::off const&)>;
  /// \brief The type of the function called for each "Profile Enabled Report" message (M2-101 section 7.10).
  using enabled_fn = std::function<void(Context, header const&, ci::profile_configuration::enabled const&)>;
  /// \brief The type of the function called for each "Profile Disabled Report" message (M2-101 section 7.11).
  using disabled_fn = std::function<void(Context, header const&, ci::profile_configuration::disabled const&)>;
  /// \brief The type of the function called for each "Profile Specific Data" message (M2-101 section 7.12).
  using specific_data_fn =
      std::function<void(Context, header const &, ci::profile_configuration::specific_data const &)>;

  /// \brief Sets the function to be called for each "Profile Inquiry" message (M2-101 section 7.2).
  /// \param inquiry  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_inquiry(inquiry_fn inquiry) {
    inquiry_ = std::move(inquiry);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Profile Inquiry" message (M2-101 section 7.3).
  /// \param inquiry_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_inquiry_reply(inquiry_reply_fn inquiry_reply) {
    inquiry_reply_ = std::move(inquiry_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Profile Added Report" message (M2-101 section 7.4).
  /// \param added  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_added(added_fn added) {
    added_ = std::move(added);
    return *this;
  }
  /// \brief Sets the function to be called for each "Profile Removed Report" message (M2-101 section 7.5).
  /// \param removed  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_removed(removed_fn removed) {
    removed_ = std::move(removed);
    return *this;
  }
  /// \brief Sets the function to be called for each "Profile Details Inquiry" message (M2-101 section 7.6).
  /// \param details  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_details(details_fn details) {
    details_ = std::move(details);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Profile Details Inquiry" message (M2-101 section 7.7).
  /// \param details_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_details_reply(details_reply_fn details_reply) {
    details_reply_ = std::move(details_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Set Profile On" message (M2-101 section 7.8).
  /// \param on  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_on(on_fn on) {
    on_ = std::move(on);
    return *this;
  }
  /// \brief Sets the function to be called for each "Set Profile Off" message (M2-101 section 7.9).
  /// \param off  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_off(off_fn off) {
    off_ = std::move(off);
    return *this;
  }
  /// \brief Sets the function to be called for each "Profile Enabled Report" message (M2-101 section 7.10).
  /// \param enabled  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_enabled(enabled_fn enabled) {
    enabled_ = std::move(enabled);
    return *this;
  }
  /// \brief Sets the function to be called for each "Profile Disabled Report" message (M2-101 section 7.11).
  /// \param disabled  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_disabled(disabled_fn disabled) {
    disabled_ = std::move(disabled);
    return *this;
  }
  /// \brief Sets the function to be called for each "Profile Specific Data" message (M2-101 section 7.12).
  /// \param specific_data  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr profile_function& on_specific_data(specific_data_fn specific_data) {
    specific_data_ = std::move(specific_data);
    return *this;
  }

  /// \brief Calls the function registered by on_inquiry(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  void inquiry(Context context, header const& ci) const { call(inquiry_, context, ci); }
  /// \brief Calls the function registered by on_inquiry_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to Profile Inquiry" message (M2-101 section 7.3).
  void inquiry_reply(Context context, header const& ci, ci::profile_configuration::inquiry_reply const& reply) const {
    call(inquiry_reply_, context, ci, reply);
  }
  /// \brief Calls the function registered by on_added(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param a  The body of the "Profile Added Report" message (M2-101 section 7.4).
  void added(Context context, header const& ci, ci::profile_configuration::added const& a) const {
    call(added_, context, ci, a);
  }
  /// \brief Calls the function registered by on_removed(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param r  The body of the "Profile Removed Report" message (M2-101 section 7.5).
  void removed(Context context, header const& ci, ci::profile_configuration::removed const& r) const {
    call(removed_, context, ci, r);
  }
  /// \brief Calls the function registered by on_details(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param d  The body of the "Profile Details Inquiry" message (M2-101 section 7.6).
  void details(Context context, header const& ci, ci::profile_configuration::details const& d) const {
    call(details_, context, ci, d);
  }
  /// \brief Calls the function registered by on_details_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to Profile Details Inquiry" message (M2-101 section 7.7).
  void details_reply(Context context, header const& ci, ci::profile_configuration::details_reply const& reply) const {
    call(details_reply_, context, ci, reply);
  }
  /// \brief Calls the function registered by on_on(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param on  The body of the "Set Profile On" message (M2-101 section 7.8).
  void on(Context context, header const& ci, ci::profile_configuration::on const& on) const {
    call(on_, context, ci, on);
  }
  /// \brief Calls the function registered by on_off(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param off  The body of the "Set Profile Off" message (M2-101 section 7.9).
  void off(Context context, header const& ci, ci::profile_configuration::off const& off) const {
    call(off_, context, ci, off);
  }
  /// \brief Calls the function registered by on_enabled(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param enabled  The body of the "Profile Enabled Report" message (M2-101 section 7.10).
  void enabled(Context context, header const& ci, ci::profile_configuration::enabled const& enabled) const {
    call(enabled_, context, ci, enabled);
  }
  /// \brief Calls the function registered by on_disabled(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param disabled  The body of the "Profile Disabled Report" message (M2-101 section 7.11).
  void disabled(Context context, header const& ci, ci::profile_configuration::disabled const& disabled) const {
    call(disabled_, context, ci, disabled);
  }
  /// \brief Calls the function registered by on_specific_data(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param sd  The body of the "Profile Specific Data" message (M2-101 section 7.12).
  void specific_data(Context context, header const& ci, ci::profile_configuration::specific_data const& sd) const {
    call(specific_data_, context, ci, sd);
  }

private:
  inquiry_fn inquiry_;
  inquiry_reply_fn inquiry_reply_;
  added_fn added_;
  removed_fn removed_;
  details_fn details_;
  details_reply_fn details_reply_;
  on_fn on_;
  off_fn off_;
  enabled_fn enabled_;
  disabled_fn disabled_;
  specific_data_fn specific_data_;
};

static_assert(profile<profile_function<int>, int>);

/// \brief A \ref property_exchange backend which forwards each MIDI-CI Property Exchange message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class property_exchange_function {
public:
  /// \brief The type of the function called for each "Inquiry: Property Exchange Capabilities" message (M2-101 section
  /// 8.5).
  using capabilities_fn = std::function<void(Context, header const&, ci::property_exchange::capabilities const&)>;
  /// \brief The type of the function called for each "Reply to Property Exchange Capabilities" message (M2-101 section
  /// 8.6).
  using capabilities_reply_fn =
      std::function<void(Context, header const&, ci::property_exchange::capabilities_reply const&)>;
  /// \brief The type of the function called for each "Inquiry: Get Property Data" message (M2-101 section 8.7).
  using get_fn = std::function<void(Context, header const&, ci::property_exchange::get const&)>;
  /// \brief The type of the function called for each "Reply to Get Property Data" message (M2-101 section 8.8).
  using get_reply_fn = std::function<void(Context, header const&, ci::property_exchange::get_reply const&)>;
  /// \brief The type of the function called for each "Inquiry: Set Property Data" message (M2-101 section 8.9).
  using set_fn = std::function<void(Context, header const&, ci::property_exchange::set const&)>;
  /// \brief The type of the function called for each "Reply to Set Property Data" message (M2-101 section 8.10).
  using set_reply_fn = std::function<void(Context, header const&, ci::property_exchange::set_reply const&)>;
  /// \brief The type of the function called for each "Subscription" message (M2-101 section 8.11).
  using subscription_fn = std::function<void(Context, header const&, ci::property_exchange::subscription const&)>;
  /// \brief The type of the function called for each "Reply to Subscription" message (M2-101 section 8.12).
  using subscription_reply_fn =
      std::function<void(Context, header const&, ci::property_exchange::subscription_reply const&)>;
  /// \brief The type of the function called for each "Notify" message (M2-101 section 8.13).
  using notify_fn = std::function<void(Context, header const&, ci::property_exchange::notify const&)>;

  /// \brief Sets the function to be called for each "Inquiry: Property Exchange Capabilities" message (M2-101 section
  /// 8.5).
  /// \param capabilities  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_capabilities(capabilities_fn capabilities) {
    capabilities_ = std::move(capabilities);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Property Exchange Capabilities" message (M2-101 section
  /// 8.6).
  /// \param capabilities_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_capabilities_reply(capabilities_reply_fn capabilities_reply) {
    capabilities_reply_ = std::move(capabilities_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Inquiry: Get Property Data" message (M2-101 section 8.7).
  /// \param get  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_get(get_fn get) {
    get_ = std::move(get);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Get Property Data" message (M2-101 section 8.8).
  /// \param get_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_get_reply(get_reply_fn get_reply) {
    get_reply_ = std::move(get_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Inquiry: Set Property Data" message (M2-101 section 8.9).
  /// \param set  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_set(set_fn set) {
    set_ = std::move(set);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Set Property Data" message (M2-101 section 8.10).
  /// \param set_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_set_reply(set_reply_fn set_reply) {
    set_reply_ = std::move(set_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Subscription" message (M2-101 section 8.11).
  /// \param subscription  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_subscription(subscription_fn subscription) {
    subscription_ = std::move(subscription);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Subscription" message (M2-101 section 8.12).
  /// \param subscription_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_subscription_reply(subscription_reply_fn subscription_reply) {
    subscription_reply_ = std::move(subscription_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Notify" message (M2-101 section 8.13).
  /// \param notify  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr property_exchange_function& on_notify(notify_fn notify) {
    notify_ = std::move(notify);
    return *this;
  }

  /// \brief Calls the function registered by on_capabilities(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param cap  The body of the "Inquiry: Property Exchange Capabilities" message (M2-101 section 8.5).
  void capabilities(Context context, header const& ci, ci::property_exchange::capabilities const& cap) const {
    call(capabilities_, context, ci, cap);
  }
  /// \brief Calls the function registered by on_capabilities_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to Property Exchange Capabilities" message (M2-101 section 8.6).
  void capabilities_reply(Context context, header const& ci,
                          ci::property_exchange::capabilities_reply const& reply) const {
    call(capabilities_reply_, context, ci, reply);
  }
  /// \brief Calls the function registered by on_get(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param get  The body of the "Inquiry: Get Property Data" message (M2-101 section 8.7).
  void get(Context context, header const& ci, ci::property_exchange::get const& get) const {
    call(get_, context, ci, get);
  }
  /// \brief Calls the function registered by on_get_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to Get Property Data" message (M2-101 section 8.8).
  void get_reply(Context context, header const& ci, ci::property_exchange::get_reply const& reply) const {
    call(get_reply_, context, ci, reply);
  }
  /// \brief Calls the function registered by on_set(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param set  The body of the "Inquiry: Set Property Data" message (M2-101 section 8.9).
  void set(Context context, header const& ci, ci::property_exchange::set const& set) const {
    call(set_, context, ci, set);
  }
  /// \brief Calls the function registered by on_set_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to Set Property Data" message (M2-101 section 8.10).
  void set_reply(Context context, header const& ci, ci::property_exchange::set_reply const& reply) const {
    call(set_reply_, context, ci, reply);
  }
  /// \brief Calls the function registered by on_subscription(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param sub  The body of the "Subscription" message (M2-101 section 8.11).
  void subscription(Context context, header const& ci, ci::property_exchange::subscription const& sub) const {
    call(subscription_, context, ci, sub);
  }
  /// \brief Calls the function registered by on_subscription_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to Subscription" message (M2-101 section 8.12).
  void subscription_reply(Context context, header const& ci,
                          ci::property_exchange::subscription_reply const& reply) const {
    call(subscription_reply_, context, ci, reply);
  }
  /// \brief Calls the function registered by on_notify(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param notify  The body of the "Notify" message (M2-101 section 8.13).
  void notify(Context context, header const& ci, ci::property_exchange::notify const& notify) const {
    call(notify_, context, ci, notify);
  }

private:
  capabilities_fn capabilities_;
  capabilities_reply_fn capabilities_reply_;
  get_fn get_;
  get_reply_fn get_reply_;
  set_fn set_;
  set_reply_fn set_reply_;
  subscription_fn subscription_;
  subscription_reply_fn subscription_reply_;
  notify_fn notify_;
};

static_assert(property_exchange<property_exchange_function<int>, int>);

/// \brief A \ref process_inquiry backend which forwards each MIDI-CI Process Inquiry message to a std::function<>.
///
/// Functions are registered with the on_...() member functions. A message for which no function has been
/// registered is ignored.
///
/// \tparam Context  The type of the context value that is passed to each handler.
template <typename Context> class process_inquiry_function {
public:
  /// \brief The type of the function called for each "Inquiry: Process Inquiry Capabilities" message (M2-101 section
  /// 9.2).
  using capabilities_fn = std::function<void(Context, header const&)>;
  /// \brief The type of the function called for each "Reply to Process Inquiry Capabilities" message (M2-101 section
  /// 9.3).
  using capabilities_reply_fn =
      std::function<void(Context, header const&, ci::process_inquiry::capabilities_reply const&)>;
  /// \brief The type of the function called for each "Inquiry: MIDI Message Report" message (M2-101 section 9.5).
  using midi_message_report_fn =
      std::function<void(Context, header const&, ci::process_inquiry::midi_message_report const&)>;
  /// \brief The type of the function called for each "Reply to MIDI Message Report" message (M2-101 section 9.6).
  using midi_message_report_reply_fn =
      std::function<void(Context, header const&, ci::process_inquiry::midi_message_report_reply const&)>;
  /// \brief The type of the function called for each "End of MIDI Message Report" message (M2-101 section 9.8).
  using midi_message_report_end_fn = std::function<void(Context, header const&)>;

  /// \brief Sets the function to be called for each "Inquiry: Process Inquiry Capabilities" message (M2-101 section
  /// 9.2).
  /// \param capabilities  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr process_inquiry_function& on_capabilities(capabilities_fn capabilities) {
    capabilities_ = std::move(capabilities);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to Process Inquiry Capabilities" message (M2-101 section
  /// 9.3).
  /// \param capabilities_reply  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr process_inquiry_function& on_capabilities_reply(capabilities_reply_fn capabilities_reply) {
    capabilities_reply_ = std::move(capabilities_reply);
    return *this;
  }
  /// \brief Sets the function to be called for each "Inquiry: MIDI Message Report" message (M2-101 section 9.5).
  /// \param mmr  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr process_inquiry_function& on_midi_message_report(midi_message_report_fn mmr) {
    midi_message_report_ = std::move(mmr);
    return *this;
  }
  /// \brief Sets the function to be called for each "Reply to MIDI Message Report" message (M2-101 section 9.6).
  /// \param mmrr  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr process_inquiry_function& on_midi_message_report_reply(midi_message_report_reply_fn mmrr) {
    midi_message_report_reply_ = std::move(mmrr);
    return *this;
  }
  /// \brief Sets the function to be called for each "End of MIDI Message Report" message (M2-101 section 9.8).
  /// \param mmre  The function to be called. An empty function causes these messages to be ignored.
  /// \returns *this
  constexpr process_inquiry_function& on_midi_message_report_end(midi_message_report_end_fn mmre) {
    midi_message_report_end_ = std::move(mmre);
    return *this;
  }

  /// \brief Calls the function registered by on_capabilities(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  void capabilities(Context context, header const& ci) const { call(capabilities_, context, ci); }
  /// \brief Calls the function registered by on_capabilities_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to Process Inquiry Capabilities" message (M2-101 section 9.3).
  void capabilities_reply(Context context, header const& ci,
                          ci::process_inquiry::capabilities_reply const& reply) const {
    call(capabilities_reply_, context, ci, reply);
  }
  /// \brief Calls the function registered by on_midi_message_report(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param mmr  The body of the "Inquiry: MIDI Message Report" message (M2-101 section 9.5).
  void midi_message_report(Context context, header const& ci,
                           ci::process_inquiry::midi_message_report const& mmr) const {
    call(midi_message_report_, context, ci, mmr);
  }
  /// \brief Calls the function registered by on_midi_message_report_reply(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  /// \param reply  The body of the "Reply to MIDI Message Report" message (M2-101 section 9.6).
  void midi_message_report_reply(Context context, header const& ci,
                                 ci::process_inquiry::midi_message_report_reply const& reply) const {
    call(midi_message_report_reply_, context, ci, reply);
  }
  /// \brief Calls the function registered by on_midi_message_report_end(), if any.
  /// \param context  The context value to be passed to the function.
  /// \param ci  The MIDI-CI header of the message.
  void midi_message_report_end(Context context, header const& ci) const { call(midi_message_report_end_, context, ci); }

private:
  capabilities_fn capabilities_;
  capabilities_reply_fn capabilities_reply_;
  midi_message_report_fn midi_message_report_;
  midi_message_report_reply_fn midi_message_report_reply_;
  midi_message_report_end_fn midi_message_report_end_;
};

static_assert(process_inquiry<process_inquiry_function<int>, int>);

}  // end namespace midi2::ci::dispatcher_backend

#endif  // MIDI2_CI_DISPATCHER_BACKEND_HPP
