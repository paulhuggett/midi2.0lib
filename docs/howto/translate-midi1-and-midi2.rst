Translate between the MIDI 1.0 and MIDI 2.0 Protocols
=====================================================

A UMP endpoint uses either the MIDI 1.0 Protocol or the MIDI 2.0 Protocol for its
Channel Voice messages. :cpp:class:`midi2::ump::ump_to_midi2` and
:cpp:class:`midi2::ump::to_midi1` translate between them, following the "Translation"
appendix of M2-104. Translation is more than scaling values: a MIDI 1.0 Bank Select or
RPN/NRPN sequence is several Control Change messages, but becomes a single MIDI 2.0
message, and the reverse.

.. literalinclude:: ../examples/howto_protocol.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   MIDI 1.0 to MIDI 2.0
     bank select and program change: 40C00001 03000102
     RPN 0 (pitch bend sensitivity): (held) then flush: 40200000 18000000
   MIDI 2.0 to MIDI 1.0
     note on: 20903C7F
     registered controller: 20B06500 20B06400 20B0060C 20B02600

``push()`` in this program uses :cpp:func:`midi2::ump::apply` to give each word of a
message to a translator, and drains the translator after each word.

MIDI 1.0 to MIDI 2.0
--------------------

* Bank Select MSB and LSB (Control Change 0 and 32) are remembered for each group and
  channel, and produce no output. A following Program Change becomes a MIDI 2.0 Program
  Change with the Bank Valid flag set, if both values have been received.
* RPN and NRPN parameter numbers (Control Change 101, 100, 99, and 98) are remembered in
  the same way. Data Entry (Control Change 6 and 38) produces a MIDI 2.0 Registered or
  Assignable Controller message, or nothing if no parameter number has been selected.
  Selecting the RPN Null Function (7F, 7F) deselects the parameter number.
* The value of a Data Entry MSB is held until a Data Entry LSB, another Data Entry MSB,
  or a parameter number shows that it is complete (M2-104 section D.3.3). Some MIDI 1.0
  devices never send the LSB, so call
  :cpp:func:`flush(group, channel) <midi2::ump::ump_to_midi2::flush>` (for example,
  after a short timeout) to send a held value. It produces at most one message, and
  nothing if no value is held.
* Other messages are scaled up from 7 or 14 bits with the min-center-max algorithm (see
  :doc:`scale-values`). Every output message has the group of the input message that
  produced it.
* Messages that aren't MIDI 1.0 Channel Voice messages pass through unchanged, except
  NOOP and messages with a reserved message type, which are discarded.

MIDI 2.0 to MIDI 1.0
--------------------

* Values are scaled down by discarding their least significant bits.
* A Program Change with the Bank Valid flag set becomes Bank Select MSB, Bank Select LSB,
  and Program Change.
* A Registered or Assignable Controller becomes the four Control Change messages of an
  RPN or NRPN sequence: the parameter number MSB and LSB, then Data Entry MSB and LSB.
  The translator remembers the parameter numbers that it most recently sent, and leaves
  them out when they haven't changed.
* The MIDI 2.0 messages with no MIDI 1.0 equivalent (the per-note controllers, the
  relative controllers, Per-Note Management, and Per-Note Pitch Bend) are discarded.
* System, MIDI 1.0 Channel Voice, and SysEx7 messages pass through unchanged. All other
  messages are discarded.

Both translators have the same interface as the other translators: see
:ref:`reference/library:translators`.
