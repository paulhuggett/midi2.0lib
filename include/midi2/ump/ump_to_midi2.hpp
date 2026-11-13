//===-- UMP to MIDI 2 ---------------------------------------------------------*- C++ -*-===//
//
// midi2 library under the MIT license.
// See https://github.com/paulhuggett/AM_MIDI2.0Lib/blob/main/LICENSE for license information.
//
// SPDX-FileCopyrightText: Copyright © 2025 Paul Bowen-Huggett
// SPDX-License-Identifier: MIT
//
//===------------------------------------------------------------------------------------===//

/// \file ump_to_midi2.hpp
/// \brief Converts UMP messages from MIDI 1.0 to MIDI 2.0.

#ifndef MIDI2_UMP_TO_MIDI2_HPP
#define MIDI2_UMP_TO_MIDI2_HPP

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>

#include "midi2/adt/fifo.hpp"
#include "midi2/translator.hpp"
#include "midi2/ump/ump_dispatcher.hpp"
#include "midi2/ump/ump_types.hpp"
#include "midi2/ump/ump_utils.hpp"

namespace midi2::ump {

/// \brief Translates a stream of UMP messages so that MIDI 1.0 Channel Voice messages are replaced by their MIDI 2.0
/// Channel Voice equivalents.
///
/// MIDI 1.0 Channel Voice messages are converted to MIDI 2.0 Channel Voice messages, scaling values as required.
/// Bank Select and RPN/NRPN Control Change sequences are tracked so that they can be converted to MIDI 2.0 Program
/// Change (with bank) and Registered/Assignable Controller messages. All other messages are passed through unchanged,
/// except NOOP and unrecognized messages which are discarded.
class ump_to_midi2 {
public:
  /// \brief The type of input UMP message.
  using input_type = std::uint32_t;
  /// \brief The type of output MIDI 2.0 message.
  using output_type = std::uint32_t;

  /// \brief Constructs a translator.
  /// \param group  The UMP group (0-15). The output messages take the group of the corresponding input message.
  explicit constexpr ump_to_midi2(std::uint8_t const group) noexcept {
    assert(group <= 0b1111);
    context_.group = group;
  }

  /// \brief Checks if the output is empty
  /// \return True if the output FIFO is empty, false otherwise.
  [[nodiscard]] constexpr bool empty() const noexcept { return context_.output.empty(); }

  /// \brief Pops the front message from the output FIFO.
  /// \return The first message in the output FIFO.
  /// \note This method asserts that the output FIFO is not empty.
  [[nodiscard]] constexpr output_type pop() noexcept {
    assert(!context_.output.empty());
    return context_.output.pop_front();
  }

  /// \brief Pushes an input UMP message for processing.
  /// \param ump The input UMP message.
  void push(input_type const ump) { p_.dispatch(ump); }

  /// \brief Restores the translator to its original state.
  /// Any in-flight messages are lost.
  constexpr void reset() { context_.reset(); }

private:
  /// \brief The translator state which is shared with the message handlers.
  struct context {
    /// \brief Pushes each of the 32-bit words of a UMP message to the output FIFO.
    /// \tparam T  A UMP message type.
    /// \param value  The message to be pushed.
    template <typename T>
      requires(std::tuple_size_v<T> >= 0)
    constexpr void push(T const& value) {
      ump::apply(value, [this](std::uint32_t const v) {
        output.push_back(v);
        return false;
      });
    }

    /// \brief Resets the translator state: clears the bank and parameter number tracking and the output FIFO.
    constexpr void reset() noexcept {
      group = 0;
      for (auto& bank_row : bank) {
        for (auto& b : bank_row) {
          b.reset();
        }
      }
      for (auto& pn_row : parameter_number) {
        for (auto& pn : pn_row) {
          pn.reset();
        }
      }
      output.clear();
    }

    /// \brief Records the most recent Bank Select MSB and LSB values received on a channel.
    struct bank {
      std::uint8_t msb_valid : 1 = false;  ///< True if a Bank Select MSB value has been received.
      std::uint8_t msb : 7 = 0;            ///< Set by bank_select control
      std::uint8_t lsb_valid : 1 = false;  ///< True if a Bank Select LSB value has been received.
      std::uint8_t lsb : 7 = 0;            ///< Set by bank_select_lsb control

      /// \brief Records a Bank Select MSB value.
      /// \param value  The 7-bit Bank Select MSB value.
      constexpr void set_msb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        msb = value;
        msb_valid = true;
      }
      /// \brief Records a Bank Select LSB value.
      /// \param value  The 7-bit Bank Select LSB value.
      constexpr void set_lsb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        lsb = value;
        lsb_valid = true;
      }
      /// \brief Returns true if both the Bank Select MSB and LSB values have been received.
      /// \returns True if both the Bank Select MSB and LSB values have been received.
      [[nodiscard]] constexpr bool is_valid() const noexcept { return lsb_valid && msb_valid; }
      /// \brief Forgets any recorded Bank Select values.
      constexpr void reset() noexcept {
        msb_valid = false;
        msb = 0;
        lsb_valid = false;
        lsb = 0;
      }
    };

    /// Represents the status of registered (RPN) or non-registered/assignable (NRPN) parameters
    struct parameter_number {
      bool pn_is_rpn = false;                 ///< Is this nrpn or rpn?
      std::uint8_t pn_msb_valid : 1 = false;  ///< True if a parameter number MSB has been received.
      std::uint8_t pn_msb : 7 = 0;            ///< Set by nrpn_msb/rpn_msb
      std::uint8_t pn_lsb_valid : 1 = false;  ///< True if a parameter number LSB has been received.
      std::uint8_t pn_lsb : 7 = 0;            ///< Set by nrpn_lsb/rpn_lsb

      std::uint8_t value_msb_valid : 1 = false;  ///< True if a Data Entry MSB value has been received.
      std::uint8_t value_msb : 7 = 0;            ///< Set by data_entry_msb

      /// \brief Records the MSB of the parameter number.
      /// \param value  The 7-bit parameter number MSB.
      constexpr void set_number_msb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        pn_msb = value;
        pn_msb_valid = true;
      }
      /// \brief Records the LSB of the parameter number.
      /// \param value  The 7-bit parameter number LSB.
      constexpr void set_number_lsb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        pn_lsb = value;
        pn_lsb_valid = true;
      }
      /// \brief Forgets the selected parameter number. The Data Entry MSB value is unaffected.
      constexpr void reset() noexcept {
        pn_msb_valid = false;
        pn_msb = 0;
        pn_lsb_valid = false;
        pn_lsb = 0;
      }
      /// \brief Records the Data Entry MSB value.
      /// \param value  The 7-bit Data Entry MSB value.
      constexpr void set_value_msb(std::uint8_t const value) noexcept {
        assert(value < 0x80);
        value_msb = value;
        value_msb_valid = true;
      }
    };

    /// \brief The FIFO of translated 32-bit UMP words waiting to be consumed by pop().
    adt::fifo<std::uint32_t, 4> output;
    // TODO: the two following arrays take a disproportionate of space (~1.5K). Perhaps
    // swap them for an LRU cache of the controller values? This trades messages against storage,
    // but how frequently are controller values for the entire range of possible values being sent?
    /// An alias template for a two-dimensional std::array
    template <typename T, std::size_t Row, std::size_t Col> using array2d = std::array<std::array<T, Col>, Row>;
    /// \brief The Bank Select state for each group and channel.
    array2d<bank, 16, 16> bank{};
    /// \brief The RPN/NRPN state for each group and channel.
    array2d<parameter_number, 16, 16> parameter_number{};
    /// \brief The group passed to the constructor.
    /// \note This value is not used during translation.
    std::uint8_t group = 0;
  };

  /// \brief The ump_dispatcher configuration which performs the translation.
  struct to_midi2_config {
    /// \brief Utility messages go straight through.
    struct utility {
      /// \brief Discards a NOOP message.
      static constexpr void noop(context const* const) { /* Just consume NOOP messages */
      }
      /// \brief Passes a JR Clock message (M2-104 section 7.2.2.1) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void jr_clock(context* const ctxt, ump::utility::jr_clock const& in) { ctxt->push(in); }
      /// \brief Passes a JR Timestamp message (M2-104 section 7.2.2.2) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void jr_timestamp(context* const ctxt, ump::utility::jr_timestamp const& in) { ctxt->push(in); }
      /// \brief Passes a Delta Clockstamp Ticks Per Quarter Note (DCTPQ) message (M2-104 section 7.2.3.1) through to
      /// the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void delta_clockstamp_tpqn(context* const ctxt, ump::utility::delta_clockstamp_tpqn const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Delta Clockstamp (DC) message (M2-104 section 7.2.3.2) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void delta_clockstamp(context* const ctxt, ump::utility::delta_clockstamp const& in) {
        ctxt->push(in);
      }
      /// \brief Discards a message whose message type or status is reserved or not recognized.
      static constexpr void unknown(context const*, std::span<std::uint32_t>) { /* Just drop bad messages */
      }
    };
    /// \brief System messages go straight through.
    struct system {
      /// \brief Passes a MIDI Time Code message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void midi_time_code(context* const ctxt, ump::system::midi_time_code const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Song Position Pointer message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void song_position_pointer(context* const ctxt, ump::system::song_position_pointer const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Song Select message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void song_select(context* const ctxt, ump::system::song_select const& in) { ctxt->push(in); }
      /// \brief Passes a Tune Request message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void tune_request(context* const ctxt, ump::system::tune_request const& in) { ctxt->push(in); }
      /// \brief Passes a Timing Clock message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void timing_clock(context* const ctxt, ump::system::timing_clock const& in) { ctxt->push(in); }
      /// \brief Passes a Start message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void seq_start(context* const ctxt, ump::system::sequence_start const& in) { ctxt->push(in); }
      /// \brief Passes a Continue message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void seq_continue(context* const ctxt, ump::system::sequence_continue const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Stop message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void seq_stop(context* const ctxt, ump::system::sequence_stop const& in) { ctxt->push(in); }
      /// \brief Passes a Active Sensing message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void active_sensing(context* const ctxt, ump::system::active_sensing const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Reset message (M2-104 section 7.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void reset(context* const ctxt, ump::system::reset const& in) { ctxt->push(in); }
    };
    /// \brief MIDI 1.0 Channel Voice messages are converted to MIDI 2.0 Channel Voice messages.
    class m1cvm {
    public:
      /// \brief Translates a MIDI 1.0 Note Off message (M2-104 section 7.3.1) to its MIDI 2.0 equivalent and pushes the
      /// result to the output.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static void note_off(context* ctxt, ump::m1cvm::note_off const& in);
      /// \brief Translates a MIDI 1.0 Note On message (M2-104 section 7.3.2) to its MIDI 2.0 equivalent and pushes the
      /// result to the output.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static void note_on(context* ctxt, ump::m1cvm::note_on const& in);
      /// \brief Translates a MIDI 1.0 Poly Pressure message (M2-104 section 7.3.3) to its MIDI 2.0 equivalent and
      /// pushes the result to the output.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static void poly_pressure(context* ctxt, ump::m1cvm::poly_pressure const& in);
      /// \brief Translates a MIDI 1.0 Control Change message (M2-104 section 7.3.4) to its MIDI 2.0 equivalent and
      /// pushes the result to the output.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static void control_change(context* ctxt, ump::m1cvm::control_change const& in);
      /// \brief Translates a MIDI 1.0 Program Change message (M2-104 section 7.3.5) to its MIDI 2.0 equivalent and
      /// pushes the result to the output.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static void program_change(context* ctxt, ump::m1cvm::program_change const& in);
      /// \brief Translates a MIDI 1.0 Channel Pressure message (M2-104 section 7.3.6) to its MIDI 2.0 equivalent and
      /// pushes the result to the output.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static void channel_pressure(context* ctxt, ump::m1cvm::channel_pressure const& in);
      /// \brief Translates a MIDI 1.0 Pitch Bend message (M2-104 section 7.3.7) to its MIDI 2.0 equivalent and pushes
      /// the result to the output.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static void pitch_bend(context* ctxt, ump::m1cvm::pitch_bend const& in);

    private:
      template <typename T>
      static void pn_control_message(struct context* const ctxt, struct context::parameter_number const& c,
                                     std::uint8_t const group, std::uint8_t const channel, std::uint8_t const value) {
        auto const out = T{}.group(group).channel(channel).bank(c.pn_msb).index(c.pn_lsb).value(
            mcm_scale<14, 32>(static_cast<std::uint16_t>((static_cast<std::uint16_t>(c.value_msb) << 7) | value)));
        ctxt->push(out);
      }
    };
    /// \brief Data 64-bit messages go straight through.
    struct data64 {
      /// \brief Passes a System Exclusive (7-bit) Complete in One UMP message (M2-104 section 7.7) through to the
      /// output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void sysex7_in_1(context* const ctxt, ump::data64::sysex7_in_1 const& in) { ctxt->push(in); }
      /// \brief Passes a System Exclusive (7-bit) Start message (M2-104 section 7.7) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void sysex7_start(context* const ctxt, ump::data64::sysex7_start const& in) { ctxt->push(in); }
      /// \brief Passes a System Exclusive (7-bit) Continue message (M2-104 section 7.7) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void sysex7_continue(context* const ctxt, ump::data64::sysex7_continue const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a System Exclusive (7-bit) End message (M2-104 section 7.7) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void sysex7_end(context* const ctxt, ump::data64::sysex7_end const& in) { ctxt->push(in); }
    };
    /// \brief MIDI 2.0 Channel Voice messages go straight through.
    struct m2cvm {
      /// \brief Passes a MIDI 2.0 Note Off message (M2-104 section 7.4.1) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void note_off(context* const ctxt, ump::m2cvm::note_off const& in) { ctxt->push(in); }
      /// \brief Passes a MIDI 2.0 Note On message (M2-104 section 7.4.2) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void note_on(context* const ctxt, ump::m2cvm::note_on const& in) { ctxt->push(in); }
      /// \brief Passes a MIDI 2.0 Poly Pressure message (M2-104 section 7.4.3) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void poly_pressure(context* const ctxt, ump::m2cvm::poly_pressure const& in) { ctxt->push(in); }
      /// \brief Passes a MIDI 2.0 Program Change message (M2-104 section 7.4.9) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void program_change(context* const ctxt, ump::m2cvm::program_change const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Channel Pressure message (M2-104 section 7.4.10) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void channel_pressure(context* const ctxt, ump::m2cvm::channel_pressure const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Registered Controller (RPN) message (M2-104 section 7.4.7) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void rpn_controller(context* const ctxt, ump::m2cvm::rpn_controller const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Assignable Controller (NRPN) message (M2-104 section 7.4.7) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void nrpn_controller(context* const ctxt, ump::m2cvm::nrpn_controller const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Registered Per-Note Controller message (M2-104 section 7.4.4) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void rpn_per_note_controller(context* const ctxt,
                                                    ump::m2cvm::rpn_per_note_controller const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Assignable Per-Note Controller message (M2-104 section 7.4.4) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void nrpn_per_note_controller(context* const ctxt,
                                                     ump::m2cvm::nrpn_per_note_controller const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Relative Registered Controller (RPN) message (M2-104 section 7.4.8) through to the
      /// output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void rpn_relative_controller(context* const ctxt,
                                                    ump::m2cvm::rpn_relative_controller const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Relative Assignable Controller (NRPN) message (M2-104 section 7.4.8) through to the
      /// output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void nrpn_relative_controller(context* const ctxt,
                                                     ump::m2cvm::nrpn_relative_controller const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Per-Note Management message (M2-104 section 7.4.5) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void per_note_management(context* const ctxt, ump::m2cvm::per_note_management const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Control Change message (M2-104 section 7.4.6) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void control_change(context* const ctxt, ump::m2cvm::control_change const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a MIDI 2.0 Pitch Bend message (M2-104 section 7.4.11) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void pitch_bend(context* const ctxt, ump::m2cvm::pitch_bend const& in) { ctxt->push(in); }
      /// \brief Passes a MIDI 2.0 Per-Note Pitch Bend message (M2-104 section 7.4.12) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      static constexpr void per_note_pitch_bend(context* const ctxt, ump::m2cvm::per_note_pitch_bend const& in) {
        ctxt->push(in);
      }
    };
    /// \brief Data 128-bit messages go straight through.
    struct data128 {
      /// \brief Passes a System Exclusive 8 (8-bit) Complete in One UMP message (M2-104 section 7.8) through to the
      /// output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void sysex8_in_1(context* const ctxt, ump::data128::sysex8_in_1 const& in) { ctxt->push(in); }
      /// \brief Passes a System Exclusive 8 (8-bit) Start message (M2-104 section 7.8) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void sysex8_start(context* const ctxt, ump::data128::sysex8_start const& in) { ctxt->push(in); }
      /// \brief Passes a System Exclusive 8 (8-bit) Continue message (M2-104 section 7.8) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void sysex8_continue(context* const ctxt, ump::data128::sysex8_continue const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a System Exclusive 8 (8-bit) End message (M2-104 section 7.8) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void sysex8_end(context* const ctxt, ump::data128::sysex8_end const& in) { ctxt->push(in); }
      /// \brief Passes a Mixed Data Set Header message (M2-104 section 7.9) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void mds_header(context* const ctxt, ump::data128::mds_header const& in) { ctxt->push(in); }
      /// \brief Passes a Mixed Data Set Payload message (M2-104 section 7.9) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void mds_payload(context* const ctxt, ump::data128::mds_payload const& in) { ctxt->push(in); }
    };
    /// \brief UMP Stream messages go straight through.
    struct stream {
      /// \brief Passes a Endpoint Discovery message (M2-104 section 7.1.1) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void endpoint_discovery(context* const ctxt, ump::stream::endpoint_discovery const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Endpoint Info Notification message (M2-104 section 7.1.2) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void endpoint_info_notification(context* const ctxt,
                                                       ump::stream::endpoint_info_notification const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Device Identity Notification message (M2-104 section 7.1.3) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void device_identity_notification(context* const ctxt,
                                                         ump::stream::device_identity_notification const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Endpoint Name Notification message (M2-104 section 7.1.4) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void endpoint_name_notification(context* const ctxt,
                                                       ump::stream::endpoint_name_notification const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Product Instance Id Notification message (M2-104 section 7.1.5) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void product_instance_id_notification(context* const ctxt,
                                                             ump::stream::product_instance_id_notification const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Stream Configuration Request message (M2-104 section 7.1.6.2) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void jr_configuration_request(context* const ctxt,
                                                     ump::stream::jr_configuration_request const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Stream Configuration Notification message (M2-104 section 7.1.6.3) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void jr_configuration_notification(context* const ctxt,
                                                          ump::stream::jr_configuration_notification const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Function Block Discovery message (M2-104 section 7.1.7) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void function_block_discovery(context* const ctxt,
                                                     ump::stream::function_block_discovery const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Function Block Info Notification message (M2-104 section 7.1.8) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void function_block_info_notification(context* const ctxt,
                                                             ump::stream::function_block_info_notification const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Function Block Name Notification message (M2-104 section 7.1.9) through to the output
      /// unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void function_block_name_notification(context* const ctxt,
                                                             ump::stream::function_block_name_notification const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Start of Clip message (M2-104 section 7.1.10) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void start_of_clip(context* const ctxt, ump::stream::start_of_clip const& in) { ctxt->push(in); }
      /// \brief Passes a End of Clip message (M2-104 section 7.1.11) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void end_of_clip(context* const ctxt, ump::stream::end_of_clip const& in) { ctxt->push(in); }
    };
    /// \brief Flex Data messages go straight through.
    struct flex_data {
      /// \brief Passes a Set Tempo message (M2-104 section 7.5.3) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void set_tempo(context* const ctxt, ump::flex_data::set_tempo const& in) { ctxt->push(in); }
      /// \brief Passes a Set Time Signature message (M2-104 section 7.5.4) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void set_time_signature(context* const ctxt, ump::flex_data::set_time_signature const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Set Metronome message (M2-104 section 7.5.5) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void set_metronome(context* const ctxt, ump::flex_data::set_metronome const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Set Key Signature message (M2-104 section 7.5.7) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void set_key_signature(context* const ctxt, ump::flex_data::set_key_signature const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Set Chord Name message (M2-104 section 7.5.8) through to the output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void set_chord_name(context* const ctxt, ump::flex_data::set_chord_name const& in) {
        ctxt->push(in);
      }
      /// \brief Passes a Flex Data text (Text Messages Common Format) message (M2-104 section 7.5.9) through to the
      /// output unchanged.
      /// \param ctxt  The translator context.
      /// \param in  The incoming message.
      constexpr static void text(context* const ctxt, ump::flex_data::text_common const& in) { ctxt->push(in); }
    };
    struct context* context = nullptr;               ///< The translator context.
    [[no_unique_address]] struct utility utility{};  ///< Handlers for utility messages.
    [[no_unique_address]] struct system system{};    ///< Handlers for system messages.
    [[no_unique_address]] class m1cvm m1cvm{};       ///< Handlers for MIDI 1.0 channel voice messages.
    [[no_unique_address]] struct data64 data64{};    ///< Handlers for data 64-bit messages.
    [[no_unique_address]] struct m2cvm m2cvm{};      ///< Handlers for MIDI 2.0 channel voice messages.
    [[no_unique_address]] struct data128 data128{};  ///< Handlers for data 128-bit messages.
    [[no_unique_address]] struct stream stream{};    ///< Handlers for UMP stream messages.
    [[no_unique_address]] struct flex_data flex{};   ///< Handlers for flex data messages.
  };

  context context_;
  ump_dispatcher<to_midi2_config> p_{to_midi2_config{.context = &context_}};
};

static_assert(translator<std::uint32_t, std::uint32_t, ump_to_midi2>);

}  // end namespace midi2::ump

#endif  // MIDI2_UMP_TO_MIDI2_HPP
