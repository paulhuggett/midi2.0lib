//===-- UMP to MIDI 2 ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

#include "midi2/ump/ump_to_midi2.hpp"

#include <cstdint>

#include "midi2/ump/ump_types.hpp"
#include "midi2/ump/ump_utils.hpp"

namespace midi2::ump {

// note off
// ~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::note_off(ump_to_midi2::context* const ctxt, ump::m1cvm::note_off const& in) {
  constexpr auto m1bits = ump::m1cvm::note_off::word0::velocity::bits();
  constexpr auto m2bits = ump::m2cvm::note_off::word1::velocity::bits();
  ctxt->push(ump::m2cvm::note_off{}
                 .group(in.group())
                 .channel(in.channel())
                 .note(in.note())
                 .velocity(mcm_scale<m1bits, m2bits>(in.velocity())));
}

// note on
// ~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::note_on(ump_to_midi2::context* const ctxt, ump::m1cvm::note_on const& in) {
  constexpr auto m1bits = ump::m1cvm::note_on::word0::velocity::bits();
  constexpr auto m2bits = ump::m2cvm::note_on::word1::velocity::bits();
  if (in.velocity() == 0U) {
    // A MIDI 1.0 Note On with a velocity of 0 is a Note Off. M2-104 section D.3.1 requires that it is translated to a
    // MIDI 2.0 Note Off with a velocity of 0x8000.
    constexpr auto note_off_velocity = std::uint16_t{0x8000};
    ctxt->push(
        ump::m2cvm::note_off{}.group(in.group()).channel(in.channel()).note(in.note()).velocity(note_off_velocity));
    return;
  }
  ctxt->push(ump::m2cvm::note_on{}
                 .group(in.group())
                 .channel(in.channel())
                 .note(in.note())
                 .velocity(mcm_scale<m1bits, m2bits>(in.velocity())));
}

// poly pressure
// ~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::poly_pressure(ump_to_midi2::context* const ctxt,
                                                         ump::m1cvm::poly_pressure const& in) {
  constexpr auto m1bits = ump::m1cvm::poly_pressure::word0::pressure::bits();
  constexpr auto m2bits = ump::m2cvm::poly_pressure::word1::pressure::bits();
  ctxt->push(ump::m2cvm::poly_pressure{}
                 .group(in.group())
                 .channel(in.channel())
                 .note(in.note())
                 .pressure(mcm_scale<m1bits, m2bits>(in.pressure())));
}

// control change
// ~~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::control_change(ump_to_midi2::context* const ctxt,
                                                          ump::m1cvm::control_change const& in) {
  auto const group = in.group();
  auto const channel = in.channel();
  auto const controller = in.controller();
  auto const value = in.value();

  auto& c = ctxt->parameter_number[group][channel];
  using enum control;
  switch (static_cast<control>(controller)) {
  case bank_select: ctxt->bank[group][channel].set_msb(value); break;
  case bank_select_lsb: ctxt->bank[group][channel].set_lsb(value); break;

  // A parameter number controller marks the end of any previous RPN/NRPN, so send its held value first.
  // (M2-104 section D.3.3)
  case nrpn_msb:
    flush_pending(ctxt, c, group, channel);
    c.pn_is_rpn = false;
    c.set_number_msb(value);
    break;
  case nrpn_lsb:
    flush_pending(ctxt, c, group, channel);
    c.pn_is_rpn = false;
    c.set_number_lsb(value);
    break;

  case rpn_msb:
    flush_pending(ctxt, c, group, channel);
    c.pn_is_rpn = true;
    c.set_number_msb(value);
    break;
  case rpn_lsb:
    flush_pending(ctxt, c, group, channel);
    // Setting RPN to 7FH,7FH will disable the data entry, data increment, and data decrement controllers
    // until a new RPN or NRPN is selected. (MIDI 1.0 Approved Protocol JMSC-0011)
    if (c.pn_is_rpn && c.pn_msb_valid != 0U && c.pn_msb == 0x7FU && value == 0x7FU) {
      c.reset();
    } else {
      c.pn_is_rpn = true;
      c.set_number_lsb(value);
    }
    break;

  case data_entry_msb:
    // A subsequent Data Entry MSB completes the previous value.
    flush_pending(ctxt, c, group, channel);
    c.set_value_msb(value);
    break;

  case data_entry_lsb:
    if (c.is_complete()) {
      if (c.pn_is_rpn) {
        pn_control_message<ump::m2cvm::rpn_controller>(ctxt, c, group, channel, value);
      } else {
        pn_control_message<ump::m2cvm::nrpn_controller>(ctxt, c, group, channel, value);
      }
    }
    c.value_pending = false;
    break;

  case reset_all_controllers:
    flush_pending(ctxt, c, group, channel);
    c.reset();
    [[fallthrough]];

  default: {
    constexpr auto m2v = ump::m2cvm::control_change::word1::value::bits();
    constexpr auto m1v = ump::m1cvm::control_change::word0::value::bits();
    ctxt->push(ump::m2cvm::control_change{}
                   .group(group)
                   .channel(channel)
                   .controller(controller)
                   .value(mcm_scale<m1v, m2v>(value)));
    break;
  }
  }
}

// flush pending
// ~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::flush_pending(ump_to_midi2::context* const ctxt,
                                                         struct ump_to_midi2::context::parameter_number& c,
                                                         std::uint8_t const group, std::uint8_t const channel) {
  if (c.value_pending == 0U) {
    return;
  }
  c.value_pending = false;
  if (c.is_complete()) {
    if (c.pn_is_rpn) {
      pn_control_message<ump::m2cvm::rpn_controller>(ctxt, c, group, channel, 0);
    } else {
      pn_control_message<ump::m2cvm::nrpn_controller>(ctxt, c, group, channel, 0);
    }
  }
}

void ump_to_midi2::flush(std::uint8_t const group, std::uint8_t const channel) {
  assert(group < 16 && channel < 16);
  to_midi2_config::m1cvm::flush_pending(&context_, context_.parameter_number[group][channel], group, channel);
}

// program change
// ~~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::program_change(ump_to_midi2::context* const ctxt,
                                                          ump::m1cvm::program_change const& in) {
  auto const group = in.group();
  auto const channel = in.channel();

  auto out = ump::m2cvm::program_change{}.group(group).channel(channel).program(in.program());
  if (auto const& b = ctxt->bank[group][channel]; b.is_valid()) {
    out.bank_valid(true).bank_msb(b.msb).bank_lsb(b.lsb);
  }
  ctxt->push(out);
}

// channel pressure
// ~~~~~~~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::channel_pressure(ump_to_midi2::context* const ctxt,
                                                            ump::m1cvm::channel_pressure const& in) {
  constexpr auto m2v = ump::m2cvm::channel_pressure::word1::value::bits();
  constexpr auto m1v = ump::m1cvm::channel_pressure::word0::data::bits();
  ctxt->push(
      ump::m2cvm::channel_pressure{}.group(in.group()).channel(in.channel()).value(mcm_scale<m1v, m2v>(in.data())));
}

// pitch bend
// ~~~~~~~~~~
void ump_to_midi2::to_midi2_config::m1cvm::pitch_bend(ump_to_midi2::context* const ctxt,
                                                      ump::m1cvm::pitch_bend const& in) {
  constexpr auto lsb_bits = ump::m1cvm::pitch_bend::word0::lsb_data::bits();
  constexpr auto msb_bits = ump::m1cvm::pitch_bend::word0::msb_data::bits();
  constexpr auto m2v = ump::m2cvm::pitch_bend::word1::value::bits();

  static_assert(lsb_bits + msb_bits <= 16U);
  auto const m1value =
      static_cast<std::uint16_t>(static_cast<std::uint16_t>(in.msb_data() << lsb_bits) | in.lsb_data());
  ctxt->push(ump::m2cvm::pitch_bend{}
                 .group(in.group())
                 .channel(in.channel())
                 .value(mcm_scale<lsb_bits + msb_bits, m2v>(m1value)));
}

}  // end namespace midi2::ump
