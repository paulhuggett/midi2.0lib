Send and receive System Exclusive messages over UMP
===================================================

UMP carries a 7-bit System Exclusive message as a sequence of SysEx7 packets (Data 64-bit
messages), each holding up to six bytes. A message of up to six bytes is sent as one
"complete in one" packet. A longer message is sent as a Start packet, any number of
Continue packets, and an End packet. The ``F0`` and ``F7`` bytes are not sent.

This program splits a message into packets, then reassembles it with a dispatcher:

.. literalinclude:: ../examples/howto_sysex7.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   30167E7F 06027D01 30260002 00010000 30320000 00000000
   received 14 bytes: 7E 7F 06 02 7D 01 00 02 00 01 00 00 00 00

Sending
-------

The four packet types are ``midi2::ump::data64::sysex7_in_1``, ``sysex7_start``,
``sysex7_continue``, and ``sysex7_end``. Their data bytes are a container of up to
``max_size()`` (six) bytes:

* ``data(range)`` copies the bytes of a range into the packet and sets its
  ``number_of_bytes`` field to the number copied. There are also overloads that take
  an initializer list, a range and a projection (for example, to convert ``std::byte``
  to an integer), or an iterator and a sentinel.
* ``data(index)`` reads one byte, and ``data(index, value)`` writes one. The fields can
  also be reached individually as ``data0()`` to ``data5()``.

Receiving
---------

* A dispatcher passes each packet to the ``sysex7_in_1()``, ``sysex7_start()``,
  ``sysex7_continue()``, or ``sysex7_end()`` handler of its ``data64`` backend.
* A packet is a range of its data bytes: ``begin()`` and ``end()`` cover the first
  ``number_of_bytes()`` bytes, ``size()`` returns their number, and ``operator[]`` reads
  or writes one of them.
* The dispatcher doesn't reassemble messages, because it doesn't allocate memory. Your
  handlers decide where the bytes go. Often they don't need to be stored at all: a
  :cpp:class:`midi2::ci::ci_dispatcher` can be given each byte as it arrives, as
  :doc:`carry-midi-ci-over-ump` shows.
* Packets on different groups belong to different messages. If you receive more than one
  group, keep the state for each group separately.

To convert System Exclusive messages between a MIDI 1.0 bytestream and UMP, use
:cpp:class:`midi2::bytestream::to_ump` and :cpp:class:`midi2::bytestream::to_bytestream`,
which do all of this for you.

The 8-bit System Exclusive messages (``midi2::ump::data128::sysex8_in_1`` and the
others) work in the same way, with up to 13 bytes in each packet. Each of their packets
also carries a stream ID, so that several messages can be interleaved on one group.
