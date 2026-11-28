Bytestream translators
======================

.. code-block:: cpp

   #include "midi2/bytestream/bytestream_to_ump.hpp"
   #include "midi2/bytestream/ump_to_bytestream.hpp"
   #include "midi2/bytestream/usbm1_to_bytestream.hpp"
   #include "midi2/bytestream/bytestream_types.hpp"

   namespace midi2::bytestream {
     class to_ump;
     class to_bytestream;
     class usbm1_to_bytestream;
     enum class status : std::uint8_t;
     constexpr bool is_system_real_time_message(std::byte) noexcept;
     constexpr bool is_status_byte(std::byte) noexcept;
   }

The three classes are translators: each satisfies :cpp:concept:`midi2::translator`, and
has ``push()``, ``empty()``, ``pop()``, and ``reset()`` members as described in
:ref:`reference/library:translators`. This page describes how each one translates, and
the members that it adds.

``to_ump``
----------

``class to_ump``

   Translates a MIDI 1.0 bytestream to MIDI 1.0 Protocol UMP messages. ``input_type`` is
   ``std::byte`` and ``output_type`` is ``std::uint32_t``.

``constexpr to_ump() noexcept``
   Constructs a translator for group 0.

``explicit constexpr to_ump(std::uint8_t group) noexcept``
   *Requires:* ``group`` is at most 15.

   Constructs a translator whose output is in group ``group``.

``void set_group(std::uint8_t group) noexcept``
   *Requires:* ``group`` is at most 15.

   Resets the translator, then sets the group of its output to ``group``.

``void reset() noexcept``
   Discards any partly received message and any waiting output, forgets the running
   status, and sets the group to 0.

Translation:

* A Channel Voice message becomes a MIDI 1.0 Channel Voice message (UMP message type
  0x2). Running status is supported. A Note On with velocity 0 is passed on as it is.
* A System Common or System Real Time message becomes a System message (message type
  0x1).
* A System Real Time byte (``F8``, ``FA``, ``FB``, ``FC``, ``FE``, or ``FF``) is translated
  as soon as it arrives, even between the bytes of another message, which it doesn't
  affect.
* The data bytes of a System Exclusive message become a sequence of SysEx7 packets
  (message type 0x3) of up to six bytes. ``F0`` and ``F7`` are not included. A message of
  up to six bytes becomes a single "complete in one" packet. A packet is produced when it
  is full and the next data byte arrives, or when the message ends.
* Any other status byte ends a System Exclusive message, as if ``F7`` had been received
  first.
* Data bytes before the first status byte are ignored. The reserved status bytes ``F4``,
  ``F5``, ``F9``, and ``FD`` produce no output, but, like the other status bytes that
  aren't System Real Time, they end a System Exclusive message and cancel running
  status.

For each byte of input there are at most four words of output.

``to_bytestream``
-----------------

``class to_bytestream``

   Translates MIDI 1.0 Protocol UMP messages to a MIDI 1.0 bytestream. ``input_type`` is
   ``std::uint32_t`` and ``output_type`` is ``std::byte``.

   Don't copy or move a ``to_bytestream``: see
   :ref:`reference/library:copying translators`.

``constexpr to_bytestream()``
   Constructs a translator that passes the messages of every group.

``constexpr void group_filter(std::uint16_t group_bitmap) noexcept``
   Passes only the messages of the groups whose bits are set in ``group_bitmap``: bit
   *n* selects group *n*. Messages of the other groups are discarded. A value of 0
   selects every group.

``constexpr void reset() noexcept``
   Discards any waiting output and any partly received System Exclusive message, forgets
   the running status, and selects every group.

Translation:

* A MIDI 1.0 Channel Voice message becomes the equivalent bytestream message. Running
  status is used: the status byte is omitted when it is the same as the previous
  Channel Voice message's status byte.
* A System message becomes the equivalent bytestream message. A System Common message
  cancels running status; a System Real Time message doesn't.
* A sequence of SysEx7 packets becomes ``F0``, the data bytes, and ``F7``. A Continue or
  End packet is discarded unless the last status byte output was the ``F0`` of a Start
  packet. A "complete in one" packet with no data bytes is discarded.
* Every other message is discarded. That includes MIDI 2.0 Channel Voice messages: pass
  them to a :cpp:class:`midi2::ump::to_midi1` first.

For each word of input there are at most eight bytes of output.

``usbm1_to_bytestream``
-----------------------

``class usbm1_to_bytestream``

   Extracts the MIDI 1.0 bytestream from the USB-MIDI 1.0 Event Packets of one virtual
   cable. ``input_type`` is ``std::uint32_t``: one Event Packet, with the cable number
   and Code Index Number (CIN) in its most significant byte. ``output_type`` is
   ``std::byte``.

``constexpr usbm1_to_bytestream() noexcept``
   Constructs a translator for cable 0.

``explicit constexpr usbm1_to_bytestream(std::uint8_t cable) noexcept``
   *Requires:* ``cable`` is at most 15.

   Constructs a translator for cable ``cable``.

``constexpr void set_cable(std::uint8_t cable)``
   *Requires:* ``cable`` is at most 15.

   Resets the translator, then selects cable ``cable``.

``constexpr std::uint8_t get_cable() const noexcept``
   Returns the selected cable.

``constexpr void reset()``
   Discards any waiting output and selects cable 0.

Translation: a packet for another cable is ignored. Otherwise, the CIN gives the number of
the packet's three MIDI bytes that are used (0 to 3), and those bytes are output in order.
The bytes are not otherwise checked. For each packet there are at most three bytes of
output.

Bytestream types
----------------

``enum class status : std::uint8_t``
   The status bytes of a MIDI 1.0 bytestream. The Channel Voice statuses, ``note_off``
   (0x80) to ``pitch_bend`` (0xE0), have a channel number of 0. The System statuses are
   ``sysex_start`` (0xF0) to ``system_reset`` (0xFF), including ``reserved1`` to
   ``reserved4`` for the undefined statuses.

``constexpr bool is_system_real_time_message(std::byte b) noexcept``
   Returns ``true`` if ``b`` is the status byte of a System Real Time message: 0xF8,
   0xFA, 0xFB, 0xFC, 0xFE, or 0xFF.

``constexpr bool is_status_byte(std::byte b) noexcept``
   Returns ``true`` if the most significant bit of ``b`` is set.
