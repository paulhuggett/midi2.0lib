Protocol translators
====================

.. code-block:: cpp

   #include "midi2/ump/ump_to_midi2.hpp"
   #include "midi2/ump/ump_to_midi1.hpp"

   namespace midi2::ump {
     class ump_to_midi2;
     class to_midi1;
   }

The two classes translate a stream of UMP words between the MIDI 1.0 and MIDI 2.0
Protocols, following appendix D (Translation) of M2-104. Both are translators: they
satisfy :cpp:concept:`midi2::translator`, with ``std::uint32_t`` as their input and
output types, and have the ``push()``, ``empty()``, ``pop()``, and ``reset()`` members
described in :ref:`reference/library:translators`. For each word of input there are at
most four words of output.

Neither translator can be copied or moved safely: see
:ref:`reference/library:copying translators`.

``ump_to_midi2``
----------------

``class ump_to_midi2``

   Replaces MIDI 1.0 Channel Voice messages with their MIDI 2.0 Channel Voice
   equivalents.

``explicit constexpr ump_to_midi2(std::uint8_t group) noexcept``
   *Requires:* ``group`` is at most 15.

   Constructs a translator. ``group`` is not used: each output message has the group of
   the input message that produced it.

``void flush(std::uint8_t group, std::uint8_t channel)``
   *Requires:* ``group`` and ``channel`` are at most 15.

   If a Data Entry MSB is held for ``group`` and ``channel`` (see below), produces the
   Registered or Assignable Controller message for it. Otherwise does nothing. Pop all
   waiting output before calling it.

``constexpr void reset()``
   Discards any waiting output and forgets every Bank Select, parameter number, and held
   Data Entry value.

Translation:

* Note Off, Note On, Poly Pressure, Channel Pressure, and Pitch Bend become the MIDI 2.0
  messages of the same names. 7-bit values are scaled to 16 or 32 bits, and the 14-bit
  Pitch Bend to 32 bits, with :cpp:func:`midi2::ump::mcm_scale`.
* A Bank Select MSB or LSB (Control Change 0 or 32) is recorded for its group and channel
  and produces no output. A Program Change becomes a MIDI 2.0 Program Change; if both
  Bank Select values have been recorded for its group and channel, the Bank Valid flag is
  set and the bank is included.
* An RPN or NRPN parameter number (Control Change 101, 100, 99, or 98) is recorded for its
  group and channel and produces no output. Selecting the RPN Null Function (7F, 7F)
  deselects the parameter number.
* A Data Entry MSB (Control Change 6) is held. It is sent, as a MIDI 2.0 Registered
  Controller (for an RPN) or Assignable Controller (for an NRPN) whose value is the MSB
  and LSB scaled to 32 bits, when a Data Entry LSB (Control Change 38), another Data
  Entry MSB, a parameter number, or Reset All Controllers arrives on the same group and
  channel, or when ``flush()`` is called. If no Data Entry LSB has arrived, the LSB is
  taken to be 0. Data Entry with no parameter number selected produces no output.
* Every other Control Change becomes a MIDI 2.0 Control Change with its value scaled to
  32 bits. Reset All Controllers (Control Change 121) also deselects the parameter
  number.
* A NOOP, and a message with a reserved message type or status, is discarded. Every
  other message is passed through unchanged.

``to_midi1``
------------

``class to_midi1``

   Replaces MIDI 2.0 Channel Voice messages with their MIDI 1.0 Channel Voice
   equivalents.

``to_midi1()``
   Constructs a translator.

``void reset()``
   Discards any waiting output and forgets the parameter numbers most recently sent.

Translation:

* Note Off, Note On, Poly Pressure, Control Change, Channel Pressure, and Pitch Bend
  become the MIDI 1.0 messages of the same names. Values are scaled down to 7 or 14 bits
  by discarding their least significant bits. The attributes of Note On and Note Off are
  discarded.
* A Program Change becomes a MIDI 1.0 Program Change. If its Bank Valid flag is set, it
  is preceded by Bank Select MSB and LSB.
* A Registered Controller or Assignable Controller becomes the Control Change messages of
  an RPN or NRPN sequence: parameter number MSB and LSB (Control Change 101 and 100, or 99
  and 98), then Data Entry MSB and LSB (Control Change 6 and 38). The translator
  remembers the parameter number that it most recently sent for each group, channel, and
  kind (RPN or NRPN), and doesn't send it again if it hasn't changed. It remembers at most
  32 of these combinations, in a pseudo-LRU cache.
* The MIDI 2.0 messages that have no MIDI 1.0 equivalent are discarded: the per-note
  controllers, the relative controllers, Per-Note Management, and Per-Note Pitch Bend.
* System, MIDI 1.0 Channel Voice, and SysEx7 messages are passed through unchanged.
  Every other message is discarded.

To send MIDI 2.0 Protocol messages to a MIDI 1.0 bytestream, give the output of
``to_midi1`` to a :cpp:class:`midi2::bytestream::to_bytestream`.
