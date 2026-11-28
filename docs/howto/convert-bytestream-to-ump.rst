Convert a MIDI 1.0 bytestream to UMP
====================================

:cpp:class:`midi2::bytestream::to_ump` converts the bytes of a MIDI 1.0 bytestream, such
as the data from a 5-pin DIN port, to MIDI 1.0 Protocol UMP messages. Push each byte
as it arrives and pop the words that it produces:

.. literalinclude:: ../examples/howto_bytestream_to_ump.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   11F80000
   31047E7F
   06010000
   21923C40

How it works
------------

* The constructor's argument is the UMP group of every message that the translator
  produces. ``set_group()`` changes it, but also discards any message that is partly
  received.
* Channel Voice messages become MIDI 1.0 Channel Voice UMP messages (message type 0x2),
  and System Common and System Real Time messages become System messages (message type
  0x1). Running status is supported.
* A System Real Time byte can appear anywhere, even between the bytes of another
  message, and is translated at once. It doesn't affect the message it interrupts: here,
  the Timing Clock (``F8``) is translated before the System Exclusive message around it.
* The data bytes of a System Exclusive message are packed into SysEx7 packets (message
  type 0x3) of up to six bytes. ``F0`` and ``F7`` are not included. A packet is produced
  once it is full and the next byte has arrived, or when the message ends, so the last
  packet of a message is produced by the ``F7`` that ends it. A message of up to six bytes
  becomes a single "complete in one" packet, like the four-byte message here.
* Any status byte other than a System Real Time byte ends a System Exclusive message,
  even if no ``F7`` was received.
* :cpp:func:`reset() <midi2::bytestream::to_ump::reset>` discards any message that is
  partly received and also sets the group back to 0.

To translate the UMP messages to the MIDI 2.0 Protocol, pass them to a
:cpp:class:`midi2::ump::ump_to_midi2`, as the :doc:`tutorial <../tutorial>` does.
