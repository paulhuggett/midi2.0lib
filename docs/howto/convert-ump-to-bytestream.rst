Convert UMP to a MIDI 1.0 bytestream
====================================

:cpp:class:`midi2::bytestream::to_bytestream` converts MIDI 1.0 Protocol UMP messages
to the bytes of a MIDI 1.0 bytestream, for example to drive a 5-pin DIN port. A UMP
stream can carry 16 groups but a bytestream has no groups, so you usually select the
groups that you want with ``group_filter()``:

.. literalinclude:: ../examples/howto_ump_to_bytestream.cpp
   :language: cpp
   :start-after: SPDX-License-Identifier: MIT

Output:

.. code-block:: text

   90 3C 40 40 40 F8 80 3C 00

How it works
------------

* ``group_filter()`` takes a bitmap in which bit *n* selects group *n*. Messages in the
  other groups are discarded, like the third Note On here. By default, every group is
  selected, and the messages of all of the groups are merged into one bytestream.
* Running status is used for Channel Voice messages, so the second Note On has no status
  byte. A System Common message cancels running status. A System Real Time message
  doesn't.
* The SysEx7 packets of a System Exclusive message become ``F0``, the data bytes, and
  ``F7``. A Continue or End packet that doesn't follow a Start packet is discarded.
* Only the System, MIDI 1.0 Channel Voice, and SysEx7 messages have bytestream
  equivalents. Everything else, including MIDI 2.0 Channel Voice messages, is
  discarded. To send MIDI 2.0 Protocol messages to a MIDI 1.0 port, first pass them to
  a :cpp:class:`midi2::ump::to_midi1`, as the :doc:`tutorial <../tutorial>` does.
* :cpp:func:`reset() <midi2::bytestream::to_bytestream::reset>` discards any waiting
  output, forgets the running status, and selects every group again.
